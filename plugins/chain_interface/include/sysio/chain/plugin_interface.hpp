#pragma once

#include <appbase/channel.hpp>
#include <appbase/method.hpp>

#include <sysio/chain/block.hpp>
#include <sysio/chain/block_state.hpp>
#include <sysio/chain/transaction_metadata.hpp>
#include <sysio/chain/trace.hpp>

namespace sysio::chain::plugin_interface {
   using namespace sysio::chain;
   using namespace appbase;
   struct chain_plugin_interface;

   namespace channels {
      using pre_accepted_block     = channel_decl<struct pre_accepted_block_tag,    signed_block_ptr>;
      using rejected_block         = channel_decl<struct rejected_block_tag,        signed_block_ptr>;
      using accepted_block_header  = channel_decl<struct accepted_block_header_tag, block_state_ptr>;
      using accepted_block         = channel_decl<struct accepted_block_tag,        block_state_ptr>;
      using irreversible_block     = channel_decl<struct irreversible_block_tag,    block_state_ptr>;
      using accepted_transaction   = channel_decl<struct accepted_transaction_tag,  transaction_metadata_ptr>;
      using applied_transaction    = channel_decl<struct applied_transaction_tag,   transaction_trace_ptr>;
   }

   namespace methods {
      using get_block_by_number    = method_decl<chain_plugin_interface, signed_block_ptr(uint32_t block_num)>;
      using get_block_by_id        = method_decl<chain_plugin_interface, signed_block_ptr(const block_id_type& block_id)>;
      using get_head_block_id      = method_decl<chain_plugin_interface, block_id_type ()>;
      using get_lib_block_id       = method_decl<chain_plugin_interface, block_id_type ()>;

      using get_last_irreversible_block_number = method_decl<chain_plugin_interface, uint32_t ()>;
   }

   namespace incoming {
      namespace methods {
         // synchronously push a block/trx to a single provider, block_state_ptr may be null
         using block_sync            = method_decl<chain_plugin_interface, bool(const signed_block_ptr&, const std::optional<block_id_type>&, const block_state_ptr&), first_provider_policy>;
         using transaction_async     = method_decl<chain_plugin_interface, void(const packed_transaction_ptr&, bool, transaction_metadata::trx_type, bool, next_function<transaction_trace_ptr>), first_provider_policy>;
      }
   }

   namespace compat {
      namespace channels {
         using transaction_ack       = channel_decl<struct accepted_transaction_tag, std::pair<fc::exception_ptr, packed_transaction_ptr>>;
      }
   }

} // namespace sysio::chain::plugin_interface
