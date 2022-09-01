#!/bin/bash

# Creating report
lcov --directory $1/src --directory $1/toolkits --capture --output-file $2/coverage.info # capture coverage info
lcov --remove $2/coverage.info '/usr/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/fma-common/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/antlr4/*' --output-file $2/coverage.info # filter out system
lcov --remove $2/coverage.info '*/cpprestsdk/*' --output-file $2/coverage.info # filter out system
# lcov --list $2/coverage.info #debug info
