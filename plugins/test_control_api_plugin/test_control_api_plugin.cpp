#include <sysio/test_control_api_plugin/test_control_api_plugin.hpp>
#include <sysio/chain/exceptions.hpp>

#include <fc/io/json.hpp>

namespace sysio {

   static auto _test_control_api_plugin = application::register_plugin<test_control_api_plugin>();

using namespace sysio;

class test_control_api_plugin_impl {
public:
   explicit test_control_api_plugin_impl(controller& db)
      : db(db) {}

   controller& db;
};


test_control_api_plugin::test_control_api_plugin() = default;
test_control_api_plugin::~test_control_api_plugin() = default;

void test_control_api_plugin::set_program_options(options_description&, options_description&) {}
void test_control_api_plugin::plugin_initialize(const variables_map&) {}

#define CALL_WITH_API_400(api_name, api_handle, api_namespace, call_name, http_response_code, params_type) \
{std::string("/v1/" #api_name "/" #call_name), \
   api_category::test_control, \
   [api_handle](string&&, string&& body, url_response_callback&& cb) mutable { \
          try { \
             auto params = parse_params<api_namespace::call_name ## _params, params_type>(body);\
             fc::variant result( api_handle.call_name( std::move(params) ) ); \
             cb(http_response_code, std::move(result)); \
          } catch (...) { \
             http_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define TEST_CONTROL_RW_CALL(call_name, http_response_code, params_type) CALL_WITH_API_400(test_control, rw_api, test_control_apis::read_write, call_name, http_response_code, params_type)

void test_control_api_plugin::plugin_startup() {
   my.reset(new test_control_api_plugin_impl(app().get_plugin<chain_plugin>().chain()));
   auto rw_api = app().get_plugin<test_control_plugin>().get_read_write_api();

   app().get_plugin<http_plugin>().add_api({
      TEST_CONTROL_RW_CALL(kill_node_on_producer, 202, http_params_types::params_required)
   }, appbase::exec_queue::read_write);
}

void test_control_api_plugin::plugin_shutdown() {}

}
