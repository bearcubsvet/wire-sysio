#include <cstdlib>

#include <iostream>

#include <sysio/chain/exceptions.hpp>

#include <fc/log/logger.hpp>

#include <boost/test/included/unit_test.hpp>

//extern uint32_t SYS_TESTING_GENESIS_TIMESTAMP;

void translate_fc_exception(const fc::exception &e) {
   std::cerr << "\033[33m" <<  e.to_detail_string() << "\033[0m" << std::endl;
   throw std::runtime_error("Caught Unexpected Exception");
}

static bool is_verbose = false;
void setup_test_logging() {
   if(is_verbose) {
      fc::logger::get(DEFAULT_LOGGER).set_log_level(fc::log_level::debug);
   } else {
      fc::logger::get(DEFAULT_LOGGER).set_log_level(fc::log_level::off);
   }
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]) {
   // Turn off blockchain logging if no --verbose parameter is not added
   // To have verbose enabled, call "tests/chain_test -- --verbose"
   std::string verbose_arg = "--verbose";
   for (int i = 0; i < argc; i++) {
      if (verbose_arg == argv[i]) {
         is_verbose = true;
         break;
      }
   }
   setup_test_logging();

   // Register fc::exception translator
   boost::unit_test::unit_test_monitor.register_exception_translator<fc::exception>(&translate_fc_exception);

   std::srand(time(NULL));
   std::cout << "Random number generator seeded to " << time(NULL) << std::endl;
   /*
   const char* genesis_timestamp_str = getenv("SYS_TESTING_GENESIS_TIMESTAMP");
   if( genesis_timestamp_str != nullptr )
   {
      SYS_TESTING_GENESIS_TIMESTAMP = std::stoul( genesis_timestamp_str );
   }
   std::cout << "SYS_TESTING_GENESIS_TIMESTAMP is " << SYS_TESTING_GENESIS_TIMESTAMP << std::endl;
   */
   return nullptr;
}
