#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

set -ex
ASAN=$1
WITH_PROCEDURE=${2:-"OFF"}

# set $WORKSPACE to root dir
pwd
ls -al
cd ${ACB_BUILD_DIR}/code-repo/deps
bash ./build_deps.sh -j6
cd ${ACB_BUILD_DIR}/code-repo
mkdir build && cd build

# build cpp
if [[ "$ASAN" == "asan" ]]; then
echo 'build with asan ...'
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DBUILD_PROCEDURE=$WITH_PROCEDURE -DWITH_TEST=OFF
else
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DBUILD_PROCEDURE=$WITH_PROCEDURE -DWITH_TEST=OFF
fi

make -j6

# package
cd ${ACB_BUILD_DIR}
tar -czf output.tar.gz code-repo