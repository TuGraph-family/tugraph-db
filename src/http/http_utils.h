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

// methods in url
static const string_t HTTP_CYPHER_METHOD = "cypher";
static const string_t HTTP_GQL_METHOD = "gql";
static const string_t HTTP_REFRESH_METHOD = "refresh";
static const string_t HTTP_LOGIN_METHOD = "login";
static const string_t HTTP_LOGOUT_METHOD = "logout";
static const string_t HTTP_UPLOAD_METHOD = "upload_files";
static const string_t HTTP_CLEAR_CACHE_METHOD = "clear_cache";
static const string_t HTTP_CHECK_FILE_METHOD = "check_file";
static const string_t HTTP_IMPORT_METHOD = "import_data";
static const string_t HTTP_IMPORT_PROGRESS_METHOD = "import_progress";
static const string_t HTTP_IMPORT_SCHEMA_METHOD = "import_schema";
static const string_t HTTP_UPLOAD_PROCEDURE_METHOD = "upload_procedure";
static const string_t HTTP_LIST_PROCEDURE_METHOD = "list_procedures";
static const string_t HTTP_GET_PROCEDURE_METHOD = "get_procedure";
static const string_t HTTP_GET_PROCEDURE_DEMO_METHOD = "get_procedure_demo";
static const string_t HTTP_DELETE_PROCEDURE_METHOD = "delete_procedure";
static const string_t HTTP_CALL_PROCEDURE_METHOD = "call_procedure";
static const string_t HTTP_CREATE_PROCEDURE_JOB_METHOD = "create_procedure_job";
static const string_t HTTP_LIST_PROCEDURE_JOB_METHOD = "list_procedure_jobs";
static const string_t HTTP_GET_PROCEDURE_JOB_RESULT_METHOD = "get_procedure_job_result";

static const string_t HTTP_USER_NAME = "userName";
static const string_t HTTP_PASSWORD = "password";
static const string_t HTTP_AUTHORIZATION = "authorization";
static const string_t HTTP_DEFAULT_PASSWORD = "default_password";
static const string_t HTTP_GRAPH = "graph";
static const string_t HTTP_GRAPH_NAME = "graphName";
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
static const string_t HTTP_PROCEDURE_DEMO_PATH_DIRECTORY = "/procedure_demo/";
static const string_t HTTP_PROCEDURE_DEMO_PATH_CPP_V1 = "v1_scan_graph.cpp";
static const string_t HTTP_PROCEDURE_DEMO_PATH_CPP_V2 = "v2_pagerank.cpp";
static const string_t HTTP_PROCEDURE_DEMO_PATH_PYTHON = "scan_graph.py";
static const string_t HTTP_PROCEDURE_OUTPUT_TYPE_FILE = "file";
static const string_t HTTP_PROCEDURE_OUTPUT_TYPE_GRAPH = "graph";
static const uint32_t HTTP_SPECIFIED_FILE = 0;
static const uint32_t HTTP_SPECIFIED_USER = 1;
static const uint32_t HTTP_ALL_USER = 2;

#define GET_FIELD_OR_THROW_BAD_REQUEST(req, type, field, value)               \
    do {                                                                      \
        if (!ExtractTypedField<type>(req, field, value)) {                    \
            THROW_CODE(BadRequest, "`{}` is not specified.", field); \
        }                                                                     \
    } while (0)

template <typename Type>
inline bool ExtractTypedField(const std::string& input, const string_t& field, Type& value) {
    try {
        nlohmann::json js = nlohmann::json::parse(input);
        if (!js.contains(field)) return false;
        value = js[field].get<Type>();
    } catch (const std::exception& exception) {
        THROW_CODE(BadRequest, "{} {}", field, exception.what());
    }
    return true;
}

inline std::string GetRandomUuid() {
    boost::uuids::uuid id = boost::uuids::random_generator()();
    return boost::uuids::to_string(id);
}

}  // end of namespace http
}  // end of namespace lgraph
