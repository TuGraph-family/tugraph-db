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

#include "lgraph/c.h"
#include <sys/types.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <map>
#include <vector>
#include <iostream>

#include "lgraph/lgraph_date_time.h"
#include "lgraph/lgraph_db.h"
#include "lgraph/lgraph_edge_index_iterator.h"
#include "lgraph/lgraph_edge_iterator.h"
#include "lgraph/lgraph_galaxy.h"
#include "lgraph/lgraph_txn.h"
#include "lgraph/lgraph_types.h"
#include "lgraph/lgraph_vertex_index_iterator.h"
#include "lgraph/lgraph_vertex_iterator.h"

using namespace lgraph_api;

extern "C" {

struct lgraph_api_date_t {
    Date repr;
};

struct lgraph_api_date_time_t {
    DateTime repr;
};

// binding types in lgraph_types.h
struct lgraph_api_field_data_t {
    FieldData repr;
};

struct lgraph_api_field_spec_t {
    FieldSpec repr;
};

struct lgraph_api_index_spec_t {
    IndexSpec repr;
};

struct lgraph_api_edge_uid_t {
    EdgeUid repr;
};

struct lgraph_api_user_info_t {
    UserInfo repr;
};

struct lgraph_api_role_info_t {
    RoleInfo repr;
};

struct lgraph_api_in_edge_iterator_t {
    InEdgeIterator repr;
};

struct lgraph_api_out_edge_iterator_t {
    OutEdgeIterator repr;
};

struct lgraph_api_vertex_iterator_t {
    VertexIterator repr;
};

struct lgraph_api_vertex_index_iterator_t {
    VertexIndexIterator repr;
};

struct lgraph_api_edge_index_iterator_t {
    EdgeIndexIterator repr;
};

struct lgraph_api_transaction_t {
    Transaction repr;
};

struct lgraph_api_graph_db_t {
    GraphDB repr;
};

struct lgraph_api_galaxy_t {
    Galaxy repr;
};
}

/* DateTime(not exhausted)*/
lgraph_api_date_time_t* lgraph_api_create_date_time() {
    return new lgraph_api_date_time_t{DateTime()};
}
lgraph_api_date_time_t* lgraph_api_create_date_time_ymdhms(int year, unsigned month, unsigned day,
                                                           unsigned hour, unsigned minute,
                                                           unsigned second) {
    return new lgraph_api_date_time_t{
        DateTime(DateTime::YMDHMS{year, month, day, hour, minute, second})};
}
lgraph_api_date_time_t* lgraph_api_create_date_time_seconds(int64_t seconds_since_epoch) {
    return new lgraph_api_date_time_t{DateTime(seconds_since_epoch)};
}
int64_t lgraph_api_date_time_seconds_since_epoch(lgraph_api_date_time_t* dt) {
    return dt->repr.SecondsSinceEpoch();
}
void lgraph_api_date_time_destroy(lgraph_api_date_time_t* dt) { delete dt; }

/* Date(not exhausted) */
lgraph_api_date_t* lgraph_api_create_date() { return new lgraph_api_date_t{Date()}; }
lgraph_api_date_t* lgraph_api_create_date_ymd(int year, unsigned month, unsigned day) {
    return new lgraph_api_date_t{Date(Date::YearMonthDay{year, month, day})};
}
int32_t lgraph_api_date_days_since_epoch(lgraph_api_date_t* date) {
    return date->repr.DaysSinceEpoch();
}
lgraph_api_date_t* lgraph_api_create_date_days(int32_t day) {
    return new lgraph_api_date_t{Date(day)};
}
void lgraph_api_date_destroy(lgraph_api_date_t* dt) { delete dt; }

lgraph_api_field_data_t* lgraph_api_create_field_data() {
    return new lgraph_api_field_data_t{FieldData()};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_bool(bool v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_int8(int8_t v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_int16(int16_t v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_int32(int32_t v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_int64(int64_t v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_float(float v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_double(double v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_date(lgraph_api_date_t* v) {
    return new lgraph_api_field_data_t{FieldData(v->repr)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_date_time(lgraph_api_date_time_t* v) {
    return new lgraph_api_field_data_t{FieldData(v->repr)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_str(const char* v) {
    return new lgraph_api_field_data_t{FieldData(v)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_str_len(const char* v, size_t len) {
    return new lgraph_api_field_data_t{FieldData(v, len)};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_blob(const uint8_t* v, size_t len) {
    return new lgraph_api_field_data_t{FieldData::Blob(std::vector<uint8_t>(v, v + len))};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_base64_blob(const uint8_t* v, size_t len) {
    return new lgraph_api_field_data_t{FieldData::BlobFromBase64(std::string(v, v + len))};
}
lgraph_api_field_data_t* lgraph_api_create_field_data_clone(lgraph_api_field_data_t* fd) {
    return new lgraph_api_field_data_t{FieldData(fd->repr)};
}
void lgraph_api_create_field_data_clone_from(lgraph_api_field_data_t* fd,
                                             lgraph_api_field_data_t* other_fd) {
    fd->repr = other_fd->repr;
}
void lgraph_api_field_data_destroy(lgraph_api_field_data_t* fd) { delete fd; }
int64_t lgraph_api_field_data_integer(lgraph_api_field_data_t* fd) { return fd->repr.integer(); }
double lgraph_api_field_data_real(lgraph_api_field_data_t* fd) { return fd->repr.real(); }
const char* lgraph_api_field_data_str(lgraph_api_field_data_t* fd) {
    return strdup(fd->repr.string().c_str());
}
bool lgraph_api_field_data_as_bool(lgraph_api_field_data_t* fd) { return fd->repr.AsBool(); }
int8_t lgraph_api_field_data_as_int8(lgraph_api_field_data_t* fd) { return fd->repr.AsInt8(); }
int16_t lgraph_api_field_data_as_int16(lgraph_api_field_data_t* fd) { return fd->repr.AsInt16(); }
int32_t lgraph_api_field_data_as_int32(lgraph_api_field_data_t* fd) { return fd->repr.AsInt32(); }
int64_t lgraph_api_field_data_as_int64(lgraph_api_field_data_t* fd) { return fd->repr.AsInt64(); }
float lgraph_api_field_data_as_float(lgraph_api_field_data_t* fd) { return fd->repr.AsFloat(); }
double lgraph_api_field_data_as_double(lgraph_api_field_data_t* fd) { return fd->repr.AsDouble(); }
lgraph_api_date_t* lgraph_api_field_data_as_date(lgraph_api_field_data_t* fd) {
    return new lgraph_api_date_t{fd->repr.AsDate()};
}
lgraph_api_date_time_t* lgraph_api_field_data_as_date_time(lgraph_api_field_data_t* fd) {
    return new lgraph_api_date_time_t{fd->repr.AsDateTime()};
}
char* lgraph_api_field_data_as_str(lgraph_api_field_data_t* fd) {
    return strdup(fd->repr.AsString().c_str());
}
char* lgraph_api_field_data_as_blob(lgraph_api_field_data_t* fd) {
    return strdup(fd->repr.AsBlob().c_str());
}
char* lgrpah_api_field_data_as_blob_base64(lgraph_api_field_data_t* fd) {
    return strdup(fd->repr.AsBase64Blob().c_str());
}
char* lgraph_api_field_data_to_string(lgraph_api_field_data_t* fd) {
    return strdup(fd->repr.ToString().c_str());
}
bool lgraph_api_field_data_eq(const lgraph_api_field_data_t* fd,
                              const lgraph_api_field_data_t* other_fd) {
    return fd->repr == other_fd->repr;
}
bool lgraph_api_field_data_not_eq(const lgraph_api_field_data_t* fd,
                                  const lgraph_api_field_data_t* other_fd) {
    return fd->repr != other_fd->repr;
}
bool lgraph_api_field_data_greater_than(const lgraph_api_field_data_t* fd,
                                        const lgraph_api_field_data_t* other_fd) {
    return fd->repr > other_fd->repr;
}
bool lgraph_api_field_data_greater_eq_than(const lgraph_api_field_data_t* fd,
                                           const lgraph_api_field_data_t* other_fd) {
    return fd->repr >= other_fd->repr;
}
bool lgraph_api_field_data_less_than(const lgraph_api_field_data_t* fd,
                                     const lgraph_api_field_data_t* other_fd) {
    return fd->repr < other_fd->repr;
}
bool lgraph_api_field_data_less_eq_than(const lgraph_api_field_data_t* fd,
                                        const lgraph_api_field_data_t* other_fd) {
    return fd->repr <= other_fd->repr;
}
int lgraph_api_field_data_get_type(lgraph_api_field_data_t* fd) { return fd->repr.GetType(); }
bool lgraph_api_field_data_is_null(lgraph_api_field_data_t* fd) { return fd->repr.IsNull(); }
bool lgraph_api_field_data_is_buf(lgraph_api_field_data_t* fd) { return fd->repr.is_buf(); }
bool lgraph_api_field_data_is_empty_buf(lgraph_api_field_data_t* fd) {
    return fd->repr.is_empty_buf();
}
bool lgraph_api_field_data_is_bool(lgraph_api_field_data_t* fd) { return fd->repr.IsBool(); }
bool lgraph_api_field_data_is_blob(lgraph_api_field_data_t* fd) { return fd->repr.IsBlob(); }
bool lgraph_api_field_data_is_string(lgraph_api_field_data_t* fd) { return fd->repr.IsString(); }
bool lgraph_api_field_data_is_int8(lgraph_api_field_data_t* fd) { return fd->repr.IsInt8(); }
bool lgraph_api_field_data_is_int16(lgraph_api_field_data_t* fd) { return fd->repr.IsInt16(); }
bool lgraph_api_field_data_is_int32(lgraph_api_field_data_t* fd) { return fd->repr.IsInt32(); }
bool lgraph_api_field_data_is_int64(lgraph_api_field_data_t* fd) { return fd->repr.IsInt64(); }
bool lgraph_api_field_data_is_integer(lgraph_api_field_data_t* fd) { return fd->repr.IsInteger(); }
bool lgraph_api_field_data_is_float(lgraph_api_field_data_t* fd) { return fd->repr.IsFloat(); }
bool lgraph_api_field_data_is_double(lgraph_api_field_data_t* fd) { return fd->repr.IsDouble(); }
bool lgraph_api_field_data_is_real(lgraph_api_field_data_t* fd) { return fd->repr.IsReal(); }
bool lgraph_api_field_data_is_date(lgraph_api_field_data_t* fd) { return fd->repr.IsDate(); }
bool lgraph_api_field_data_is_date_time(lgraph_api_field_data_t* fd) {
    return fd->repr.IsDateTime();
}

lgraph_api_field_spec_t* lgraph_api_create_field_spec() {
    return new lgraph_api_field_spec_t{FieldSpec()};
}
lgraph_api_field_spec_t* lgraph_api_create_field_spec_name_type_optional(const char* name, int type,
                                                                         bool optional) {
    return new lgraph_api_field_spec_t{FieldSpec(name, static_cast<FieldType>(type), optional)};
}
void lgraph_api_field_spec_destroy(lgraph_api_field_spec_t* fs) { delete fs; }
bool lgraph_api_field_spec_eq(const lgraph_api_field_spec_t* fs,
                              const lgraph_api_field_spec_t* other_fs) {
    return fs->repr == other_fs->repr;
}
char* lgraph_api_field_spec_to_string(lgraph_api_field_spec_t* fs) {
    return strdup(fs->repr.ToString().c_str());
}
const char* lgraph_api_field_spec_get_name(lgraph_api_field_spec_t* fs) {
    return strdup(fs->repr.name.c_str());
}
void lgraph_api_field_spec_set_name(lgraph_api_field_spec_t* fs, const char* name) {
    fs->repr.name = name;
}
int lgraph_api_field_spec_get_type(lgraph_api_field_spec_t* fs) { return fs->repr.type; }
void lgraph_api_field_spec_set_type(lgraph_api_field_spec_t* fs, int type) {
    fs->repr.type = static_cast<FieldType>(type);
}
bool lgraph_api_field_spec_get_optional(lgraph_api_field_spec_t* fs) { return fs->repr.optional; }
void lgraph_api_field_spec_set_optional(lgraph_api_field_spec_t* fs, bool optional) {
    fs->repr.optional = optional;
}

lgraph_api_index_spec_t* lgraph_api_create_index_spec() {
    return new lgraph_api_index_spec_t{IndexSpec()};
}
void lgraph_api_index_spec_destroy(lgraph_api_index_spec_t* is) { delete is; }
const char* lgraph_api_index_spec_get_label(lgraph_api_index_spec_t* is) {
    return strdup(is->repr.label.c_str());
}
const char* lgraph_api_index_spec_get_field(lgraph_api_index_spec_t* is) {
    return strdup(is->repr.field.c_str());
}
bool lgraph_api_index_spec_get_unique(lgraph_api_index_spec_t* is) { return is->repr.unique; }
void lgraph_api_index_spec_set_label(lgraph_api_index_spec_t* is, const char* label) {
    is->repr.label = label;
}
void lgraph_api_index_spec_set_field(lgraph_api_index_spec_t* is, const char* field) {
    is->repr.field = field;
}
void lgraph_api_index_spec_set_unique(lgraph_api_index_spec_t* is, bool unique) {
    is->repr.unique = unique;
}

lgraph_api_edge_uid_t* lgraph_api_create_edge_euid(int64_t src, int64_t dst, uint16_t lid,
                                                   int64_t tid, int64_t eid) {
    return new lgraph_api_edge_uid_t{EdgeUid(src, dst, lid, tid, eid)};
}
void lgraph_api_edge_euid_destroy(lgraph_api_edge_uid_t* euid) { delete euid; }
int64_t lgraph_api_edge_euid_get_src(lgraph_api_edge_uid_t* euid) { return euid->repr.src; }
int64_t lgraph_api_edge_euid_get_dst(lgraph_api_edge_uid_t* euid) { return euid->repr.dst; }
uint16_t lgraph_api_edge_euid_get_lid(lgraph_api_edge_uid_t* euid) { return euid->repr.lid; }
int64_t lgraph_api_edge_euid_get_tid(lgraph_api_edge_uid_t* euid) { return euid->repr.tid; }
int64_t lgraph_api_edge_euid_get_eid(lgraph_api_edge_uid_t* euid) { return euid->repr.eid; }
void lgraph_api_edge_euid_set_src(lgraph_api_edge_uid_t* euid, int64_t src) {
    euid->repr.src = src;
}
void lgraph_api_edge_euid_set_dst(lgraph_api_edge_uid_t* euid, int64_t dst) {
    euid->repr.dst = dst;
}
void lgraph_api_edge_euid_set_lid(lgraph_api_edge_uid_t* euid, uint16_t lid) {
    euid->repr.lid = lid;
}
void lgraph_api_edge_euid_set_tid(lgraph_api_edge_uid_t* euid, int64_t tid) {
    euid->repr.tid = tid;
}
void lgraph_api_edge_euid_set_eid(lgraph_api_edge_uid_t* euid, int64_t eid) {
    euid->repr.eid = eid;
}
void lgraph_api_edge_euid_reverse(lgraph_api_edge_uid_t* euid) { euid->repr.Reverse(); }
bool lgraph_api_edge_euid_eq(const lgraph_api_edge_uid_t* euid,
                             const lgraph_api_edge_uid_t* other_euid) {
    return euid->repr == other_euid->repr;
}
bool lgraph_api_edge_euid_out_less(const lgraph_api_edge_uid_t* euid,
                                   const lgraph_api_edge_uid_t* other_euid) {
    return EdgeUid::OutEdgeSortOrder()(euid->repr, other_euid->repr);
}
bool lgraph_api_edge_euid_in_less(const lgraph_api_edge_uid_t* euid,
                                  const lgraph_api_edge_uid_t* other_euid) {
    return EdgeUid::InEdgeSortOrder()(euid->repr, other_euid->repr);
}
char* lgraph_api_edge_euid_to_string(lgraph_api_edge_uid_t* euid) {
    return strdup(euid->repr.ToString().c_str());
}

lgraph_api_user_info_t* lgraph_api_create_user_info() {
    return new lgraph_api_user_info_t{UserInfo()};
}
void lgraph_api_user_info_destroy(lgraph_api_user_info_t* ui) { delete ui; }
const char* lgraph_api_user_info_get_desc(lgraph_api_user_info_t* ui) {
    return strdup(ui->repr.desc.c_str());
}
size_t lgraph_api_user_info_get_roles(lgraph_api_user_info_t* ui, char*** roles) {
    size_t n = ui->repr.roles.size();
    if (n != 0 && roles != nullptr) {
        *roles = new char*[n];
    }
    size_t i = 0;
    for (const auto& r : ui->repr.roles) {
        (*roles)[i] = strdup(r.c_str());
        i++;
    }
    return n;
}
void lgraph_api_user_info_destroy_roles(char** roles, size_t n) {
    for (size_t i = 0; i < n; i++) {
        delete roles[i];
    }
    delete[] roles;
}
bool lgraph_api_user_info_get_disable(lgraph_api_user_info_t* ui) { return ui->repr.disabled; }
size_t lgraph_api_user_info_get_memory_limit(lgraph_api_user_info_t* ui) {
    return ui->repr.memory_limit;
}
void lgraph_api_user_info_set_desc(lgraph_api_user_info_t* ui, const char* desc) {
    ui->repr.desc = desc;
}
void lgraph_api_user_info_add_role(lgraph_api_user_info_t* ui, const char* role) {
    ui->repr.roles.insert(role);
}
void lgraph_api_user_info_set_roles(lgraph_api_user_info_t* ui, const char** roles, size_t n) {
    std::set<std::string> new_roles;
    for (size_t i = 0; i < n; i++) {
        new_roles.insert(roles[i]);
    }
    ui->repr.roles = new_roles;
}
int lgraph_api_user_info_del_role(lgraph_api_user_info_t* ui, const char* role) {
    return ui->repr.roles.erase(role);
}
void lgraph_api_user_info_set_disable(lgraph_api_user_info_t* ui, bool disable) {
    ui->repr.disabled = disable;
}
void lgraph_api_user_info_set_memory_limit(lgraph_api_user_info_t* ui, size_t memory_limit) {
    ui->repr.memory_limit = memory_limit;
}

lgraph_api_role_info_t* lgraph_api_create_role_info() {
    return new lgraph_api_role_info_t{RoleInfo()};
}
void lgraph_api_role_info_destroy(lgraph_api_role_info_t* ri) { delete ri; }
const char* lgraph_api_role_info_get_desc(lgraph_api_role_info_t* ri) {
    return strdup(ri->repr.desc.c_str());
}
/* if graph_name not exist, return -1 */
int lgraph_api_role_info_get_access_level(lgraph_api_role_info_t* ri, const char* graph_name) {
    auto it = ri->repr.graph_access.find(graph_name);
    return it == ri->repr.graph_access.end() ? -1 : static_cast<int>(it->second);
}
size_t lgraph_api_role_info_get_graph_access(lgraph_api_role_info_t* ri, char*** graph_names,
                                             int** access_levels) {
    size_t n = ri->repr.graph_access.size();
    if (n != 0 && graph_names != nullptr && access_levels != nullptr) {
        *graph_names = new char*[n];
        *access_levels = new int[n];
    }

    size_t i = 0;
    for (const auto& kv : ri->repr.graph_access) {
        (*graph_names)[i] = strdup(kv.first.c_str());
        (*access_levels)[i] = static_cast<int>(kv.second);
        i++;
    }
    return n;
}
void lgraph_api_role_info_destroy_graph_access(char** graph_names, int* access_levels, size_t n) {
    for (size_t i = 0; i < n; i++) {
        delete graph_names[i];
    }
    delete[] graph_names;
    delete[] access_levels;
}
bool lgraph_api_role_info_get_disabled(lgraph_api_role_info_t* ri) { return ri->repr.disabled; }
void lgraph_api_role_info_set_desc(lgraph_api_role_info_t* ri, const char* desc) {
    ri->repr.desc = desc;
}
bool lgraph_api_role_info_add_access_level(lgraph_api_role_info_t* ri, const char* graph,
                                           int access_level) {
    auto result = ri->repr.graph_access.insert({graph, static_cast<AccessLevel>(access_level)});
    return result.second;
}
void lgraph_api_role_info_set_graph_access(lgraph_api_role_info_t* ri, char** graph_names,
                                           int* access_levels, size_t n) {
    std::map<std::string, AccessLevel> graph_access;
    for (size_t i = 0; i < n; i++) {
        graph_access.insert({graph_names[i], static_cast<AccessLevel>(access_levels[i])});
    }
    ri->repr.graph_access = graph_access;
}
/*return the number of deleted ones */
int lgraph_api_role_info_del_access_level(lgraph_api_role_info_t* ri, const char* graph_name) {
    return ri->repr.graph_access.erase(graph_name);
}
void lgraph_api_role_info_set_disabled(lgraph_api_role_info_t* ri, bool disabled) {
    ri->repr.disabled = disabled;
}

void lgraph_api_out_edge_iterator_destroy(lgraph_api_out_edge_iterator_t* it) { delete it; }

void lgraph_api_out_edge_iterator_close(lgraph_api_out_edge_iterator_t* it) { it->repr.Close(); }

bool lgraph_api_out_edge_iterator_goto(lgraph_api_out_edge_iterator_t* it,
                                       const lgraph_api_edge_uid_t* euid, bool nearest,
                                       char** errptr) {
    try {
        return it->repr.Goto(euid->repr, nearest);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_out_edge_iterator_is_valid(lgraph_api_out_edge_iterator_t* it) {
    return it->repr.IsValid();
}

bool lgraph_api_out_edge_iterator_next(lgraph_api_out_edge_iterator_t* it, char** errptr) {
    try {
        return it->repr.Next();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

lgraph_api_edge_uid_t* lgraph_api_out_edge_iterator_get_uid(lgraph_api_out_edge_iterator_t* it,
                                                            char** errptr) {
    try {
        return new lgraph_api_edge_uid_t{it->repr.GetUid()};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

int64_t lgraph_api_out_edge_iterator_get_dst(lgraph_api_out_edge_iterator_t* it, char** errptr) {
    try {
        return it->repr.GetDst();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

int64_t lgraph_api_out_edge_iterator_get_edge_id(lgraph_api_out_edge_iterator_t* it,
                                                 char** errptr) {
    try {
        return it->repr.GetEdgeId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

int64_t lgraph_api_out_edge_iterator_get_temporal_id(lgraph_api_out_edge_iterator_t* it,
                                                     char** errptr) {
    try {
        return it->repr.GetTemporalId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

int64_t lgraph_api_out_edge_iterator_get_src(lgraph_api_out_edge_iterator_t* it, char** errptr) {
    try {
        return it->repr.GetSrc();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

const char* lgraph_api_out_edge_iterator_get_label(lgraph_api_out_edge_iterator_t* it,
                                                   char** errptr) {
    try {
        return strdup(it->repr.GetLabel().c_str());
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

uint16_t lgraph_api_out_edge_iterator_get_label_id(lgraph_api_out_edge_iterator_t* it,
                                                   char** errptr) {
    try {
        return it->repr.GetLabelId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

size_t lgraph_api_out_edge_iterator_get_fields_by_names(lgraph_api_out_edge_iterator_t* it,
                                                        const char* const* field_names,
                                                        size_t field_names_len,
                                                        lgraph_api_field_data_t*** field_data,
                                                        size_t* field_data_len, char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldData> _field_data = it->repr.GetFields(
            std::vector<std::string>(field_names, field_names + field_names_len));
        n = _field_data.size();
        *field_data = new lgraph_api_field_data_t*[n];
        for (size_t i = 0; i < _field_data.size(); i++) {
            (*field_data)[i] = new lgraph_api_field_data_t{_field_data[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

lgraph_api_field_data_t* lgraph_api_out_edge_iterator_get_field_by_name(
    lgraph_api_out_edge_iterator_t* it, const char* field_name, char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetField(field_name)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_out_edge_iterator_get_fields_by_ids(lgraph_api_out_edge_iterator_t* it,
                                                      const size_t* field_ids, size_t field_ids_len,
                                                      lgraph_api_field_data_t*** field_data,
                                                      char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldData> _field_data =
            it->repr.GetFields(std::vector<size_t>(field_ids, field_ids + field_ids_len));
        n = _field_data.size();
        *field_data = new lgraph_api_field_data_t*[n];
        for (size_t i = 0; i < _field_data.size(); i++) {
            (*field_data)[i] = new lgraph_api_field_data_t{_field_data[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

lgraph_api_field_data_t* lgraph_api_out_edge_iterator_get_field_by_id(
    lgraph_api_out_edge_iterator_t* it, size_t field_id, char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetField(field_id)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_out_edge_iterator_get_all_fields(lgraph_api_out_edge_iterator_t* it,
                                                   char*** field_names,
                                                   lgraph_api_field_data_t*** field_data,
                                                   char** errptr) {
    size_t n = 0;
    try {
        std::map<std::string, FieldData> field_data_map = it->repr.GetAllFields();
        n = field_data_map.size();
        *field_names = new char*[n];
        *field_data = new lgraph_api_field_data_t*[n];
        size_t i = 0;
        for (auto const& [key, val] : field_data_map) {
            (*field_names)[i] = strdup(key.c_str());
            (*field_data)[i] = new lgraph_api_field_data_t{val};
            i++;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

void lgraph_api_field_names_destroy(char** field_names, size_t n) {
    for (size_t i = 0; i < n; i++) {
        delete field_names[i];
    }
    delete[] field_names;
}

void lgraph_api_out_edge_iterator_set_field_by_name(lgraph_api_out_edge_iterator_t* it,
                                                    const char* field_name,
                                                    const lgraph_api_field_data_t* field_value,
                                                    char** errptr) {
    try {
        it->repr.SetField(field_name, field_value->repr);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_out_edge_iterator_set_field_by_id(lgraph_api_out_edge_iterator_t* it,
                                                  size_t field_id,
                                                  const lgraph_api_field_data_t* field_value,
                                                  char** errptr) {
    try {
        it->repr.SetField(field_id, field_value->repr);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_out_edge_iterator_set_fields_by_value_strings(
    lgraph_api_out_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const char* const* field_value_strings, size_t field_value_strings_len, char** errptr) {
    try {
        std::vector<std::string> field_names_vec(field_names, field_names + field_names_len);
        std::vector<std::string> field_value_strings_vec(
            field_value_strings, field_value_strings + field_value_strings_len);
        it->repr.SetFields(field_names_vec, field_value_strings_vec);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

static std::vector<FieldData> lgraph_api_field_data_array_to_field_data_vec(
    const lgraph_api_field_data_t* const* field_data_array, size_t len) {
    std::vector<FieldData> field_data_vec;
    field_data_vec.reserve(len);
    for (size_t i = 0; i < len; i++) {
        field_data_vec.emplace_back(field_data_array[i]->repr);
    }
    return field_data_vec;
}

void lgraph_api_out_edge_iterator_set_fields_by_data(
    lgraph_api_out_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr) {
    try {
        std::vector<std::string> field_names_vec(field_names, field_names + field_names_len);
        it->repr.SetFields(field_names_vec, lgraph_api_field_data_array_to_field_data_vec(
                                                field_values, field_values_len));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_out_edge_iterator_set_fields_by_ids(
    lgraph_api_out_edge_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr) {
    try {
        it->repr.SetFields(
            std::vector<size_t>(field_ids, field_ids + field_ids_len),
            lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_len));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_out_edge_iterator_delete(lgraph_api_out_edge_iterator_t* it, char** errptr) {
    try {
        it->repr.Delete();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

char* lgraph_api_out_edge_iterator_to_string(lgraph_api_out_edge_iterator_t* it, char** errptr) {
    try {
        return strdup(it->repr.ToString().c_str());
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

void lgraph_api_in_edge_iterator_destroy(lgraph_api_in_edge_iterator_t* it) { delete it; }

void lgraph_api_in_edge_iterator_close(lgraph_api_in_edge_iterator_t* it) { it->repr.Close(); }

bool lgraph_api_in_edge_iterator_next(lgraph_api_in_edge_iterator_t* it, char** errptr) {
    try {
        return it->repr.Next();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_in_edge_iterator_goto(lgraph_api_in_edge_iterator_t* it,
                                      lgraph_api_edge_uid_t* euid, bool nearest, char** errptr) {
    try {
        return it->repr.Goto(euid->repr, nearest);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}
lgraph_api_edge_uid_t* lgraph_api_in_edge_iterator_get_uid(lgraph_api_in_edge_iterator_t* it,
                                                           char** errptr) {
    try {
        return new lgraph_api_edge_uid_t{it->repr.GetUid()};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
int64_t lgraph_api_in_edge_iterator_get_src(lgraph_api_in_edge_iterator_t* it, char** errptr) {
    try {
        return it->repr.GetSrc();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
int64_t lgraph_api_in_edge_iterator_get_dst(lgraph_api_in_edge_iterator_t* it, char** errptr) {
    try {
        return it->repr.GetDst();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
int64_t lgraph_api_in_edge_iterator_get_edge_id(lgraph_api_in_edge_iterator_t* it, char** errptr) {
    try {
        return it->repr.GetEdgeId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
int64_t lgraph_api_in_edge_iterator_get_temporal_id(lgraph_api_in_edge_iterator_t* it,
                                                    char** errptr) {
    try {
        return it->repr.GetTemporalId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
bool lgraph_api_in_edge_iterator_is_valid(lgraph_api_in_edge_iterator_t* it) {
    return it->repr.IsValid();
}

const char* lgraph_api_in_edge_iterator_get_label(lgraph_api_in_edge_iterator_t* it,
                                                  char** errptr) {
    try {
        return strdup(it->repr.GetLabel().c_str());
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

uint16_t lgraph_api_in_edge_iterator_get_label_id(lgraph_api_in_edge_iterator_t* it,
                                                  char** errptr) {
    try {
        return it->repr.GetLabelId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

size_t lgraph_api_in_edge_iterator_get_fields_by_names(lgraph_api_in_edge_iterator_t* it,
                                                       const char* const* field_names,
                                                       size_t field_names_len,
                                                       lgraph_api_field_data_t*** field_data,
                                                       char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldData> _field_data = it->repr.GetFields(
            std::vector<std::string>(field_names, field_names + field_names_len));
        n = _field_data.size();
        *field_data = new lgraph_api_field_data_t*[n];
        for (size_t i = 0; i < _field_data.size(); i++) {
            (*field_data)[i] = new lgraph_api_field_data_t{_field_data[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

lgraph_api_field_data_t* lgraph_api_in_edge_iterator_get_field(lgraph_api_in_edge_iterator_t* it,
                                                               const char* field_name,
                                                               char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetField(field_name)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_in_edge_iterator_get_fields_by_ids(lgraph_api_in_edge_iterator_t* it,
                                                     const size_t* field_ids, size_t field_ids_len,
                                                     lgraph_api_field_data_t*** field_data,
                                                     char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldData> _field_data =
            it->repr.GetFields(std::vector<size_t>(field_ids, field_ids + field_ids_len));
        n = _field_data.size();
        *field_data = new lgraph_api_field_data_t*[n];
        for (size_t i = 0; i < _field_data.size(); i++) {
            (*field_data)[i] = new lgraph_api_field_data_t{_field_data[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

lgraph_api_field_data_t* lgraph_api_in_edge_iterator_get_field_by_id(
    lgraph_api_in_edge_iterator_t* it, size_t field_id, char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetField(field_id)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_field_data_t* lgraph_api_in_edge_iterator_get_field_by_name(
    lgraph_api_in_edge_iterator_t* it, const char* field_name, char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetField(field_name)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_in_edge_iterator_get_all_fields(lgraph_api_in_edge_iterator_t* it,
                                                  char*** field_names,
                                                  lgraph_api_field_data_t*** field_data,
                                                  char** errptr) {
    size_t n = 0;
    try {
        std::map<std::string, FieldData> field_data_map = it->repr.GetAllFields();
        n = field_data_map.size();
        *field_names = new char*[n];
        *field_data = new lgraph_api_field_data_t*[n];
        size_t i = 0;
        for (auto const& [key, val] : field_data_map) {
            (*field_names)[i] = strdup(key.c_str());
            (*field_data)[i] = new lgraph_api_field_data_t{val};
            i++;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

void lgraph_api_in_edge_iterator_set_field_by_name(lgraph_api_in_edge_iterator_t* it,
                                                   const char* field_name,
                                                   const lgraph_api_field_data_t* field_value,
                                                   char** errptr) {
    try {
        it->repr.SetField(field_name, field_value->repr);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_in_edge_iterator_set_field_by_id(lgraph_api_in_edge_iterator_t* it, size_t field_id,
                                                 const lgraph_api_field_data_t* field_value,
                                                 char** errptr) {
    try {
        it->repr.SetField(field_id, field_value->repr);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_in_edge_iterator_set_fields_by_value_strings(
    lgraph_api_in_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const char* const* field_value_strings, size_t field_value_strings_len, char** errptr) {
    try {
        std::vector<std::string> names(field_names, field_names + field_names_len);
        std::vector<std::string> values(field_value_strings,
                                        field_value_strings + field_value_strings_len);
        it->repr.SetFields(names, values);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_in_edge_iterator_set_fields_by_data(
    lgraph_api_in_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr) {
    try {
        std::vector<std::string> field_names_vec(field_names, field_names + field_names_len);
        it->repr.SetFields(field_names_vec, lgraph_api_field_data_array_to_field_data_vec(
                                                field_values, field_values_len));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_in_edge_iterator_set_fields_by_ids(
    lgraph_api_in_edge_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr) {
    try {
        std::vector<size_t> ids(field_ids, field_ids + field_ids_len);
        it->repr.SetFields(
            ids, lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_len));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_in_edge_iterator_delete(lgraph_api_in_edge_iterator_t* it, char** errptr) {
    try {
        it->repr.Delete();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

char* lgraph_api_in_edge_iterator_to_string(lgraph_api_in_edge_iterator_t* it, char** errptr) {
    try {
        return strdup(it->repr.ToString().c_str());
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

bool lgraph_api_out_edge_iterator_eq(const lgraph_api_out_edge_iterator_t* lhs,
                                     const lgraph_api_out_edge_iterator_t* rhs) {
    return lhs->repr == rhs->repr;
}
bool LGraphApi_OutInEdgeIterator_eq(const lgraph_api_out_edge_iterator_t* lhs,
                                    const lgraph_api_in_edge_iterator_t* rhs) {
    return lhs->repr == rhs->repr;
}
bool lgraph_api_in_edge_iterator_eq2(const lgraph_api_in_edge_iterator_t* lhs,
                                     const lgraph_api_in_edge_iterator_t* rhs) {
    return lhs->repr == rhs->repr;
}

void lgraph_api_vertex_iterator_destroy(lgraph_api_vertex_iterator_t* it) { delete it; }

void lgraph_api_vertex_iterator_close(lgraph_api_vertex_iterator_t* it) { it->repr.Close(); }

bool lgraph_api_vertex_iterator_next(lgraph_api_vertex_iterator_t* it, char** errptr) {
    try {
        return it->repr.Next();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_vertex_iterator_goto(lgraph_api_vertex_iterator_t* it, int64_t vid, bool nearest,
                                     char** errptr) {
    try {
        return it->repr.Goto(vid, nearest);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}
int64_t lgraph_api_vertex_iterator_get_id(lgraph_api_vertex_iterator_t* it, char** errptr) {
    try {
        return it->repr.GetId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

lgraph_api_out_edge_iterator_t* lgraph_api_vertex_iterator_get_out_edge_iterator(
    lgraph_api_vertex_iterator_t* it, char** errptr) {
    try {
        auto out_edge_it = it->repr.GetOutEdgeIterator();
        return new lgraph_api_out_edge_iterator_t{std::move(out_edge_it)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_out_edge_iterator_t* lgraph_api_vertex_iterator_get_out_edge_iterator_by_euid(
    lgraph_api_vertex_iterator_t* it, const lgraph_api_edge_uid_t* euid, bool nearest,
    char** errptr) {
    try {
        auto out_edge_it = it->repr.GetOutEdgeIterator(euid->repr, nearest);
        return new lgraph_api_out_edge_iterator_t{std::move(out_edge_it)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_in_edge_iterator_t* lgraph_api_vertex_iterator_get_in_edge_iterator(
    lgraph_api_vertex_iterator_t* it, char** errptr) {
    try {
        auto in_edge_it = it->repr.GetInEdgeIterator();
        return new lgraph_api_in_edge_iterator_t{std::move(in_edge_it)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_in_edge_iterator_t* lgraph_api_vertex_iterator_get_in_edge_iterator_by_euid(
    lgraph_api_vertex_iterator_t* it, const lgraph_api_edge_uid_t* euid, bool nearest,
    char** errptr) {
    try {
        auto in_edge_it = it->repr.GetInEdgeIterator(euid->repr, nearest);
        return new lgraph_api_in_edge_iterator_t{std::move(in_edge_it)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

bool lgraph_api_vertex_iterator_is_valid(lgraph_api_vertex_iterator_t* it) {
    return it->repr.IsValid();
}

const char* lgraph_api_vertex_iterator_get_label(lgraph_api_vertex_iterator_t* it, char** errptr) {
    try {
        return strdup(it->repr.GetLabel().c_str());
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

uint16_t lgraph_api_vertex_iterator_get_label_id(lgraph_api_vertex_iterator_t* it, char** errptr) {
    try {
        return it->repr.GetLabelId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

size_t lgraph_api_vertex_iterator_get_fields_by_names(lgraph_api_vertex_iterator_t* it,
                                                      const char* const* field_names,
                                                      size_t field_names_len,
                                                      lgraph_api_field_data_t*** field_data,
                                                      char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldData> _field_data = it->repr.GetFields(
            std::vector<std::string>(field_names, field_names + field_names_len));
        n = _field_data.size();
        *field_data = new lgraph_api_field_data_t*[n];
        for (size_t i = 0; i < _field_data.size(); i++) {
            (*field_data)[i] = new lgraph_api_field_data_t{_field_data[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

lgraph_api_field_data_t* lgraph_api_vertex_iterator_get_field_by_name(
    lgraph_api_vertex_iterator_t* it, const char* field_name, char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetField(field_name)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_vertex_iterator_get_fields_by_ids(lgraph_api_vertex_iterator_t* it,
                                                    const size_t* field_ids, size_t field_ids_len,
                                                    lgraph_api_field_data_t*** field_data,
                                                    size_t* size, char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldData> _field_data =
            it->repr.GetFields(std::vector<size_t>(field_ids, field_ids + field_ids_len));
        n = _field_data.size();
        *field_data = new lgraph_api_field_data_t*[n];
        for (size_t i = 0; i < _field_data.size(); i++) {
            (*field_data)[i] = new lgraph_api_field_data_t{_field_data[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

lgraph_api_field_data_t* lgraph_api_vertex_iterator_get_field_by_id(
    lgraph_api_vertex_iterator_t* it, size_t field_id, char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetField(field_id)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_vertex_iterator_get_all_fields(lgraph_api_vertex_iterator_t* it,
                                                 char*** field_names,
                                                 lgraph_api_field_data_t*** field_data,
                                                 char** errptr) {
    size_t n = 0;
    try {
        std::map<std::string, FieldData> field_data_map = it->repr.GetAllFields();
        n = field_data_map.size();
        *field_names = new char*[n];
        *field_data = new lgraph_api_field_data_t*[n];
        size_t i = 0;
        for (auto const& [key, val] : field_data_map) {
            (*field_names)[i] = strdup(key.c_str());
            (*field_data)[i] = new lgraph_api_field_data_t{val};
            i++;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

void lgraph_api_vertex_iterator_set_field_by_name(lgraph_api_vertex_iterator_t* it,
                                                  const char* field_name,
                                                  const lgraph_api_field_data_t* field_value,
                                                  char** errptr) {
    try {
        it->repr.SetField(field_name, field_value->repr);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_vertex_iterator_set_field_by_id(lgraph_api_vertex_iterator_t* it, size_t field_id,
                                                const lgraph_api_field_data_t* field_value,
                                                char** errptr) {
    try {
        it->repr.SetField(field_id, field_value->repr);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_vertex_iterator_set_fields_by_value_strings(
    lgraph_api_vertex_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const char* const* field_value_strings, size_t field_value_strings_len, char** errptr) {
    try {
        std::vector<std::string> field_names_vec(field_names, field_names + field_names_len);
        std::vector<std::string> field_value_strings_vec(field_value_strings,
                                                         field_value_strings + field_names_len);
        it->repr.SetFields(field_names_vec, field_value_strings_vec);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_vertex_iterator_set_fields_by_data(
    lgraph_api_vertex_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr) {
    try {
        std::vector<std::string> field_names_vec(field_names, field_names + field_names_len);
        it->repr.SetFields(field_names_vec, lgraph_api_field_data_array_to_field_data_vec(
                                                field_values, field_values_len));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_vertex_iterator_set_fields_by_ids(
    lgraph_api_vertex_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr) {
    try {
        std::vector<size_t> ids(field_ids, field_ids + field_ids_len);
        it->repr.SetFields(
            ids, lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_len));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}
size_t lgraph_api_vertex_iterator_list_src_vids(lgraph_api_vertex_iterator_t* it, size_t n_limit,
                                                bool* more_to_go, int64_t** vids, char** errptr) {
    size_t n = 0;
    try {
        auto vids_vec = it->repr.ListSrcVids(n_limit, more_to_go);
        n = vids_vec.size();
        *vids = new int64_t[n];
        for (size_t i = 0; i < n; ++i) {
            (*vids)[i] = vids_vec[i];
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        *vids = nullptr;
    }
    return n;
}

size_t lgraph_api_vertex_iterator_list_dst_vids(lgraph_api_vertex_iterator_t* it, size_t n_limit,
                                                bool* more_to_go, int64_t** vids, char** errptr) {
    size_t n = 0;
    try {
        auto vids_vec = it->repr.ListDstVids(n_limit, more_to_go);
        n = vids_vec.size();
        *vids = new int64_t[n];
        for (size_t i = 0; i < n; ++i) {
            (*vids)[i] = vids_vec[i];
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        *vids = nullptr;
    }
    return n;
}

void lgraph_api_vertex_iterator_list_vids_destroy(int64_t* vids) { delete[] vids; }
size_t lgraph_api_vertex_iterator_get_num_in_edges(lgraph_api_vertex_iterator_t* it, size_t n_limit,
                                                   bool* more_to_go, char** errptr) {
    try {
        return it->repr.GetNumInEdges(n_limit, more_to_go);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

size_t lgraph_api_vertex_iterator_get_num_out_edges(lgraph_api_vertex_iterator_t* it,
                                                    size_t n_limit, bool* more_to_go,
                                                    char** errptr) {
    try {
        return it->repr.GetNumOutEdges(n_limit, more_to_go);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

void lgraph_api_vertex_iterator_delete(lgraph_api_vertex_iterator_t* it, size_t* n_in_edges,
                                       size_t* n_out_edges, char** errptr) {
    try {
        it->repr.Delete(n_in_edges, n_out_edges);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

char* lgraph_api_vertex_iterator_to_string(lgraph_api_vertex_iterator_t* it, char** errptr) {
    return strdup(it->repr.ToString().c_str());
}

void lgraph_api_vertex_index_iterator_destroy(lgraph_api_vertex_index_iterator_t* it) { delete it; }

void lgraph_api_vertex_index_iterator_close(lgraph_api_vertex_index_iterator_t* it) {
    it->repr.Close();
}

bool lgraph_api_vertex_index_iterator_is_valid(lgraph_api_vertex_index_iterator_t* it) {
    return it->repr.IsValid();
}

bool lgraph_api_vertex_index_iterator_next(lgraph_api_vertex_index_iterator_t* it, char** errptr) {
    try {
        return it->repr.Next();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

lgraph_api_field_data_t* lgraph_api_vertex_index_iterator_get_index_value(
    lgraph_api_vertex_index_iterator_t* it, char** errptr) {
    try {
        return new lgraph_api_field_data_t{it->repr.GetIndexValue()};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

int64_t lgraph_api_vertex_index_iterator_get_vid(lgraph_api_vertex_index_iterator_t* it,
                                                 char** errptr) {
    try {
        return it->repr.GetVid();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

void lgraph_api_edge_index_iterator_close(lgraph_api_edge_index_iterator_t* iter) {
    iter->repr.Close();
}

void lgraph_api_edge_index_iterator_destroy(lgraph_api_edge_index_iterator_t* iter) { delete iter; }

bool lgraph_api_edge_index_iterator_is_valid(lgraph_api_edge_index_iterator_t* iter) {
    return iter->repr.IsValid();
}

bool lgraph_api_edge_index_iterator_next(lgraph_api_edge_index_iterator_t* iter, char** errptr) {
    try {
        return iter->repr.Next();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

lgraph_api_field_data_t* lgraph_api_edge_index_iterator_get_index_value(
    lgraph_api_edge_index_iterator_t* iter, char** errptr) {
    try {
        return new lgraph_api_field_data_t{iter->repr.GetIndexValue()};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_edge_uid_t* lgraph_api_edge_index_iterator_get_uid(
    lgraph_api_edge_index_iterator_t* iter, char** errptr) {
    try {
        return new lgraph_api_edge_uid_t{iter->repr.GetUid()};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

int64_t lgraph_api_edge_index_iterator_get_src(lgraph_api_edge_index_iterator_t* iter,
                                               char** errptr) {
    try {
        return iter->repr.GetSrc();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}

int64_t lgraph_api_edge_index_iterator_get_dst(lgraph_api_edge_index_iterator_t* iter,
                                               char** errptr) {
    try {
        return iter->repr.GetDst();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}

uint16_t lgraph_api_edge_index_iterator_get_label_id(lgraph_api_edge_index_iterator_t* iter,
                                                     char** errptr) {
    try {
        return iter->repr.GetLabelId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}

int64_t lgraph_api_edge_index_iterator_get_edge_id(lgraph_api_edge_index_iterator_t* iter,
                                                   char** errptr) {
    try {
        return iter->repr.GetEdgeId();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}

void lgraph_api_transaction_destroy(lgraph_api_transaction_t* txn) { delete txn; }
void lgraph_api_transaction_commit(lgraph_api_transaction_t* txn, char** errptr) {
    try {
        txn->repr.Commit();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}
void lgraph_api_transaction_abort(lgraph_api_transaction_t* txn, char** errptr) {
    try {
        txn->repr.Abort();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}
bool lgraph_api_transaction_is_valid(lgraph_api_transaction_t* txn) { return txn->repr.IsValid(); }
bool lgraph_api_transaction_is_read_only(lgraph_api_transaction_t* txn) {
    return txn->repr.IsReadOnly();
}

lgraph_api_vertex_iterator_t* lgraph_api_transaction_get_vertex_iterator(
    lgraph_api_transaction_t* txn, char** errptr) {
    try {
        return new lgraph_api_vertex_iterator_t{txn->repr.GetVertexIterator()};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
lgraph_api_vertex_iterator_t* lgraph_api_transaction_get_vertex_iterator_by_vid(
    lgraph_api_transaction_t* txn, int64_t vid, bool nearest, char** errptr) {
    try {
        return new lgraph_api_vertex_iterator_t{txn->repr.GetVertexIterator(vid, nearest)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
lgraph_api_out_edge_iterator_t* lgraph_api_transaction_get_out_edge_iterator_by_euid(
    lgraph_api_transaction_t* txn, const lgraph_api_edge_uid_t* euid, bool nearest, char** errptr) {
    try {
        return new lgraph_api_out_edge_iterator_t{
            txn->repr.GetOutEdgeIterator(euid->repr, nearest)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
lgraph_api_out_edge_iterator_t* lgraph_api_transaction_get_out_edge_iterator_by_src_dst_lid(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, int16_t lid, char** errptr) {
    try {
        return new lgraph_api_out_edge_iterator_t{txn->repr.GetOutEdgeIterator(src, dst, lid)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
lgraph_api_in_edge_iterator_t* lgraph_api_transaction_get_in_edge_iterator_by_euid(
    lgraph_api_transaction_t* txn, const lgraph_api_edge_uid_t* euid, bool nearest, char** errptr) {
    try {
        return new lgraph_api_in_edge_iterator_t{txn->repr.GetInEdgeIterator(euid->repr, nearest)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
lgraph_api_in_edge_iterator_t* lgraph_api_transaction_get_in_edge_iterator_by_src_dst_lid(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, int16_t lid, char** errptr) {
    try {
        return new lgraph_api_in_edge_iterator_t{txn->repr.GetInEdgeIterator(src, dst, lid)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
size_t lgraph_api_transaction_get_num_vertex_labels(lgraph_api_transaction_t* txn, char** errptr) {
    try {
        return txn->repr.GetNumVertexLabels();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
size_t lgraph_api_transaction_get_num_edge_labels(lgraph_api_transaction_t* txn, char** errptr) {
    try {
        return txn->repr.GetNumEdgeLabels();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
size_t lgraph_api_transaction_list_vertex_labels(lgraph_api_transaction_t* txn, char*** labels,
                                                 char** errptr) {
    size_t n = 0;
    try {
        std::vector<std::string> label_vec = txn->repr.ListVertexLabels();
        n = label_vec.size();
        *labels = new char*[n];
        for (size_t i = 0; i < n; ++i) {
            (*labels)[i] = strdup(label_vec[i].c_str());
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}
size_t lgraph_api_transaction_list_edge_labels(lgraph_api_transaction_t* txn, char*** labels,
                                               char** errptr) {
    size_t n = 0;
    try {
        std::vector<std::string> label_vec = txn->repr.ListEdgeLabels();
        n = label_vec.size();
        *labels = new char*[n];
        for (size_t i = 0; i < n; ++i) {
            (*labels)[i] = strdup(label_vec[i].c_str());
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}
void lgraph_api_transaction_list_labels_destroy(char** labels, size_t n) {
    for (size_t i = 0; i < n; i++) {
        delete labels[i];
    }
    delete[] labels;
}
size_t lgraph_api_transaction_get_vertex_label_id(lgraph_api_transaction_t* txn, const char* label,
                                                  char** errptr) {
    try {
        return txn->repr.GetVertexLabelId(label);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
size_t lgraph_api_transaction_get_edge_label_id(lgraph_api_transaction_t* txn, const char* label,
                                                char** errptr) {
    try {
        return txn->repr.GetEdgeLabelId(label);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
size_t lgraph_api_transaction_get_vertex_schema(lgraph_api_transaction_t* txn, const char* label,
                                                lgraph_api_field_spec_t*** field_specs,
                                                char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldSpec> field_vec = txn->repr.GetVertexSchema(label);
        n = field_vec.size();
        *field_specs = new lgraph_api_field_spec_t*[n];
        for (size_t i = 0; i < n; ++i) {
            (*field_specs)[i] = new lgraph_api_field_spec_t{field_vec[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}
size_t lgraph_api_transaction_get_edge_schema(lgraph_api_transaction_t* txn, const char* label,
                                              lgraph_api_field_spec_t*** field_specs,
                                              char** errptr) {
    size_t n = 0;
    try {
        std::vector<FieldSpec> field_vec = txn->repr.GetEdgeSchema(label);
        n = field_vec.size();
        *field_specs = new lgraph_api_field_spec_t*[n];
        for (size_t i = 0; i < n; ++i) {
            (*field_specs)[i] = new lgraph_api_field_spec_t{field_vec[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}
size_t lgraph_api_transaction_get_vertex_field_id(lgraph_api_transaction_t* txn, size_t label_id,
                                                  const char* field_name, char** errptr) {
    try {
        return txn->repr.GetVertexFieldId(label_id, field_name);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
size_t lgraph_api_transaction_get_vertex_field_ids(lgraph_api_transaction_t* txn, size_t label_id,
                                                   const char* const* field_names,
                                                   size_t field_names_size, size_t** field_ids,
                                                   char** errptr) {
    size_t n = 0;
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        std::vector<size_t> id_vec = txn->repr.GetVertexFieldIds(label_id, name_vec);
        n = id_vec.size();
        *field_ids = new size_t[n];
        for (size_t i = 0; i < n; ++i) {
            (*field_ids)[i] = id_vec[i];
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}
size_t lgraph_api_transaction_get_edge_field_id(lgraph_api_transaction_t* txn, size_t label_id,
                                                const char* field_name, char** errptr) {
    try {
        return txn->repr.GetEdgeFieldId(label_id, field_name);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}
size_t lgraph_api_transaction_get_edge_field_ids(lgraph_api_transaction_t* txn, size_t label_id,
                                                 const char** field_names, size_t field_names_size,
                                                 size_t** field_ids, char** errptr) {
    size_t n = 0;
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        std::vector<size_t> id_vec = txn->repr.GetEdgeFieldIds(label_id, name_vec);
        n = id_vec.size();
        *field_ids = new size_t[n];
        for (size_t i = 0; i < n; ++i) {
            (*field_ids)[i] = id_vec[i];
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}
int64_t lgraph_api_transaction_add_vertex_with_value_strings(
    lgraph_api_transaction_t* txn, const char* label_name, const char* const* field_names,
    size_t field_names_size, const char* const* field_value_strings,
    size_t field_value_strings_size, char** errptr) {
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        std::vector<std::string> value_vec(field_value_strings,
                                           field_value_strings + field_value_strings_size);
        return txn->repr.AddVertex(label_name, name_vec, value_vec);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}
int64_t lgraph_api_transaction_add_vertex_with_field_data(
    lgraph_api_transaction_t* txn, const char* label_name, const char* const* field_names,
    size_t field_names_size, const lgraph_api_field_data_t* const* field_values,
    size_t field_values_size, char** errptr) {
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        return txn->repr.AddVertex(
            label_name, name_vec,
            lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_size));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}
int64_t lgraph_api_transaction_add_vertex_with_field_data_and_id(
    lgraph_api_transaction_t* txn, size_t label_id, const size_t* field_ids, size_t field_ids_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr) {
    try {
        std::vector<size_t> ids_vec(field_ids, field_ids + field_ids_size);
        return txn->repr.AddVertex(
            label_id, ids_vec,
            lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_size));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}
lgraph_api_edge_uid_t* lgraph_api_transaction_add_edge_with_value_strings(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char* const* field_names, size_t field_names_size, const char* const* field_value_strings,
    size_t field_value_strings_size, char** errptr) {
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        std::vector<std::string> value_vec(field_value_strings,
                                           field_value_strings + field_value_strings_size);
        return new lgraph_api_edge_uid_t{txn->repr.AddEdge(src, dst, label, name_vec, value_vec)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_edge_uid_t* lgraph_api_transaction_add_edge_with_field_data(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char* const* field_names, size_t field_names_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr) {
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        EdgeUid euid = txn->repr.AddEdge(
            src, dst, label, name_vec,
            lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_size));
        return new lgraph_api_edge_uid_t{euid};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
lgraph_api_edge_uid_t* lgraph_api_transaction_add_edge_with_field_data_and_id(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, size_t label_id,
    const size_t* field_ids, size_t field_ids_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr) {
    try {
        std::vector<size_t> id_vec(field_ids, field_ids + field_ids_size);

        EdgeUid euid = txn->repr.AddEdge(
            src, dst, label_id, id_vec,
            lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_size));
        return new lgraph_api_edge_uid_t{euid};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}
bool lgraph_api_transaction_upsert_edge_with_value_strings(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char** field_names, size_t field_names_size, const char** field_value_strings,
    size_t field_value_strings_size, char** errptr) {
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        std::vector<std::string> value_vec(field_value_strings,
                                           field_value_strings + field_value_strings_size);
        return txn->repr.UpsertEdge(src, dst, label, name_vec, value_vec);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_transaction_upsert_edge_with_field_data(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char* const* field_names, size_t field_names_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr) {
    try {
        std::vector<std::string> name_vec(field_names, field_names + field_names_size);
        return txn->repr.UpsertEdge(
            src, dst, label, name_vec,
            lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_size));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_transaction_upsert_edge_with_field_data_and_id(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, size_t label_id,
    const size_t* field_ids, size_t field_ids_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr) {
    try {
        std::vector<size_t> id_vec(field_ids, field_ids + field_ids_size);
        return txn->repr.UpsertEdge(
            src, dst, label_id, id_vec,
            lgraph_api_field_data_array_to_field_data_vec(field_values, field_values_size));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

size_t lgraph_api_transaction_list_vertex_indexes(lgraph_api_transaction_t* txn,
                                                  lgraph_api_index_spec_t*** indexes,
                                                  char** errptr) {
    size_t n = 0;
    try {
        auto vec = txn->repr.ListVertexIndexes();
        n = vec.size();
        *indexes = new lgraph_api_index_spec_t*[n];
        for (size_t i = 0; i < vec.size(); i++) {
            (*indexes)[i] = new lgraph_api_index_spec_t{vec[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

size_t lgraph_api_transaction_list_edge_indexes(lgraph_api_transaction_t* txn,
                                                lgraph_api_index_spec_t*** indexes, char** errptr) {
    size_t n = 0;
    try {
        auto vec = txn->repr.ListEdgeIndexes();
        n = vec.size();
        *indexes = new lgraph_api_index_spec_t*[n];
        for (size_t i = 0; i < vec.size(); i++) {
            (*indexes)[i] = new lgraph_api_index_spec_t{vec[i]};
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

void lgraph_api_list_indexes_detroy(lgraph_api_index_spec_t** indexes, size_t n) {
    for (size_t i = 0; i < n; i++) {
        delete indexes[i];
    }
    delete[] indexes;
}

lgraph_api_vertex_index_iterator_t* lgraph_api_transaction_get_vertex_index_iterator_by_id(
    lgraph_api_transaction_t* txn, size_t label_id, size_t field_id,
    const lgraph_api_field_data_t* key_start, const lgraph_api_field_data_t* key_end,
    char** errptr) {
    try {
        return new lgraph_api_vertex_index_iterator_t{
            txn->repr.GetVertexIndexIterator(label_id, field_id, key_start->repr, key_end->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_edge_index_iterator_t* lgraph_api_transaction_get_edge_index_iterator_by_id(
    lgraph_api_transaction_t* txn, size_t label_id, size_t field_id,
    const lgraph_api_field_data_t* key_start, const lgraph_api_field_data_t* key_end,
    char** errptr) {
    try {
        return new lgraph_api_edge_index_iterator_t{
            txn->repr.GetEdgeIndexIterator(label_id, field_id, key_start->repr, key_end->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_vertex_index_iterator_t* lgraph_api_transaction_get_vertex_index_iterator_by_data(
    lgraph_api_transaction_t* txn, const char* label, const char* field,
    const lgraph_api_field_data_t* key_start, const lgraph_api_field_data_t* key_end,
    char** errptr) {
    try {
        return new lgraph_api_vertex_index_iterator_t{
            txn->repr.GetVertexIndexIterator(label, field, key_start->repr, key_end->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_edge_index_iterator_t* lgraph_api_transaction_get_edge_index_iterator_by_data(
    lgraph_api_transaction_t* txn, const char* label, const char* field,
    const lgraph_api_field_data_t* key_start, const lgraph_api_field_data_t* key_end,
    char** errptr) {
    try {
        return new lgraph_api_edge_index_iterator_t{
            txn->repr.GetEdgeIndexIterator(label, field, key_start->repr, key_end->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_vertex_index_iterator_t*
lgraph_api_transaction_get_vertex_index_iterator_by_value_string(
    lgraph_api_transaction_t* txn, const char* label, const char* field, const char* key_start,
    const char* key_end, char** errptr) {
    try {
        return new lgraph_api_vertex_index_iterator_t{
            txn->repr.GetVertexIndexIterator(label, field, key_start, key_end)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_edge_index_iterator_t* lgraph_api_transaction_get_edge_index_iterator_by_value_string(
    lgraph_api_transaction_t* txn, const char* label, const char* field, const char* key_start,
    const char* key_end, char** errptr) {
    try {
        return new lgraph_api_edge_index_iterator_t{
            txn->repr.GetEdgeIndexIterator(label, field, key_start, key_end)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

bool lgraph_api_transaction_is_vertex_indexed(lgraph_api_transaction_t* txn, const char* label,
                                              const char* field, char** errptr) {
    try {
        return txn->repr.IsVertexIndexed(label, field);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_transaction_is_edge_indexed(lgraph_api_transaction_t* txn, const char* label,
                                            const char* field, char** errptr) {
    try {
        return txn->repr.IsEdgeIndexed(label, field);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

lgraph_api_vertex_iterator_t* lgraph_api_transaction_get_vertex_by_unique_index_value_string(
    lgraph_api_transaction_t* txn, const char* label_name, const char* field_name,
    const char* field_value_string, char** errptr) {
    try {
        return new lgraph_api_vertex_iterator_t{
            txn->repr.GetVertexByUniqueIndex(label_name, field_name, field_value_string)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_out_edge_iterator_t* lgraph_api_transaction_get_edge_by_unique_index(
    lgraph_api_transaction_t* txn, const char* label_name, const char* field_name,
    const char* field_value_string, char** errptr) {
    try {
        return new lgraph_api_out_edge_iterator_t{
            txn->repr.GetEdgeByUniqueIndex(label_name, field_name, field_value_string)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_vertex_iterator_t* lgraph_api_transaction_get_vertex_by_unique_index_with_data(
    lgraph_api_transaction_t* txn, const char* label_name, const char* field_name,
    const lgraph_api_field_data_t* field_value, char** errptr) {
    try {
        return new lgraph_api_vertex_iterator_t{
            txn->repr.GetVertexByUniqueIndex(label_name, field_name, field_value->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_out_edge_iterator_t* lgraph_api_transaction_get_edge_by_unique_index_with_data(
    lgraph_api_transaction_t* txn, const char* label_name, const char* field_name,
    const lgraph_api_field_data_t* field_value, char** errptr) {
    try {
        return new lgraph_api_out_edge_iterator_t{
            txn->repr.GetEdgeByUniqueIndex(label_name, field_name, field_value->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_vertex_iterator_t* lgraph_api_transaction_get_vertex_by_unique_index_with_id(
    lgraph_api_transaction_t* txn, size_t label_id, size_t field_id,
    const lgraph_api_field_data_t* field_value, char** errptr) {
    try {
        return new lgraph_api_vertex_iterator_t{
            txn->repr.GetVertexByUniqueIndex(label_id, field_id, field_value->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_out_edge_iterator_t* lgraph_api_transaction_get_edge_by_unique_index_with_id(
    lgraph_api_transaction_t* txn, size_t label_id, size_t field_id,
    const lgraph_api_field_data_t* field_value, char** errptr) {
    try {
        return new lgraph_api_out_edge_iterator_t{
            txn->repr.GetEdgeByUniqueIndex(label_id, field_id, field_value->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_transaction_get_num_vertices(lgraph_api_transaction_t* txn, char** errptr) {
    try {
        return txn->repr.GetNumVertices();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}

const char* lgraph_api_transaction_get_vertex_primary_field(lgraph_api_transaction_t* txn,
                                                            const char* label, char** errptr) {
    try {
        return strdup(txn->repr.GetVertexPrimaryField(label).c_str());
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

void lgraph_api_graph_db_close(lgraph_api_graph_db_t* graphdb, char** errptr) {
    try {
        graphdb->repr.Close();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_graph_db_destroy(lgraph_api_graph_db_t* graphdb) { delete graphdb; }

lgraph_api_transaction_t* lgraph_api_graph_db_create_read_txn(lgraph_api_graph_db_t* graphdb,
                                                              char** errptr) {
    try {
        return new lgraph_api_transaction_t{graphdb->repr.CreateReadTxn()};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_transaction_t* lgraph_api_graph_db_create_write_txn(lgraph_api_graph_db_t* graphdb,
                                                               bool optimistic, char** errptr) {
    try {
        return new lgraph_api_transaction_t{graphdb->repr.CreateWriteTxn(optimistic)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_transaction_t* lgraph_api_graph_db_fork_txn(lgraph_api_graph_db_t* graphdb,
                                                       lgraph_api_transaction_t* txn,
                                                       char** errptr) {
    try {
        return new lgraph_api_transaction_t{graphdb->repr.ForkTxn(txn->repr)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

void lgraph_api_graph_db_flush(lgraph_api_graph_db_t* graphdb, char** errptr) {
    try {
        graphdb->repr.Flush();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_graph_db_drop_all_data(lgraph_api_graph_db_t* graphdb, char** errptr) {
    try {
        graphdb->repr.DropAllData();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_graph_db_drop_all_vertex(lgraph_api_graph_db_t* graphdb, char** errptr) {
    try {
        graphdb->repr.DropAllVertex();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

size_t lgraph_api_graph_db_estimate_num_vertices(lgraph_api_graph_db_t* graphdb, char** errptr) {
    try {
        return graphdb->repr.EstimateNumVertices();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}

static std::vector<FieldSpec> lgraph_api_field_spec_array_to_feild_spec_vec(
    const lgraph_api_field_spec_t* const* fds_array, size_t fds_len) {
    std::vector<FieldSpec> field_specs;
    field_specs.reserve(fds_len);
    for (size_t i = 0; i < fds_len; i++) {
        field_specs.emplace_back(fds_array[i]->repr);
    }
    return field_specs;
}

bool lgraph_api_graph_db_add_vertex_label(lgraph_api_graph_db_t* graphdb, const char* label,
                                          const lgraph_api_field_spec_t* const* fds, size_t fds_len,
                                          const char* primary_field, char** errptr) {
    try {
        graphdb->repr.AddVertexLabel(
            label, lgraph_api_field_spec_array_to_feild_spec_vec(fds, fds_len),
            VertexOptions(primary_field));
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_delete_vertex_label(lgraph_api_graph_db_t* graphdb, const char* label,
                                             size_t* n_modified, char** errptr) {
    try {
        *n_modified = graphdb->repr.DeleteVertexLabel(label);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_alter_vertex_label_del_fields(lgraph_api_graph_db_t* graphdb,
                                                       const char* label,
                                                       const char* const* del_fields,
                                                       size_t del_fields_len, size_t* n_modified,
                                                       char** errptr) {
    try {
        std::vector<std::string> fields(del_fields, del_fields + del_fields_len);
        *n_modified = graphdb->repr.AlterVertexLabelDelFields(label, fields);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_alter_vertex_label_add_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* add_fields, size_t add_fields_len,
    const lgraph_api_field_data_t* const* default_values, size_t default_values_len,
    size_t* n_modified, char** errptr) {
    try {
        return graphdb->repr.AlterVertexLabelAddFields(
            label, lgraph_api_field_spec_array_to_feild_spec_vec(add_fields, add_fields_len),
            lgraph_api_field_data_array_to_field_data_vec(default_values, default_values_len),
            n_modified);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_alter_vertex_label_mod_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* mod_fields, size_t mod_fields_len, size_t* n_modified,
    char** errptr) {
    try {
        *n_modified = graphdb->repr.AlterVertexLabelModFields(
            label, lgraph_api_field_spec_array_to_feild_spec_vec(mod_fields, mod_fields_len),
            n_modified);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_add_edge_label(lgraph_api_graph_db_t* graphdb, const char* label,
                                        const lgraph_api_field_spec_t* const* fds, size_t fds_len,
                                        const char* temporal_field,
                                        const char* const* first_edge_constraints,
                                        const char* const* second_edge_constraints,
                                        size_t edge_constraints_len, char** errptr) {
    try {
        std::vector<std::pair<std::string, std::string>> constraints;
        for (size_t i = 0; i < edge_constraints_len; i++) {
            constraints.emplace_back(first_edge_constraints[i], second_edge_constraints[i]);
        }
        EdgeOptions options;
        options.temporal_field = temporal_field;
        options.edge_constraints = constraints;
        graphdb->repr.AddEdgeLabel(label,
                                   lgraph_api_field_spec_array_to_feild_spec_vec(fds, fds_len),
                                   options);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_delete_edge_label(lgraph_api_graph_db_t* graphdb, const char* label,
                                           size_t* n_modified, char** errptr) {
    try {
        return graphdb->repr.DeleteEdgeLabel(label, n_modified);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_alter_label_mod_edge_constraints(
    lgraph_api_graph_db_t* graphdb, const char* label, const char* const* first_edge_constraints,
    const char* const* second_edge_constraints, size_t edge_constraints_len, char** errptr) {
    try {
        std::vector<std::pair<std::string, std::string>> constraints_vec;
        for (size_t i = 0; i < edge_constraints_len; i++) {
            constraints_vec.emplace_back(first_edge_constraints[i], second_edge_constraints[i]);
        }
        graphdb->repr.AlterLabelModEdgeConstraints(label, constraints_vec);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_alter_edge_label_del_fields(lgraph_api_graph_db_t* graphdb,
                                                     const char* label,
                                                     const char* const* del_fields,
                                                     size_t del_fields_len, size_t* n_modified,
                                                     char** errptr) {
    try {
        std::vector<std::string> fields(del_fields, del_fields + del_fields_len);
        return graphdb->repr.AlterEdgeLabelDelFields(label, fields, n_modified);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_alter_edge_label_add_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* add_fields, size_t add_fields_len,
    const lgraph_api_field_data_t* const* default_values, size_t default_values_len,
    size_t* n_modified, char** errptr) {
    try {
        return graphdb->repr.AlterEdgeLabelAddFields(
            label, lgraph_api_field_spec_array_to_feild_spec_vec(add_fields, add_fields_len),
            lgraph_api_field_data_array_to_field_data_vec(default_values, default_values_len),
            n_modified);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_alter_edge_label_mod_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* mod_fields, size_t mod_fields_len, size_t* n_modified,
    char** errptr) {
    try {
        return graphdb->repr.AlterEdgeLabelModFields(
            label, lgraph_api_field_spec_array_to_feild_spec_vec(mod_fields, mod_fields_len),
            n_modified);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_add_vertex_index(lgraph_api_graph_db_t* graphdb, const char* label,
                                          const char* field, bool is_unique, char** errptr) {
    try {
        graphdb->repr.AddVertexIndex(label, field, is_unique);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_add_edge_index(lgraph_api_graph_db_t* graphdb, const char* label,
                                        const char* field, bool is_unique, char** errptr) {
    try {
        graphdb->repr.AddEdgeIndex(label, field, is_unique);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_is_vertex_indexed(lgraph_api_graph_db_t* graphdb, const char* label,
                                           const char* field, char** errptr) {
    try {
        return graphdb->repr.IsVertexIndexed(label, field);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_is_edge_indexed(lgraph_api_graph_db_t* graphdb, const char* label,
                                         const char* field, char** errptr) {
    try {
        return graphdb->repr.IsEdgeIndexed(label, field);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_delete_vertex_index(lgraph_api_graph_db_t* graphdb, const char* label,
                                             const char* field, char** errptr) {
    try {
        graphdb->repr.DeleteVertexIndex(label, field);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_delete_edge_index(lgraph_api_graph_db_t* graphdb, const char* label,
                                           const char* field, char** errptr) {
    try {
        graphdb->repr.DeleteEdgeIndex(label, field);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

const char* lgraph_api_graph_db_get_description(lgraph_api_graph_db_t* graphdb, char** errptr) {
    try {
        return strdup(graphdb->repr.GetDescription().c_str());
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_graph_db_get_max_size(lgraph_api_graph_db_t* graphdb, char** errptr) {
    try {
        return graphdb->repr.GetMaxSize();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return 0;
    }
}

bool lgraph_api_graph_db_add_vertex_full_text_index(lgraph_api_graph_db_t* graphdb,
                                                    const char* vertex_label, const char* field,
                                                    char** errptr) {
    try {
        graphdb->repr.AddVertexFullTextIndex(vertex_label, field);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_add_edge_full_text_index(lgraph_api_graph_db_t* graphdb,
                                                  const char* edge_label, const char* field,
                                                  char** errptr) {
    try {
        graphdb->repr.AddEdgeFullTextIndex(edge_label, field);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_delete_vertex_full_text_index(lgraph_api_graph_db_t* graphdb,
                                                       const char* vertex_label, const char* field,
                                                       char** errptr) {
    try {
        graphdb->repr.DeleteVertexFullTextIndex(vertex_label, field);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_graph_db_delete_edge_full_text_index(lgraph_api_graph_db_t* graphdb,
                                                     const char* edge_label, const char* field,
                                                     char** errptr) {
    try {
        graphdb->repr.DeleteEdgeFullTextIndex(edge_label, field);
        return true;
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

void lgraph_api_graph_db_rebuild_full_text_index(lgraph_api_graph_db_t* graphdb,
                                                 const char* const* vertex_labels,
                                                 size_t vertex_labels_len,
                                                 const char* const* edge_labels,
                                                 size_t edge_labels_len, char** errptr) {
    try {
        std::set<std::string> vertex_labels_vec(vertex_labels, vertex_labels + vertex_labels_len);
        std::set<std::string> edge_labels_vec(edge_labels, edge_labels + edge_labels_len);
        graphdb->repr.RebuildFullTextIndex(vertex_labels_vec, edge_labels_vec);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

size_t lgraph_api_graph_db_list_full_text_indexes(lgraph_api_graph_db_t* graphdb, bool** is_vertex,
                                                  char*** label_names, char*** property_names,
                                                  char** errptr) {
    size_t n = 0;
    try {
        std::vector<std::tuple<bool, std::string, std::string>> indexes =
            graphdb->repr.ListFullTextIndexes();
        n = indexes.size();
        *is_vertex = new bool[n];
        *label_names = new char*[n];
        *property_names = new char*[n];
        for (size_t i = 0; i < n; i++) {
            (*is_vertex)[i] = std::get<0>(indexes[i]);
            (*label_names)[i] = strdup(std::get<1>(indexes[i]).c_str());
            (*property_names)[i] = strdup(std::get<2>(indexes[i]).c_str());
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

void lgraph_api_graph_db_list_full_text_indexes_destroy(bool* is_vertex, char** label_names,
                                                        char** property_names, size_t n) {
    for (size_t i = 0; i < n; i++) {
        delete label_names[i];
        delete property_names[i];
    }
    delete[] is_vertex;
    delete[] label_names;
    delete[] property_names;
}

size_t lgraph_api_graph_db_query_vertex_by_full_text_index(lgraph_api_graph_db_t* graphdb,
                                                           const char* label, const char* query,
                                                           int top_n, int64_t** vids,
                                                           float** scores, char** errptr) {
    size_t n = 0;
    try {
        std::vector<std::pair<int64_t, float>> results =
            graphdb->repr.QueryVertexByFullTextIndex(label, query, top_n);
        n = results.size();
        *vids = new int64_t[n];
        *scores = new float[n];
        for (size_t i = 0; i < n; i++) {
            (*vids)[i] = results[i].first;
            (*scores)[i] = results[i].second;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

size_t lgraph_api_graph_db_query_edge_by_full_text_index(lgraph_api_graph_db_t* graphdb,
                                                         const char* label, const char* query,
                                                         int top_n, lgraph_api_edge_uid_t*** euids,
                                                         float** scores, char** errptr) {
    size_t n = 0;
    try {
        std::vector<std::pair<EdgeUid, float>> results =
            graphdb->repr.QueryEdgeByFullTextIndex(label, query, top_n);
        n = results.size();
        *euids = new lgraph_api_edge_uid_t*[n];
        *scores = new float[n];
        for (size_t i = 0; i < n; i++) {
            (*euids)[i] = new lgraph_api_edge_uid_t{results[i].first};
            (*scores)[i] = results[i].second;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

lgraph_api_galaxy_t* lgraph_api_galaxy_create(const char* dir, bool durable,
                                              bool create_if_not_exist, char** errptr) {
    try {
        return new lgraph_api_galaxy_t{Galaxy(dir, durable, create_if_not_exist)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

lgraph_api_galaxy_t* lgraph_api_galaxy_create_with_user(const char* dir, const char* user,
                                                        const char* password, bool durable,
                                                        bool create_if_not_exist, char** errptr) {
    try {
        return new lgraph_api_galaxy_t{Galaxy(dir, user, password, durable, create_if_not_exist)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

void lgraph_api_galaxy_destroy(lgraph_api_galaxy_t* galaxy) { delete galaxy; }

void lgraph_api_galaxy_set_current_user(lgraph_api_galaxy_t* galaxy, const char* user,
                                        const char* password, char** errptr) {
    try {
        galaxy->repr.SetCurrentUser(user, password);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

void lgraph_api_galaxy_set_user(lgraph_api_galaxy_t* galaxy, const char* user, char** errptr) {
    try {
        galaxy->repr.SetUser(user);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}

bool lgraph_api_galaxy_create_graph(lgraph_api_galaxy_t* galaxy, const char* graph_name,
                                    const char* description, size_t max_size, char** errptr) {
    try {
        return galaxy->repr.CreateGraph(graph_name, description, max_size);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_delete_graph(lgraph_api_galaxy_t* galaxy, const char* graph_name,
                                    char** errptr) {
    try {
        return galaxy->repr.DeleteGraph(graph_name);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_mod_graph(lgraph_api_galaxy_t* galaxy, const char* graph_name, bool mod_desc,
                                 const char* desc, bool mod_size, size_t new_max_size,
                                 char** errptr) {
    try {
        return galaxy->repr.ModGraph(graph_name, mod_desc, desc, mod_size, new_max_size);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

size_t lgraph_api_galaxy_list_graphs(lgraph_api_galaxy_t* galaxy, char*** graph_names,
                                     char*** graph_descs, size_t** graph_sizes, char** errptr) {
    size_t n = 0;
    try {
        auto graphs = galaxy->repr.ListGraphs();
        n = graphs.size();
        *graph_names = (char**)malloc(sizeof(char*) * n);
        *graph_descs = (char**)malloc(sizeof(char*) * n);
        *graph_sizes = (size_t*)malloc(sizeof(size_t) * n);
        size_t i = 0;
        for (const auto& [name, desc_size] : graphs) {
            (*graph_names)[i] = strdup(name.c_str());
            (*graph_descs)[i] = strdup(desc_size.first.c_str());
            (*graph_sizes)[i] = desc_size.second;
            ++i;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

bool lgraph_api_galaxy_create_user(lgraph_api_galaxy_t* galaxy, const char* user,
                                   const char* password, const char* desc, char** errptr) {
    try {
        return galaxy->repr.CreateUser(user, password, desc);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_delete_user(lgraph_api_galaxy_t* galaxy, const char* user, char** errptr) {
    try {
        return galaxy->repr.DeleteUser(user);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_set_password(lgraph_api_galaxy_t* galaxy, const char* user,
                                    const char* old_password, const char* new_password,
                                    char** errptr) {
    try {
        return galaxy->repr.SetPassword(user, old_password, new_password);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_set_user_desc(lgraph_api_galaxy_t* galaxy, const char* user,
                                     const char* desc, char** errptr) {
    try {
        return galaxy->repr.SetUserDesc(user, desc);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_set_user_roles(lgraph_api_galaxy_t* galaxy, const char* user,
                                      const char* const* roles, size_t num_roles, char** errptr) {
    try {
        std::vector<std::string> roles_vec;
        for (size_t i = 0; i < num_roles; ++i) {
            roles_vec.emplace_back(roles[i]);
        }
        return galaxy->repr.SetUserRoles(user, roles_vec);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_set_user_graph_access(lgraph_api_galaxy_t* galaxy, const char* user,
                                             const char* graph, int access, char** errptr) {
    try {
        return galaxy->repr.SetUserGraphAccess(user, graph, static_cast<AccessLevel>(access));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_disable_user(lgraph_api_galaxy_t* galaxy, const char* user, char** errptr) {
    try {
        return galaxy->repr.DisableUser(user);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_enable_user(lgraph_api_galaxy_t* galaxy, const char* user, char** errptr) {
    try {
        return galaxy->repr.EnableUser(user);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

size_t lgraph_api_galaxy_list_users(lgraph_api_galaxy_t* galaxy, char*** user_names,
                                    lgraph_api_user_info_t*** user_infos, char** errptr) {
    size_t n = 0;
    try {
        auto users = galaxy->repr.ListUsers();
        n = users.size();
        *user_names = new char*[n];
        *user_infos = new lgraph_api_user_info_t*[n];
        size_t i = 0;
        for (const auto& [name, info] : users) {
            (*user_names)[i] = strdup(name.c_str());
            (*user_infos)[i] = new lgraph_api_user_info_t{info};
            ++i;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

void lgraph_api_galaxy_list_users_destroy(char** user_names, lgraph_api_user_info_t** user_infos,
                                          size_t num_users) {
    for (size_t i = 0; i < num_users; i++) {
        delete user_names[i];
        delete user_infos[i];
    }
    delete[] user_names;
    delete[] user_infos;
}

lgraph_api_user_info_t* lgraph_api_galaxy_get_user_info(lgraph_api_galaxy_t* galaxy,
                                                        const char* user, char** errptr) {
    try {
        return new lgraph_api_user_info_t{galaxy->repr.GetUserInfo(user)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

bool lgraph_api_galaxy_create_role(lgraph_api_galaxy_t* galaxy, const char* role, const char* desc,
                                   char** errptr) {
    try {
        return galaxy->repr.CreateRole(role, desc);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_delete_role(lgraph_api_galaxy_t* galaxy, const char* role, char** errptr) {
    try {
        return galaxy->repr.DeleteRole(role);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_disable_role(lgraph_api_galaxy_t* galaxy, const char* role, char** errptr) {
    try {
        return galaxy->repr.DisableRole(role);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_enable_role(lgraph_api_galaxy_t* galaxy, const char* role, char** errptr) {
    try {
        return galaxy->repr.EnableRole(role);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_set_role_desc(lgraph_api_galaxy_t* galaxy, const char* role,
                                     const char* desc, char** errptr) {
    try {
        return galaxy->repr.SetRoleDesc(role, desc);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_set_role_access_rights(lgraph_api_galaxy_t* galaxy, const char* role,
                                              const char* const* graph_names,
                                              const int* access_levels, size_t num_graphs,
                                              char** errptr) {
    try {
        std::map<std::string, AccessLevel> access_map;
        for (size_t i = 0; i < num_graphs; ++i) {
            access_map.emplace(graph_names[i], static_cast<AccessLevel>(access_levels[i]));
        }
        return galaxy->repr.SetRoleAccessRights(role, access_map);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

bool lgraph_api_galaxy_set_role_access_rights_incremental(lgraph_api_galaxy_t* galaxy,
                                                          const char* role,
                                                          const char* const* graph_names,
                                                          const int* access_levels,
                                                          size_t num_graphs, char** errptr) {
    try {
        std::map<std::string, AccessLevel> access_map;
        for (size_t i = 0; i < num_graphs; ++i) {
            access_map.emplace(graph_names[i], static_cast<AccessLevel>(access_levels[i]));
        }
        return galaxy->repr.SetRoleAccessRightsIncremental(role, access_map);
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return false;
    }
}

lgraph_api_role_info_t* lgraph_api_galaxy_get_role_info(lgraph_api_galaxy_t* galaxy,
                                                        const char* role, char** errptr) {
    try {
        return new lgraph_api_role_info_t{galaxy->repr.GetRoleInfo(role)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

size_t lgraph_api_galaxy_list_roles(lgraph_api_galaxy_t* galaxy, char*** role_names,
                                    lgraph_api_role_info_t*** role_info, char** errptr) {
    size_t n = 0;
    try {
        auto roles = galaxy->repr.ListRoles();
        n = roles.size();
        *role_names = new char*[n];
        *role_info = new lgraph_api_role_info_t*[n];
        size_t i = 0;
        for (const auto& [name, info] : roles) {
            (*role_names)[i] = strdup(name.c_str());
            (*role_info)[i] = new lgraph_api_role_info_t{info};
            ++i;
        }
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
    return n;
}

int lgraph_api_galaxy_get_access_level(lgraph_api_galaxy_t* galaxy, const char* user,
                                       const char* graph, char** errptr) {
    try {
        return static_cast<int>(galaxy->repr.GetAccessLevel(user, graph));
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return -1;
    }
}

lgraph_api_graph_db_t* lgraph_api_galaxy_open_graph(lgraph_api_galaxy_t* galaxy, const char* graph,
                                                    bool read_only, char** errptr) {
    try {
        return new lgraph_api_graph_db_t{galaxy->repr.OpenGraph(graph, read_only)};
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
        return nullptr;
    }
}

void lgraph_api_galaxy_close(lgraph_api_galaxy_t* galaxy, char** errptr) {
    try {
        galaxy->repr.Close();
    } catch (const std::exception& e) {
        *errptr = strdup(e.what());
    }
}
