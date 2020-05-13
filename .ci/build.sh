#!/usr/bin/env bash

set -e -o pipefail
shopt -s failglob
export LC_ALL=C

# Check Qt installation
if [[ -z "${QT_INSTALL_PREFIX}" ]]; then
	command -v qmake >/dev/null 2>&1 || { echo >&2 "[Error] Qt is required."; exit 1; }
fi

# Check requirements
command -v git >/dev/null 2>&1 || { echo >&2 "[Error] git is required."; exit 1; }
command -v conan >/dev/null 2>&1 || { echo >&2 "[Error] conan is required."; exit 1; }

# Check package tool
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)
		command -v linuxdeployqt >/dev/null 2>&1 || { echo >&2 "[Error] linuxdeployqt is required."; exit 1; }
		;;
    Darwin*)
		command -v appdmg >/dev/null 2>&1 || { echo >&2 "[Error] appdmg is required."; exit 1; }
		;;
esac

# Setup work folder
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