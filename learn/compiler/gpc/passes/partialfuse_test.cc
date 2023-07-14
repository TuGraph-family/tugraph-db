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

#include "compiler/gpc/passes/partialfuse.h"

#include "third_party/Halide/include/Halide.h"

using namespace Halide;

int main(int argc, char const *argv[]) {
    Func f;
    Var x;
    f(x) = x / 1.0f;
    AddPartialfusePass(f);
    f.realize({10});
    return 0;
}