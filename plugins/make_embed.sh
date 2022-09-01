#!/bin/bash

# This file shows you how to build an embedding application using TuGraph. We assume you are
# building the application to debug a plugin. So the PLUGIN_CPP must be provided as the paramemter
# to this script.
#
# LG_INCLUDE must point to the include directory of TuGraph
# LG_LIB must point to the library path of TuGraph
#
# If everything runs smoothly, you will get an executable called `embed` which you can simply
# run or debug.

PLUGIN_NAME=$1
LG_INCLUDE=../include
LG_LIB=../build/output/

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    g++ -fno-gnu-unique -fPIC -g --std=c++14 -I${LG_INCLUDE} -rdynamic -O3 -fopenmp -DNDEBUG -o cpp/${PLUGIN_NAME}_procedure cpp/${PLUGIN_NAME}_core.cpp cpp/${PLUGIN_NAME}_procedure.cpp embed_main.cpp "${LG_LIB}/liblgraph.so" -lrt
elif [[ "$OSTYPE" == "darwin"* ]]; then
    clang++ -stdlib=libc++ -fPIC -g --std=c++14 -I${LG_INCLUDE} -rdynamic -O3 -Xpreprocessor -fopenmp -lomp -DNDEBUG -o cpp/${PLUGIN_NAME}_procedure cpp/${PLUGIN_NAME}_core.cpp cpp/${PLUGIN_NAME}_procedure.cpp embed_main.cxx "${LG_LIB}/liblgraph.so"
fi
