#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

ASAN=$1

set -e

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
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
else
cmake .. -DCMAKE_BUILD_TYPE=Coverage
fi

make package -j6

# build java
cd ${ACB_BUILD_DIR}/code-repo/deps/tugraph-db-client-java/
sh local_build.sh
cp rpc-client-test/target/tugraph-db-java-rpc-client-test-*.jar ${ACB_BUILD_DIR}/code-repo/build/output/
cp ogm/tugraph-db-ogm-test/target/tugraph-db-ogm-test-*.jar ${ACB_BUILD_DIR}/code-repo/build/output/

# package
cd ${ACB_BUILD_DIR}/code-repo
tar -czf output.tar.gz ./build
cp output.tar.gz $ACB_BUILD_DIR
