project(TestWizUpgradeChecker)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(TestWizUpgradeChecker_headers
    ${CMAKE_SOURCE_DIR}/src/share/WizEventLoop.h    
    ${CMAKE_SOURCE_DIR}/src/WizUpgrade.h
    src/TestWizUpgradeChecker.h
)

set(TestWizUpgradeChecker_sources
    ${CMAKE_SOURCE_DIR}/src/share/WizEventLoop.cpp
    ${CMAKE_SOURCE_DIR}/src/WizUpgrade.cpp
    src/TestWizUpgradeChecker.cpp
)

add_executable(TestWizUpgradeChecker ${TestWizUpgradeChecker_headers} ${TestWizUpgradeChecker_sources})

qt_use_modules(TestWizUpgradeChecker)

add_test(TestWizUpgradeChecker ${CMAKE_BINARY_DIR}/bin/TestWizUpgradeChecker)