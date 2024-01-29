/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include <string>
#ifdef _WIN32
// TODO(hct): Implement fsync for Windows
#include <fstream>
#else
#include <unistd.h>
#endif

#include "fma-common/fma_stream.h"

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
    fma_common::OutputMemoryFileStream buffer_;

 public:
    SyncFile() {}

    explicit SyncFile(const std::string& path);

    ~SyncFile();

    void Open(const std::string& path);

    void Close();

    void Write(const void* buf, size_t s);

    const std::string& Path() const;

    void Sync();

    size_t TellP();
};

}  // namespace lgraph
