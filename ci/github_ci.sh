#!/bin/bash
set -e

ASAN=$1
TEST=$2
WITH_PROCEDURE=${3:-"OFF"}

cd $WORKSPACE

# cpplint check
bash ./cpplint/check_all.sh

# build deps
cd deps
SKIP_WEB=1 bash ./build_deps.sh -j2

# build tugraph
cd $WORKSPACE
mkdir build && cd build
if [[ "$ASAN" == "asan" ]]; then
echo 'build with asan ...'
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DBUILD_PROCEDURE=$WITH_PROCEDURE
else
cmake .. -DCMAKE_BUILD_TYPE=Coverage -DBUILD_PROCEDURE=$WITH_PROCEDURE
fi
make -j2

if [[ "$TEST" == "ut" ]]; then
  make unit_test fma_unit_test -j2

  # build tugraph db management
  cd $WORKSPACE/deps/tugraph-db-management/
  sh local_build.sh
  cp target/tugraph-db-management-*.jar $WORKSPACE/build/output/

  # unittest
  mkdir -p $WORKSPACE/testresult/gtest/
  cd $WORKSPACE/build/output
  OMP_NUM_THREADS=2 ./fma_unit_test -t all
  if [[ "$ASAN" == "asan" ]]; then
      export LSAN_OPTIONS=suppressions=$WORKSPACE/test/asan.suppress
  fi
  OMP_NUM_THREADS=2 ./unit_test --gtest_output=xml:$WORKSPACE/testresult/gtest/
  rm -rf testdb* .import_tmp
  if [[ "$ASAN" == "asan" ]]; then
    exit 0
  fi
  # codecov
  cd $WORKSPACE
  bash ./ci/codecov.sh $WORKSPACE/build $WORKSPACE/testresult
  # Uploading report to CodeCov
  bash <(curl -s https://codecov.io/bash) -f $WORKSPACE/testresult/coverage.info -t $CODECOV_TOKEN || echo "Codecov did not collect coverage reports"
  python3  ./ci/lcov_cobertura.py $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
else
  # build java client
  cd $WORKSPACE/deps/tugraph-db-client-java/
  sh local_build.sh
  cp rpc-client-test/target/tugraph-db-java-rpc-client-test-*.jar $WORKSPACE/build/output/
  cp ogm/tugraph-db-ogm-test/target/tugraph-db-ogm-test-*.jar $WORKSPACE/build/output/

  # build cpp client test
  cd $WORKSPACE/test/test_rpc_client
  sh ./cpp/CppClientTest/compile.sh
  cp -r ./cpp/CppClientTest/build/clienttest $WORKSPACE/build/output/

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
  cp -r ../../learn/examples/* ./
  cp -r ../../demo/movie .
  if [[ "$WITH_PROCEDURE" == "OFF" ]]; then
      rm -rf test_algo.py test_sampling.py test_train.py
  fi
  pytest ./
  # codecov
  cd $WORKSPACE
  mkdir testresult
  bash ./ci/codecov.sh $WORKSPACE/build $WORKSPACE/testresult
  # Uploading report to CodeCov
  bash <(curl -s https://codecov.io/bash) -f $WORKSPACE/testresult/coverage.info -t $CODECOV_TOKEN || echo "Codecov did not collect coverage reports"
  python3  ./ci/lcov_cobertura.py $WORKSPACE/testresult/coverage.info --output $WORKSPACE/testresult/coverage.xml --demangle
fi
