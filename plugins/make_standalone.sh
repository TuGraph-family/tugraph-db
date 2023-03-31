#!/bin/bash

# This file shows you how to build an embedding application using TuGraph. We assume you are
# building the application to debug a plugin. So the PLUGIN_CPP must be provided as the parameter
# to this script.
#
# LG_INCLUDE_PATH must point to the include, src and deps/install/include directory of TuGraph
# SRC must point to the src/lgraph_api/olap_base.cpp, src/lgraph_api/lgraph_utils.cpp and
# src/lgraph_api/olap_profile.cpp.
#
# If everything runs smoothly, you will get an executable called `embed` which you can simply
# run or debug.

PLUGIN_NAME=$1
LG_INCLUDE_PATH="-I../include -I../src -I../deps/install/include"
SRC="../src/lgraph_api/olap_base.cpp ../src/lgraph_api/lgraph_utils.cpp ../src/lgraph_api/olap_profile.cpp"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  g++ -fno-gnu-unique -fPIC -g --std=c++17 $LG_INCLUDE_PATH -rdynamic -O3 -fopenmp -DNDEBUG -o cpp/${PLUGIN_NAME}_standalone cpp/${PLUGIN_NAME}_core.cpp cpp/${PLUGIN_NAME}_standalone.cpp $SRC -lrt -lstdc++fs
elif [[ "$OSTYPE" == "darwin"* ]]; then
  clang++ -stdlib=libc++ -fPIC -g --std=c++17 $LG_INCLUDE_PATH -rdynamic -O3 -Xpreprocessor -fopenmp -lomp -DNDEBUG -o cpp/${PLUGIN_NAME}_standalone cpp/${PLUGIN_NAME}_core.cpp cpp/${PLUGIN_NAME}_standalone.cpp $SRC -lstdc++fs
fi

