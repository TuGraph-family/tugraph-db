#!/bin/bash
# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

set -ex
BUILD_OUTPUT_TAR_URL=$1
ASAN=$2

# set $WORKSPACE to root dir
pwd
ls -al
rm -rf ${ACB_BUILD_DIR}/code-repo
cd ${ACB_BUILD_DIR}
echo $BUILD_OUTPUT_TAR_URL
wget -q -t 3 $BUILD_OUTPUT_TAR_URL -O output.tar.gz
tar -zxf output.tar.gz
rm -rf output.tar.gz

# build tugraph db management
cd ${ACB_BUILD_DIR}/code-repo/deps/tugraph-db-management/
sh local_build.sh
cp target/tugraph-db-management-*.jar ${ACB_BUILD_DIR}/code-repo/build/output/

# build unit_test
cd ${ACB_BUILD_DIR}/code-repo/build
make unit_test fma_unit_test -j6
# package
cd ${ACB_BUILD_DIR}
tar -czf output.tar.gz code-repo