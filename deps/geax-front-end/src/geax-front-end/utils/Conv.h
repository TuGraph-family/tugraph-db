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


#ifndef FRONTEND_UTILS_CONV_H_
#define FRONTEND_UTILS_CONV_H_

namespace geax {
namespace utils {

template <typename B, typename D>
bool checkedCast(B* b, D*& d) {
    static_assert(std::is_base_of<B, D>::value, "type `B` must be the base of type `D`");
    d = dynamic_cast<D*>(b);
    return d != nullptr;
}

template <typename B, typename D>
bool uncheckedCast(B* b, D*& d) {
    static_assert(std::is_base_of<B, D>::value, "type `B` must be the base of type `D`");
    d = static_cast<D*>(b);
    return d != nullptr;
}

}  // namespace utils
}  // namespace geax

#endif  // FRONTEND_UTILS_CONV_H_
