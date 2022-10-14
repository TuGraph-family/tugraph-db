/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#ifndef _WIN32
#include <fcntl.h>
#endif
#include "core/data_type.h"
#include "core/sync_file.h"

lgraph::SyncFile::SyncFile(const std::string &path) {
    Open(path);
}

lgraph::SyncFile::~SyncFile() {
    Close();
}

const std::string& lgraph::SyncFile::Path() const {
    return path_;
}

#ifdef _WIN32
void lgraph::SyncFile::Open(const std::string &path) {
    Close();
    path_ = path;
    file_.open(path, std::ios_base::binary | std::ios_base::trunc);
    if (!file_.good())
        throw lgraph_api::IOError(FMA_FMT("Failed to open file {} for write", path_));
}

void lgraph::SyncFile::Close() {
    path_.clear();
    file_.close();
}

void lgraph::SyncFile::Write(const void* buf, size_t s) {
    file_.write(static_cast<const char*>(buf), s);
}

void lgraph::SyncFile::Sync() {
    file_.flush();
}

size_t lgraph::SyncFile::TellP() {
    return file_.tellp();
}
#else
void lgraph::SyncFile::Open(const std::string &path) {
    Close();
    path_ = path;
    file_ = open(path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (file_ == -1)
        throw lgraph_api::IOError(
            FMA_FMT("Failed to open file {} for write: {}",
                    path_, std::string(strerror(errno))));
}

void lgraph::SyncFile::Close() {
    if (file_ != -1) {
        path_.clear();
        close(file_);
        file_ = -1;
        p_pos_ = 0;
    }
}

void lgraph::SyncFile::Write(const void *buf, size_t s) {
    ssize_t r = write(file_, buf, s);
    if (r == -1)
        throw lgraph_api::IOError(
            FMA_FMT("Failed to write to file {}: {}",
                    path_, std::string(strerror(errno))));
    p_pos_ += s;
}

void lgraph::SyncFile::Sync() {
    fsync(file_);
}

size_t lgraph::SyncFile::TellP() {
    return p_pos_;
}
#endif
