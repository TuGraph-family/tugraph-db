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
 *         lili <liangjingru.ljr@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_STMT_FULLTRANSACTION_H_
#define GEAXFRONTEND_AST_STMT_FULLTRANSACTION_H_

#include "geax-front-end/ast/stmt/NormalTransaction.h"
#include "geax-front-end/ast/stmt/StartTransaction.h"
#include "geax-front-end/ast/stmt/Transaction.h"

namespace geax {
namespace frontend {

class FullTransaction : public Transaction {
public:
    FullTransaction()
        : Transaction(AstNodeType::kFullTransaction),
          startTransaction_(nullptr) {}
    ~FullTransaction() = default;

    void setStartTransaction(StartTransaction* startTransaction) {
        startTransaction_ = startTransaction;
    }
    StartTransaction* startTransaction() const { return startTransaction_; }

    void setNormalTransaction(NormalTransaction* normalTransaction) {
        normalTransaction_ = normalTransaction;
    }
    const std::optional<NormalTransaction*>& normalTransaction() const {
        return normalTransaction_;
    }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    StartTransaction* startTransaction_;
    std::optional<NormalTransaction*> normalTransaction_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_FULLTRANSACTION_H_
