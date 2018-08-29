
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

set(Gumbo_VERSION 0.10.1)

set(Gumbo_DIR ${PROJECT_SOURCE_DIR})

# Gumbo source files.
set(Gumbo_SRCS
    ${Gumbo_DIR}/src/attribute.c
    ${Gumbo_DIR}/src/attribute.h
    ${Gumbo_DIR}/src/char_ref.c
    ${Gumbo_DIR}/src/char_ref.h
    ${Gumbo_DIR}/src/error.c
    ${Gumbo_DIR}/src/error.h
    ${Gumbo_DIR}/src/insertion_mode.h
    ${Gumbo_DIR}/src/parser.c
    ${Gumbo_DIR}/src/parser.h
    ${Gumbo_DIR}/src/string_buffer.c
    ${Gumbo_DIR}/src/string_buffer.h
    ${Gumbo_DIR}/src/string_piece.c
    ${Gumbo_DIR}/src/string_piece.h
    ${Gumbo_DIR}/src/tag.c
    ${Gumbo_DIR}/src/tag_enum.h
    ${Gumbo_DIR}/src/tag_gperf.h
    ${Gumbo_DIR}/src/tag_strings.h
    ${Gumbo_DIR}/src/tag_sizes.h
    ${Gumbo_DIR}/src/token_type.h
    ${Gumbo_DIR}/src/tokenizer.c
    ${Gumbo_DIR}/src/tokenizer.h
    ${Gumbo_DIR}/src/tokenizer_states.h
    ${Gumbo_DIR}/src/utf8.c
    ${Gumbo_DIR}/src/utf8.h
    ${Gumbo_DIR}/src/util.c
    ${Gumbo_DIR}/src/util.h
    ${Gumbo_DIR}/src/vector.c
    ${Gumbo_DIR}/src/vector.h)

include_directories(${Gumbo_DIR}/src)

# Compile as C99 (default is C89, which Gumbo is not written in).
if(CMAKE_C_COMPILER_ID EQUAL GNU OR
   CMAKE_CXX_COMPILER_ID EQUAL GNU OR
   CMAKE_CXX_COMPILER_ID EQUAL Clang)
  set(CMAKE_C_FLAGS "-std=c99")
endif()

# Library target.
add_library(Gumbo STATIC ${Gumbo_SRCS})
set_target_properties(Gumbo PROPERTIES VERSION ${Gumbo_VERSION})

