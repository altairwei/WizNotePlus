project(libs)

option(WIZNOTE_GIT_SUBMODULE "Check submodules during build" ON)

find_package(ZLIB)
if (NOT ZLIB_FOUND)
    # Update submodules as needed
    if(WIZNOTE_GIT_SUBMODULE)
        message("ZLIB not found, using submodule from github. ")
        message("Start download: ")
        execute_process(COMMAND git submodule update --init -- zlib
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
        # build from source.
        add_subdirectory(zlib)

        set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
        set(ZLIB_LIBRARY zlib)
    endif()
endif()


find_path(QuaZip_INCLUDE_DIR "quazip/quazip.h" "${CMAKE_CURRENT_SOURCE_DIR}/quazip")
if (NOT QuaZip_INCLUDE_DIR)
    # Update submodules as needed
    if(WIZNOTE_GIT_SUBMODULE)
        message("QuaZip not found, it needs to be downloaded from github and build from source.")
        message("Start download: ")
        execute_process(COMMAND git submodule update --init -- quazip
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
        # build from source.
        add_subdirectory(quazip)
    endif()
else()
    add_subdirectory(quazip)
endif()

find_path(cryptopp_INCLUDE_DIR "cryptlib.h" "${CMAKE_CURRENT_SOURCE_DIR}/cryptopp")
if (NOT cryptopp_INCLUDE_DIR)
    # Update submodules as needed
    if(WIZNOTE_GIT_SUBMODULE)
        message("CryptoPP not found, it needs to be downloaded from github and build from source.")
        message("Start download: ")
        execute_process(COMMAND git submodule update --init -- cryptopp
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		    RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
        # build from source.
        add_subdirectory(cryptopp)
    endif()
else()
    add_subdirectory(cryptopp)
endif()

#add_subdirectory(aggregation)
#add_subdirectory(extensionsystem)

find_package(Gumbo QUIET)
find_path(Gumbo_INCLUDE_DIR "gumbo.h" PATHS "${CMAKE_CURRENT_SOURCE_DIR}/gumbo-parser/src")
if ( (NOT Gumbo_LIBRARY) AND (NOT Gumbo_INCLUDE_DIR) )
    # Update submodules as needed
    if(WIZNOTE_GIT_SUBMODULE)
        message("Gumbo not found, it needs to be downloaded from github and build from source.")
        message("Start download: ")
        execute_process(COMMAND git submodule update --init -- gumbo-parser
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		    RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
        # build from source.
        add_subdirectory(gumbo-parser)
        set(Gumbo_static_LIBRARY Gumbo_static)
        set(Gumbo_LIBRARY Gumbo)
    endif()
elseif( (NOT Gumbo_LIBRARY) AND Gumbo_INCLUDE_DIR )
    add_subdirectory(gumbo-parser)
    set(Gumbo_static_LIBRARY Gumbo_static)
    set(Gumbo_LIBRARY Gumbo)
endif()

find_path(GumboQuery_INCLUDE_DIR "Document.h" "${CMAKE_CURRENT_SOURCE_DIR}/gumbo-query/src")
if (NOT GumboQuery_INCLUDE_DIR)
    # Update submodules as needed
    if(WIZNOTE_GIT_SUBMODULE)
        message("GumboQuery not found, it needs to be downloaded from github and build from source.")
        message("Start download: ")
        execute_process(COMMAND git submodule update --init -- gumbo-query
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		    RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
        # build from source.
        add_subdirectory(gumbo-query)
    endif()
else()
    add_subdirectory(gumbo-query)
endif()
