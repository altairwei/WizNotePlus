#!/usr/bin/cmake -P
# Program:
#   Package WizNotePlus and it's dependencies into a portable AppImage.

cmake_minimum_required(VERSION 3.5)

#============================================================================
# Options and Settings
#============================================================================

get_filename_component(OUTSIDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# Your Qt5 libraries path.
set(CMAKE_PREFIX_PATH /home/altairwei/usr/Qt5.11.1/5.11.1/gcc_64)

# WizNotePlus source directory.
set(WIZNOTE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Build directory.
set(WIZNOTE_BUILD_DIR ${OUTSIDE_DIR}/build-WizNotePlus)

# Install or Package directory.
set(WIZNOTE_PACKAGE_DIR ${OUTSIDE_DIR}/package-WizNotePlus)

# install_prefix WizNote
set(WIZNOTE_INSTALL_PREFIX ${WIZNOTE_PACKAGE_DIR}/WizNote)

message(
    "\nThe following variables must be confirmed:\n"
    "-- CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}\n"
    "-- WIZNOTE_SOURCE_DIR: ${WIZNOTE_SOURCE_DIR}\n"
    "-- WIZNOTE_BUILD_DIR: ${WIZNOTE_BUILD_DIR}\n"
    "-- WIZNOTE_PACKAGE_DIR: ${WIZNOTE_PACKAGE_DIR}\n"
    "-- WIZNOTE_INSTALL_PREFIX: ${WIZNOTE_INSTALL_PREFIX}\n"
)

#============================================================================
# Construct directory tree
#============================================================================

file(MAKE_DIRECTORY 
    ${WIZNOTE_BUILD_DIR}
    ${WIZNOTE_PACKAGE_DIR}
)

#============================================================================
# Configure and generate WizNotePlus project
#============================================================================

message("\nStart configure and generate WizNotePlus project:\n")
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release 
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_INSTALL_PREFIX=${WIZNOTE_INSTALL_PREFIX}
        -H${WIZNOTE_SOURCE_DIR} -B${WIZNOTE_BUILD_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL "0")
    message(FATAL_ERROR "Fail to generate WizNotePlus project!")
endif()

#============================================================================
# Build WizNotePlus project
#============================================================================

message("\nStart build WizNotePlus project:\n")
execute_process(COMMAND ${CMAKE_COMMAND} --build ${WIZNOTE_BUILD_DIR} --target all
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL "0")
    message(FATAL_ERROR "Fail to build WizNotePlus project!")
endif()

#============================================================================
# Install WizNotePlus project
#============================================================================

# TODO: 需要根据不同平台指定不同的CMAKE_GENERATOR
if(UNIX)
    message("\nStart install WizNotePlus project:\n")
    execute_process(COMMAND make install
        WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL "0")
        message(FATAL_ERROR "Fail to install WizNotePlus project!")
    endif()
endif()

#============================================================================
# Package resource files
#============================================================================

# qtwebengine_dictionaries
file(COPY ${WIZNOTE_SOURCE_DIR}/share/qtwebengine_dictionaries 
    DESTINATION ${WIZNOTE_PACKAGE_DIR}/WizNote/bin)

# copy WizNote logos
file(MAKE_DIRECTORY 
    ${WIZNOTE_PACKAGE_DIR}/logo
    ${WIZNOTE_PACKAGE_DIR}/logo/hicolor
)
foreach(ICON_SIZE 16 32 64 128 256 512)
    set(icon_dir ${WIZNOTE_PACKAGE_DIR}/logo/hicolor/${ICON_SIZE}x${ICON_SIZE})
    file(MAKE_DIRECTORY ${icon_dir})
    file(COPY ${WIZNOTE_SOURCE_DIR}/build/common/logo/wiznote${ICON_SIZE}.png
        DESTINATION ${icon_dir}
    )
    file(RENAME ${icon_dir}/wiznote${ICON_SIZE}.png ${icon_dir}/wiznote.png)
endforeach(ICON_SIZE)

# copy desktop file
file(COPY ${WIZNOTE_SOURCE_DIR}/build/common/wiznote2.desktop
    DESTINATION ${WIZNOTE_PACKAGE_DIR})
file(RENAME ${WIZNOTE_PACKAGE_DIR}/wiznote2.desktop 
    ${WIZNOTE_PACKAGE_DIR}/wiznote.desktop)

# copy fcitx-qt5 library
find_file (fcitx-qt5-lib libfcitxplatforminputcontextplugin.so 
    /usr/lib/x86_64-linux-gnu/qt5/plugins/platforminputcontexts/ 
)
if(NOT fcitx-qt5-lib)
    message(FATAL_ERROR "Fail to find fcitx-qt5 !")
endif()

file(MAKE_DIRECTORY 
    ${WIZNOTE_PACKAGE_DIR}/WizNote/plugins/platforminputcontexts
)
file(COPY ${fcitx-qt5-lib} 
    DESTINATION ${WIZNOTE_PACKAGE_DIR}/WizNote/plugins/platforminputcontexts)

# TODO: 如何处理 OpenSSL1.02 的问题？
# 因为还有Windows平台，所以不能直接复制libssl.so了事，应该从头构建。

#============================================================================
# Deploy Qt5 libraries and package WizNotePlus to an AppImage
#============================================================================

message("\nStart package WizNotePlus project:\n")
execute_process(COMMAND ${WIZNOTE_SOURCE_DIR}/external/linuxdeployqt
    ${WIZNOTE_PACKAGE_DIR}/WizNote/share/applications/wiznote.desktop
    -verbose=1 -appimage -qmake=${CMAKE_PREFIX_PATH}/bin/qmake
    WORKING_DIRECTORY ${OUTSIDE_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL "0")
    message(FATAL_ERROR "Fail to package WizNotePlus project!")
endif()