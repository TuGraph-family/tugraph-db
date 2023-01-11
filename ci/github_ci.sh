#!/bin/bash
set -e

cd $WORKSPACE
pwd
ls -al

# cpplint check
bash ./cpplint/check_all.sh

# build deps
cd deps
SKIP_WEB=1 bash ./build_deps.sh -j6

# build tugraph
cd $WORKSPACE
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DENABLE_INTERNAL_BUILD=1
make -j6

# unittest
mkdir -p $WORKSPACE/testresult/gtest/
cd $WORKSPACE/build/output
OMP_NUM_THREADS=8 ./unit_test --gtest_output=xml:$WORKSPACE/testresult/gtest/
rm -rf testdb* .import_tmp

cd $WORKSPACE/src/client/python/TuGraphClient
# install TuGraphClient
python3 setup.py install
# run TuGraphClient unit_tests
python3 setup.py test

# integrate tests
cd $WORKSPACE/build/output
cp ../../src/client/python/TuGraphClient/TuGraphClient.py .
cp -r ../../test/integration/* ./
pytest ./

# codecov
cd $WORKSPACE
bash ./ci/codecov.sh $WORKSPACE/build $WORKSPACE/testresult $CODECOV_TOKEN
python3  ./ci/lcov_cobertura.py $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
