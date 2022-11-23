/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <random>

#ifdef _MSC_VER
// visual c++ does not have rand_r() function
struct RandomSeed {
    explicit RandomSeed(int s) : mt(s), dist(1, RAND_MAX) {}

    int Rand() { return dist(mt); }

    std::mt19937 mt;
    std::uniform_int_distribution<int> dist;
};
inline int rand_r(RandomSeed* seed) { return seed->Rand(); }
#else
typedef unsigned int RandomSeed;
#endif

// a replacement for rand() just to pass cpplint
// if you need thread-safe one, use rand_r(&seed) with different seeds per thread
inline int myrand() {
    static RandomSeed seed(0);
    return rand_r(&seed);
}
