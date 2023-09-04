set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Static linking")

# when including QuaZip sources directly into a project, QUAZIP_STATIC should
# be defined explicitly to avoid possible troubles with unnecessary importing/exporting.
if (WIN32)
    add_definitions(-DQUAZIP_STATIC)
endif()

add_subdirectory(framelesshelper)
add_subdirectory(quazip)

# Build gumbo-parser
file(GLOB GUMBO_SOURCES gumbo-parser/src/*.c)
add_library(gumbo ${GUMBO_SOURCES})
if(WIN32)
	target_include_directories(gumbo PRIVATE "gumbo-parser/visualc/include")
endif()

#because gumbo is a pure C 99 library
set_property(TARGET gumbo PROPERTY C_STANDARD 99)
