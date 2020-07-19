project(test-Html)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Gumbo_INCLUDE_DIRS}
    ${GumboQuery_INCLUDE_DIRS}
)

set(test_Html_headers
    test-WizHtmlTool.h
    ${CMAKE_SOURCE_DIR}/src/html/WizHtmlTool.h
    ${CMAKE_SOURCE_DIR}/src/html/WizGumboHelper.h
)

set(test_Html_sources
    test-WizHtmlTool.cpp
    ${CMAKE_SOURCE_DIR}/src/html/WizHtmlTool.cpp
    ${CMAKE_SOURCE_DIR}/src/html/WizGumboHelper.cpp
)

find_package( Qt5Widgets CONFIG REQUIRED )

add_executable(test-html ${test_Html_headers} ${test_Html_sources})
qt_use_modules(test-html)

target_link_libraries(test-html
    ${GumboQuery_LIBRARIES}
    ${Gumbo_LIBRARIES}
)

add_test(test-Html ${CMAKE_BINARY_DIR}/bin/test-html)