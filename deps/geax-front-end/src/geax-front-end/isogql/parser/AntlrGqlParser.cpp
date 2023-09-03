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
 *
 *  Author:
 *         suosi.zl <suosi.zl@alibaba-inc.com>
 */

#include "geax-front-end/isogql/parser/AntlrGqlParser.h"

#include "geax-front-end/utils/Copilot.h"
#include "geax-front-end/utils/Logging.h"

namespace geax {
namespace frontend {

bool AntlrGqlParser::tryParseBySLLMode() {
    bool ret = true;
    parser_.reset();
    parser_.getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(
        antlr4::atn::PredictionMode::SLL);
    parser_.removeErrorListeners();
    parser_.setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());
    rule_ = parser_.gqlRequest();
    if (GEAX_IS_NULL(rule_) || parser_.getNumberOfSyntaxErrors() > 0) {
        ret = false;
        LOG(INFO) << "Failed to parse query by SLL mode: " << query_;
    }
    return ret;
}

bool AntlrGqlParser::tryParseByLLMode() {
    bool ret = true;
    parser_.reset();
    parser_.addErrorListener(this);
    parser_.setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());
    parser_.getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(
        antlr4::atn::PredictionMode::LL);
    rule_ = parser_.gqlRequest();
    if (GEAX_IS_NULL(rule_) || parser_.getNumberOfSyntaxErrors() > 0) {
        ret = false;
        LOG(INFO) << "Failed to parse query by LL mode: " << query_;
    }
    return ret;
}

void AntlrGqlParser::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
                                 size_t line, size_t charPositionInLine, const std::string& msg,
                                 std::exception_ptr e) {
    errorMsg_.append("line")
        .append(std::to_string(line))
        .append(":")
        .append(std::to_string(charPositionInLine))
        .append(" ")
        .append(msg);
}

}  // namespace frontend
}  // namespace geax
