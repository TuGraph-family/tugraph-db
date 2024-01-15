//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\binary_buffer.h.
 *
 * \brief   Declares the binary buffer class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once

#include <algorithm>
#include <cstring>
#include <memory>

#include "fma-common/assert.h"
#include "fma-common/type_traits.h"

namespace fma_common {
/*!
 * \class   BinaryBuffer
 *
 * \brief   Buffer for storing binary data, allows read and write.
 */
class BinaryBuffer {
    /** Whether to shrink(realloc) the buffer when we read from the front of the buffer. */
    static const bool SHRINK_WITH_GET = false;
    /** Size to grow when we go out of buffer size. */
    static const size_t GROW_SIZE = 1024;
    /** The reserved header space, which we can use in WriteHead() */
    static const size_t RESERVED_HEADER_SPACE = 64;

    /*! \brief   The buffer */
    char* buf_ = nullptr;
    /*! \brief   const buffers should not be written into */
    bool const_buf_ = false;
    /*! \brief   end of buffer0 */
    size_t pend_ = 0;
    /*! \brief   start of get */
    size_t gpos_ = 0;
    /*! \brief   start of put */
    size_t ppos_ = 0;

 public:
    DISABLE_COPY(BinaryBuffer);

    BinaryBuffer() : buf_(nullptr), const_buf_(false), pend_(0), gpos_(0), ppos_(0) {}

    /*!
     * \fn  void BinaryBuffer::InitOstream()
     *
     * \brief   Initializes the output stream.
     */
    void InitOstream() {
        /*!
         * \brief Since we might further push some header information such as message ID
         *        into this buffer, we would like to reserve some space for the header info.
         *        Here we allocate 128 bytes and reserve the first 64 bytes as header space.
         */
        FMA_ASSERT(buf_ == nullptr) << "trying to init an already-initialized buffer";
        buf_ = (char*)malloc(RESERVED_HEADER_SPACE * 2);
        const_buf_ = false;
        pend_ = RESERVED_HEADER_SPACE * 2;
        gpos_ = RESERVED_HEADER_SPACE;
        ppos_ = RESERVED_HEADER_SPACE;
    }

    /*!
     * \fn  BinaryBuffer::BinaryBuffer(const char * buf, size_t size)
     *
     * \brief   Create a new instance using an existing buffer. Since the
     *          buffer is already managed outside BinaryBuffer, we don't
     *          want to free that space in destructor.
     *
     * \param   buf     The buffer.
     * \param   size    Buffer size.
     */
    BinaryBuffer(const char* buf, size_t size)
        : buf_(const_cast<char*>(buf)), const_buf_(true), pend_(size), gpos_(0), ppos_(size) {}

    /*!
     * \fn  BinaryBuffer::BinaryBuffer(size_t size)
     *
     * \brief   Create a new instance using a new buffer.
     *
     * \param   size    Buffer size.
     */
    explicit BinaryBuffer(size_t size)
        : buf_((char*)malloc(size)), const_buf_(false), pend_(size), gpos_(0), ppos_(0) {}

    /*!
     * \fn  BinaryBuffer::BinaryBuffer(BinaryBuffer&& rhs)
     *
     * \brief   Move constructor.
     *
     * \param [in,out]  rhs The right hand side.
     */
    BinaryBuffer(BinaryBuffer&& rhs) {
        buf_ = rhs.buf_;
        const_buf_ = rhs.const_buf_;
        pend_ = rhs.pend_;
        gpos_ = rhs.gpos_;
        ppos_ = rhs.ppos_;
        rhs.buf_ = nullptr;
        rhs.const_buf_ = false;
        rhs.pend_ = 0;
        rhs.gpos_ = 0;
        rhs.ppos_ = 0;
    }

    /*!
     * \fn  BinaryBuffer::~BinaryBuffer()
     *
     * \brief   Destructor.
     */
    ~BinaryBuffer() {
        if (!const_buf_) {
            free(buf_);
        }
    }

    /*!
     * \fn  void BinaryBuffer::Swap(BinaryBuffer & rhs)
     *
     * \brief   Swaps the two buffers.
     *
     * \param [in,out]  rhs The right hand side.
     */
    void Swap(BinaryBuffer& rhs) {
        std::swap(const_buf_, rhs.const_buf_);
        std::swap(buf_, rhs.buf_);
        std::swap(pend_, rhs.pend_);
        std::swap(gpos_, rhs.gpos_);
        std::swap(ppos_, rhs.ppos_);
    }

    /*!
     * \fn  char * BinaryBuffer::GetBuf()
     *
     * \brief   Gets the pointer to valid content.
     *
     * \return  Pointer to the next read position, nullptr if empty buffer.
     */
    char* GetBuf() { return buf_ + gpos_; }

    /*!
     * \fn  void BinaryBuffer::DetachBuf(void** buf, size_t* size)
     *
     * \brief   Detach the underlying memory and give the ownership to
     *          user. If it is a constant buffer, a copy is given.
     *
     * \note    The returned buffer MUST be released with FreeDetachedBuf()
     *
     * \param [in,out]  buf     Stores the memory pointer.
     * \param [in,out]  size    Stores the size of the memory.
     */
    void DetachBuf(void** buf, size_t* size) {
        *size = GetSize();
        if (const_buf_) {
            LOG_WARN() << "Detaching a constant stream buffer will result in a memory copy";
            void* new_buf = malloc(GetSize() + sizeof(void*));
            *buf = new_buf;
            *(void**)(new_buf) = new_buf;
            memcpy((char*)new_buf + sizeof(void*), GetBuf(), GetSize());
        } else {
            void* orig_buf = buf_;
            WriteHead((const char*)&orig_buf, sizeof(void*));
            if (orig_buf != buf_) {
                // buffer was reallocated due to too large header
                *(void**)GetBuf() = buf_;
            }
            *buf = GetBuf() + sizeof(void*);
        }
        // now detach the buffer
        buf_ = nullptr;
        const_buf_ = false;
        ppos_ = 0;
        gpos_ = 0;
        pend_ = 0;
    }

    /*!
     * \fn  static void BinaryBuffer::FreeDetachedBuf(void* detached_buf, void* hint)
     *
     * \brief   Free a detached memory block.
     *
     * \param [in]  detached_buf    Pointer to a detached memory block
     * \param [in]  hint            Hint, currently unused.
     */
    static void FreeDetachedBuf(void* detached_buf, void* hint) {
        void* buf_head = *((void**)detached_buf - 1);
        free(buf_head);
    }

    /*!
     * \fn  void BinaryBuffer::SetBuf(const char * buf, size_t size)
     *
     * \brief   Sets the memory block of this buffer to a constant buffer.
     *          Ownership of this memory block is not transffered.
     *
     * \param   buf     The buffer.
     * \param   size    The buffer size.
     */
    void SetBuf(const char* buf, size_t size) {
        if (!const_buf_) free(buf_);
        const_buf_ = true;
        buf_ = const_cast<char*>(buf);
        ppos_ = size;
        gpos_ = 0;
        pend_ = size;
    }

    /*!
     * \fn  void BinaryBuffer::SetBuf(char * buf, size_t size)
     *
     * \brief   Sets the memory block of this buffer to a non-constant
     *          buffer. Ownship of this memory block is transferred to
     *          the BinaryBuffer.
     *
     * \param  buf     If non-null, the buffer.
     * \param  size    The buffer size.
     */
    void SetBuf(char* buf, size_t size) {
        if (!const_buf_) free(buf_);
        const_buf_ = false;
        buf_ = buf;
        ppos_ = size;
        gpos_ = 0;
        pend_ = size;
    }

    /*!
     * \fn  size_t BinaryBuffer::GetSize()
     *
     * \brief   Gets the number of valid bytes in buffer.
     *
     * \return  Number of bytes in buffer that can be read.
     */
    size_t GetSize() { return ppos_ - gpos_; }

    /*!
     * \fn  void BinaryBuffer::Write(const void * buf, size_t size)
     *
     * \brief   Writes to the buffer.
     *
     * \param   buf     The buffer to be written.
     * \param   size    Number of bytes to be written.
     */
    void Write(const void* buf, size_t size) {
        FMA_ASSERT(!const_buf_) << "writing into a const buffer is not allowed.";
        if (buf_ == nullptr) {
            InitOstream();
        }
        size_t new_size = size + ppos_;
        if (new_size > pend_) {
            // reallocate buffer
            // LOG() << "buffer is full, reallocating. pend_=" << pend_
            //    << ", new_size=" << new_size;
            new_size = std::max(new_size, ppos_ * 2);
            char* new_buf = (char*)realloc(buf_, new_size);
            FMA_ASSERT(new_buf != nullptr) << "realloc failed";
            buf_ = new_buf;
            pend_ = new_size;
        }
        memcpy(buf_ + ppos_, buf, size);
        ppos_ += size;
    }

    /*!
     * \fn  size_t BinaryBuffer::Read(void * buf, size_t size)
     *
     * \brief   Reads from the buffer.
     *
     * \param   buf     Pointer to a buffer where bytes will be read into.
     * \param   size    The maximum size of the buffer.
     *
     * \return  Number of bytes actually read.
     */
    size_t Read(void* buf, size_t size) {
        FMA_CHECK(gpos_ + size <= ppos_) << "reading beyond the array: required size=" << size
                                         << ", actual size=" << ppos_ - gpos_;
        memcpy(buf, buf_ + gpos_, size);
        gpos_ += size;
        if (gpos_ > GROW_SIZE && SHRINK_WITH_GET && !const_buf_) {
            memmove(buf_, buf_ + gpos_, ppos_ - gpos_);
            char* new_buf = (char*)realloc(buf_, pend_ - gpos_);
            FMA_ASSERT(new_buf != nullptr) << "realloc failed";
            buf_ = new_buf;
            pend_ -= gpos_;
            ppos_ -= gpos_;
            gpos_ = 0;
        }
        return size;
    }

    /*!
     * \fn  void BinaryBuffer::WriteHead(const T & val)
     *
     * \brief   Writes an element to the head of the buffer.
     *
     * \note    T must be a POD, because we will write it with memcpy.
     *
     * \param   val The value to be written.
     */
    template <class T>
    void WriteHead(const T& val) {
        static_assert(std::is_pod<T>::value,
                      "BinaryBuffer::write_head(T) not implemented for this type.");
        WriteHead((char*)&val, sizeof(val));
    }

    /*!
     * \fn  void BinaryBuffer::WriteHead(const char * buf, size_t size)
     *
     * \brief   Writes content to the head of the BinaryBuffer.
     *
     * \note    64 bytes is reserved at the head of the buffer, writing more
     *          than that amount will cause a realloc, which should be avoided.
     *
     * \param   buf     The buffer to be written.
     * \param   size    Number of bytes to be written.
     */
    void WriteHead(const char* buf, size_t size) {
        FMA_ASSERT(!const_buf_) << "writing into a const buffer is not allowed.";
        if (buf_ == nullptr) {
            InitOstream();
        }
        if (gpos_ < size) {
            // this should rarely happen, since we already have 64 bytes reserved
            LOG_WARN() << "reallocating due to write_head, possible performance loss. "
                       << "gpos_=" << gpos_ << ", size=" << size;
            size_t new_size = std::max(size + ppos_, ppos_ + RESERVED_HEADER_SPACE);
            char* new_buf = (char*)malloc(new_size);
            FMA_ASSERT(new_buf != nullptr) << "realloc failed";
            // copy existing contents to the new buffer
            size_t new_gpos = new_size - (ppos_ - gpos_);
            memcpy(new_buf + new_gpos, buf_ + gpos_, ppos_ - gpos_);
            free(buf_);
            buf_ = new_buf;
            gpos_ = new_gpos;
            ppos_ = pend_ = new_size;
        }
        gpos_ -= size;
        memcpy(buf_ + gpos_, buf, size);
    }
};
}  // namespace fma_common
