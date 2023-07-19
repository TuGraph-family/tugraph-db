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

#include "parser/cypher_error_listener.h"
#include "cypher/cypher_exception.h"
using namespace parser;

CypherErrorListener CypherErrorListener::INSTANCE;

void CypherErrorListener::syntaxError(antlr4::Recognizer *recognizer,
                                      antlr4::Token *offendingSymbol, size_t line,
                                      size_t charPositionInLine, const std::string &msg,
                                      std::exception_ptr e) {
    std::string report;
    report.append("line ")
        .append(std::to_string(line))
        .append(":")
        .append(std::to_string(charPositionInLine))
        .append(" ")
        .append(msg);
    throw lgraph::ParserException(report);
}
