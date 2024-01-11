#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

ASAN=$1

set -ex

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
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_Address_Sanitizer=1
else
cmake .. -DCMAKE_BUILD_TYPE=Release
fi

make -j6 package
