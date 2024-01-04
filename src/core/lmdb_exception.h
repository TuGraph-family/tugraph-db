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

#include <exception>
#include <iomanip>
#include <sstream>
#include <string>

#include "tools/lgraph_log.h"
#include "core/lmdb/lmdb.h"
#include "core/data_type.h"

namespace lgraph {
class KvException : public InternalError {
    int ec_ = 0;

 public:
    explicit KvException(const std::string& str) : InternalError("KvException: " + str) {}

    explicit KvException(int mdb_error_code)
        : InternalError(std::string("KvException: ") + mdb_strerror(mdb_error_code)),
          ec_(mdb_error_code) {
        if (ec_ != MDB_CONFLICTS) LOG_WARN() << InternalError::what();
    }

    KvException(int mdb_error_code, const MDB_val& k, const MDB_val& v)
        : InternalError(std::string("KvException: ") + mdb_strerror(mdb_error_code) +
                        "\n\tKey: " + DumpMdbVal(k) + "\n\tValue: " + DumpMdbVal(v)),
          ec_(mdb_error_code) {
        if (ec_ != MDB_CONFLICTS) LOG_WARN() << InternalError::what();
    }

    int code() const { return ec_; }

 private:
    static std::string DumpMdbVal(const MDB_val& v) {
        static const char FourBitToHex[] = {"0123456789ABCDEF"};

        std::ostringstream oss;
        oss << "size=" << v.mv_size << ", ptr=" << v.mv_data;
        if (!v.mv_data) return oss.str();

        oss << ", data=[";
        const uint8_t* p = (const uint8_t*)v.mv_data;
        for (size_t i = 0; i < v.mv_size && i < 30; i++) {
            uint8_t byte = p[i];
            oss << FourBitToHex[byte >> 4] << FourBitToHex[byte & 0x0F] << " ";
        }
        oss << "]";
        return oss.str();
    }
};

#define THROW_ON_ERR(stmt)                            \
    do {                                              \
        int ec = (stmt);                              \
        if (ec != MDB_SUCCESS) throw KvException(ec); \
    } while (0)
#define THROW_ON_ERR_WITH_KV(stmt, k, v)                    \
    do {                                                    \
        int ec = (stmt);                                    \
        if (ec != MDB_SUCCESS) throw KvException(ec, k, v); \
    } while (0)
#define THROW_ERR(ec)          \
    do {                       \
        throw KvException(ec); \
    } while (0)
#define THROW_ERR_WITH_KV(ec, k, v)  \
    do {                             \
        throw KvException(ec, k, v); \
    } while (0)
}  // namespace lgraph
