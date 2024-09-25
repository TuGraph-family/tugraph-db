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

#include "./antlr4-runtime.h"
#include "parser/generated/LcypherLexer.h"
#include "parser/generated/LcypherParser.h"

#include "execution_plan/runtime_context.h"
#include "parser/clause.h"
#include "parser/expression.h"

namespace cypher {

// Leverage the lexer to parameterize queries
std::string fastQueryParam(RTContext *ctx, const std::string query);
}
