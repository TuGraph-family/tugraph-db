//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\hard_container.h.
 *
 * \brief   Declares the hard container class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <cstdint>
#include <type_traits>

#include "fma-common/utils.h"

#ifdef _WIN32
#include "fma-common/hard_array_windows.h"
#else
#include "fma-common/hard_array_linux.h"
#endif

namespace fma_common {
/*!
 * \class   HardContainer
 *
 * \brief   A hard container is a template container that uses mmap()
 *          memory. Thus it can be used to store data arrays that are
 *          larger than memory.
 *
 * \tparam  T   Type of the elements stored in the array.
 */
template <typename T>
class HardContainer {
#ifdef _WIN32
    static_assert(std::is_trivially_copyable<T>::value, "Only supports trivial types.");
#else
    static_assert(std::is_pod<T>::value, "Only supports trivial types.");
#endif

    HardArray memory_;
    size_t size_;

 public:
    DISABLE_COPY(HardContainer);

    /*!
     * \fn  HardContainer::HardContainer(const std::string& filename, bool read_only = true)
     *
     * \brief   Create a hard container by reading the file, array size
     *          will be the same as the file size multiplied by sizeof(T).
     *
     * \param   filename    Filename of the file.
     * \param   read_only   (Optional) True if read only.
     */
    explicit HardContainer(const std::string& filename, bool read_only = true)
        : memory_(filename, read_only, 0) {
        size_ = memory_.Size() / sizeof(T);
    }

    /*!
     * \fn  HardContainer::HardContainer(size_t size, const std::string& filename)
     *
     * \brief   Create a hard container with empty content, and write content
     *          into file when HardContainer is destroyed.
     *
     * \param   size        Number of elements in the array.
     * \param   filename    Path of the file.
     */
    HardContainer(size_t size, const std::string& filename)
        : memory_(filename, false, size * sizeof(T)) {
        size_ = size;
    }

    /*!
     * \fn  size_t HardContainer::Size() const
     *
     * \brief   Gets the number of elements in the array.
     *
     * \return  Number of elements in the array.
     */
    size_t Size() const { return size_; }

    /*!
     * \fn  T* HardContainer::Data()
     *
     * \brief   Gets modifiable pointer to the array.
     *
     * \return  Modifiable pointer to the array.
     */
    T* Data() { return (T*)(memory_.Data()); }

    /*!
     * \fn  const T* HardContainer::Data() const
     *
     * \brief   Gets const pointer to the array.
     *
     * \return  Const pointer to the array.
     */
    const T* Data() const { return (T*)(memory_.Data()); }

    /*!
     * \fn  const T& HardContainer::operator[](size_t idx) const
     *
     * \brief   Gets const reference to the ith element.
     *
     * \param   idx Zero-based index of the array.
     *
     * \return  Const reference to the ith element.
     */
    const T& operator[](size_t idx) const { return ((T*)(memory_.Data()))[idx]; }

    /*!
     * \fn  T& HardContainer::operator[](size_t idx)
     *
     * \brief   Gets reference to the ith element.
     *
     * \param   idx Zero-based index of the array.
     *
     * \return  Reference to the ith element.
     */
    T& operator[](size_t idx) { return ((T*)(memory_.Data()))[idx]; }
};

/*!
 * \class   HardBitmap
 *
 * \brief   A bitmap using mmap() memory
 */
class HardBitmap {
    HardArray memory_;
    size_t size_;
    /*! \brief   Pointer to the underlying memory, with 1-byte addressing */
    unsigned char* chars_;
    size_t n_chars_;
    /*! \brief   Pointer to the underlying memory, with 8-byte addressing,
     *           used in Any() to quickly find if the array is all 0 */
    uint64_t* words_;
    size_t n_words_;

    static const unsigned char ONE = 0x1;

 public:
    DISABLE_COPY(HardBitmap);

    /*!
     * \fn  HardBitmap::HardBitmap(size_t size, const std::string& filename)
     *
     * \brief   Constructor.
     *
     * \param   size        The number of bits in the array.
     * \param   filename    Filename of the file.
     */
    HardBitmap(size_t size, const std::string& filename)
        : memory_(filename, false, AlignTo8(ToByte(size))) {
        size_ = size;
        chars_ = (unsigned char*)(memory_.Data());
        n_chars_ = memory_.Size();
        words_ = (uint64_t*)(memory_.Data());
        n_words_ = memory_.Size() / 8;
    }

    /*!
     * \fn  size_t HardBitmap::Size() const
     *
     * \brief   Gets the number of bits in the array.
     *
     * \return  Number of bits in the array.
     */
    size_t Size() const { return size_; }

    /*!
     * \fn  bool HardBitmap::operator[](size_t idx) const
     *
     * \brief   Array indexer operator.
     *
     * \param   idx Zero-based index of the.
     *
     * \return  The ith bit in the array.
     */
    bool operator[](size_t idx) const { return (GetByte(idx) & (ONE << (idx % 8))) != 0; }

    /*!
     * \fn  bool HardBitmap::None() const
     *
     * \brief   No element is true.
     *
     * \return  True if all elements are false.
     */
    bool None() const { return !Any(); }

    /*!
     * \fn  bool HardBitmap::Any() const
     *
     * \brief   At least one element is true.
     *
     * \return  True if there is any element that is true.
     */
    bool Any() const {
        for (size_t i = 0; i < n_words_; i++) {
            if (words_[i] != 0) {
                return true;
            }
        }
        return false;
    }

    /*!
     * \fn  void HardBitmap::Set(size_t idx)
     *
     * \brief   Sets the ith element to true.
     *
     * \param   idx Zero-based index.
     */
    void Set(size_t idx) { GetByte(idx) |= (ONE << (idx % 8)); }

    /*!
     * \fn  void HardBitmap::Reset(size_t idx)
     *
     * \brief   Sets the ith element to false.
     *
     * \param   idx Zero-based index.
     */
    void Reset(size_t idx) { GetByte(idx) &= ((ONE << (idx % 8)) == 0); }

 private:
    static inline size_t ToByte(size_t size) { return (size + 7) / 8; }

    static inline size_t AlignTo8(size_t size) { return ((size + 7) / 8) * 8; }

    const unsigned char& GetByte(size_t idx) const { return chars_[idx / 8]; }

    unsigned char& GetByte(size_t idx) { return chars_[idx / 8]; }
};
}  // namespace fma_common
