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

#ifndef GPC_PASSES_TRACECOMPUTE_H
#define GPC_PASSES_TRACECOMPUTE_H

#include "third_party/Halide/include/Halide.h"

// Halide has a bug:
// If the tiling happens in the outer dimension
// it will occur many redundant compuations.

// Solution:
// Becase the stores to the results are precise
// so we use trace the stores to remove redundant
// computations.
// Algo:
// for every result(`intm` to global):
//   trace result to trace_list
// for every compute(store to `intm`):
//   if compute not in trace_list:
//		remove the compute
// for every compute:
//	 re-index so every index is correct and the size of `intm` allocation is
// shrinked.
void AddRemoveRedundantComputePass(Halide::Func &func);

#endif // GPC_PASSES_TRACECOMPUTE_H