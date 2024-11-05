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

#include <exception>
#include <string>
#include "common/exceptions.h"

namespace lgraph {

#ifndef NDEBUG
#define CYPHER_TODO_FILE_NAME std::string(__FILE__)
#else
#define CYPHER_TODO_FILE_NAME ""
#endif

#define CYPHER_TODO() \
    THROW_CODE(CypherException, "Function not implemented yet: {} at {}:{}", __func__, CYPHER_TODO_FILE_NAME, __LINE__)

#define CYPHER_INTL_ERR() \
    THROW_CODE(CypherException, "Internal error: {} at {}:{}", __func__, CYPHER_TODO_FILE_NAME, __LINE__)

#define CYPHER_THROW_ASSERT(pred)       \
    do {                                \
        if (!(pred)) CYPHER_INTL_ERR(); \
    } while (0)

#define CYPHER_PARSER_CHECK(pred, msg)    \
    if (!(pred)) {                        \
        THROW_CODE(ParserException, msg); \
    }
#define CYPHER_ARGUMENT_ERROR() \
    THROW_CODE(CypherException, "There are errors in the number of arguments or the type of arguments of function {}.", __func__)


}  // namespace lgraph
