project(test-WizUpgradeChecker)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(test-WizUpgradeChecker_headers
    ${CMAKE_SOURCE_DIR}/src/share/WizEventLoop.h    
    ${CMAKE_SOURCE_DIR}/src/WizUpgrade.h
    test-WizUpgradeChecker.h
)

set(test-WizUpgradeChecker_sources
    ${CMAKE_SOURCE_DIR}/src/share/WizEventLoop.cpp
    ${CMAKE_SOURCE_DIR}/src/WizUpgrade.cpp
    test-WizUpgradeChecker.cpp
)

add_executable(test-WizUpgradeChecker ${test-WizUpgradeChecker_headers} ${test-WizUpgradeChecker_sources})

qt_use_modules(test-WizUpgradeChecker)

add_test(test-WizUpgradeChecker ${CMAKE_BINARY_DIR}/bin/test-WizUpgradeChecker)