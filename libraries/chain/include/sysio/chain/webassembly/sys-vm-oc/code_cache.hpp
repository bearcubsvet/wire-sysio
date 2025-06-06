#pragma once

#include <boost/lockfree/spsc_queue.hpp>

#include <sysio/chain/webassembly/sys-vm-oc/sys-vm-oc.hpp>
#include <sysio/chain/webassembly/sys-vm-oc/ipc_helpers.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/key_extractors.hpp>

#include <boost/interprocess/mem_algo/rbtree_best_fit.hpp>
#include <boost/asio/local/datagram_protocol.hpp>


#include <thread>

namespace std {
    template<> struct hash<sysio::chain::sysvmoc::code_tuple> {
        size_t operator()(const sysio::chain::sysvmoc::code_tuple& ct) const noexcept {
            return ct.code_id._hash[0];
        }
    };
}

namespace sysio { namespace chain { namespace sysvmoc {

using namespace boost::multi_index;
using namespace boost::asio;

namespace bip = boost::interprocess;

using allocator_t = bip::rbtree_best_fit<bip::null_mutex_family, bip::offset_ptr<void>, alignof(std::max_align_t)>;

struct config;


class code_cache_base {
   public:
      code_cache_base(const std::filesystem::path& data_dir, const sysvmoc::config& sysvmoc_config, const chainbase::database& db);
      ~code_cache_base();

      const int& fd() const { return _cache_fd; }

      void free_code(const digest_type& code_id, const uint8_t& vm_version);

      // get_descriptor_for_code failure reasons
      enum class get_cd_failure {
         temporary, // oc compile not done yet, users like read-only trxs can retry
         permanent  // oc will not start, users should not retry
      };

   protected:
      struct by_hash;

      typedef boost::multi_index_container<
         code_descriptor,
         indexed_by<
            sequenced<>,
            hashed_unique<tag<by_hash>,
               composite_key< code_descriptor,
                  member<code_descriptor, digest_type, &code_descriptor::code_hash>,
                  member<code_descriptor, uint8_t,     &code_descriptor::vm_version>
               >
            >
         >
      > code_cache_index;
      code_cache_index _cache_index;

      const chainbase::database& _db;
      sysvmoc::config            _sysvmoc_config;

      std::filesystem::path _cache_file_path;
      int                   _cache_fd;

      io_context _ctx;
      local::datagram_protocol::socket _compile_monitor_write_socket{_ctx};
      local::datagram_protocol::socket _compile_monitor_read_socket{_ctx};

      //these are really only useful to the async code cache, but keep them here so free_code can be shared
      using queued_compilies_t = boost::multi_index_container<
         code_tuple,
         indexed_by<
            sequenced<>,
            hashed_unique<tag<by_hash>,
               composite_key< code_tuple,
                  member<code_tuple, digest_type, &code_tuple::code_id>,
                  member<code_tuple, uint8_t,     &code_tuple::vm_version>
               >
            >
         >
      >;
      queued_compilies_t _queued_compiles;
      std::unordered_map<code_tuple, bool> _outstanding_compiles_and_poison;

      size_t _free_bytes_eviction_threshold;
      void check_eviction_threshold(size_t free_bytes);
      void run_eviction_round();

      void set_on_disk_region_dirty(bool);

      template <typename T>
      void serialize_cache_index(fc::datastream<T>& ds);
};

class code_cache_async : public code_cache_base {
   public:
      code_cache_async(const std::filesystem::path& data_dir, const sysvmoc::config& sysvmoc_config, const chainbase::database& db);
      ~code_cache_async();

      //If code is in cache: returns pointer & bumps to front of MRU list
      //If code is not in cache, and not blacklisted, and not currently compiling: return nullptr and kick off compile
      //otherwise: return nullptr
      const code_descriptor* const get_descriptor_for_code(bool high_priority, const digest_type& code_id, const uint8_t& vm_version, bool is_write_window, get_cd_failure& failure);

   private:
      std::thread _monitor_reply_thread;
      boost::lockfree::spsc_queue<wasm_compilation_result_message> _result_queue;
      void wait_on_compile_monitor_message();
      std::tuple<size_t, size_t> consume_compile_thread_queue();
      std::unordered_set<code_tuple> _blacklist;
      size_t _threads;
};

class code_cache_sync : public code_cache_base {
   public:
      using code_cache_base::code_cache_base;
      ~code_cache_sync();

      //Can still fail and return nullptr if, for example, there is an expected instantiation failure
      const code_descriptor* const get_descriptor_for_code_sync(const digest_type& code_id, const uint8_t& vm_version, bool is_write_window);
};

}}}
