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

#include <cstring>

#include "fma-common/assert.h"
#include "core/lmdb/lmdb.h"
#include "core/mem_profiler.h"
#include "lgraph/lgraph_types.h"

namespace lgraph {
namespace _detail {
template <typename T>
inline T UnalignedGet(const void* p) {
    T d;
    memcpy(&d, p, sizeof(T));
    return d;
}

template <typename T>
inline void UnalignedGet(const void* p, T& d) {
    memcpy(&d, p, sizeof(T));
}

template <typename T>
inline void UnalignedSet(void* p, const T& d) {
    memcpy(p, &d, sizeof(T));
}

template <typename T>
struct TypeCast {
    static T AsType(void* p, size_t s) {
        FMA_DBG_CHECK_EQ(sizeof(T), s);
        T d;
        memcpy(&d, p, s);
        return d;
    }
};

template <>
struct TypeCast<std::string> {
    static std::string AsType(void* p, size_t s) { return std::string((char*)p, s); }
};

template <>
struct TypeCast<lgraph_api::Point<lgraph_api::Wgs84>> {
    static lgraph_api::Point<lgraph_api::Wgs84> AsType(void* p, size_t s) {
        return lgraph_api::Point<lgraph_api::Wgs84>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<lgraph_api::Point<lgraph_api::Cartesian>> {
    static lgraph_api::Point<lgraph_api::Cartesian> AsType(void* p, size_t s) {
        return lgraph_api::Point<lgraph_api::Cartesian>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<lgraph_api::LineString<lgraph_api::Wgs84>> {
    static lgraph_api::LineString<lgraph_api::Wgs84> AsType(void* p, size_t s) {
        return lgraph_api::LineString<lgraph_api::Wgs84>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<lgraph_api::LineString<lgraph_api::Cartesian>> {
    static lgraph_api::LineString<lgraph_api::Cartesian> AsType(void* p, size_t s) {
        return lgraph_api::LineString<lgraph_api::Cartesian>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<lgraph_api::Polygon<lgraph_api::Wgs84>> {
    static lgraph_api::Polygon<lgraph_api::Wgs84> AsType(void* p, size_t s) {
        return lgraph_api::Polygon<lgraph_api::Wgs84>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<lgraph_api::Polygon<lgraph_api::Cartesian>> {
    static lgraph_api::Polygon<lgraph_api::Cartesian> AsType(void* p, size_t s) {
        return lgraph_api::Polygon<lgraph_api::Cartesian>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<lgraph_api::Spatial<lgraph_api::Wgs84>> {
    static lgraph_api::Spatial<lgraph_api::Wgs84> AsType(void* p, size_t s) {
        return lgraph_api::Spatial<lgraph_api::Wgs84>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<lgraph_api::Spatial<lgraph_api::Cartesian>> {
    static lgraph_api::Spatial<lgraph_api::Cartesian> AsType(void* p, size_t s) {
        return lgraph_api::Spatial<lgraph_api::Cartesian>(std::string((char*)p, s));
    }
};

template <>
struct TypeCast<std::vector<float>> {
    static std::vector<float> AsType(void* p, size_t s) {
        size_t num_floats = s / sizeof(float);
        std::vector<float> floatvector(num_floats);
        std::memcpy(floatvector.data(), p, s);
        return floatvector;
    }
};
}  // namespace _detail

/**
 * Representation for a memory block. It can be used to store a const reference
 * to a memory block, or a memory block malloced, and thus owned by the Value
 * object. A const reference is valid only when the memory block it refers to is
 * valid. It does not free the memory block when the Value object is destructed.
 * A Value owning a memory block will free its memory when the Value object is
 * destructed.
 *
 * \note  The implementation uses small-object optimization. When the memory
 * block is smaller or equal to 64 bytes, it just uses stack memory.
 */
class Value {
    friend class Transaction;
    friend class Vertex;
    static const int STACK_SIZE = 64;

    char* data_ = nullptr;
    size_t size_ = 0;
    bool need_delete_ = false;
    char stack_[STACK_SIZE];

    void Malloc(size_t s) {
        if (s <= STACK_SIZE) {
            data_ = stack_;
            need_delete_ = false;
            size_ = s;
        } else {
            data_ = (char*)LBMalloc(s);
            FMA_ASSERT(data_) << "Allocation falied, size = " << s;
            need_delete_ = true;
            size_ = s;
        }
    }

 public:
    /** Construct an empty value */
    Value() {
        data_ = nullptr;
        size_ = 0;
        need_delete_ = false;
    }

    /**
     * Construct a Value object with the specified size.
     *
     * \param   s   Size of the memory block to allocate.
     */
    explicit Value(size_t s) { Malloc(s); }

    explicit Value(size_t s, uint8_t fill) {
        Malloc(s);
        memset(Data(), fill, s);
    }

    /**
     * Construct a const reference to the given memory block.
     *
     * \param   buf Pointer to the memory block.
     * \param   s   Size of the buffer.
     */
    explicit Value(const void* buf, size_t s) {
        data_ = (char*)(buf);
        size_ = s;
        need_delete_ = false;
    }

    /**
     * Set the Value to the const reference to the given memory block.
     *
     * \param   buf Pointer to the memory block.
     * \param   s   Size of the memory block.
     */
    void AssignConstRef(const char* buf, size_t s) {
        if (need_delete_) LBFree(data_);
        data_ = const_cast<char*>(buf);
        size_ = s;
        need_delete_ = false;
    }

    /**
     * Take ownership of the buf given.
     *
     * \param   buf Pointer to the memory block.
     * \param   s   Size of the memory block.
     */
    void TakeOwnershipFrom(void* buf, size_t s) {
        if (need_delete_) LBFree(data_);
        data_ = static_cast<char*>(buf);
        size_ = s;
        need_delete_ = true;
    }

    /**
     * Constructs a const reference to the memory block given in the MDB_val
     * object.
     * \param   val An MDB_val describing memory block and its size.
     */
    explicit Value(const MDB_val& val)
        : data_(static_cast<char*>(val.mv_data)), size_(val.mv_size), need_delete_(false) {}

    /**
     * Construct a Value object by copying the content of the string.
     *
     * \param   buf The string to copy.
     */
    explicit Value(const std::string& buf) {
        Malloc(buf.size());
        memcpy(data_, buf.data(), buf.size());
    }

    explicit Value(const char* buf) {
        data_ = (char*)buf;
        size_ = strlen(buf);
        need_delete_ = false;
    }

    Value(const Value& rhs) {
        if (rhs.need_delete_) {
            Malloc(rhs.size_);
            memcpy(data_, rhs.data_, rhs.size_);
        } else {
            if (rhs.size_ > 0 && rhs.data_ == rhs.stack_) {
                // copy over
                memcpy(stack_, rhs.stack_, rhs.size_);
                data_ = stack_;
            } else {
                data_ = rhs.data_;
            }
            size_ = rhs.size_;
            need_delete_ = false;
        }
    }

    Value(Value&& rhs) {
        need_delete_ = rhs.need_delete_;
        size_ = rhs.size_;
        if (rhs.size_ > 0 && rhs.data_ == rhs.stack_) {
            // copy over
            memcpy(stack_, rhs.stack_, rhs.size_);
            data_ = stack_;
        } else {
            data_ = rhs.data_;
            rhs.need_delete_ = false;
            rhs.data_ = nullptr;
            rhs.size_ = 0;
        }
    }

    Value& operator=(const Value& rhs) {
        if (this == &rhs) return *this;
        if (rhs.need_delete_) {
            if (need_delete_) {
                FMA_DBG_ASSERT(data_ != stack_);
                data_ = (char*)LBRealloc(data_, rhs.size_);
                size_ = rhs.size_;
            } else {
                Malloc(rhs.size_);
            }
            memcpy(data_, rhs.data_, rhs.size_);
        } else {
            if (rhs.size_ > 0 && rhs.data_ == rhs.stack_) {
                if (need_delete_) LBFree(data_);
                // copy over
                memcpy(stack_, rhs.stack_, rhs.size_);
                data_ = stack_;
            } else {
                data_ = rhs.data_;
            }
            size_ = rhs.size_;
            need_delete_ = false;
        }
        return *this;
    }

    Value& operator=(Value&& rhs) {
        if (this == &rhs) return *this;
        if (need_delete_) LBFree(data_);
        need_delete_ = rhs.need_delete_;
        size_ = rhs.size_;
        if (rhs.size_ > 0 && rhs.data_ == rhs.stack_) {
            // copy over
            memcpy(stack_, rhs.stack_, rhs.size_);
            data_ = stack_;
        } else {
            data_ = rhs.data_;
            rhs.need_delete_ = false;
            rhs.size_ = 0;
        }
        return *this;
    }

    void Clear() {
        if (need_delete_) LBFree(data_);
        data_ = nullptr;
        size_ = 0;
        need_delete_ = false;
    }

    void AssignConstRef(void* p, size_t s) {
        if (need_delete_) LBFree(data_);
        need_delete_ = false;
        data_ = (char*)p;
        size_ = s;
    }

    template <typename T>
    void AssignConstRef(const T& d) {
        AssignConstRef(&d, sizeof(T));
    }

    void AssignConstRef(const std::string& str) { AssignConstRef(str.data(), str.size()); }

    /**
     * Equality operator. Compares by the byte content.
     *
     * @param rhs   The right hand side.
     *
     * @return  True if the parameters are considered equivalent.
     */
    bool operator==(const Value& rhs) const {
        return Size() == rhs.Size() && memcmp(Data(), rhs.Data(), Size()) == 0;
    }

    bool operator!=(const Value& rhs) const { return !(*this == rhs); }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif
    ~Value() {
        if (need_delete_) LBFree(data_);
    }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    static Value MakeCopy(const Value& rhs) {
        Value v;
        v.Copy(rhs);
        return v;
    }

    static Value MakeCopy(const MDB_val& v) {
        Value rv(v.mv_size);
        memcpy(rv.Data(), v.mv_data, v.mv_size);
        return rv;
    }

    static Value MakeCopy(const char* buf, size_t s) {
        Value rv(s);
        memcpy(rv.Data(), buf, s);
        return rv;
    }

    // make a copy of current data
    Value MakeCopy() const {
        Value v;
        v.Copy(*this);
        return v;
    }

    template <typename T>
    void Copy(const T& d) {
        Resize(sizeof(T));
        memcpy(data_, &d, sizeof(T));
    }

    void Copy(const char* buf, size_t s) {
        Resize(s);
        memcpy(data_, buf, s);
    }

    void Copy(const std::string& s) {
        Resize(s.size());
        memcpy(data_, s.data(), s.size());
    }

    void Copy(const std::vector<float>& vec) {
        Resize(vec.size() * sizeof(float));
        memcpy(data_, vec.data(), vec.size() * sizeof(float));
    }

    /**
     * Make a copy of the memory referred to by rhs. The new memory block is owned
     * by *this.
     *
     * \param   rhs The right hand side.
     */
    void Copy(const Value& rhs) {
        Resize(rhs.Size());
        memcpy(data_, rhs.data_, rhs.Size());
    }

    /**
     * Make a copy of the memory referred to by v. The new memory block is owned
     * by *this.
     *
     * \param   v   A MDB_val.
     */
    void Copy(MDB_val v) {
        Resize(v.mv_size);
        memcpy(data_, v.mv_data, v.mv_size);
    }

    /**
     * Gets the pointer to memory block.
     *
     * \return  Memory block referred to by this
     */
    char* Data() const { return data_; }

    /**
     * Gets the size of the memory block
     *
     * \return  A size_t.
     */
    size_t Size() const { return size_; }

    /**
     * If this Value empty?
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Empty() const { return size_ == 0; }

    /**
     * Makes mdb value that refers to current memory block.
     *
     * \return  A MDB_val.
     */
    MDB_val MakeMdbVal() const {
        MDB_val v;
        v.mv_data = data_;
        v.mv_size = size_;
        return v;
    }

    /**
     * Means only hold the data ptr and size, not the memory block.
     *
     * \return  true or false.
     */
    bool IsSlice() const {
        return !need_delete_;
    }

    /**
     * Resizes the memory block. If *this is a const reference, a new memory block
     * is created so *this will own the memory.
     *
     * \param   s       Size of the new memory block.
     * \param   reserve (Optional) Size of the memory block to reserve. Although
     * this->Size() will be equal to s, the underlying memory block is at least
     * reserve bytes. This is to reduce further memory allocation if we need to
     * expand the memory block later.
     */
    void Resize(size_t s, size_t reserve = 0) {
        size_t msize = std::max<size_t>(reserve, s);
        if (need_delete_) {
            if (msize > size_) {
                // do realloc only if we are expanding
                data_ = (char*)LBRealloc(data_, msize);
                FMA_ASSERT(data_ != nullptr) << "Allocation failed";
            }
        } else {
            if (data_ != nullptr) {
                void* oldp = data_;
                size_t olds = size_;
                Malloc(msize);
                if (oldp != data_) {
                    memcpy(data_, oldp, std::min<size_t>(olds, s));
                }
            } else {
                Malloc(msize);
            }
        }
        size_ = s;
    }

    /**
     * Converts this object to data of type T
     *
     * \tparam  T   Type of data to convert to
     *
     * \return  A object of type T.
     */
    template <typename T>
    T AsType() const {
        return _detail::TypeCast<T>::AsType(data_, size_);
    }

    /**
     * Converts this object to a string. The memory is copied as-is into the
     * string.
     *
     * \return  A std::string which has size()==this->Size() and
     * data()==this->Data()
     */
    std::string AsString() const { return AsType<std::string>(); }

    std::vector<float> AsFloatVector() const { return AsType<std::vector<float>>(); }

    /**
     * Create a Value that is a const reference to the object t
     *
     * \tparam  T   Generic type parameter.
     * \param   t   A T to process.
     *
     * \return  A Value.
     */
    template <typename T, size_t S = sizeof(T)>
    static Value ConstRef(const T& t) {
        return Value(&t, S);
    }

    static Value ConstRef(const Value& v) { return Value(v.Data(), v.Size()); }

    /**
     * Create a Value that is a const reference to s.c_str()
     *
     * \param   s   A std::string.
     *
     * \return  A Value.
     */
    static Value ConstRef(const std::string& s) { return Value(s.data(), s.size()); }

    /**
     * Create a Value that is a const reference to a c-string
     *
     * \param   s   The c-string
     *
     * \return  A Value.
     */
    static Value ConstRef(const char* const& s) { return Value(s, std::strlen(s)); }

    /**
     * Create a Value that is a const reference to a c-string
     *
     * \param   s   The c-string
     *
     * \return  A Value.
     */
    static Value ConstRef(char* const& s) { return Value(s, std::strlen(s)); }

    /**
     * Create a Value that is a const reference to a float vector
     *
     * \param   s   The float vector
     *
     * \return  A Value.
     */
    static Value ConstRef(const std::vector<float>& s) {
        return Value(s.data(), s.size() * sizeof(float));
    }

    /**
     * Create a Value that is a const reference to a string literal
     *
     * \param   s   The string literal
     *
     * \return  A Value.
     */
    template <size_t S>
    static Value ConstRef(const char (&s)[S]) {
        return Value(s, S - 1);
    }

    std::string DebugString(size_t line_width = 32) const {
        const char N2C[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        const uint8_t* ptr = (const uint8_t*)Data();
        size_t s = Size();
        std::string ret;
        for (size_t i = 0; i < s; i++) {
            if (i % line_width == 0 && i != 0) ret.push_back('\n');
            uint8_t c = ptr[i];
            ret.push_back(N2C[c >> 4]);
            ret.push_back(N2C[c & 0xF]);
            if (i != s - 1) ret.push_back(' ');
        }
        return ret;
    }
};

#define ENABLE_RVALUE(RT)                                                                 \
    template <typename VALUE>                                                             \
    typename std::enable_if<std::is_same<typename std::decay<VALUE>::type, Value>::value, \
                            RT>::type
}  // namespace lgraph
