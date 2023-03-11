#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

set -e

BUILD_OUTPUT_TAR_URL=$1

echo 'build output tar url:'
echo $BUILD_OUTPUT_TAR_URL

# mv code path for coverage
MY_WORKSPACE='/workspace/code-repo'

if [[ "$WORKSPACE" != "$MY_WORKSPACE" ]]; then
echo "$WORKSPACE is not equal to $MY_WORKSPACE"
mkdir -p $MY_WORKSPACE
echo 'mv source code to my workspace'
mv $WORKSPACE/* $WORKSPACE/.[^.]* $MY_WORKSPACE
fi

# set $MY_WORKSPACE to root dir
cd $MY_WORKSPACE

# download build.output
wget -q -t 3 ${BUILD_OUTPUT_TAR_URL} -O output.tar.gz
tar -zxvf output.tar.gz

# set $MY_WORKSPACE to root dir
cd $MY_WORKSPACE

# integrate tests 
cd build/output
cp ../../src/client/python/TuGraphClient/TuGraphClient.py .
cp -r ../../test/integration/* ./
pytest ./

# codecov
cd $MY_WORKSPACE
mkdir testresult
bash $MY_WORKSPACE/ci/codecov.sh $MY_WORKSPACE/build $MY_WORKSPACE/testresult

# mv code to $WORKSPACE
if [[ "$WORKSPACE" != "$MY_WORKSPACE" ]]; then
echo "$WORKSPACE is not equal to $MY_WORKSPACE"
echo 'mv source code back to workspace'
mv $MY_WORKSPACE/* $MY_WORKSPACE/.[^.]* $WORKSPACE
fi

cd $WORKSPACE

python3  ./ci/lcov_cobertura.py  $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
