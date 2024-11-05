/**
* Copyright 2024 AntGroup CO., Ltd.
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
*/

#pragma once

#define EXPECT_THROW_CODE(statement, error_code)                        \
    {                                                                   \
        try {                                                           \
            statement;                                                  \
            FAIL() << "Expecting exception, but nothing is thrown.";    \
        } catch (LgraphException &e) {                                  \
            if (e.code() != ErrorCode::error_code) {                    \
                FAIL() << "Unexpected exception message: " << e.what(); \
            } else {                                                    \
                SUCCEED() << "Expected exception: " << e.what();        \
            }                                                           \
        } catch (std::exception &e) {                                   \
            FAIL() << "Unexpected exception message: " << e.what();     \
        }                                                               \
    }                                                                   \

#define EXPECT_THROW_CODE_MSG(statement, error_code, msg)               \
    {                                                                   \
        try {                                                           \
            statement;                                                  \
            FAIL() << "Expecting exception, but nothing is thrown.";    \
        } catch (LgraphException &e) {                                  \
            if (e.code() != ErrorCode::error_code) {                    \
                FAIL() << "Unexpected exception message: " << e.what(); \
            } else {                                                    \
                std::string what = e.what();                            \
                if (what.find(msg) != what.npos) {                      \
                    SUCCEED() << "Expected exception: " << e.what();    \
                } else {                                                \
                    FAIL() << "Unexpected exception message: " << what; \
                }                                                       \
            }                                                           \
        } catch (std::exception &e) {                                   \
            FAIL() << "Unexpected exception message: " << e.what();     \
        }                                                               \
    }                                                                   \

