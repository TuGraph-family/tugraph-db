#!/bin/bash

APP=$1
INCLUDE_PATH=../build/output

cython -+ -3 cython/$APP.py -o $APP.cpp
gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -D_GNU_SOURCE -fPIC -fwrapv -fPIC -I../deps/fma-common -I../src -I../include -I/usr/include/python3.6m -c $APP.cpp -o $APP.o -Wall -g -fno-gnu-unique -fPIC --std=c++17 -rdynamic -O3 -fopenmp
g++ -pthread -shared -Wl,-z,relro -g $APP.o -L$INCLUDE_PATH -L/usr/lib64 -llgraph -lpython3.6m -o $APP.so -Wall -g -fno-gnu-unique -fPIC --std=c++17 -rdynamic -O3 -fopenmp
rm $APP.o
rm $APP.cpp
