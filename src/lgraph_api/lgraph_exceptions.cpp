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
#include "lgraph/lgraph_exceptions.h"

namespace lgraph_api {

const char* ErrorCodeToString(ErrorCode code) {
    switch (code) {
#define X(code, msg) case ErrorCode::code: return #code;
        ERROR_CODES
#undef X
    default: return "Unknown Error Code";
    }
}

const char* ErrorCodeDesc(ErrorCode code) {
    switch (code) {
#define X(code, msg) case ErrorCode::code: return msg;
        ERROR_CODES
#undef X
    default: return "Unknown Error Code";
    }
}

LgraphException::LgraphException(ErrorCode code)
    : code_(code), msg_(ErrorCodeDesc(code)) {
    what_ = FMA_FMT("[{}] {}", ErrorCodeToString(code_), msg_);
}

LgraphException::LgraphException(ErrorCode code, const std::string& msg)
    : code_(code), msg_(msg) {
    what_ = FMA_FMT("[{}] {}", ErrorCodeToString(code_), msg_);
}

LgraphException::LgraphException(ErrorCode code, const char* msg)
    : code_(code), msg_(msg) {
    what_ = FMA_FMT("[{}] {}", ErrorCodeToString(code_), msg_);
}

}  // namespace lgraph_api
