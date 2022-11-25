#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

ASAN=$1

# set $WORKSPACE to root dir
cd ${ACB_BUILD_DIR}/code-repo
pwd
ls -al
cd deps
SKIP_WEB=1 bash ./build_deps.sh -j6
cd ${ACB_BUILD_DIR}/code-repo
mkdir build && cd build

if [[ "$ASAN" == "asan" ]]; then
echo 'build with asan ...'
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DENABLE_INTERNAL_BUILD=1  -DENABLE_Address_Sanitizer=1
else
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DENABLE_INTERNAL_BUILD=1
fi

make -j6

cd ${ACB_BUILD_DIR}/code-repo

tar -czvf output.tar.gz ./build
cp output.tar.gz $ACB_BUILD_DIR
