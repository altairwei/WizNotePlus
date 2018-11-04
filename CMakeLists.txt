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

project(WizNotePlus VERSION 2.7.0)

#============================================================================
# 3rd-party libraries
#============================================================================

option(WIZNOTE_CONAN_LIBS "If enable conan packages" ON)
# install conan 3rdparty libraries to package dirs.
if(WIZNOTE_CONAN_LIBS)
    message("\nStart generate 3rdparty dependencies at ${CMAKE_BINARY_DIR}:\n")
    execute_process(COMMAND conan install ${CMAKE_SOURCE_DIR}
        -s build_type=${CMAKE_BUILD_TYPE}
        --build missing
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE conan_result
        OUTPUT_VARIABLE conan_output
        ERROR_VARIABLE conan_error
    )
    if(NOT DEFINED conan_result)
        message(FATAL_ERROR
            "Conan cannot be found, please ensure the location of excutable conan is in your PATH."
            "-- Exit Code: ${conan_result}."
        )
    endif()
    if(NOT conan_result EQUAL "0")
        message("${conan_error}")
        message("${conan_output}")
        message(FATAL_ERROR
            "\nFail to generate 3rdparty dependencies! "
            "\n-- Exit code: ${conan_result}"
            "\n-- Build type: ${CMAKE_BUILD_TYPE} \n")
    endif()

    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()

    install(DIRECTORY ${CONAN_LIB_DIRS} DESTINATION lib )
endif(WIZNOTE_CONAN_LIBS)

#============================================================================
# Set Variables
#============================================================================

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

# rpath resolve.
#set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)     // would cause wheelEvent problem on centos/fedora when compiled with qt5
if(APPLE)
    # make the install names for dylibs include "@rpath/" prefix.
    set(MACOSX_RPATH ON)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # used to initialize the INSTALL_NAME_DIR property on all targets.
    set(CMAKE_INSTALL_NAME_DIR "@rpath/PlugIns")
    set(CMAKE_INSTALL_RPATH "@loader_path/..;@loader_path/../Frameworks;@executable_path/..;@executable_path/../Frameworks;@rpath")
    #SET(PLCrashReporter on)
else(APPLE)
    # $ORIGING代表程序目录，CMAKE_INSTALL_RPATH是分号分隔的list，它指定了在安装目标时使用的rpath
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/..")
    set (openssl-linked on)
endif(APPLE)

# 缺省构建类型Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

#============================================================================
# Internal compiler options
#============================================================================

# MSVC don't need this check process.
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

# WizNotePlus Dirs
if(APPLE)
    set(WIZNOTE_ROOT_DIR ${CMAKE_BINARY_DIR}/bin/WizNote.app)
    set(WIZNOTE_CONTENTS_DIR ${WIZNOTE_ROOT_DIR}/Contents)
    set(WIZNOTE_RESOURCE_DIR ${WIZNOTE_ROOT_DIR}/Contents/Resources)
else(APPLE)
    set(WIZNOTE_ROOT_DIR ${CMAKE_BINARY_DIR}) # 根目录设置为CMake构建树的顶层路径
    set(WIZNOTE_RESOURCE_DIR ${WIZNOTE_ROOT_DIR}/share) # 资源文件目录
endif(APPLE)

# when including QuaZip sources directly into a project, QUAZIP_STATIC should
# be defined explicitly to avoid possible troubles with unnecessary importing/exporting.
if (WIN32)
    add_definitions(-DQUAZIP_STATIC)
endif()

#============================================================================
# 执行后续命令
#============================================================================

add_subdirectory(lib)
add_subdirectory(src)
