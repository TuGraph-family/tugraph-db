#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

set -ex
WITH_PROCEDURE=${1:-"OFF"}
ASAN=$2

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
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DBUILD_PROCEDURE=$WITH_PROCEDURE
else
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DBUILD_PROCEDURE=$WITH_PROCEDURE
fi

make -j6

# build java client
cd ${ACB_BUILD_DIR}/code-repo/deps/tugraph-db-client-java/
sh local_build.sh
cp rpc-client-test/target/tugraph-db-java-rpc-client-test-*.jar ${ACB_BUILD_DIR}/code-repo/build/output/
cp ogm/tugraph-db-ogm-test/target/tugraph-db-ogm-test-*.jar ${ACB_BUILD_DIR}/code-repo/build/output/

# build tugraph db management
cd ${ACB_BUILD_DIR}/code-repo/deps/tugraph-db-management/
sh local_build.sh
cp target/tugraph-db-management-*.jar ${ACB_BUILD_DIR}/code-repo/build/output/

# build cpp client test
if [[ "$ASAN" != "asan" ]]; then
cd ${ACB_BUILD_DIR}/code-repo/test/test_rpc_client
sh ./cpp/CppClientTest/compile.sh
cp -r ./cpp/CppClientTest/build/clienttest ${ACB_BUILD_DIR}/code-repo/build/output/
fi

# package
cd ${ACB_BUILD_DIR}
tar -czf output.tar.gz code-repo