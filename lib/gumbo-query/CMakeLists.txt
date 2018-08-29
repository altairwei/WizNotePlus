project(gumbo_query CXX)
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

include(CTest)
include(cmake/PreventInSourceBuilds.cmake)

# Debug build.
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)

# Set our output directories.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create library directory if it does not exist.
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Add the cmake modules directory to our module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
add_subdirectory(gumbo-parser)
find_package(Gumbo REQUIRED)

# Set our library source directory.
set(gumbo_query_SOURCE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)

# Add source subdirectories.
add_subdirectory(src)
add_subdirectory(test)

enable_testing()
add_test(NAME "ID Test" COMMAND id_test)

# Generate Doxygen documentation.
set(DOXYGEN_CONFIG_FILE Doxyfile.d)
find_package(Doxygen)
if(DOXYGEN_FOUND)
	if(EXISTS ${PROJECT_SOURCE_DIR}/${DOXYGEN_CONFIG_FILE})
		add_custom_target(
			doc
			${DOXYGEN_EXECUTABLE}
			${PROJECT_SOURCE_DIR}/${DOXYGEN_CONFIG_FILE}
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "Generating API documentation with Doxygen" VERBATIM
		)
	endif(EXISTS ${PROJECT_SOURCE_DIR}/${DOXYGEN_CONFIG_FILE})
endif(DOXYGEN_FOUND)
