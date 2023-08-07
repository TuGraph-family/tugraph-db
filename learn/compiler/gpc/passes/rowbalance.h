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

#ifndef GPC_PASSES_ROWBALANCE_H
#define GPC_PASSES_ROWBALANCE_H

#include "third_party/Halide/include/Halide.h"

// Row balance with sorted row indices.
// replace each row index with the value of the row indices array.
// row_idx => row_indices[row_idx]
// may occure load and store.
void AddRowBalancePass(Halide::Func &func, int tile_thread_y,
                       bool do_row_balance);

#endif // GPC_PASSES_ROWBALANCE_H