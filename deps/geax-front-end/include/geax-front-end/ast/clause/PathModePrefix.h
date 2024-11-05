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

#ifndef GEAXFRONTEND_AST_CLAUSE_PATHMODEPREFIX_H_
#define GEAXFRONTEND_AST_CLAUSE_PATHMODEPREFIX_H_

#include "geax-front-end/ast/clause/PathPrefix.h"

namespace geax {
namespace frontend {

enum class ModeType : uint8_t {
    kWalk,
    kTrail,
    kAcyclic,
    kSimple,
    kMax,
};

inline const char* ToString(const ModeType type) {
    static const StrArray<enumNum(ModeType::kMax)> kArray = {
        "walk",
        "trail",
        "acyclic",
        "simple",
    };
    const auto idx = static_cast<size_t>(type);
    return idx < kArray.size() ? kArray[idx] : geax::frontend::kUnknown;
}

inline bool ToEnum(std::string_view sv, ModeType& mode) {
    static const std::unordered_map<std::string_view, ModeType> kMap = {
        {"walk", ModeType::kWalk},
        {"acyclic", ModeType::kAcyclic},
        {"simple", ModeType::kSimple},
        {"trail", ModeType::kTrail},
    };
    auto it = kMap.find(sv);
    return it == kMap.end() ? false : (mode = it->second, true);
}

class PathModePrefix : public PathPrefix {
public:
    PathModePrefix() : PathPrefix(AstNodeType::kPathModePrefix) {}
    ~PathModePrefix() = default;

    void setPathMode(ModeType mode) { mode_ = mode; }
    ModeType pathMode() const { return mode_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    ModeType mode_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_PATHMODEPREFIX_H_
