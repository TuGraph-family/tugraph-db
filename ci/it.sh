#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

set -e

BUILD_OUTPUT_TAR_URL=$1
WITH_PROCEDURE=${2:-"OFF"}

echo 'build output tar url:'
echo $BUILD_OUTPUT_TAR_URL

# mv code path for coverage
MY_WORKSPACE='/workspace/code-repo'

if [[ -d "$MY_WORKSPACE" ]]; then
  echo "$MY_WORKSPACE already exists, will be removed!"
  rm -rf $MY_WORKSPACE
fi

# download build.output
mkdir -p /workspace
cd /workspace
echo $BUILD_OUTPUT_TAR_URL
wget -q -t 3 $BUILD_OUTPUT_TAR_URL -O output.tar.gz
tar -zxmf output.tar.gz
# refresh timestamp of build
tar -zxmf output.tar.gz code-repo/build

# set $MY_WORKSPACE to root dir
cd $MY_WORKSPACE

# build java
cd ${MY_WORKSPACE}/deps/tugraph-db-client-java/
sh local_build.sh
cp rpc-client-test/target/tugraph-db-java-rpc-client-test-*.jar ${MY_WORKSPACE}/build/output/
cp ogm/tugraph-db-ogm-test/target/tugraph-db-ogm-test-*.jar ${MY_WORKSPACE}/build/output/

# build cpp client test
cd ${MY_WORKSPACE}/test/test_rpc_client
sh ./cpp/CppClientTest/compile.sh
cp -r ./cpp/CppClientTest/build/clienttest ${MY_WORKSPACE}/build/output/

# set $MY_WORKSPACE to root dir
cd $MY_WORKSPACE

# integrate tests 
cd build/output
ln -s ../../src/client/python/TuGraphClient/TuGraphClient.py .
ln -s ../../src/client/python/TuGraphClient/TuGraphRestClient.py .
ln -s ../../test/integration/* ./
ln -s ../../learn/examples/* ./
ln -s ../../demo/movie .
if [[ "$WITH_PROCEDURE" == "OFF" ]]; then
    rm -rf test_algo.py test_sampling.py test_train.py
fi
pytest ./

# codecov
cd $MY_WORKSPACE
mkdir testresult
bash $MY_WORKSPACE/ci/codecov.sh $MY_WORKSPACE/build $MY_WORKSPACE/testresult

# mv code to $WORKSPACE
if [[ "$WORKSPACE" != "$MY_WORKSPACE" ]]; then
  echo "$WORKSPACE is not equal to $MY_WORKSPACE"
  echo 'mv source code back to workspace'
  rm -rf $WORKSPACE
  mkdir -p $WORKSPACE
  mv $MY_WORKSPACE/* $MY_WORKSPACE/.[^.]* $WORKSPACE
fi

cd $WORKSPACE

python3 ./ci/lcov_cobertura.py $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
