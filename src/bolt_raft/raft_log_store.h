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

// written by botu.wzy

#pragma once
#include <rocksdb/db.h>
#include <rocksdb/convenience.h>
#include <boost/noncopyable.hpp>
#include "etcd-raft-cpp/rawnode.h"
namespace bolt_raft {
struct RaftLogStorage : private boost::noncopyable, eraft::Storage {
 public:
    RaftLogStorage(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* log_cf,
                   rocksdb::ColumnFamilyHandle* meta_cf)
        : db_(db), log_cf_(log_cf), meta_cf_(meta_cf) {}

    std::tuple<raftpb::HardState, raftpb::ConfState, eraft::Error> InitialState() override;
    std::pair<std::vector<raftpb::Entry>, eraft::Error> Entries(uint64_t lo, uint64_t hi,
                                                                uint64_t maxSize) override;
    std::pair<uint64_t, eraft::Error> Term(uint64_t i) override;
    std::pair<uint64_t, eraft::Error> LastIndex() override;
    std::pair<uint64_t, eraft::Error> FirstIndex() override;
    std::pair<raftpb::Snapshot, eraft::Error> Snapshot() override;

    bool Init();
    void Close();
    void Compact(uint64_t index);
    eraft::Error SetHardState(const raftpb::HardState& hs, rocksdb::WriteBatch& batch);
    eraft::Error SetConfState(const raftpb::ConfState& hs, rocksdb::WriteBatch& batch);
    eraft::Error SetNodeInfos(const std::string& info, rocksdb::WriteBatch& batch);
    std::optional<std::string> GetNodeInfos();
    eraft::Error SetApplyIndex(uint64_t apply_index, rocksdb::WriteBatch& batch);
    uint64_t GetApplyIndex();
    eraft::Error Append(std::vector<raftpb::Entry> entries, rocksdb::WriteBatch& batch);
    void WriteBatch(rocksdb::WriteBatch& batch);

 private:
    uint64_t firstIndex() const { return first_entry_index_ + 1; }
    uint64_t lastIndex() const { return last_entry_index_; }
    raftpb::Entry get_first_log_entry();
    raftpb::Entry get_last_log_entry();

    rocksdb::DB* db_ = nullptr;
    rocksdb::ColumnFamilyHandle* log_cf_ = nullptr;
    rocksdb::ColumnFamilyHandle* meta_cf_ = nullptr;
    uint64_t first_entry_index_ = 0;
    uint64_t last_entry_index_ = 0;
    raftpb::HardState hard_state_;
    raftpb::ConfState conf_state_;
};
}  // namespace bolt_raft
