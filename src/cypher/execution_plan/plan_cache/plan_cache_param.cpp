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
#include "cypher/execution_plan/plan_cache/plan_cache_param.h"

namespace cypher {
std::string fastQueryParam(RTContext *ctx, const std::string query) {
    antlr4::ANTLRInputStream input(query);
    parser::LcypherLexer lexer(&input);
    antlr4::CommonTokenStream token_stream(&lexer);
    token_stream.fill();

    std::vector<antlr4::Token *> tokens = token_stream.getTokens();
    size_t delete_size = 0;
    std::string param_query = query;
    
    bool prev_limit_skip = false;
    int param_num = 0;
    if (tokens[0]->getType() == parser::LcypherParser::CALL) {
        // Don't parameterize plugin CALL statements
        return query;
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        parser::Expression expr;
        bool is_param;
        switch (tokens[i]->getType())
        {
        case parser::LcypherParser::StringLiteral: {
            // String literal
            auto str = tokens[i]->getText();
            std::string res;
            // remove escape character
            for (size_t i = 1; i < str.length() - 1; i++) {
                if (str[i] == '\\') {
                    i++;
                }
                res.push_back(str[i]);
            }
            expr.type = parser::Expression::STRING;
            expr.data = std::make_shared<std::string>(std::move(res));
            ctx->param_tab_.emplace("$" + std::to_string(param_num), MakeFieldData(expr));
            is_param = true;
            break;
        }
        case parser::LcypherParser::HexInteger:
        case parser::LcypherParser::DecimalInteger:
        case parser::LcypherParser::OctalInteger: {
            if (prev_limit_skip) {
                break;
            }
            // Integer literal
            expr.type = parser::Expression::DataType::INT;
            expr.data = std::stol(tokens[i]->getText());
            ctx->param_tab_.emplace("$" + std::to_string(param_num), MakeFieldData(expr));
            is_param = true;
            break;
        }
        case parser::LcypherParser::ExponentDecimalReal:
        case parser::LcypherParser::RegularDecimalReal: {
            // Double literal
            expr.type = parser::Expression::DataType::DOUBLE;
            expr.data = std::stod(tokens[i]->getText());
            ctx->param_tab_.emplace("$" + std::to_string(param_num), MakeFieldData(expr));
            is_param = true;
            break;
        }
        case parser::LcypherParser::TRUE_: {
            expr.type = parser::Expression::DataType::BOOL;
            expr.data = true;
            ctx->param_tab_.emplace("$" + std::to_string(param_num), MakeFieldData(expr));
            is_param = true;
            break;
        }
        case parser::LcypherParser::FALSE_: {
            expr.type = parser::Expression::DataType::BOOL;
            expr.data = false;
            ctx->param_tab_.emplace("$" + std::to_string(param_num), MakeFieldData(expr));
            is_param = true;
            break;
        }
        default:
            break;
        }

        // Replace the token with placeholder
        if (is_param) {
            size_t start_index = tokens[i]->getStartIndex() - delete_size;
            size_t end_index = tokens[i]->getStopIndex() - delete_size;
            // Indicate the position in raw parameterized query
            std::string count = "$" + std::to_string(param_num);
            param_query.replace(start_index, end_index - start_index + 1, count);
            delete_size += (end_index - start_index + 1) - count.size();
            is_param = false;
            param_num++;
        }
        if (tokens[i]->getType() == parser::LcypherParser::LIMIT ||
            tokens[i]->getType() == parser::LcypherParser::L_SKIP) {
            prev_limit_skip = true;
        } else if (tokens[i]->getType() < parser::LcypherParser::SP) {
            prev_limit_skip = false;
        }
    }
    return param_query;
}
}