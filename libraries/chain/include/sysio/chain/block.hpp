#pragma once
#include <sysio/chain/block_header.hpp>
#include <sysio/chain/transaction.hpp>

namespace sysio { namespace chain {

   /**
    * When a transaction is referenced by a block it could imply one of several outcomes which
    * describe the state-transition undertaken by the block producer.
    */

   struct transaction_receipt_header {
      enum status_enum {
         executed  = 0, ///< succeed, no error handler executed
         soft_fail = 1, ///< objectively failed (not executed), error handler executed
         hard_fail = 2, ///< objectively failed and error handler objectively failed thus no state change
         delayed   = 3, ///< transaction delayed/deferred/scheduled for future execution
         expired   = 4  ///< transaction expired and storage space refuned to user
      };

      transaction_receipt_header():status(hard_fail){}
      explicit transaction_receipt_header( status_enum s ):status(s){}

      friend inline bool operator ==( const transaction_receipt_header& lhs, const transaction_receipt_header& rhs ) {
         return std::tie(lhs.status, lhs.cpu_usage_us, lhs.net_usage_words) == std::tie(rhs.status, rhs.cpu_usage_us, rhs.net_usage_words);
      }

      fc::enum_type<uint8_t,status_enum>   status;
      uint32_t                             cpu_usage_us = 0; ///< total billed CPU usage (microseconds)
      fc::unsigned_int                     net_usage_words; ///<  total billed NET usage, so we can reconstruct resource state when skipping context free data... hard failures...
   };

   struct transaction_receipt : public transaction_receipt_header {

      transaction_receipt():transaction_receipt_header(){}
      explicit transaction_receipt( const transaction_id_type& tid ):transaction_receipt_header(executed),trx(tid){}
      explicit transaction_receipt( const packed_transaction& ptrx ):transaction_receipt_header(executed),trx(std::in_place_type<packed_transaction>, ptrx){}

      std::variant<transaction_id_type, packed_transaction> trx;
   private:
      enum trx_digest_type { uncompressed_digest, compressed_digest};
      digest_type get_digest(trx_digest_type packing)const {
         digest_type::encoder enc;
         fc::raw::pack( enc, status );
         fc::raw::pack( enc, cpu_usage_us );
         fc::raw::pack( enc, net_usage_words );
         if( std::holds_alternative<transaction_id_type>(trx) )
            fc::raw::pack( enc, std::get<transaction_id_type>(trx) );
         else {
            if (packing == trx_digest_type::compressed_digest)
               fc::raw::pack( enc, std::get<packed_transaction>(trx).packed_digest() );
            else
               fc::raw::pack( enc, std::get<packed_transaction>(trx).digest() );
         }
         return enc.result();
      }
   public:
      digest_type digest()const {
         return get_digest(trx_digest_type::uncompressed_digest);
      }

      digest_type packed_digest()const {
         return get_digest(trx_digest_type::compressed_digest);
      }
   };

   struct additional_block_signatures_extension : fc::reflect_init {
      static constexpr uint16_t extension_id() { return 2; }
      static constexpr bool     enforce_unique() { return true; }

      additional_block_signatures_extension() = default;

      additional_block_signatures_extension( const vector<signature_type>& signatures )
      :signatures( signatures )
      {}

      additional_block_signatures_extension( vector<signature_type>&& signatures )
      :signatures( std::move(signatures) )
      {}

      void reflector_init();

      vector<signature_type> signatures;
   };

   namespace detail {
      template<typename... Ts>
      struct block_extension_types {
         using block_extension_t = std::variant< Ts... >;
         using decompose_t = decompose< Ts... >;
      };
   }

   using block_extension_types = detail::block_extension_types<
         additional_block_signatures_extension
   >;

   using block_extension = block_extension_types::block_extension_t;

   /**
    */
   struct signed_block : public signed_block_header{
   private:
      signed_block( const signed_block& ) = default;
   public:
      signed_block() = default;
      explicit signed_block( const signed_block_header& h ):signed_block_header(h){}
      signed_block( signed_block&& ) = default;
      signed_block& operator=(const signed_block&) = delete;
      signed_block& operator=(signed_block&&) = default;
      signed_block clone() const { return *this; }

      deque<transaction_receipt>   transactions; /// new or generated transactions
      extensions_type               block_extensions;

      flat_multimap<uint16_t, block_extension> validate_and_extract_extensions()const;
   };
   using signed_block_ptr = std::shared_ptr<signed_block>;

   struct producer_confirmation {
      block_id_type   block_id;
      digest_type     block_digest;
      account_name    producer;
      signature_type  sig;
   };

} } /// sysio::chain

FC_REFLECT_ENUM( sysio::chain::transaction_receipt::status_enum,
                 (executed)(soft_fail)(hard_fail)(delayed)(expired) )

FC_REFLECT(sysio::chain::transaction_receipt_header, (status)(cpu_usage_us)(net_usage_words) )
FC_REFLECT_DERIVED(sysio::chain::transaction_receipt, (sysio::chain::transaction_receipt_header), (trx) )
FC_REFLECT(sysio::chain::additional_block_signatures_extension, (signatures));
FC_REFLECT_DERIVED(sysio::chain::signed_block, (sysio::chain::signed_block_header), (transactions)(block_extensions) )
