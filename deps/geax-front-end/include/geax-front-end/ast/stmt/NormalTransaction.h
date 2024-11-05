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

#ifndef GEAXFRONTEND_AST_STMT_NORMALTRANSACTION_H_
#define GEAXFRONTEND_AST_STMT_NORMALTRANSACTION_H_

#include "geax-front-end/ast/stmt/EndTransaction.h"
#include "geax-front-end/ast/stmt/ProcedureBody.h"
#include "geax-front-end/ast/stmt/Transaction.h"

namespace geax {
namespace frontend {

class NormalTransaction : public Transaction {
public:
    NormalTransaction()
        : Transaction(AstNodeType::kNormalTransaction), query_(nullptr) {}
    ~NormalTransaction() = default;

    void setProcedureBody(ProcedureBody* query) { query_ = query; }
    ProcedureBody* query() const { return query_; }

    void setEndTransaction(EndTransaction* endTransaction) { endTransaction_ = endTransaction; }
    const std::optional<EndTransaction*>& endTransaction() const { return endTransaction_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    ProcedureBody* query_;
    std::optional<EndTransaction*> endTransaction_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_NORMALTRANSACTION_H_
