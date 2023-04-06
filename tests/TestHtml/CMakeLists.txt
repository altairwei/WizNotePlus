project(test-Html)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src/libs/3rdparty
)

set(test_Html_headers
    test-WizHtmlTool.h
    ${CMAKE_SOURCE_DIR}/src/html/WizHtmlTool.h
    ${CMAKE_SOURCE_DIR}/src/html/WizGumboHelper.h
    ${CMAKE_SOURCE_DIR}/src/html/WizHtmlConverter.h
)

set(test_Html_sources
    test-WizHtmlTool.cpp
    ${CMAKE_SOURCE_DIR}/src/html/WizHtmlTool.cpp
    ${CMAKE_SOURCE_DIR}/src/html/WizGumboHelper.cpp
    ${CMAKE_SOURCE_DIR}/src/html/WizHtmlConverter.cpp
)

find_package( Qt5Widgets CONFIG REQUIRED )

add_executable(test-html ${test_Html_headers} ${test_Html_sources})
qt_use_modules(test-html)

target_link_libraries(test-html gumbo)

add_test(test-Html ${CMAKE_BINARY_DIR}/bin/test-html)
