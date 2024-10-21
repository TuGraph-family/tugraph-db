#!/bin/bash

# Standard Github-hosted runner is 2core currently.
# Larger runner will support soon(current in Beta).

set -ex

BUILD_OUTPUT_TAR_URL=$1
ASAN=$2

# mv code to my workspace
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
# set $WORKSPACE to root dir
cd $MY_WORKSPACE
mkdir -p testresult/gtest

# unittest
cd $MY_WORKSPACE/build/output
ln -s ../../test/integration/data ./
OMP_NUM_THREADS=8 ./fma_unit_test -t all
if [[ "$ASAN" == "asan" ]]; then
    export LSAN_OPTIONS=suppressions=$MY_WORKSPACE/test/asan.suppress
fi
OMP_NUM_THREADS=8 ./unit_test --gtest_output=xml:$MY_WORKSPACE/testresult/gtest/ --gtest_break_on_failure=false
rm -rf testdb* .import_tmp

if [[ "$ASAN" == "asan" ]]; then
  exit 0
fi
# codecov
cd $MY_WORKSPACE
bash $MY_WORKSPACE/ci/codecov.sh $MY_WORKSPACE/build $MY_WORKSPACE/testresult

# mv code to $WORKSPACE
if [[ "$MY_WORKSPACE" != "$WORKSPACE" ]]; then
  echo "$WORKSPACE is not equal to $MY_WORKSPACE"
  echo 'mv source code back to workspace'
  rm -rf $WORKSPACE
  mkdir -p $WORKSPACE
  mv $MY_WORKSPACE/* $MY_WORKSPACE/.[^.]* $WORKSPACE
fi

cd $WORKSPACE
python3  ./ci/lcov_cobertura.py $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
