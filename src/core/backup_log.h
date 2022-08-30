/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "fma-common/rotating_files.h"

#include "core/data_type.h"
#include "protobuf/ha.pb.h"

namespace lgraph {
class BackupLog {
    std::mutex mtx_;
    std::unique_ptr<fma_common::RotatingFiles> file_;
    std::atomic<int64_t> idx_;

 public:
    BackupLog(const std::string& dir, size_t max_file_size) {
        file_.reset(
            new fma_common::RotatingFiles(dir,
                                          std::unique_ptr<fma_common::RotatingFileNameGenerator>(
                                              new fma_common::IndexedFileNameGenerator("binlog_")),
                                          max_file_size));
        idx_ = 0;
    }

    static std::vector<std::string> SortLogFiles(const std::vector<std::string>& paths) {
        std::vector<std::string> ret;
        std::unordered_map<std::string, std::string> name_to_path;
        std::vector<std::string> names;
        fma_common::IndexedFileNameGenerator fn_gen("binlog_");
        for (auto& p : paths) {
            std::string name = fma_common::FilePath(p).Name();
            if (fn_gen.IsMyFile(name)) {
                name_to_path.insert(std::make_pair(name, p));
                names.push_back(name);
            }
        }
        std::sort(names.begin(), names.end(), [&](const std::string& lhs, const std::string& rhs) {
            return fn_gen.NameCompareLess(lhs, rhs);
        });
        for (auto& n : names) ret.push_back(name_to_path[n]);
        return ret;
    }

    static bool ReadNextLogEntry(fma_common::InputFmaStream& stream, BackupLogEntry* e) {
        size_t size;
        size_t s = stream.Read(&size, sizeof(size));
        if (s != sizeof(size)) return false;
        std::string buf(size, 0);
        s = stream.Read(&buf[0], size);
        if (s != size) return false;
        return e->ParseFromString(buf);
    }

    void Write(const LGraphRequest* req) {
        std::lock_guard<std::mutex> l(mtx_);
        BackupLogEntry log;
        *log.mutable_req() = *req;
        log.set_time(lgraph::DateTime::LocalNow().SecondsSinceEpoch());
        log.set_index(idx_++);
        std::string buf = log.SerializeAsString();
        size_t s = buf.size();
        file_->Write(&s, sizeof(s), false);
        file_->Write(log.SerializeAsString(), true);
    }

    void Flush() { file_->Flush(); }

    std::vector<std::string> ListLogFiles() { return file_->ListFiles(); }

    void TruncateLogs() {
        std::lock_guard<std::mutex> l(mtx_);
        idx_.store(0, std::memory_order_release);
        file_->DeleteExistingFiles();
    }
};
}  // namespace lgraph
