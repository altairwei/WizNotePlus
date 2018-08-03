project(libs)

find_package(ZLIB)
if(NOT ZLIB_FOUND)
    add_subdirectory(zlib)
    message("ZLIB not found, using internal: ${zlib_SOURCE_DIR}")
    set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
    set(ZLIB_LIBRARY zlib)
endif(NOT ZLIB_FOUND)

#add_subdirectory(cryptopp)
#add_subdirectory(cryptopp562) 
add_subdirectory(cryptopp) #用支持CMake的v5.6.4替代旧版本
add_subdirectory(quazip)
#add_subdirectory(aggregation)
#add_subdirectory(extensionsystem)
