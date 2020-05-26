project(test-JSPlugin)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(test_JSPlugin_headers
    test-JSPlugin.h
    ${CMAKE_SOURCE_DIR}/src/jsplugin/JSPluginSpec.h
)

set(test_JSPlugin_sources
    test-JSPlugin.cpp
    ${CMAKE_SOURCE_DIR}/src/jsplugin/JSPluginSpec.cpp
)

add_executable(test-jsplugin ${test_JSPlugin_headers} ${test_JSPlugin_sources})
qt_use_modules(test-jsplugin)

add_test(test-JSPlugin ${CMAKE_BINARY_DIR}/bin/test-jsplugin)