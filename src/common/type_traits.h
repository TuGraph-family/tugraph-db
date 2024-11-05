/**
* Copyright 2024 AntGroup CO., Ltd.
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

#define DISABLE_COPY(class_name)             \
    class_name(const class_name &) = delete; \
    class_name &operator=(const class_name &) = delete;

#define DISABLE_MOVE(class_name)        \
    class_name(class_name &&) = delete; \
    class_name &operator=(class_name &&) = delete;

#define LIKELY(x) __builtin_expect(!!(x), 1)