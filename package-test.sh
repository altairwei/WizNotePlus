#!/bin/bash
# Program:
#   Package WizNotePlus and it's dependencies into a portable AppImage.

# Options
# - CMAKE_PREFIX_PATH should be your Qt5 librar path.
# - WIZNOTE_SOURCE_DIR should be WizNotePlus top-level source tree/
# - WIZNOTE_BINARY_DIR should be CMake building directory.
CMAKE_PREFIX_PATH=/home/altairwei/usr/Qt/5.11.1/gcc_64
WIZNOTE_SOURCE_DIR=./
WIZNOTE_BINARY_DIR=../build-WizNotePlus
WIZNOTE_PACKAGE_DIR=../package-WizNotePlus

# Create Build Directory
mkdir ${WIZNOTE_BINARY_DIR}
rm -rf ${WIZNOTE_BINARY_DIR}/*

# Configure adn Generate CMake and Makefile
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}  -H${WIZNOTE_SOURCE_DIR} -B${WIZNOTE_BINARY_DIR} && \
make -j2
