# NOTE for OSX users:
# export MACOSX_DEPLOYMENT_TARGET=10.7 on .bash_profile if you want to compile use 10.7 sdk
# sdk location: /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
# you can download old sdk from here: https://github.com/JuliaEichler/Mac_OSX_SDKs
# default is 10.7 for binary release.

# CMake arguments for WizNote
# APPSTORE_BUILD  build for Apple Appsotre, would not use qt deploy
# XCODEBUILD      adjust project params to suit for xcode
# UPDATE_TRANSLATIONS  update transation files
# PLCrashReporter   use PLCrashReporter for mac, need PLCrashReporter framework

cmake_minimum_required(VERSION 3.0)

project(WizNotePlus VERSION 2.8.0)

#============================================================================
# Options and Variables
#============================================================================

option(BUILD_TESTING "Building the test target or not." OFF)

# Excute conan outside or not.
option(CONAN_INSTALL_MANUALLY "Run Conan install command manually." OFF)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: Debug Release."
      FORCE)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11)

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

# WizNotePlus Dirs
if(APPLE)
    set(WIZNOTE_ROOT_DIR ${CMAKE_BINARY_DIR}/bin/WizNote.app)
    set(WIZNOTE_CONTENTS_DIR ${WIZNOTE_ROOT_DIR}/Contents)
    set(WIZNOTE_RESOURCE_DIR ${WIZNOTE_ROOT_DIR}/Contents/Resources)
else(APPLE)
    set(WIZNOTE_ROOT_DIR ${CMAKE_BINARY_DIR})
    set(WIZNOTE_RESOURCE_DIR ${WIZNOTE_ROOT_DIR}/share)
endif(APPLE)

#============================================================================
# RPATH settings
#============================================================================

# rpath resolve.
#set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)     // would cause wheelEvent problem on centos/fedora when compiled with qt5
if(APPLE)
    # make the install names for dylibs include "@rpath/" prefix.
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_NAME_DIR "@rpath/PlugIns")
    set(CMAKE_INSTALL_RPATH "@loader_path/..;@loader_path/../Frameworks;@executable_path/..;@executable_path/../Frameworks;@rpath")
    #SET(PLCrashReporter on)
else(APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/..;$ORIGIN/../lib")
    set (openssl-linked on)
endif(APPLE)

#============================================================================
# 3rd-party libraries
#============================================================================

# install conan 3rdparty libraries to package dirs.
message("\nStart generate 3rdparty dependencies as ${CMAKE_BUILD_TYPE}:\n")

if(NOT CONAN_INSTALL_MANUALLY)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                        "${CMAKE_BINARY_DIR}/conan.cmake")
    endif()
    include(${CMAKE_BINARY_DIR}/conan.cmake)
    conan_check(REQUIRED)
    conan_cmake_run(
        CONANFILE conanfile.py
        INSTALL_FOLDER ${CMAKE_BINARY_DIR}
        BUILD_TYPE ${CMAKE_BUILD_TYPE}
        BUILD missing
    )
    include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

# set find_package search path
# This is a workaround of conan-zlib bug https://github.com/conan-community/community/issues/259
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})

# setup 3rd-party package
find_package(cryptopp REQUIRED)
find_package(quazip REQUIRED)
find_package(zlib REQUIRED)
find_package(OpenSSL)

# install 3rd-party package
# list(REMOVE_ITEM OpenSSL_LIBRARIES dl)
# if(NOT APPLE)
#     set(lib_install_dest lib)
# else()
#     set(lib_install_dest Contents/MacOS)
# endif()
# install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/ DESTINATION ${lib_install_dest} )

# set output path
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_output_dirs_setup()

#============================================================================
# Internal compiler options
#============================================================================

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(UNIX)
    if(COMPILER_SUPPORTS_CXX11)
        if(APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
        else(APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif(APPLE)
    elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()

if(APPLE)
    if(APPSTORE_BUILD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-2")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gdwarf-2")
        add_definitions(-DBUILD4APPSTORE=TRUE)
    endif(APPSTORE_BUILD)
    if (PLCrashReporter)
        add_definitions(-DUsePLCrashReporter=TRUE)
    endif(PLCrashReporter)
    add_definitions(-DUSECOCOATOOLBAR=TRUE)
endif(APPLE)

# when including QuaZip sources directly into a project, QUAZIP_STATIC should
# be defined explicitly to avoid possible troubles with unnecessary importing/exporting.
if (WIN32)
    add_definitions(-DQUAZIP_STATIC)
endif()

#============================================================================
# Subfolders
#============================================================================

#add_subdirectory(lib)
add_subdirectory(src)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
