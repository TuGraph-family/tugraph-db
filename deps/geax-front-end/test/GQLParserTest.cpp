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
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#include "GQLParserTest.h"

#include <dirent.h>
#include <stdio.h>

#include <algorithm>
#include <queue>
#include <string>

#include "folly/Range.h"
#include "folly/String.h"
#include "geax-front-end/utils/Logging.h"
#include "geax-front-end/isogql/parser/GqlLexer.h"
#include "geax-front-end/isogql/parser/GqlParser.h"

namespace geax {
namespace frontend {

GEAXErrorCode GQLParserTest::test(const std::string& testdir) {
    GEAXErrorCode ret = GEAXErrorCode::GEAX_SUCCEED;
    std::unordered_map<int32_t, TestFile> testFileMap;
    if (GEAX_RET_FAIL(loadTestFile(testdir.c_str(), testFilePattern, testFileMap))) {
        LOG(ERROR) << "failed to loadTestFile. " << K(ret);
    } else {
        for (auto& cell : testFileMap) {
            if (GEAX_RET_FAIL(run(cell.second.testPath_))) {
                LOG(ERROR) << "failed to run. " << KM(cell.second.testName_, ret);
                break;
            }
        }
    }
    return ret;
}

GEAXErrorCode GQLParserTest::run(const std::string& testPath) {
    GEAXErrorCode ret = GEAXErrorCode::GEAX_SUCCEED;
    LOG(INFO) << "begin run TEST: " << testPath;
    if (GEAX_RET_FAIL(parseTest(testPath))) {
        LOG(ERROR) << "failed to parseTest. " << K(ret);
    } else if (GEAX_RET_FAIL(testGQLs())) {
        LOG(ERROR) << "failed to testGQL. " << K(ret);
    } else {
        LOG(INFO) << "succeed run TEST: " << testPath;
    }
    return ret;
}

GEAXErrorCode GQLParserTest::parseTest(const std::string& testPath) {
    GEAXErrorCode ret = GEAXErrorCode::GEAX_SUCCEED;

    std::string line;
    std::ifstream testfile(testPath);
    size_t lineNo = 1;
    bool multiLine = false;
    GQLCase case_;
    while (std::getline(testfile, line)) {
        auto onExit = folly::makeGuard([&]() { lineNo++; });
        folly::StringPiece str = folly::trimWhitespace(line);
        if (str.empty()) {
            continue;
        } else if (str.startsWith("#")) {
            continue;
        } else if (str.startsWith("--error")) {
            case_.error_ = true;
        } else if (str.startsWith("```")) {
            multiLine = !multiLine;
            if (!case_.gql_.empty()) {
                case_.lineNo_ = lineNo;
                gqlCases_.emplace_back(case_);
                case_.reset();
            }
        } else {
            case_.gql_ += std::move(line) + " ";
            if (!multiLine) {
                case_.lineNo_ = lineNo;
                gqlCases_.emplace_back(case_);
                case_.reset();
            }
        }
    }
    return ret;
}

GEAXErrorCode GQLParserTest::testGQLs() {
    GEAXErrorCode ret = GEAXErrorCode::GEAX_SUCCEED;
    for (auto& case_ : gqlCases_) {
        antlr4::ANTLRInputStream input(case_.gql_.data(), case_.gql_.size());
        parser::GqlLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        parser::GqlParser parser(&tokens);
        parser.removeErrorListeners();
        parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());
        parser.getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(
            antlr4::atn::PredictionMode::SLL);
        try {
            parser.gqlRequest();
        } catch (std::exception& ex) {
            tokens.reset();
            parser.reset();
            parser.addErrorListener(&antlr4::ConsoleErrorListener::INSTANCE);
            parser.setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());
            parser.getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(
                antlr4::atn::PredictionMode::LL);
            parser.gqlRequest();
        }

        if (static_cast<bool>(parser.getNumberOfSyntaxErrors()) != case_.error_) {
            ret = GEAXErrorCode::GEAX_ERROR;
            LOG(ERROR) << KV("lineNo", case_.lineNo_) << DKV("gql", case_.gql_)
                       << DKV("actualErr", parser.getNumberOfSyntaxErrors())
                       << DKV("expectErr", case_.error_) << DK(ret);
            break;
        }
    }

    return ret;
}

std::vector<std::string> GQLParserTest::listAllFilesInDir(const char* dirname,
                                                          const char* pattern) {
    std::vector<std::string> files;
    DIR* dir{opendir(dirname)};
    // check if the path is valid
    if (dir == nullptr) {
        LOG(ERROR) << "Failed to read the directory \"" << dirname << "\": " << strerror(errno);
        return files;
    }
    dirent* f_ptr{nullptr};
    while ((f_ptr = readdir(dir)) != nullptr) {
        if (strcmp(strrchr(f_ptr->d_name, '.'), pattern) == 0) {
            std::string fname = f_ptr->d_name;
            std::string dname = dirname;
            files.push_back(dname + fname);
        }
    }
    closedir(dir);
    return files;
}

void GQLParserTest::dividePath(const std::string& path, std::string& parent, std::string& child) {
    if (path.empty() || path == "/") {
        // The given string is empty or just "/"
        parent = "";
        child = path;
        return;
    }

    auto startPos = (path.back() == '/' ? path.size() - 2 : path.size() - 1);
    auto pos = path.rfind('/', startPos);
    if (pos == std::string::npos) {
        // Not found
        parent = "";
        child = std::string(path, 0, startPos + 1);
        return;
    }

    // Found the last "/"
    child = std::string(path, pos + 1, startPos - pos);
    if (pos == 0) {
        // In the root directory
        parent = "/";
    } else {
        parent = std::string(path, 0, pos);
    }

    return;
}

GEAXErrorCode GQLParserTest::loadTestFile(const char* rootPath, const char* pattern,
                                          std::unordered_map<int32_t, TestFile>& testFileMap) {
    GEAXErrorCode ret = GEAXErrorCode::GEAX_SUCCEED;
    std::vector<std::string> files = listAllFilesInDir(rootPath, pattern);
    std::sort(files.begin(), files.end());

    std::string parent;
    std::string child;
    std::vector<std::string> data;
    for (std::string& file : files) {
        LOG(INFO) << "Found CASE: " << file;
        parent.clear();
        child.clear();
        data.clear();

        dividePath(file, parent, child);
        folly::split("_", child, data, true);
        if (data.size() < 2) {
            ret = GEAXErrorCode::GEAX_ERROR;
            LOG(ERROR) << "Found invalid case. " << KM(file, ret);
            break;
        } else {
            try {
                TestFile testFile(child, file);
                int32_t id = folly::to<int32_t>(data[0]);
                if (!testFileMap.emplace(id, testFile).second) {
                    ret = GEAXErrorCode::GEAX_ERROR;
                    LOG(ERROR) << "found duplicate id. " << KM(id, file, ret);
                    break;
                }
            } catch (...) {
                ret = GEAXErrorCode::GEAX_ERROR;
                LOG(ERROR) << "Invalid int32: " << data[0] << DK(ret);
                break;
            }
        }
    }
    return ret;
}
}  // end of namespace frontend
}  // end of namespace geax
