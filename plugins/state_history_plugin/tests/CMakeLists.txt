add_executable( test_state_history main.cpp session_test.cpp plugin_config_test.cpp)
target_link_libraries(test_state_history state_history_plugin sysio_testing sysio_chain_wrap)
target_include_directories( test_state_history PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include" )

add_test(test_state_history test_state_history)
set_property(TEST test_state_history PROPERTY LABELS nonparallelizable_tests)