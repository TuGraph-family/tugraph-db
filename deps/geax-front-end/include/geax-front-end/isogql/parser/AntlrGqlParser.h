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

#ifndef FRONTEND_ISOGQL_PARSER_ANTLRGQLPARSER_H_
#define FRONTEND_ISOGQL_PARSER_ANTLRGQLPARSER_H_

#include "BaseErrorListener.h"
#include "geax-front-end/isogql/parser/GqlLexer.h"
#include "geax-front-end/isogql/parser/GqlParser.h"

namespace geax {
namespace frontend {

class AntlrGqlParser : public antlr4::BaseErrorListener {
public:
    explicit AntlrGqlParser(const std::string& query)
        : query_(query),
          input_(query_),
          lexer_(&input_),
          tokens_(&lexer_),
          parser_(&tokens_),
          rule_(nullptr) {}

    parser::GqlParser::GqlRequestContext* gqlRequest() {
        if (!tryParseBySLLMode() && !tryParseByLLMode()) {
            rule_ = nullptr;
        }
        return rule_;
    }

    antlr4::ANTLRInputStream& input() { return input_; }

    parser::GqlLexer& lexer() { return lexer_; }

    const std::string& query() { return query_; }

    std::string& error() { return errorMsg_; }

private:
    bool tryParseBySLLMode();

    bool tryParseByLLMode();
    /**
     * {@inheritDoc}
     *
     * <p>
     * This implementation prints messages to {@link System#err} containing the
     * values of {@code line}, {@code charPositionInLine}, and {@code msg} using
     * the following format.</p>
     *
     * <pre>
     * line <em>line</em>:<em>charPositionInLine</em> <em>msg</em>
     * </pre>
     */
    void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string& msg,
                     std::exception_ptr e) override;

private:
    const std::string& query_;
    antlr4::ANTLRInputStream input_;
    parser::GqlLexer lexer_;
    antlr4::CommonTokenStream tokens_;
    parser::GqlParser parser_;
    std::string errorMsg_;
    parser::GqlParser::GqlRequestContext* rule_;
};

}  // namespace frontend
}  // namespace geax

#endif  // FRONTEND_ISOGQL_PARSER_ANTLRGQLPARSER_H_
