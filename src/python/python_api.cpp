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

#ifdef _WIN32
// Workaround for VS _DEBUG bug.
// VS STL uses _DEBUG macro and some tricks that conflicts with pybind11, causing
// some '_invalid_parameter not a member of std namespace' error.
#include <corecrt.h>
#endif

#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>

#include <boost/interprocess/ipc/message_queue.hpp>

#include "fma-common/fma_stream.h"
#include "fma-common/string_formatter.h"

#include "core/data_type.h"
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_types.h"
#include "plugin/plugin_manager_impl.h"
#include "plugin/plugin_context.h"

#if LGRAPH_ENABLE_PYTHON_PLUGIN

namespace lgraph_api {
namespace python {

struct SignalsGuard {
    SignalsGuard() {
        if (PyErr_CheckSignals() != 0) {
            throw pybind11::error_already_set();
        }
    }
};

inline FieldData ObjectToFieldData(const pybind11::object& o) {
    if (pybind11::isinstance<FieldData>(o)) {
        return o.cast<FieldData>();
    } else if (pybind11::isinstance<pybind11::bytes>(o)) {
        return FieldData::Blob(o.cast<std::vector<uint8_t>>());
    } else if (pybind11::isinstance<pybind11::str>(o)) {
        return FieldData(o.cast<std::string>());
    } else if (pybind11::isinstance<pybind11::int_>(o)) {
        return FieldData(o.cast<int64_t>());
    } else if (pybind11::isinstance<pybind11::float_>(o)) {
        return FieldData(o.cast<double>());
    } else if (pybind11::isinstance<pybind11::bool_>(o)) {
        return FieldData(o.cast<bool>());
    } else {
        throw std::runtime_error("Illegal field data given.");
    }
}

inline void PyDictToVectors(const pybind11::dict& dict, std::vector<std::string>& fnames,
                            std::vector<FieldData>& fdata) {
    fnames.clear();
    fdata.clear();
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        fnames.push_back(pybind11::str(it->first));
        fdata.push_back(ObjectToFieldData(it->second.cast<pybind11::object>()));
    }
}

inline pybind11::object FieldDataToPyObj(const FieldData& data) {
    switch (data.type) {
    case FieldType::NUL:
        return pybind11::none();
    case FieldType::BOOL:
        return pybind11::bool_(data.data.boolean);
    case FieldType::INT8:
        return pybind11::int_(data.data.int8);
    case FieldType::INT16:
        return pybind11::int_(data.data.int16);
    case FieldType::INT32:
        return pybind11::int_(data.data.int32);
    case FieldType::INT64:
        return pybind11::int_(data.data.int64);
    case FieldType::FLOAT:
        return pybind11::float_(data.data.sp);
    case FieldType::DOUBLE:
        return pybind11::float_(data.data.dp);
    case FieldType::DATE:
        return pybind11::cast(
            data.AsDate().operator lgraph_api::DateTime().ConvertToUTC().TimePoint());
    case FieldType::DATETIME:
        return pybind11::cast(data.AsDateTime().ConvertToUTC().TimePoint());
    case FieldType::STRING:
        return pybind11::str(*data.data.buf);
    case FieldType::BLOB:
        return pybind11::bytes(*data.data.buf);
    }
    FMA_ASSERT(false);
    return pybind11::none();
}

inline pybind11::list FieldDataVectorToPyList(const std::vector<FieldData>& v) {
    pybind11::list l;
    for (auto& f : v) l.append(FieldDataToPyObj(f));
    return l;
}

inline pybind11::dict FieldDataMapToPyDict(const std::map<std::string, FieldData>& data) {
    pybind11::dict d;
    for (auto& kv : data) {
        d[kv.first.c_str()] = FieldDataToPyObj(kv.second);
    }
    return d;
}

void register_python_api(pybind11::module& m) {
    //======================================
    // Register data types
    //======================================
    pybind11::class_<FieldSpec> field(m, "FieldSpec", "Field specification.");
    field.def(pybind11::init<>())
        .def(pybind11::init<const std::string&, FieldType, bool>(),
             "Defines a FieldSpec with its name (type:str), type "
             "(type:FieldType) and nullable (type:bool).",
             pybind11::arg("name"), pybind11::arg("type"), pybind11::arg("nullable"),
             pybind11::call_guard<SignalsGuard>())
        .def_readwrite("name", &FieldSpec::name, "Name of this field.")
        .def_readwrite("type", &FieldSpec::type,
                       "Type of this field, INT8, INT16, ..., FLOAT, DOUBLE, STRING.")
        .def_readwrite("nullable", &FieldSpec::optional, "Whether this field can be null.")
        .def("__repr__", [](const FieldSpec& a) {
            return fma_common::StringFormatter::Format("(name:{}, type:{}, nullable:{})", a.name,
                                                       a.type, a.optional);
        });

    pybind11::class_<EdgeUid>(m, "EdgeUid", "Edge identifier.")
        .def(pybind11::init<>(),
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<int64_t, int64_t, uint16_t, int64_t, int64_t>(),
             "Defines a EdgeUid with (src_id, dst_id, label_id, primary_id, edge_id).",
             pybind11::arg("src_id"), pybind11::arg("dst_id"), pybind11::arg("label_id"),
             pybind11::arg("primary_id"), pybind11::arg("edge_id"),
             pybind11::call_guard<SignalsGuard>())
        .def_readwrite("src", &EdgeUid::src, "Source vertex ID.")
        .def_readwrite("dst", &EdgeUid::dst, "Destination vertex ID.")
        .def_readwrite("lid", &EdgeUid::lid, "Label ID of the edge.")
        .def_readwrite("tid", &EdgeUid::tid, "Temporal ID of the edge.")
        .def_readwrite("eid", &EdgeUid::eid, "ID of the edge.")
        .def("__repr__",
             [](const EdgeUid& a) {
                 return fma_common::StringFormatter::Format(
                     "(src:{}, dst:{}, lid:{}, tid:{}, eid:{})", a.src, a.dst, a.lid, a.tid,
                     a.eid);
             })
        .def("__str__", [](const EdgeUid& a) {
            return fma_common::StringFormatter::Format("{}_{}_{}_{}_{}", a.src, a.dst, a.lid, a.tid,
                                                       a.eid);
        });

    pybind11::enum_<lgraph_api::IndexType>(m, "IndexType")
        .value("NonuniqueIndex", IndexType::NonuniqueIndex, "NonuniqueIndex")
        .value("GlobalUniqueIndex", IndexType::GlobalUniqueIndex, "GlobalUniqueIndex")
        .value("PairUniqueIndex", IndexType::PairUniqueIndex, "PairUniqueIndex")
        .export_values();

    pybind11::class_<IndexSpec>(m, "IndexSpec", "Index specification.")
        .def(pybind11::init<>(),
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<const std::string&, const std::string&, IndexType>(),
             "Defines an IndexSpec with its label_name:str, field_name:str and "
             "type:int.",
             pybind11::arg("label_name"), pybind11::arg("field_name"), pybind11::arg("type"),
             pybind11::call_guard<SignalsGuard>())
        .def_readwrite("label", &IndexSpec::label, "Name of the label.")
        .def_readwrite("field", &IndexSpec::field, "Name of the field")
        .def_readwrite("type", &IndexSpec::type, "Index type. eg Unique index or not")
        .def("__repr__", [](const IndexSpec& a) {
            return fma_common::StringFormatter::Format("(label:{}, field:{}, type:{})", a.label,
                                                       a.field, static_cast<int>(a.type));
        });

    pybind11::class_<EdgeOptions>(m, "EdgeOptions", "Edge options.")
        .def(pybind11::init<>(),
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<const std::vector<std::pair<std::string, std::string>>&>(),
             "Define EdgeOptions with edge constraints",
             pybind11::arg("edge_constraints"),
             pybind11::call_guard<SignalsGuard>())
        .def_readwrite("edge_constraints", &EdgeOptions::edge_constraints, "Edge constraints.")
        .def_readwrite("temporal_field", &EdgeOptions::temporal_field, "Edge temporal field.")
        .def_readwrite("detach_property", &EdgeOptions::detach_property,
                       "Whether to store property in detach model.")
        .def("__repr__", [](const EdgeOptions& a) {
            return a.to_string();
        });

    pybind11::class_<VertexOptions>(m, "VertexOptions", "Vertex options.")
        .def(pybind11::init<>(),
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<const std::string&>(),
             "Define VertexOptions with primary field",
             pybind11::arg("primary_field"),
             pybind11::call_guard<SignalsGuard>())
        .def_readwrite("primary_field", &VertexOptions::primary_field,
                       "Vertex primary field.")
        .def_readwrite("detach_property", &VertexOptions::detach_property,
                       "Whether to store property in detach model.")
        .def("__repr__", [](const VertexOptions& a) {
            return a.to_string();
        });

    pybind11::class_<FieldData> data(m, "FieldData", "FieldData is the data type of field value.");
    data.def(pybind11::init<>(), "Constructs an empty FieldData (is_null() == true).",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<bool>(), "Constructs an bool type FieldData.",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<int8_t>(), "Constructs an int8 type FieldData.",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<int16_t>(), "Constructs an int16 type FieldData.",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<int32_t>(), "Constructs an int32 type FieldData.",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<int64_t>(), "Constructs an int64 type FieldData.",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<float>(), "Constructs an float type FieldData.",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<double>(), "Constructs a double type FieldData.",
             pybind11::call_guard<SignalsGuard>())
        .def(pybind11::init<const std::string&>(), "Constructs a FielData contains binary bytes.",
             pybind11::call_guard<SignalsGuard>())
        .def("isNull", [](const FieldData& a) { return a.IsNull(); },
            pybind11::call_guard<SignalsGuard>())
        .def("get", [](const FieldData& a) -> pybind11::object { return FieldDataToPyObj(a); },
            pybind11::call_guard<SignalsGuard>())
        .def("set", [](FieldData& a, const pybind11::object& o) { a = ObjectToFieldData(o); },
            pybind11::call_guard<SignalsGuard>())
        .def_static("Bool", &FieldData::Bool, "Make a BOOL value")
        .def_static("Int8", &FieldData::Int8, "Make a INT8 value")
        .def_static("Int16", &FieldData::Int16, "Make a INT16 value")
        .def_static("Int32", &FieldData::Int32, "Make a INT32 value")
        .def_static("Int64", &FieldData::Int64, "Make a INT64 value")
        .def_static("Float", &FieldData::Float, "Make a FLOAT value")
        .def_static("Double", &FieldData::Double, "Make a DOUBLE value")
        .def_static(
            "Date", [](const std::string& str) { return FieldData::Date(str); },
            "Make a DATE value")
        .def_static(
            "Date",
            [](const std::chrono::system_clock::time_point& tp) {
                return FieldData(lgraph::DateTime(tp).ConvertToLocal().operator lgraph_api::Date());
            },
            "Make a DATE value")
        .def_static(
            "DateTime", [](const std::string& str) { return FieldData::DateTime(str); },
            "Make a DATETIME value")
        .def_static(
            "DateTime",
            // pybind11 converts datetime from local to utc when passing to c++
            [](const std::chrono::system_clock::time_point& tp) {
                return FieldData(lgraph_api::DateTime(tp).ConvertToLocal());
            },
            "Make a DATETIME value")
        .def_static(
            "String", [](const std::string& str) { return FieldData::String(str); },
            "Make a STRING value")
        .def_static(
            "Blob", [](const pybind11::bytes& str) { return FieldData::Blob(str); },
            "Make a BLOB value")
        .def("AsBool", &FieldData::AsBool, "Get value as bool, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def("AsInt8", &FieldData::AsInt8, "Get value as int8, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def("AsInt16", &FieldData::AsInt16,
             "Get value as int16, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def("AsInt32", &FieldData::AsInt32,
             "Get value as int32, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def("AsInt64", &FieldData::AsInt64,
             "Get value as int64, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def("AsFloat", &FieldData::AsFloat,
             "Get value as float, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def("AsDouble", &FieldData::AsDouble,
             "Get value as double, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def(
            "AsDate",
            [](const FieldData& a) {
                return a.AsDate().operator lgraph_api::DateTime().ConvertToUTC().TimePoint();
            },
            "Get value as date, throws exception on type mismatch",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AsDateTime",
            // python sees time_point as utc time and will always convert it to local time
            [](const FieldData& a) { return a.AsDateTime().ConvertToUTC().TimePoint(); },
            "Get value as datetime, throws exception on type mismatch",
            pybind11::call_guard<SignalsGuard>())
        .def("AsString", &FieldData::AsString,
             "Get value as string, throws exception on type mismatch",
             pybind11::call_guard<SignalsGuard>())
        .def(
            "AsBlob", [](const FieldData& a) { return pybind11::bytes(*a.data.buf); },
            "Get value as double, throws exception on type mismatch",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "ToPython", [](const FieldData& fd) { return FieldDataToPyObj(fd); },
            "Convert to corresponding Python type.",
            pybind11::call_guard<SignalsGuard>())
        .def_readonly("type", &FieldData::type)
        .def("__repr__", [](const FieldData& a) { return a.ToString(); },
            pybind11::call_guard<SignalsGuard>())
        .def("__eq__", [](const FieldData& a, const FieldData& b) { return a == b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__eq__", [](const FieldData& a,
                          const pybind11::object& b) { return a == ObjectToFieldData(b); },
            pybind11::call_guard<SignalsGuard>())
        .def("__eq__", [](const pybind11::object& a,
                          const FieldData& b) { return ObjectToFieldData(a) == b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__gt__", [](const FieldData& a, const FieldData& b) { return a > b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__gt__",
             [](const FieldData& a, const pybind11::object& b) { return a > ObjectToFieldData(b); },
            pybind11::call_guard<SignalsGuard>())
        .def("__gt__",
             [](const pybind11::object& a, const FieldData& b) { return ObjectToFieldData(a) > b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__lt__", [](const FieldData& a, const FieldData& b) { return a < b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__lt__",
             [](const FieldData& a, const pybind11::object& b) { return a < ObjectToFieldData(b); },
            pybind11::call_guard<SignalsGuard>())
        .def("__lt__",
             [](const pybind11::object& a, const FieldData& b) { return ObjectToFieldData(a) < b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__le__", [](const FieldData& a, const FieldData& b) { return a <= b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__le__", [](const FieldData& a,
                          const pybind11::object& b) { return a <= ObjectToFieldData(b); },
            pybind11::call_guard<SignalsGuard>())
        .def("__le__", [](const pybind11::object& a,
                          const FieldData& b) { return ObjectToFieldData(a) <= b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__ge__", [](const FieldData& a, const FieldData& b) { return a >= b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__ge__", [](const FieldData& a,
                          const pybind11::object& b) { return a >= ObjectToFieldData(b); },
            pybind11::call_guard<SignalsGuard>())
        .def("__ge__", [](const pybind11::object& a,
                          const FieldData& b) { return ObjectToFieldData(a) >= b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__neq__", [](const FieldData& a, const FieldData& b) { return a != b; },
            pybind11::call_guard<SignalsGuard>())
        .def("__neq__", [](const FieldData& a,
                           const pybind11::object& b) { return a != ObjectToFieldData(b); },
            pybind11::call_guard<SignalsGuard>())
        .def("__neq__", [](const pybind11::object& a, const FieldData& b) {
            return ObjectToFieldData(a) != b;
        },
            pybind11::call_guard<SignalsGuard>());

    pybind11::enum_<FieldType>(m, "FieldType", pybind11::arithmetic(), "Data type of FieldData.")
        .value("NUL", FieldType::NUL)
        .value("BOOL", FieldType::BOOL)
        .value("INT8", FieldType::INT8)
        .value("INT16", FieldType::INT16)
        .value("INT32", FieldType::INT32)
        .value("INT64", FieldType::INT64)
        .value("FLOAT", FieldType::FLOAT)
        .value("DOUBLE", FieldType::DOUBLE)
        .value("DATE", FieldType::DATE)
        .value("DATETIME", FieldType::DATETIME)
        .value("STRING", FieldType::STRING)
        .value("BLOB", FieldType::BLOB)
        .export_values();

    pybind11::enum_<lgraph_api::AccessLevel>(m, "AccessLevel", pybind11::arithmetic(),
                                             "Access that a user has on a graph.")
        .value("NONE", AccessLevel::NONE)
        .value("READ", AccessLevel::READ)
        .value("WRITE", AccessLevel::WRITE)
        .value("FULL", AccessLevel::FULL)
        .export_values();

    pybind11::enum_<::lgraph::python_plugin::TaskOutput::ErrorCode>(
        m, "PluginErrorCode", pybind11::arithmetic(), "ErrorCode of plugin.")
        .value("SUCCESS", ::lgraph::python_plugin::TaskOutput::ErrorCode::SUCCESS)
        .value("INPUT_ERR", ::lgraph::python_plugin::TaskOutput::ErrorCode::INPUT_ERR)
        .value("INTERNAL_ERR", ::lgraph::python_plugin::TaskOutput::ErrorCode::INTERNAL_ERR)
        .value("SUCCESS_WITH_SIGNATURE",
               ::lgraph::python_plugin::TaskOutput::ErrorCode::SUCCESS_WITH_SIGNATURE);

    //======================================
    // Register APIs
    //======================================
    pybind11::class_<Galaxy> galaxy(
        m, "Galaxy",
        "A galaxy is a TuGraph instance that holds multiple GraphDBs.\n"
        "A galaxy is stored in a directory and manages users and GraphDBs. "
        "Each (user, GraphDB) pair can have different access levels. "
        "You can use db=Galaxy.OpenGraph(graph) to open a graph. "
        "Since garbage collection in Python is automatic, you need to "
        "close the galaxy with Galaxy.Close() when you are done with it.");
    galaxy
        .def(
            "__enter__", [&](Galaxy& r) -> Galaxy& { return r; }, "Init galaxy.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__exit__",
            [&](Galaxy& g, pybind11::object exc_type, pybind11::object exc_value,
                pybind11::object traceback) { g.Close(); },
            "Release memory of this galaxy.",
            pybind11::call_guard<SignalsGuard>());
    galaxy.def(pybind11::init<const std::string&, bool, bool>(),
               "Initializes a galaxy instance stored in dir.\n"
               "dir: directory of the database\n"
               "durable: whether to turn on durable mode. Note that a database can only be opened"
               "by one process in durable mode.\n"
               "create_if_not_exist: whether to create the database if dir does not exist",
               pybind11::arg("dir"), pybind11::arg("durable") = false,
               pybind11::arg("create_if_not_exist") = false,
               pybind11::return_value_policy::move,
               pybind11::call_guard<SignalsGuard>());
    galaxy.def("SetCurrentUser", &Galaxy::SetCurrentUser,
               "Validate user password and set current user.\n"
               "user: user name\n"
               "password: password of the user",
               pybind11::arg("user"), pybind11::arg("password"),
               pybind11::call_guard<SignalsGuard>());
    galaxy.def("SetUser", &Galaxy::SetUser,
               "Validate the given user and set current user given in the user.",
               pybind11::arg("user"),
               pybind11::call_guard<SignalsGuard>());
    galaxy.def("Close", &Galaxy::Close, "Closes this galaxy")
        .def("CreateGraph", &Galaxy::CreateGraph,
             "Creates a graph.\n"
             "name: the name of the graph\n"
             "description: description of the graph\n"
             "max_size: maximum size of the graph, default 1TB",
             pybind11::arg("name"), pybind11::arg("description") = "",
             pybind11::arg("max_size") = lgraph::_detail::DEFAULT_GRAPH_SIZE,
             pybind11::call_guard<SignalsGuard>())
        .def("DeleteGraph", &Galaxy::DeleteGraph, "Deletes a graph",
             pybind11::call_guard<SignalsGuard>())
        .def("ListGraphs", &Galaxy::ListGraphs,
             "Lists graphs and returns a dictionary of {name:(desc, max_size)}",
             pybind11::call_guard<SignalsGuard>())
        .def("ModGraph", &Galaxy::ModGraph,
             "Modifies the information of the graph, returns true if successful, false if no such "
             "graph.\n"
             "graph_name: name of the graph\n"
             "mod_desc: whether to modify description\n"
             "description: new description for the graph\n",
             "mod_size: whether to modify max graph size\n"
             "new_max_size: new maximum size of the graph, in bytes",
             pybind11::arg("graph_name"), pybind11::arg("mod_desc"), pybind11::arg("description"),
             pybind11::arg("mod_size"), pybind11::arg("new_max_size"),
             pybind11::call_guard<SignalsGuard>())
        .def("CreateUser", &Galaxy::CreateUser,
             "Creates a new user account.\n"
             "name: name of the user\n"
             "password: password for the user\n"
             "desc: description of this user",
             pybind11::arg("name"), pybind11::arg("password"), pybind11::arg("desc"),
             pybind11::call_guard<SignalsGuard>())
        .def("DeleteUser", &Galaxy::DeleteUser, "Deletes a user account",
             pybind11::call_guard<SignalsGuard>())
        .def("SetUserPass", &Galaxy::SetPassword,
             "Modifies user password.\n"
             "name: name of the user\n"
             "old_password: current password, not needed when modifying another user\n"
             "new_password: new password for the user",
             pybind11::arg("name"), pybind11::arg("old_password") = "",
             pybind11::arg("new_password"),
             pybind11::call_guard<SignalsGuard>())
        .def("SetUserRoles", &Galaxy::SetUserRoles,
             "Set the roles for the specified user.\n"
             "name: name of the user\n"
             "roles: list of roles for this user",
             pybind11::arg("name"), pybind11::arg("roles"),
             pybind11::call_guard<SignalsGuard>())
        .def("SetUserGraphAccess", &Galaxy::SetUserGraphAccess,
             "Set the access level of the specified user on the graph.\n"
             "user: name of the user\n"
             "graph: name of the graph\n"
             "access: access level of the user on that graph",
             pybind11::arg("user"), pybind11::arg("graph"), pybind11::arg("access"),
             pybind11::call_guard<SignalsGuard>())
        .def("DisableUser", &Galaxy::DisableUser, "Disables a user",
             pybind11::call_guard<SignalsGuard>())
        .def("EnableUser", &Galaxy::EnableUser, "Enables a user",
             pybind11::call_guard<SignalsGuard>())
        .def("ListUsers", &Galaxy::ListUsers, "Lists all users and whether they are admin",
             pybind11::call_guard<SignalsGuard>())
        .def("GetUserInfo", &Galaxy::GetUserInfo, "Get information of the specified user",
             pybind11::call_guard<SignalsGuard>())
        .def("CreateRole", &Galaxy::CreateRole,
             "Create a role.\n"
             "name: name of the role\n"
             "desc: description of the role",
             pybind11::arg("name"), pybind11::arg("desc"),
             pybind11::call_guard<SignalsGuard>())
        .def("DeleteRole", &Galaxy::DeleteRole, "Deletes the specified role",
             pybind11::call_guard<SignalsGuard>())
        .def("SetRoleDesc", &Galaxy::SetRoleDesc,
             "Set description of the specified role.\n"
             "name: name of the role\n"
             "desc: description of the role",
             pybind11::arg("name"), pybind11::arg("desc"),
             pybind11::call_guard<SignalsGuard>())
        .def("SetRoleAccessRights", &Galaxy::SetRoleAccessRights,
             "Set access rights for the specified role",
             pybind11::call_guard<SignalsGuard>())
        .def("SetRoleAccessRightsIncremental", &Galaxy::SetRoleAccessRightsIncremental,
             "Set access rights for the specified role, only affects the specified graphs",
             pybind11::call_guard<SignalsGuard>())
        .def("OpenGraph", &Galaxy::OpenGraph,
             "Opens a graph and returns a GraphDB instance.\n"
             "graph: name of the graph\n"
             "read_only: whether to open the graph in read-only mode",
             pybind11::arg("graph"), pybind11::arg("read_only") = false,
             pybind11::call_guard<SignalsGuard>());

    pybind11::class_<GraphDB> graph_db(
        m, "GraphDB",
        "The graph database class.\n"
        "A GraphDB stores the data about the graph, including labels, vertices, "
        "edges and indexes."
        "Since Garbage Collection in Python is automatic, you need to close the "
        "DB with GraphDB.Close() at the end of its lifetime."
        "Make sure you have either committed or aborted every transaction that "
        "is using the DB before you close the DB.");
    graph_db
        .def(
            "__enter__", [&](GraphDB& r) -> GraphDB& { return r; }, "Init GraphDB.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__exit__",
            [&](GraphDB& g, pybind11::object exc_type, pybind11::object exc_value,
                pybind11::object traceback) { g.Close(); },
            "Release memory of this GraphDB.",
            pybind11::call_guard<SignalsGuard>());
    graph_db.def("Close", &GraphDB::Close, "Closes the DB.",
                 pybind11::call_guard<SignalsGuard>())
        .def("CreateWriteTxn", &GraphDB::CreateWriteTxn,
             "Create a write transaction.\n",
             pybind11::arg("optimistic") = false,
             pybind11::return_value_policy::move,
             pybind11::call_guard<SignalsGuard>())
        .def("CreateReadTxn", &GraphDB::CreateReadTxn, pybind11::return_value_policy::move,
             pybind11::call_guard<SignalsGuard>())
        .def("Flush", &GraphDB::Flush, "Flushes written data into disk.",
             pybind11::call_guard<SignalsGuard>())
        .def("DropAllData", &GraphDB::DropAllData,
             "Drop all the data in this DB.\n"
             "All vertices, edges, labels and indexes will be dropped.",
             pybind11::call_guard<SignalsGuard>())
        .def("DropAllVertex", &GraphDB::DropAllVertex,
             "Drops all the vertices and edges in this DB.\n"
             "Labels and indexes (though index contents will be cleared due to "
             "deletion of vertices) will be preserved.",
             pybind11::call_guard<SignalsGuard>())
        .def("EstimateNumVertices", &GraphDB::EstimateNumVertices,
             "Gets an estimation of the number of vertices.\n"
             "This can be inaccurate if there were vertex removals.",
             pybind11::call_guard<SignalsGuard>())
        .def("AddVertexLabel", &GraphDB::AddVertexLabel, "Add a vertex label.",
             pybind11::arg("label_name"), pybind11::arg("field_specs"),
             pybind11::arg("options"),
             pybind11::call_guard<SignalsGuard>())
        .def(
            "DeleteVertexLabel",
            [](GraphDB& db, const std::string& label) {
                size_t n;
                if (db.DeleteVertexLabel(label, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Deletes a vertex label", pybind11::arg("label_name"),
            pybind11::call_guard<SignalsGuard>())
        .def("AlterEdgeLabelModifyConstraints", &GraphDB::AlterLabelModEdgeConstraints,
             "Modify edge constraints", pybind11::arg("label_name"), pybind11::arg("constraints"),
             pybind11::call_guard<SignalsGuard>())
        .def(
            "AlterVertexLabelDelFields",
            [](GraphDB& db, const std::string& label, const std::vector<std::string>& del_fields) {
                size_t n;
                if (db.AlterVertexLabelDelFields(label, del_fields, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Delete fields from a vertex label\n"
            "label: name of the label\n"
            "del_fields: list of field names",
            pybind11::arg("label"), pybind11::arg("del_fields"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AlterVertexLabelAddFields",
            [](GraphDB& db, const std::string& label, const std::vector<FieldSpec>& add_fields,
               const std::vector<FieldData>& default_values) {
                size_t n;
                if (db.AlterVertexLabelAddFields(label, add_fields, default_values, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Add fields to a vertex label\n"
            "label: name of the label\n"
            "add_fields: list of FieldSpec for the newly added fields\n"
            "default_values: default values of the added fields",
            pybind11::arg("label"), pybind11::arg("add_fields"), pybind11::arg("default_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AlterVertexLabelModFields",
            [](GraphDB& db, const std::string& label, const std::vector<FieldSpec>& mod_fields) {
                size_t n;
                if (db.AlterVertexLabelModFields(label, mod_fields, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Modify fields in a vertex label\n"
            "label: name of the label\n"
            "mod_fields: list of FieldSpec for the modified fields",
            pybind11::arg("label"), pybind11::arg("mod_fields"),
            pybind11::call_guard<SignalsGuard>())
        .def("AddEdgeLabel", &GraphDB::AddEdgeLabel, "Adds an edge label.",
             pybind11::arg("label_name"), pybind11::arg("field_specs"),
             pybind11::arg("options"),
             pybind11::call_guard<SignalsGuard>())
        .def(
            "DeleteEdgeLabel",
            [](GraphDB& db, const std::string& label) {
                size_t n;
                if (db.DeleteEdgeLabel(label, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Deletes an edge label", pybind11::arg("label_name"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AlterEdgeLabelDelFields",
            [](GraphDB& db, const std::string& label, const std::vector<std::string>& del_fields) {
                size_t n;
                if (db.AlterEdgeLabelDelFields(label, del_fields, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Delete fields from an edge label\n"
            "label: name of the label\n"
            "del_fields: list of field names",
            pybind11::arg("label"), pybind11::arg("del_fields"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AlterEdgeLabelAddFields",
            [](GraphDB& db, const std::string& label, const std::vector<FieldSpec>& add_fields,
               const std::vector<FieldData>& default_values) {
                size_t n;
                if (db.AlterEdgeLabelAddFields(label, add_fields, default_values, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Add fields to an edge label\n"
            "label: name of the label\n"
            "add_fields: list of FieldSpec for the newly added fields\n"
            "default_values: default values of the added fields",
            pybind11::arg("label"), pybind11::arg("add_fields"), pybind11::arg("default_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AlterEdgeLabelModFields",
            [](GraphDB& db, const std::string& label, const std::vector<FieldSpec>& mod_fields) {
                size_t n;
                if (db.AlterEdgeLabelModFields(label, mod_fields, &n)) return n;
                throw lgraph::InputError("No such label.");
            },
            "Modify fields in an edge label\n"
            "label: name of the label\n"
            "mod_fields: list of FieldSpec for the modified fields",
            pybind11::arg("label"), pybind11::arg("mod_fields"),
            pybind11::call_guard<SignalsGuard>())
        .def("AddVertexIndex", &GraphDB::AddVertexIndex, "Adds an index.",
             pybind11::arg("label_name"), pybind11::arg("field_name"), pybind11::arg("type"),
             pybind11::call_guard<SignalsGuard>())
        .def("IsVertexIndexed", &GraphDB::IsVertexIndexed,
             "Tells whether the specified field is indexed.", pybind11::arg("label_name"),
             pybind11::arg("field_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("DeleteVertexIndex", &GraphDB::DeleteVertexIndex, "Deletes the specified index.",
             pybind11::arg("label_name"), pybind11::arg("field_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetDescription", &GraphDB::GetDescription, "Gets description of the graph.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetMaxSize", &GraphDB::GetMaxSize, "Gets maximum size of the graph.",
             pybind11::call_guard<SignalsGuard>());

    pybind11::class_<Transaction>(
        m, "Transaction",
        "In embedded mode, all the operations are performed in transactions and "
        "thus enjoys the power of\n"
        "transactions such as atomicity and isolation. You can commit or abort a "
        "transaction at any time\n"
        "without worrying about the side effects it has already made.\n"
        "Transactions can be either committed or aborted, after which it is "
        "destructed and becomes invalid.\n"
        "Make sure you have destructed every transaction before you closes the "
        "corresponding GraphDB.\n"
        "Transactions also track the created iterators and releases all the "
        "iterators during destruction.")
        .def(
            "__enter__", [&](Transaction& r) -> Transaction& { return r; }, "Init Transaction.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__exit__",
            [&](Transaction& t, pybind11::object exc_type, pybind11::object exc_value,
                pybind11::object traceback) { t.Abort(); },
            "Aborts this transaction if it has not been committed.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "VertexToString",
            [](Transaction& a, int64_t vid) {
                VertexIterator vit = a.GetVertexIterator(vid);
                return vit.ToString();
            },
            "Returns the string representation of the vertex specified by vid.",
            pybind11::arg("vid"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "DumpGraph",
            [](Transaction& a) {
                std::cout << "Current graph: {\n";
                for (auto it = a.GetVertexIterator(); it.IsValid(); it.Next()) {
                    std::cout << it.ToString() << "\n";
                }
                std::cout << "}\n";
            },
            "Prints the string representation of the WHOLE graph to stdout.",
            pybind11::call_guard<SignalsGuard>())
        .def("Commit", &Transaction::Commit,
             pybind11::call_guard<SignalsGuard>())
        .def("Abort", &Transaction::Abort,
             pybind11::call_guard<SignalsGuard>())
        .def("IsValid", &Transaction::IsValid,
             pybind11::call_guard<SignalsGuard>())
        .def("IsReadOnly", &Transaction::IsReadOnly,
             pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIterator", [](Transaction& a) { return a.GetVertexIterator(); },
            "Returns a VertexIterator pointing to the first vertex in the graph.",
            pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIterator", [](Transaction& a, int64_t id) { return a.GetVertexIterator(id); },
            "Returns a VertexIterator pointing to the vertex specified by vid.",
            pybind11::arg("vid"), pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIterator",
            [](Transaction& a, int64_t id, bool nearest) {
                return a.GetVertexIterator(id, nearest);
            },
            "Gets VertexIterator with vertex id.\n"
            "If nearest==true, go to the first vertex with id >= vid.",
            pybind11::arg("vid"), pybind11::arg("nearest"), pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetOutEdgeIterator",
            [](Transaction& txn, EdgeUid euid, bool nearest) {
                return txn.GetOutEdgeIterator(euid, nearest);
            },
            "Gets an OutEdgeIterator pointing to the edge identified by euid.",
            pybind11::arg("euid"), pybind11::arg("nearest"), pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetOutEdgeIterator",
            [](Transaction& txn, int64_t src, int64_t dst, int16_t label_id) {
                return txn.GetOutEdgeIterator(src, dst, label_id);
            },
            "Gets an OutEdgeIterator from src to dst with label specified by label_id.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_id"),
            pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetInEdgeIterator",
            [](Transaction& txn, EdgeUid euid, bool nearest) {
                return txn.GetInEdgeIterator(euid, nearest);
            },
            "Gets an InEdgeIterator pointing to the in-edge of vertex dst with "
            "EdgeUid==euid.",
            pybind11::arg("euid"), pybind11::arg("nearest"), pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetInEdgeIterator",
            [](Transaction& txn, int64_t src, int64_t dst, int16_t label_id) {
                return txn.GetInEdgeIterator(src, dst, label_id);
            },
            "Gets an InEdgeIterator from src to dst with label specified by label_id.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_id"),
            pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def("GetNumVertexLabels", &Transaction::GetNumVertexLabels,
             pybind11::call_guard<SignalsGuard>())
        .def("GetNumEdgeLabels", &Transaction::GetNumEdgeLabels,
             pybind11::call_guard<SignalsGuard>())
        .def("ListVertexLabels", &Transaction::ListVertexLabels,
             pybind11::call_guard<SignalsGuard>())
        .def("ListEdgeLabels", &Transaction::ListEdgeLabels,
             pybind11::call_guard<SignalsGuard>())
        .def("GetVertexLabelId", &Transaction::GetVertexLabelId,
             "Gets the vertex label id associated with this label.\n"
             "GraphDB assigns integer ids to each label. "
             "Using label id instead of label name can have performance benefits.",
             pybind11::arg("label_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetEdgeLabelId", &Transaction::GetEdgeLabelId,
             "Gets the edge label id associated with this label.\n"
             "GraphDB assigns integer ids to each label. "
             "Using label id instead of label name can have performance benefits.",
             pybind11::arg("label_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetVertexSchema", &Transaction::GetVertexSchema,
             "Gets the schema specification of the vertex label.", pybind11::arg("label_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetEdgeSchema", &Transaction::GetEdgeSchema,
             "Gets the schema specification of the edge label.", pybind11::arg("label_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetVertexFieldId", &Transaction::GetVertexFieldId,
             "Gets the vertex field id associated with this (label_id, field_name).\n"
             "GraphDB assigns integer ids to each field of the same label. "
             "Using field id instead of field name can have performance benefits.",
             pybind11::arg("label_id"), pybind11::arg("field_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetVertexFieldIds", &Transaction::GetVertexFieldIds,
             "Gets the vertex field ids associated with this (label_id, [field_names]).\n"
             "GraphDB assigns integer ids to each field of the same label. "
             "Using field id instead of field name can have performance benefits.",
             pybind11::arg("label_id"), pybind11::arg("field_names"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetEdgeFieldId", &Transaction::GetEdgeFieldId,
             "Gets the edge field id associated with this (label_id, field_name).\n"
             "GraphDB assigns integer ids to each field of the same label. "
             "Using field id instead of field name can have performance benefits.",
             pybind11::arg("label_id"), pybind11::arg("field_name"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetEdgeFieldId", &Transaction::GetEdgeFieldIds,
             "Gets the edge field ids associated with this (label_id, field_names).\n"
             "GraphDB assigns integer ids to each field of the same label. "
             "Using field id instead of field name can have performance benefits.",
             pybind11::arg("label_id"), pybind11::arg("field_names"),
             pybind11::call_guard<SignalsGuard>())
        .def(
            "AddVertex",
            [](Transaction& a, const std::string& label_name,
               const std::vector<std::string>& field_names,
               const std::vector<std::string>& field_value_strings) {
                return a.AddVertex(label_name, field_names, field_value_strings);
            },
            "Adds a vertex with the specified label name, field names, and field "
            "values in string format.\n"
            "Returns the id of the newly added vertex.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("label_name"), pybind11::arg("field_names"),
            pybind11::arg("field_value_strings"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AddVertex",
            [](Transaction& a, const std::string& label_name,
               const std::vector<std::string>& field_names,
               const std::vector<FieldData>& field_values) {
                return a.AddVertex(label_name, field_names, field_values);
            },
            "Adds a vertex with the specified label name, field names, and field "
            "values.\n"
            "Returns the id of the newly added vertex.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("label_name"), pybind11::arg("field_names"),
            pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AddVertex",
            [](Transaction& a, size_t label_id, const std::vector<size_t>& field_ids,
               const std::vector<FieldData>& field_values) {
                return a.AddVertex(label_id, field_ids, field_values);
            },
            "Adds a vertex with the specified label ids, field ids, and field "
            "values.\n"
            "Returns the id of the newly added vertex.\n"
            "Fields that are not in field_ids are considered null.",
            pybind11::arg("label_id"), pybind11::arg("field_ids"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AddVertex",
            [](Transaction& a, const std::string& label_name, const pybind11::dict& value_dict) {
                std::vector<std::string> fnames;
                std::vector<FieldData> fdata;
                PyDictToVectors(value_dict, fnames, fdata);
                return a.AddVertex(label_name, fnames, fdata);
            },
            "Adds a vertex with the specified label name and set the value as "
            "specified in value_dict.\n"
            "Returns the id of the newly added vertex.\n"
            "Fields that are not specified in the dict are considered null.",
            pybind11::arg("label_name"), pybind11::arg("value_dict"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AddEdge",
            [](Transaction& a, int64_t src, int64_t dst, const std::string& label_name,
               const std::vector<std::string>& field_names,
               const std::vector<std::string>& field_value_strings) {
                return a.AddEdge(src, dst, label_name, field_names, field_value_strings);
            },
            "Adds an edge from src to dst with the specified label name, field "
            "names, and field values in string format.\n"
            "Returns the id of the newly added edge.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_name"),
            pybind11::arg("field_names"), pybind11::arg("field_value_strings"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AddEdge",
            [](Transaction& a, int64_t src, int64_t dst, const std::string& label_name,
               const std::vector<std::string>& field_names,
               const std::vector<FieldData>& field_values) {
                return a.AddEdge(src, dst, label_name, field_names, field_values);
            },
            "Adds an edge from src to dst with the specified label name, field "
            "names, and field values.\n"
            "Returns the id of the newly added edge.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_name"),
            pybind11::arg("field_names"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AddEdge",
            [](Transaction& a, int64_t src, int64_t dst, size_t label_id,
               const std::vector<size_t>& field_ids, const std::vector<FieldData>& field_values) {
                return a.AddEdge(src, dst, label_id, field_ids, field_values);
            },
            "Adds an edge from src to dst with the specified label id, field "
            "ids, and field values.\n"
            "Returns the id of the newly added edge.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_id"),
            pybind11::arg("field_ids"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "AddEdge",
            [](Transaction& a, int64_t src, int64_t dst, std::string& label,
               const pybind11::dict& value_dict) {
                std::vector<std::string> fnames;
                std::vector<FieldData> fdata;
                PyDictToVectors(value_dict, fnames, fdata);
                return a.AddEdge(src, dst, label, fnames, fdata);
            },
            "Adds an edge from src to dst with the specified label, and fill it "
            "with the values given in value_dict.\n"
            "Returns the id of the newly added edge.\n"
            "Fields that are not in value_dict are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_name"),
            pybind11::arg("value_dict"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "UpsertEdge",
            [](Transaction& a, int64_t src, int64_t dst, const std::string& label_name,
               const std::vector<std::string>& field_names,
               const std::vector<std::string>& field_value_strings) {
                return a.UpsertEdge(src, dst, label_name, field_names, field_value_strings);
            },
            "Upserts an edge from src to dst with the specified label name, "
            "field names, and field values in string format.\n"
            "If an src->dst edge already exists, it is updated with the new "
            "value. Otherwise a new edge is created.\n"
            "Returns True if the edge is created, False if the edge is updated.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_name"),
            pybind11::arg("field_names"), pybind11::arg("field_value_strings"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "UpsertEdge",
            [](Transaction& a, int64_t src, int64_t dst, size_t label_id,
               const std::vector<size_t>& field_ids, const std::vector<FieldData>& field_values) {
                return a.UpsertEdge(src, dst, (uint16_t)label_id, field_ids, field_values);
            },
            "Upserts an edge from src to dst with the specified label id, field "
            "ids, and field values.\n"
            "If an src->dst edge already exists, it is updated with the new "
            "value. Otherwise a new edge is created.\n"
            "Returns True if the edge is created, False if the edge is updated.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_id"),
            pybind11::arg("field_ids"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "UpsertEdge",
            [](Transaction& a, int64_t src, int64_t dst, const std::string& label_name,
               const std::vector<std::string>& field_names,
               const std::vector<FieldData>& field_values) {
                return a.UpsertEdge(src, dst, label_name, field_names, field_values);
            },
            "Upserts an edge from src to dst with the specified label name, "
            "field names, and field values.\n"
            "If an src->dst edge already exists, it is updated with the new "
            "value. Otherwise a new edge is created.\n"
            "Returns True if the edge is created, False if the edge is updated.\n"
            "Fields that are not in field_names are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_name"),
            pybind11::arg("field_names"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "UpsertEdge",
            [](Transaction& a, int64_t src, int64_t dst, std::string& label,
               const pybind11::dict& value_dict) {
                std::vector<std::string> fnames;
                std::vector<FieldData> fdata;
                PyDictToVectors(value_dict, fnames, fdata);
                return a.UpsertEdge(src, dst, label, fnames, fdata);
            },
            "Upserts an edge from src to dst with the specified label, and fill "
            "it with the values given in value_dict.\n"
            "If an src->dst edge already exists, it is updated with the new "
            "value. Otherwise a new edge is created.\n"
            "Returns True if the edge is created, False if the edge is updated.\n"
            "Fields that are not in value_dict are considered null.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("label_name"),
            pybind11::arg("value_dict"),
            pybind11::call_guard<SignalsGuard>())
        .def("ListVertexIndexes", &Transaction::ListVertexIndexes,
             "Gets the list of all the vertex indexes in the DB.",
             pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIndexIterator",
            [](Transaction& a, size_t label_id, size_t field_id, const FieldData& key_start,
               const FieldData& key_end) {
                return a.GetVertexIndexIterator(label_id, field_id, key_start, key_end);
            },
            "Gets an VertexIndexIterator pointing to the indexed item which has index "
            "value [key_start, key_end].\n"
            "key_start=key_end=v returns an iterator pointing to all vertexes "
            "that has field value v.\n"
            "label_id specifies the id of the indexed label.\n"
            "field_id specifies the id of the indexed field.\n"
            "key_start is a FieldData containing the minimum indexed value.\n"
            "key_end is a FieldData containing the maximum indexed value.",
            pybind11::arg("label_id"), pybind11::arg("field_id"), pybind11::arg("key_start"),
            pybind11::arg("key_end"), pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIndexIterator",
            [](Transaction& a, size_t label_id, size_t field_id, const FieldData& value) {
                return a.GetVertexIndexIterator(label_id, field_id, value, value);
            },
            "Gets an VertexIndexIterator pointing to the indexed item which has index "
            "value [value].",
            "label_id specifies the id of the indexed label.\n"
            "field_id specifies the id of the indexed field.\n"
            "value is a FieldData containing the indexed value.",
            pybind11::arg("label_id"), pybind11::arg("field_id"), pybind11::arg("value"),
            pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIndexIterator",
            [](Transaction& a, const std::string& label, const std::string& field,
               const std::string& key_start, const std::string& key_end) {
                return a.GetVertexIndexIterator(label, field, key_start, key_end);
            },
            "Gets an VertexIndexIterator pointing to the indexed item which has index "
            "value [key_start, key_end].\n"
            "key_start=key_end=v returns an iterator pointing to all vertexes "
            "that has field value v.",
            "label_name specifies the name of the indexed label.\n"
            "field_id specifies the name of the indexed field.\n"
            "key_start_string is the string representation of the minimum "
            "indexed value.\n"
            "key_end_string is the string representation of the maximum indexed "
            "value.",
            pybind11::arg("label_name"), pybind11::arg("field_name"),
            pybind11::arg("key_start_string"), pybind11::arg("key_end_string"),
            pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIndexIterator",
            [](Transaction& a, const std::string& label, const std::string& field,
               const FieldData& key_start, const FieldData& key_end) {
                return a.GetVertexIndexIterator(label, field, key_start, key_end);
            },
            "Gets an VertexIndexIterator pointing to the indexed item which has index "
            "value [key_start, key_end].\n"
            "key_start=key_end=v returns an iterator pointing to all vertexes "
            "that has field value v.",
            "label_name specifies the name of the indexed label.\n"
            "field_id specifies the name of the indexed field.\n"
            "key_start is a FieldData containing the minimum indexed value.\n"
            "key_end is a FieldData containing the maximum indexed value.",
            pybind11::arg("label_name"), pybind11::arg("field_name"), pybind11::arg("key_start"),
            pybind11::arg("key_end"), pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIndexIterator",
            [](Transaction& a, const std::string& label, const std::string& field,
               const std::string& value_string) {
                return a.GetVertexIndexIterator(label, field, value_string, value_string);
            },
            "Gets an VertexIndexIterator pointing to the indexed item which has index "
            "value given as value_string.",
            "label_name specifies the name of the indexed label.\n"
            "field_id specifies the name of the indexed field.\n"
            "value_string is the string representation of the indexed value.",
            pybind11::arg("label_name"), pybind11::arg("field_name"), pybind11::arg("value_string"),
            pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexIndexIterator",
            [](Transaction& a, const std::string& label, const std::string& field,
               const FieldData& value) {
                return a.GetVertexIndexIterator(label, field, value, value);
            },
            "Gets an VertexIndexIterator pointing to the indexed item which has index "
            "value given as value.",
            "label_name specifies the name of the indexed label.\n"
            "field_id specifies the name of the indexed field.\n"
            "value is a FieldData containing the indexed value.",
            pybind11::arg("label_name"), pybind11::arg("field_name"), pybind11::arg("value"),
            pybind11::return_value_policy::move,
            pybind11::call_guard<SignalsGuard>())
        .def("IsVertexIndexed", &Transaction::IsVertexIndexed,
             "Tells whether the specified field is indexed.", pybind11::arg("label_name"),
             pybind11::arg("field_name"),
             pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexByUniqueIndex",
            [](Transaction& txn, const std::string& label_name, const std::string& field_name,
               const std::string& field_value_string) {
                return txn.GetVertexByUniqueIndex(label_name, field_name, field_value_string);
            },
            "Gets vertex iterator by unique index.\n"
            "Throws exception if there is no such vertex.\n"
            "label_name specifies the name of the indexed label.\n"
            "field_name specifies the name of the indexed field.\n"
            "field_value_string specifies the string representation of the "
            "indexed field value.",
            pybind11::arg("label_name"), pybind11::arg("field_name"),
            pybind11::arg("field_value_string"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexByUniqueIndex",
            [](Transaction& txn, const std::string& label_name, const std::string& field_name,
               const pybind11::object& field_value) {
                return txn.GetVertexByUniqueIndex(label_name, field_name,
                                                  ObjectToFieldData(field_value));
            },
            "Gets vertex iterator by unique index.\n"
            "Throws exception if there is no such vertex.\n"
            "label_name specifies the name of the indexed label.\n"
            "field_name specifies the name of the indexed field.\n"
            "field_value specifies the indexed field value.",
            pybind11::arg("label_name"), pybind11::arg("field_name"), pybind11::arg("field_value"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetVertexByUniqueIndex",
            [](Transaction& txn, size_t label_id, size_t field_id, const FieldData& field_value) {
                return txn.GetVertexByUniqueIndex(label_id, field_id, field_value);
            },
            "Gets vertex iterator by unique index.\n"
            "Throws exception if there is no such vertex.\n"
            "label_id specifies the id of the indexed label.\n"
            "field_id specifies the id of the indexed field.\n"
            "field_value is a FieldData specifying the indexed field value.",
            pybind11::arg("label_id"), pybind11::arg("field_id"), pybind11::arg("field_value"),
            pybind11::call_guard<SignalsGuard>());

    // Vertex iterator
    pybind11::class_<VertexIterator>(
        m, "VertexIterator",
        "VertexIterator can be used to retrieve info of a vertex, or to scan "
        "through multiple vertices.\n"
        "Vertexes are sorted in ascending order of the their ids.")
        .def(
            "__enter__", [&](VertexIterator& r) -> VertexIterator& { return r; }, "Init iterator.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__exit__",
            [&](VertexIterator& r, pybind11::object exc_type, pybind11::object exc_value,
                pybind11::object traceback) { r.Close(); },
            "Delete iterator",
            pybind11::call_guard<SignalsGuard>())
        .def("Next", &VertexIterator::Next, "Goes to the next vertex with id>{current_vid}.",
             pybind11::call_guard<SignalsGuard>())
        .def("Goto", &VertexIterator::Goto,
             "Goes to the vertex specified by vid.\n"
             "If nearest==true, go to the nearest vertex with id>=vid.",
             pybind11::arg("vid"), pybind11::arg("nearest"),
             pybind11::call_guard<SignalsGuard>())
        .def("GetId", &VertexIterator::GetId,
             "Gets the integer id of this vertex.\n"
             "GraphDB assigns an integer id for each vertex.",
             pybind11::call_guard<SignalsGuard>())
        .def(
            "GetOutEdgeIterator", [](VertexIterator& vit) { return vit.GetOutEdgeIterator(); },
            "Gets an OutEgdeIterator pointing to the first out-going edge of "
            "this edge.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetOutEdgeIterator",
            [](VertexIterator& vit, const EdgeUid& euid, bool nearest) {
                return vit.GetOutEdgeIterator(euid, nearest);
            },
            "Gets an OutEgdeIterator pointing to the out-edge of this vertex "
            "with EdgeUid==euid.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetInEdgeIterator", [](VertexIterator& vit) { return vit.GetInEdgeIterator(); },
            "Gets an InEgdeIterator pointing to the first in-coming edge of this "
            "edge.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetInEdgeIterator",
            [](VertexIterator& vit, const EdgeUid euid, bool nearest) {
                return vit.GetInEdgeIterator(euid, nearest);
            },
            "Gets an InEgdeIterator pointing to the in-edge of this vertex with "
            "EdgeUid==euid.",
            pybind11::call_guard<SignalsGuard>())
        .def("IsValid", &VertexIterator::IsValid,
             pybind11::call_guard<SignalsGuard>())
        .def("GetLabel", &VertexIterator::GetLabel, "Gets the label name of current vertex.",
             pybind11::return_value_policy::copy,
             pybind11::call_guard<SignalsGuard>())
        .def("GetLabelId", &VertexIterator::GetLabelId, "Gets the label id of current vertex.",
             pybind11::call_guard<SignalsGuard>())
        .def(
            "GetField",
            [](VertexIterator& vit, const std::string& field_name) {
                return FieldDataToPyObj(vit.GetField(field_name));
            },
            "Gets the field value of the field specified by field_name.",
            pybind11::arg("field_name"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__getitem__",
            [](VertexIterator& vit, const std::string& field_name) {
                return FieldDataToPyObj(vit.GetField(field_name));
            },
            "Gets the field value of the field specified by field_name.",
            pybind11::arg("field_name"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetField",
            [](VertexIterator& vit, size_t field_id) {
                return FieldDataToPyObj(vit.GetField(field_id));
            },
            "Gets the field value of the field specified by field_id.", pybind11::arg("field_id"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__getitem__",
            [](VertexIterator& vit, size_t field_id) {
                return FieldDataToPyObj(vit.GetField(field_id));
            },
            "Gets the field value of the field specified by field_id.", pybind11::arg("field_id"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetFields",
            [](VertexIterator& vit, const std::vector<std::string>& field_names) {
                return FieldDataVectorToPyList(vit.GetFields(field_names));
            },
            "Gets the field values of the fields specified by field_names.",
            pybind11::arg("field_names"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetFields",
            [](VertexIterator& vit, const std::vector<size_t>& field_ids) {
                return FieldDataVectorToPyList(vit.GetFields(field_ids));
            },
            "Gets the field values of the fields specified by field_ids.",
            pybind11::arg("field_ids"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetAllFields",
            [](VertexIterator& vit) { return FieldDataMapToPyDict(vit.GetAllFields()); },
            "Gets all the field values and return as a dict.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetField",
            [](VertexIterator& vit, const std::string& field_name,
               const pybind11::object& field_value_object) {
                FieldData field_value = ObjectToFieldData(field_value_object);
                return vit.SetField(field_name, field_value);
            },
            "Sets the specified field", pybind11::arg("field_name"),
            pybind11::arg("field_value_object"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](VertexIterator& vit, const std::vector<std::string>& field_names,
               const std::vector<std::string>& field_value_strings) {
                return vit.SetFields(field_names, field_value_strings);
            },
            "Sets the fields specified by field_names with field values in "
            "string representation.\n"
            "field_names specifies the names of the fields to set.\n"
            "field_value_strings are the field values in string representation.",
            pybind11::arg("field_names"), pybind11::arg("field_value_strings"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](VertexIterator& vit, const std::vector<std::string>& field_names,
               const std::vector<FieldData>& field_values) {
                return vit.SetFields(field_names, field_values);
            },
            "Sets the fields specified by field_names with new values.\n"
            "field_names specifies the names of the fields to set.\n"
            "field_values are the FieldData containing field values.",
            pybind11::arg("field_names"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](VertexIterator& vit, const pybind11::dict& value_dict) {
                std::vector<std::string> fnames;
                std::vector<FieldData> fdata;
                PyDictToVectors(value_dict, fnames, fdata);
                return vit.SetFields(fnames, fdata);
            },
            "Sets the fields with values as specified in value_dict.\n"
            "value_dict specifies the field_name:value dict.",
            pybind11::arg("value_dict"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](VertexIterator& vit, const std::vector<size_t>& field_ids,
               const std::vector<FieldData>& field_values) {
                return vit.SetFields(field_ids, field_values);
            },
            "Sets the fields specified by field_ids with field values.\n"
            "field_ids specifies the ids of the fields to set.\n"
            "field_values are the field values to be set.",
            pybind11::arg("field_ids"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "ListSrcVids",
            [](VertexIterator& vit, size_t n_limit) {
                bool more_to_go = false;
                auto vids = vit.ListSrcVids(n_limit, &more_to_go);
                return std::make_pair(vids, more_to_go);
            },
            "Lists all source vids of the in edges.\n"
            "n_limit specifies the maximum number of src vids to return.\n"
            "Returns a tuple containing a list of vids and a bool value "
            "indicating whether the limit is exceeded.",
            pybind11::arg("n_limit") = std::numeric_limits<size_t>::max(),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "ListDstVids",
            [](VertexIterator& vit, size_t n_limit) {
                bool more_to_go = false;
                auto vids = vit.ListDstVids(n_limit, &more_to_go);
                return std::make_pair(vids, more_to_go);
            },
            "Lists all destination vids of the out edges.\n"
            "n_limit specifies the maximum number of vids to return.\n"
            "Returns a tuple containing a list of vids and a bool value "
            "indicating whether the limit is exceeded.",
            pybind11::arg("n_limit") = std::numeric_limits<size_t>::max(),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetNumInEdges",
            [](VertexIterator& vit, size_t n_limit) {
                bool more_to_go = false;
                auto n = vit.GetNumInEdges(n_limit, &more_to_go);
                return std::make_pair(n, more_to_go);
            },
            "Gets the number of in-coming edges of this vertex.\n"
            "n_limit specifies the maximum number of edges to scan.\n"
            "Returns a tuple containing the number of in-edges and a bool value "
            "indicating whether the limit is exceeded.",
            pybind11::arg("n_limit") = std::numeric_limits<size_t>::max(),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetNumOutEdges",
            [](VertexIterator& vit, size_t n_limit) {
                bool more_to_go = false;
                auto n = vit.GetNumOutEdges(n_limit, &more_to_go);
                return std::make_pair(n, more_to_go);
            },
            "Gets the number of out edges of this vertex.\n"
            "n_limit specifies the maximum number of vids to scan."
            "Returns a tuple containing the number of out-edges and a bool value "
            "indicating whether the limit is exceeded.",
            pybind11::arg("n_limit") = std::numeric_limits<size_t>::max(),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "Delete",
            [](::lgraph_api::VertexIterator& vit) {
                size_t ni, no;
                vit.Delete(&ni, &no);
                return std::make_tuple(ni, no);
            },
            "Deletes current vertex.\n"
            "The iterator will point to the next vertex if there is any.",
            pybind11::call_guard<SignalsGuard>())
        .def("ToString", &VertexIterator::ToString,
             "Returns the string representation of current vertex, including "
             "properties and edges.",
             pybind11::call_guard<SignalsGuard>());

    // OutEdgeIterator
    pybind11::class_<OutEdgeIterator>(
        m, "OutEdgeIterator",
        "OutEdgeIterator can be used to iterate through all the out-going edges "
        "of the source vertex.\n"
        "Out-going edges are sorted in (src, lid, dst, eid) order.")
        .def(
            "__enter__", [&](OutEdgeIterator& r) -> OutEdgeIterator& { return r; },
            "Init iterator.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__exit__",
            [&](OutEdgeIterator& r, pybind11::object exc_type, pybind11::object exc_value,
                pybind11::object traceback) { r.Close(); },
            "Delete iterator",
            pybind11::call_guard<SignalsGuard>())
        .def("Goto", &OutEdgeIterator::Goto, "Goes to the out edge specified by euid.\n",
             pybind11::arg("euid"), pybind11::arg("nearest") = false,
             pybind11::call_guard<SignalsGuard>())
        .def("Next", &OutEdgeIterator::Next,
             "Goes to the next out edge from current source vertex.\n"
             "If there is no more out edge left, the iterator becomes invalid.",
             pybind11::call_guard<SignalsGuard>())
        .def("IsValid", &OutEdgeIterator::IsValid, "Tells whether the iterator is valid.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetUid", &OutEdgeIterator::GetUid, "Returns the EdgeUid of the edge.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetSrc", &OutEdgeIterator::GetSrc, "Returns the id of the source vertex.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetDst", &OutEdgeIterator::GetDst, "Returns the id of the destination vertex.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetEdgeId", &OutEdgeIterator::GetEdgeId,
             "Returns the id of current edge. Edge id is unique across the same "
             "(src, dst) set.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetLabel", &OutEdgeIterator::GetLabel, "Returns the name of the edge label.",
             pybind11::return_value_policy::copy,
             pybind11::call_guard<SignalsGuard>())
        .def("GetLabelId", &OutEdgeIterator::GetLabelId, "Returns the id of the edge label.",
             pybind11::call_guard<SignalsGuard>())
        .def(
            "GetField",
            [](OutEdgeIterator& eit, const std::string& field_name) {
                return FieldDataToPyObj(eit.GetField(field_name));
            },
            "Gets the field value of the field specified by field_name.",
            pybind11::arg("field_name"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__getitem__",
            [](OutEdgeIterator& eit, const std::string& field_name) {
                return FieldDataToPyObj(eit.GetField(field_name));
            },
            "Gets the field value of the field specified by field_name.",
            pybind11::arg("field_name"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetField",
            [](OutEdgeIterator& eit, size_t field_id) {
                return FieldDataToPyObj(eit.GetField(field_id));
            },
            "Gets the field value of the field specified by field_id.", pybind11::arg("field_id"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__getitem__",
            [](OutEdgeIterator& eit, size_t field_id) {
                return FieldDataToPyObj(eit.GetField(field_id));
            },
            "Gets the field value of the field specified by field_id.", pybind11::arg("field_id"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetFields",
            [](OutEdgeIterator& eit, const std::vector<std::string>& field_names) {
                return FieldDataVectorToPyList(eit.GetFields(field_names));
            },
            "Gets field values of the fields specified by field_names.",
            pybind11::arg("field_names"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetFields",
            [](OutEdgeIterator& eit, const std::vector<size_t>& field_ids) {
                return FieldDataVectorToPyList(eit.GetFields(field_ids));
            },
            "Gets field values of the fields specified by field_ids.", pybind11::arg("field_ids"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetAllFields",
            [](OutEdgeIterator& vit) { return FieldDataMapToPyDict(vit.GetAllFields()); },
            "Gets all the field values and return as a dict.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetField",
            [](OutEdgeIterator& eit, const std::string& field_name,
               const pybind11::object& field_value_object) {
                FieldData field_value = ObjectToFieldData(field_value_object);
                return eit.SetField(field_name, field_value);
            },
            "Sets the specified field", pybind11::arg("field_name"),
            pybind11::arg("field_value_object"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](OutEdgeIterator& eit, const std::vector<std::string>& field_names,
               const std::vector<std::string>& field_value_strings) {
                return eit.SetFields(field_names, field_value_strings);
            },
            "Sets the fields specified by field_names with field values in "
            "string representation.\n"
            "field_names specifies the names of the fields to set.\n"
            "field_value_strings are the field values in string representation.",
            pybind11::arg("field_names"), pybind11::arg("field_value_strings"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](OutEdgeIterator& eit, const std::vector<std::string>& field_names,
               const std::vector<FieldData>& field_values) {
                return eit.SetFields(field_names, field_values);
            },
            "Sets the fields specified by field_names with field values in "
            "string representation.\n"
            "field_names specifies the names of the fields to set.\n"
            "field_values are FieldData containing the field values.",
            pybind11::arg("field_names"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](OutEdgeIterator& eit, const pybind11::dict& value_dict) {
                std::vector<std::string> fnames;
                std::vector<FieldData> fdata;
                PyDictToVectors(value_dict, fnames, fdata);
                return eit.SetFields(fnames, fdata);
            },
            "Sets the field values as specified in value_dict.\n"
            "value_dict specifies the field_name:value dictionary.",
            pybind11::arg("value_dict"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](OutEdgeIterator& eit, const std::vector<size_t>& field_ids,
               const std::vector<FieldData>& field_values) {
                return eit.SetFields(field_ids, field_values);
            },
            "Sets the fields specified by field_ids with field values.\n"
            "field_ids specifies the ids of the fields to set.\n"
            "field_values are the field values to be set.",
            pybind11::arg("field_ids"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def("Delete", &OutEdgeIterator::Delete,
             "Deletes current edge.\n"
             "The iterator will point to the next out edge if there is any.",
             pybind11::call_guard<SignalsGuard>())
        .def("ToString", &OutEdgeIterator::ToString,
             "Returns the string representation of current edge.",
             pybind11::call_guard<SignalsGuard>());

    // InEdgeIterator
    pybind11::class_<InEdgeIterator>(
        m, "InEdgeIterator",
        "InEdgeIterator can be used to iterate through all the incoming edges of "
        "the destination vertex.\n"
        "Incoming edges are sorted in (dst, label, src, eid) order.")
        .def(
            "__enter__", [&](InEdgeIterator& r) -> InEdgeIterator& { return r; }, "Init iterator.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__exit__",
            [&](InEdgeIterator& r, pybind11::object exc_type, pybind11::object exc_value,
                pybind11::object traceback) { r.Close(); },
            "Delete iterator",
            pybind11::call_guard<SignalsGuard>())
        .def("Goto", &InEdgeIterator::Goto, "Goes to the in edge specified by euid.\n",
             pybind11::arg("euid"), pybind11::arg("nearest"),
             pybind11::call_guard<SignalsGuard>())
        .def("Next", &InEdgeIterator::Next,
             "Goes to the next in edge to current destination vertex.\n"
             "If there is no more in edge left, the iterator becomes invalid.",
             pybind11::call_guard<SignalsGuard>())
        .def("IsValid", &InEdgeIterator::IsValid, "Tells whether the iterator is valid.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetUid", &InEdgeIterator::GetUid, "Returns the EdgeUid of the edge.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetSrc", &InEdgeIterator::GetSrc, "Returns the id of the source vertex.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetDst", &InEdgeIterator::GetDst, "Returns the id of the destination vertex.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetEdgeId", &InEdgeIterator::GetEdgeId,
             "Returns the id of current edge. Edge id is unique across the same "
             "(src, dst) set.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetLabel", &InEdgeIterator::GetLabel, "Returns the name of the edge label.",
             pybind11::return_value_policy::copy,
             pybind11::call_guard<SignalsGuard>())
        .def("GetLabelId", &InEdgeIterator::GetLabelId, "Returns the id of the edge label.",
             pybind11::call_guard<SignalsGuard>())
        .def(
            "GetField",
            [](InEdgeIterator& eit, const std::string& field_name) {
                return FieldDataToPyObj(eit.GetField(field_name));
            },
            "Gets the field value of the field specified by field_name.",
            pybind11::arg("field_name"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__getitem__",
            [](InEdgeIterator& eit, const std::string& field_name) {
                return FieldDataToPyObj(eit.GetField(field_name));
            },
            "Gets the field value of the field specified by field_name.",
            pybind11::arg("field_name"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetField",
            [](InEdgeIterator& eit, size_t field_id) {
                return FieldDataToPyObj(eit.GetField(field_id));
            },
            "Gets the field value of the field specified by field_id.", pybind11::arg("field_id"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__getitem__",
            [](InEdgeIterator& eit, size_t field_id) {
                return FieldDataToPyObj(eit.GetField(field_id));
            },
            "Gets the field value of the field specified by field_id.", pybind11::arg("field_id"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetFields",
            [](InEdgeIterator& eit, const std::vector<std::string>& field_names) {
                return FieldDataVectorToPyList(eit.GetFields(field_names));
            },
            "Gets field values of the fields specified by field_names.",
            pybind11::arg("field_names"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetFields",
            [](InEdgeIterator& eit, const std::vector<size_t>& field_ids) {
                return FieldDataVectorToPyList(eit.GetFields(field_ids));
            },
            "Gets field values of the fields specified by field_ids.", pybind11::arg("field_ids"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "GetAllFields",
            [](InEdgeIterator& vit) { return FieldDataMapToPyDict(vit.GetAllFields()); },
            "Gets all the field values and return as a dict.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetField",
            [](InEdgeIterator& eit, const std::string& field_name,
               const pybind11::object& field_value_object) {
                FieldData field_value = ObjectToFieldData(field_value_object);
                return eit.SetField(field_name, field_value);
            },
            "Sets the specified field", pybind11::arg("field_name"),
            pybind11::arg("field_value_object"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](InEdgeIterator& eit, const std::vector<std::string>& field_names,
               const std::vector<std::string>& field_value_strings) {
                return eit.SetFields(field_names, field_value_strings);
            },
            "Sets the fields specified by field_names with field values in "
            "string representation.\n"
            "field_names specifies the names of the fields to set.\n"
            "field_value_strings are the field values in string representation.",
            pybind11::arg("field_names"), pybind11::arg("field_value_strings"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](InEdgeIterator& vit, const std::vector<std::string>& field_names,
               const std::vector<FieldData>& field_values) {
                return vit.SetFields(field_names, field_values);
            },
            "Sets the fields specified by field_names with new values.\n"
            "field_names specifies the names of the fields to set.\n"
            "field_values are the FieldData containing field values.",
            pybind11::arg("field_names"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](InEdgeIterator& vit, const pybind11::dict& value_dict) {
                std::vector<std::string> fnames;
                std::vector<FieldData> fdata;
                PyDictToVectors(value_dict, fnames, fdata);
                return vit.SetFields(fnames, fdata);
            },
            "Sets the fields with values as specified in value_dict.\n"
            "value_dict specifies the field_name:value dict.",
            pybind11::arg("value_dict"),
            pybind11::call_guard<SignalsGuard>())
        .def(
            "SetFields",
            [](InEdgeIterator& eit, const std::vector<size_t>& field_ids,
               const std::vector<FieldData>& field_values) {
                return eit.SetFields(field_ids, field_values);
            },
            "Sets the fields specified by field_ids with field values.\n"
            "field_ids specifies the ids of the fields to set.\n"
            "field_values are the field values to be set.",
            pybind11::arg("field_ids"), pybind11::arg("field_values"),
            pybind11::call_guard<SignalsGuard>())
        .def("Delete", &InEdgeIterator::Delete,
             "Deletes current edge.\n"
             "The iterator will point to the next out edge if there is any.",
             pybind11::call_guard<SignalsGuard>())
        .def("ToString", &InEdgeIterator::ToString,
             "Returns the string representation of current edge.",
             pybind11::call_guard<SignalsGuard>());

    // VertexIndexIterator
    pybind11::class_<VertexIndexIterator>(
        m, "VertexIndexIterator",
        "VertexIndexIterator can be used to retrieve the id of indexed vertices.\n"
        "Vertex ids are sorted in ascending order of (index_value, vertex_id).")
        .def(
            "__enter__", [&](VertexIndexIterator& r) -> VertexIndexIterator& { return r; },
            "Init iterator.",
            pybind11::call_guard<SignalsGuard>())
        .def(
            "__exit__",
            [&](VertexIndexIterator& r, pybind11::object exc_type, pybind11::object exc_value,
                pybind11::object traceback) { r.Close(); },
            "Delete iterator",
            pybind11::call_guard<SignalsGuard>())
        .def("Next", &VertexIndexIterator::Next,
             "Goes to the next indexed vid.\n"
             "If there is no more vertex within the specified key range, the "
             "iterator becomes invalid.",
             pybind11::call_guard<SignalsGuard>())
        .def("IsValid", &VertexIndexIterator::IsValid, "Tells whether this iterator is valid.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetIndexValue", &VertexIndexIterator::GetIndexValue,
             "Gets the indexed value.\n"
             "Since vertex ids are sorted in (index_value, vertex_id) order, "
             "calling Next() may change the current indexed value.",
             pybind11::call_guard<SignalsGuard>())
        .def("GetVid", &VertexIndexIterator::GetVid,
             "Gets the id of the vertex currently pointed to.",
             pybind11::call_guard<SignalsGuard>());
    //====================================
    // Register SigSpec
    //====================================
    pybind11::enum_<lgraph_api::LGraphType>(m, "LGraphType")
        .value("NUL", LGraphType::NUL, "NUL")
        .value("INTEGER", LGraphType::INTEGER, "INTEGER")
        .value("FLOAT", LGraphType::FLOAT, "FLOAT")
        .value("DOUBLE", LGraphType::DOUBLE, "DOUBLE")
        .value("BOOLEAN", LGraphType::BOOLEAN, "BOOLEAN")
        .value("STRING", LGraphType::STRING, "STRING")
        .value("NODE", LGraphType::NODE, "NODE")
        .value("RELATIONSHIP", LGraphType::RELATIONSHIP, "RELATIONSHIP")
        .value("PATH", LGraphType::PATH, "PATH")
        .value("LIST", LGraphType::LIST, "LIST")
        .value("MAP", LGraphType::MAP, "MAP")
        .value("ANY", LGraphType::ANY, "ANY")
        .export_values();

    pybind11::class_<lgraph_api::Parameter>(m, "Parameter")
        .def(pybind11::init<const std::string&, int, LGraphType>())
        .def_readwrite("name", &lgraph_api::Parameter::name, "name of the parameter")
        .def_readwrite("index", &lgraph_api::Parameter::index,
                       "index of the parameter list in which the parameter stay")
        .def_readwrite("type", &lgraph_api::Parameter::type, "type of the parameter");

    pybind11::class_<lgraph_api::SigSpec>(m, "SigSpec")
        .def(pybind11::init<const std::vector<Parameter>&, const std::vector<Parameter>&>(),
             pybind11::call_guard<SignalsGuard>())
        .def(
            "serialize",
            [](lgraph_api::SigSpec& sig_spec) -> std::string {
                fma_common::BinaryBuffer buffer;
                fma_common::BinaryWrite(buffer, sig_spec);
                char *owned_buffer = nullptr;
                size_t size = 0;
                // give ownership to serialized
                buffer.DetachBuf((void **)&owned_buffer, &size);
                return {owned_buffer, size };
            },
            "Serialize SigSpec into string",
            pybind11::call_guard<SignalsGuard>())
        .def_readwrite("input_list", &lgraph_api::SigSpec::input_list,
                       "input parameter list of the signature")
        .def_readwrite("result_list", &lgraph_api::SigSpec::result_list,
                       "return items of the signature");
}  // NOLINT

void register_lgraph_plugin(pybind11::module& m) {
    //======================================
    // Register Python task runner
    //======================================
    pybind11::class_<lgraph::python_plugin::TaskInput>(m, "TaskInput")
        .def_static(
            "ReadTaskInput",
            [](const std::string& pipename) {
                boost::interprocess::message_queue mq(boost::interprocess::open_only,
                                                      pipename.c_str());
                lgraph::python_plugin::TaskInput input;
                while (!input.ReadFromMessageQueue(mq, 100)) {
                    SignalsGuard signalsGuard;
                }
                return input;
            },
            "Read TaskInput from message queue",
            pybind11::call_guard<SignalsGuard>())
        .def_readonly("user", &lgraph::python_plugin::TaskInput::user, "user to be used")
        .def_readonly("graph", &lgraph::python_plugin::TaskInput::graph, "Graph to be used")
        .def_readonly("plugin_dir", &lgraph::python_plugin::TaskInput::plugin_dir,
                      "Directory where .py files are stored")
        .def_readonly("function", &lgraph::python_plugin::TaskInput::function, "The function name")
        .def(
            "get_input",
            [](const lgraph::python_plugin::TaskInput& in) { return pybind11::bytes(in.input); },
            "The input byte array",
            pybind11::call_guard<SignalsGuard>())
        .def_readonly("read_only", &lgraph::python_plugin::TaskInput::read_only);

    pybind11::class_<lgraph::python_plugin::TaskOutput>(m, "TaskOutput")
        .def_static(
            "WriteTaskOutput",
            [](const std::string& pipename, lgraph::python_plugin::TaskOutput::ErrorCode error_code,
               const std::string& output) {
                boost::interprocess::message_queue mq(boost::interprocess::open_only,
                                                      pipename.c_str());
                lgraph::python_plugin::TaskOutput out;
                out.error_code = error_code;
                out.output = output;
                out.WriteToMessageQueue(mq);
            },
            "Write TaskOutput to stdout",
            pybind11::call_guard<SignalsGuard>());
}

class EdgeListWriter {
    fma_common::OutputFmaStream ofs_;
    bool binary_;

 public:
    EdgeListWriter(const std::string& path, bool binary) : ofs_(path), binary_(binary) {}
    void EmitEdge(size_t src, size_t dst) {
        if (binary_) {
            static const size_t length = lgraph::_detail::VID_SIZE * 2;
            char line[length];  // NOLINT
            lgraph::_detail::WriteVid(line, src);
            lgraph::_detail::WriteVid(line + lgraph::_detail::VID_SIZE, dst);
            ofs_.Write(line, length);
        } else {
            std::string line = fma_common::StringFormatter::Format("{} {}\n", src, dst);
            ofs_.Write(line.data(), line.size());
        }
    }
    template <typename Weight>
    void EmitWeightedEdge(size_t src, size_t dst, Weight weight) {
        if (binary_) {
            static const size_t length = lgraph::_detail::VID_SIZE * 2 + sizeof(weight);
            char line[length];  // NOLINT
            lgraph::_detail::WriteVid(line, src);
            lgraph::_detail::WriteVid(line + lgraph::_detail::VID_SIZE, dst);
            memcpy(line + lgraph::_detail::VID_SIZE * 2, &weight, sizeof(weight));
            ofs_.Write(line, length);
        } else {
            std::string line = fma_common::StringFormatter::Format("{} {} {}\n", src, dst, weight);
            ofs_.Write(line.data(), line.size());
        }
    }
    void Close() { ofs_.Close(); }
};

void register_gemini_adapter(pybind11::module& m) {
    pybind11::class_<EdgeListWriter>(m, "EdgeListWriter")
        .def(pybind11::init<const std::string&, bool>(),
             "Open a new file for writing out a list of edges.\n"
             "`binary` denotes whether the output should be in binary (true) or "
             "text (false) format.",
             pybind11::arg("path"), pybind11::arg("binary") = true,
             pybind11::call_guard<SignalsGuard>())
        .def("EmitEdge", &EdgeListWriter::EmitEdge, "Append an edge to the opened file.",
             pybind11::arg("src"), pybind11::arg("dst"),
             pybind11::call_guard<SignalsGuard>())
        .def(
            "EmitWeightedEdge",
            [](EdgeListWriter& self, size_t src, size_t dst, const pybind11::object& weight) {
                if (pybind11::isinstance<pybind11::float_>(weight)) {
                    self.EmitWeightedEdge<float>(src, dst, weight.cast<float>());
                } else if (pybind11::isinstance<pybind11::int_>(weight)) {
                    self.EmitWeightedEdge<int>(src, dst, weight.cast<int>());
                } else if (pybind11::isinstance<pybind11::bool_>(weight)) {
                    self.EmitWeightedEdge<bool>(src, dst, weight.cast<bool>());
                } else {
                    throw std::runtime_error("Not supported weight type.");
                }
            },
            "Append a weighted edge to the opened file.\n"
            "Only {bool, int, float} weights are supported.",
            pybind11::arg("src"), pybind11::arg("dst"), pybind11::arg("weight"),
            pybind11::call_guard<SignalsGuard>())
        .def("Close", &EdgeListWriter::Close, "Close the edge list file.",
             pybind11::call_guard<SignalsGuard>());
}

// Declare the python api with pybind11
PYBIND11_MODULE(liblgraph_python_api, m) {
    python::register_python_api(m);
    python::register_lgraph_plugin(m);
    python::register_gemini_adapter(m);
}
}  // namespace python
}  // namespace lgraph_api

#endif
