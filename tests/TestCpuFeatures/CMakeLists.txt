project(TestCpuFeatures)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${quazip_INCLUDE_DIRS}
    ${zlib_INCLUDE_DIRS}
)

set(TestWizUpgradeChecker_headers

)

set(TestWizUpgradeChecker_sources
    src/main.cpp
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options("-march=native")
endif()

find_package( Qt5Widgets CONFIG REQUIRED )

add_executable(TestCpuFeatures ${TestWizUpgradeChecker_headers} ${TestWizUpgradeChecker_sources})
qt_use_modules(TestCpuFeatures)

target_link_libraries(TestCpuFeatures 
    Qt5::Gui Qt5::Widgets 
    ${quazip_LIBRARIES}
    ${zlib_LIBRARIES}
)

add_test(TestCpuFeatures ${CMAKE_BINARY_DIR}/bin/TestCpuFeatures)