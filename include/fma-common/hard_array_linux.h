//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\hard_array_linux.h.
 *
 * \brief   Declares the hard array linux class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

namespace fma_common {
/*!
 * \class   HardArray
 *
 * \brief   A hard array is an mmap backed array, so it can be larger than
 *          memory.
 */
class HardArray {
    const std::string filename_;
    size_t size_;
    int fd_;
    char* memory_;

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
    HardArray(const std::string& filename, bool read_only, size_t size) : filename_(filename) {
        int64_t file_size = FileSize(filename);
        if (size == 0 && file_size < 0) {
            FMA_LOG() << "error opening file " << filename << " for read";
            FMA_ASSERT(false);
        }
        size_ = (size == 0) ? file_size : size;
        if (read_only) {
            fd_ = open(filename_.c_str(), O_RDONLY);
            FMA_ASSERT(fd_ != -1) << "Error opening file " << filename
                                  << " for read. errno=" << errno;
            memory_ = (char*)mmap(0, size_, PROT_READ, MAP_SHARED, fd_, 0);
            FMA_ASSERT(memory_ != MAP_FAILED)
                << "Error mmaping file " << filename << ". errno=" << errno;
        } else {
            fd_ = open(filename_.c_str(), O_RDWR | O_CREAT, (mode_t)0600);
            FMA_ASSERT(fd_ != -1) << "Error opening file " << filename
                                  << " for write. errno=" << errno;
            int r = ftruncate(fd_, size_);
            FMA_ASSERT(r == 0);
            memory_ = (char*)mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            FMA_ASSERT(memory_ != MAP_FAILED)
                << "Error mmaping file " << filename << ". errno=" << errno;
        }
    }

    ~HardArray() {
        int r = munmap(memory_, size_);
        FMA_ASSERT(r == 0);
        close(fd_);
    }

    /*!
     * \fn  const char* HardArray::Data() const
     *
     * \brief   Gets the pointer to the data array.
     *
     * \return  Pointer to the data array
     */
    const char* Data() const { return memory_; }

    /*!
     * \fn  char* HardArray::Data()
     *
     * \brief   Gets the modifiable pointer to the data array.
     *
     * \return  Pointer to the data array
     */
    char* Data() { return memory_; }

    /*!
     * \fn  size_t HardArray::Size() const
     *
     * \brief   Gets the size of the memory block, in bytes.
     *
     * \return  Size of the memory block in bytes.
     */
    size_t Size() const { return size_; }

 private:
    int64_t FileSize(const std::string& filename) {
        struct stat file_stat;
        int r = stat(filename.c_str(), &file_stat);
        return (r == 0) ? file_stat.st_size : -1;
    }
};
}  // namespace fma_common
