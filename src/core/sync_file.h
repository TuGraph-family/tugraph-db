/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <string>
#ifdef _WIN32
// TODO(hct): Implement fsync for Windows
#include <fstream>
#else
#include <unistd.h>
#endif

namespace lgraph {

/**
 * @brief   A file that can be Sync()ed to disk. This is to ensure that log is persisted to
 *          disk properly. Used in WAL.
 */
class SyncFile {
#ifdef _WIN32
    std::ofstream file_;
#else
    int file_ = -1;
    size_t p_pos_ = 0;
#endif
    std::string path_;

 public:
    SyncFile() {}

    explicit SyncFile(const std::string& path);

    ~SyncFile();

    void Open(const std::string& path);

    void Close();

    void Write(const void* buf, size_t s);

    const std::string& Path() const;

    void Sync();

    size_t TellP() const;
};

}  // namespace lgraph
