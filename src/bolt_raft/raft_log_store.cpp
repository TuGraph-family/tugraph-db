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

#include <boost/endian/conversion.hpp>
#include "tools/lgraph_log.h"
#include "fma-common/string_formatter.h"
#include "bolt_raft/raft_log_store.h"

namespace bolt_raft {
std::string raft_log_key(uint64_t log_id) {
    std::string ret;
    boost::endian::native_to_big_inplace(log_id);
    ret.append((const char *)&log_id, sizeof(log_id));
    return ret;
}

const char raft_hardstate_key[] = "hardState";
const char raft_applyindex_key[] = "applyIndex";
const char raft_confstate_key[] = "confState";
const char raft_nodeinfos_key[] = "nodeInfos";

bool RaftLogStorage::Init() {
    std::string value;
    raftpb::HardState hardstate;
    raftpb::ConfState confstate;
    auto s = db_->Get(rocksdb::ReadOptions(), meta_cf_, raft_hardstate_key, &value);
    if (s.IsNotFound()) {
        rocksdb::WriteBatch batch;
        // dummy entry
        raftpb::Entry entry;
        entry.SerializeToString(&value);
        batch.Put(log_cf_, raft_log_key(0), value);
        // dummy hardstate
        hardstate.SerializeToString(&value);
        batch.Put(meta_cf_, raft_hardstate_key, value);
        // dummy confstate
        confstate.SerializeToString(&value);
        batch.Put(meta_cf_, raft_confstate_key, value);
        s = db_->Write(rocksdb::WriteOptions(), &batch);
        if (!s.ok()) {
            LOG_FATAL() << "failed to write db in RaftLogStorage init, err: " << s.ToString();
        }
        return false;
    }
    if (!s.ok()) {
        LOG_FATAL() << "failed to get hardstate from db, err: " << s.ToString();
    }

    if (!hardstate.ParseFromString(value)) {
        LOG_FATAL() << "failed to parse HardState from string";
    }
    s = db_->Get(rocksdb::ReadOptions(), meta_cf_, raft_confstate_key, &value);
    if (!s.ok()) {
        LOG_FATAL() << "failed to get confstate from db: " << s.ToString();
    }
    if (!confstate.ParseFromString(value)) {
        LOG_FATAL() << "failed to parse ConfState from string";
    }
    first_entry_index_ = get_first_log_entry().index();
    last_entry_index_ = get_last_log_entry().index();
    conf_state_ = std::move(confstate);
    hard_state_ = std::move(hardstate);
    LOG_INFO() << FMA_FMT(
        "read raft state from db, first_index:{}, last_index:{}, hardstate:[{}], confstate:[{}]",
        first_entry_index_, last_entry_index_, hardstate.ShortDebugString(),
        confstate.ShortDebugString());
    return last_entry_index_ > 0;
}

void RaftLogStorage::Close() {
    db_->DestroyColumnFamilyHandle(log_cf_);
    db_->DestroyColumnFamilyHandle(meta_cf_);
    auto s = db_->Close();
    if (!s.ok()) {
        LOG_WARN() << FMA_FMT("failed to close db, error:{}", s.ToString());
    }
    delete db_;
    db_ = nullptr;
    LOG_INFO() << "raft log storage is closed";
}

void RaftLogStorage::Compact(uint64_t index) {
    if (index >= last_entry_index_) {
        LOG_FATAL() << FMA_FMT("compact raft log out of range, compact:{}, last:{}", index,
                               last_entry_index_);
    }
    auto min = raft_log_key(0);
    auto max = raft_log_key(index);
    auto s = db_->DeleteRange({}, log_cf_, min, max);
    if (!s.ok()) {
        LOG_ERROR() << FMA_FMT("failed to delete range, error:{}", s.ToString());
    }
    first_entry_index_ = index;
}

void RaftLogStorage::WriteBatch(rocksdb::WriteBatch &batch) {
    auto s = db_->Write(rocksdb::WriteOptions(), &batch);
    if (!s.ok()) {
        LOG_FATAL() << FMA_FMT("failed to write db: {}", s.ToString());
    }
}

raftpb::Entry RaftLogStorage::get_first_log_entry() {
    std::string min_raft_log_key = raft_log_key(0);
    std::unique_ptr<rocksdb::Iterator> iter =
        std::unique_ptr<rocksdb::Iterator>(db_->NewIterator(rocksdb::ReadOptions(), log_cf_));
    iter->Seek(min_raft_log_key);
    if (!iter->Valid()) {
        LOG_FATAL() << "failed to get first raft log key";
    }
    auto log = iter->value();
    raftpb::Entry entry;
    if (!entry.ParseFromArray(log.data(), log.size())) {
        LOG_FATAL() << "failed to parse raft log";
    }
    return entry;
}

raftpb::Entry RaftLogStorage::get_last_log_entry() {
    std::string max_raft_log_key = raft_log_key(std::numeric_limits<uint64_t>::max());
    std::unique_ptr<rocksdb::Iterator> iter =
        std::unique_ptr<rocksdb::Iterator>(db_->NewIterator(rocksdb::ReadOptions(), log_cf_));
    iter->SeekForPrev(max_raft_log_key);
    if (!iter->Valid()) {
        LOG_FATAL() << "failed to get last raft log key";
    }
    auto log = iter->value();
    raftpb::Entry entry;
    if (!entry.ParseFromArray(log.data(), log.size())) {
        LOG_FATAL() << "failed to parse raft log";
    }
    return entry;
}

eraft::Error RaftLogStorage::SetHardState(const raftpb::HardState &hs, rocksdb::WriteBatch &batch) {
    std::string val;
    hs.SerializeToString(&val);
    batch.Put(meta_cf_, raft_hardstate_key, val);
    return nullptr;
}

eraft::Error RaftLogStorage::SetConfState(const raftpb::ConfState &hs, rocksdb::WriteBatch &batch) {
    std::string val;
    hs.SerializeToString(&val);
    batch.Put(meta_cf_, raft_confstate_key, val);
    return nullptr;
}

eraft::Error RaftLogStorage::SetNodeInfos(const std::string &info, rocksdb::WriteBatch &batch) {
    std::string val;
    batch.Put(meta_cf_, raft_nodeinfos_key, info);
    return nullptr;
}

std::optional<std::string> RaftLogStorage::GetNodeInfos() {
    std::optional<std::string> ret;
    std::string val;
    auto s = db_->Get(rocksdb::ReadOptions(), meta_cf_, raft_nodeinfos_key, &val);
    if (s.ok()) {
        ret = val;
    } else if (!s.IsNotFound()) {
        LOG_FATAL() << FMA_FMT("failed to get nodes info: {}", s.ToString());
    }
    return ret;
}

eraft::Error RaftLogStorage::SetApplyIndex(uint64_t apply_index, rocksdb::WriteBatch &batch) {
    std::string val;
    batch.Put(meta_cf_, raft_applyindex_key, std::to_string(apply_index));
    return nullptr;
}

uint64_t RaftLogStorage::GetApplyIndex() {
    uint64_t apply_index = 0;
    std::string val;
    auto s = db_->Get(rocksdb::ReadOptions(), meta_cf_, raft_applyindex_key, &val);
    if (s.ok()) {
        apply_index = std::stoull(val);
    } else if (!s.IsNotFound()) {
        LOG_FATAL() << FMA_FMT("failed to get apply index: {}", s.ToString());
    }
    return apply_index;
}

eraft::Error RaftLogStorage::Append(std::vector<raftpb::Entry> entries,
                                    rocksdb::WriteBatch &batch) {
    if (entries.empty()) {
        return nullptr;
    }

    auto first = firstIndex();
    auto last = entries[0].index() + entries.size() - 1;

    // shortcut if there is no new entry.
    if (last < first) {
        return nullptr;
    }
    // truncate compacted entries
    if (first > entries[0].index()) {
        entries.erase(entries.begin(), entries.begin() + (int64_t)(first - entries[0].index()));
    }

    uint64_t pre_last_entry_index = last_entry_index_;
    std::string value;
    for (auto &entry : entries) {
        entry.SerializeToString(&value);
        batch.Put(log_cf_, raft_log_key(entry.index()), value);
        last_entry_index_ = entry.index();
    }
    for (uint64_t i = last_entry_index_ + 1; i <= pre_last_entry_index; i++) {
        batch.Delete(log_cf_, raft_log_key(i));
    }
    return nullptr;
}

std::tuple<raftpb::HardState, raftpb::ConfState, eraft::Error> RaftLogStorage::InitialState() {
    return {hard_state_, conf_state_, nullptr};
}

std::pair<std::vector<raftpb::Entry>, eraft::Error> RaftLogStorage::Entries(uint64_t lo,
                                                                            uint64_t hi,
                                                                            uint64_t maxSize) {
    if (lo <= first_entry_index_) {
        return {{}, eraft::ErrCompacted};
    }
    if (hi > lastIndex() + 1) {
        LOG_FATAL() << FMA_FMT("entries' hi({}) is out of bound lastindex({})", hi, lastIndex());
    }
    // only contains dummy entries.
    if (last_entry_index_ == first_entry_index_) {
        return {{}, eraft::ErrUnavailable};
    }

    uint64_t next_index = lo;
    uint64_t total_size = 0;
    std::vector<raftpb::Entry> ret;
    std::string start_key = raft_log_key(lo);
    std::string end_key = raft_log_key(hi);
    std::unique_ptr<rocksdb::Iterator> iter(db_->NewIterator(rocksdb::ReadOptions(), log_cf_));
    for (iter->Seek(start_key); iter->Valid(); iter->Next()) {
        auto k = iter->key();
        if (k.compare(rocksdb::Slice(end_key)) >= 0) {
            break;
        }
        auto v = iter->value();
        raftpb::Entry entry;
        if (!entry.ParseFromArray(v.data(), v.size())) {
            LOG_FATAL() << "failed to parse entry";
        }
        if (entry.index() != next_index) {
            LOG_FATAL() << "raft log index does not match";
        }
        next_index += 1;
        total_size += v.size();
        if (!ret.empty() && total_size > maxSize) {
            break;
        }
        ret.emplace_back(std::move(entry));
    }
    if (ret.empty()) {
        LOG_FATAL() << "unexpected empty Entry vector";
    }
    return {std::move(ret), nullptr};
}

std::pair<uint64_t, eraft::Error> RaftLogStorage::Term(uint64_t i) {
    if (i < first_entry_index_) {
        return {0, eraft::ErrCompacted};
    }
    if (i > last_entry_index_) {
        return {0, eraft::ErrUnavailable};
    }
    std::string val;
    auto s = db_->Get(rocksdb::ReadOptions(), log_cf_, raft_log_key(i), &val);
    if (!s.ok()) {
        LOG_FATAL() << FMA_FMT("failed to get term: {}", s.ToString());
    }
    raftpb::Entry entry;
    if (!entry.ParseFromString(val)) {
        LOG_FATAL() << "failed to parse entry";
    }
    return {entry.term(), nullptr};
}

std::pair<uint64_t, eraft::Error> RaftLogStorage::LastIndex() { return {lastIndex(), nullptr}; }

std::pair<uint64_t, eraft::Error> RaftLogStorage::FirstIndex() { return {firstIndex(), nullptr}; }

std::pair<raftpb::Snapshot, eraft::Error> RaftLogStorage::Snapshot() {
    // disable snapshot
    return {raftpb::Snapshot{}, eraft::ErrSnapshotTemporarilyUnavailable};
}
}  // namespace bolt_raft
