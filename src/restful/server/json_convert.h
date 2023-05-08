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

#pragma once

#include <map>
#include <string>
#include <vector>

#include "cpprest/json.h"
#include "fma-common/string_formatter.h"
#include "fma-common/type_traits.h"
#include "fma-common/hardware_info.h"
#include "fma-common/file_system.h"

#include "core/audit_logger.h"
#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "core/global_config.h"
#include "core/task_tracker.h"
#include "core/schema.h"
#include "core/field_extractor.h"
#include "db/acl.h"
#include "plugin/plugin_desc.h"
#include "server/state_machine.h"

namespace lgraph {

#ifdef _WIN32
#define ToUtilityStringT(str) utility::conversions::to_string_t(str)
#define ToStdString(str) utility::conversions::to_utf8string(str)
#else
#define ToUtilityStringT(buf) buf
#define ToStdString(buf) buf
#endif

#define _TU ToUtilityStringT
#define _TS ToStdString

struct AuditLog;

namespace RestStrings {
static const char* EILLEGAL = "Illegal URI.";
static const utility::string_t ACTION = _TU("action");
static const utility::string_t ADDED = _TU("added");
static const utility::string_t ALGO = _TU("algorithm");
static const utility::string_t ALL = _TU("all");
static const utility::string_t DURABLE = _TU("durable");
static const utility::string_t AUTH_METHOD = _TU("auth_method");
static const utility::string_t BEGIN_TIME = _TU("begin_time");
static const utility::string_t BRANCH = _TU("git_branch");
static const utility::string_t CODE = _TU("code_base64");
static const utility::string_t CODE_TYPE = _TU("code_type");
static const utility::string_t COMMIT = _TU("git_commit");
static const utility::string_t CONFIG = _TU("config");
static const utility::string_t CPP = _TU("cpp_plugin");
static const utility::string_t CPU = _TU("cpu");
static const utility::string_t CPP_ID = _TU("cpp_id");
static const utility::string_t CPP_VERSION = _TU("cpp_version");
static const utility::string_t CURR_PASS = _TU("current_password");
static const utility::string_t CYPHER = _TU("cypher");
static const utility::string_t DATA = _TU("data");
static const utility::string_t DB = _TU("db");
static const utility::string_t DBCONFIG = _TU("db_config");
static const utility::string_t DBSPACE = _TU("db_space");
static const utility::string_t DELETED = _TU("deleted");
static const utility::string_t DESC = _TU("description");
static const utility::string_t DESCENDING_ORDER = _TU("descending_order");
static const utility::string_t DIFF = _TU("diff");
static const utility::string_t DISABLE = _TU("disable");
static const utility::string_t DISABLED = _TU("disabled");
static const utility::string_t DISK = _TU("disk");
static const utility::string_t DST = _TU("destination");
static const utility::string_t EDGE = _TU("edge");
static const utility::string_t EID = _TU("edge_id");
static const utility::string_t EIDS = _TU("edge_ids");
static const utility::string_t ELAPSED = _TU("elapsed");
static const utility::string_t ENABLE = _TU("enable");
static const utility::string_t END_TIME = _TU("end_time");
static const utility::string_t ERR_MSG = _TU("error_message");
static const utility::string_t EUID = _TU("uid");
static const utility::string_t EXCP = _TU("exception");
static const utility::string_t FIELD = _TU("field");
static const utility::string_t FIELDS = _TU("fields");
static const utility::string_t GRAPH = _TU("graph");
static const utility::string_t HEADER = _TU("header");
static const utility::string_t IMPORT = _TU("import");
static const utility::string_t EXPORT = _TU("export");
static const utility::string_t IMPORT_TEXT = _TU("text");
static const utility::string_t INDEX = _TU("index");
static const utility::string_t INDEXES = _TU("indexes");
static const utility::string_t INE = _TU("in");
static const utility::string_t INFO = _TU("info");
static const utility::string_t INPROCESS = _TU("in_process");
static const utility::string_t ISADMIN = _TU("is_admin");
static const utility::string_t ISUNIQUE = _TU("is_unique");
static const utility::string_t ISV = _TU("is_vertex");
static const utility::string_t HA_STATE = _TU("ha_state");
static const utility::string_t LABEL = _TU("label");
static const utility::string_t LEADER = _TU("leader");
static const utility::string_t LID = _TU("label_id");
static const utility::string_t LOCATION = _TU("location");
static const utility::string_t LOG = _TU("log");
static const utility::string_t LOGIN = _TU("login");
static const utility::string_t LOGOUT = _TU("logout");
static const utility::string_t UpdateTokenTime = _TU("update_token_time");
static const utility::string_t GetTokenTime = _TU("get_token_time");
static const utility::string_t MAX_SIZE_GB = _TU("max_size_GB");
static const utility::string_t MEM = _TU("memory");
static const utility::string_t MEM_LIMIT = _TU("memory_limit");
static const utility::string_t MISC = _TU("misc");
static const utility::string_t MODIFIED = _TU("modified");
static const utility::string_t NAME = _TU("name");
static const utility::string_t PRIMARY = _TU("primary");
static const utility::string_t EDGE_CONSTRAINTS = _TU("edge_constraints");
static const utility::string_t NEW_PASS = _TU("new_password");
static const utility::string_t NODE = _TU("node");
static const utility::string_t NODES = _TU("nodes");
static const utility::string_t NULLABLE = _TU("optional");
static const utility::string_t NUM_LABELS = _TU("num_label");
static const utility::string_t NUM_LOG = _TU("num_log");
static const utility::string_t NUMV = _TU("num_vertex");
static const utility::string_t OUTE = _TU("out");
static const utility::string_t OVERWRITE = _TU("overwrite");
static const utility::string_t PARAMETERS = _TU("parameters");
static const utility::string_t PASS = _TU("password");
static const utility::string_t PEERS = _TU("peers");
static const utility::string_t PERMISSIONS = _TU("permissions");
static const utility::string_t FIELD_PERMISSIONS = _TU("field_permissions");
static const utility::string_t PLUGINS = _TU("plugins");
static const utility::string_t PROP = _TU("property");
static const utility::string_t PROPS = _TU("properties");
static const utility::string_t PYTHON = _TU("python_plugin");
static const utility::string_t PYTHON_VERSION = _TU("python_version");
static const utility::string_t READONLY = _TU("read_only");
static const utility::string_t REL = _TU("relationship");
static const utility::string_t RELS = _TU("relationships");
static const utility::string_t REST_ADDR = _TU("rest_address");
static const utility::string_t RESTORE = _TU("restore");
static const utility::string_t RESULT = _TU("result");
static const utility::string_t REFRESH = _TU("refresh");
static const utility::string_t ROLE = _TU("role");
static const utility::string_t ROLES = _TU("roles");
static const utility::string_t RPC_ADDR = _TU("rpc_address");
static const utility::string_t SCHEMA = _TU("schema");
static const utility::string_t SCHEMA_TEXT = _TU("text");
static const utility::string_t SCRIPT = _TU("script");
static const utility::string_t SRC = _TU("source");
static const utility::string_t STATE = _TU("state");
static const utility::string_t STATISTICS = _TU("statistics");
static const utility::string_t SUB_GRAPH = _TU("sub_graph");
static const utility::string_t SVR_VER = _TU("server_version");
static const utility::string_t SZ = _TU("size");
static const utility::string_t TASK_ID = _TU("task_id");
static const utility::string_t TASKS = _TU("task");
static const utility::string_t THREAD_ID = _TU("thread_id");
static const utility::string_t TID = _TU("termporal_id");
static const utility::string_t TIME_USED = _TU("time_elapsed");
static const utility::string_t TIMEOUT = _TU("timeout");
static const utility::string_t TOKEN = _TU("jwt");
static const utility::string_t TYPE = _TU("type");
static const utility::string_t UP_TIME = _TU("up_time");
static const utility::string_t USER = _TU("user");
static const utility::string_t USERS = _TU("users");
static const utility::string_t VALUE = _TU("value");
static const utility::string_t VALUES = _TU("values");
static const utility::string_t VER = _TU("lgraph_version");
static const utility::string_t VERTEX = _TU("vertex");
static const utility::string_t VID = _TU("vid");
static const utility::string_t VIDS = _TU("vertex_ids");
static const utility::string_t WEB = _TU("resource");
static const utility::string_t WEB_COMMIT = _TU("web_commit");
static const utility::string_t REFRESH_TIME = _TU("refresh_time");
static const utility::string_t EXPIRE_TIME = _TU("expire_time");
};  // namespace RestStrings

inline bool ExtractIntField(const web::json::value& js, const utility::string_t& field,
                            int64_t& ret) {
    if (!js.has_integer_field(field)) return false;
    ret = js.at(field).as_number().to_int64();
    return true;
}

inline bool ExtractIntField(const web::json::value& js, const utility::string_t& field,
                            int& ret) {
    int64_t r;
    if (!ExtractIntField(js, field, r)) return false;
    // check for overflow
    if (r < std::numeric_limits<int>::min() || r > std::numeric_limits<int>::max()) return false;
    ret = static_cast<int>(r);
    return true;
}

inline bool ExtractStringField(const web::json::value& js, const utility::string_t& field,
                               ::std::string& ret) {
    if (!js.has_string_field(field)) return false;
    ret = _TS(js.at(field).as_string());
    return true;
}

inline bool ExtractDoubleField(const web::json::value& js, const utility::string_t& field,
                               double& ret) {
    if (!js.has_number_field(field)) return false;
    ret = js.at(field).as_double();
    return true;
}

inline bool ExtractBoolField(const web::json::value& js, const utility::string_t& field,
                             bool& ret) {
    if (!js.has_boolean_field(field)) return false;
    ret = js.at(field).as_bool();
    return true;
}

template <typename T>
inline bool JsonToType(const web::json::value& js, T& d) {
    static_assert(fma_common::_user_friendly_static_assert_<false, T>::value,
                  "bool JsonToType(const web::json::value& js, T& d) not defined.");
    return false;
}

template <typename T>
inline bool JsonToType(const web::json::value& js, std::map<std::string, T>& d) {
    for (auto& kv : js.as_object()) {
        T v;
        if (!JsonToType(kv.second, v)) return false;
        d.emplace_hint(d.end(), _TS(kv.first), std::move(v));
    }
    return true;
}

template <typename T>
inline bool JsonToType(const web::json::value& js, std::unordered_map<std::string, T>& d) {
    for (auto& kv : js.as_object()) {
        T v;
        if (!JsonToType(kv.second, v)) return false;
        d.emplace_hint(d.end(), _TS(kv.first), std::move(v));
    }
    return true;
}

template <typename T>
inline bool JsonToType(const web::json::value& js, std::set<T>& d) {
    for (auto& jv : js.as_array()) {
        T v;
        if (!JsonToType(jv, v)) return false;
        d.emplace_hint(d.end(), v);
    }
    return true;
}

template <typename T>
inline bool JsonToType(const web::json::value& js, std::unordered_set<T>& d) {
    for (auto& jv : js.as_array()) {
        T v;
        if (!JsonToType(jv, v)) return false;
        d.emplace_hint(d.end(), v);
    }
    return true;
}

template <typename T>
inline bool JsonToType(const web::json::value& js, std::vector<T>& d) {
    for (auto& jv : js.as_array()) {
        T v;
        if (!JsonToType(jv, v)) return false;
        d.emplace_back(v);
    }
    return true;
}

template <typename T>
inline bool ExtractTypedField(const web::json::value& js, const utility::string_t& key, T& d) {
    if (!js.has_field(key)) return false;
    return JsonToType(js.at(key), d);
}

template <>
inline bool ExtractTypedField<int64_t>(const web::json::value& js, const utility::string_t& key,
                                       int64_t& d) {
    return ExtractIntField(js, key, d);
}

template <>
inline bool ExtractTypedField<size_t>(const web::json::value& js, const utility::string_t& key,
                                      size_t& d) {
    int64_t i;
    if (!ExtractIntField(js, key, i)) return false;
    if (i < 0) return false;
    d = (size_t)i;
    return true;
}

template <>
inline bool ExtractTypedField<double>(const web::json::value& js, const utility::string_t& key,
                                      double& d) {
    return ExtractDoubleField(js, key, d);
}

template <>
inline bool ExtractTypedField<::std::string>(const web::json::value& js,
                                             const utility::string_t& key, ::std::string& d) {
    return ExtractStringField(js, key, d);
}

template <>
inline bool ExtractTypedField<bool>(const web::json::value& js, const utility::string_t& key,
                                    bool& d) {
    return ExtractBoolField(js, key, d);
}

template <>
inline bool JsonToType<int64_t>(const web::json::value& js, int64_t& d) {
    if (!js.is_integer()) return false;
    d = js.as_number().to_int64();
    return true;
}

template <>
inline bool JsonToType<int>(const web::json::value& js, int& d) {
    if (!js.is_integer()) return false;
    d = js.as_number().to_int32();
    return true;
}

template <>
inline bool JsonToType<double>(const web::json::value& js, double& d) {
    if (!js.is_double()) return false;
    d = js.as_double();
    return true;
}

template <>
inline bool JsonToType<bool>(const web::json::value& js, bool& d) {
    if (!js.is_array()) return false;
    d = js.as_bool();
    return true;
}

template <>
inline bool JsonToType<::std::string>(const web::json::value& js, ::std::string& d) {
    if (!js.is_string()) return false;
    d = _TS(js.as_string());
    return true;
}

template <typename T>
struct _ValueToJson {
 public:
    static inline web::json::value Convert(const T& d) { return web::json::value(d); }
};

template <typename T>
web::json::value ValueToJson(const T& v) {
    return _ValueToJson<T>::Convert(v);
}

inline web::json::value ValueToJson(const ::std::string& s) { return web::json::value(_TU(s)); }

inline web::json::value ValueToJson(const char* s) { return web::json::value(_TU(s)); }

inline web::json::value ValueToJson(const FieldData& fd) {
    switch (fd.type) {
    case FieldType::NUL:
        return web::json::value::null();
    case FieldType::BOOL:
        return web::json::value::boolean(fd.data.boolean);
    case FieldType::INT8:
        return web::json::value::number(fd.data.int8);
    case FieldType::INT16:
        return web::json::value::number(fd.data.int16);
    case FieldType::INT32:
        return web::json::value::number(fd.data.int32);
    case FieldType::INT64:
        return web::json::value::number(fd.data.int64);
    case FieldType::FLOAT:
        return web::json::value::number(fd.data.sp);
    case FieldType::DOUBLE:
        return web::json::value::number(fd.data.dp);
    case FieldType::DATE:
        return web::json::value::string(_TU(Date(fd.data.int32).ToString()));
    case FieldType::DATETIME:
        return web::json::value::string(_TU(DateTime(fd.data.int64).ToString()));
    case FieldType::STRING:
        return web::json::value::string(_TU(*fd.data.buf));
    case FieldType::BLOB:
        return web::json::value::string(_TU(::lgraph_api::base64::Encode(*fd.data.buf)));
    }
    FMA_DBG_ASSERT(false);  // unhandled FieldData type
    return web::json::value::null();
}

inline web::json::value ValueToJson(const TaskTracker::Stats& stats) {
    web::json::value ret;
    ret[_TU("requests/second")] = web::json::value::number(stats.qps);
    ret[_TU("writes/second")] = web::json::value::number(stats.tps);
    ret[_TU("failure_rate")] = web::json::value::number(stats.failure_rate);
    ret[_TU("running_tasks")] = web::json::value::number(stats.n_running);
    return ret;
}

inline web::json::value ValueToJson(const fma_common::HardwareInfo::CPURate& cpuRate) {
    web::json::value js_cpu;
    js_cpu[_TU("self")] = web::json::value::number((size_t)cpuRate.selfCPURate);
    js_cpu[_TU("server")] = web::json::value::number((size_t)cpuRate.serverCPURate);
    js_cpu[_TU("unit")] = web::json::value::string(_TU("%"));
    return js_cpu;
}

inline web::json::value ValueToJson(const fma_common::HardwareInfo::DiskRate& diskRate) {
    web::json::value js_disk;
    js_disk[_TU("read")] = web::json::value::number((size_t)diskRate.readRate);
    js_disk[_TU("write")] = web::json::value::number((size_t)diskRate.writeRate);
    js_disk[_TU("unit")] = web::json::value::string(_TU("B/s"));
    return js_disk;
}

inline web::json::value ValueToJson(const fma_common::HardwareInfo::MemoryInfo& memoryInfo) {
    web::json::value js_mem;
    js_mem[_TU("self")] = web::json::value::number((uint64_t)(memoryInfo.selfMemory));
    js_mem[_TU("available")] = web::json::value::number((uint64_t)(memoryInfo.available));
    js_mem[_TU("total")] = web::json::value::number((uint64_t)(memoryInfo.total));
    js_mem[_TU("unit")] = web::json::value::string(_TU("KB"));
    return js_mem;
}

inline web::json::value ValueToJson(const fma_common::DiskInfo& diskInfo, size_t graph_used) {
    web::json::value js_space;
    js_space[_TU("total")] = web::json::value::number((uint64_t)diskInfo.total);
    js_space[_TU("available")] = web::json::value::number((uint64_t)diskInfo.avail);
    js_space[_TU("self")] = web::json::value::number(graph_used);
    js_space[_TU("unit")] = web::json::value::string(_TU("B"));
    return js_space;
}

inline web::json::value ValueToJson(const std::pair<std::string, std::string>& pairs) {
    auto arr = web::json::value::array();
    arr[0] = web::json::value::string(_TU(pairs.first));
    arr[1] = web::json::value::string(_TU(pairs.second));
    return arr;
}

inline web::json::value ValueToJson(const DBConfig& conf) {
    web::json::value ret;
    ret[RestStrings::MAX_SIZE_GB] = conf.db_size / 1024 / 1024 / 1024;
    ret[RestStrings::DESC] = ValueToJson(conf.desc);
    return ret;
}

inline web::json::value ValueToJson(const StateMachine::Peer& peer) {
    web::json::value v;
    v[RestStrings::RPC_ADDR] = ValueToJson(peer.rpc_addr);
    v[RestStrings::REST_ADDR] = ValueToJson(peer.rest_addr);
    v[RestStrings::STATE] = ValueToJson(peer.StateString());
    return v;
}

template <>
inline bool JsonToType<DBConfig>(const web::json::value& js, DBConfig& conf) {
    int64_t size_gb = 0;
    if (!ExtractTypedField(js, RestStrings::MAX_SIZE_GB, size_gb))
        return false;
    if (size_gb <= 0 || size_gb > 1 << 20) return false;
    std::string desc;
    ExtractTypedField(js, RestStrings::DESC, desc);
    conf.db_size = (size_t)size_gb << 30;
    conf.desc = desc;
    conf.durable = false;
    return true;
}

inline web::json::value ValueToJson(const FieldSpec& fs) {
    web::json::value v;
    v[RestStrings::NAME] = ValueToJson(fs.name);
    v[RestStrings::NULLABLE] = web::json::value::boolean(fs.optional);
    v[RestStrings::TYPE] = ValueToJson(field_data_helper::FieldTypeName(fs.type));
    return v;
}

inline web::json::value ValueToJson(const GlobalConfig& fs) {
    web::json::value v;
    for (auto& kv : fs.ToFieldDataMap()) v[_TU(kv.first)] = ValueToJson(kv.second);
    return v;
}

inline web::json::value ValueToJson(const IndexSpec& is) {
    web::json::value v;
    v[RestStrings::LABEL] = ValueToJson(is.label);
    v[RestStrings::FIELD] = ValueToJson(is.field);
    v[RestStrings::ISUNIQUE] = ValueToJson(is.unique);
    return v;
}

inline web::json::value ValueToJson(const lgraph::AuditLog& log) {
    web::json::value ret;
    ret[_TU("index")] = ValueToJson(log.index);
    ret[_TU("begin_time")] = ValueToJson(log.begin_time);
    ret[_TU("end_time")] = ValueToJson(log.end_time);
    ret[_TU("user")] = ValueToJson(log.user);
    ret[_TU("graph")] = ValueToJson(log.graph);
    ret[_TU("type")] = ValueToJson(log.type);
    ret[_TU("read_write")] = ValueToJson(log.read_write);
    ret[_TU("success")] = ValueToJson(log.success);
    ret[_TU("content")] = ValueToJson(log.content);
    return ret;
}

inline web::json::value ValueToJson(const std::vector<std::pair<std::string, std::string>>& vec) {
    auto arr = web::json::value::array();
    for (int idx = 0; idx < (int)vec.size(); ++idx) {
        auto inner = web::json::value::array();
        auto pair = vec[idx];
        inner[0] = ValueToJson(pair.first);
        inner[1] = ValueToJson(pair.second);
        arr[idx] = inner;
    }
    return arr;
}

inline web::json::value ValueToJson(const std::vector<lgraph::_detail::FieldExtractor>& fields) {
    auto arr = web::json::value::array();
    for (int idx = 0; idx < (int)fields.size(); ++idx) {
        web::json::value js;
        js[_TU("name")] = ValueToJson(fields[idx].GetFieldSpec().name);
        js[_TU("type")] = ValueToJson(to_string(fields[idx].GetFieldSpec().type));
        js[_TU("optional")] = ValueToJson(fields[idx].GetFieldSpec().optional);
        if (fields[idx].GetVertexIndex()) {
            js[_TU("index")] = ValueToJson(true);
            if (fields[idx].GetVertexIndex()->IsUnique()) {
                js[_TU("unique")] = ValueToJson(true);
            } else {
                js[_TU("unique")] = ValueToJson(false);
            }
        } else {
            js[_TU("index")] = ValueToJson(false);
        }
        arr[idx] = js;
    }
    return arr;
}

inline web::json::value ValueToJson(const lgraph::Schema* schema) {
    web::json::value js;
    js[_TU("label")] = ValueToJson(schema->GetLabel());
    js[_TU("properties")] = ValueToJson(schema->GetFields());
    if (schema->IsVertex()) {
        js[_TU("type")] = ValueToJson("VERTEX");
        js[_TU("primary")] = ValueToJson(schema->GetPrimaryField());
    } else {
        js[_TU("type")] = ValueToJson("EDGE");
        js[_TU("constraints")] = ValueToJson(schema->GetEdgeConstraints());
    }
    return js;
}

inline web::json::value ValueToJson(const EdgeUid& euid) {
    return web::json::value(_TU(fma_common::StringFormatter::Format("{}", euid.ToString())));
}

inline web::json::value ValueToJson(const TaskTracker::TaskDesc& task) {
    web::json::value v;
    v[RestStrings::DESC] = ValueToJson(task.desc);
    v[RestStrings::TASK_ID] = web::json::value::string(_TU(task.id.ToString()));
    v[RestStrings::TIME_USED] = task.time_elpased;
    return v;
}

inline web::json::value ValueToJson(const AccessLevel& ac) {
    switch (ac) {
    case AccessLevel::NONE:
        return web::json::value(_TU("NONE"));
    case AccessLevel::READ:
        return web::json::value(_TU("READ"));
    case AccessLevel::WRITE:
        return web::json::value(_TU("WRITE"));
    case AccessLevel::FULL:
        return web::json::value(_TU("FULL"));
    default:
        FMA_ASSERT(false) << "Unrecognized AccessLevel value: " << (int)ac;
        return web::json::value();
    }
}

inline web::json::value ValueToJson(const FieldAccessLevel& ac) {
    return web::json::value(_TU(lgraph_api::to_string(ac)));
}

inline web::json::value ValueToJson(const AclManager::UserInfo& info) {
    web::json::value js;
    js[RestStrings::DISABLED] = web::json::value::boolean(info.disabled);
    js[RestStrings::ROLES] = ValueToJson(info.roles);
    js[RestStrings::AUTH_METHOD] = ValueToJson(info.auth_method);
    js[RestStrings::DESC] = web::json::value(_TU(info.desc));
    js[RestStrings::MEM_LIMIT] = web::json::value(info.memory_limit);
    return js;
}

inline web::json::value ValueToJson(const lgraph::PluginDesc& desc) {
    web::json::value js;
    js[RestStrings::NAME] = web::json::value::string(_TU(desc.name));
    js[RestStrings::DESC] = web::json::value::string(_TU(desc.desc));
    js[RestStrings::READONLY] = web::json::value::boolean(desc.read_only);
    return js;
}

inline web::json::value ValueToJson(const PluginCode& co) {
    web::json::value v;
    v[RestStrings::NAME] = ValueToJson(co.name);
    v[RestStrings::DESC] = ValueToJson(co.desc);
    v[RestStrings::READONLY] = ValueToJson(co.read_only);
    v[RestStrings::CODE] = ValueToJson(co.code);
    v[RestStrings::CODE_TYPE] = ValueToJson(co.code_type);
    return v;
}

inline bool JsonToType(const web::json::value& js, AclManager::UserInfo& d) {
    if (!ExtractBoolField(js, RestStrings::DISABLED, d.disabled) ||
        ExtractStringField(js, RestStrings::AUTH_METHOD, d.auth_method))
        return false;
    ExtractStringField(js, RestStrings::DESC, d.desc);
    if (!js.has_array_field(RestStrings::ROLES)) return false;
    return JsonToType(js.at(RestStrings::ROLES), d.roles);
}

template <>
inline bool JsonToType<::lgraph_api::UserInfo>(const web::json::value& js,
                                               ::lgraph_api::UserInfo& d) {
    d.disabled = false;
    ExtractBoolField(js, RestStrings::DISABLED, d.disabled);
    ExtractStringField(js, RestStrings::DESC, d.desc);
    d.memory_limit = js.at(RestStrings::MEM_LIMIT).as_number().to_int64();
    if (!js.has_array_field(RestStrings::ROLES)) return false;
    return JsonToType(js.at(RestStrings::ROLES), d.roles);
}

inline web::json::value ValueToJson(const AclManager::FieldAccess& field_access) {
    std::vector<web::json::value> jsv;
    for (auto& kv : field_access) {
        std::string label_type;
        if (kv.first.is_vertex)
            label_type = "VERTEX";
        else
            label_type = "EDGE";
        web::json::value js;
        js[_TU("LABEL_TYPE")] = ValueToJson(label_type);
        js[_TU("LABEL")] = ValueToJson(kv.first.label);
        js[_TU("FIELD")] = ValueToJson(kv.first.field);
        js[_TU("FIELD_ACCESS_LEVEL")] = ValueToJson(kv.second);
        jsv.emplace_back(js);
    }
    return ValueToJson(jsv);
}

inline web::json::value ValueToJson(const AclManager::RoleInfo& info) {
    web::json::value js;
    js[RestStrings::DISABLED] = web::json::value::boolean(info.disabled);
    js[RestStrings::DESC] = web::json::value(_TU(info.desc));
    js[RestStrings::PERMISSIONS] = ValueToJson(info.graph_access);
    js[RestStrings::FIELD_PERMISSIONS] = ValueToJson(info.field_access);
    return js;
}

template <>
inline bool JsonToType<lgraph_api::RoleInfo>(const web::json::value& js,
                                             ::lgraph_api::RoleInfo& info) {
    info.disabled = false;
    ExtractBoolField(js, RestStrings::DISABLED, info.disabled);
    ExtractStringField(js, RestStrings::DESC, info.desc);
    if (!js.has_object_field(RestStrings::PERMISSIONS)) return false;
    return JsonToType(js.at(RestStrings::PERMISSIONS), info.graph_access);
}

template <>
inline bool JsonToType<AccessLevel>(const web::json::value& js, AccessLevel& ar) {
    if (!js.is_string()) return false;
    const std::string& str = _TS(js.as_string());
    if (str == "FULL") {
        ar = AccessLevel::FULL;
        return true;
    }
    if (str == "WRITE") {
        ar = AccessLevel::WRITE;
        return true;
    }
    if (str == "READ") {
        ar = AccessLevel::READ;
        return true;
    }
    if (str == "NONE") {
        ar = AccessLevel::NONE;
        return true;
    }
    return false;
}

template <typename T>
struct _ValueToJson<::std::unordered_set<T>> {
    static inline web::json::value Convert(const ::std::unordered_set<T>& s) {
        ::std::vector<web::json::value> vs;
        vs.reserve(s.size());
        for (auto& v : s) vs.emplace_back(ValueToJson(v));
        return web::json::value::array(std::move(vs));
    }
};

template <typename V>
struct _ValueToJson<::std::map<::std::string, V>> {
    static inline web::json::value Convert(const ::std::map<::std::string, V>& m) {
        web::json::value v;
        for (auto& kv : m) v[_TU(kv.first)] = ValueToJson(kv.second);
        return v;
    }
};

template <typename V>
struct _ValueToJson<::std::unordered_map<::std::string, V>> {
    static inline web::json::value Convert(const ::std::unordered_map<::std::string, V>& m) {
        web::json::value v;
        for (auto& kv : m) v[_TU(kv.first)] = ValueToJson(kv.second);
        return v;
    }
};

template <typename V>
struct _ValueToJson<::std::vector<V>> {
    static inline web::json::value Convert(const std::vector<V>& v) {
        ::std::vector<web::json::value> jsv;
        jsv.reserve(v.size());
        for (auto& d : v) {
            jsv.emplace_back(ValueToJson(d));
        }
        return web::json::value::array(jsv);
    }
};

template <typename T>
inline web::json::value VectorToJson(const ::std::vector<T>& v) {
    ::std::vector<web::json::value> jsv;
    jsv.reserve(v.size());
    for (auto& d : v) {
        jsv.emplace_back(ValueToJson(d));
    }
    return web::json::value::array(jsv);
}

template <typename... Ts>
inline web::json::value ValueToJson(const Ts&... ds) {
    ::std::vector<web::json::value> arr = {ValueToJson(ds)...};
    return web::json::value::array(::std::move(arr));
}

template <typename... Ts>
inline web::json::value ValueToJsonArray(const Ts&... ds) {
    ::std::vector<web::json::value> arr = {ValueToJson(ds)...};
    return web::json::value::array(::std::move(arr));
}

template <>
inline bool JsonToType<FieldSpec>(const web::json::value& js, FieldSpec& fs) {
    if (!ExtractStringField(js, RestStrings::NAME, fs.name)) return false;
    if (!js.has_string_field(RestStrings::TYPE)) return false;
    if (!field_data_helper::TryGetFieldType(_TS(js.at(RestStrings::TYPE).as_string()), fs.type))
        return false;
    if (!ExtractBoolField(js, RestStrings::NULLABLE, fs.optional)) return false;
    return true;
}

template <>
inline bool JsonToType<IndexSpec>(const web::json::value& js, IndexSpec& i) {
    if (!ExtractStringField(js, RestStrings::LABEL, i.label)) return false;
    if (!ExtractStringField(js, RestStrings::FIELD, i.field)) return false;
    if (!ExtractBoolField(js, RestStrings::ISUNIQUE, i.unique)) return false;
    return true;
}

template <>
inline bool JsonToType<FieldData>(const web::json::value& js, FieldData& fd) {
    bool ret = true;
    if (js.is_null())
        fd = FieldData();
    else if (js.is_boolean())
        fd = FieldData(js.as_bool());
    else if (js.is_integer())
        fd = FieldData(js.as_number().to_int64());
    else if (js.is_double())
        fd = FieldData(js.as_double());
    else if (js.is_string())
        fd = FieldData(_TS(js.as_string()));
    else
        ret = false;
    return ret;
}

template <>
inline bool JsonToType<PluginDesc>(const web::json::value& js, PluginDesc& pd) {
    if (!ExtractStringField(js, RestStrings::NAME, pd.name)) return false;
    if (!ExtractStringField(js, RestStrings::DESC, pd.desc)) return false;
    if (!ExtractBoolField(js, RestStrings::READONLY, pd.read_only)) return false;
    return true;
}

template <>
inline bool JsonToType<PluginCode>(const web::json::value& js, PluginCode& pc) {
    if (!ExtractStringField(js, RestStrings::NAME, pc.name)) return false;
    if (!ExtractStringField(js, RestStrings::DESC, pc.desc)) return false;
    if (!ExtractBoolField(js, RestStrings::READONLY, pc.read_only)) return false;
    if (!ExtractStringField(js, RestStrings::CODE, pc.code)) return false;
    if (!ExtractStringField(js, RestStrings::CODE_TYPE, pc.code_type)) return false;
    return true;
}

template <>
inline bool JsonToType<std::map<std::string, FieldData>>(const web::json::value& js,
                                                         std::map<std::string, FieldData>& dict) {
    dict.clear();
    for (auto& kv : js.as_object()) {
        FieldData& d = dict[_TS(kv.first)];
        if (!JsonToType<FieldData>(kv.second, d)) return false;
    }
    return true;
}
}  // namespace lgraph
