/**
 * Copyright 2022 AntGroup CO., Ltd.
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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "client/python/rpc/lgraph_python_client.h"

namespace py = pybind11;

void register_liblgraph_client_python(pybind11::module& m) {
    // define lgraph::RpcClient class
    py::class_<LGraphPythonClient> c(m, "client", "this is a python rpc client");
    // define the constructor
    c.def(pybind11::init<const std::string &, const std::string &,
          const std::string &>(),
          "Client Login\n"
          "url        Login address.\n"
          "user       The username.\n"
          "password   The password.\n",
          pybind11::arg("url"), pybind11::arg("user"),
          pybind11::arg("password"));

    c.def(pybind11::init<std::vector<std::string> &, const std::string &,
                  const std::string &>(),
          "Client Login\n"
          "urls       Login address.\n"
          "user       The username.\n"
          "password   The password.\n",
          pybind11::arg("urls"), pybind11::arg("user"),
          pybind11::arg("password"));

    // define other function
    c.def("callCypher", &LGraphPythonClient::CallCypher,
          "Execute a cypher query\n"
          "cypher          [in] inquire statement.\n"
          "graph           [in] the graph to query.\n"
          "json_format     [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout         [in] Maximum execution time, overruns will be interrupted\n"
          "url             [in] server address.\n",
          pybind11::arg("cypher"), pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true, pybind11::arg("timeout") = 0,
          pybind11::arg("url") = "",
          pybind11::return_value_policy::move);

    c.def("callCypherToLeader", &LGraphPythonClient::CallCypherToLeader,
          "Execute a cypher query\n"
          "cypher          [in] inquire statement.\n"
          "graph           [in] the graph to query.\n"
          "json_format     [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout         [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("cypher"), pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true, pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("callGql", &LGraphPythonClient::CallGql,
          "Execute a cypher query\n"
          "gql             [in] inquire statement.\n"
          "graph           [in] the graph to query.\n"
          "json_format     [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout         [in] Maximum execution time, overruns will be interrupted\n"
          "url             [in] server address.\n",
          pybind11::arg("gql"), pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true, pybind11::arg("timeout") = 0,
          pybind11::arg("url") = "",
          pybind11::return_value_policy::move);

    c.def("callGqlToLeader", &LGraphPythonClient::CallGqlToLeader,
          "Execute a cypher query\n"
          "gql             [in] inquire statement.\n"
          "graph           [in] the graph to query.\n"
          "json_format     [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout         [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("gql"), pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true, pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("loadProcedure", [] (LGraphPythonClient& self, pybind11::object source_files,
                            pybind11::object procedure_type,
                            pybind11::object procedure_name,
                            pybind11::object code_type,
                            pybind11::object procedure_description,
                            pybind11::object read_only, pybind11::object version,
                            pybind11::object graph) {
            if (pybind11::isinstance<pybind11::str>(source_files)) {
                return self.LoadProcedure(
                    source_files.cast<std::string>(), procedure_type.cast<std::string>(),
                    procedure_name.cast<std::string>(), code_type.cast<std::string>(),
                    procedure_description.cast<std::string>(), read_only.cast<bool>(),
                    version.cast<std::string>(), graph.cast<std::string>());
            } else if (pybind11::isinstance<pybind11::list>(source_files)) {
                return self.LoadProcedure(
                    source_files.cast<std::vector<std::string>>(),
                    procedure_type.cast<std::string>(),
                    procedure_name.cast<std::string>(), code_type.cast<std::string>(),
                    procedure_description.cast<std::string>(), read_only.cast<bool>(),
                    version.cast<std::string>(), graph.cast<std::string>());
            } else {
                throw std::invalid_argument("Invalid argument types for LoadProcedure");
            }
        },
          "Load a user-defined procedure\n"
          "source_files           [in] Source file or source file list containing code (only cpp "
          "files support uploading multiple files)\n"
          "procedure_type         [in] the procedure type, currently supported CPP and PY\n"
          "procedure_name         [in] procedure name\n"
          "code_type              [in] code type, currently supported PY, SO, CPP, ZIP\n"
          "procedure_description  [in] procedure description\n"
          "read_only              [in] procedure is read only or not\n"
          "version                [in] procedure version, currently v1 or v2"
          "graph                  [in] the graph to query.\n",
          pybind11::arg("source_files"), pybind11::arg("procedure_type"),
          pybind11::arg("procedure_name"),
          pybind11::arg("code_type"), pybind11::arg("procedure_description"),
          pybind11::arg("read_only"),
          pybind11::arg("version") = "v1",
          pybind11::arg("graph") = "default",
          pybind11::return_value_policy::move);

    c.def("callProcedure", &LGraphPythonClient::CallProcedure,
          "Execute a user-defined procedure\n"
          "procedure_type         [in] the procedure type, currently supported CPP and PY\n"
          "procedure_name         [in] procedure name\n"
          "param                  [in] the execution parameters\n"
          "procedure_time_out     [in] Maximum execution time, overruns will be interrupted\n"
          "in_process             [in] support in future\n"
          "graph                  [in] the graph to query.\n"
          "json_format            [in] Returns the format， true is json，Otherwise, "
          "                            binary format\n"
          "url                    [in] server address.\n",
          pybind11::arg("procedure_type"), pybind11::arg("procedure_name"), pybind11::arg("param"),
          pybind11::arg("procedure_time_out") = 0.0, pybind11::arg("in_process") = false,
          pybind11::arg("graph") = "default", pybind11::arg("json_format") = true,
          pybind11::arg("url") = "", pybind11::return_value_policy::move);

    c.def("callProcedureToLeader", &LGraphPythonClient::CallProcedureToLeader,
          "Execute a user-defined procedure\n"
          "procedure_type         [in] the procedure type, currently supported CPP and PY\n"
          "procedure_name         [in] procedure name\n"
          "param                  [in] the execution parameters\n"
          "procedure_time_out     [in] Maximum execution time, overruns will be interrupted\n"
          "in_process             [in] support in future\n"
          "graph                  [in] the graph to query.\n"
          "json_format            [in] Returns the format， true is json，Otherwise, "
          "                            binary format\n",
          pybind11::arg("procedure_type"), pybind11::arg("procedure_name"), pybind11::arg("param"),
          pybind11::arg("procedure_time_out") = 0.0, pybind11::arg("in_process") = false,
          pybind11::arg("graph") = "default", pybind11::arg("json_format") = true,
          pybind11::return_value_policy::move);

    c.def("listProcedures", &LGraphPythonClient::ListProcedures,
          "Execute built-in procedures\n"
          "procedure_type      [in] the procedure type, currently supported CPP and PY\n"
          "version             [in] procedure version, currently v1 or v2 or any"
          "graph               [in] the graph to query.\n"
          "url                 [in] server address.\n",
          pybind11::arg("procedure_type"),
          pybind11::arg("version") = "any",
          pybind11::arg("graph") = "default",
          pybind11::arg("url") = "", pybind11::return_value_policy::move);

    c.def("deleteProcedure", &LGraphPythonClient::DeleteProcedure,
          "Delete built-in procedure\n"
          "procedure_type      [in] the procedure type, currently supported CPP and PY\n"
          "procedure_name      [in] procedure name\n"
          "graph               [in] the graph to query.\n",
          pybind11::arg("procedure_type"), pybind11::arg("procedure_name"),
          pybind11::arg("graph") = "default", pybind11::return_value_policy::move);

    c.def("importSchemaFromFile", &LGraphPythonClient::ImportSchemaFromFile,
          "import vertex or edge schema from file\n"
          "schema_file         [in] the schema_file contain schema\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("schema_file"), pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true, pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("importDataFromFile", &LGraphPythonClient::ImportDataFromFile,
          "import vertex or edge data from file\n"
          "conf_file           [in] data file contain format description and data\n"
          "delimiter           [in] data separator\n"
          "continue_on_error   [in] whether to continue when importing data fails\n"
          "thread_nums         [in] maximum number of threads\n"
          "skip_packages       [in] skip packages number\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("conf_file"), pybind11::arg("delimiter"),
          pybind11::arg("continue_on_error") = false, pybind11::arg("thread_nums") = 8,
          pybind11::arg("skip_packages") = 0, pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true, pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("importSchemaFromContent", &LGraphPythonClient::ImportSchemaFromContent,
          "import vertex or edge schema from content string\n"
          "schema              [in] the schema contain schema\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("schema"), pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true, pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("importDataFromContent", &LGraphPythonClient::ImportDataFromContent,
          "import vertex or edge data from content string\n"
          "desc                [in] data format description\n"
          "data                [in] the data to be imported\n"
          "delimiter           [in] data separator\n"
          "continue_on_error   [in] whether to continue when importing data fails\n"
          "thread_nums         [in] maximum number of threads\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("desc"), pybind11::arg("data"), pybind11::arg("delimiter"),
          pybind11::arg("continue_on_error") = false, pybind11::arg("thread_nums") = 8,
          pybind11::arg("graph") = "default", pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0, pybind11::return_value_policy::move);

    c.def("logout", &LGraphPythonClient::Logout, "Execute unbind token\n");
}

PYBIND11_MODULE(liblgraph_client_python, m) {
    register_liblgraph_client_python(m);
}
