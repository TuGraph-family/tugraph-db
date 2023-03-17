#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

BUILD_OUTPUT_TAR_URL=$1
ASAN=$2

if [[ "$ASAN" == "asan" ]]; then
echo 'run ut with asan ...'
set +e
else
set -e
fi

# mv code to my workspace
MY_WORKSPACE='/workspace/code-repo'

if [[ "$MY_WORKSPACE" != "$WORKSPACE" ]]; then
echo "$WORKSPACE is not equal to $MY_WORKSPACE"
mkdir -p $MY_WORKSPACE
echo 'mv source code to my workspace'
echo $WORKSPACE
mv $WORKSPACE/* $WORKSPACE/.[^.]* $MY_WORKSPACE
fi

# set $WORKSPACE to root dir
cd $MY_WORKSPACE
mkdir -p testresult/gtest

# download build.output
echo $BUILD_OUTPUT_TAR_URL
wget -q -t 3 $BUILD_OUTPUT_TAR_URL -O output.tar.gz
tar -zxvf output.tar.gz

# unittest
cd build/output
OMP_NUM_THREADS=8 ./unit_test --gtest_output=xml:$MY_WORKSPACE/testresult/gtest/
rm -rf testdb* .import_tmp

# codecov
cd $MY_WORKSPACE
bash $MY_WORKSPACE/ci/codecov.sh $MY_WORKSPACE/build $MY_WORKSPACE/testresult
cd testresult/gtest

# mv code to $WORKSPACE
if [[ "$MY_WORKSPACE" != "$WORKSPACE" ]]; then
echo "$WORKSPACE is not equal to $MY_WORKSPACE"
echo 'mv source code back to workspace'
mv $MY_WORKSPACE/* $MY_WORKSPACE/.[^.]* $WORKSPACE
fi

cd $WORKSPACE
python3  ./ci/lcov_cobertura.py $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
