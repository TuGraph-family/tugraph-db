#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

ASAN=$1

set -e

# set $WORKSPACE to root dir
cd ${ACB_BUILD_DIR}/code-repo
pwd
ls -al
cd deps
bash ./build_deps.sh -j6
cd ${ACB_BUILD_DIR}/code-repo
mkdir build && cd build

# build cpp
if [[ "$ASAN" == "asan" ]]; then
echo 'build with asan ...'
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_INTERNAL_BUILD=1  -DENABLE_Address_Sanitizer=1
else
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_INTERNAL_BUILD=1
fi

make package -j6

# build java
cd ${ACB_BUILD_DIR}/code-repo/deps/tugraph-db-client-java/
sh local_build.sh
