#include <sysio/chain/asset.hpp>
#include <sysio/chain/authority.hpp>
#include <sysio/chain/authority_checker.hpp>
#include <sysio/chain/types.hpp>
#include <sysio/chain/thread_utils.hpp>
#include <sysio/testing/tester.hpp>

#include <fc/io/json.hpp>
#include <fc/log/logger_config.hpp>
#include <appbase/application.hpp>
#include <fc/bitutil.hpp>

#include <thread>

#include <boost/test/unit_test.hpp>

using namespace sysio::chain;
using namespace sysio::testing;

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

struct base_reflect : fc::reflect_init {
   int bv = 0;
   bool base_reflect_initialized = false;
   int base_reflect_called = 0;
protected:
   friend struct fc::reflector<base_reflect>;
   friend struct fc::reflector_init_visitor<base_reflect>;
   friend struct fc::has_reflector_init<base_reflect>;
   void reflector_init() {
      BOOST_CHECK_EQUAL( bv, 42 ); // should be deserialized before called, set by test
      ++base_reflect_called;
      base_reflect_initialized = true;
   }
};

struct derived_reflect : public base_reflect {
   int dv = 0;
   bool derived_reflect_initialized = false;
   int derived_reflect_called = 0;
protected:
   friend struct fc::reflector<derived_reflect>;
   friend struct fc::reflector_init_visitor<derived_reflect>;
   friend struct fc::has_reflector_init<derived_reflect>;
   void reflector_init() {
      BOOST_CHECK_EQUAL( bv, 42 ); // should be deserialized before called, set by test
      BOOST_CHECK_EQUAL( dv, 52 ); // should be deserialized before called, set by test
      ++derived_reflect_called;
      base_reflect::reflector_init();
      derived_reflect_initialized = true;
   }
};

struct final_reflect : public derived_reflect {
   int fv = 0;
   bool final_reflect_initialized = false;
   int final_reflect_called = 0;
private:
   friend struct fc::reflector<final_reflect>;
   friend struct fc::reflector_init_visitor<final_reflect>;
   friend struct fc::has_reflector_init<final_reflect>;
   void reflector_init() {
      BOOST_CHECK_EQUAL( bv, 42 ); // should be deserialized before called, set by test
      BOOST_CHECK_EQUAL( dv, 52 ); // should be deserialized before called, set by test
      BOOST_CHECK_EQUAL( fv, 62 ); // should be deserialized before called, set by test
      ++final_reflect_called;
      derived_reflect::reflector_init();
      final_reflect_initialized = true;
   }
};

FC_REFLECT( base_reflect, (bv) )
FC_REFLECT_DERIVED( derived_reflect, (base_reflect), (dv) )
FC_REFLECT_DERIVED( final_reflect, (derived_reflect), (fv) )

namespace sysio
{
using namespace chain;
using namespace std;

static constexpr uint64_t name_suffix( name nv ) {
   uint64_t n = nv.to_uint64_t();
   uint32_t remaining_bits_after_last_actual_dot = 0;
   uint32_t tmp = 0;
   for( int32_t remaining_bits = 59; remaining_bits >= 4; remaining_bits -= 5 ) { // Note: remaining_bits must remain signed integer
      // Get characters one-by-one in name in order from left to right (not including the 13th character)
      auto c = (n >> remaining_bits) & 0x1Full;
      if( !c ) { // if this character is a dot
         tmp = static_cast<uint32_t>(remaining_bits);
      } else { // if this character is not a dot
         remaining_bits_after_last_actual_dot = tmp;
      }
   }

   uint64_t thirteenth_character = n & 0x0Full;
   if( thirteenth_character ) { // if 13th character is not a dot
      remaining_bits_after_last_actual_dot = tmp;
   }

   if( remaining_bits_after_last_actual_dot == 0 ) // there is no actual dot in the name other than potentially leading dots
      return n;

   // At this point remaining_bits_after_last_actual_dot has to be within the range of 4 to 59 (and restricted to increments of 5).

   // Mask for remaining bits corresponding to characters after last actual dot, except for 4 least significant bits (corresponds to 13th character).
   uint64_t mask = (1ull << remaining_bits_after_last_actual_dot) - 16;
   uint32_t shift = 64 - remaining_bits_after_last_actual_dot;

   return ( ((n & mask) << shift) + (thirteenth_character << (shift-1)) );
}

BOOST_AUTO_TEST_SUITE(misc_tests)

BOOST_AUTO_TEST_CASE(reverse_endian_tests)
{
    BOOST_CHECK_EQUAL( endian_reverse_u64(0x0123456789abcdef), 0xefcdab8967452301u );
    BOOST_CHECK_EQUAL( endian_reverse_u64(0x0102030405060708), 0x0807060504030201u );
    BOOST_CHECK_EQUAL( endian_reverse_u32(0x01234567), 0x67452301u );
    BOOST_CHECK_EQUAL( endian_reverse_u32(0x01020304), 0x04030201u );
}

BOOST_AUTO_TEST_CASE(name_suffix_tests)
{
   BOOST_CHECK_EQUAL( name{name_suffix(name(0))}, name{0} );
   BOOST_CHECK_EQUAL( name{name_suffix("abcdehijklmn"_n)}, name{"abcdehijklmn"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("abcdehijklmn1"_n)}, name{"abcdehijklmn1"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("abc.def"_n)}, name{"def"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix(".abc.def"_n)}, name{"def"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("..abc.def"_n)}, name{"def"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("abc..def"_n)}, name{"def"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("abc.def.ghi"_n)}, name{"ghi"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix(".abcdefghij"_n)}, name{"abcdefghij"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix(".abcdefghij.1"_n)}, name{"1"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("a.bcdefghij"_n)}, name{"bcdefghij"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("a.bcdefghij.1"_n)}, name{"1"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("......a.b.c"_n)}, name{"c"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("abcdefhi.123"_n)}, name{"123"_n} );
   BOOST_CHECK_EQUAL( name{name_suffix("abcdefhij.123"_n)}, name{"123"_n} );
}

BOOST_AUTO_TEST_CASE(name_prefix_tests)
{
   BOOST_CHECK_EQUAL("e"_n.prefix(), "e"_n);
   BOOST_CHECK_EQUAL(""_n.prefix(), ""_n);
   BOOST_CHECK_EQUAL("abcdefghijklm"_n.prefix(), "abcdefghijklm"_n);
   BOOST_CHECK_EQUAL("abcdefghijkl"_n.prefix(), "abcdefghijkl"_n);
   BOOST_CHECK_EQUAL("abc.xyz"_n.prefix(), "abc"_n);
   BOOST_CHECK_EQUAL("abc.xyz.qrt"_n.prefix(), "abc.xyz"_n);
   BOOST_CHECK_EQUAL("."_n.prefix(), ""_n);

   BOOST_CHECK_EQUAL("sysio.any"_n.prefix(), "sysio"_n);
   BOOST_CHECK_EQUAL("sysio"_n.prefix(), "sysio"_n);
   BOOST_CHECK_EQUAL("sysio"_n.prefix(), config::system_account_name);
   BOOST_CHECK_EQUAL("sysio."_n.prefix(), "sysio"_n);
   BOOST_CHECK_EQUAL("sysio.evm"_n.prefix(), "sysio"_n);
   BOOST_CHECK_EQUAL(".sysio"_n.prefix(), ""_n);
   BOOST_CHECK_NE("sysi"_n.prefix(), "sysio"_n);
   BOOST_CHECK_NE("sysiosysio"_n.prefix(), "sysio"_n);
   BOOST_CHECK_NE("sysioe"_n.prefix(), "sysio"_n);
}

/// Test processing of unbalanced strings
BOOST_AUTO_TEST_CASE(json_from_string_test)
{
  bool exc_found = false;
  try {
    auto val = fc::json::from_string("{\"}");
  } catch(...) {
    exc_found = true;
  }
  BOOST_CHECK_EQUAL(exc_found, true);

  exc_found = false;
  try {
    auto val = fc::json::from_string("{\"block_num_or_id\":5");
  } catch(...) {
    exc_found = true;
  }
  BOOST_CHECK_EQUAL(exc_found, true);
}

BOOST_AUTO_TEST_CASE(variant_format_string_limited)
{
   const string format = "${a} ${b} ${c}";
   {
      fc::mutable_variant_object mu;
      mu( "a", string( 1024, 'a' ) );
      mu( "b", string( 1024, 'b' ) );
      mu( "c", string( 1024, 'c' ) );
      string result = fc::format_string( format, mu, true );
      BOOST_CHECK_LT(0u, mu.size());
      BOOST_CHECK_LT(format.size(), 1024u);
      const size_t target_size = (1024u -  format.size()) / mu.size();
      BOOST_CHECK_EQUAL( result, string( target_size, 'a' ) + "... " + string( target_size, 'b' ) + "... " + string( target_size, 'c' ) + "..." );
   }
   {
      fc::mutable_variant_object mu;
      signed_block a;
      blob b;
      for( int i = 0; i < 1024; ++i)
         b.data.push_back('b');
      variants c;
      c.push_back(fc::variant(a));
      mu( "a", a );
      mu( "b", b );
      mu( "c", c );
      string result = fc::format_string( format, mu, true );
      BOOST_CHECK_EQUAL( result, "${a} ${b} ${c}");
   }
   {
      // test cases for issue #8741, short version, all fields being displayed
      flat_set <permission_level> provided_permissions;
      for(char ch = 'a'; ch < 'c'; ++ch) {
         provided_permissions.insert( {name(std::string_view(string(4, ch))), name(std::string_view(string(4, ch + 1)))});
      }
      flat_set <public_key_type> provided_keys;
      auto fill_keys = [](const flat_set <permission_level>& provided_permissions, flat_set <public_key_type>& provided_keys) {
         std::string digest = "1234567";
         for (auto &permission : provided_permissions) {
            digest += "1";
            const std::string key_name_str = permission.actor.to_string() + permission.permission.to_string();
            auto sig_digest = digest_type::hash(std::make_pair("1234", "abcd"));
            const fc::crypto::signature sig = private_key_type::regenerate<fc::ecc::private_key_shim>(
                  fc::sha256::hash(key_name_str + "active")).sign(sig_digest);
            provided_keys.insert(public_key_type{sig, fc::sha256{digest}, true});
         }
      };
      fill_keys(provided_permissions, provided_keys);
      const string format = "transaction declares authority '${auth}', provided permissions ${provided_permissions}, provided keys ${provided_keys}";
      fc::mutable_variant_object mu;
      mu("auth", *provided_permissions.begin());
      mu("provided_permissions", provided_permissions);
      mu("provided_keys", provided_keys);
      BOOST_CHECK_LT(0u, mu.size());
      const auto arg_limit_size = (1024u - format.size()) / mu.size();
      const string result = fc::format_string(format, mu, true);
      BOOST_CHECK(provided_permissions.begin() != provided_permissions.end());
      const string auth_str = fc::json::to_string(*provided_permissions.begin(), fc::time_point::maximum());
      string target_str = "transaction declares authority '" + fc::json::to_string(*provided_permissions.begin(), fc::time_point::maximum());
      target_str += "', provided permissions " + fc::json::to_string(provided_permissions, fc::time_point::maximum());
      target_str += ", provided keys " + fc::json::to_string(provided_keys, fc::time_point::maximum()).substr(0, arg_limit_size);
      BOOST_CHECK_EQUAL(result, target_str);
      BOOST_CHECK_LT(result.size(), 1024u + 3 * mu.size());

      // test cases for issue #8741, longer version, permission and keys field being folded
      provided_permissions.clear();
      provided_keys.clear();
      for(char ch = 'c'; ch < 'z'; ++ch) {
         provided_permissions.insert( {name(std::string_view(string(5, ch))), name(std::string_view(string(5, ch + 1)))});
      }
      fill_keys(provided_permissions, provided_keys);
      fc::mutable_variant_object mu_fold;
      mu_fold("auth", *provided_permissions.begin());
      mu_fold("provided_permissions", provided_permissions);
      mu_fold("provided_keys", provided_keys);
      BOOST_CHECK_LT(0u, mu_fold.size());
      string target_fold_str = "transaction declares authority '" + fc::json::to_string(*provided_permissions.begin(), fc::time_point::maximum());
      target_fold_str += "', provided permissions ${provided_permissions}";
      target_fold_str += ", provided keys ${provided_keys}";
      const string result_fold = fc::format_string(format, mu_fold, true);
      BOOST_CHECK_EQUAL(result_fold, target_fold_str);
      BOOST_CHECK_LT(result_fold.size(), 1024u + 3 * mu.size());
   }
}

// Test overflow handling in asset::from_string
BOOST_AUTO_TEST_CASE(asset_from_string_overflow)
{
   asset a;

   // precision = 19, magnitude < 2^61
   BOOST_CHECK_EXCEPTION( asset::from_string("0.1000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-0.1000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("1.0000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-1.0000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });

   // precision = 18, magnitude < 2^58
   a = asset::from_string("0.100000000000000000 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), 100000000000000000L);
   a = asset::from_string("-0.100000000000000000 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), -100000000000000000L);

   // precision = 18, magnitude = 2^62
   BOOST_CHECK_EXCEPTION( asset::from_string("4.611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-4.611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("4611686018427387.904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-4611686018427387.904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });

   // precision = 18, magnitude = 2^62-1
   a = asset::from_string("4.611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), 4611686018427387903L);
   a = asset::from_string("-4.611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), -4611686018427387903L);

   // precision = 0, magnitude = 2^62
   BOOST_CHECK_EXCEPTION( asset::from_string("4611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-4611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });

   // precision = 0, magnitude = 2^62-1
   a = asset::from_string("4611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), 4611686018427387903L);
   a = asset::from_string("-4611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), -4611686018427387903L);

   // precision = 18, magnitude = 2^65
   BOOST_CHECK_EXCEPTION( asset::from_string("36.893488147419103232 CUR") , overflow_exception, [](const overflow_exception& e) {
      return true;
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-36.893488147419103232 CUR") , underflow_exception, [](const underflow_exception& e) {
      return true;
   });

   // precision = 14, magnitude > 2^76
   BOOST_CHECK_EXCEPTION( asset::from_string("1000000000.00000000000000 CUR") , overflow_exception, [](const overflow_exception& e) {
      return true;
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-1000000000.00000000000000 CUR") , underflow_exception, [](const underflow_exception& e) {
      return true;
   });

   // precision = 0, magnitude > 2^76
   BOOST_CHECK_EXCEPTION( asset::from_string("100000000000000000000000 CUR") , parse_error_exception, [](const parse_error_exception& e) {
      return expect_assert_message(e, "Couldn't parse int64_t");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-100000000000000000000000 CUR") , parse_error_exception, [](const parse_error_exception& e) {
      return expect_assert_message(e, "Couldn't parse int64_t");
   });

   // precision = 20, magnitude > 2^142
   BOOST_CHECK_EXCEPTION( asset::from_string("100000000000000000000000.00000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 20 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-100000000000000000000000.00000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 20 should be <= 18");
   });
}

struct permission_visitor {
   std::vector<permission_level> permissions;
   std::vector<size_t> size_stack;
   bool _log;

   permission_visitor(bool log = false) : _log(log) {}

   void operator()(const permission_level& permission) {
      permissions.push_back(permission);
   }

   void operator()(const permission_level& permission, bool repeat ) {}

   void push_undo() {
      if( _log )
         ilog("push_undo called");
      size_stack.push_back(permissions.size());
   }

   void pop_undo() {
      if( _log )
         ilog("pop_undo called");
      FC_ASSERT( size_stack.back() <= permissions.size() && size_stack.size() >= 1,
                 "invariant failure in test permission_visitor" );
      permissions.erase( permissions.begin() + size_stack.back(), permissions.end() );
      size_stack.pop_back();
   }

   void squash_undo() {
      if( _log )
         ilog("squash_undo called");
      FC_ASSERT( size_stack.size() >= 1, "invariant failure in test permission_visitor" );
      size_stack.pop_back();
   }

};

BOOST_AUTO_TEST_CASE(authority_checker)
{ try {
   testing::validating_tester test;
   auto a = test.get_public_key(name("a"), "active");
   auto b = test.get_public_key(name("b"), "active");
   auto c = test.get_public_key(name("c"), "active");

   auto GetNullAuthority = [](auto){abort(); return authority();};

   auto A = authority(2, {key_weight{a, 1}, key_weight{b, 1}});
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {a, b});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2u);
      BOOST_TEST(checker.unused_keys().size() == 0u);
   }
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {a, c});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 0u);
      BOOST_TEST(checker.unused_keys().size() == 2u);
   }
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2u);
      BOOST_TEST(checker.used_keys().count(a) == 1u);
      BOOST_TEST(checker.used_keys().count(b) == 1u);
      BOOST_TEST(checker.unused_keys().size() == 1u);
      BOOST_TEST(checker.unused_keys().count(c) == 1u);
   }
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {b, c});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 0u);
   }

   A = authority(3, {key_weight{a, 1}, key_weight{b, 1}, key_weight{c, 1}});
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {c, b, a}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {a, b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {a, c}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {b, c}).satisfied(A));

   A = authority(1, {key_weight{a, 1}, key_weight{b, 1}});
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {c}).satisfied(A));

   A = authority(1, {key_weight{a, 2}, key_weight{b, 1}});
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {c}).satisfied(A));

   auto GetCAuthority = [c](auto){
      return authority(1, {key_weight{c, 1}});
   };

   A = authority(2, {key_weight{a, 2}, key_weight{b, 1}}, {permission_level_weight{{name("hello"), name("world")}, 1}});
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {b});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(checker.used_keys().size() == 0u);
      BOOST_TEST(checker.unused_keys().size() == 1u);
      BOOST_TEST(checker.unused_keys().count(b) == 1u);
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {c});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(checker.used_keys().size() == 0u);
      BOOST_TEST(checker.unused_keys().size() == 1u);
      BOOST_TEST(checker.unused_keys().count(c) == 1u);
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2u);
      BOOST_TEST(checker.unused_keys().size() == 0u);
      BOOST_TEST(checker.used_keys().count(b) == 1u);
      BOOST_TEST(checker.used_keys().count(c) == 1u);
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {b, c, a});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 1u);
      BOOST_TEST(checker.used_keys().count(a) == 1u);
      BOOST_TEST(checker.unused_keys().size() == 2u);
      BOOST_TEST(checker.unused_keys().count(b) == 1u);
      BOOST_TEST(checker.unused_keys().count(c) == 1u);
   }

   A = authority(3, {key_weight{a, 2}, key_weight{b, 1}}, {permission_level_weight{{name("hello"), name("world")}, 3}});
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 1u);
      BOOST_TEST(checker.used_keys().count(c) == 1u);
      BOOST_TEST(checker.unused_keys().size() == 2u);
      BOOST_TEST(checker.unused_keys().count(a) == 1u);
      BOOST_TEST(checker.unused_keys().count(b) == 1u);
   }

   A = authority(2, {key_weight{a, 1}, key_weight{b, 1}}, {permission_level_weight{{name("hello"), name("world")}, 1}});
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {c}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {a, b}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {b, c}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {a, c}).satisfied(A));
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2u);
      BOOST_TEST(checker.unused_keys().size() == 1u);
      BOOST_TEST(checker.unused_keys().count(c) == 1u);
   }

   A = authority(2, {key_weight{a, 1}, key_weight{b, 1}}, {permission_level_weight{{name("hello"), name("world")}, 2}});
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {a, b}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {c}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {b}).satisfied(A));
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 1u);
      BOOST_TEST(checker.unused_keys().size() == 2u);
      BOOST_TEST(checker.used_keys().count(c) == 1u);
   }

   auto d = test.get_public_key(name("d"), "active");
   auto e = test.get_public_key(name("e"), "active");

   auto GetAuthority = [d, e] (const permission_level& perm) {
      if (perm.actor == name("top"))
         return authority(2, {key_weight{d, 1}}, {permission_level_weight{{name("bottom"), name("bottom")}, 1}});
      return authority{1, {{e, 1}}, {}};
   };

   A = authority(5, {key_weight{a, 2}, key_weight{b, 2}, key_weight{c, 2}}, {permission_level_weight{{name("top"), name("top")}, 5}});
   {
      auto checker = make_auth_checker(GetAuthority, 2, {d, e});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
   }
   {
      auto checker = make_auth_checker(GetAuthority, 2, {a, b, c, d, e});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2u);
      BOOST_TEST(checker.unused_keys().size() == 3u);
      BOOST_TEST(checker.used_keys().count(d) == 1u);
      BOOST_TEST(checker.used_keys().count(e) == 1u);
   }
   {
      auto checker = make_auth_checker(GetAuthority, 2, {a, b, c, e});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 3u);
      BOOST_TEST(checker.unused_keys().size() == 1u);
      BOOST_TEST(checker.used_keys().count(a) == 1u);
      BOOST_TEST(checker.used_keys().count(b) == 1u);
      BOOST_TEST(checker.used_keys().count(c) == 1u);
   }
   BOOST_TEST(make_auth_checker(GetAuthority, 1, {a, b, c}).satisfied(A));
   // Fails due to short recursion depth limit
   BOOST_TEST(!make_auth_checker(GetAuthority, 1, {d, e}).satisfied(A));

   BOOST_TEST(b < a);
   BOOST_TEST(b < c);
   BOOST_TEST(a < c);
   {
      // valid key order: b < a < c
      A = authority(2, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}});
      // valid key order: b < c
      auto B = authority(1, {key_weight{b, 1}, key_weight{c, 1}});
      // invalid key order: c > b
      auto C = authority(1, {key_weight{b, 1}, key_weight{c, 1}, key_weight{b, 1}});
      // invalid key order: duplicate c
      auto D = authority(1, {key_weight{b, 1}, key_weight{c, 1}, key_weight{c, 1}});
      // invalid key order: duplicate b
      auto E = authority(1, {key_weight{b, 1}, key_weight{b, 1}, key_weight{c, 1}});
      // unvalid: insufficient weight
      auto F = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}});

      auto checker = make_auth_checker(GetNullAuthority, 2, {a, b, c});
      BOOST_TEST(validate(A));
      BOOST_TEST(validate(B));
      BOOST_TEST(!validate(C));
      BOOST_TEST(!validate(D));
      BOOST_TEST(!validate(E));
      BOOST_TEST(!validate(F));

      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.unused_keys().count(b) == 1u);
      BOOST_TEST(checker.unused_keys().count(a) == 1u);
      BOOST_TEST(checker.unused_keys().count(c) == 1u);
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.satisfied(B));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.unused_keys().count(b) == 0u);
      BOOST_TEST(checker.unused_keys().count(a) == 0u);
      BOOST_TEST(checker.unused_keys().count(c) == 1u);
   }
   {
      auto A2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{name("a"), name("world")},     1},
                            permission_level_weight{{name("hello"), name("world")}, 1},
                            permission_level_weight{{name("hi"), name("world")},    1}
                          });
      auto B2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          {permission_level_weight{{name("hello"), name("world")}, 1}
                          });
      auto C2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{name("hello"), name("there")}, 1},
                            permission_level_weight{{name("hello"), name("world")}, 1}
                          });
      // invalid: duplicate
      auto D2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{name("hello"), name("world")}, 1},
                            permission_level_weight{{name("hello"), name("world")}, 2}
                          });
      // invalid: wrong order
      auto E2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{name("hello"), name("world")}, 2},
                            permission_level_weight{{name("hello"), name("there")}, 1}
                          });
      // invalid: wrong order
      auto F2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{name("hi"), name("world")}, 2},
                            permission_level_weight{{name("hello"), name("world")}, 1}
                          });

      // invalid: insufficient weight
      auto G2 = authority(7, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                             { permission_level_weight{{name("a"), name("world")},     1},
                               permission_level_weight{{name("hello"), name("world")}, 1},
                               permission_level_weight{{name("hi"), name("world")},    1}
                             });

      BOOST_TEST(validate(A2));
      BOOST_TEST(validate(B2));
      BOOST_TEST(validate(C2));
      BOOST_TEST(!validate(D2));
      BOOST_TEST(!validate(E2));
      BOOST_TEST(!validate(F2));
      BOOST_TEST(!validate(G2));
   }
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE(alphabetic_sort)
{ try {

  vector<string> words = {
    "com.o",
    "te",
    "a.....5",
    "a...4",
    ".va.ku",
    "gh",
    "1ho.la",
    "g1",
    "g",
    "a....2",
    "gg",
    "va",
    "lale.....12b",
    "a....3",
    "a....1",
    "..g",
    ".g",
    "....g",
    "a....y",
    "...g",
    "lale.....333",
  };

  std::sort(words.begin(), words.end(), std::less<string>());

  vector<uint64_t> uwords;
  for(const auto& w: words) {
    auto n = name(w.c_str());
    uwords.push_back(n.to_uint64_t());
  }

  std::sort(uwords.begin(), uwords.end(), std::less<uint64_t>());

  vector<string> tmp;
  for(const auto uw: uwords) {
    auto str = name(uw).to_string();
    tmp.push_back(str);
  }

  for(size_t i = 0; i < words.size(); ++i ) {
    BOOST_TEST(tmp[i] == words[i]);
  }

} FC_LOG_AND_RETHROW() }


BOOST_AUTO_TEST_CASE(transaction_test) { try {

   testing::validating_tester test;
   signed_transaction trx;

   fc::variant pretty_trx = fc::mutable_variant_object()
      ("actions", fc::variants({
         fc::mutable_variant_object()
            ("account", "sysio")
            ("name", "reqauth")
            ("authorization", fc::variants({
               fc::mutable_variant_object()
                  ("actor", "sysio")
                  ("permission", "active")
            }))
            ("data", fc::mutable_variant_object()
               ("from", "sysio")
            )
         })
      )
      // lets also push a context free action, the multi chain test will then also include a context free action
      ("context_free_actions", fc::variants({
         fc::mutable_variant_object()
            ("account", "sysio")
            ("name", "nonce")
            ("data", fc::raw::pack(std::string("dummy")))
         })
      );

   abi_serializer::from_variant(pretty_trx, trx, test.get_resolver(), abi_serializer::create_yield_function( test.abi_serializer_max_time ));

   test.set_transaction_headers(trx);

   trx.expiration = fc::time_point_sec{fc::time_point::now()};
   trx.validate();
   BOOST_CHECK_EQUAL(0u, trx.signatures.size());
   ((const signed_transaction &)trx).sign( test.get_private_key( config::system_account_name, "active" ), test.control->get_chain_id());
   BOOST_CHECK_EQUAL(0u, trx.signatures.size());
   auto private_key = test.get_private_key( config::system_account_name, "active" );
   auto public_key = private_key.get_public_key();
   trx.sign( private_key, test.control->get_chain_id()  );
   BOOST_CHECK_EQUAL(1u, trx.signatures.size());
   trx.validate();

   packed_transaction pkt(trx, packed_transaction::compression_type::none);

   packed_transaction pkt2(trx, packed_transaction::compression_type::zlib);

   BOOST_CHECK_EQUAL(true, trx.expiration ==  pkt.expiration());
   BOOST_CHECK_EQUAL(true, trx.expiration == pkt2.expiration());

   BOOST_CHECK_EQUAL(trx.id(), pkt.id());
   BOOST_CHECK_EQUAL(trx.id(), pkt2.id());

   bytes raw = pkt.get_raw_transaction();
   bytes raw2 = pkt2.get_raw_transaction();
   BOOST_CHECK_EQUAL(raw.size(), raw2.size());
   BOOST_CHECK_EQUAL(true, std::equal(raw.begin(), raw.end(), raw2.begin()));

   BOOST_CHECK_EQUAL(pkt.get_signed_transaction().id(), pkt2.get_signed_transaction().id());
   BOOST_CHECK_EQUAL(pkt.get_signed_transaction().id(), pkt2.id());

   flat_set<public_key_type> keys;
   auto cpu_time1 = pkt.get_signed_transaction().get_signature_keys(test.control->get_chain_id(), fc::time_point::maximum(), keys);
   BOOST_CHECK_EQUAL(1u, keys.size());
   BOOST_CHECK_EQUAL(public_key, *keys.begin());
   keys.clear();
   auto cpu_time2 = pkt.get_signed_transaction().get_signature_keys(test.control->get_chain_id(), fc::time_point::maximum(), keys);
   BOOST_CHECK_EQUAL(1u, keys.size());
   BOOST_CHECK_EQUAL(public_key, *keys.begin());

   BOOST_CHECK(cpu_time1 > fc::microseconds(0));
   BOOST_CHECK(cpu_time2 > fc::microseconds(0));

   // pack
   uint32_t pack_size = fc::raw::pack_size( pkt );
   vector<char> buf(pack_size);
   fc::datastream<char*> ds(buf.data(), pack_size);

   fc::raw::pack( ds, pkt );
   // unpack
   ds.seekp(0);
   packed_transaction pkt3;
   fc::raw::unpack(ds, pkt3);
   // pack again
   pack_size = fc::raw::pack_size( pkt3 );
   fc::datastream<char*> ds2(buf.data(), pack_size);
   fc::raw::pack( ds2, pkt3 );
   // unpack
   ds2.seekp(0);
   packed_transaction pkt4;
   fc::raw::unpack(ds2, pkt4);
   // to/from variant
   fc::variant pkt_v( pkt3 );
   packed_transaction pkt5;
   fc::from_variant(pkt_v, pkt5);

   bytes raw3 = pkt3.get_raw_transaction();
   bytes raw4 = pkt4.get_raw_transaction();
   BOOST_CHECK_EQUAL(raw.size(), raw3.size());
   BOOST_CHECK_EQUAL(raw3.size(), raw4.size());
   BOOST_CHECK_EQUAL(true, std::equal(raw.begin(), raw.end(), raw3.begin()));
   BOOST_CHECK_EQUAL(true, std::equal(raw.begin(), raw.end(), raw4.begin()));
   BOOST_CHECK_EQUAL(pkt.get_signed_transaction().id(), pkt3.get_signed_transaction().id());
   BOOST_CHECK_EQUAL(pkt.get_signed_transaction().id(), pkt4.get_signed_transaction().id());
   BOOST_CHECK_EQUAL(pkt.get_signed_transaction().id(), pkt5.get_signed_transaction().id()); // failure indicates reflector_init not working
   BOOST_CHECK_EQUAL(pkt.id(), pkt4.get_signed_transaction().id());
   BOOST_CHECK_EQUAL(true, trx.expiration == pkt4.expiration());
   BOOST_CHECK_EQUAL(true, trx.expiration == pkt4.get_signed_transaction().expiration);
   keys.clear();
   pkt4.get_signed_transaction().get_signature_keys(test.control->get_chain_id(), fc::time_point::maximum(), keys);
   BOOST_CHECK_EQUAL(1u, keys.size());
   BOOST_CHECK_EQUAL(public_key, *keys.begin());

   // verify packed_transaction creation from packed data
   {
      auto packed = fc::raw::pack( static_cast<const transaction&>(pkt5.get_transaction()) );
      vector<signature_type> psigs = pkt5.get_signatures();
      vector<bytes> pcfd = pkt5.get_context_free_data();
      packed_transaction pkt7( std::move(packed), std::move(psigs), std::move(pcfd), packed_transaction::compression_type::none );
      BOOST_CHECK_EQUAL(pkt5.get_transaction().id(), pkt7.get_transaction().id());
   }
   {
      auto packed = fc::raw::pack( static_cast<const transaction&>(pkt5.get_transaction()) );
      vector<signature_type> psigs = pkt5.get_signatures();
      vector<bytes> pcfd = pkt5.get_context_free_data();
      packed.push_back( '8' ); packed.push_back( '8' ); // extra ignored
      auto packed_copy = packed;
      packed_transaction pkt8( std::move(packed), std::move(psigs), std::move(pcfd), packed_transaction::compression_type::none );
      BOOST_CHECK_EQUAL( pkt5.get_transaction().id(), pkt8.get_transaction().id() );
      BOOST_CHECK( packed_copy != fc::raw::pack( static_cast<const transaction&>(pkt8.get_transaction()) ) );
      BOOST_CHECK( packed_copy == pkt8.get_packed_transaction() ); // extra maintained
   }

} FC_LOG_AND_RETHROW() }


BOOST_AUTO_TEST_CASE(signed_int_test) { try {
    char buf[32];
    fc::datastream<char*> ds(buf,32);
    signed_int a(47), b((1<<30)+2), c(-47), d(-(1<<30)-2); //small +, big +, small -, big -
    signed_int ee;
    fc::raw::pack(ds,a);
    ds.seekp(0);
    fc::raw::unpack(ds,ee);
    ds.seekp(0);
    BOOST_CHECK_EQUAL(a,ee);
    fc::raw::pack(ds,b);
    ds.seekp(0);
    fc::raw::unpack(ds,ee);
    ds.seekp(0);
    BOOST_CHECK_EQUAL(b,ee);
    fc::raw::pack(ds,c);
    ds.seekp(0);
    fc::raw::unpack(ds,ee);
    ds.seekp(0);
    BOOST_CHECK_EQUAL(c,ee);
    fc::raw::pack(ds,d);
    ds.seekp(0);
    fc::raw::unpack(ds,ee);
    ds.seekp(0);
    BOOST_CHECK_EQUAL(d,ee);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE(transaction_metadata_test) { try {

   testing::validating_tester test;
   signed_transaction trx;

   fc::variant pretty_trx = fc::mutable_variant_object()
      ("actions", fc::variants({
         fc::mutable_variant_object()
            ("account", "sysio")
            ("name", "reqauth")
            ("authorization", fc::variants({
               fc::mutable_variant_object()
                  ("actor", "sysio")
                  ("permission", "active")
            }))
            ("data", fc::mutable_variant_object()
               ("from", "sysio")
            )
         })
      )
      ("context_free_actions", fc::variants({
         fc::mutable_variant_object()
            ("account", "sysio")
            ("name", "nonce")
            ("data", fc::raw::pack(std::string("dummy data")))
         })
      );

      abi_serializer::from_variant(pretty_trx, trx, test.get_resolver(), abi_serializer::create_yield_function( test.abi_serializer_max_time ));

      test.set_transaction_headers(trx);
      trx.expiration = fc::time_point_sec{fc::time_point::now()};

      auto private_key = test.get_private_key( config::system_account_name, "active" );
      auto public_key = private_key.get_public_key();
      trx.sign( private_key, test.control->get_chain_id()  );
      BOOST_CHECK_EQUAL(1u, trx.signatures.size());

      packed_transaction pkt(trx, packed_transaction::compression_type::none);
      packed_transaction pkt2(trx, packed_transaction::compression_type::zlib);

      packed_transaction_ptr ptrx = std::make_shared<packed_transaction>( trx, packed_transaction::compression_type::none);
      packed_transaction_ptr ptrx2 = std::make_shared<packed_transaction>( trx, packed_transaction::compression_type::zlib);

      BOOST_CHECK_EQUAL(trx.id(), pkt.id());
      BOOST_CHECK_EQUAL(trx.id(), pkt2.id());
      BOOST_CHECK_EQUAL(trx.id(), ptrx->id());
      BOOST_CHECK_EQUAL(trx.id(), ptrx2->id());

      named_thread_pool<struct misc> thread_pool;
      thread_pool.start( 5, {} );

      auto fut = transaction_metadata::start_recover_keys( ptrx, thread_pool.get_executor(), test.control->get_chain_id(), fc::microseconds::maximum(), transaction_metadata::trx_type::input );
      auto fut2 = transaction_metadata::start_recover_keys( ptrx2, thread_pool.get_executor(), test.control->get_chain_id(), fc::microseconds::maximum(), transaction_metadata::trx_type::input );

      // start another key reovery on same packed_transaction, creates a new future with transaction_metadata, should not interfere with above
      transaction_metadata::start_recover_keys( ptrx, thread_pool.get_executor(), test.control->get_chain_id(), fc::microseconds::maximum(), transaction_metadata::trx_type::input );
      transaction_metadata::start_recover_keys( ptrx2, thread_pool.get_executor(), test.control->get_chain_id(), fc::microseconds::maximum(), transaction_metadata::trx_type::input );

      auto mtrx = fut.get();
      const auto& keys = mtrx->recovered_keys();
      BOOST_CHECK_EQUAL(1u, keys.size());
      BOOST_CHECK_EQUAL(public_key, *keys.begin());

      // again, can be called multiple times, current implementation it is just an attribute of transaction_metadata
      const auto& keys2 = mtrx->recovered_keys();
      BOOST_CHECK_EQUAL(1u, keys2.size());
      BOOST_CHECK_EQUAL(public_key, *keys2.begin());

      auto mtrx2 = fut2.get();
      const auto& keys3 = mtrx2->recovered_keys();
      BOOST_CHECK_EQUAL(1u, keys3.size());
      BOOST_CHECK_EQUAL(public_key, *keys3.begin());

      thread_pool.stop();

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE(reflector_init_test) {
   try {

      base_reflect br;
      br.bv = 42;
      derived_reflect dr;
      dr.bv = 42;
      dr.dv = 52;
      final_reflect fr;
      fr.bv = 42;
      fr.dv = 52;
      fr.fv = 62;
      BOOST_CHECK_EQUAL( br.base_reflect_initialized, false );
      BOOST_CHECK_EQUAL( dr.derived_reflect_initialized, false );

      { // base
         // pack
         uint32_t pack_size = fc::raw::pack_size( br );
         vector<char> buf( pack_size );
         fc::datastream<char*> ds( buf.data(), pack_size );

         fc::raw::pack( ds, br );
         // unpack
         ds.seekp( 0 );
         base_reflect br2;
         fc::raw::unpack( ds, br2 );
         // pack again
         pack_size = fc::raw::pack_size( br2 );
         fc::datastream<char*> ds2( buf.data(), pack_size );
         fc::raw::pack( ds2, br2 );
         // unpack
         ds2.seekp( 0 );
         base_reflect br3;
         fc::raw::unpack( ds2, br3 );
         // to/from variant
         fc::variant v( br3 );
         base_reflect br4;
         fc::from_variant( v, br4 );

         BOOST_CHECK_EQUAL( br2.bv, 42 );
         BOOST_CHECK_EQUAL( br2.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( br2.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( br3.bv, 42 );
         BOOST_CHECK_EQUAL( br3.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( br3.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( br4.bv, 42 );
         BOOST_CHECK_EQUAL( br4.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( br4.base_reflect_called, 1 );
      }
      { // derived
         // pack
         uint32_t pack_size = fc::raw::pack_size( dr );
         vector<char> buf( pack_size );
         fc::datastream<char*> ds( buf.data(), pack_size );

         fc::raw::pack( ds, dr );
         // unpack
         ds.seekp( 0 );
         derived_reflect dr2;
         fc::raw::unpack( ds, dr2 );
         // pack again
         pack_size = fc::raw::pack_size( dr2 );
         fc::datastream<char*> ds2( buf.data(), pack_size );
         fc::raw::pack( ds2, dr2 );
         // unpack
         ds2.seekp( 0 );
         derived_reflect dr3;
         fc::raw::unpack( ds2, dr3 );
         // to/from variant
         fc::variant v( dr3 );
         derived_reflect dr4;
         fc::from_variant( v, dr4 );

         BOOST_CHECK_EQUAL( dr2.bv, 42 );
         BOOST_CHECK_EQUAL( dr2.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr2.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( dr3.bv, 42 );
         BOOST_CHECK_EQUAL( dr3.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr3.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( dr4.bv, 42 );
         BOOST_CHECK_EQUAL( dr4.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr4.base_reflect_called, 1 );

         BOOST_CHECK_EQUAL( dr2.dv, 52 );
         BOOST_CHECK_EQUAL( dr2.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr2.derived_reflect_called, 1 );
         BOOST_CHECK_EQUAL( dr3.dv, 52 );
         BOOST_CHECK_EQUAL( dr3.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr3.derived_reflect_called, 1 );
         BOOST_CHECK_EQUAL( dr4.dv, 52 );
         BOOST_CHECK_EQUAL( dr4.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr4.derived_reflect_called, 1 );

         base_reflect br5;
         ds2.seekp( 0 );
         fc::raw::unpack( ds2, br5 );
         base_reflect br6;
         fc::from_variant( v, br6 );

         BOOST_CHECK_EQUAL( br5.bv, 42 );
         BOOST_CHECK_EQUAL( br5.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( br5.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( br6.bv, 42 );
         BOOST_CHECK_EQUAL( br6.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( br6.base_reflect_called, 1 );
      }
      { // final
         // pack
         uint32_t pack_size = fc::raw::pack_size( fr );
         vector<char> buf( pack_size );
         fc::datastream<char*> ds( buf.data(), pack_size );

         fc::raw::pack( ds, fr );
         // unpack
         ds.seekp( 0 );
         final_reflect fr2;
         fc::raw::unpack( ds, fr2 );
         // pack again
         pack_size = fc::raw::pack_size( fr2 );
         fc::datastream<char*> ds2( buf.data(), pack_size );
         fc::raw::pack( ds2, fr2 );
         // unpack
         ds2.seekp( 0 );
         final_reflect fr3;
         fc::raw::unpack( ds2, fr3 );
         // to/from variant
         fc::variant v( fr3 );
         final_reflect fr4;
         fc::from_variant( v, fr4 );

         BOOST_CHECK_EQUAL( fr2.bv, 42 );
         BOOST_CHECK_EQUAL( fr2.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr2.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( fr3.bv, 42 );
         BOOST_CHECK_EQUAL( fr3.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr3.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( fr4.bv, 42 );
         BOOST_CHECK_EQUAL( fr4.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr4.base_reflect_called, 1 );

         BOOST_CHECK_EQUAL( fr2.dv, 52 );
         BOOST_CHECK_EQUAL( fr2.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr2.derived_reflect_called, 1 );
         BOOST_CHECK_EQUAL( fr3.dv, 52 );
         BOOST_CHECK_EQUAL( fr3.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr3.derived_reflect_called, 1 );
         BOOST_CHECK_EQUAL( fr4.dv, 52 );
         BOOST_CHECK_EQUAL( fr4.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr4.derived_reflect_called, 1 );

         BOOST_CHECK_EQUAL( fr2.fv, 62 );
         BOOST_CHECK_EQUAL( fr2.final_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr2.final_reflect_called, 1 );
         BOOST_CHECK_EQUAL( fr3.fv, 62 );
         BOOST_CHECK_EQUAL( fr3.final_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr3.final_reflect_called, 1 );
         BOOST_CHECK_EQUAL( fr4.fv, 62 );
         BOOST_CHECK_EQUAL( fr4.final_reflect_initialized, true );
         BOOST_CHECK_EQUAL( fr4.final_reflect_called, 1 );

         base_reflect br5;
         ds2.seekp( 0 );
         fc::raw::unpack( ds2, br5 );
         base_reflect br6;
         fc::from_variant( v, br6 );

         BOOST_CHECK_EQUAL( br5.bv, 42 );
         BOOST_CHECK_EQUAL( br5.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( br5.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( br6.bv, 42 );
         BOOST_CHECK_EQUAL( br6.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( br6.base_reflect_called, 1 );

         derived_reflect dr7;
         ds2.seekp( 0 );
         fc::raw::unpack( ds2, dr7 );
         derived_reflect dr8;
         fc::from_variant( v, dr8 );

         BOOST_CHECK_EQUAL( dr7.bv, 42 );
         BOOST_CHECK_EQUAL( dr7.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr7.base_reflect_called, 1 );
         BOOST_CHECK_EQUAL( dr8.bv, 42 );
         BOOST_CHECK_EQUAL( dr8.base_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr8.base_reflect_called, 1 );

         BOOST_CHECK_EQUAL( dr7.dv, 52 );
         BOOST_CHECK_EQUAL( dr7.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr7.derived_reflect_called, 1 );
         BOOST_CHECK_EQUAL( dr8.dv, 52 );
         BOOST_CHECK_EQUAL( dr8.derived_reflect_initialized, true );
         BOOST_CHECK_EQUAL( dr8.derived_reflect_called, 1 );
      }

   } FC_LOG_AND_RETHROW()
}

// Verify appbase::execution_priority_queue uses a stable priority queue so that jobs are executed
// in order, FIFO, as submitted.
BOOST_AUTO_TEST_CASE(stable_priority_queue_test) {
  try {
     using namespace std::chrono_literals;

     appbase::execution_priority_queue pri_queue;
     auto io_serv = std::make_shared<boost::asio::io_service>();
     auto work_ptr = std::make_unique<boost::asio::io_service::work>(*io_serv);
     std::atomic<int> posted{0};

     std::thread t( [io_serv, &pri_queue, &posted]() {
        while( posted < 100 && io_serv->run_one() ) {
           ++posted;
        }
        bool more = true;
        while( more || io_serv->run_one() ) {
           while( io_serv->poll_one() ) {}
           // execute the highest priority item
           more = pri_queue.execute_highest();
        }
     } );
     std::atomic<int> ran{0};
     std::mutex mx;
     std::vector<int> results;
     size_t order = std::numeric_limits<size_t>::max();
     for( int i = 0; i < 50; ++i ) {
        boost::asio::post(*io_serv, pri_queue.wrap(appbase::priority::low, --order, [io_serv, &mx, &ran, &results, i](){
           std::lock_guard<std::mutex> g(mx);
           results.push_back( 50 + i );
           ++ran;
        }));
        boost::asio::post(*io_serv, pri_queue.wrap(appbase::priority::high, --order, [io_serv, &mx, &ran, &results, i](){
           std::lock_guard<std::mutex> g(mx);
           results.push_back( i );
           ++ran;
        }));
     }

     while( ran < 100 ) std::this_thread::sleep_for( 5us );

     work_ptr.reset();
     io_serv->stop();
     t.join();

     std::lock_guard<std::mutex> g(mx);
     BOOST_CHECK_EQUAL( 100u, results.size() );
     for( int i = 0; i < 100; ++i ) {
        BOOST_CHECK_EQUAL( i, results.at( i ) );
     }

  } FC_LOG_AND_RETHROW()
}

// test that std::bad_alloc is being thrown
// ASAN warns "exceeds maximum supported size", so skip when ASAN enabled
// gcc sets __SANITIZE_ADDRESS__, but clang uses __has_feature(), when ASAN enabled
#if defined(__has_feature) && !defined(__SANITIZE_ADDRESS__)
   #if __has_feature(address_sanitizer)
      #define __SANITIZE_ADDRESS__ 1
   #endif
#endif
#ifndef __SANITIZE_ADDRESS__
BOOST_AUTO_TEST_CASE(bad_alloc_test) {
   tester t; // force a controller to be constructed and set the new_handler
   int* ptr = nullptr;
   const auto fail = [&]() {
      ptr = new int[std::numeric_limits<int64_t>::max()/16];
   };
   BOOST_CHECK_THROW( fail(), std::bad_alloc );
   BOOST_CHECK( ptr == nullptr );
}
#endif

BOOST_AUTO_TEST_CASE(named_thread_pool_test) {
   {
      named_thread_pool<struct misc> thread_pool;
      thread_pool.start( 5, {} );

      std::promise<void> p;
      auto f = p.get_future();
      boost::asio::post( thread_pool.get_executor(), [&p](){
         p.set_value();
      });
      BOOST_TEST( (f.wait_for( 100ms ) == std::future_status::ready) );
   }
   { // delayed start
      named_thread_pool<struct misc> thread_pool;

      std::promise<void> p;
      auto f = p.get_future();
      boost::asio::post( thread_pool.get_executor(), [&p](){
         p.set_value();
      });
      BOOST_TEST( (f.wait_for( 10ms ) == std::future_status::timeout) );
      thread_pool.start( 5, {} );
      BOOST_TEST( (f.wait_for( 100ms ) == std::future_status::ready) );
   }
   { // exception
      std::promise<fc::exception> ep;
      auto ef = ep.get_future();
      named_thread_pool<struct misc> thread_pool;
      thread_pool.start( 5, [&ep](const fc::exception& e) { ep.set_value(e); } );

      boost::asio::post( thread_pool.get_executor(), [](){
         FC_ASSERT( false, "oops throw in thread pool" );
      });
      BOOST_TEST( (ef.wait_for( 100ms ) == std::future_status::ready) );
      BOOST_TEST( ef.get().to_detail_string().find("oops throw in thread pool") != std::string::npos );

      // we can restart, after a stop
      BOOST_REQUIRE_THROW( thread_pool.start( 5, [&ep](const fc::exception& e) { ep.set_value(e); } ), fc::assert_exception );
      thread_pool.stop();

      std::promise<void> p;
      auto f = p.get_future();
      boost::asio::post( thread_pool.get_executor(), [&p](){
         p.set_value();
      });
      thread_pool.start( 5, [&ep](const fc::exception& e) { ep.set_value(e); } );
      BOOST_TEST( (f.wait_for( 100ms ) == std::future_status::ready) );
   }
}

BOOST_AUTO_TEST_CASE(public_key_from_hash) {
   auto private_key_string = std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3");
   auto expected_public_key = std::string("SYS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV");
   auto test_private_key = fc::crypto::private_key(private_key_string);
   auto test_public_key = test_private_key.get_public_key();
   fc::crypto::public_key sys_pk(expected_public_key);

   BOOST_CHECK_EQUAL(private_key_string, test_private_key.to_string({}));
   BOOST_CHECK_EQUAL(expected_public_key, test_public_key.to_string({}));
   BOOST_CHECK_EQUAL(expected_public_key, sys_pk.to_string({}));

   fc::ecc::public_key_data data;
   data.data[0] = 0x80; // not necessary, 0 also works
   fc::sha256 hash = fc::sha256::hash("unknown private key");
   std::memcpy(&data.data[1], hash.data(), hash.data_size() );
   fc::ecc::public_key_shim shim(data);
   fc::crypto::public_key sys_unknown_pk(std::move(shim));
   ilog( "public key with no known private key: ${k}", ("k", sys_unknown_pk) );
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace sysio
