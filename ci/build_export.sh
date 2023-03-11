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

# build cpp
if [[ "$ASAN" == "asan" ]]; then
echo 'build with asan ...'
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DENABLE_INTERNAL_BUILD=1  -DENABLE_Address_Sanitizer=1
else
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DENABLE_INTERNAL_BUILD=1
fi

make -j6

# build java
cd ${ACB_BUILD_DIR}/code-repo/test/integration/
sh rpc_client/java/local_build.sh
cp rpc_client/java/JavaClientTest/target/tugraph-db-rpc-client-test-1.1.0-jar-with-dependencies.jar ${ACB_BUILD_DIR}/code-repo/build/output/
cp rpc_client/java/TuGraphOGMTest/target/TuGraphOgmTest-1.1.0.jar ${ACB_BUILD_DIR}/code-repo/build/output/

# package
cd ${ACB_BUILD_DIR}/code-repo

tar -czvf output.tar.gz ./build
cp output.tar.gz $ACB_BUILD_DIR
