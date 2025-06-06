#pragma once

#include <sysio/chain/block_header_state.hpp>
#include <sysio/chain/block.hpp>
#include <sysio/chain/transaction_metadata.hpp>
#include <sysio/chain/action_receipt.hpp>

namespace sysio { namespace chain {

   struct block_state : public block_header_state {
      block_state( const block_header_state& prev,
                   signed_block_ptr b,
                   const protocol_feature_set& pfs,
                   const std::function<void( block_timestamp_type,
                                             const flat_set<digest_type>&,
                                             const vector<digest_type>& )>& validator,
                   bool skip_validate_signee
                 );

      block_state( pending_block_header_state&& cur,
                   signed_block_ptr&& b, // unsigned block
                   deque<transaction_metadata_ptr>&& trx_metas,
                   const protocol_feature_set& pfs,
                   const std::function<void( block_timestamp_type,
                                             const flat_set<digest_type>&,
                                             const vector<digest_type>& )>& validator,
                   const signer_callback_type& signer
                );

      block_state() = default;


      signed_block_ptr                                    block;

   private: // internal use only, not thread safe
      friend struct fc::reflector<block_state>;
      friend bool block_state_is_valid( const block_state& ); // work-around for multi-index access
      friend struct controller_impl;
      friend class  fork_database;
      friend struct fork_database_impl;
      friend class  unapplied_transaction_queue;
      friend struct pending_state;

      bool is_valid()const { return validated; }
      bool is_pub_keys_recovered()const { return _pub_keys_recovered; }

      deque<transaction_metadata_ptr> extract_trxs_metas() {
         _pub_keys_recovered = false;
         auto result = std::move( _cached_trxs );
         _cached_trxs.clear();
         return result;
      }
      void set_trxs_metas( deque<transaction_metadata_ptr>&& trxs_metas, bool keys_recovered ) {
         _pub_keys_recovered = keys_recovered;
         _cached_trxs = std::move( trxs_metas );
      }
      const deque<transaction_metadata_ptr>& trxs_metas()const { return _cached_trxs; }

      bool                                                validated = false;

      bool                                                _pub_keys_recovered = false;
      /// this data is redundant with the data stored in block, but facilitates
      /// recapturing transactions when we pop a block
      deque<transaction_metadata_ptr>                    _cached_trxs;
   };

   using block_state_ptr = std::shared_ptr<block_state>;
   using branch_type = deque<block_state_ptr>;

} } /// namespace sysio::chain

FC_REFLECT_DERIVED( sysio::chain::block_state, (sysio::chain::block_header_state), (block)(validated) )
