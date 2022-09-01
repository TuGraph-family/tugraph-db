/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/file_system.h"

#include "lmdb/lmdb.h"
#include "core/kv_store_exception.h"
#include "core/wal.h"

namespace lgraph {

namespace _wal {
static const int MAGIC_NUM = 20220708;
static const char* WAL_FILE_PREFIX = "wal.log.";
static const char* WAL_DBI_FILE_PREFIX = "dbi.log";
};

enum WalOpType {
    TXN_BEGIN = 0,
    TXN_COMMIT = 1,
    TXN_ABORT = 2,
    KV_PUT = 3,
    KV_DEL = 4,
    TABLE_DROP = 5,
    TABLE_OPEN = 6,
    INVALID_ENTRY = 255
};

template <typename T>
void LogToFile(SyncFile &of, const T &d) {
    of.Write((const char*)&d, sizeof(d));
}

template<>
void LogToFile<std::string>(SyncFile& of, const std::string& str) {
    LogToFile(of, str.size());
    of.Write(str.data(), str.size());
}

template<>
void LogToFile<Value>(SyncFile &of, const Value& v) {
    LogToFile(of, v.Size());
    of.Write(v.Data(), v.Size());
}

template<typename T>
inline T ReadT(std::ifstream &in) {
    T ret{};
    auto &r = in.read((char *)&ret, sizeof(T));
    if (!r) throw KvException("in.read failed");
    return ret;
}

template<>
inline std::string ReadT<std::string>(std::ifstream& in) {
    size_t s = ReadT<size_t>(in);
    std::string ret;
    ret.resize(s);
    auto &r = in.read(&ret[0], s);
    if (!r) throw KvException("in.read failed");
    return ret;
}

template <>
inline Value ReadT<Value>(std::ifstream &in) {
    size_t s = ReadT<size_t>(in);
    Value ret;
    ret.Resize(s);
    auto &r = in.read(ret.Data(), s);
    if (!r) throw KvException("in.read failed");
    return ret;
}

template<>
void LogToFile<ComparatorDesc>(SyncFile &of, const ComparatorDesc &d) {
    LogToFile(of, d.comp_type);
    LogToFile(of, d.data_type);
}

template<>
inline ComparatorDesc ReadT<ComparatorDesc>(std::ifstream &in) {
    ComparatorDesc ret;
    ret.comp_type = ReadT<decltype(ret.comp_type)>(in);
    ret.data_type = ReadT<decltype(ret.data_type)>(in);
    return ret;
}

/*
 * Log format:
 * [MAGIC_NUM] [TXN_ID] [OP_ID] [OP_TYPE] [OP PARAMS]
 *
 * [OP PARAMS]:
 * txn_begin:    [bool is_child]
 * txn_abort:    [bool is_child]
 * table_open:   [uint dbi] [string name] [comparator_desc]
 * table_drop:   [uint dbi]
 * kv_put:       [uint dbi] [key] [value]
 * kv_del:       [uint dbi] [key]
 */
struct LogEntry {
    mdb_size_t txn_id;
    int64_t op_id;
    WalOpType op_type = WalOpType::INVALID_ENTRY;
    bool is_child;
    MDB_dbi dbi;
    void *content = nullptr;    // different for differnt types
    typedef std::pair<std::string, ComparatorDesc> TableOpenContent;
    typedef std::pair<Value, Value> KvPutContent;
    typedef Value KeyDelContent;

    DISABLE_COPY(LogEntry);

    LogEntry() : content(nullptr) {
    }

    LogEntry(LogEntry &&rhs) {
        txn_id = rhs.txn_id;
        op_id = rhs.op_id;
        op_type = rhs.op_type;
        rhs.op_type = WalOpType::INVALID_ENTRY;
        is_child = rhs.is_child;
        dbi = rhs.dbi;
        content = rhs.content;
        rhs.content = nullptr;
    }

    LogEntry &operator=(LogEntry &&rhs) {
        DeleteContent();
        txn_id = rhs.txn_id;
        op_id = rhs.op_id;
        op_type = rhs.op_type;
        rhs.op_type = WalOpType::INVALID_ENTRY;
        is_child = rhs.is_child;
        dbi = rhs.dbi;
        content = rhs.content;
        rhs.content = nullptr;
        return *this;
    }

    ~LogEntry() {
        DeleteContent();
    }

    static void WriteHeader(SyncFile &of, mdb_size_t txn_id, size_t op_id, WalOpType op_type) {
        LogToFile(of, _wal::MAGIC_NUM);
        LogToFile(of, txn_id);
        LogToFile(of, op_id);
        LogToFile(of, op_type);
    }

    static void LogTxnBegin(SyncFile &of, mdb_size_t txn_id, size_t op_id, bool is_child) {
        WriteHeader(of, txn_id, op_id, WalOpType::TXN_BEGIN);
        LogToFile(of, is_child);
    }

    static void LogTxnCommit(SyncFile &of, mdb_size_t txn_id, size_t op_id, bool is_child) {
        WriteHeader(of, txn_id, op_id, WalOpType::TXN_COMMIT);
        LogToFile(of, is_child);
    }

    static void LogTxnAbort(SyncFile &of, mdb_size_t txn_id, size_t op_id, bool is_child) {
        WriteHeader(of, txn_id, op_id, WalOpType::TXN_ABORT);
        LogToFile(of, is_child);
    }

    static void LogTableOpen(
        SyncFile &of,
        mdb_size_t txn_id,
        size_t op_id,
        MDB_dbi dbi,
        const std::string &name,
        const ComparatorDesc& desc) {
        WriteHeader(of, txn_id, op_id, WalOpType::TABLE_OPEN);
        LogToFile(of, dbi);
        LogToFile(of, name);
        LogToFile(of, desc);
    }

    static void LogTableDrop(SyncFile &of, mdb_size_t txn_id, size_t op_id, MDB_dbi dbi) {
        WriteHeader(of, txn_id, op_id, WalOpType::TABLE_DROP);
        LogToFile(of, dbi);
    }

    static void LogKvPut(SyncFile &of, mdb_size_t txn_id, size_t op_id, MDB_dbi dbi,
                         const Value &k,
                          const Value &v) {
        WriteHeader(of, txn_id, op_id, WalOpType::KV_PUT);
        LogToFile(of, dbi);
        LogToFile(of, k);
        LogToFile(of, v);
    }

    static void LogKvDel(SyncFile &of, mdb_size_t txn_id, size_t op_id, MDB_dbi dbi,
                         const Value &k) {
        WriteHeader(of, txn_id, op_id, WalOpType::KV_DEL);
        LogToFile(of, dbi);
        LogToFile(of, k);
    }

    // must be a table_open, otherwise may crash
    const TableOpenContent &GetTableOpenContent() const {
        return *static_cast<TableOpenContent *>(content);
    }

    // must be a kv_put
    const KvPutContent &GetKvPutContent() const {
        return *static_cast<KvPutContent *>(content);
    }

    // must be a kv_del
    const KeyDelContent &GetKeyDelContent() const {
        return *static_cast<KeyDelContent *>(content);
    }

    bool IsValid() const { return op_type != WalOpType::INVALID_ENTRY; }

 private:
    void DeleteContent() {
        switch (op_type) {
        case WalOpType::TABLE_OPEN:
            delete static_cast<TableOpenContent *>(content);
            break;
        case WalOpType::KV_PUT:
            delete static_cast<KvPutContent *>(content);
            break;
        case WalOpType::KV_DEL:
            delete static_cast<KeyDelContent *>(content);
            break;
        default:
            break;
        }
        op_type = WalOpType::INVALID_ENTRY;
    }
};

template<>
void LogToFile<LogEntry>(SyncFile &of, const LogEntry &l) {
    // write header
    LogToFile(of, _wal::MAGIC_NUM);
    LogToFile(of, l.txn_id);
    LogToFile(of, l.op_id);
    LogToFile(of, l.op_type);
    // now write other contents
    switch (l.op_type) {
    case WalOpType::TXN_BEGIN:
    case WalOpType::TXN_COMMIT:
    case WalOpType::TXN_ABORT:
        LogToFile(of, l.is_child);
        break;
    case WalOpType::TABLE_DROP:
        LogToFile(of, l.dbi);
        break;
    case WalOpType::TABLE_OPEN:
        LogToFile(of, l.dbi);
        LogToFile(of, l.GetTableOpenContent());
        break;
    case WalOpType::KV_PUT: {
            LogToFile(of, l.dbi);
            auto &p = l.GetKvPutContent();
            LogToFile(of, p.first);
            LogToFile(of, p.second);
            break;
        }
    case WalOpType::KV_DEL:
        LogToFile(of, l.dbi);
        LogToFile(of, l.GetKeyDelContent());
    default:
        FMA_WARN() << "Illegal op type: " << l.op_type;
        break;
    }
}

template<typename T>
void ReadInto(std::ifstream &in, T &d) {
    d = ReadT<T>(in);
}

// read the next log entry
// success is an output parameter, will be set to true if success
// if failed, return an invalid entry
inline LogEntry ReadNextLog(std::ifstream &in, bool &success) {
    success = true;
    try {
        // read header
        if (ReadT<std::decay<decltype(_wal::MAGIC_NUM)>::type>(in) != _wal::MAGIC_NUM) {
            success = false;
            return {};
        }
        LogEntry ret;
        ReadInto(in, ret.txn_id);
        ReadInto(in, ret.op_id);
        ReadInto(in, ret.op_type);
        // read others
        switch (ret.op_type) {
        case WalOpType::TXN_BEGIN:
        case WalOpType::TXN_COMMIT:
        case WalOpType::TXN_ABORT:
            ReadInto(in, ret.is_child);
            break;
        case WalOpType::TABLE_DROP:
            ReadInto(in, ret.dbi);
            break;
        case WalOpType::TABLE_OPEN:
            {
                ReadInto(in, ret.dbi);
                auto* c = new LogEntry::TableOpenContent();
                ret.content = c;
                ReadInto(in, c->first);
                ReadInto(in, c->second);
                break;
            }
        case WalOpType::KV_PUT:
            {
                ReadInto(in, ret.dbi);
                auto *c = new LogEntry::KvPutContent();
                ret.content = c;
                ReadInto(in, c->first);
                ReadInto(in, c->second);
                break;
            }
        case WalOpType::KV_DEL:
            {
                ReadInto(in, ret.dbi);
                auto *c = new LogEntry::KeyDelContent();
                ret.content = c;
                ReadInto(in, *c);
                break;
            }
        default:
            FMA_WARN() << "Unrecognized op type from log: " << ret.op_type;
            success = false;
        }
        return ret;
    } catch (KvException &e) {
        FMA_WARN() << "KvException occurred while reading log: " << e.what();
        success = false;
        return {};
    } catch (std::exception &e) {
        FMA_WARN() << "Failed to read next log: " << e.what();
        success = false;
        return {};
    }
}

void Wal::WriteTxnBegin(mdb_size_t txn_id, bool is_child) {
    if (!is_child) op_id_ = 0;
    curr_txn_id_ = txn_id;
    LogEntry::LogTxnBegin(log_file_, txn_id, op_id_++, is_child);
}

void Wal::WriteTxnCommit(mdb_size_t txn_id, bool is_child) {
    LogEntry::LogTxnCommit(log_file_, txn_id, op_id_++, is_child);
    if (!is_child) {
        log_file_.Sync();
        FlushDbAndRotateLogIfNecessary();
    }
}

void Wal::WriteTxnAbort(mdb_size_t txn_id, bool is_child) {
    LogEntry::LogTxnAbort(log_file_, txn_id, op_id_++, is_child);
}

void Wal::WriteTableOpen(
    MDB_dbi dbi,
    const std::string &name,
    const ComparatorDesc& desc) {
    LogEntry::LogTableOpen(dbi_file_, curr_txn_id_, op_id_++, dbi, name, desc);
    dbi_file_.Sync();
}

void Wal::WriteTableDrop(MDB_dbi dbi) {
    LogEntry::LogTableDrop(log_file_, curr_txn_id_, op_id_++, dbi);
}

void Wal::WriteKvPut(MDB_dbi dbi, const Value &key, const Value &value) {
    LogEntry::LogKvPut(log_file_, curr_txn_id_, op_id_++, dbi, key, value);
}

void Wal::WriteKvDel(MDB_dbi dbi, const Value &key) {
    LogEntry::LogKvDel(log_file_, curr_txn_id_, op_id_++, dbi, key);
}

Wal::Wal(MDB_env* env, const std::string &log_dir, size_t flush_interval_ms)
    : env_(env), log_dir_(log_dir), flush_interval_ms_(flush_interval_ms) {
    // redo the logs by scanning existing log files
    dbi_log_path_ = FMA_FMT("{}/{}", log_dir_, _wal::WAL_DBI_FILE_PREFIX);
    ReplayLogs();
    // open dbi_file for write
    dbi_file_.Open(dbi_log_path_);
    // now re-open wal for write
    OpenNextLogForWrite();
    last_flush_time_ = std::chrono::system_clock::now();
}

inline void TryDeleteLog(const std::string &path) {
    if (!fma_common::file_system::RemoveFile(path)) {
        FMA_WARN() << "Failed to delete log file " << path;
    }
}

Wal::~Wal() {
    int ec = mdb_env_sync(env_, true);
    for (auto &t : delete_tasks_) {
        t.second->Cancel();
        TryDeleteLog(t.first);
    }
    if (ec == MDB_SUCCESS) {
        log_file_.Close();
        dbi_file_.Close();
        TryDeleteLog(curr_log_path_);
        TryDeleteLog(dbi_log_path_);
    }
}

void Wal::ReplayLogs()  {
    std::ifstream dbi_in(dbi_log_path_, std::ios::binary);
    if (!dbi_in.good()) {
        FMA_DBG() << "No wal found, starting clean.";
        return;
    }
    // list log files
    std::set<uint64_t> log_file_ids;
    for (auto &fname : fma_common::file_system::ListFiles(log_dir_, nullptr, false)) {
        if (fma_common::StartsWith(fname, _wal::WAL_FILE_PREFIX)) {
            uint64_t id = 0;
            if (!fma_common::ParseString(fname.substr(strlen(_wal::WAL_FILE_PREFIX)), id)) {
                FMA_WARN() << "Unrecognized log file name " << fname;
            }
            log_file_ids.insert(id);
        }
    }
    if (log_file_ids.empty()) {
        FMA_DBG() << "No op wal found, starting clean.";
        return;
    }
    // Read files and replay
    // get lmdb txn id
    MDB_txn *root_txn;
    THROW_ON_ERR(mdb_txn_begin(env_, nullptr, MDB_RDONLY, &root_txn));
    mdb_size_t lmdb_txn_id = mdb_txn_id(root_txn);
    mdb_txn_abort(root_txn);
    root_txn = nullptr;
    MDB_txn* child_txn = nullptr;

    // read first entry of both
    bool succ_dbi = false;
    LogEntry next_dbi_entry = ReadNextLog(dbi_in, succ_dbi);
    auto log_file_it = log_file_ids.begin();
    std::string op_log_path = GetLogFilePathFromId(*log_file_it);
    std::ifstream op_in(op_log_path, std::ios::binary);
    LogEntry next_op_entry;
    bool succ_op = false;
    bool should_read_op = true;
    std::string err_msg;

#define THROW_ON_ERR(stmt)                            \
    do {                                              \
        int ec = (stmt);                              \
        if (ec != MDB_SUCCESS) throw KvException(ec); \
    } while (0)
#define BREAK_ON_ERR(stmt) \
    do {                   \
        int ec = stmt;     \
        if (ec) {          \
            err_msg = mdb_strerror(ec); \
            fatal_error = true;         \
        }                  \
    } while (0);            \
    if (fatal_error) break;

#define BREAK_WITH_MSG(msg)  \
    err_msg = msg;           \
    fatal_error = true;      \
    break;

#define BREAK_ON_UNEXPECTED_OP(op) \
    BREAK_WITH_MSG("Failed to apply logs, unexpected " op);

    bool fatal_error = false;
    std::unordered_map<MDB_dbi, MDB_dbi> old_dbi_to_new_dbi;
    while (true) {
        if (should_read_op) {
            while (true) {
                next_op_entry = ReadNextLog(op_in, succ_op);
                // if (succ_op) break;
                if (!succ_op) {
                    FMA_LOG() << "Finished to read from log file " << op_log_path;
                    // delete this file
                    op_in.close();
                    // open next file
                    log_file_it++;
                    if (log_file_it == log_file_ids.end()) {
                        break;
                    }
                    op_log_path = GetLogFilePathFromId(*log_file_it);
                    op_in.open(op_log_path, std::ios::binary);
                } else {
                    break;
                }
            }
            // cannot read any more op log, just quit
            if (!succ_op) break;
        } else {
            // should read dbi log
            next_dbi_entry = ReadNextLog(dbi_in, succ_dbi);
        }
        // now compare their id and see which one to apply
        LogEntry *to_apply = nullptr;
        if (next_dbi_entry.IsValid()) {
            if (!next_op_entry.IsValid()) {
                to_apply = &next_dbi_entry;
            } else {
                if (std::make_pair(next_dbi_entry.txn_id, next_dbi_entry.op_id) <
                    std::make_pair(next_op_entry.txn_id, next_op_entry.op_id)) {
                    to_apply = &next_dbi_entry;
                } else {
                    to_apply = &next_op_entry;
                }
            }
        } else {
            // invalid dbi entry
            if (next_op_entry.IsValid()) {
                to_apply = &next_op_entry;
            }
        }
        // if nothing to apply, stop
        if (!to_apply) break;
        // if next op is to be consumed, should read next op
        if (to_apply == &next_op_entry)
            should_read_op = true;
        else
            should_read_op = false;
        // check txn_id and skip if necessary
        if (to_apply == &next_op_entry && to_apply->txn_id <= lmdb_txn_id) continue;
        // now apply this log
        switch (to_apply->op_type) {
        case WalOpType::INVALID_ENTRY: {
            BREAK_ON_UNEXPECTED_OP("INVALID_ENTRY");
        }
        case WalOpType::TXN_BEGIN: {
            // begin a mdb txn
            if (to_apply->is_child) {
                if (!root_txn || child_txn) {
                    BREAK_ON_UNEXPECTED_OP("TXN_BEGIN");
                }
                THROW_ON_ERR(mdb_txn_begin(env_, root_txn, MDB_NOSYNC, &child_txn));
            } else {
                if (root_txn || child_txn) {
                    BREAK_ON_UNEXPECTED_OP("TXN_BEGIN");
                }
                THROW_ON_ERR(mdb_txn_begin(env_, nullptr, MDB_NOSYNC, &root_txn));
            }
            break;
        }
        case WalOpType::TXN_COMMIT: {
            // commit current txn
            if (to_apply->is_child) {
                if (!child_txn) {
                    throw KvException("TXN_COMMIT is_child");
                    BREAK_ON_UNEXPECTED_OP("TXN_COMMIT");
                }
                THROW_ON_ERR(mdb_txn_commit(child_txn));
                child_txn = nullptr;
            } else {
                if (!root_txn || child_txn) {
                    throw KvException("TXN_COMMIT !is_child");
                    BREAK_ON_UNEXPECTED_OP("TXN_COMMIT");
                }
                int ec = mdb_txn_commit(root_txn);
                root_txn = nullptr;
            }
            break;
        }
        case WalOpType::TXN_ABORT: {
            // abort current txn
            if (to_apply->is_child) {
                if (!child_txn) {
                    BREAK_ON_UNEXPECTED_OP("TXN_ABORT");
                }
                mdb_txn_abort(child_txn);
                child_txn = nullptr;
            } else {
                if (!root_txn || child_txn) {
                    BREAK_ON_UNEXPECTED_OP("TXN_ABORT");
                }
                mdb_txn_abort(root_txn);
                root_txn = nullptr;
            }
            break;
        }
        case WalOpType::TABLE_OPEN: {
            // update dbi table
            // table open should always happen in root_txn
            if (child_txn) {
                // BREAK_ON_UNEXPECTED_OP("TXN_OPEN");
                throw KvException("table open should always happen in root_txn");
            }
            bool abort_txn = false;
            unsigned int flags = MDB_CREATE;
            if (!root_txn) {
                THROW_ON_ERR(mdb_txn_begin(env_, nullptr, MDB_NOSYNC, &root_txn));
                abort_txn = true;
                flags = 0;
            }
            MDB_dbi dbi = 0;
            // get comparator
            auto &c = to_apply->GetTableOpenContent();
            THROW_ON_ERR(mdb_dbi_open(root_txn, c.first.c_str(), flags, &dbi));
            auto comp = GetKeyComparator(c.second);
            if (comp) THROW_ON_ERR(mdb_set_compare(root_txn, dbi, comp));
            old_dbi_to_new_dbi[to_apply->dbi] = dbi;
            if (abort_txn) {
                mdb_txn_abort(root_txn);
                root_txn = nullptr;
            }
            break;
        }
        case WalOpType::TABLE_DROP: {
            // delete a table
            if (!root_txn || child_txn) {
                BREAK_ON_UNEXPECTED_OP("TXN_OPEN");
            }
            auto it = old_dbi_to_new_dbi.find(to_apply->dbi);
            if (it == old_dbi_to_new_dbi.end()) {
                BREAK_WITH_MSG(FMA_FMT("Unexpected dbi: {}", to_apply->dbi));
            }
            THROW_ON_ERR(mdb_drop(root_txn, it->second, true));
            break;
        }
        case WalOpType::KV_PUT: {
            // put a kv
            MDB_txn *txn = child_txn ? child_txn : root_txn;
            if (!txn) {
                BREAK_ON_UNEXPECTED_OP("KV_PUT");
            }
            auto it = old_dbi_to_new_dbi.find(to_apply->dbi);
            if (it == old_dbi_to_new_dbi.end()) {
                BREAK_WITH_MSG(FMA_FMT("Unexpected dbi: {}", to_apply->dbi));
            }
            auto &p = to_apply->GetKvPutContent();
            MDB_val k = p.first.MakeMdbVal();
            MDB_val v = p.second.MakeMdbVal();
            THROW_ON_ERR(mdb_put(txn, it->second, &k, &v, 0));
            break;
        }
        case WalOpType::KV_DEL: {
            // del a key
            MDB_txn *txn = child_txn ? child_txn : root_txn;
            if (!txn) {
                BREAK_ON_UNEXPECTED_OP("KV_DEL");
            }
            auto it = old_dbi_to_new_dbi.find(to_apply->dbi);
            if (it == old_dbi_to_new_dbi.end()) {
                BREAK_WITH_MSG(FMA_FMT("Unexpected dbi: {}", to_apply->dbi));
            }
            MDB_val k = to_apply->GetKeyDelContent().MakeMdbVal();
            THROW_ON_ERR(mdb_del(txn, it->second, &k, nullptr));
            break;
        }
        }
    }
    if (child_txn) mdb_txn_abort(child_txn);
    if (root_txn) mdb_txn_abort(root_txn);
    mdb_env_sync(env_, true);
    dbi_in.close();
    op_in.close();
    if (fatal_error) {
        // something bad happened, thow an exception and see if we can recover
        throw KvException(err_msg);
    }
    for (auto i : log_file_ids) {
        TryDeleteLog(GetLogFilePathFromId(i));
    }
}

void Wal::FlushDbAndRotateLogIfNecessary() {
    auto now = std::chrono::system_clock::now();
    if (now - last_flush_time_ <= std::chrono::milliseconds(flush_interval_ms_)) {
        return;
    }
    // need to rotate file
    last_flush_time_ = now;
    // if no log has been written in current log file, just return
    if (log_file_.TellP() == 0) return;
    std::string old_log_path = curr_log_path_;
    // close current file and open new one
    OpenNextLogForWrite();
    // schedule a task to flush and delete
    {
        std::lock_guard<std::mutex> l(tasks_lock_);
        delete_tasks_[old_log_path] =
            fma_common::TimedTaskScheduler::GetInstance().RunAfterDuration(
                0,  // start right now
                [this, old_log_path](fma_common::TimedTask* self) {
                    // flush env
                    mdb_env_sync(env_, true);
                    // delete file
                    TryDeleteLog(old_log_path);
                    // delete task ptr from delete_tasks
                    if (tasks_lock_.try_lock()) {
                        // try_lock is used here to avoid deadlock.
                        // In ~Wal() the main thread will try to lock tasks_lock_ and then
                        // call task->Cancel(), which requires locking task->lock.
                        // While this task is holding task->lock when it executes, if it tries
                        // to get tasks_lock, a deadlock will happen.
                        delete_tasks_.erase(old_log_path);
                        std::vector<std::string> executed_tasks;
                        for (auto &t : delete_tasks_) {
                            if (t.second->Executed()) executed_tasks.push_back(t.first);
                        }
                        for (auto &t : executed_tasks) delete_tasks_.erase(t);
                        tasks_lock_.unlock();
                    }
                });
    }
}

void Wal::OpenNextLogForWrite() {
    // get next log file path
    curr_log_path_ = GetLogFilePathFromId(next_log_file_id_++);
    // open log file
    log_file_.Close();
    log_file_.Open(curr_log_path_);
}

std::string Wal::GetLogFilePathFromId(uint64_t log_file_id) const {
    return FMA_FMT("{}/{}{}", log_dir_, _wal::WAL_FILE_PREFIX, log_file_id);
}

}  // namespace lgraph
