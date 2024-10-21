//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common/binary_read_write_helper.h.
 *
 * \brief   Declares the binary read write helper class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once

#include <lgraph/lgraph_types.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>



#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

#ifdef _MSC_VER
#pragma warning(disable : 4800)  // disable warning C4800: force value to bool
#endif
namespace lgraph_api {
struct FieldData;
}

namespace fma_common {
DEFINE_HAS_MEM_FUNC_TEMPLATE(Serialize, _has_serialize_);
DEFINE_HAS_MEM_FUNC_TEMPLATE(Deserialize, _has_deserialize_);

/*!
 * \struct  _has_serialize
 *
 * \brief   Check whether T has a member function std::string& T::Serialize(std::string&)
 */
template <typename T, typename StreamT, bool isClass = std::is_class<T>::value>
struct _has_serialize {
    static const bool value = _has_serialize_<T, size_t (T::*)(StreamT&) const>::value;
};

template <typename T, typename StreamT>
struct _has_serialize<T, StreamT, false> {
    static const bool value = false;
};

template <typename T, typename StreamT, bool isClass = std::is_class<T>::value>
struct _has_deserialize {
    static const bool value = _has_deserialize_<T, size_t (T::*)(StreamT&)>::value;
};

template <typename T, typename StreamT>
struct _has_deserialize<T, StreamT, false> {
    static const bool value = false;
};

/*!
 * \def ENABLE_IF_HAS_SERIALIZE(T, RT)
 *
 * \brief   Enable a function with return type RT if T has is serializable
 *
 * \tparam   T   The class to be tested for serializability
 * \tparam   RT  The returned type
 */
#define ENABLE_IF_HAS_SERIALIZE(T, StreamT, RT) \
    typename std::enable_if<_has_serialize<T, StreamT>::value, RT>::type

#define ENABLE_IF_HAS_DESERIALIZE(T, StreamT, RT) \
    typename std::enable_if<_has_deserialize<T, StreamT>::value, RT>::type

/*!
 * \struct  _should_do_memcpy
 *
 * \brief   Type trait to determine whether type T should be copied with memcpy
 */
template <typename T, typename StreamT>
struct _can_do_memcpy {
    static const bool value = std::is_trivially_copyable<T>::value && !std::is_pointer<T>::value;
};

/*!
 * \def ENABLE_IF_MEMCPY_ALLOWED(T_, RT_)
 *
 * \brief   Enable a function with return type RT if T can be copied with memcpy
 *
 * \param   T_  The type to be tested for memcpy
 * \param   RT_ The return type of the function
 */
#define ENABLE_IF_MEMCPY_ALLOWED_READ(T_, StreamT, RT_) \
    typename std::enable_if<                            \
        _can_do_memcpy<T_, StreamT>::value && !_has_deserialize<T_, StreamT>::value, RT_>::type
#define ENABLE_IF_MEMCPY_ALLOWED_WRITE(T_, StreamT, RT_) \
    typename std::enable_if<                             \
        _can_do_memcpy<T_, StreamT>::value && !_has_serialize<T_, StreamT>::value, RT_>::type

#define ENABLE_IF_MEMCPY_NOT_ALLOWED_READ(T_, StreamT, RT_) \
    typename std::enable_if<                                \
        !_can_do_memcpy<T_, StreamT>::value || _has_deserialize<T_, StreamT>::value, RT_>::type
#define ENABLE_IF_MEMCPY_NOT_ALLOWED_WRITE(T_, StreamT, RT_) \
    typename std::enable_if<                                 \
        !_can_do_memcpy<T_, StreamT>::value || _has_serialize<T_, StreamT>::value, RT_>::type

/*!
 * \def ENABLE_IF_MEMCPY_ALLOWED(T_, RT_)
 *
 * \brief   Enable a function with return type RT if T cannot be copied with memcpy
 *
 * \param   T_  The type to be tested for memcpy
 * \param   RT_ The return type of the function
 */
#define ENABLE_IF_CANNOT_SERIALIZE(T_, StreamT, RT_) \
    typename std::enable_if<                         \
        !_can_do_memcpy<T_, StreamT>::value && !_has_serialize<T, StreamT>::value, RT_>::type

#define ENABLE_IF_CANNOT_DESERIALIZE(T_, StreamT, RT_) \
    typename std::enable_if<                           \
        !_can_do_memcpy<T_, StreamT>::value && !_has_deserialize<T, StreamT>::value, RT_>::type

/*!
 * \class   BinaryReaderWriter
 *
 * \brief   A binary reader writer that can read/write arbitrary data type
 *          into binary bytes in a Stream that can handle Read() and Write()
 *
 * \tparam  StreamT     stream type to read from / write to, must have Read()
 *                      or Write() interface
 * \tparam  T           type of data to be read/written
 */
template <typename StreamT, typename T>
class BinaryReader {
 public:
    /*!
     * \brief   Read data of type T from stream to d.
     *          This is enabled only when T can be copied with memcpy.
     *
     * \return  Number of bytes read.
     */
    template <typename T2 = T>
    static ENABLE_IF_MEMCPY_ALLOWED_READ(T2, StreamT, size_t) Read(StreamT& s, T& d) {
        return s.Read(&d, sizeof(T));
    }

    /*!
     * \brief   Read data of type T from stream to d.
     *          This is enabled only when T can be copied with memcpy.
     *
     * \return  Number of bytes read.
     */
    template <typename T2 = T>
    static ENABLE_IF_HAS_DESERIALIZE(T2, StreamT, size_t) Read(StreamT& s, T& d) {
        return d.Deserialize(s);
    }

    /*!
     * \brief   Read data of type T from stream to d.
     *          This is enabled only when T cannot be copied with memcpy.
     *
     * \return  Number of bytes read.
     */
    template <typename T2 = T>
    static ENABLE_IF_CANNOT_DESERIALIZE(T2, StreamT, size_t) Read(StreamT& s, T& d) {
        static_assert(_user_friendly_static_assert_<false, T2>::value,
                      "size_t BinaryRead(StreamT&, T&)    is not defined for this type.\n"
                      "You can define it by defining one of the following functions:\n"
                      "    1. size_t T::Deserialize(StreamT&)\n"
                      "    2. static size_t BinaryReader<StreamT, T>::Read(StreamT&, T&)\n"
                      "    3. size_t BinaryRead<StreamT, T>(StreamT&, T&)");
        return 0;
    }
};

/*!
 * \class   BinaryWriter
 *
 * \brief   A binary reader writer that can read/write arbitrary data type
 *          into binary bytes in a Stream that can handle Read() and Write()
 *
 * \tparam  StreamT     stream type to read from / write to, must have Read()
 *                      or Write() interface
 * \tparam  T           type of data to be read/written
 */
template <typename StreamT, typename T>
class BinaryWriter {
 public:
    /*!
     * \brief   Write data d into stream s.
     *          This is enabled only when T can be copied with memcpy.
     *
     * \return  Number of bytes written.
     */
    template <typename T2 = T>
    static ENABLE_IF_MEMCPY_ALLOWED_WRITE(T2, StreamT, size_t) Write(StreamT& s, const T& d) {
        s.Write(&d, sizeof(T));
        return sizeof(T);
    }

    /*!
     * \brief   Write data d into stream s.
     *          This is enabled only when T can be copied with memcpy.
     *
     * \return  Number of bytes written.
     */
    template <typename T2 = T>
    static ENABLE_IF_HAS_SERIALIZE(T2, StreamT, size_t) Write(StreamT& s, const T& d) {
        return d.Serialize(s);
    }

    /*!
     * \fn  static void Write(StreamT& s, T& d)
     *
     * \brief   Write data d into stream s.
     *          This is enabled only when T cannot be copied with memcpy.
     */
    template <typename T2 = T>
    static ENABLE_IF_CANNOT_SERIALIZE(T2, StreamT, size_t) Write(StreamT& s, const T& d) {
        static_assert(
            _user_friendly_static_assert_<false, T2>::value,
            "size_t BinaryWrite(StreamT&, const T&) const    is not defined for this type.\n"
            "You can define it by defining one of the following functions:\n"
            "    1. size_t T::Serialize(StreamT&) const\n"
            "    2. static size_t BinaryWriter<StreamT, T>::Write(StreamT&, const T&)\n"
            "    3. size_t BinaryWrite<StreamT, T>(StreamT&, const T&)");
        return 0;
    }
};

template <typename StreamT, typename T>
size_t BinaryRead(StreamT& s, T& d);

template <typename StreamT, typename T>
size_t BinaryWrite(StreamT& s, const T& d);

template <typename StreamT>
class BinaryWriterForFieldData {
 public:
    static size_t Write(StreamT& s, const ::lgraph_api::FieldData& data) {
        typedef ::lgraph_api::FieldType type;
        size_t data_size = 0;
        data_size += BinaryWrite<StreamT, int32_t>(s, static_cast<int32_t>(data.type));
        switch (data.type) {
        case type::NUL:
            break;
        case type::BOOL:
            data_size += BinaryWrite<StreamT, bool>(s, data.data.boolean);
            break;
        case type::INT8:
            data_size += BinaryWrite<StreamT, int8_t>(s, data.data.int8);
            break;
        case type::INT16:
            data_size += BinaryWrite<StreamT, int16_t>(s, data.data.int16);
            break;
        case type::INT32:
            data_size += BinaryWrite<StreamT, int32_t>(s, data.data.int32);
            break;
        case type::INT64:
            data_size += BinaryWrite<StreamT, int64_t>(s, data.data.int64);
            break;
        case type::FLOAT:
            data_size += BinaryWrite<StreamT, float>(s, data.data.sp);
            break;
        case type::DOUBLE:
            data_size += BinaryWrite<StreamT, double>(s, data.data.dp);
            break;
        case type::DATE:
            data_size += BinaryWrite<StreamT, int32_t>(s, data.data.int32);
            break;
        case type::DATETIME:
            data_size += BinaryWrite<StreamT, int64_t>(s, data.data.int64);
            break;
        case type::STRING:
        case type::POINT:
        case type::LINESTRING:
        case type::POLYGON:
        case type::SPATIAL:
        case type::BLOB:
            data_size += BinaryWriter<StreamT, std::string>::Write(s, *(std::string*)data.data.buf);
            break;
        case type::FLOAT_VECTOR:
            data_size += BinaryWriter<StreamT, std::vector<float>>::Write(
                s, *(std::vector<float>*)data.data.vp);

            break;
        }
        return data_size;
    }
};

template <typename StreamT>
class BinaryReaderForFieldData {
 public:
    static size_t Read(StreamT& s, ::lgraph_api::FieldData& data) {
        typedef ::lgraph_api::FieldType type;
        size_t read_size = 0;
        int32_t field_type;
        BinaryRead<StreamT, int32_t>(s, field_type);
        read_size += sizeof(int32_t);
        data.type = static_cast<::lgraph_api::FieldType>(field_type);
        std::vector<float> float_vec;
        std::string value;
        switch (data.type) {
        case type::NUL:
            break;
        case type::BOOL:
            read_size += BinaryRead<StreamT, bool>(s, data.data.boolean);

            break;
        case type::INT8:
            read_size += BinaryRead<StreamT, int8_t>(s, data.data.int8);

            break;
        case type::INT16:
            read_size += BinaryRead<StreamT, int16_t>(s, data.data.int16);

            break;
        case type::INT32:
            read_size += BinaryRead<StreamT, int32_t>(s, data.data.int32);
            break;
        case type::INT64:
            read_size += BinaryRead<StreamT, int64_t>(s, data.data.int64);
            break;
        case type::FLOAT:
            read_size += BinaryRead<StreamT, float>(s, data.data.sp);
            break;
        case type::DOUBLE:
            read_size += BinaryRead<StreamT, double>(s, data.data.dp);
            break;
        case type::DATE:
            read_size += BinaryRead<StreamT, int32_t>(s, data.data.int32);
            break;
        case type::DATETIME:
            read_size += BinaryRead<StreamT, int64_t>(s, data.data.int64);
            break;
        case type::STRING:
        case type::POINT:
        case type::LINESTRING:
        case type::POLYGON:
        case type::SPATIAL:
        case type::BLOB:
            read_size += BinaryRead<StreamT, std::string>(s, value);
            data.data.buf = new std::string(value);
            break;
        case type::FLOAT_VECTOR:
            read_size += BinaryRead<StreamT, std::vector<float>>(s, float_vec);
            data.data.vp = new std::vector<float>(std::move(float_vec));
            break;
        }
        return read_size;
    }
};

// \cond 0        Disable doxygen
/*!
 * \class    BinaryReaderWriter<StreamT,std::pair<T1,T2>>
 *
 * \brief    BinaryReaderWriter for pair types.
 *
 * \tparam    StreamT    Type of the stream t.
 * \tparam    T1           First type of the pair.
 * \tparam    T2           Second type of the pair.
 */
template <typename StreamT, typename T1, typename T2>
class BinaryReader<StreamT, std::pair<T1, T2>> {
 public:
    /**
     * Reads a pair
     *
     * \param [in,out] s    A StreamT to process.
     * \param [in,out] d    A std::pair&lt;T1,T2&gt; to process.
     *
     * \return  A size_t.
     */
    static size_t Read(StreamT& s, std::pair<T1, T2>& d) {
        return BinaryRead<StreamT, T1>(s, d.first) + BinaryRead<StreamT, T2>(s, d.second);
    }
};

/*!
 * \class    BinaryReaderWriter<StreamT,std::pair<T1,T2>>
 *
 * \brief    BinaryReaderWriter for pair types.
 *
 * \tparam    StreamT    Type of the stream t.
 * \tparam    T1           First type of the pair.
 * \tparam    T2           Second type of the pair.
 */
template <typename StreamT, typename T1, typename T2>
class BinaryWriter<StreamT, std::pair<T1, T2>> {
 public:
    /*!
     * \fn  static void Write(StreamT& s, const std::pair<T1, T2>& d)
     *
     * \brief   Writes data to stream.
     *
     * \param [in,out]  s   A StreamT to write to.
     * \param           d   A std::pair&lt;T1,T2&gt; to write.
     */
    static size_t Write(StreamT& s, const std::pair<T1, T2>& d) {
        return BinaryWrite<StreamT, T1>(s, d.first) + BinaryWrite<StreamT, T2>(s, d.second);
    }
};

/*!
 * \class    BinaryReaderWriter<StreamT,std::tuple<T1,T2,T3>>
 *
 * \brief    BinaryReaderWriter for tuple types.
 *
 * \tparam    StreamT      Type of the stream t.
 * \tparam    T1           First type of the tuple.
 * \tparam    T2           Second type of the tuple.
 * \tparam    T3           Third type of the tuple.
 */
template <typename StreamT, typename T1, typename T2, typename T3>
class BinaryReader<StreamT, std::tuple<T1, T2, T3>> {
 public:
    /**
     * Reads a tuple
     *
     * \param [in,out] s    A StreamT to process.
     * \param [in,out] d    A std::tuple&lt;T1,T2,T3&gt; to process.
     *
     * \return  A size_t.
     */
    static size_t Read(StreamT& s, std::tuple<T1, T2, T3>& d) {
        return BinaryRead<StreamT, T1>(s, std::get<0>(d)) +
               BinaryRead<StreamT, T2>(s, std::get<1>(d)) +
               BinaryRead<StreamT, T3>(s, std::get<2>(d));
    }
};

/*!
 * \class    BinaryReaderWriter<StreamT,std::tuple<T1,T2>>
 *
 * \brief    BinaryReaderWriter for tuple types.
 *
 * \tparam    StreamT      Type of the stream t.
 * \tparam    T1           First type of the tuple.
 * \tparam    T2           Second type of the tuple.
 * \tparam    T3           Third type of the tuple.
 */
template <typename StreamT, typename T1, typename T2, typename T3>
class BinaryWriter<StreamT, std::tuple<T1, T2, T3>> {
 public:
    /*!
     * \fn  static void Write(StreamT& s, const std::tuple<T1, T2, T3>& d)
     *
     * \brief   Writes data to stream.
     *
     * \param [in,out]  s   A StreamT to write to.
     * \param           d   A std::tuple&lt;T1,T2,T3&gt; to write.
     */
    static size_t Write(StreamT& s, const std::tuple<T1, T2, T3>& d) {
        return BinaryWrite<StreamT, T1>(s, std::get<0>(d)) +
               BinaryWrite<StreamT, T2>(s, std::get<1>(d)) +
               BinaryWrite<StreamT, T3>(s, std::get<2>(d));
    }
};
// \endcond 0    Enable doxygen

/*!
 * \class   BinaryReaderWriterForSequentialContainer
 *
 * \brief   A binary reader writer for sequential container types such as list, vector,...
 *
 * \tparam  StreamT Type of stream
 * \tparam  ContainerT  Type of container
 * \tparam  ElementT    Type of element
 */
template <typename StreamT, typename ContainerT, typename ElementT>
class BinaryReaderForSequentialContainer {
 public:
    /*!
     * \fn  static bool Read(StreamT& s, ContainerT& d)
     *
     * \brief   Read container of type ContainerT from stream to d.
     *          This is enabled only when ElementT can be copied with memcpy.
     *
     * \return  Whether the read is successful
     */
    template <typename T2 = ElementT>
    static ENABLE_IF_MEMCPY_ALLOWED_READ(T2, StreamT, size_t) Read(StreamT& s, ContainerT& d) {
        size_t size;
        if (!BinaryRead<StreamT, size_t>(s, size)) return 0;
        d.resize(size);
        size_t bytes = s.Read(&d[0], size * sizeof(T2));
        return bytes == size * sizeof(T2) ? bytes + sizeof(size_t) : 0;
    }

    /*!
     * \fn  static bool Read(StreamT& s, ContainerT& d)
     *
     * \brief   Read container of type ContainerT from stream to d.
     *          This is enabled only when ElementT cannot be copied with memcpy.
     *
     * \return  Whether the read is successful
     */
    template <typename T2 = ElementT>
    static ENABLE_IF_MEMCPY_NOT_ALLOWED_READ(T2, StreamT, size_t) Read(StreamT& s, ContainerT& d) {
        size_t size;
        if (!BinaryRead<StreamT, size_t>(s, size)) return 0;
        d.resize(size);
        size_t bytes_read = sizeof(size_t);
        for (size_t i = 0; i < size; i++) {
            size_t bytes = BinaryRead<StreamT, T2>(s, d[i]);
            if (!bytes) return 0;
            bytes_read += bytes;
        }
        return bytes_read;
    }
};

template <typename StreamT, typename ContainerT, typename ElementT>
class BinaryWriterForSequentialContainer {
 public:
    /*!
     * \fn  static void Write(StreamT& s, const ContainerT& d)
     *
     * \brief   Write container of type ContainerT to stream.
     *          This is enabled only when ElementT can be copied with memcpy.
     */
    template <typename T2 = ElementT>
    static ENABLE_IF_MEMCPY_ALLOWED_WRITE(T2, StreamT, size_t)
        Write(StreamT& s, const ContainerT& d) {
        BinaryWrite<StreamT, size_t>(s, d.size());
        s.Write(&d[0], d.size() * sizeof(T2));
        return sizeof(size_t) + sizeof(T2) * d.size();
    }

    /*!
     * \fn  static void Write(StreamT& s, const ContainerT& d)
     *
     * \brief   Write container of type ContainerT to stream.
     *          This is enabled only when ElementT cannot be copied with memcpy.
     */
    template <typename T2 = ElementT>
    static ENABLE_IF_MEMCPY_NOT_ALLOWED_WRITE(T2, StreamT, size_t)
        Write(StreamT& s, const ContainerT& d) {
        size_t size = BinaryWrite<StreamT, size_t>(s, d.size());
        for (size_t i = 0; i < d.size(); i++) {
            size += BinaryWrite<StreamT, T2>(s, d[i]);
        }
        return size;
    }
};

/*!
 * \class   BinaryReaderWriterForNonSequentialContainer
 *
 * \brief   A binary reader writer for non sequential container, such as
 *          map, set, etc.
 */
template <typename StreamT, typename ContainerT, typename ElementT>
class BinaryReaderForNonSequentialContainer {
 public:
    /*!
     * \fn  static bool BinaryReaderWriterForNonSequentialContainer::Read(StreamT& s, ContainerT& d)
     *
     * \brief   Reads from stream.
     *
     * \param [in,out]  s   A StreamT to read from.
     * \param [in,out]  d   A ContainerT to read into.
     *
     * \return  True if it succeeds, false if it fails.
     */
    static size_t Read(StreamT& s, ContainerT& d) {
        size_t size = 0;
        size_t bytes_read = 0;
        size_t r = BinaryRead<StreamT, size_t>(s, size);
        if (r == 0) return 0;
        bytes_read += r;
        for (size_t i = 0; i < size; i++) {
            ElementT e;
            r = BinaryRead<StreamT, ElementT>(s, e);
            if (r == 0) return 0;
            bytes_read += r;
            d.insert(d.end(), std::move(e));
        }
        return bytes_read;
    }
};

template <typename StreamT, typename ContainerT, typename ElementT>
class BinaryWriterForNonSequentialContainer {
 public:
    /*!
     * \fn  static void BinaryReaderWriterForNonSequentialContainer::Write(StreamT& s, const
     * ContainerT& d)
     *
     * \brief   Writes to a stream.
     *
     * \param [in,out]  s   A StreamT to write to.
     * \param           d   A ContainerT to write.
     */
    static size_t Write(StreamT& s, const ContainerT& d) {
        size_t size = BinaryWrite<StreamT, size_t>(s, d.size());
        for (const auto& e : d) {
            size += BinaryWrite<StreamT, ElementT>(s, e);
        }
        return size;
    }
};

template <typename StreamT>
class BinaryReader<StreamT, ::lgraph_api::FieldData> : public BinaryReaderForFieldData<StreamT> {};

template <typename StreamT>
class BinaryWriter<StreamT, ::lgraph_api::FieldData> : public BinaryWriterForFieldData<StreamT> {};

template <typename StreamT>
class BinaryReader<StreamT, std::string>
    : public BinaryReaderForSequentialContainer<StreamT, std::string, char> {};
template <typename StreamT>
class BinaryWriter<StreamT, std::string>
    : public BinaryWriterForSequentialContainer<StreamT, std::string, char> {};

template <typename StreamT, typename T>
class BinaryReader<StreamT, std::vector<T>>
    : public BinaryReaderForSequentialContainer<StreamT, std::vector<T>, T> {};
template <typename StreamT, typename T>
class BinaryWriter<StreamT, std::vector<T>>
    : public BinaryWriterForSequentialContainer<StreamT, std::vector<T>, T> {};

template <typename StreamT, typename K, typename V, typename Compare, typename Allocator>
class BinaryReader<StreamT, std::map<K, V, Compare, Allocator>>
    : public BinaryReaderForNonSequentialContainer<StreamT, std::map<K, V, Compare, Allocator>,
                                                   std::pair<K, V>> {};
template <typename StreamT, typename K, typename V, typename Compare, typename Allocator>
class BinaryWriter<StreamT, std::map<K, V, Compare, Allocator>>
    : public BinaryWriterForNonSequentialContainer<StreamT, std::map<K, V, Compare, Allocator>,
                                                   std::pair<K, V>> {};

template <typename StreamT, typename T, typename Compare, typename Allocator>
class BinaryReader<StreamT, std::set<T, Compare, Allocator>>
    : public BinaryReaderForNonSequentialContainer<StreamT, std::set<T, Compare, Allocator>, T> {};
template <typename StreamT, typename T, typename Compare, typename Allocator>
class BinaryWriter<StreamT, std::set<T, Compare, Allocator>>
    : public BinaryWriterForNonSequentialContainer<StreamT, std::set<T, Compare, Allocator>, T> {};

template <typename StreamT, typename K, typename V, typename Hash, typename Equal, typename Alloc>
class BinaryReader<StreamT, std::unordered_map<K, V, Hash, Equal, Alloc>>
    : public BinaryReaderForNonSequentialContainer<
          StreamT, std::unordered_map<K, V, Hash, Equal, Alloc>, std::pair<K, V>> {};
template <typename StreamT, typename K, typename V, typename Hash, typename Equal, typename Alloc>
class BinaryWriter<StreamT, std::unordered_map<K, V, Hash, Equal, Alloc>>
    : public BinaryWriterForNonSequentialContainer<
          StreamT, std::unordered_map<K, V, Hash, Equal, Alloc>, std::pair<K, V>> {};

template <typename StreamT, typename T, typename Hash, typename Equal, typename Alloc>
class BinaryReader<StreamT, std::unordered_set<T, Hash, Equal, Alloc>>
    : public BinaryReaderForNonSequentialContainer<StreamT,
                                                   std::unordered_set<T, Hash, Equal, Alloc>, T> {};
template <typename StreamT, typename T, typename Hash, typename Equal, typename Alloc>
class BinaryWriter<StreamT, std::unordered_set<T, Hash, Equal, Alloc>>
    : public BinaryWriterForNonSequentialContainer<StreamT,
                                                   std::unordered_set<T, Hash, Equal, Alloc>, T> {};

/*!
 * \fn  bool BinaryRead(StreamT& s, T& d)
 *
 * \brief   Read data of type T from binary stream s
 *
 * \param [in,out]  s   A StreamT to read from
 * \param [in,out]  d   A T to write into
 *
 * \return  True if it succeeds, false if it fails.
 */
template <typename StreamT, typename T>
size_t BinaryRead(StreamT& s, T& d) {
    return BinaryReader<StreamT, T>::Read(s, d);
}

template <typename EncodeType, typename ReturnType, typename StreamT>
size_t DecodeBinaryAsType(StreamT& s, ReturnType& d) {
    EncodeType v;
    size_t ret = BinaryRead(s, v);
    d = static_cast<ReturnType>(v);
    return ret;
}

/*!
 * \fn  template<typename StreamT, typename T> void BinaryWrite(StreamT&s, const T& d)
 *
 * \brief   Write data of type T into stream s as binary bytes
 *
 * \param [in,out]  s   A StreamT to write into
 * \param           d   A T to write
 */
template <typename StreamT, typename T>
size_t BinaryWrite(StreamT& s, const T& d) {
    return BinaryWriter<StreamT, T>::Write(s, d);
}
}  // namespace fma_common
