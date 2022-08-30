/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "BaseErrorListener.h"

namespace parser {

class CypherErrorListener : public antlr4::BaseErrorListener {
 public:
    /**
     * Provides a default instance of {@link CypherErrorListener}.
     */
    static CypherErrorListener INSTANCE;

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
    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string &msg,
                     std::exception_ptr e) override;
};

}  // namespace parser
