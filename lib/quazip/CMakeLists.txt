project(QuaZip)
cmake_minimum_required(VERSION 2.6)

# CMP0042: Explicitly acknowledge MACOSX_RPATH
# (introduced in CMake 2.8.12, enabled by default in CMake 3.0,
# and producing a warning when unset since 3.7.1)
cmake_policy(SET CMP0042 NEW)

option(BUILD_WITH_QT4 "Build QuaZip with Qt4 no matter if Qt5 was found" OFF)

if( NOT BUILD_WITH_QT4 )
    # try Qt5 first, and prefer that if found
    find_package(Qt5Core QUIET)
endif()

if(Qt5Core_FOUND)
    set(CMAKE_CXX_STANDARD 11)
    set(QTCORE_LIBRARIES ${Qt5Core_LIBRARIES})
    set(QUAZIP_LIB_VERSION_SUFFIX 5)
    # if there is no QT_ROOT, try to deduce it from Qt QtCore include
    if("${QT_ROOT}" STREQUAL "")
        set(QT_ROOT ${QT_QTCORE_INCLUDE_DIR}/../..)
    endif()
    include_directories(${Qt5Core_INCLUDE_DIRS})

    macro(qt_wrap_cpp)
        qt5_wrap_cpp(${ARGN})
    endmacro()
else()
    set(qt_min_version "4.5.0")
    find_package(Qt4 REQUIRED)
    set(QT_USE_QTGUI false)
    include(${QT_USE_FILE})
    include_directories(${QT_INCLUDES})
    set(QTCORE_LIBRARIES ${QT_QTCORE_LIBRARY})

    macro(qt_wrap_cpp)
        qt4_wrap_cpp(${ARGN})
    endmacro()
endif()

# Use system zlib on unix and Qt ZLIB on Windows
if(UNIX OR MINGW)
    find_package(ZLIB REQUIRED)
else(UNIX OR MINGW)
    set(ZLIB_INCLUDE_DIRS "${QT_ROOT}/src/3rdparty/zlib" CACHE STRING "Path to ZLIB headers of Qt")
    set(ZLIB_LIBRARIES "")
    if(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
        message("Please specify a valid zlib include dir")
    endif(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
endif(UNIX OR MINGW)

# All build libraries are moved to this directory
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name" FORCE)
set(QUAZIP_LIB_TARGET_NAME quazip${QUAZIP_LIB_VERSION_SUFFIX} CACHE
    INTERNAL "Target name of libquazip" FORCE)

#add_subdirectory(quazip)

#install(FILES FindQuaZip.cmake RENAME FindQuaZip${QUAZIP_LIB_VERSION_SUFFIX}.cmake DESTINATION ${CMAKE_ROOT}/Modules)

# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIRS}
)

file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

qt_wrap_cpp(MOC_SRCS ${PUBLIC_HEADERS})
set(SRCS ${SRCS} ${MOC_SRCS})

add_library(${QUAZIP_LIB_TARGET_NAME} SHARED ${SRCS})
add_library(quazip_static STATIC ${SRCS})
 
# Windows uses .lib extension for both static and shared library
# *nix systems use different extensions for SHARED and STATIC library and by convention both libraries have the same name
if (NOT WIN32)
	set_target_properties(quazip_static PROPERTIES OUTPUT_NAME quazip${QUAZIP_LIB_VERSION_SUFFIX})
endif ()

set_target_properties(${QUAZIP_LIB_TARGET_NAME} quazip_static PROPERTIES VERSION 1.0.0 SOVERSION 1 DEBUG_POSTFIX d)
# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty)
target_link_libraries(${QUAZIP_LIB_TARGET_NAME} ${QT_QTMAIN_LIBRARY} ${QTCORE_LIBRARIES} ${ZLIB_LIBRARIES})
target_link_libraries(quazip_static ${QT_QTMAIN_LIBRARY} ${QTCORE_LIBRARIES} ${ZLIB_LIBRARIES})

#install(FILES ${PUBLIC_HEADERS} DESTINATION include/quazip${QUAZIP_LIB_VERSION_SUFFIX})
#install(TARGETS ${QUAZIP_LIB_TARGET_NAME} quazip_static LIBRARY DESTINATION ${LIB_DESTINATION} ARCHIVE DESTINATION ${LIB_DESTINATION} RUNTIME DESTINATION ${LIB_DESTINATION})
