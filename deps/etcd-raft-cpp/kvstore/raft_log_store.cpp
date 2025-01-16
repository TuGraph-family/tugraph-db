#include "raft_log_store.h"
#include <boost/endian/conversion.hpp>
#include "util.h"
#include "logger.h"
#define ASSERT(cond) if (!(cond)) std::abort()

std::string raft_log_key(uint64_t log_id) {
    std::string ret;
    boost::endian::native_to_big_inplace(log_id);
    ret.append((const char *)&log_id, sizeof(log_id));
    return ret;
}

std::string raft_hardstate_key() {
    return "hardState";
}

std::string raft_applyindex_key() {
    return "applyIndex";
}

std::string raft_confstate_key() {
    return "confState";
}

std::string raft_nodesinfo_key() {
    return "nodesInfo";
}

bool RaftLogStorage::Init() {
    std::string value;
    raftpb::HardState hardstate;
    raftpb::ConfState confstate;
    auto s = db_->Get(rocksdb::ReadOptions(), meta_cf_, raft_hardstate_key(), &value);
    ASSERT(s.ok() || s.IsNotFound());
    if (s.IsNotFound()) {
        rocksdb::WriteBatch batch;
        // dummy entry
        raftpb::Entry entry;
        entry.SerializeToString(&value);
        batch.Put(log_cf_, raft_log_key(0), value);
        // dummy hardstate
        hardstate.SerializeToString(&value);
        batch.Put(meta_cf_, raft_hardstate_key(), value);
        // dummy confstate
        confstate.SerializeToString(&value);
        batch.Put(meta_cf_, raft_confstate_key(), value);
        s = db_->Write(rocksdb::WriteOptions(), &batch);
        if (!s.ok()) {
            LOG_FATAL("failed to write db");
        }
        return false;
    }

    if (!hardstate.ParseFromString(value)) {
        LOG_FATAL("hardstate ParseFromString failed");
    }
    s = db_->Get(rocksdb::ReadOptions(), meta_cf_, raft_confstate_key(), &value);
    ASSERT(s.ok());
    if (!confstate.ParseFromString(value)) {
        LOG_FATAL("confstate ParseFromString failed");
    }
    LOG_INFO("Read raft state from db, hardstate:[{}], confstate:[{}]", hardstate.ShortDebugString(), confstate.ShortDebugString());
    first_entry_index_ = get_first_log_entry().index();
    last_entry_index_ = get_last_log_entry().index();
    conf_state_ = std::move(confstate);
    hard_state_ = std::move(hardstate);
    return last_entry_index_ > 0;
}

void RaftLogStorage::WriteBatch(rocksdb::WriteBatch &batch) {
    auto s = db_->Write(rocksdb::WriteOptions(), &batch);
    ASSERT(s.ok());
}

raftpb::Entry RaftLogStorage::get_first_log_entry() {
    std::string min_raft_log_key = raft_log_key(0);
    std::unique_ptr<rocksdb::Iterator> iter =
            std::unique_ptr<rocksdb::Iterator>(db_->NewIterator(rocksdb::ReadOptions(), log_cf_));
    iter->Seek(min_raft_log_key);
    ASSERT(iter->Valid());
    auto log = iter->value();
    raftpb::Entry entry;
    ASSERT(entry.ParseFromArray(log.data(), log.size()));
    return entry;
}

raftpb::Entry RaftLogStorage::get_last_log_entry() {
    std::string max_raft_log_key = raft_log_key(std::numeric_limits<uint64_t>::max());
    std::unique_ptr<rocksdb::Iterator> iter =
            std::unique_ptr<rocksdb::Iterator>(db_->NewIterator(rocksdb::ReadOptions(), log_cf_));
    iter->SeekForPrev(max_raft_log_key);
    ASSERT(iter->Valid());
    auto log = iter->value();
    raftpb::Entry entry;
    ASSERT(entry.ParseFromArray(log.data(), log.size()));
    return entry;
}
/*
eraft::Error RaftLogStorage::SetSnapshot(const raftpb::Snapshot& sp, rocksdb::WriteBatch &batch) {
    std::string val;
    std::string key = raft_snapshot_key();
    sp.SerializeToString(&val);
    batch.Put(meta_cf_, key, val);
    return nullptr;
}
*/
eraft::Error RaftLogStorage::SetHardState(const raftpb::HardState& hs, rocksdb::WriteBatch &batch) {
    std::string val;
    std::string key = raft_hardstate_key();
    hs.SerializeToString(&val);
    batch.Put(meta_cf_, key, val);
    return nullptr;
}

eraft::Error RaftLogStorage::SetConfState(const raftpb::ConfState& hs, rocksdb::WriteBatch &batch) {
    std::string val;
    std::string key = raft_confstate_key();
    hs.SerializeToString(&val);
    batch.Put(meta_cf_, key, val);
    return nullptr;
}

eraft::Error RaftLogStorage::SetNodesInfo(const std::string& info, rocksdb::WriteBatch &batch) {
    std::string val;
    std::string key = raft_nodesinfo_key();
    batch.Put(meta_cf_, key, info);
    return nullptr;
}

std::string RaftLogStorage::GetNodesInfo() {
    std::string key = raft_nodesinfo_key();
    std::string val;
    auto s = db_->Get(rocksdb::ReadOptions(), meta_cf_, key, &val);
    if (s.ok()) {
        return val;
    }
    if (s.IsNotFound()) {
        return "[]";
    }
    LOG_FATAL("Failed to get nodes info: {}", s.ToString());
}

eraft::Error RaftLogStorage::SetApplyIndex(uint64_t apply_index, rocksdb::WriteBatch &batch) {
    std::string val;
    std::string key = raft_applyindex_key();
    batch.Put(meta_cf_, key, std::to_string(apply_index));
    return nullptr;
}

uint64_t RaftLogStorage::GetApplyIndex() {
    uint64_t apply_index = 0;
    std::string key = raft_applyindex_key();
    std::string val;
    auto s = db_->Get(rocksdb::ReadOptions(), meta_cf_, key, &val);
    if (s.ok()) {
        apply_index = std::stoull(val);
    } else if (!s.IsNotFound()) {
        LOG_FATAL("get applyindex failed: {}", s.ToString());
    }
    return apply_index;
}


/*
eraft::Error RaftLogStorage::ApplySnapshot(raftpb::Snapshot snap, rocksdb::WriteBatch &batch) {
    //handle check for old snapshot being applied
    auto msIndex = snapshot_.metadata().index();
    auto snapIndex = snap.metadata().index();
    if (msIndex >= snapIndex) {
        return eraft::ErrSnapOutOfDate;
    }

    //delete all raft log
    batch.DeleteRange(log_cf_, raft_log_key(0),
                      raft_log_key(std::numeric_limits<uint64_t>::max()));

    //put a dummy raft log entry
    std::string v;
    raftpb::Entry e;
    e.set_term(snapshot_.metadata().term());
    e.set_index(snapshot_.metadata().index());
    e.SerializeToString(&v);
    batch.Put(log_cf_, raft_log_key(e.index()), v);

    // snapshot
    snap.SerializeToString(&v);
    batch.Put(meta_cf_, raft_snapshot_key(), v);

    first_entry_index_ = last_entry_index_ = e.index();
    snapshot_ = snap;
    auto s = db_->Write(rocksdb::WriteOptions(), &batch);
    ASSERT(s.ok());
    return nullptr;
}
*/

eraft::Error RaftLogStorage::Append(std::vector<raftpb::Entry> entries, rocksdb::WriteBatch &batch) {
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
        entries.erase(entries.begin(), entries.begin() + (int64_t)(first-entries[0].index()));
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

std::pair<std::vector<raftpb::Entry>, eraft::Error> RaftLogStorage::Entries(uint64_t lo, uint64_t hi, uint64_t maxSize) {
    if (lo <= first_entry_index_) {
        return {{}, eraft::ErrCompacted};
    }
    if (hi > lastIndex() + 1) {
        LOG_FATAL("entries' hi({}) is out of bound lastindex({})", hi, lastIndex());
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
        ASSERT(entry.ParseFromArray(v.data(), v.size()));
        ASSERT(entry.index() == next_index);
        next_index += 1;
        total_size += v.size();
        if (total_size > maxSize) {
            break;
        }
        ret.emplace_back(std::move(entry));
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
    ASSERT(s.ok());
    raftpb::Entry entry;
    ASSERT(entry.ParseFromString(val));
    return {entry.term(), nullptr};
}

std::pair<uint64_t, eraft::Error> RaftLogStorage::LastIndex() {
    return {lastIndex(), nullptr};
}

std::pair<uint64_t, eraft::Error> RaftLogStorage::FirstIndex() {
    return {firstIndex(), nullptr};
}

std::pair<raftpb::Snapshot, eraft::Error> RaftLogStorage::Snapshot() {
    // disable snapshot
    return {raftpb::Snapshot{}, eraft::ErrSnapshotTemporarilyUnavailable};
}