#pragma once

#include <sysio/http_plugin/http_plugin.hpp>
#include <sysio/chain_plugin/chain_plugin.hpp>

#include <sysio/chain/application.hpp>

namespace sysio {

using namespace appbase;

struct db_size_index_count {
   string   index;
   uint64_t row_count = 0;
};

struct db_size_stats {
   uint64_t                    free_bytes = 0;
   uint64_t                    used_bytes = 0;
   uint64_t                    size = 0;
   vector<db_size_index_count> indices;
};

class db_size_api_plugin : public plugin<db_size_api_plugin> {
public:
   APPBASE_PLUGIN_REQUIRES((http_plugin) (chain_plugin))

   db_size_api_plugin() = default;
   db_size_api_plugin(const db_size_api_plugin&) = delete;
   db_size_api_plugin(db_size_api_plugin&&) = delete;
   db_size_api_plugin& operator=(const db_size_api_plugin&) = delete;
   db_size_api_plugin& operator=(db_size_api_plugin&&) = delete;
   virtual ~db_size_api_plugin() override = default;

   virtual void set_program_options(options_description& cli, options_description& cfg) override {}
   void plugin_initialize(const variables_map& vm) {}
   void plugin_startup();
   void plugin_shutdown() {}

   db_size_stats get();

private:
};

}

FC_REFLECT( sysio::db_size_index_count, (index)(row_count) )
FC_REFLECT( sysio::db_size_stats, (free_bytes)(used_bytes)(size)(indices) )