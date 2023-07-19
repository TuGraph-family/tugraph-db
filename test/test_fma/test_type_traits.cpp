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

#include <functional>

#include "fma-common/type_traits.h"
#include "fma-common/unit_test_utils.h"

FMA_SET_TEST_PARAMS(TypeTraits, "");

using namespace fma_common;

class Foo {
 public:
    void Bar(int i, double j) { FMA_LOG() << i << ", " << j; }
};

FMA_UNIT_TEST(TypeTraits) {
    Foo f;
    _detail::ApplyTuple(std::bind(&Foo::Bar, &f, std::placeholders::_1, std::placeholders::_2),
                        std::make_tuple<int, double>(1, 2.3));

    return 0;
}
