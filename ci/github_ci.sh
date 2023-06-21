#!/bin/bash
set -e

cd $WORKSPACE
pwd
ls -al

# cpplint check
bash ./cpplint/check_all.sh

# build deps
cd deps
SKIP_WEB=1 bash ./build_deps.sh -j2

# build tugraph
cd $WORKSPACE
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DENABLE_INTERNAL_BUILD=1
make -j2

# build java
JAVA_CLIENT_VERSION=1.2.1

cd $WORKSPACE/deps/tugraph-db-client-java/
sh local_build.sh
cp rpc-client-test/target/tugraph-db-java-rpc-client-test-${JAVA_CLIENT_VERSION}.jar $WORKSPACE/build/output/
cp ogm/tugraph-db-ogm-test/target/tugraph-db-ogm-test-${JAVA_CLIENT_VERSION}.jar $WORKSPACE/build/output/

# unittest
mkdir -p $WORKSPACE/testresult/gtest/
cd $WORKSPACE/build/output
OMP_NUM_THREADS=8 ./unit_test --gtest_output=xml:$WORKSPACE/testresult/gtest/
rm -rf testdb* .import_tmp

#cd $WORKSPACE/src/client/python/TuGraphClient
# install TuGraphClient
#python3 setup.py install
# run TuGraphClient unit_tests
#python3 setup.py test

# integrate tests
cd $WORKSPACE/build/output
cp ../../src/client/python/TuGraphClient/TuGraphClient.py .
cp ../../src/client/python/TuGraphClient/TuGraphRestClient.py .
cp -r ../../test/integration/* ./
cp -r ../../demo/movie .
pytest ./

# codecov
cd $WORKSPACE
bash ./ci/codecov.sh $WORKSPACE/build $WORKSPACE/testresult $CODECOV_TOKEN
python3  ./ci/lcov_cobertura.py $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
