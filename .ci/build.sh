#!/usr/bin/env bash

set -e -o pipefail
shopt -s failglob
export LC_ALL=C

SRC_DIR=$1
WORK_DIR=$2
WORK_DIR_SOURCE=${WORK_DIR}/source
WORK_DIR_BUILD=${WORK_DIR}/build
WORK_DIR_PKG=${WORK_DIR}/package
WORK_DIR_DIST=${WORK_DIR}/dist
VERSION="$(cd ${SRC_DIR} && git describe --tags)"

# Check input and output directories
if [[ ! -d ${SRC_DIR} || ! -d ${WORK_DIR} ]]; then
	echo "Can not find 'WizNotePlus source folder' or 'Build working folder'" >&2
	exit 1
fi

# Create dirs
cd ${WORK_DIR}
mkdir -p ${WORK_DIR_BUILD}
mkdir -p ${WORK_DIR_PKG}

# Build whole project
conan install ${SRC_DIR} -if ${WORK_DIR_BUILD} --build missing -o qtdir=${QT_INSTALL_PREFIX}
conan build ${SRC_DIR} -bf ${WORK_DIR_BUILD}
conan package ${SRC_DIR} -bf ${WORK_DIR_BUILD} -pf ${WORK_DIR_PKG}