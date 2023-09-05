#!/bin/bash

APP=$1
APP_PATH=algo_cpp
LG_INCLUDE_PATH="-I../include -I../src -I../deps/install/include"
SRC="../src/lgraph_api/olap_base.cpp ../src/lgraph_api/lgraph_utils.cpp ../src/lgraph_api/olap_profile.cpp"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  g++ -fno-gnu-unique -fPIC -g --std=c++17 $LG_INCLUDE_PATH -rdynamic -O3 -fopenmp -DNDEBUG -o $APP_PATH/${APP}_standalone $APP_PATH/${APP}_core.cpp $APP_PATH/${APP}_standalone.cpp $SRC -lrt -lstdc++fs
elif [[ "$OSTYPE" == "darwin"* ]]; then
  clang++ -stdlib=libc++ -fPIC -g --std=c++17 $LG_INCLUDE_PATH -rdynamic -O3 -Xpreprocessor -fopenmp -lomp -DNDEBUG -o $APP_PATH/${APP}_standalone $APP_PATH/${APP}_core.cpp $APP_PATH/${APP}_standalone.cpp $SRC -lstdc++fs
fi
