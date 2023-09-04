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

#ifndef GEAXFRONTEND_AST_CLAUSE_READCONSISTENCY_H_
#define GEAXFRONTEND_AST_CLAUSE_READCONSISTENCY_H_

#include "geax-front-end/ast/clause/Hint.h"

namespace geax {
namespace frontend {

class ReadConsistency : public Hint {
public:
    ReadConsistency() : Hint(AstNodeType::kReadConsistency) {}
    ~ReadConsistency() = default;

    void setVal(std::string&& val) { val_ = std::move(val); }
    const std::string& val() const { return val_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::string val_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_READCONSISTENCY_H_
