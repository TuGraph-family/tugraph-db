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
BUILD_DIR=${DEPS_DIR}/build

mkdir -p ${INSTALL_DIR}
mkdir -p ${BUILD_DIR}

function BuildPackage {
    ARGS=("$@")
    NAME=$1
    SOURCE_DIR=$2

    echo "======================================"
    echo "Building ${NAME}"
    echo "======================================"
    mkdir -p ${BUILD_DIR}/${NAME}
    cd ${BUILD_DIR}/${NAME}
    cmake ${DEPS_DIR}/${SOURCE_DIR} \
        -DCMAKE_CXX_FLAGS="-fPIC -fpic" \
        -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=0 \
        -DBUILD_TESTS=0 \
        -DBUILD_SAMPLES=0 \
        -DCMAKE_C_COMPILER=${CC} \
        -DCMAKE_CXX_COMPILER=${CXX} \
        ${ARGS[@]:2}
    make ${MAKE_ARGS}
    make install
    res=$?
    if [ ${res} -ne 0 ]; then
        echo "Failed to build ${NAME}"
    fi
    cd -
    return ${res}
}

if [ ! ${ONLY_WEB} ] || [ ! ${ONLY_WEB} = 1 ]; then
    sed -i 's/CMAKE_CXX_STANDARD 11/CMAKE_CXX_STANDARD 17/g' ${DEPS_DIR}/antlr4/runtime/Cpp/CMakeLists.txt
    BuildPackage antlr4 antlr4/runtime/Cpp/ -DWITH_DEMO=False || exit 1
    # remove Any.h because it is modified
    rm -f ${INSTALL_DIR}/include/antlr4-runtime/support/Any.h

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

