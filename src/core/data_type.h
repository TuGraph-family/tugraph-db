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

#include <cstdint>
#include <exception>
#include <limits>
#include <type_traits>

#include "fma-common/binary_read_write_helper.h"
#include "fma-common/string_formatter.h"
#include "fma-common/text_parser.h"

#include "tiny-process-library/process.hpp"

#include "core/defs.h"
#include "core/task_tracker.h"
#include "core/value.h"
#include "lgraph/lgraph_date_time.h"
#include "lgraph/lgraph_types.h"
#include "lgraph/lgraph_result.h"

namespace lgraph {
typedef lgraph_api::FieldType FieldType;
typedef lgraph_api::FieldData FieldData;
typedef lgraph_api::IndexType IndexType;
typedef lgraph_api::IndexSpec IndexSpec;
typedef lgraph_api::CompositeIndexType CompositeIndexType;
typedef lgraph_api::CompositeIndexSpec CompositeIndexSpec;
typedef lgraph_api::VectorIndexSpec VectorIndexSpec;
typedef lgraph_api::FieldSpec FieldSpec;
typedef lgraph_api::EdgeUid EdgeUid;
typedef lgraph_api::Date Date;
typedef lgraph_api::DateTime DateTime;
typedef lgraph_api::Point<lgraph_api::Wgs84> PointWgs84;
typedef lgraph_api::Point<lgraph_api::Cartesian> PointCartesian;
typedef lgraph_api::LineString<lgraph_api::Wgs84> LineStringWgs84;
typedef lgraph_api::LineString<lgraph_api::Cartesian> LineStringCartesian;
typedef lgraph_api::Polygon<lgraph_api::Wgs84> PolygonWgs84;
typedef lgraph_api::Polygon<lgraph_api::Cartesian> PolygonCartesian;
typedef lgraph_api::Spatial<lgraph_api::Wgs84> SpatialWgs84;
typedef lgraph_api::Spatial<lgraph_api::Cartesian> SpatialCartesian;
typedef lgraph_api::LGraphType ElementType;
typedef lgraph_api::Result Result;
typedef lgraph_api::EdgeConstraints EdgeConstraints;
typedef lgraph_api::LabelOptions LabelOptions;
typedef lgraph_api::VertexOptions VertexOptions;
typedef lgraph_api::EdgeOptions EdgeOptions;
typedef lgraph_api::EdgeOptions::TemporalFieldOrder TemporalFieldOrder;


typedef int64_t VertexId;
typedef int64_t EdgeId;
typedef int32_t DataOffset;      // offset used in a record
typedef int32_t PackDataOffset;  // offset used in a packed data (maximum 1024)
typedef uint16_t LabelId;
typedef int64_t TemporalId;

typedef uint16_t FieldId;   // Field id in schema Fields
typedef uint8_t VersionId;  // Schema version

enum CompareOp { LBR_EQ = 0, LBR_NEQ = 1, LBR_LT = 2, LBR_LE = 3, LBR_GT = 4, LBR_GE = 5 };

enum LogicalOp { LBR_EMPTY = 0, LBR_AND = 1, LBR_OR = 2, LBR_NOT = 3, LBR_XOR = 4 };

//===============================
// Similar to EdgeUid, but lack eid
//===============================
struct EdgeSid {
    EdgeSid() : src(0), dst(0), lid(0), tid(0) {}
    EdgeSid(int64_t s, int64_t d, uint16_t l, int64_t t) : src(s), dst(d), lid(l), tid(t) {}

    int64_t src;
    int64_t dst;
    uint16_t lid;
    int64_t tid;

    void Reverse() { std::swap(src, dst); }

    inline bool operator==(const EdgeSid& rhs) const {
        return src == rhs.src && dst == rhs.dst && lid == rhs.lid && tid == rhs.tid;
    }

    EdgeUid ConstructEdgeUid(EdgeId eid) const { return {src, dst, lid, tid, eid}; }

    std::string ToString() const {
        return std::to_string(src) + "_" + std::to_string(dst) + "_" + std::to_string(lid) + "_" +
               std::to_string(tid);
    }
};

//===============================
// Exceptions
//===============================

class InternalError : public std::exception {
 private:
    std::string err_;

 public:
    explicit InternalError(const std::string& msg) {
        err_ = msg;
    }

    template <typename... Ts>
    InternalError(const char* format, const Ts&... ds) {
        FMA_FMT(err_, format, ds...);
    }

    const char* what() const noexcept override { return err_.c_str(); }
};

//===============================
// Id types and DBConfig
//===============================
inline std::string PrintNestedException(const std::exception& e, int level = 0) {
    std::string ret(level * 2, ' ');
    ret.append(e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& e) {
        ret.append("\n" + PrintNestedException(e, level + 1));
    } catch (...) {
    }
    return ret;
}

struct FullTextIndexOptions {
    bool enable_fulltext_index = false;
    std::string fulltext_analyzer = "StandardAnalyzer";
    int fulltext_commit_interval = 60;
    int fulltext_refresh_interval = 5;
};

struct DBConfig {
    std::string name;
    std::string desc;
    std::string dir;
#ifdef USE_VALGRIND
    size_t db_size = (size_t)1 << 30;
#else
    size_t db_size = _detail::DEFAULT_GRAPH_SIZE;
#endif
    bool durable = false;
    size_t subprocess_max_idle_seconds = 600;

    // whether to load plugins on startup
    bool load_plugins = true;
    bool create_if_not_exist = true;
    bool enable_wal = false;

    FullTextIndexOptions ft_index_options;
    bool enable_realtime_count = true;

    template <typename StreamT>
    size_t Serialize(StreamT& stream) const {
        // db_async and subprocess_max_idle_seconds are configured
        // globally, so we don't store them in DB
        return fma_common::BinaryWrite(stream, name) + fma_common::BinaryWrite(stream, desc) +
               fma_common::BinaryWrite(stream, dir) + fma_common::BinaryWrite(stream, db_size);
    }

    template <typename StreamT>
    size_t Deserialize(StreamT& stream) {
        return fma_common::BinaryRead(stream, name) + fma_common::BinaryRead(stream, desc) +
               fma_common::BinaryRead(stream, dir) + fma_common::BinaryRead(stream, db_size);
    }

    std::string ToString() const {
        return fma_common::StringFormatter::Format("name: {}, desc:{}, dir: {}, db_size: {}", name,
                                                   desc, dir, db_size);
    }
};

class AutoCleanupAction {
    std::function<void(void)> action_;
    DISABLE_COPY(AutoCleanupAction);

 public:
    explicit AutoCleanupAction(const std::function<void(void)>& action) : action_(action) {}

    ~AutoCleanupAction() {
        if (action_) action_();
    }

    AutoCleanupAction(AutoCleanupAction&& rhs) {
        action_ = rhs.action_;
        rhs.action_ = nullptr;
    }

    AutoCleanupAction& operator=(AutoCleanupAction&& rhs) {
        if (this == &rhs) return *this;
        action_ = rhs.action_;
        rhs.action_ = nullptr;
        return *this;
    }

    void Reset(const std::function<void(void)>& action) { action_ = action; }

    void Cancel() { action_ = nullptr; }

    void Do() {
        if (action_) action_();
        action_ = nullptr;
    }
};

class CleanupActions {
    std::deque<AutoCleanupAction> actions_;
    DISABLE_COPY(CleanupActions);

 public:
    CleanupActions() {}

    void Push(AutoCleanupAction&& rhs) { actions_.emplace_back(std::move(rhs)); }

    template <typename... Ts>
    void Emplace(Ts&&... d) {
        actions_.emplace_back(std::forward<Ts>(d)...);
    }

    ~CleanupActions() {
        for (auto it = actions_.rbegin(); it != actions_.rend(); it++) it->Do();
    }

    void CancelAll() {
        for (auto& act : actions_) act.Cancel();
        actions_.clear();
    }
};

class AutoCleanDir {
    std::string dir_;

    DISABLE_COPY(AutoCleanDir);

 public:
    explicit AutoCleanDir(const std::string& dir, bool create_empty = false);

    AutoCleanDir(AutoCleanDir&& rhs) : dir_(std::move(rhs.dir_)) {}

    void Clean();

    void ReCreate();

    ~AutoCleanDir();
};

class SubProcess {
    std::unique_ptr<TinyProcessLib::Process> proc_;
    int exit_code_ = 0;

    std::condition_variable out_cv_;
    std::mutex out_mtx_;
    std::string stdout_;
    std::string stderr_;

 public:
    explicit SubProcess(const std::string& cmd, bool print_output = true);

    ~SubProcess();

    bool ExpectOutput(const std::string& pattern, size_t n_milliseconds = 0);

    void Kill();

    bool Wait(size_t n_milliseconds = 0);

    int GetExitCode() const;

    bool CheckIsAlive();

    std::string Stdout();

    std::string Stderr();
};

//===============================
// Id helpers
//===============================
namespace _detail {
static const size_t VID_SIZE = 5;
static const size_t EID_SIZE = 4;
static const size_t LID_BEGIN = 10;
static const size_t TID_BEGIN = 12;
static const size_t EID_BEGIN = 20;
static const size_t EUID_SIZE = 24;
static const size_t LID_SIZE = sizeof(LabelId);
static const size_t TID_SIZE = sizeof(TemporalId);
// maximum label id. -1 is reserved, so maximum is 65534
static const int64_t MAX_VID = (((int64_t)1) << (VID_SIZE * 8)) - 2;
static const int64_t MAX_EID = 0xffffff;  // 3 bytes
static const int64_t MAX_TID = std::numeric_limits<TemporalId>::max() - 2;
static const LabelId MAX_LID = std::numeric_limits<LabelId>::max() - 2;
static const size_t NODE_SPLIT_THRESHOLD = 1000;
static const size_t MAX_PROP_SIZE = ((size_t)16 << 20) - 1;
static const size_t MAX_STRING_SIZE = ((size_t)4 << 20) - 1;
static const size_t MAX_IN_PLACE_BLOB_SIZE = 512;
static const size_t MAX_BLOB_SIZE = ((size_t)1 << 32) - 1;
static const size_t MAX_KEY_SIZE = 480;
static const size_t MAX_HOST_ADDR_LEN = 256;
static const uint8_t SCHEMA_VERSION = 0;

template <size_t NBYTE>
inline int64_t GetNByteIdFromBuf(const char* p) {
    int64_t id = 0;
#if BYTE_ORDER == LITTLE_ENDIAN
    memcpy((char*)&id, p, NBYTE);
#else
    memcpy(((char*)&id) + sizeof(int64_t) - NBYTE, p, NBYTE);
#endif
    return id;
}

inline int64_t GetNByteIdFromBuf(const char* p, size_t n_bytes) {
    int64_t id = 0;
#if BYTE_ORDER == LITTLE_ENDIAN
    memcpy((char*)&id, p, n_bytes);
#else
    memcpy(((char*)&id) + sizeof(int64_t) - n_bytes, p, n_bytes);
#endif
    return id;
}

template <size_t NBYTE>
inline void SetNByteIdToBuf(char* p, int64_t id) {
#if BYTE_ORDER == LITTLE_ENDIAN
    memcpy(p, &id, NBYTE);
#else
    memcpy(p, ((char*)&id) + sizeof(int64_t) - NBYTE, NBYTE);
#endif
}

inline void SetNByteIdToBuf(char* p, size_t n_bytes, int64_t id) {
#if BYTE_ORDER == LITTLE_ENDIAN
    memcpy(p, &id, n_bytes);
#else
    memcpy(p, ((char*)&id) + sizeof(int64_t) - n_bytes, n_bytes);
#endif
}

inline VertexId GetVid(const char* p) { return GetNByteIdFromBuf<VID_SIZE>(p); }

inline EdgeId GetEid(const char* p) { return GetNByteIdFromBuf<EID_SIZE>(p); }

inline LabelId GetLabelId(const char* p) {
    LabelId lid;
    memcpy(&lid, p, LID_SIZE);
    return lid;
}

inline TemporalId GetTemporalId(const char* p) { return GetNByteIdFromBuf<TID_SIZE>(p); }

inline void WriteVid(char* p, VertexId id) { SetNByteIdToBuf<VID_SIZE>(p, id); }

inline void WriteEid(char* p, EdgeId id) { SetNByteIdToBuf<EID_SIZE>(p, id); }

inline void WriteLabelId(char* p, LabelId id) { memcpy(p, &id, LID_SIZE); }

inline void WriteTemporalId(char* p, TemporalId id) { SetNByteIdToBuf<TID_SIZE>(p, id); }

inline PackDataOffset GetOffset(const char* offset_array, size_t i) {
    PackDataOffset r;
    memcpy(&r, offset_array + sizeof(PackDataOffset) * i, sizeof(PackDataOffset));
    return r;
}

inline void SetOffset(char* offset_array, size_t i, size_t off) {
    PackDataOffset r = static_cast<PackDataOffset>(off);
    memcpy(offset_array + sizeof(PackDataOffset) * i, &r, sizeof(PackDataOffset));
}

inline void CheckVid(VertexId vid) {
    if (vid < 0 || vid > ::lgraph::_detail::MAX_VID) {
        THROW_CODE(InputError, "vertex id out of range: must be a number between 0 and {}",
                   ::lgraph::_detail::MAX_VID);
    }
}

inline void CheckEid(EdgeId eid) {
    if (eid < 0 || eid > ::lgraph::_detail::MAX_EID) {
        THROW_CODE(InputError, "edge id out of range: must be a number between 0 and {}",
                   ::lgraph::_detail::MAX_EID);
    }
}

inline void CheckTid(TemporalId tid) {
    if (tid > ::lgraph::_detail::MAX_TID) {
        THROW_CODE(InputError, "edge id out of range: must be a number between 0 and {}",
                   ::lgraph::_detail::MAX_TID);
    }
}

inline void CheckLid(size_t lid) {
    if (lid > ::lgraph::_detail::MAX_LID) {
        THROW_CODE(InputError, "label id out of range: must be a number between 0 and {}",
                   ::lgraph::_detail::MAX_LID);
    }
}

inline void CheckEdgeUid(const EdgeUid& euid) {
    _detail::CheckVid(euid.src);
    _detail::CheckLid(euid.lid);
    _detail::CheckVid(euid.dst);
    _detail::CheckTid(euid.tid);
    _detail::CheckEid(euid.eid);
}
}  // namespace _detail
}  // namespace lgraph

