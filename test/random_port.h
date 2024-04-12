/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include <random>

#ifdef _MSC_VER
// visual c++ does not have rand_r() function
struct RandomSeed {
    explicit RandomSeed(int s = 0) : mt(s), dist(1, RAND_MAX) {}

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
