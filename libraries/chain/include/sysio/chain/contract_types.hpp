#pragma once

#include <sysio/chain/authority.hpp>
#include <sysio/chain/types.hpp>
#include <sysio/chain/asset.hpp> // Included for reducepolicy

namespace sysio { namespace chain {

using action_name    = sysio::chain::action_name;

struct newaccount {
   account_name                     creator;
   account_name                     name;
   authority                        owner;
   authority                        active;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "newaccount"_n;
   }
};

struct setcode {
   account_name                     account;
   uint8_t                          vmtype = 0;
   uint8_t                          vmversion = 0;
   bytes                            code;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "setcode"_n;
   }
};

struct setabi {
   account_name                     account;
   bytes                            abi;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "setabi"_n;
   }
};


struct updateauth {
   account_name                      account;
   permission_name                   permission;
   permission_name                   parent;
   authority                         auth;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "updateauth"_n;
   }
};

struct deleteauth {
   deleteauth() = default;
   deleteauth(const account_name& account, const permission_name& permission)
   :account(account), permission(permission)
   {}

   account_name                      account;
   permission_name                   permission;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "deleteauth"_n;
   }
};

struct linkauth {
   linkauth() = default;
   linkauth(const account_name& account, const account_name& code, const action_name& type, const permission_name& requirement)
   :account(account), code(code), type(type), requirement(requirement)
   {}

   account_name                      account;
   account_name                      code;
   action_name                       type;
   permission_name                   requirement;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "linkauth"_n;
   }
};

struct unlinkauth {
   unlinkauth() = default;
   unlinkauth(const account_name& account, const account_name& code, const action_name& type)
   :account(account), code(code), type(type)
   {}

   account_name                      account;
   account_name                      code;
   action_name                       type;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "unlinkauth"_n;
   }
};

struct canceldelay {
   permission_level      canceling_auth;
   transaction_id_type   trx_id;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "canceldelay"_n;
   }
};

struct onerror {
   uint128_t      sender_id;
   bytes          sent_trx;

   onerror( uint128_t sid, const char* data, size_t len )
   :sender_id(sid),sent_trx(data,data+len){}

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return "onerror"_n;
   }
};

// **Roa change** to facilitate native action
struct reducepolicy {
   account_name owner;
   account_name issuer;
   asset        net_weight;
   asset        cpu_weight;
   asset        ram_weight;
   uint8_t      network_gen;

   static account_name get_account() {
      return "sysio.roa"_n; // The account this contract is deployed to.
   }

   static action_name get_name() {
      return "reducepolicy"_n; // The action name.
   }
};

} } /// namespace sysio::chain

FC_REFLECT( sysio::chain::newaccount                       , (creator)(name)(owner)(active) )
FC_REFLECT( sysio::chain::setcode                          , (account)(vmtype)(vmversion)(code) )
FC_REFLECT( sysio::chain::setabi                           , (account)(abi) )
FC_REFLECT( sysio::chain::updateauth                       , (account)(permission)(parent)(auth) )
FC_REFLECT( sysio::chain::deleteauth                       , (account)(permission) )
FC_REFLECT( sysio::chain::linkauth                         , (account)(code)(type)(requirement) )
FC_REFLECT( sysio::chain::unlinkauth                       , (account)(code)(type) )
FC_REFLECT( sysio::chain::canceldelay                      , (canceling_auth)(trx_id) )
FC_REFLECT( sysio::chain::onerror                          , (sender_id)(sent_trx) )
// **Roa change**
FC_REFLECT( sysio::chain::reducepolicy                     , (owner)(issuer)(net_weight)(cpu_weight)(ram_weight)(network_gen))
