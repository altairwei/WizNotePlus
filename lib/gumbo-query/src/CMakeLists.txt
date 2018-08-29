# libgumbo_query

# Gumbo dependency include directory.
include_directories(${Gumbo_INCLUDE_DIRS})

file(GLOB SRC_FILES *.cpp)
add_library(gumbo_query_shared SHARED ${SRC_FILES})
set_target_properties(gumbo_query_shared PROPERTIES OUTPUT_NAME "gq")
add_library(gumbo_query_static STATIC ${SRC_FILES})
set_target_properties(gumbo_query_static PROPERTIES OUTPUT_NAME "gq")

target_link_libraries(gumbo_query_shared ${Gumbo_LIBRARY})
# Link our static library with the static Gumbo dependency if it exists.
if(${Gumbo_static_LIBRARY})
  target_link_libraries(gumbo_query_static ${Gumbo_static_LIBRARY})
else(${Gumbo_LIBRARY})
  target_link_libraries(gumbo_query_static ${Gumbo_LIBRARY})
endif(${Gumbo_static_LIBRARY})

set(gumbo_query_LIBRARIES gumbo_query_shared gumbo_query_static)
set_property(TARGET ${gumbo_query_LIBRARIES} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${gumbo_query_LIBRARIES} PROPERTY CXX_STANDARD_REQUIRED ON)

install(TARGETS gumbo_query_shared  gumbo_query_static
	LIBRARY DESTINATION lib
     	ARCHIVE DESTINATION lib)

file(GLOB HEAD_FILES *.h)
install(FILES ${HEAD_FILES} DESTINATION include/gq)
