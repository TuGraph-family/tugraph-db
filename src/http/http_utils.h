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
 */

#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace lgraph {
namespace http {

typedef std::string string_t;  // add by cpplint

static const string_t HTTP_USER_NAME = "userName";
static const string_t HTTP_PASSWORD = "password";
static const string_t HTTP_AUTHORIZATION = "authorization";
static const string_t HTTP_GRAPH = "graph";
static const string_t HTTP_SCRIPT = "script";
static const string_t HTTP_TIMEOUT = "timeout";
static const string_t HTTP_JSON_FORMAT = "jsonFormat";
static const string_t HTTP_NAME = "name";
static const string_t HTTP_TYPE = "type";
static const string_t HTTP_HEADER = "header";
static const string_t HTTP_RESULT = "result";
static const string_t HTTP_SIZE = "size";
static const string_t HTTP_ELAPSED = "elapsed";
static const string_t HTTP_FILE_DIRECTORY = "upload_files";
static const string_t HTTP_FILE_NAME = "fileName";
static const string_t HTTP_FLAG = "flag";
static const string_t HTTP_SCHEMA = "schema";
static const string_t HTTP_DESCRIPTION = "description";
static const string_t HTTP_DELIMITER = "delimiter";
static const string_t HTTP_CONTINUE_ON_ERROR = "continueOnError";
static const string_t HTTP_SKIP_PACKAGES = "skipPackages";
static const string_t HTTP_TASK_ID = "taskId";
static const string_t HTTP_PROGRESS = "progress";
static const string_t HTTP_SUCCESS = "success";
static const string_t HTTP_ERROR_CODE = "errorCode";
static const string_t HTTP_ERROR_MESSAGE = "errorMessage";
static const string_t HTTP_DATA = "data";
static const string_t HTTP_REASON = "reason";
static const string_t HTTP_FILES = "files";
static const string_t HTTP_PATH = "path";
static const string_t HTTP_PROGRESS_STATE = "state";
static const string_t HTTP_HEADER_FILE_NAME = "File-Name";
static const string_t HTTP_HEADER_BEGIN_POS = "Begin-Pos";
static const string_t HTTP_HEADER_SIZE = "Size";
static const uint32_t HTTP_SPECIFIED_FILE = 0;
static const uint32_t HTTP_SPECIFIED_USER = 1;
static const uint32_t HTTP_ALL_USER = 2;

template <typename Type>
inline bool ExtractTypedField(const std::string& input,
                              const string_t& field,
                              Type& value) {
    try {
        nlohmann::json js = nlohmann::json::parse(input);
        if (!js.contains(field)) return false;
        value = js[field].get<Type>();
    } catch (const std::exception& exception) {
        throw lgraph_api::BadRequestException(FMA_FMT("{} {}", field, exception.what()));
    }
    return true;
}

inline std::string GetRandomUuid() {
    boost::uuids::uuid id = boost::uuids::random_generator()();
    return boost::uuids::to_string(id);
}

}  // end of namespace http
}  // end of namespace lgraph
