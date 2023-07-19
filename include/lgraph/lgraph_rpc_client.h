//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

namespace fma_common {
class Logger;
}

namespace lgraph_rpc {
class m_channel;
class m_controller;
class m_channel_options;
}  // namespace lgraph_rpc

namespace lgraph {

class LGraphRequest;
class LGraphResponse;
class CypherResult;
class CypherResponse;
class BackupLogEntry;

class RpcClient {
 public:
    /**
     * @brief   RpcClient Login.
     *
     * @param   url         Login address.
     * @param   user        The username.
     * @param   password    The password.
     */
    RpcClient(const std::string& url, const std::string& user, const std::string& password);

    ~RpcClient();

    // TODO(jzj)
    int64_t Restore(const std::vector<BackupLogEntry>& requests);

    /**
     * @brief   Load a user-defined procedure
     *
     * @param [out] result                  The result.
     * @param [in]  source_file             the source_file contain procedure code.
     * @param [in]  procedure_type          the procedure type, currently supported CPP and PY.
     * @param [in]  procedure_name          procedure name.
     * @param [in]  code_type               code type, currently supported PY, SO, CPP, ZIP.
     * @param [in]  procedure_description   procedure description.
     * @param [in]  read_only               procedure is read only or not.
     * @param [in]  graph                   (Optional) the graph to query.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool LoadProcedure(std::string& result, const std::string& source_file,
                    const std::string& procedure_type, const std::string& procedure_name,
                    const std::string& code_type, const std::string& procedure_description,
                    bool read_only, const std::string& graph = "default");

    /**
     * @brief   Execute a user-defined procedure
     *
     * @param [out] result              The result.
     * @param [in]  procedure_type      the procedure type, currently supported CPP and PY.
     * @param [in]  procedure_name      procedure name.
     * @param [in]  param               the execution parameters.
     * @param [in]  procedure_time_out  (Optional) Maximum execution time, overruns will be
     *                                  interrupted.
     * @param [in]  in_process          (Optional) support in future.
     * @param [in]  graph               (Optional) the graph to query.
     * @param [in]  json_format         (Optional) Returns the format， true is json，
     *                                  Otherwise, binary format.
     * @returns True if it succeeds, false if it fails.
     */
    bool CallProcedure(std::string& result, const std::string& procedure_type,
                    const std::string& procedure_name, const std::string& param,
                    double procedure_time_out = 0.0, bool in_process = false,
                    const std::string& graph = "default", bool json_format = true);

    /**
     * @brief   List user-defined procedures
     *
     * @param [out] result          The result.
     * @param [in]  procedure_type  (Optional) the procedure type, "" for all procedures,
     *                              CPP and PY for special type.
     * @param [in]  graph           (Optional) the graph to query.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool ListProcedures(std::string& result, const std::string& procedure_type,
                       const std::string& graph = "default");

    /**
     * @brief   Execute a user-defined procedure
     *
     * @param [out] result              The result.
     * @param [in]  procedure_type      the procedure type, currently supported CPP and PY.
     * @param [in]  procedure_name      procedure name.
     * @param [in]  graph               (Optional) the graph to query.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool DeleteProcedure(std::string& result, const std::string& procedure_type,
                       const std::string& procedure_name,
                       const std::string& graph = "default");

    /**
     * @brief   Import vertex or edge schema from file
     *
     * @param [out] result      The result.
     * @param [in]  schema_file the schema_file contain schema.
     * @param [in]  graph       (Optional) the graph to query.
     * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
     *                          format.
     * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool ImportSchemaFromFile(std::string& result, const std::string& schema_file,
                              const std::string& graph = "default", bool json_format = true,
                              double timeout = 0);

    /**
     * @brief   Import vertex or edge data from file
     *
     * @param [out] result              The result.
     * @param [in]  conf_file           data file contain format description and data.
     * @param [in]  delimiter           data separator.
     * @param [in]  continue_on_error   (Optional) whether to continue when importing data fails.
     * @param [in]  thread_nums         (Optional) maximum number of threads.
     * @param [in]  skip_packages       (Optional) skip packages number.
     * @param [in]  graph               (Optional) the graph to query.
     * @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
     *                                  binary format.
     * @param [in]  timeout             (Optional) Maximum execution time, overruns will be
     *                                  interrupted.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool ImportDataFromFile(std::string& result, const std::string& conf_file,
                            const std::string& delimiter, bool continue_on_error = false,
                            int thread_nums = 8, int skip_packages = 0,
                            const std::string& graph = "default", bool json_format = true,
                            double timeout = 0);

    /**
     * @brief   Import vertex or edge schema from content string
     *
     * @param [out] result      The result.
     * @param [in]  schema      the schema to be imported.
     * @param [in]  graph       (Optional) the graph to query.
     * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
     *                          format.
     * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool ImportSchemaFromContent(std::string& result, const std::string& schema,
                                 const std::string& graph = "default", bool json_format = true,
                                 double timeout = 0);

    /**
     * @brief   Import vertex or edge data from content string
     *
     * @param [out] result              The result.
     * @param [in]  desc                data format description.
     * @param [in]  data                the data to be imported.
     * @param [in]  delimiter           data separator.
     * @param [in]  continue_on_error   (Optional) whether to continue when importing data fails.
     * @param [in]  thread_nums         (Optional) maximum number of threads.
     * @param [in]  graph               (Optional) the graph to query.
     * @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
     *                                  binary format.
     * @param [in]  timeout             (Optional) Maximum execution time, overruns will be
     *                                  interrupted.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool ImportDataFromContent(std::string& result, const std::string& desc,
                               const std::string& data, const std::string& delimiter,
                               bool continue_on_error = false, int thread_nums = 8,
                               const std::string& graph = "default", bool json_format = true,
                               double timeout = 0);

    /**
     * @brief   Execute a cypher query
     *
     * @param [out] result      The result.
     * @param [in]  cypher      inquire statement.
     * @param [in]  graph       (Optional) the graph to query.
     * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
     *                          format.
     * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool CallCypher(std::string& result, const std::string& cypher,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0);

    /**
     * @brief   Get the url of client.
     *
     * @returns the url of client.
     */
     std::string GetUrl();

    void Logout();

 private:
    LGraphResponse HandleRequest(LGraphRequest* req);

    bool HandleCypherRequest(LGraphResponse* res, const std::string& query,
                             const std::string& graph, bool json_format, double timeout);
#ifdef BINARY_RESULT_BUG_TO_BE_SOLVE

    std::string SingleElementExtractor(const CypherResult cypher);

    std::string MultElementExtractor(const CypherResult cypher);

    std::string CypherResultExtractor(const CypherResult cypher);
#endif
    std::string CypherResponseExtractor(CypherResponse cypher);

    std::string url;
    std::string user;
    std::string password;
    std::string token;
    int64_t server_version;
    // A Channel represents a communication line to a Server. Notice that
    // Channel is thread-safe and can be shared by all threads in your program.
    std::shared_ptr<lgraph_rpc::m_channel> channel;
    std::shared_ptr<lgraph_rpc::m_controller> cntl;
    // Initialize the channel, NULL means using default options.
    std::shared_ptr<lgraph_rpc::m_channel_options> options;

    fma_common::Logger& logger_;
};
}  // namespace lgraph
