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

#include <unordered_map>
#include "libcuckoo/cuckoohash_map.hh"

#include "core/lightning_graph.h"
#include "import/dense_string.h"

namespace lgraph {
namespace import_v2 {
typedef uint64_t VidType;

template <typename T>
inline size_t GetBucketId(const T& d, size_t n_buckets) {
    return d % n_buckets;
}

inline size_t GetBucketId(float d, size_t n_buckets) { return *(uint32_t*)&d % n_buckets; }

inline size_t GetBucketId(double d, size_t n_buckets) { return *(uint64_t*)&d % n_buckets; }

inline size_t GetBucketId(const std::string& str, size_t n_buckets) {
    uint64_t hash = 5381;
    size_t s = str.size();
    const uint8_t* p = (const uint8_t*)str.data();
    while (s--) hash = ((hash << 5) + hash) + *p++;
    return hash % n_buckets;
}

inline size_t GetBucketId(const DenseString& str, size_t n_buckets) {
    return str.hash2() % n_buckets;
}

template <FieldType FT>
struct FieldType2VidType {
    typedef typename ::lgraph::field_data_helper::FieldType2StorageType<FT>::type type;
};
template <>
struct FieldType2VidType<FieldType::STRING> {
    typedef DenseString type;
};
template <>
struct FieldType2VidType<FieldType::BLOB> {
    typedef DenseString type;
};

class VidTableInterface {
 public:
    virtual ~VidTableInterface() {}

    /**
     * Adds a key to the table
     *
     * @param          key  The key.
     * @param          vid  The vid.
     *
     * @returns True if the key does not exist, false if it already exists.
     */
    virtual bool AddKey(const FieldData& key, const VidType& vid) = 0;

    /**
     * Gets vid corresponding to the key
     *
     * @param          key  The key.
     * @param [out]    vid  The corresponding vid, if the key exists.
     *
     * @returns True if it succeeds, false if the key does not exist.
     */
    virtual bool GetVid(const FieldData& key, VidType& vid) = 0;
};

namespace _detail {
template <typename T>
struct MyHash {
    std::hash<T> hash_;
    size_t operator()(const T& d) const { return hash_(d); }
};

template <>
struct MyHash<DenseString> {
    size_t operator()(const DenseString& d) const { return d.hash1(); }
};
}  // namespace _detail

template <FieldType FT>
class SingleVidTable : public VidTableInterface {
    typedef typename FieldType2VidType<FT>::type KeyT;
    static const int N_MAPS = 13;  // use multiple map to reduce peak memory usage during rehash
    std::vector<cuckoohash_map<KeyT, VidType, _detail::MyHash<KeyT>>> maps_;

 public:
    SingleVidTable() : maps_(N_MAPS) {}

    /**
     * Adds a key to the table
     *
     * @param          key  The key.
     * @param          vid  The vid.
     *
     * @returns True if the key does not exist, false if it already exists.
     */
    bool AddKey(const FieldData& key, const VidType& vid) override {
        KeyT k(::lgraph::field_data_helper::GetStoredValue<FT>(key));
        return maps_[GetBucketId(k, N_MAPS)].insert(k, vid);
    }

    /**
     * Gets vid corresponding to the key
     *
     * @param          key  The key.
     * @param [out]    vid  The corresponding vid, if the key exists.
     *
     * @returns True if it succeeds, false if the key does not exist.
     */
    bool GetVid(const FieldData& key, VidType& vid) override {
        KeyT k(::lgraph::field_data_helper::GetStoredValue<FT>(key));
        return maps_[GetBucketId(k, N_MAPS)].find(k, vid);
    }

    std::vector<cuckoohash_map<KeyT, VidType, _detail::MyHash<KeyT>>>& GetTables() { return maps_; }
};

class AllVidTables {
    struct VidTableStartEndId {
        explicit VidTableStartEndId(VidTableInterface* table_ = nullptr, VidType start_id_ = 0)
            : table(table_), start_id(start_id_) {}

        std::unique_ptr<VidTableInterface> table;  // VidTable
        VidType start_id;                          // starting vertex id
        VidType end_id;                            // last vertex id + 1
    };
    std::map<std::string, VidTableStartEndId> label_map_;

 public:
    /**
     * Starts to build a vid table
     *
     * @param label     The label.
     * @param type      The type of unique id.
     * @param start_id  The first vertex id.
     */
    void StartVidTable(const std::string& label, FieldType type, VidType start_id) {
        if (label_map_.find(label) != label_map_.end()) {
            LOG_ERROR() << "Label [" << label << "] added more than once, current labels: "
                      << fma_common::ToString(GetLabels());
        }
        std::unique_ptr<VidTableInterface> table;
        switch (type) {
        case FieldType::NUL:
            FMA_ASSERT(false);
        case FieldType::BOOL:
            table.reset(new SingleVidTable<FieldType::BOOL>());
            break;
        case FieldType::INT8:
            table.reset(new SingleVidTable<FieldType::INT8>());
            break;
        case FieldType::INT16:
            table.reset(new SingleVidTable<FieldType::INT16>());
            break;
        case FieldType::INT32:
            table.reset(new SingleVidTable<FieldType::INT32>());
            break;
        case FieldType::INT64:
            table.reset(new SingleVidTable<FieldType::INT64>());
            break;
        case FieldType::FLOAT:
            table.reset(new SingleVidTable<FieldType::FLOAT>());
            break;
        case FieldType::DOUBLE:
            table.reset(new SingleVidTable<FieldType::DOUBLE>());
            break;
        case FieldType::DATE:
            table.reset(new SingleVidTable<FieldType::DATE>());
            break;
        case FieldType::DATETIME:
            table.reset(new SingleVidTable<FieldType::DATETIME>());
            break;
        case FieldType::STRING:
            table.reset(new SingleVidTable<FieldType::STRING>());
            break;
        case FieldType::BLOB:
            table.reset(new SingleVidTable<FieldType::BLOB>());
            break;
        default:
            FMA_ASSERT(false);
        }
        label_map_[label] = VidTableStartEndId(table.release(), start_id);
    }

    /**
     * Seals a vid table by setting its end id. The end id is one past the last
     * vertex id.
     *
     * @param label     The label.
     * @param end_id    Last vertex id plus one.
     */
    void SealVidTable(const std::string& label, VidType end_id) {
        auto it = label_map_.find(label);
        if (it == label_map_.end()) {
            LOG_ERROR() << "Label [" << label
                      << "] not found, current labels: " << fma_common::ToString(GetLabels());
        }
        it->second.end_id = end_id;
    }

    /**
     * Gets a vid table by label.
     *
     * @param label The label.
     *
     * @return  Null if it fails, else the vid table.
     */
    VidTableInterface* GetVidTable(const std::string& label) {
        auto it = label_map_.find(label);
        if (it == label_map_.end()) {
            return nullptr;
        }
        return it->second.table.get();
    }

    /**
     * Deletes the vid table by label
     *
     * @param label The label.
     */
    void DeleteVidTable(const std::string& label) {
        auto it = label_map_.find(label);
        if (it == label_map_.end()) {
            LOG_ERROR() << "Label [" << label
                      << "] not found, current labels: " << fma_common::ToString(GetLabels());
        }
        label_map_.erase(it);
    }

    VidType GetStartVid(const std::string& label) const {
        auto it = label_map_.find(label);
        if (it == label_map_.end()) {
            LOG_ERROR() << "Label [" << label
                      << "] not found, current labels: " << fma_common::ToString(GetLabels());
        }
        return it->second.start_id;
    }

    VidType GetEndVid(const std::string& label) const {
        auto it = label_map_.find(label);
        if (it == label_map_.end()) {
            LOG_ERROR() << "Label [" << label
                      << "] not found, current labels: " << fma_common::ToString(GetLabels());
        }
        return it->second.end_id;
    }

 protected:
    std::vector<std::string> GetLabels() const {
        std::vector<std::string> v;
        v.reserve(label_map_.size());
        for (auto& kv : label_map_) v.push_back(kv.first);
        return v;
    }
};
}  // namespace import_v2
}  // namespace lgraph
