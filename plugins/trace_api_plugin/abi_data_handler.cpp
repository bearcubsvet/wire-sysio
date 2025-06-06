#include <sysio/trace_api/abi_data_handler.hpp>
#include <sysio/chain/abi_serializer.hpp>

namespace sysio::trace_api {

   void abi_data_handler::add_abi( const chain::name& name, chain::abi_def&& abi ) {
      // currently abis are operator provided so no need to protect against abuse
      abi_serializer_by_account.emplace(name,
            std::make_shared<chain::abi_serializer>(std::move(abi), chain::abi_serializer::create_yield_function(fc::microseconds::maximum())));
   }

   std::tuple<fc::variant, std::optional<fc::variant>> abi_data_handler::serialize_to_variant(const std::variant<action_trace_v0, action_trace_v1>& action) {
      auto account = std::visit([](auto &&action) -> auto { return action.account; }, action);

      if (abi_serializer_by_account.count(account) > 0) {
         const auto &serializer_p = abi_serializer_by_account.at(account);
         auto action_name = std::visit([](auto &&action) -> auto { return action.action; }, action);
         auto type_name = serializer_p->get_action_type(action_name);

         if (!type_name.empty()) {
            try {
               // abi_serializer expects a yield function that takes a recursion depth
               // abis are user provided, do not use a deadline
               auto abi_yield = [](size_t recursion_depth) {
                  SYS_ASSERT( recursion_depth < chain::abi_serializer::max_recursion_depth, chain::abi_recursion_depth_exception,
                              "exceeded max_recursion_depth ${r} ", ("r", chain::abi_serializer::max_recursion_depth) );
               };
               return std::visit([&](auto &&action) -> std::tuple<fc::variant, std::optional<fc::variant>> {
                  using T = std::decay_t<decltype(action)>;
                  std::optional<fc::variant> ret_data;
                  auto params = serializer_p->binary_to_variant(type_name, action.data, abi_yield);
                  if constexpr (std::is_same_v<T, action_trace_v1>) {
                     if(action.return_value.size() > 0) {
                        auto return_type_name = serializer_p->get_action_result_type(action_name);
                        if (!return_type_name.empty()) {
                           ret_data = serializer_p->binary_to_variant(return_type_name, action.return_value, abi_yield);
                        }
                     }
                  }
                  return {params, ret_data};
               }, action);
            } catch (...) {
               except_handler(MAKE_EXCEPTION_WITH_CONTEXT(std::current_exception()));
            }
         }
      }

      return {};
   }
}
