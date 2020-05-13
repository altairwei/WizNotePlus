find_package( Qt5Core CONFIG REQUIRED )
find_package( Qt5Widgets CONFIG REQUIRED )
find_package( Qt5LinguistTools CONFIG REQUIRED )
find_package( Qt5Xml CONFIG REQUIRED )
find_package( Qt5Network CONFIG REQUIRED )
find_package( Qt5PrintSupport CONFIG REQUIRED )
find_package( Qt5WebEngine CONFIG REQUIRED )
find_package( Qt5WebEngineWidgets CONFIG REQUIRED )
find_package( Qt5WebSockets CONFIG REQUIRED )
find_package( Qt5WebChannel CONFIG REQUIRED )
find_package( Qt5Svg CONFIG REQUIRED )

if(APPLE)
    find_package( Qt5MacExtras REQUIRED )
endif(APPLE)

get_filename_component(qt_binary_dir ${Qt5_DIR}/../../../bin/ ABSOLUTE)
get_filename_component(qt_translations_dir ${Qt5_DIR}/../../../translations/ ABSOLUTE)

set(QT_VERSION ${Qt5_VERSION})

set(CMAKE_AUTOMOC ON)

macro(qt_add_translation)
    set(_files)
    foreach(_file ${ARGV})
        if(NOT ${_file} STREQUAL ${ARGV0})
            list(APPEND _files ${_file})
        endif()
    endforeach()

    qt5_add_translation(${ARGV0} ${_files})
endmacro()

macro(qt_add_ui)
    set(_files)
    foreach(_file ${ARGV})
        if(NOT ${_file} STREQUAL ${ARGV0})
            list(APPEND _files ${_file})
        endif()
    endforeach()

    qt5_wrap_ui(${ARGV0} ${_files})
endmacro()

macro(qt_add_resources)
    set(_files)
    foreach(_file ${ARGV})
        if(NOT ${_file} STREQUAL ${ARGV0})
            list(APPEND _files ${_file})
        endif()
    endforeach()
    message(STATUS ${_files})

    qt5_add_resources(${ARGV0} ${_files})
endmacro()

macro(qt_use_modules)
    target_link_libraries( ${ARGV0} 
        Qt5::Core 
        Qt5::Gui 
        Qt5::Widgets 
        Qt5::Xml 
        Qt5::Network 
        Qt5::PrintSupport 
        Qt5::WebEngine 
        Qt5::WebEngineWidgets 
        Qt5::WebSockets 
        Qt5::WebChannel 
        Qt5::Svg
    )
    if(APPLE)
        target_link_libraries( ${ARGV0} Qt5::MacExtras)
    elseif(WIN32)
        target_link_libraries( ${ARGV0} Qt5::WinMain)
    endif(APPLE)
endmacro()

macro(qt_suppress_warnings)
    if(APPLE)
        set_target_properties(${ARGV0} PROPERTIES
            COMPILE_FLAGS "-Wno-#warnings")
    endif(APPLE)
endmacro()
