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
 * author mengyu.lmy <mengyu.lmy@antgroup.com>
 *
 */

#ifndef FRONTEND_ISOGQL_PARSER_TEST_GQLPARSERTEST_H_
#define FRONTEND_ISOGQL_PARSER_TEST_GQLPARSERTEST_H_

#include <unordered_map>
#include <vector>

#include "geax-front-end/GEAXErrorCode.h"

namespace geax {
namespace frontend {

using geax::frontend::GEAXErrorCode;

class TestFile final {
public:
    TestFile(const std::string& testName, const std::string& testPath)
        : testName_(testName), testPath_(testPath) {}

public:
    std::string testName_;
    std::string testPath_;
};

class GQLCase final {
public:
    GQLCase() = default;
    ~GQLCase() = default;

    void reset() {
        error_ = false;
        gql_.clear();
        lineNo_ = 0;
    }

public:
    bool error_{false};
    std::string gql_;
    size_t lineNo_{0};
};

class GQLParserTest final {
public:
    GQLParserTest() = default;
    ~GQLParserTest() = default;

    GEAXErrorCode test(const std::string& testdir);
    GEAXErrorCode run(const std::string& testPath);

private:
    GEAXErrorCode parseTest(const std::string& testPath);
    GEAXErrorCode testGQLs();
    std::vector<std::string> listAllFilesInDir(const char* dirname, const char* pattern);
    void dividePath(const std::string& path, std::string& parent, std::string& child);
    GEAXErrorCode loadTestFile(const char* rootPath, const char* pattern,
                               std::unordered_map<int32_t, TestFile>& testFileMap);

private:
    std::vector<GQLCase> gqlCases_;
    static constexpr const char* testFilePattern = ".test";
};
}  // end of namespace frontend
}  // end of namespace geax

#endif  // FRONTEND_ISOGQL_PARSER_TEST_GQLPARSERTEST_H_
