/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "parser/cypher_error_listener.h"
#include "cypher_exception.h"
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
