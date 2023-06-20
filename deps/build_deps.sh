#!/bin/bash

if [ $# -eq 0 ]; then
    MAKE_ARGS=-j8
elif [ $# -eq 1 ]; then
    MAKE_ARGS=$1
else
    echo "usage: ./build_deps [-jN]"
    exit 1
fi

if [ ! ${USE_CLANG} ] || [ ${USE_CLANG} = 0 ]; then
    CC="gcc"
    CXX="g++"
elif [ ${USE_CLANG} = 1 ]; then
    CC="clang"
    CXX="clang"
fi

DEPS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
INSTALL_DIR=${DEPS_DIR}/install

mkdir -p ${INSTALL_DIR}/include

if [ ! ${ONLY_WEB} ] || [ ! ${ONLY_WEB} = 1 ]; then
    ln -sf ${DEPS_DIR}/fma-common/fma-common ${INSTALL_DIR}/include/
    ln -sf ${DEPS_DIR}/tiny-process-library ${INSTALL_DIR}/include/
fi

if [ ! ${SKIP_WEB} ] || [ ! ${SKIP_WEB} = 1 ]; then
    echo "======================================"
    echo "Building tugraph-web"
    echo "======================================"
    cd ${DEPS_DIR}/tugraph-web
    npm i
    npm run build
    RESOURCE_DIR=${DEPS_DIR}/../src/restful/server/resource/
    rm -rf ${RESOURCE_DIR}
    cp -r dist ${RESOURCE_DIR}
    cd -
fi

