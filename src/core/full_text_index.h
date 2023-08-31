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
#if LGRAPH_ENABLE_FULLTEXT_INDEX
#include <jni.h>
#endif
#include <string>
#include <vector>
#include "core/data_type.h"

namespace lgraph {

class FTIndexException : public InternalError {
 public:
    explicit FTIndexException(const std::string& str) : InternalError("FTIndexException: " + str) {}
};

enum class FTIndexEntryType { ADD_VERTEX = 0, ADD_EDGE, DELETE_VERTEX, DELETE_EDGE };

struct FTIndexEntry {
    FTIndexEntryType type;
    VertexId vid1;
    VertexId vid2;
    LabelId lid;
    TemporalId tid;
    EdgeId eid;
    std::vector<std::pair<std::string, std::string>> kvs;
};
#if LGRAPH_ENABLE_FULLTEXT_INDEX
struct LuceneJNIEnv {
    static JavaVM* vm;
    static std::once_flag once_flag;
    JNIEnv* env = nullptr;
    bool need_detach = false;
    jfieldID vid;
    jfieldID vertexScore;
    jfieldID srcId;
    jfieldID destId;
    jfieldID edgeId;
    jfieldID labelId;
    jfieldID edgeScore;

    DISABLE_COPY(LuceneJNIEnv);
    LuceneJNIEnv();
    ~LuceneJNIEnv();
    static void InitJniEnv();
    void CheckException(const std::string& call_method);
};
#endif

#if LGRAPH_ENABLE_FULLTEXT_INDEX
class FullTextIndex {
 public:
    explicit FullTextIndex(const std::string& path, const std::string& analyzer,
                           int commit_interval, int refresh_interval);
    ~FullTextIndex();
    void AddVertex(int64_t vid, LabelId labelId,
                   const std::vector<std::pair<std::string, std::string>>& kvs);
    void AddEdge(const EdgeUid& euid, const std::vector<std::pair<std::string, std::string>>& kvs);
    void DeleteVertex(int64_t vid);
    void DeleteEdge(const EdgeUid& euid);
    void DeleteLabel(bool is_vertex, LabelId labelId);
    std::vector<std::pair<int64_t, float>> QueryVertex(LabelId lid, const std::string& query,
                                                       int top_n);
    std::vector<std::pair<EdgeUid, float>> QueryEdge(LabelId lid, const std::string& query,
                                                     int top_n);
    void Clear();
    void Refresh();
    void Commit();
    void Backup(const std::string& backup_dir);
    int commit_interval() const { return commit_interval_; }
    int refresh_interval() const { return refresh_interval_; }

 private:
    static thread_local LuceneJNIEnv jni_env_;
    std::string path_;
    int commit_interval_;
    int refresh_interval_;
    jobject g_lucene_;
};
#else
class FullTextIndex {
 public:
    explicit FullTextIndex(const std::string& path, const std::string& analyzer,
                           int commit_interval, int refresh_interval) {}
    void AddVertex(int64_t vid, LabelId labelId,
                   const std::vector<std::pair<std::string, std::string>>& kvs) {}
    void AddEdge(const EdgeUid& euid,
        const std::vector<std::pair<std::string, std::string>>& kvs) {
    }
    void DeleteVertex(int64_t vid) {}
    void DeleteEdge(const EdgeUid& euid) {}
    void DeleteLabel(bool is_vertex, LabelId labelId) {}
    std::vector<std::pair<int64_t, float>> QueryVertex(LabelId lid, const std::string& query,
                                                       int top_n) {
        return std::vector<std::pair<int64_t, float>>();
    }
    std::vector<std::pair<EdgeUid, float>> QueryEdge(LabelId lid, const std::string& query,
                                                     int top_n) {
        return std::vector<std::pair<EdgeUid, float>>();
    }
    void Clear() {}
    void Refresh() {}
    void Commit() {}
    void Backup(const std::string& backup_dir) {}

    int commit_interval() const { return 0; }
    int refresh_interval() const { return 0; }
};
#endif

}  // namespace lgraph

