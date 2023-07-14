/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#ifndef GPC_PASSES_PARTIALPRELOAD_H
#define GPC_PASSES_PARTIALPRELOAD_H

#include "third_party/Halide/include/Halide.h"

/*
* After partialfuse, we preload the `buf` only in the first partialfuse loop.
* Also, the k dim split only happens in the first loop.
  // range 0 could be splitted
  for (k.x, 0, range0) {
        preload_buffer[k.x] = buf[k.x]  // we can parallel preload the buf.
  }
  for (k.x, 0, range0) {
        ...fused stmt... use preload_buffer ...
  }
  for (k.x, o, range1 - range0) {
        ...stmt... no preload ...
  }
  NOTE: for simplity, we only do the partial preload in the fisrt partialfuse
for loop.
*/
void AddPartialPreloadPass(Halide::Func &func, std::vector<std::string> names,
                           Halide::Target target, int k_tile_size = 2,
                           int m_tile_size = 2, int m_thread_size = 1,
                           int n_thread_size = 1);

#endif // GPC_PASSES_PARTIALPRELOAD_H
