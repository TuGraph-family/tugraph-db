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

#include <algorithm>
#include <exception>
#include <cstdint>
#include <string>

#include "fma-common/string_formatter.h"
#include "core/defs.h"

namespace lgraph {
namespace import_v2 {

class DenseString {
    static const size_t SMALL_BUF_SIZE = 8;

    uint32_t size_ = 0;
    union {
        char buf_[SMALL_BUF_SIZE];
        char* ptr_;
    };

    static_assert(sizeof(decltype(ptr_)) == 8, "must be using 64bit pointer");

 public:
    DenseString() : ptr_(nullptr) {}

    DenseString(const char* ptr, size_t s) { SetContent(ptr, s); }

    template <size_t N>
    explicit DenseString(char cstr[N]) {
        SetContent(cstr, N);
    }

    explicit DenseString(const std::string& s) { SetContent(s.data(), s.size()); }

    explicit DenseString(std::string& s) { SetContent(s.data(), s.size()); }

    ~DenseString() { Destroy(); }

    DenseString(DenseString&& rhs) {
        ptr_ = rhs.ptr_;
        size_ = rhs.size_;
        rhs.size_ = 0;
    }

    DenseString(const DenseString& rhs) {
        if (rhs.size_ > SMALL_BUF_SIZE) {
            SetContent(rhs.data(), rhs.size_);
        } else {
            ptr_ = rhs.ptr_;
            size_ = rhs.size_;
        }
    }

    DenseString& operator=(DenseString&& rhs) {
        if (this == &rhs) return *this;
        Destroy();
        ptr_ = rhs.ptr_;
        size_ = rhs.size_;
        rhs.size_ = 0;
        return *this;
    }

    DenseString& operator=(const DenseString& rhs) {
        if (this == &rhs) return *this;
        Destroy();
        SetContent(rhs.data(), rhs.size_);
        return *this;
    }

    bool operator==(const DenseString& rhs) const {
        if (this == &rhs) return true;
        if (size_ != rhs.size_) return false;
        if (size_ > SMALL_BUF_SIZE) {
            const char* p1 = data();
            const char* p2 = rhs.data();
            for (size_t i = 0; i < size_; i++)
                if (p1[i] != p2[i]) return false;
            return true;
        } else {
            return ptr_ == rhs.ptr_;
        }
    }

    bool operator!=(const DenseString& rhs) const { return !(*this == rhs); }

    bool operator<(const DenseString& rhs) const {
        if (this == &rhs) return false;
        size_t min_size = std::min(size_, rhs.size_);
        const char* p1 = data();
        const char* p2 = rhs.data();
        int r = memcmp(p1, p2, min_size);
        if (r < 0) return true;
        if (r > 0) return false;
        return size_ < rhs.size_;
    }

    const char* data() const { return size_ > SMALL_BUF_SIZE ? ptr_ : buf_; }

    size_t size() const { return size_; }

    size_t hash1() const {
        size_t ret = 0x811C9DC5;
        if (size_ == 0) return ret;
        const size_t fnv_prime = 0x1000193;
        const uint8_t* p = (const uint8_t*)data();
        size_t s = size_;
        while (s--) {
            ret ^= ((uint32_t)(*p++));
            ret *= fnv_prime;
        }
        return ret;
    }

    size_t hash2() const {
        size_t hash = 5381;
        size_t s = size_;
        const uint8_t* p = (const uint8_t*)data();
        while (s--) hash = ((hash << 5) + hash) + *p++;
        return hash;
    }

    std::string ToString() const { return std::string(data(), size()); }

    template <typename StreamT>
    size_t Serialize(StreamT& stream) const {
        stream.Write(&size_, sizeof(size_));
        stream.Write(data(), size_);
        return sizeof(size_) + size_;
    }

    template <typename StreamT>
    size_t Deserialize(StreamT& stream) {
        Destroy();
        decltype(size_) s = 0;
        size_t r = stream.Read(&s, sizeof(s));
        if (r == 0) return 0;
        FMA_DBG_CHECK_EQ(r, sizeof(s));
        if (s > SMALL_BUF_SIZE) {
            ptr_ = new char[s];
            r = stream.Read(ptr_, s);
            FMA_DBG_CHECK_EQ(r, s);
        } else {
            ptr_ = nullptr;
            r = stream.Read(buf_, s);
            FMA_DBG_CHECK_EQ(r, s);
        }
        size_ = static_cast<decltype(size_)>(s);
        return sizeof(size_t) + size_;
    }

 private:
    char* GetPtr() const { return (char*)((uint64_t)ptr_ & 0x0000FFFFFFFFFFFF); }

    void SetContent(const char* ptr, size_t s) {
        if (s > std::numeric_limits<decltype(size_)>::max()) {
            throw std::runtime_error(FMA_FMT("DenseString cannot hold data larger than {}.",
                                             std::numeric_limits<decltype(size_)>::max()));
        }
        if (s > SMALL_BUF_SIZE) {
            ptr_ = new char[s];
            memcpy(ptr_, ptr, s);
        } else {
            ptr_ = nullptr;
            memcpy(buf_, ptr, s);
        }
        size_ = static_cast<decltype(size_)>(s);
    }

    void Destroy() {
        if (size_ > SMALL_BUF_SIZE) delete[] ptr_;
    }
};

}  // namespace import_v2
}  // namespace lgraph
