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

APP=$1
APP_PATH=algo_cpp
LG_INCLUDE=../include
LG_LIB=../build/output/

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    g++ -fno-gnu-unique -fPIC -g --std=c++17 -I${LG_INCLUDE} -rdynamic -O3 -fopenmp -ldl -DNDEBUG -o $APP_PATH/${APP}_procedure $APP_PATH/${APP}_core.cpp $APP_PATH/${APP}_procedure.cpp embed_main.cpp "${LG_LIB}/liblgraph.so" -lrt
elif [[ "$OSTYPE" == "darwin"* ]]; then
    clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I${LG_INCLUDE} -rdynamic -O3 -Xpreprocessor -fopenmp -ldl -lomp -DNDEBUG -o $APP_PATH/${APP}_procedure $APP_PATH/${APP}_core.cpp $APP_PATH/${APP}_procedure.cpp embed_main.cxx "${LG_LIB}/liblgraph.so"
fi
