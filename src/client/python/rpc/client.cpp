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
    py::class_<LgraphPythonClient> c(m, "client", "this is a python rpc client");
    // define the constructor
    c.def(pybind11::init<const std::string &, const std::string &, const std::string &>(),
            "Client Login\n"
            "url        Login address.\n"
            "user       The username.\n"
            "password   The password.\n",
            pybind11::arg("url"),
            pybind11::arg("user"),
            pybind11::arg("password"));

    // define other function
    c.def("callCypher", &LgraphPythonClient::CallCypher,
          "Execute a cypher query\n"
          "cypher          [in] inquire statement.\n"
          "graph           [in] the graph to query.\n"
          "json_format     [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout         [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("cypher"),
          pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("loadPlugin", &LgraphPythonClient::LoadPlugin,
          "Load a built-in plugin\n"
          "source_file         [in] the source_file contain plugin code\n"
          "plugin_type         [in] the plugin type, currently supported CPP and PY\n"
          "plugin_name         [in] plugin name\n"
          "code_type           [in] code type, currently supported PY, SO, CPP, ZIP\n"
          "plugin_description  [in] plugin description\n"
          "read_only           [in] plugin is read only or not\n"
          "graph               [in] the graph to query.\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("source_file"),
          pybind11::arg("plugin_type"),
          pybind11::arg("plugin_name"),
          pybind11::arg("code_type"),
          pybind11::arg("plugin_description"),
          pybind11::arg("read_only"),
          pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("callPlugin", &LgraphPythonClient::CallPlugin,
          "Execute a built-in plugin\n"
          "plugin_type         [in] the plugin type, currently supported CPP and PY\n"
          "plugin_name         [in] plugin name\n"
          "param               [in] the execution parameters\n"
          "plugin_time_out     [in] Maximum execution time, overruns will be interrupted\n"
          "in_process          [in] support in future\n"
          "graph               [in] the graph to query.\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("plugin_type"),
          pybind11::arg("plugin_name"),
          pybind11::arg("param"),
          pybind11::arg("plugin_time_out") = 0.0,
          pybind11::arg("in_process") = false,
          pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("importSchemaFromFile", &LgraphPythonClient::ImportSchemaFromFile,
          "import vertex or edge schema from file\n"
          "schema_file         [in] the schema_file contain schema\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("schema_file"),
          pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("importDataFromFile", &LgraphPythonClient::ImportDataFromFile,
          "import vertex or edge data from file\n"
          "conf_file           [in] data file contain format description and data\n"
          "delimiter           [in] data separator\n"
          "continue_on_error   [in] whether to continue when importing data fails\n"
          "thread_nums         [in] maximum number of threads\n"
          "skip_packages       [in] skip packages number\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("conf_file"),
          pybind11::arg("delimiter"),
          pybind11::arg("continue_on_error") = false,
          pybind11::arg("thread_nums") = 8,
          pybind11::arg("skip_packages") = 0,
          pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("importSchemaFromContent", &LgraphPythonClient::ImportSchemaFromContent,
          "import vertex or edge schema from content string\n"
          "schema              [in] the schema contain schema\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("schema"),
          pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("importDataFromContent", &LgraphPythonClient::ImportDataFromContent,
          "import vertex or edge data from content string\n"
          "desc                [in] data format description\n"
          "data                [in] the data to be imported\n"
          "delimiter           [in] data separator\n"
          "continue_on_error   [in] whether to continue when importing data fails\n"
          "thread_nums         [in] maximum number of threads\n"
          "graph               [in] the graph to query\n"
          "json_format         [in] Returns the format， true is json，Otherwise, binary format\n"
          "timeout             [in] Maximum execution time, overruns will be interrupted\n",
          pybind11::arg("desc"),
          pybind11::arg("data"),
          pybind11::arg("delimiter"),
          pybind11::arg("continue_on_error") = false,
          pybind11::arg("thread_nums") = 8,
          pybind11::arg("graph") = "default",
          pybind11::arg("json_format") = true,
          pybind11::arg("timeout") = 0,
          pybind11::return_value_policy::move);

    c.def("logout", &LgraphPythonClient::Logout,
          "Execute unbind token\n");


    c.def("close", &LgraphPythonClient::Close,
          "close the channel \n");
}

PYBIND11_MODULE(liblgraph_client_python, m) {
    register_liblgraph_client_python(m);
}
