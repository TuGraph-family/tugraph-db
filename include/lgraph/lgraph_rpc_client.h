/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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

    ~RpcClient() {
    }

    // TODO(jzj)
    int64_t Restore(const std::vector<BackupLogEntry>& requests);

    /**
     * @brief   Load a built-in plugin
     *
     * @param [out] result              The result.
     * @param [in]  source_file         the source_file contain plugin code.
     * @param [in]  plugin_type         the plugin type, currently supported CPP and PY.
     * @param [in]  plugin_name         plugin name.
     * @param [in]  code_type           code type, currently supported PY, SO, CPP, ZIP.
     * @param [in]  plugin_description  plugin description.
     * @param [in]  read_only           plugin is read only or not.
     * @param [in]  graph               (Optional) the graph to query.
     * @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
     *                                  binary format.
     * @param [in]  timeout             (Optional) Maximum execution time, overruns will be
     *                                  interrupted.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool LoadPlugin(std::string& result, const std::string& source_file,
                    const std::string& plugin_type, const std::string& plugin_name,
                    const std::string& code_type, const std::string& plugin_description,
                    bool read_only, const std::string& graph = "default", bool json_format = true,
                    double timeout = 0);

    /**
     * @brief   Execute a built-in plugin
     *
     * @param [out] result          The result.
     * @param [in]  plugin_type     the plugin type, currently supported CPP and PY.
     * @param [in]  plugin_name     plugin name.
     * @param [in]  param           the execution parameters.
     * @param [in]  plugin_time_out (Optional) Maximum execution time, overruns will be
     *                              interrupted.
     * @param [in]  in_process      (Optional) support in future.
     * @param [in]  graph           (Optional) the graph to query.
     * @param [in]  json_format     (Optional) Returns the format， true is json，Otherwise, binary
     *                              format.
     * @param [in]  timeout         (Optional) Maximum execution time, overruns will be
     *                              interrupted.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool CallPlugin(std::string& result, const std::string& plugin_type,
                    const std::string& plugin_name, const std::string& param,
                    double plugin_time_out = 0.0, bool in_process = false,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0);

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

 private:
    LGraphResponse HandleRequest(LGraphRequest* req);

    bool HandleCypherRequest(LGraphResponse* res, const std::string& query,
                             const std::string& graph, bool json_format, double timeout);

    std::string SingleElementExtractor(const CypherResult cypher);

    std::string MultElementExtractor(const CypherResult cypher);

    std::string CypherResultExtractor(const CypherResult cypher);

    std::string CypherResponseExtractor(const CypherResponse cypher);

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
