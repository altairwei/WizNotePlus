#!/usr/bin/cmake -P

# Program:
#   Package WizNotePlus and it's dependencies into a portable AppImage or MacOS dmg.

cmake_minimum_required(VERSION 3.5)

#============================================================================
# Options and Settings 选项和设置
#============================================================================

message("\nThe following variables must be confirmed:\n")

get_filename_component(OUTSIDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# Your Qt5 libraries path. 你的Qt5库位置
if(NOT QTDIR)
    # check default Qt library
    execute_process(COMMAND qmake -query QT_INSTALL_PREFIX
        OUTPUT_VARIABLE qt_dir
        RESULT_VARIABLE result
    )
    if( (NOT result EQUAL "0") OR (NOT qt_dir) )
        message(FATAL_ERROR "\nQTDIR is not valid, Qt5 library cannot be found !\nPlease define QTDIR!")
    endif()
    set(QTDIR ${qt_dir})
else()
    # check user defined Qt library
    find_file(qmake_file "bin/qmake" ${QTDIR})
    execute_process(COMMAND ${qmake_file} -query QT_INSTALL_PREFIX
        OUTPUT_VARIABLE qt_dir
        RESULT_VARIABLE result
    )
    if( (NOT result EQUAL "0") OR (NOT qt_dir) )
        message(FATAL_ERROR "\nQTDIR is not valid, Qt5 library cannot be found !\nPlease define QTDIR!")
    else()
        set(QTDIR ${qt_dir})
    endif()
endif()
string(STRIP ${QTDIR} QTDIR)

if(NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${QTDIR})
endif()
message(STATUS "Using CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# MacOSX.SDK
if(APPLE)
    if(NOT CMAKE_OSX_SYSROOT)
        execute_process(COMMAND xcodebuild -version -sdk macosx Path
            OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        find_path(osx_sysroot "SDKSettings.plist" ${CMAKE_OSX_SYSROOT})
        if(NOT osx_sysroot)
            message(FATAL_ERROR "CMAKE_OSX_SYSROOT is not valid, macOS SDK cannot be found !\nPlease define CMAKE_OSX_SYSROOT!")
        endif(NOT osx_sysroot)
    endif(NOT CMAKE_OSX_SYSROOT)
    message (STATUS "Using CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT}")
endif(APPLE)

# WizNotePlus source directory. 项目源代码位置
if(NOT WIZNOTE_SOURCE_DIR)
    set(WIZNOTE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()
message(STATUS "Using WIZNOTE_SOURCE_DIR: ${WIZNOTE_SOURCE_DIR}")

# Build directory. 项目构建位置
if(NOT WIZNOTE_BUILD_DIR)
    set(WIZNOTE_BUILD_DIR ${OUTSIDE_DIR}/build-WizNotePlus)
endif()
message(STATUS "Using WIZNOTE_BUILD_DIR: ${WIZNOTE_BUILD_DIR}")

# Install or Package directory. 项目打包程序工作目录
if(NOT WIZNOTE_PACKAGE_DIR)
    set(WIZNOTE_PACKAGE_DIR ${OUTSIDE_DIR}/package-WizNotePlus)
endif()
message(STATUS "Using WIZNOTE_PACKAGE_DIR: ${WIZNOTE_PACKAGE_DIR}")

# install_prefix WizNote. 项目安装位置
if(NOT WIZNOTE_INSTALL_PREFIX)
    set(WIZNOTE_INSTALL_PREFIX ${WIZNOTE_PACKAGE_DIR}/WizNote)
    if(APPLE)
        set(WIZNOTE_INSTALL_PREFIX ${WIZNOTE_PACKAGE_DIR}/WizNote.app)
    endif(APPLE)
endif()
message(STATUS "Using WIZNOTE_INSTALL_PREFIX: ${WIZNOTE_INSTALL_PREFIX}")

# auto detect proccess number
if(NOT CMAKE_BUILD_PARALLEL_LEVEL)
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
    endif()
endif()
message(STATUS "Using CMAKE_BUILD_PARALLEL_LEVEL: ${CMAKE_BUILD_PARALLEL_LEVEL}")

# Package output directory. 打包结果输出位置
if(NOT WIZNOTE_PACKAGE_OUTPUT_PATH)
    set(WIZNOTE_PACKAGE_OUTPUT_PATH ${OUTSIDE_DIR})
endif()

if(NOT GENERATE_INSTALL_DIR)
    option(GENERATE_INSTALL_DIR "Decide whether install or not." ON)
endif()
message(STATUS "GENERATE_INSTALL_DIR: ${GENERATE_INSTALL_DIR}")

if(NOT GENERATE_PACKAGE)
    option(GENERATE_PACKAGE "Decide whether generate AppImage or not." ON)
endif()
message(STATUS "GENERATE_PACKAGE: ${GENERATE_PACKAGE}")

if(NOT USE_FCITX)
    option(USE_FCITX "Decide whether use fcitx-qt5 or not." ON)
endif()
message(STATUS "USE_FCITX: ${USE_FCITX}")

if(NOT VERBOSE_LEVEL)
    set(VERBOSE_LEVEL 0)
endif()

if(NOT CMAKE_VERBOSE_MAKEFILE)
    set(CMAKE_VERBOSE_MAKEFILE FALSE)
endif()

# Extract WizNotePlus Version
file(STRINGS ${WIZNOTE_SOURCE_DIR}/CMakeLists.txt project_command_str 
    ENCODING UTF-8
    REGEX "project\\(WizNotePlus VERSION (.*)\\)"
)
string(REGEX MATCH "([0-9]+)\.([0-9]+)\.([0-9]+)" WIZNOTEPLUS_VERSION ${project_command_str})

#============================================================================
# Construct directory tree 构建目录树
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
    #TODO: Windows platform

else(UNIX)
    message(FATAL_ERROR "\nCan't detect which platform your are useing!")
endif(UNIX)

#============================================================================
# Configure and generate WizNotePlus project 生成 WizNotePlus CMake 项目
#============================================================================

if(UNIX)
    if(APPLE)
        # MacOS platform use Unix Makefiles generator.
        message("\nStart configure and generate WizNotePlus project:\n")
        execute_process(COMMAND ${CMAKE_COMMAND}
                -DCMAKE_BUILD_TYPE=Release 
                -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                -DCMAKE_INSTALL_PREFIX=${WIZNOTE_INSTALL_PREFIX}
                -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
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
                -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
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
    # Windows platform use NMake Makefiles JOM generator
    message("\nStart configure and generate WizNotePlus project:\n")
    execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release 
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DCMAKE_INSTALL_PREFIX=${WIZNOTE_INSTALL_PREFIX}
            -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
            -H${WIZNOTE_SOURCE_DIR} -B${WIZNOTE_BUILD_DIR}
            -G "NMake Makefiles JOM"
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
# Build WizNotePlus project 调用本地工具构建项目
#============================================================================

message("\nStart build WizNotePlus project:\n")
execute_process(COMMAND ${CMAKE_COMMAND} --build ${WIZNOTE_BUILD_DIR} --target all
    --config Release
    --
    -j ${CMAKE_BUILD_PARALLEL_LEVEL}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL "0")
    message(FATAL_ERROR "Fail to build WizNotePlus project!")
endif()

#============================================================================
# Install WizNotePlus project 将项目安装到打包位置
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
# Package resource files 打包相关文件
#============================================================================

if(GENERATE_INSTALL_DIR)
    if(UNIX)
        if(APPLE)
            # MacOS platform

            # copy files
            file(COPY ${WIZNOTE_BUILD_DIR}/bin/WizNote.app
                DESTINATION ${WIZNOTE_PACKAGE_DIR}
            )

            # get build version
            execute_process(
                COMMAND git rev-list HEAD
                COMMAND wc -l
                COMMAND awk "{print $1}"
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
                message(FATAL_ERROR "Fail to replace build version to Info.plist!")
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
            if(USE_FCITX)
                set(fcitx_lib_paths
                    /usr/lib/x86_64-linux-gnu/qt5/plugins/platforminputcontexts/
                    /usr/lib64/qt5/plugins/platforminputcontexts/
                    /usr/lib/qt/plugins/platforminputcontexts/
                )
                find_file(fcitx_qt5_lib libfcitxplatforminputcontextplugin.so ${fcitx_lib_paths})
                find_file(fcitx5_qt5_lib libfcitx5platforminputcontextplugin.so ${fcitx_lib_paths})

                if(NOT fcitx_qt5_lib AND NOT fcitx5_qt5_lib)
                    if(FCITX_QT5_LIB)
                        set(fcitx_qt5_lib ${FCITX_QT5_LIB})
                    elseif(FCITX5_QT5_LIB)
                        set(fcitx5_qt5_lib ${FCITX5_QT5_LIB})
                    else()
                        message(FATAL_ERROR "Fail to find fcitx-qt5 and fcitx5-qt5!"
                        " Please specify the path of fcitx-qt5 or fcitx5-qt5 library"
                        " with -DFCITX_QT5_LIB=<path/to/libfcitxplatforminputcontextplugin.so>"
                        " or -DFCITX5_QT5_LIB=<path/to/libfcitx5platforminputcontextplugin.so>"
                        " respectively")
                    endif()
                endif()

                file(MAKE_DIRECTORY 
                    ${WIZNOTE_PACKAGE_DIR}/WizNote/plugins/platforminputcontexts
                )
                if(fcitx_qt5_lib)
                    file(COPY ${fcitx_qt5_lib} 
                        DESTINATION ${WIZNOTE_PACKAGE_DIR}/WizNote/plugins/platforminputcontexts)
                endif()
                if(fcitx5_qt5_lib)
                    file(COPY ${fcitx5_qt5_lib}
                        DESTINATION ${WIZNOTE_PACKAGE_DIR}/WizNote/plugins/platforminputcontexts)
                endif()
                
            endif(USE_FCITX)
        endif(APPLE)
    elseif(WIN32)
        # Windows platform

    else(UNIX)
        # unavailable platform
        message(FATAL_ERROR "\nCan't detect which platform your are useing!")
    endif(UNIX)
endif(GENERATE_INSTALL_DIR)

#============================================================================
# Deploy Qt5 libraries and package WizNotePlus 部署Qt5并生成包
#============================================================================

if(GENERATE_INSTALL_DIR AND GENERATE_PACKAGE)
    if(UNIX)
        if(APPLE)
            # MacOS platform
            find_file(create_dmg "create-dmg" "${WIZNOTE_SOURCE_DIR}/external/create-dmg")
            if (NOT create_dmg)
                message(STATUS "Downloading dmg package tool...")
                execute_process(COMMAND git submodule update --init -- ${WIZNOTE_SOURCE_DIR}/external/create-dmg
                            WORKING_DIRECTORY ${WIZNOTE_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_RESULT)
                if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                    message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
                endif()
                set(create_dmg "${WIZNOTE_SOURCE_DIR}/external/create-dmg/create-dmg")
            endif()

            # change Info.plist version.
            file(READ ${WIZNOTE_INSTALL_PREFIX}/Contents/Info.plist info_plist ENCODING UTF-8)
            string(REGEX REPLACE "<string>2.7.0</string>" "<string>${WIZNOTEPLUS_VERSION}</string>" info_plist "${info_plist}")
            file(WRITE ${WIZNOTE_INSTALL_PREFIX}/Contents/Info.plist "${info_plist}")

            # deploy 3rdpaty libraries
            message("\nStart deploy WizNotePlus project:\n")
            execute_process(COMMAND ${QTDIR}/bin/macdeployqt
                ${WIZNOTE_INSTALL_PREFIX} -verbose=${VERBOSE_LEVEL}
                -executable=${WIZNOTE_INSTALL_PREFIX}/Contents/MacOS/WizNote
                -libpath=${QTDIR}
                WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
                RESULT_VARIABLE macdeployqt_result
            )
            if(NOT macdeployqt_result EQUAL "0")
                message(FATAL_ERROR
                    "\nFail to package WizNotePlus project!"
                    "\n-- Command: ${QTDIR}/bin/macdeployqt"
                    "\n-- Exit Code: ${macdeployqt_result}"
                )
            endif()
            execute_process(COMMAND python ${WIZNOTE_SOURCE_DIR}/external/macdeployqtfix.py
                ${WIZNOTE_INSTALL_PREFIX}/Contents/MacOS/WizNote ${QTDIR}
                WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
                RESULT_VARIABLE macdeployqtfix_result
            )
            if(NOT macdeployqtfix_result EQUAL "0")
                message(FATAL_ERROR
                    "\nFail to package WizNotePlus project!"
                    "\n-- Command: python ${WIZNOTE_SOURCE_DIR}/external/macdeployqtfix.py"
                    "\n-- Exit Code: ${macdeployqtfix_result}"
                )
            endif()
            #FIXME: Should not add rpath by hand!
            execute_process(COMMAND install_name_tool
                -add_rpath "@executable_path/../Frameworks"
                ${WIZNOTE_INSTALL_PREFIX}/Contents/MacOS/WizNote
                WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
                RESULT_VARIABLE rpath_result
            )
            if(NOT rpath_result EQUAL "0")
                message(FATAL_ERROR
                    "\nFail to package WizNotePlus project!"
                    "\n-- Command: install_name_tool"
                    "\n-- Exit Code: ${rpath_result}"
                )
            endif()

            # sign code
            #set(APPLCERT "Developer ID Application: Beijing Wozhi Technology Co. Ltd (KCS8N3QJ92)")
            #execute_process(COMMAND codesign --verbose=2 --deep --sign "${APPLCERT}" ${WIZNOTE_INSTALL_PREFIX}
            #    WORKING_DIRECTORY ${WIZNOTE_BUILD_DIR}
            #    RESULT_VARIABLE result
            #)
            #if(NOT result EQUAL "0")
            #    message(FATAL_ERROR "Fail to sign code!")
            #endif()

            # change Package format
            set(package_home "${WIZNOTE_SOURCE_DIR}/macos-package")
            set(package_output_path ${WIZNOTE_PACKAGE_OUTPUT_PATH})
            set(volumn_name "wiznote-disk")
            set(volumn_path "/Volumes/${volumn_name}")

            # create dmg
            file(REMOVE
                ${package_output_path}/Wiznote-macOS.dmg
            )
            execute_process(COMMAND ${create_dmg}
                --volname ${volumn_name}
                --background ${WIZNOTE_SOURCE_DIR}/resources/wiznote-disk-cover.jpg
                --window-pos 200 120
                --window-size 522 350
                --icon-size 100
                --icon "WizNote.app" 100 190
                --hide-extension "WizNote.app"
                --app-drop-link 400 190
                --format UDZO
                ${package_output_path}/WizNotePlus-mac-v${WIZNOTEPLUS_VERSION}.dmg
                ${WIZNOTE_PACKAGE_DIR}/
                WORKING_DIRECTORY ${WIZNOTE_SOURCE_DIR}
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL "0")
                message(FATAL_ERROR "Fail to execute command:"
                "${WIZNOTE_SOURCE_DIR}/external/create-dmg")
            endif()

        else(APPLE)
            # Linux platform
            message("\nStart package WizNotePlus project:\n")
            execute_process(COMMAND ${WIZNOTE_SOURCE_DIR}/external/linuxdeployqt
                ${WIZNOTE_PACKAGE_DIR}/WizNote/share/applications/wiznote.desktop
                -verbose=${VERBOSE_LEVEL} -appimage -qmake=${qmake_file}
                WORKING_DIRECTORY ${OUTSIDE_DIR}
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL "0")
                message(FATAL_ERROR "Fail to package WizNotePlus project!")
            endif()
            # Rename AppImage with VERSION.
            file(GLOB wiznote_appimage_files
                LIST_DIRECTORIES false
                ${OUTSIDE_DIR}/WizNote*.AppImage)
            list(GET wiznote_appimage_files 0 wiznote_appimage_file)
            string(REGEX REPLACE "WizNote\\-(.*)\\.AppImage"
                "WizNotePlus-linux-\\1-v${WIZNOTEPLUS_VERSION}.AppImage" new_appimage_filename
                ${wiznote_appimage_file})
            file(RENAME ${wiznote_appimage_file} ${new_appimage_filename})
        endif(APPLE)
    elseif(WIN32)
        # Windows platform

    else(UNIX)
        message(FATAL_ERROR "\nCan't detect which platform your are useing!")
    endif(UNIX)
endif(GENERATE_INSTALL_DIR AND GENERATE_PACKAGE)

