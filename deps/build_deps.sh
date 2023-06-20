#!/bin/bash

function usage {
    echo "usage: ./build_deps [-jN] [-o <output_dir>] [-h]"
    echo "  -jN: use N threads to build"
    echo "  -o <output_dir>: output directory"
    echo "  -h: print this message"
    exit 0
}

# usage: ./build_deps [-jN] [-o <output_dir>] [-h]
# parse arguments
while getopts ":j:o:h" opt; do
    case ${opt} in
        j)
            MAKE_ARGS="-j${OPTARG}"
            ;;
        o)
            OUT_DIR=${OPTARG}
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done


if [ ! ${USE_CLANG} ] || [ ${USE_CLANG} = 0 ]; then
    CC="gcc"
    CXX="g++"
elif [ ${USE_CLANG} = 1 ]; then
    CC="clang"
    CXX="clang"
fi

DEPS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
if [ ! ${OUT_DIR} ]; then
    OUT_DIR=${DEPS_DIR}
fi
INSTALL_DIR=${OUT_DIR}/install
BUILD_DIR=${OUT_DIR}/build

mkdir -p ${INSTALL_DIR}
mkdir -p ${BUILD_DIR}

function BuildPackage {
    ARGS=("$@")
    NAME=$1
    SOURCE_DIR=$2

    echo "======================================"
    echo "Building ${NAME}"
    echo "======================================"
    cmake -S ${DEPS_DIR}/${SOURCE_DIR} -B ${BUILD_DIR}/${NAME} \
        -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=0 \
        -DBUILD_TESTS=0 \
        -DBUILD_SAMPLES=0 \
        -DCMAKE_C_COMPILER=${CC} \
        -DCMAKE_CXX_COMPILER=${CXX} \
        ${ARGS[@]:2}
    cmake --build ${BUILD_DIR}/${NAME} --target install  ${MAKE_ARGS}
    res=$?
    if [ ${res} -ne 0 ]; then
        echo "Failed to build ${NAME}"
    fi
    return ${res}
}

if [ ! ${ONLY_WEB} ] || [ ! ${ONLY_WEB} = 1 ]; then
    BuildPackage antlr4 antlr4/runtime/Cpp/ -DWITH_DEMO=0 -DANTLR_BUILD_CPP_TESTS=0 -DCMAKE_CXX_STANDARD=17 || exit 1
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

