g++ -fopenmp -std=c++11 -I../deps/install/include/fma-common -I../include -O3 -g -o $1 $1.cpp ../build/liblgraph.so
