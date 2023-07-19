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

#ifndef GPC_PASSES_PARTIALFUSE_H
#define GPC_PASSES_PARTIALFUSE_H

#include "third_party/Halide/include/Halide.h"

void AddPartialfusePass(Halide::Func &func);

inline std::string GetMagicRange() { return "magic_extent"; }

inline std::string GetMagicStart() { return "magic_start"; }

inline std::string GetMagicRowIndices() { return "magic_indices"; }

inline std::string GetMagicRow() { return "magic_r"; }

inline bool RewriteMagicStart() { return true; }

#endif // GPC_PASSES_PARTIALFUSE_H