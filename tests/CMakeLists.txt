cmake_minimum_required(VERSION 3.0)

project(Tests)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Import QtTest module
find_package( Qt5Core CONFIG REQUIRED )
find_package( Qt5Network CONFIG REQUIRED )
find_package( Qt5Test CONFIG REQUIRED )
set(QT_VERSION ${Qt5_VERSION})
set(CMAKE_AUTOMOC ON)
macro(qt_use_modules)
    target_link_libraries( ${ARGV0} 
        Qt5::Core 
        Qt5::Network 
        Qt5::Test
    )
endmacro()

add_subdirectory(TestWizUpgradeChecker)
add_subdirectory(TestHtml)