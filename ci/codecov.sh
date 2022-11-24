#!/bin/bash

# Creating report
lcov --directory $1/src --directory $1/test --directory $1/toolkits --capture --output-file $2/coverage.info # capture coverage info
lcov --remove $2/coverage.info '/usr/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/fma-common/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/antlr4/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/cpprestsdk/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/test/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/include/libcuckoo/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/src/cypher/parser/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/include/tools/json.hpp' --output-file $2/coverage.info # filter out system
# lcov --list $2/coverage.info #debug info

# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) -f $2/coverage.info -t  $3 || echo "Codecov did not collect coverage reports"