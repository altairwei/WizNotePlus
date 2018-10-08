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
set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk)

# WizNotePlus source directory.
set(WIZNOTE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# Build directory.
set(WIZNOTE_BUILD_DIR ${OUTSIDE_DIR}/build-WizNotePlus)
# Install or Package directory.
set(WIZNOTE_PACKAGE_DIR ${OUTSIDE_DIR}/package-WizNotePlus)
# install_prefix WizNote
set(WIZNOTE_INSTALL_PREFIX ${WIZNOTE_PACKAGE_DIR}/WizNote)
if(APPLE)
    set(WIZNOTE_INSTALL_PREFIX ${WIZNOTE_PACKAGE_DIR}/WizNote.app)
endif(APPLE)
# Package output directory
set(WIZNOTE_PACKAGE_OUTPUT_PATH ${OUTSIDE_DIR})

option(GENERATE_INSTALL_DIR "Decide whether install or not." ON)
option(GENERATE_APPIMAGE "Decide whether generate AppImage or not." ON)

message(
    "\nThe following variables must be confirmed:\n"
    "-- CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}\n"
    "-- WIZNOTE_SOURCE_DIR: ${WIZNOTE_SOURCE_DIR}\n"
    "-- WIZNOTE_BUILD_DIR: ${WIZNOTE_BUILD_DIR}\n"
    "-- WIZNOTE_PACKAGE_DIR: ${WIZNOTE_PACKAGE_DIR}\n"
    "-- WIZNOTE_INSTALL_PREFIX: ${WIZNOTE_INSTALL_PREFIX}\n"
    "-- GENERATE_INSTALL_DIR: ${GENERATE_INSTALL_DIR}\n"
    "-- GENERATE_APPIMAGE: ${GENERATE_APPIMAGE}\n"
)

#============================================================================
# Construct directory tree
#============================================================================

if(UNIX)
    if(APPLE)
        # MacOS platform
        file(MAKE_DIRECTORY 
            ${WIZNOTE_BUILD_DIR}
            ${WIZNOTE_PACKAGE_DIR}
            ${WIZNOTE_PACKAGE_DIR}/WizNote.app
            ${WIZNOTE_PACKAGE_DIR}/WizNote.app/Contents
            ${WIZNOTE_PACKAGE_DIR}/WizNote.app/Contents/Frameworks
        )
    else(APPLE)
        # Linux platform
        file(MAKE_DIRECTORY 
            ${WIZNOTE_BUILD_DIR}
            ${WIZNOTE_PACKAGE_DIR}
        )
    endif(APPLE)
elseif(WIN32)
    # Windows platform

else(UNIX)
    message(FATAL_ERROR "\nCan't detect which platform your are useing!")
endif(UNIX)

#============================================================================
# Generate thirdparty dependencies
#============================================================================

message("\nStart generate 3rdparty dependencies:\n")
execute_process(COMMAND conan install ${WIZNOTE_SOURCE_DIR}
    -s build_type=Release
    WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL "0")
    message(FATAL_ERROR "Fail to generate 3rdparty dependencies!")
endif()

#============================================================================
# Configure and generate WizNotePlus project
#============================================================================

if(UNIX)
    if(APPLE)
        # MacOS platform use Unix Makefiles generator
        message("\nStart configure and generate WizNotePlus project:\n")
        execute_process(COMMAND ${CMAKE_COMMAND}
                -DCMAKE_BUILD_TYPE=Release 
                -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                -DCMAKE_INSTALL_PREFIX=${WIZNOTE_INSTALL_PREFIX}
                -H${WIZNOTE_SOURCE_DIR} -B${WIZNOTE_BUILD_DIR}
                -G "Unix Makefiles"
                -UPDATE_TRANSLATIONS=YES
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL "0")
            message(FATAL_ERROR "Fail to generate WizNotePlus project!")
        endif()
    else(APPLE)
        # Linux platform use Unix Makefiles generator
        message("\nStart configure and generate WizNotePlus project:\n")
        execute_process(COMMAND ${CMAKE_COMMAND}
                -DCMAKE_BUILD_TYPE=Release 
                -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                -DCMAKE_INSTALL_PREFIX=${WIZNOTE_INSTALL_PREFIX}
                -H${WIZNOTE_SOURCE_DIR} -B${WIZNOTE_BUILD_DIR}
                -G "Unix Makefiles"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL "0")
            message(FATAL_ERROR "Fail to generate WizNotePlus project!")
        endif()
    endif(APPLE)
elseif(WIN32)
    # Windows platform use NMake Makefiles generator
    message("\nStart configure and generate WizNotePlus project:\n")
    execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release 
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DCMAKE_INSTALL_PREFIX=${WIZNOTE_INSTALL_PREFIX}
            -H${WIZNOTE_SOURCE_DIR} -B${WIZNOTE_BUILD_DIR}
            -G "NMake Makefiles"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL "0")
        message(FATAL_ERROR "Fail to generate WizNotePlus project!")
    endif()
else(UNIX)
    message(FATAL_ERROR "\nCan't detect which platform your are useing!")
endif(UNIX)

#============================================================================
# Build WizNotePlus project
#============================================================================

message("\nStart build WizNotePlus project:\n")
execute_process(COMMAND ${CMAKE_COMMAND} --build ${WIZNOTE_BUILD_DIR} --target all
    --config Release
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL "0")
    message(FATAL_ERROR "Fail to build WizNotePlus project!")
endif()

#============================================================================
# Install WizNotePlus project
#============================================================================

if(GENERATE_INSTALL_DIR)
    if(UNIX)
        # use Unix Makefiles generator
        message("\nStart install WizNotePlus project:\n")
        execute_process(COMMAND make install
            WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL "0")
            message(FATAL_ERROR "Fail to install WizNotePlus project!")
        endif()
    elseif(WIN32)
        # use NMake Makefiles generator
        message("\nStart install WizNotePlus project:\n")
        execute_process(COMMAND nmake install
            WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL "0")
            message(FATAL_ERROR "Fail to install WizNotePlus project!")
        endif()
    else()
        # unavailable platform
        message(FATAL_ERROR "\nCan't detect which platform your are useing!")
    endif()
endif()

#============================================================================
# Package resource files
#============================================================================

if(GENERATE_INSTALL_DIR)
    if(UNIX)
        if(APPLE)
            # MacOS platform

            # get build version
            execute_process(
                COMMAND git rev-list HEAD
                COMMAND wc -l
                COMMAND awk '{print $1}'
                WORKING_DIRECTORY ${WIZNOTE_SOURCE_DIR}
                OUTPUT_VARIABLE wiznote_build_version
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL "0")
                message(FATAL_ERROR "Fail to get build version!")
            endif()

            # replace version
            execute_process(COMMAND plutil 
                -replace CFBundleVersion
                -string ${wiznote_build_version}
                ${WIZNOTE_PACKAGE_DIR}/WizNote.app/Contents/Info.plist
                WORKING_DIRECTORY ${WIZNOTE_SOURCE_DIR}
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL "0")
                message(FATAL_ERROR "Fail to replace build version!")
            endif()

        else(APPLE)
            # Linux platform

            # qtwebengine_dictionaries
            file(COPY ${WIZNOTE_SOURCE_DIR}/share/qtwebengine_dictionaries 
                DESTINATION ${WIZNOTE_PACKAGE_DIR}/WizNote/bin)

            # copy WizNote logos
            option(CREATE_LOGO OFF)
            if(CREATE_LOGO)
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
            endif()

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
        endif(APPLE)
    elseif(WIN32)
        # Windows platform

    else(UNIX)
        # unavailable platform
        message(FATAL_ERROR "\nCan't detect which platform your are useing!")
    endif(UNIX)
endif(GENERATE_INSTALL_DIR)

#============================================================================
# Deploy Qt5 libraries and package WizNotePlus to an AppImage or dmg
#============================================================================

if(GENERATE_APPIMAGE)
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
endif()