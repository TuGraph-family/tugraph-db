//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\hard_array_windows.h.
 *
 * \brief   Mocks the HardArray class on Windows.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "fma-common/type_traits.h"

namespace fma_common {
/*!
 * \class   HardArray
 *
 * \brief   A hard array is an mmap backed array, so it can be larger than
 *          memory.
 *
 * \note    Currently the implementation on Windows is a mock class, which
 *          stores all the data in memory. So it should only be used for
 *          testing purpose. No performace guarantee is provided.
 */
class HardArray {
    const std::string filename_;
    bool read_only_;
    std::vector<char> memory_;

 public:
    DISABLE_COPY(HardArray);

    /*!
     * \fn  HardArray::HardArray(const std::string& filename, bool read_only, size_t size)
     *
     * \brief   Open a file and map it into memory
     *          If readOnly == true and size == 0, then we read the whole file,
     *          otherwise, the array will have exactly the size.
     *          If readOnly == false and fileSize != size, then the file will be
     *          extended or truncated.
     *
     * \param   filename    Filename of the file.
     * \param   read_only   True if read only.
     * \param   size        The size.
     */
    explicit HardArray(const std::string& filename, bool read_only = true, size_t size = 0)
        : filename_(filename), read_only_(read_only) {
        int64_t file_size = FileSize(filename);
        if (size == 0 && file_size < 0) {
            LOG_INFO() << "error opening file " << filename << " for read";
            FMA_ASSERT(false);
        }
        size_t s = (size == 0) ? file_size : size;
        memory_.resize(s);
        if (file_size > 0) {
            std::ifstream in(filename, std::ios_base::binary);
            in.read(&memory_[0], s);
        }
    }

    ~HardArray() {
        if (!read_only_) {
            std::ofstream os(filename_, std::ios_base::binary | std::ios_base::trunc);
            os.write(memory_.data(), memory_.size());
        }
    }

    /*!
     * \fn  char* HardArray::Data()
     *
     * \brief   Gets the modifiable pointer to the data array.
     *
     * \return  Pointer to the data array
     */
    char* Data() { return &memory_[0]; }

    /*!
     * \fn  const char* HardArray::Data() const
     *
     * \brief   Gets the pointer to the data array.
     *
     * \return  Pointer to the data array
     */
    const char* Data() const { return memory_.data(); }

    /*!
     * \fn  size_t HardArray::Size() const
     *
     * \brief   Gets the size of the memory block, in bytes.
     *
     * \return  Size of the memory block in bytes.
     */
    size_t Size() const { return memory_.size(); }

 private:
    int64_t FileSize(const std::string& filename) {
        std::ifstream is(filename, std::ios_base::binary);
        if (!is.good()) {
            return -1;
        } else {
            auto p = is.tellg();
            is.seekg(0, std::ios_base::end);
            return is.tellg() - p;
        }
    }
};
}  // namespace fma_common
