file(GLOB HEADERS "include/sysio/http_plugin/*.hpp")
add_library( http_plugin
             http_plugin.cpp
             ${HEADERS} )

target_link_libraries( http_plugin sysio_chain custom_appbase fc)
target_include_directories( http_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" ${CMAKE_SOURCE_DIR}/plugins/chain_interface/include )

add_subdirectory( tests )
