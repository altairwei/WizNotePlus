project(libs)

option(WIZNOTE_GIT_SUBMODULE "Check submodules during build" ON)

find_package(ZLIB)
if (NOT ZLIB_FOUND)
    # Update submodules as needed
    # if(WIZNOTE_GIT_SUBMODULE)
    #     message("ZLIB not found, using submodule from github. ")
    #     message("Start download: ")
    #     execute_process(COMMAND git submodule update --init -- zlib
    #                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    #                 RESULT_VARIABLE GIT_SUBMOD_RESULT)
    #     if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    #         message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    #     endif()
    #     # build from source.
    #     add_subdirectory(zlib)

    #     set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
    #     set(ZLIB_LIBRARY zlib)
    # endif()
    
    # conan zlib package
    find_package(zlib REQUIRED)
    set(ZLIB_INCLUDE_DIR ${zlib_INCLUDE_DIRS})
    set(ZLIB_LIBRARY ${zlib_LIBRARIES})
endif()

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/lib/quazip/quazip/quazip.h)
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

#add_subdirectory(aggregation)
#add_subdirectory(extensionsystem)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/lib/gumbo-parser/src/gumbo.h)
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
        set(Gumbo_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/gumbo-parser/src/)
        add_subdirectory(gumbo-parser)
        set(Gumbo_static_LIBRARY Gumbo_static)
        set(Gumbo_LIBRARY Gumbo)
    endif()
else()
    set(Gumbo_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/gumbo-parser/src/)
    add_subdirectory(gumbo-parser)
    set(Gumbo_static_LIBRARY Gumbo_static)
    set(Gumbo_LIBRARY Gumbo)
endif()

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/lib/gumbo-query/src/Document.h)
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
