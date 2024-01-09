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

/**
 * This file contains the code to convert between ProtoBuf data structures and
 *LGraph data structures.
 **/

#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "db/acl.h"
#include "lgraph/lgraph_types.h"
#include "protobuf/ha.pb.h"

#include "lgraph_api/result_element.h"

#ifndef _WIN32
#include "cypher/resultset/record.h"
#endif

namespace lgraph {
struct FieldDataConvert {
    static inline FieldData ToLGraphT(ProtoFieldData&& fd) {
        switch (fd.Data_case()) {
        case ProtoFieldData::DATA_NOT_SET:
            return FieldData();
        case ProtoFieldData::kBoolean:
            return FieldData::Bool(fd.boolean());
        case ProtoFieldData::kInt8:
            return FieldData::Int8(fd.int8_());
        case ProtoFieldData::kInt16:
            return FieldData::Int16(fd.int16_());
        case ProtoFieldData::kInt32:
            return FieldData::Int32(fd.int32_());
        case ProtoFieldData::kInt64:
            return FieldData::Int64(fd.int64_());
        case ProtoFieldData::kSp:
            return FieldData::Float(fd.sp());
        case ProtoFieldData::kDp:
            return FieldData::Double(fd.dp());
        case ProtoFieldData::kVector:
            {
                std::vector<float> vec;
                std::string str(std::move(*fd.release_str()));
                std::regex pattern("-?[0-9]+\\.?[0-9]*");
                std::sregex_iterator begin_it(str.begin(), str.end(), pattern), end_it;
                while (begin_it != end_it) {  
                    std::smatch match = *begin_it;  
                    vec.push_back(std::stof(match.str()));  
                    ++begin_it; 
                }    
                return FieldData::Vector(vec);
            }          
        case ProtoFieldData::kDate:
            return FieldData::Date(Date(fd.date()));
        case ProtoFieldData::kDatetime:
            return FieldData::DateTime(DateTime(fd.datetime()));
        case ProtoFieldData::kStr:
            return FieldData::String(std::move(*fd.release_str()));
        case ProtoFieldData::kBlob:
            return FieldData::Blob(std::move(*fd.release_blob()));
        case ProtoFieldData::kPoint:
            return FieldData::Point(std::move(*fd.release_point()));
        case ProtoFieldData::kLinestring:
            return FieldData::LineString(std::move(*fd.release_linestring()));
        case ProtoFieldData::kPolygon:
            return FieldData::Polygon(std::move(*fd.release_polygon()));
        case ProtoFieldData::kSpatial:
            return FieldData::Spatial(std::move(*fd.release_polygon()));
        }
        FMA_ASSERT(false);
        return FieldData();
    }

    static inline FieldData ToLGraphT(const ProtoFieldData& fd) {
        switch (fd.Data_case()) {
        case ProtoFieldData::DATA_NOT_SET:
            return FieldData();
        case ProtoFieldData::kBoolean:
            return FieldData::Bool(fd.boolean());
        case ProtoFieldData::kInt8:
            return FieldData::Int8(fd.int8_());
        case ProtoFieldData::kInt16:
            return FieldData::Int16(fd.int16_());
        case ProtoFieldData::kInt32:
            return FieldData::Int32(fd.int32_());
        case ProtoFieldData::kInt64:
            return FieldData::Int64(fd.int64_());
        case ProtoFieldData::kSp:
            return FieldData::Float(fd.sp());
        case ProtoFieldData::kDp:
            return FieldData::Double(fd.dp());
        case ProtoFieldData::kVector:
            {
                std::vector<float> vec;
                std::string str(fd.str());
                std::regex pattern("-?[0-9]+\\.?[0-9]*");
                std::sregex_iterator begin_it(str.begin(), str.end(), pattern), end_it;
                while (begin_it != end_it) {  
                    std::smatch match = *begin_it;  
                    vec.push_back(std::stof(match.str()));  
                    ++begin_it; 
                }    
                return FieldData::Vector(vec);
            }
        case ProtoFieldData::kDate:
            return FieldData::Date(Date(fd.date()));
        case ProtoFieldData::kDatetime:
            return FieldData::DateTime(DateTime(fd.datetime()));
        case ProtoFieldData::kStr:
            return FieldData::String(fd.str());
        case ProtoFieldData::kBlob:
            return FieldData::Blob(fd.blob());
        case ProtoFieldData::kPoint:
            return FieldData::Point(fd.point());
        case ProtoFieldData::kLinestring:
            return FieldData::LineString(fd.linestring());
        case ProtoFieldData::kPolygon:
            return FieldData::Polygon(fd.polygon());
         case ProtoFieldData::kSpatial:
            return FieldData::Spatial(fd.spatial());
        }
        FMA_ASSERT(false);
        return FieldData();
    }

    static inline void FromLGraphT(FieldData&& fd, ProtoFieldData* ret) {
        switch (fd.type) {
        case FieldType::NUL:
            return ret->Clear();
        case FieldType::BOOL:
            return ret->set_boolean(fd.data.boolean);
        case FieldType::INT8:
            return ret->set_int8_(fd.data.int8);
        case FieldType::INT16:
            return ret->set_int16_(fd.data.int16);
        case FieldType::INT32:
            return ret->set_int32_(fd.data.int32);
        case FieldType::INT64:
            return ret->set_int64_(fd.data.int64);
        case FieldType::FLOAT:
            return ret->set_sp(fd.data.sp);
        case FieldType::DOUBLE:
            return ret->set_dp(fd.data.dp);
        case FieldType::VECTOR:
            return ret->set_vector(std::move(*fd.data.buf));
        case FieldType::DATE:
            return ret->set_date(fd.data.int32);
        case FieldType::DATETIME:
            return ret->set_datetime(fd.data.int64);
        case FieldType::STRING:
            return ret->set_str(std::move(*fd.data.buf));
        case FieldType::BLOB:
            return ret->set_blob(std::move(*fd.data.buf));
        case FieldType::POINT:
            return ret->set_point(std::move(*fd.data.buf));
        case FieldType::LINESTRING:
            return ret->set_linestring(std::move(*fd.data.buf));
        case FieldType::POLYGON:
            return ret->set_polygon(std::move(*fd.data.buf));
        case FieldType::SPATIAL:
            return ret->set_spatial(std::move(*fd.data.buf));
        }
        FMA_ASSERT(false);
    }

    static inline void FromLGraphT(const FieldData& fd, ProtoFieldData* ret) {
        switch (fd.type) {
        case FieldType::NUL:
            return ret->Clear();
        case FieldType::BOOL:
            return ret->set_boolean(fd.data.boolean);
        case FieldType::INT8:
            return ret->set_int8_(fd.data.int8);
        case FieldType::INT16:
            return ret->set_int16_(fd.data.int16);
        case FieldType::INT32:
            return ret->set_int32_(fd.data.int32);
        case FieldType::INT64:
            return ret->set_int64_(fd.data.int64);
        case FieldType::FLOAT:
            return ret->set_sp(fd.data.sp);
        case FieldType::DOUBLE:
            return ret->set_dp(fd.data.dp);
        case FieldType::VECTOR:
            return ret->set_vector(*fd.data.buf);
        case FieldType::DATE:
            return ret->set_date(fd.data.int32);
        case FieldType::DATETIME:
            return ret->set_datetime(fd.data.int64);
        case FieldType::STRING:
            return ret->set_str(*fd.data.buf);
        case FieldType::BLOB:
            return ret->set_blob(*fd.data.buf);
        case FieldType::POINT:
            return ret->set_point(*fd.data.buf);
        case FieldType::LINESTRING:
            return ret->set_linestring(*fd.data.buf);
        case FieldType::POLYGON:
            return ret->set_polygon(*fd.data.buf);
        case FieldType::SPATIAL:
            return ret->set_spatial(*fd.data.buf);
        }
        FMA_ASSERT(false);
    }

    static inline std::vector<FieldData> ToLGraphT(
        const ::google::protobuf::RepeatedPtrField<ProtoFieldData>& l) {
        std::vector<FieldData> ret;
        ret.reserve(l.size());
        for (auto& fd : l) ret.emplace_back(ToLGraphT(fd));
        return ret;
    }

    static inline void FromLGraphT(const std::vector<FieldData>& fds,
                                   ::google::protobuf::RepeatedPtrField<ProtoFieldData>* ret) {
        ret->Clear();
        FMA_DBG_ASSERT(fds.size() <= std::numeric_limits<int>::max());
        ret->Reserve(static_cast<int>(fds.size()));
        for (auto& fd : fds) FromLGraphT(fd, ret->Add());
    }

    static inline void FromLGraphT(std::vector<FieldData>&& fds,
                                   ::google::protobuf::RepeatedPtrField<ProtoFieldData>* ret) {
        ret->Clear();
        FMA_DBG_ASSERT(fds.size() <= std::numeric_limits<int>::max());
        ret->Reserve(static_cast<int>(fds.size()));
        for (auto& fd : fds) FromLGraphT(std::move(fd), ret->Add());
    }

#ifndef _WIN32
    static inline void FromLGraphT(
        const std::vector<std::pair<std::string, lgraph_api::LGraphType>>& header,
        const std::unordered_map<std::string, std::shared_ptr<lgraph_api::ResultElement>>& fds,
        ::google::protobuf::RepeatedPtrField<ProtoFieldData>* ret) {
        ret->Clear();
        FMA_DBG_ASSERT(fds.size() <= std::numeric_limits<int>::max());
        ret->Reserve(static_cast<int>(fds.size()));

        for (auto& h : header) {
            if (lgraph_api::LGraphTypeIsField(h.second) || lgraph_api::LGraphTypeIsAny(h.second)) {
                FromLGraphT(*fds.at(h.first)->v.fieldData, ret->Add());
            } else {
                FromLGraphT(FieldData(fds.at(h.first)->ToString()), ret->Add());
            }
        }
    }
#endif
};

struct FieldSpecConvert {
#define _CHECK_FIELD_TYPE_PROTO_SAME_VALUE_                                  \
    static_assert((int)FieldType::NUL == (int)ProtoFieldType::NUL,           \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::BOOL == (int)ProtoFieldType::BOOL,         \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::INT8 == (int)ProtoFieldType::INT8,         \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::INT16 == (int)ProtoFieldType::INT16,       \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::INT32 == (int)ProtoFieldType::INT32,       \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::INT64 == (int)ProtoFieldType::INT64,       \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::FLOAT == (int)ProtoFieldType::FLOAT,       \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::DOUBLE == (int)ProtoFieldType::DOUBLE,     \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::DATE == (int)ProtoFieldType::DATE,         \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::DATETIME == (int)ProtoFieldType::DATETIME, \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::STRING == (int)ProtoFieldType::STRING,     \
                  "FieldType and ProtoFieldSpec must have the same order."); \
    static_assert((int)FieldType::BLOB == (int)ProtoFieldType::BLOB,         \
                  "FieldType and ProtoFieldSpec must have the same order.");

    static inline FieldSpec ToLGraphT(ProtoFieldSpec&& fs) {
        _CHECK_FIELD_TYPE_PROTO_SAME_VALUE_;

        FieldSpec ret;
        ret.name = std::move(*fs.release_name());
        ret.optional = fs.nullable();
        ret.type = (FieldType)(fs.type());
        return ret;
    }

    static inline FieldSpec ToLGraphT(const ProtoFieldSpec& fs) {
        _CHECK_FIELD_TYPE_PROTO_SAME_VALUE_;

        FieldSpec ret;
        ret.name = fs.name();
        ret.optional = fs.nullable();
        ret.type = (FieldType)(fs.type());
        return ret;
    }

    static inline void FromLGraphT(FieldSpec&& fd, ProtoFieldSpec* ret) {
        _CHECK_FIELD_TYPE_PROTO_SAME_VALUE_;

        ret->set_name(std::move(fd.name));
        ret->set_nullable(fd.optional);
        ret->set_type((ProtoFieldType)(fd.type));
    }

    static inline void FromLGraphT(const FieldSpec& fd, ProtoFieldSpec* ret) {
        _CHECK_FIELD_TYPE_PROTO_SAME_VALUE_;

        ret->set_name(fd.name);
        ret->set_nullable(fd.optional);
        ret->set_type((ProtoFieldType)(fd.type));
    }

    static inline std::vector<FieldSpec> ToLGraphT(
        const ::google::protobuf::RepeatedPtrField<ProtoFieldSpec>& l) {
        std::vector<FieldSpec> ret;
        ret.reserve(l.size());
        for (auto& fd : l) ret.emplace_back(ToLGraphT(fd));
        return ret;
    }

    static inline void FromLGraphT(const std::vector<FieldSpec>& fds,
                                   ::google::protobuf::RepeatedPtrField<ProtoFieldSpec>* ret) {
        ret->Clear();
        FMA_DBG_ASSERT(fds.size() <= std::numeric_limits<int>::max());
        ret->Reserve(static_cast<int>(fds.size()));
        for (auto& fd : fds) FromLGraphT(fd, ret->Add());
    }

    static inline void FromLGraphT(std::vector<FieldSpec>&& fds,
                                   ::google::protobuf::RepeatedPtrField<ProtoFieldSpec>* ret) {
        ret->Clear();
        FMA_DBG_ASSERT(fds.size() <= std::numeric_limits<int>::max());
        ret->Reserve(static_cast<int>(fds.size()));
        for (auto& fd : fds) FromLGraphT(std::move(fd), ret->Add());
    }
};

namespace convert {
static inline DBConfig ToLGraphT(const ProtoDBConfig& conf) {
    DBConfig ret;
    ret.db_size = conf.db_size();
    ret.durable = !conf.async();
    ret.desc = conf.desc();
    return ret;
}

static inline void FromLGraphT(const DBConfig& from, ProtoDBConfig* to) {
    to->set_db_size(from.db_size);
    to->set_async(!from.durable);
    to->set_desc(from.desc);
}

static_assert((int)AccessLevel::NONE == (int)ProtoAccessLevel::NONE,
              "AccessLevel must have the same value as ProtoAccessLevel");
static_assert((int)AccessLevel::READ == (int)ProtoAccessLevel::READ_ONLY,
              "AccessLevel must have the same value as ProtoAccessLevel");
static_assert((int)AccessLevel::WRITE == (int)ProtoAccessLevel::READ_WRITE,
              "AccessLevel must have the same value as ProtoAccessLevel");
static_assert((int)AccessLevel::FULL == (int)ProtoAccessLevel::FULL,
              "AccessLevel must have the same value as ProtoAccessLevel");

static inline AccessLevel ToLGraphT(const ProtoAccessLevel& al) { return AccessLevel(al); }

static inline ProtoAccessLevel FromLGraphT(const AccessLevel& al) { return ProtoAccessLevel(al); }

static inline ProtoRoleInfo FromLGraphT(const AclManager::RoleInfo& info) {
    ProtoRoleInfo pri;
    pri.set_desc(info.desc);
    pri.set_is_disabled(info.disabled);
    auto* access = pri.mutable_graph_access()->mutable_values();
    for (auto& kv : info.graph_access) (*access)[kv.first] = FromLGraphT(kv.second);
    return pri;
}

static inline ProtoUserInfo FromLGraphT(const AclManager::UserInfo& info) {
    ProtoUserInfo pui;
    pui.set_is_disabled(info.disabled);
    pui.set_auth_method(info.auth_method);
    for (auto& r : info.roles) pui.mutable_roles()->Add()->assign(r);
    return pui;
}

static_assert((int)lgraph_api::GraphQueryType::CYPHER == (int)ProtoGraphQueryType::CYPHER,
              "GraphQueryType must have the same value as ProtoGraphQueryType");
static_assert((int)lgraph_api::GraphQueryType::GQL == (int)ProtoGraphQueryType::GQL,
              "GraphQueryType must have the same value as ProtoGraphQueryType");

static inline lgraph_api::GraphQueryType ToLGraphT(const ProtoGraphQueryType& al) {
    return lgraph_api::GraphQueryType(al);
}

static inline ProtoGraphQueryType FromLGraphT(const lgraph_api::GraphQueryType& al) {
    return ProtoGraphQueryType(al);
}

}  // namespace convert
}  // namespace lgraph
