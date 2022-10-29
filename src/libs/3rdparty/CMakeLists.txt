set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Static linking")

# when including QuaZip sources directly into a project, QUAZIP_STATIC should
# be defined explicitly to avoid possible troubles with unnecessary importing/exporting.
if (WIN32)
    add_definitions(-DQUAZIP_STATIC)
endif()

add_subdirectory(framelesshelper)
add_subdirectory(quazip)