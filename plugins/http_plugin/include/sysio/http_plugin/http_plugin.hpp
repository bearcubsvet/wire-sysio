#pragma once

#include <sysio/chain/application.hpp>
#include <sysio/chain/exceptions.hpp>
#include <sysio/http_plugin/api_category.hpp>
#include <fc/exception/exception.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/io/json.hpp>
namespace sysio {
   using namespace appbase;

   /**
    * @brief A callback function provided to a URL handler to
    * allow it to specify the HTTP response code and body
    *
    * Arguments: response_code, response_body
    */
   using url_response_callback = std::function<void(int,std::optional<fc::variant>)>;

   /**
    * @brief Callback type for a URL handler
    *
    * URL handlers have this type
    *
    * The handler must guarantee that url_response_callback() is called;
    * otherwise, the connection will hang and result in a memory leak.
    *
    * Arguments: url, request_body, response_callback
    **/
   using url_handler = std::function<void(string&&, string&&, url_response_callback&&)>;

   /**
    * @brief An API, containing URLs and handlers
    *
    * An API is composed of several calls, where each call has a URL and
    * a handler. The URL is the path on the web server that triggers the
    * call, and the handler is the function which implements the API call
    */
   struct api_entry {
      string path;
      api_category category;
      url_handler handler;
   };

   using api_description = std::vector<api_entry>;

   enum class http_content_type {
      json = 1,
      plaintext = 2
   };

   struct http_plugin_defaults {
      //If empty, unix socket support will be completely disabled. If not empty,
      // unix socket support is enabled with the given default path (treated relative
      // to the datadir)
      string default_unix_socket_path;
      //If non 0, HTTP will be enabled by default on the given port number. If
      // 0, HTTP will not be enabled by default
      uint16_t default_http_port{0};
      //If set, a Server header will be added to the HTTP reply with this value
      string server_header;
      bool   support_categories = true;
   };

   /**
    *  This plugin starts an HTTP server and dispatches queries to
    *  registered handles based upon URL. The handler is passed the
    *  URL that was requested and a callback method that should be
    *  called with the response code and body.
    *
    *  The handler will be called from the appbase application io_service
    *  thread.  The callback can be called from any thread and will
    *  automatically propagate the call to the http thread.
    *
    *  The HTTP service will run in its own thread with its own io_service to
    *  make sure that HTTP request processing does not interfer with other
    *  plugins.
    */
   class http_plugin : public appbase::plugin<http_plugin>
   {
      public:
        http_plugin();
        ~http_plugin() override;

        //must be called before initialize
        static void set_defaults(const http_plugin_defaults& config);
        static std::string get_server_header();

        APPBASE_PLUGIN_REQUIRES()
        void set_program_options(options_description&, options_description& cfg) override;

        void plugin_initialize(const variables_map& options);
        void plugin_startup();
        void plugin_shutdown();
        void handle_sighup() override;

        void add_handler(api_entry&& entry, appbase::exec_queue q, int priority = appbase::priority::medium_low, http_content_type content_type = http_content_type::json);
        void add_api(api_description&& api, appbase::exec_queue q, int priority = appbase::priority::medium_low, http_content_type content_type = http_content_type::json) {
           for (auto& call : api)
              add_handler(std::move(call), q, priority, content_type);
        }

        void add_async_handler(api_entry&& entry, http_content_type content_type = http_content_type::json);
        void add_async_api(api_description&& api, http_content_type content_type = http_content_type::json) {
           for (auto& call : api)
              add_async_handler(std::move(call), content_type);
        }

        // standard exception handling for api handlers
        static void handle_exception( const char *api_name, const char *call_name, const string& body, const url_response_callback& cb );

        void post_http_thread_pool(std::function<void()> f);

        bool is_on_loopback(api_category category) const;

        static bool verbose_errors();

        struct get_supported_apis_result {
           vector<string> apis;
        };

        /// @return the configured http-max-response-time-ms
        fc::microseconds get_max_response_time()const;

        size_t get_max_body_size()const;

        struct metrics {
           std::string target;
        };

        void register_update_metrics(std::function<void(metrics)>&& fun);

        std::atomic<bool>& listening();
   private:
        std::shared_ptr<class http_plugin_impl> my;
   };

   /**
    * @brief Structure used to create JSON error responses
    */
   struct error_results {
      uint16_t code{};
      string message;

      struct error_info {
         int64_t code{};
         string name;
         string what;

         struct error_detail {
            string message;
            string file;
            uint64_t line_number{};
            string method;
         };

         vector<error_detail> details;

         static const uint8_t details_limit = 10;

         error_info() = default;

         error_info(const fc::exception& exc, bool include_full_log) {
            code = exc.code();
            name = exc.name();
            what = exc.what();
            uint8_t limit = include_full_log ? details_limit : 1;
            for( auto itr = exc.get_log().begin(); itr != exc.get_log().end(); ++itr ) {
               // Prevent sending trace that are too big
               if( details.size() >= limit ) break;
               // Append error
               error_detail detail = {
                     include_full_log ? itr->get_message() : itr->get_limited_message(),
                     itr->get_context().get_file(),
                     itr->get_context().get_line_number(),
                     itr->get_context().get_method()
               };
               details.emplace_back( detail );
            }
         }
      };

      error_info error;
   };

   /**
    * @brief Used to trim whitespace from body.
    * Returned string_view valid only for lifetime of body
    */
   inline std::string_view make_trimmed_string_view(const std::string& body) {
      if (body.empty()) {
         return {};
      }
      size_t left = 0;
      size_t right = body.size() - 1;

      while(left < right)
      {
         if (body[left] == ' ') {
            ++left;
         } else {
            break;
         }
      }
      while(left < right)
      {
         if (body[right] == ' ') {
            --right;
         } else {
            break;
         }
      }
      if ((left == right) && (body[left] == ' ')) {
         return {};
      }
      return std::string_view(body).substr(left, right-left+1);
   }

   inline bool is_empty_content(const std::string& body) {
      const auto trimmed_body_view = make_trimmed_string_view(body);
      if (trimmed_body_view.empty()) {
         return true;
      }
      const size_t body_size = trimmed_body_view.size();
      if ((body_size > 1) && (trimmed_body_view.at(0) == '{') && (trimmed_body_view.at(body_size - 1) == '}')) {
         for(size_t idx=1; idx<body_size-1; ++idx)
         {
            if ((trimmed_body_view.at(idx) != ' ') && (trimmed_body_view.at(idx) != '\t'))
            {
               return false;
            }
         }
      } else {
         return false;
      }
      return true;
   }

   enum class http_params_types {
      no_params = 0,
      params_required = 1,
      possible_no_params = 2
   };

   template<typename T, http_params_types params_type>
   T parse_params(const std::string& body) {
      if constexpr (params_type == http_params_types::params_required) {
         if (is_empty_content(body)) {
            SYS_THROW(chain::invalid_http_request, "A Request body is required");
         }
      }

      try {
         try {
            if constexpr (params_type == http_params_types::no_params || params_type == http_params_types::possible_no_params) {
               if (is_empty_content(body)) {
                  if constexpr (std::is_same_v<T, std::string>) {
                     return std::string("{}");
                  }
                  return {};
               }
               if constexpr (params_type == http_params_types::no_params) {
                  SYS_THROW(chain::invalid_http_request, "no parameter should be given");
               }
            }
            return fc::json::from_string(body).as<T>();
         } catch (const chain::chain_exception& e) { // SYS_RETHROW_EXCEPTIONS does not re-type these so, re-code it
            throw fc::exception(e);
         }
      } SYS_RETHROW_EXCEPTIONS(chain::invalid_http_request, "Unable to parse valid input from POST body");
   }
}

FC_REFLECT(sysio::error_results::error_info::error_detail, (message)(file)(line_number)(method))
FC_REFLECT(sysio::error_results::error_info, (code)(name)(what)(details))
FC_REFLECT(sysio::error_results, (code)(message)(error))
FC_REFLECT(sysio::http_plugin::get_supported_apis_result, (apis))
