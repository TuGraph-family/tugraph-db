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

#ifndef GEAXFRONTEND_AST_STMT_STARTTRANSACTION_H_
#define GEAXFRONTEND_AST_STMT_STARTTRANSACTION_H_

#include "geax-front-end/ast/AstNode.h"

namespace geax {
namespace frontend {

enum class TransactionMode : uint8_t {
    kReadOnly,
    kReadWrite,
    kImplDefinedAccess,
    kMax,
};
inline const char* ToString(TransactionMode dir) {
    static const StrArray<enumNum(TransactionMode::kMax)> kNameMap = {"ReadOnly", "ReadWrite",
                                                                      "ImplDefinedAccess"};
    const auto idx = static_cast<size_t>(dir);
    return idx < kNameMap.size() ? kNameMap[idx] : geax::frontend::kUnknown;
}
inline bool ToEnum(std::string_view sv, TransactionMode& dir) {
    static const std::unordered_map<std::string_view, TransactionMode> kDirMap = {
        {"ReadOnly", TransactionMode::kReadOnly},
        {"ReadWrite", TransactionMode::kReadWrite},
        {"ImplDefinedAccess", TransactionMode::kImplDefinedAccess},
    };
    auto it = kDirMap.find(sv);
    return it == kDirMap.end() ? false : (dir = it->second, true);
}

class StartTransaction : public AstNode {
public:
    StartTransaction() : AstNode(AstNodeType::kStartTransaction) {}
    ~StartTransaction() = default;

    void appendMode(TransactionMode mode) { modes_.emplace_back(mode); }
    void setModes(std::vector<TransactionMode>&& modes) { modes_ = std::move(modes); }
    const std::vector<TransactionMode>& modes() const { return modes_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<TransactionMode> modes_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_STARTTRANSACTION_H_
