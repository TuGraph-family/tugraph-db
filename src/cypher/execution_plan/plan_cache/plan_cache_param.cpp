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
    /**
     * We don't parameterize the queries or literals in query if:
     * 1. The query is a CALL statement.
     * 2. limit/skip `n`.
     * 3. Range literals: ()->[e*..3]->(m)
     * 4. the items in return body: return RETURN a,-2,9.78,'im a string' (@todo)
     * 5. match ... create: MATCH (c {name:$0}) CREATE (p:Person {name:$1, birthyear:$2})-[r:BORN_IN]->(c) RETURN p,r,c
     */
    antlr4::ANTLRInputStream input(query);
    parser::LcypherLexer lexer(&input);
    antlr4::CommonTokenStream token_stream(&lexer);
    token_stream.fill();

    std::vector<antlr4::Token *> tokens = token_stream.getTokens();
    size_t delete_size = 0;
    std::string param_query = query;

    bool prev_limit_skip = false;
    bool in_return_body = false;
    bool prev_double_dots = false;  // e*..3
    bool in_rel = false;  // -[n]->
    int param_num = 0;
    if (tokens[0]->getType() == parser::LcypherParser::CALL) {
        // Don't parameterize plugin CALL statements
        return query;
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        parser::Expression expr;
        bool is_param;
        switch (tokens[i]->getType()) {
        case parser::LcypherParser::CREATE: {
            // We don't parameterize the Create statements
            // Remove the parsed parameters.
            for (auto it = ctx->param_tab_.begin(); it!= ctx->param_tab_.end(); ) {
                if (it->first[0] == '$' && std::isdigit(it->first[1])) {
                    it = ctx->param_tab_.erase(it);
                } else {
                    ++it;
                }
            }
            return query;
        }
        case parser::LcypherParser::T__13: {  // '-'
            size_t j = i;
            while (++j < tokens.size() && tokens[j]->getType() == parser::LcypherParser::SP) {
            }
            if (j < tokens.size() && tokens[j]->getType() == parser::LcypherParser::T__7) {
                in_rel = true;
            }
            i = j;
            break;
        }
        case parser::LcypherParser::T__8: {  // ']'
            in_rel = false;
            break;
        }
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
            if (in_rel) {
                // The integer literals in relationships are range literals.
                // -[:HAS_CHILD*1..]->
                break;
            }
            if (prev_limit_skip || prev_double_dots) {
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
        case parser::LcypherParser::RETURN: {
            in_return_body = true;
            break;
        }
        default:
            break;
        }

        // Replace the token with placeholder
        if (is_param) {
            if (!in_return_body) {
                size_t start_index = tokens[i]->getStartIndex() - delete_size;
                size_t end_index = tokens[i]->getStopIndex() - delete_size;
                // Indicate the position in raw parameterized query
                std::string count = "$" + std::to_string(param_num);
                param_query.replace(start_index, end_index - start_index + 1, count);
                delete_size += (end_index - start_index + 1) - count.size();
                param_num++;
            }
            is_param = false;
        }
        if (tokens[i]->getType() == parser::LcypherParser::LIMIT ||
            tokens[i]->getType() == parser::LcypherParser::L_SKIP) {
            prev_limit_skip = true;
        } else if (tokens[i]->getType() == parser::LcypherParser::T__11) {
            prev_double_dots = true;
        } else if (tokens[i]->getType() < parser::LcypherParser::SP) {
            prev_limit_skip = false;
            prev_double_dots = false;
        }
    }
    return param_query;
}
}  // namespace cypher
