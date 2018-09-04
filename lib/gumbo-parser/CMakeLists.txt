
# Copyright 2016 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.7)

project(Gumbo C)

set(Gumbo_VERSION "0.10.1")

# Gumbo source files.
set(Gumbo_SRCS
    attribute.c
    attribute.h
    char_ref.c
    char_ref.h
    error.c
    error.h
    insertion_mode.h
    parser.c
    parser.h
    string_buffer.c
    string_buffer.h
    string_piece.c
    string_piece.h
    tag.c
    tag_enum.h
    tag_gperf.h
    tag_strings.h
    tag_sizes.h
    token_type.h
    tokenizer.c
    tokenizer.h
    tokenizer_states.h
    utf8.c
    utf8.h
    util.c
    util.h
    vector.c
    vector.h)

include_directories(
	${PROJECT_SOURCE_DIR}
	${PROJECT_BINARY_DIR}
)

if(MSVC)
    include_directories(
        ${PROJECT_SOURCE_DIR}/msvc
        ${PROJECT_BINARY_DIR}/msvc
    )
endif(MSVC)

# Compile as C99 (default is C89, which Gumbo is not written in).
if(CMAKE_C_COMPILER_ID EQUAL GNU OR
   CMAKE_CXX_COMPILER_ID EQUAL GNU OR
   CMAKE_CXX_COMPILER_ID EQUAL Clang)
  set(CMAKE_C_FLAGS "-std=c99")
endif()

# Library target.
add_library(Gumbo SHARED ${Gumbo_SRCS})
add_library(Gumbo_static STATIC ${Gumbo_SRCS})

set_target_properties(Gumbo PROPERTIES VERSION ${Gumbo_VERSION})
set_target_properties(Gumbo_static PROPERTIES OUTPUT_NAME "Gumbo")

