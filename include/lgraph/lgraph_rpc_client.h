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
#include <deque>
#include "tools/json.hpp"

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
class GraphQueryResult;
class GraphQueryResponse;
class BackupLogEntry;

enum class GraphQueryType {
    // Cypher type query
    CYPHER = 0,
    // GQL type query
    GQL = 1
};

enum ClientType {
    // Connection to HA group using direct network address defined in conf.
    DIRECT_HA_CONNECTION = 0,
    // Connection to HA group using indirect network address different from conf.
    INDIRECT_HA_CONNECTION = 1,
    // Connection to single node.
    SINGLE_CONNECTION = 2
};

class RpcClient {
 private:
    class RpcSingleClient {
     public:
        /**
         * @brief   RpcSingleClient Login.
         *
         * @param   url         Login address.
         * @param   user        The username.
         * @param   password    The password.
         */
        RpcSingleClient(const std::string& url, const std::string& user,
                        const std::string& password);

        ~RpcSingleClient();

        // TODO(jzj)
        int64_t Restore(const std::vector<BackupLogEntry>& requests);

        /**
         * @brief   Load a user-defined procedure
         *
         * @param [out] result                  The result.
         * @param [in]  source_files            the source_file list contain procedure code(only
         *                                      for code_type cpp)
         * @param [in]  procedure_type          the procedure type, currently supported CPP and PY.
         * @param [in]  procedure_name          procedure name.
         * @param [in]  code_type               code type, currently supported PY, SO, CPP, ZIP.
         * @param [in]  procedure_description   procedure description.
         * @param [in]  read_only               procedure is read only or not.
         * @param [in]  version                 (Optional) the version of procedure.
         * @param [in]  graph                   (Optional) the graph to query.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool LoadProcedure(std::string& result, const std::vector<std::string>& source_files,
                           const std::string& procedure_type, const std::string& procedure_name,
                           const std::string& code_type, const std::string& procedure_description,
                           bool read_only, const std::string& version = "v1",
                           const std::string& graph = "default");

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
         * @param [in]  version         (Optional) the version of procedure.
         * @param [in]  graph           (Optional) the graph to query.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool ListProcedures(std::string& result, const std::string& procedure_type,
                            const std::string& version = "any",
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
         * @param [in]  continue_on_error   (Optional) whether to continue when importing data
         * fails.
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
         * @param [in]  continue_on_error   (Optional) whether to continue when importing data
         * fails.
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
         * @brief   Execute a gql query
         *
         * @param [out] result      The result.
         * @param [in]  gql         inquire statement.
         * @param [in]  graph       (Optional) the graph to query.
         * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
         *                          format.
         * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool CallGql(std::string& result, const std::string& gql,
                        const std::string& graph = "default", bool json_format = true,
                        double timeout = 0);

        /**
         * @brief   Get the url of single client.
         *
         * @returns the url of single client.
         */
        std::string GetUrl();

        /**
         * @brief   Client log out
         */
        void Logout();

     private:
        LGraphResponse HandleRequest(LGraphRequest* req);

        bool HandleGraphQueryRequest(lgraph::GraphQueryType type,
                                     LGraphResponse* res, const std::string& query,
                                     const std::string& graph, bool json_format, double timeout);
#ifdef BINARY_RESULT_BUG_TO_BE_SOLVE

        std::string SingleElementExtractor(const CypherResult cypher);

        std::string MultElementExtractor(const CypherResult cypher);

        std::string CypherResultExtractor(const CypherResult cypher);
#endif
        std::string GraphQueryResponseExtractor(const GraphQueryResponse& cypher);

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
    };

 public:
    /**
     * @brief   RpcClient Login.
     *
     * @param   url         Login address.
     * @param   user        The username.
     * @param   password    The password.
     */
    explicit RpcClient(const std::string& url,
                       const std::string& user,
                       const std::string& password);

    /**
     * @brief   RpcClient Login.
     *
     * @param   urls        Login address.
     * @param   user        The username.
     * @param   password    The password.
     */
    explicit RpcClient(std::vector<std::string>& urls,
                       std::string user,
                       std::string password);

    ~RpcClient();

    /**
     * @brief   Execute a cypher query
     *
     * @param [out] result      The result.
     * @param [in]  cypher      inquire statement.
     * @param [in]  graph       (Optional) the graph to query.
     * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
     *                          format.
     * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
     * @param [in]  url         (Optional) Node address of calling cypher.
     * @returns True if it succeeds, false if it fails.
     */
    bool CallCypher(std::string& result, const std::string& cypher,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0, const std::string& url = "");

    /**
     * @brief   Execute a cypher query to leader
     *
     * @param [out] result      The result.
     * @param [in]  cypher      inquire statement.
     * @param [in]  graph       (Optional) the graph to query.
     * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
     *                          format.
     * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
     * @returns True if it succeeds, false if it fails.
     */
    bool CallCypherToLeader(std::string& result, const std::string& cypher,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0);

    /**
     * @brief   Execute a gql query
     *
     * @param [out] result      The result.
     * @param [in]  gql         inquire statement.
     * @param [in]  graph       (Optional) the graph to query.
     * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
     *                          format.
     * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
     * @param [in]  url         (Optional) Node address of calling cypher.
     * @returns True if it succeeds, false if it fails.
     */
    bool CallGql(std::string& result, const std::string& gql,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0, const std::string& url = "");

    /**
     * @brief   Execute a gql query to leader
     *
     * @param [out] result      The result.
     * @param [in]  gql         inquire statement.
     * @param [in]  graph       (Optional) the graph to query.
     * @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
     *                          format.
     * @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
     * @returns True if it succeeds, false if it fails.
     */
    bool CallGqlToLeader(std::string& result, const std::string& gql,
                 const std::string& graph = "default", bool json_format = true,
                 double timeout = 0);

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
     * @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
     *                                  binary format.
     * @param [in]  url                 (Optional) Node address of calling procedure.
     * @returns True if it succeeds, false if it fails.
     */
    bool CallProcedure(std::string& result, const std::string& procedure_type,
                       const std::string& procedure_name, const std::string& param,
                       double procedure_time_out = 0.0, bool in_process = false,
                       const std::string& graph = "default", bool json_format = true,
                       const std::string& url = "");

    /**
     * @brief   Execute a user-defined procedure to leader
     *
     * @param [out] result              The result.
     * @param [in]  procedure_type      the procedure type, currently supported CPP and PY.
     * @param [in]  procedure_name      procedure name.
     * @param [in]  param               the execution parameters.
     * @param [in]  procedure_time_out  (Optional) Maximum execution time, overruns will be
     *                                  interrupted.
     * @param [in]  in_process          (Optional) support in future.
     * @param [in]  graph               (Optional) the graph to query.
     * @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
     *                                  binary format.
     * @returns True if it succeeds, false if it fails.
     */
    bool CallProcedureToLeader(std::string& result, const std::string& procedure_type,
                       const std::string& procedure_name, const std::string& param,
                       double procedure_time_out = 0.0, bool in_process = false,
                       const std::string& graph = "default", bool json_format = true);

    /**
     * @brief   Load a built-in procedure
     *
     * @param [out] result                  The result.
     * @param [in]  source_file             the source_file contain procedure code.
     * @param [in]  procedure_type          the procedure type, currently supported CPP and PY.
     * @param [in]  procedure_name          procedure name.
     * @param [in]  code_type               code type, currently supported PY, SO, CPP, ZIP.
     * @param [in]  procedure_description   procedure description.
     * @param [in]  read_only               procedure is read only or not.
     * @param [in]  version                 (Optional) the version of procedure.
     * @param [in]  graph                   (Optional) the graph to query.
     * @returns True if it succeeds, false if it fails.
     */
    bool LoadProcedure(std::string& result, const std::string& source_file,
                       const std::string& procedure_type, const std::string& procedure_name,
                       const std::string& code_type, const std::string& procedure_description,
                       bool read_only, const std::string& version = "v1",
                       const std::string& graph = "default");

    /**
     * @brief   Load a built-in procedure
     *
     * @param [out] result                  The result.
     * @param [in]  source_files            the source_file list contain procedure code(only
*                                           for code_type cpp)
     * @param [in]  procedure_type          the procedure type, currently supported CPP and PY.
     * @param [in]  procedure_name          procedure name.
     * @param [in]  code_type               code type, currently supported PY, SO, CPP, ZIP.
     * @param [in]  procedure_description   procedure description.
     * @param [in]  read_only               procedure is read only or not.
     * @param [in]  version                 (Optional) the version of procedure.
     * @param [in]  graph                   (Optional) the graph to query.
     * @returns True if it succeeds, false if it fails.
     */
    bool LoadProcedure(std::string& result, const std::vector<std::string>& source_files,
                       const std::string& procedure_type, const std::string& procedure_name,
                       const std::string& code_type, const std::string& procedure_description,
                       bool read_only, const std::string& version = "v1",
                       const std::string& graph = "default");

    /**
     * @brief   List user-defined procedures
     *
     * @param [out] result          The result.
     * @param [in]  procedure_type  (Optional) the procedure type, "" for all procedures,
     *                              CPP and PY for special type.
     * @param [in]  version         (Optional) the version of procedure.
     * @param [in]  graph           (Optional) the graph to query.
     * @param [in]  url             Node address of calling procedure.
     * @returns True if it succeeds, false if it fails.
     */
    bool ListProcedures(std::string& result, const std::string& procedure_type,
                        const std::string& version = "any",
                        const std::string& graph = "default", const std::string& url = "");

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
                         const std::string& procedure_name, const std::string& graph = "default");

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

    int64_t Restore(const std::vector<BackupLogEntry>& requests);

    /**
     * @brief   Client log out
     */
    void Logout();

 private:
    ClientType client_type;
    std::string user;
    std::string password;

    // Attribute to SAME_WITH_CONF client
    std::shared_ptr<lgraph::RpcClient::RpcSingleClient> base_client;

    // Attribute to DIFF_FROM_CONF client
    std::vector<std::string> urls;

    // Attributes common to all types of clients
    std::shared_ptr<lgraph::RpcClient::RpcSingleClient> leader_client;
    std::deque<std::shared_ptr<lgraph::RpcClient::RpcSingleClient>> client_pool;
    nlohmann::json built_in_procedures{};
    nlohmann::json user_defined_procedures{};
    std::vector<std::string> cypher_write_constant;
    std::vector<std::string> gql_write_constant;

    /**
     * @brief   Determine whether it is a read-only query
     *
     * @param [in]  type                inquire query type.
     * @param [in]  query               inquire statement.
     * @param [in]  graph               (Optional) the graph to query.
     * @returns True if it succeeds, false if it fails.
     */
    bool IsReadQuery(lgraph::GraphQueryType type,
                      const std::string& query, const std::string& graph);

    /**
     * @brief   Return rpc client based on whether it is a read-only query
     *
     * @param [in]  type                inquire query type.
     * @param [in]  query               inquire statement.
     * @param [in]  graph               (Optional) the graph to query.
     * @returns Master rpc client if cypher is not read-only, slaver rpc client if cypher is
     * read-only.
     */
    std::shared_ptr<lgraph::RpcClient::RpcSingleClient> GetClient(lgraph::GraphQueryType type,
                                                                  const std::string& cypher,
                                                                  const std::string& graph);

    /**
     * @brief   Return rpc client based on whether it is a read-only query
     *
     * @param [in]  isReadQuery              read query or not.
     * @returns Master rpc client if cypher is not read-only, slaver rpc client if cypher is
     * read-only.
     */
    std::shared_ptr<lgraph::RpcClient::RpcSingleClient> GetClient(bool isReadQuery);

    /**
     * @brief   Get the client according to the node url
     *
     * @param [in]  url             Node address of client connection.
     * @returns Rpc client connecting to url.
     */
    std::shared_ptr<lgraph::RpcClient::RpcSingleClient> GetClientByNode(const std::string& url);

    /**
     * @brief   Refresh User-defined Procedure
     */
    void RefreshUserDefinedProcedure();

    /**
     * @brief   Refresh Built-in Procedure
     */
    void RefreshBuiltInProcedure();

    /**
     * @brief   Refresh the client connection pool according to the cluster status
     */
    void RefreshClientPool();

    /**
     * @brief  Load balance the client pool
     */
    void LoadBalanceClientPool();

    /**
     * @brief   Refresh client pool, procedure info
     */
    void RefreshConnection();

    /**
     * @brief   If an exception is thrown in the query, refresh the connection and re-execute
     */
    template <typename F>
    bool DoubleCheckQuery(F const& f);
};
}  // namespace lgraph
