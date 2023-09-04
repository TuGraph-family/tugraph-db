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

#ifndef GEAXFRONTEND_AST_CLAUSE_PATHSEARCHPREFIX_H_
#define GEAXFRONTEND_AST_CLAUSE_PATHSEARCHPREFIX_H_

#include "geax-front-end/ast/clause/PathPrefix.h"

namespace geax {
namespace frontend {

enum class SearchType : uint8_t {
    kAllSearch,
    kAnySearch,
    kAllShortest,
    kAnyShortest,
    kCountedShortestPaths,
    kCountedShortestGroups,
    kMax,
};

inline const char* ToString(const SearchType type) {
    static const StrArray<enumNum(SearchType::kMax)> kArray = {
        "allSearch",
        "anySearch",
        "allShortest",
        "anyShortest",
        "countedShortestPaths",
        "countedShortestGroups",
    };
    const auto idx = static_cast<size_t>(type);
    return idx < kArray.size() ? kArray[idx] : geax::frontend::kUnknown;
}

inline bool ToEnum(std::string_view sv, SearchType& type) {
    static const std::unordered_map<std::string_view, SearchType> kMap = {
        {"allSearch", SearchType::kAllSearch},
        {"anySearch", SearchType::kAnySearch},
        {"allShortest", SearchType::kAllShortest},
        {"anyShortest", SearchType::kAnyShortest},
        {"countedShortestPaths", SearchType::kCountedShortestPaths},
        {"countedShortestGroups", SearchType::kCountedShortestGroups},
    };
    auto it = kMap.find(sv);
    return it == kMap.end() ? false : (type = it->second, true);
}

class PathSearchPrefix : public PathPrefix {
public:
    PathSearchPrefix() : PathPrefix(AstNodeType::kPathSearchPrefix) {}
    ~PathSearchPrefix() = default;

    void setNum(int64_t num) { num_ = num; }
    const std::optional<int64_t>& num() const { return num_; }

    void setSearchType(SearchType searchType) { searchType_ = searchType; }
    SearchType searchType() const { return searchType_; }

    void setSearchMode(ModeType searchMode) { searchMode_ = searchMode; }
    const std::optional<ModeType>& searchMode() const { return searchMode_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::optional<int64_t> num_;
    SearchType searchType_;
    std::optional<ModeType> searchMode_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_PATHSEARCHPREFIX_H_
