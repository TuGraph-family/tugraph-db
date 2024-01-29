//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\data_file.h.
 *
 * \brief   Declares the data file class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once

#include <string>

#include "fma-common/binary_read_write_helper.h"
#include "fma-common/local_file_stream.h"
#include "fma-common/utils.h"

namespace fma_common {
/*!
 * \class   DataFileReader
 *
 * \brief   Abstraction for a read-only binary data file.
 *          Header is at the beginning of file, and T must be a POD type.
 * \tparam  T   type of data, must be POD type
 * \tparam  H   type of header, must be POD type
 *
 * \note    A copy of header will be stored in each instance of DataReader,
 *          so make sure header is not large.
 */
template <typename T, typename H = void>
class DataFileReader {
    using HT = DUMMY_TYPE_IF_VOID(H);
#define HS SizeOfType<H>::value()
    HT header_;
    std::string path_;
    InputLocalFileStream file_;

 public:
    /*!
     * \class   DataBatch
     *
     * \brief   A data batch.
     */
    class DataBatch {
        T* ptr_ = nullptr;
        size_t size_ = 0;

     public:
        DataBatch() {}

        ~DataBatch() { free(ptr_); }

        void Reserve(size_t size) { ptr_ = (T*)realloc(ptr_, size * sizeof(T)); }

        void Resize(size_t size) { size_ = size; }

        size_t Size() const { return size_; }

        const T& operator[](size_t i) const { return ptr_[i]; }

        T& operator[](size_t i) { return ptr_[i]; }
    };

    /*!
     * \class   BatchIterator
     *
     * \brief   Iterator which returns list of batches
     */
    class BatchIterator {
        DataFileReader<T, H>& reader_;
        size_t batch_size_;
        DataBatch curr_batch_;

     public:
        BatchIterator(DataFileReader& reader, size_t batch_size)
            : reader_(reader), batch_size_(batch_size) {
            curr_batch_.Reserve(batch_size_);
            Next();
        }

        /*
         * \brief Reset iterator to the beginning
         */
        // void Reset() {
        //    reader_.Reset();
        //    curr_batch_.Reserve(batch_size_);
        //    Next();
        //}

        const DataBatch& operator*() const { return curr_batch_; }

        const DataBatch* operator->() const { return &curr_batch_; }

        /*
         * \brief Whether the iterator is valid
         */
        operator bool() const { return curr_batch_.Size() > 0; }

        /*
         * \brief Go to the next edge batch
         */
        void Next() {
            uint64_t readBytes = reader_.file_.Read(&curr_batch_[0], batch_size_ * sizeof(T));
            FMA_CHECK_EQ(readBytes % sizeof(T), 0)
                << "Data file " << reader_.path_ << " is corrupted.";
            curr_batch_.Resize(readBytes / sizeof(T));
        }
    };

    using DataType = T;

    DataFileReader()= default;

    /*!
     * \fn  DataFileReader::DataFileReader(const std::string& path)
     *
     * \brief   Constructor.
     *
     * \param   path    Full pathname of the file.
     */
    explicit DataFileReader(const std::string& path) : path_(path) { Open(path); }

    /*!
     * \fn  void DataFileReader::Open(const std::string& path)
     *
     * \brief   Opens the given file.
     *
     * \param   path    Full pathname of the file.
     */
    void Open(const std::string& path) {
        file_.Open(path, 0);
        if (!file_.Good()) {
            LOG_ERROR() << "Error opening file " << path << " for read";
        }
        size_t read_size = file_.Read(&header_, HS);
        if (read_size != HS) {
            LOG_ERROR() << "File " << path << " seem to be corruptted: "
                      << "Expected header size=" << HS << ", but read " << read_size;
        }
    }

    /*!
     * \fn  bool DataFileReader::IsOpen() const
     *
     * \brief   Query if this file reader is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return file_.Good(); }

    /*!
     * \fn  DataFileReader::const HT& Header() const
     *
     * \brief   Get the file header. This function is enabled only when
     *          H is not void.
     */
    ENABLE_IF_NOT_VOID(H, const HT&) Header() const {
        CheckOpen();
        return header_;
    }

    /*!
     * \fn  BatchIterator DataFileReader::Iterator(size_t batch_size = 1024 * 1024)
     *
     * \brief   Get a batch iterator with the given batch size.
     *
     * \param   batch_size  (Optional) Size of the batch.
     *
     * \return  A BatchIterator.
     */
    BatchIterator Iterator(size_t batch_size = 1024 * 1024) {
        CheckOpen();
        return BatchIterator(*this, batch_size);
    }

 private:
    void CheckOpen() const {
        if (!IsOpen()) {
            throw std::runtime_error("Failed to open file " + file_.Path() + " for write.");
        }
    }
};

/*!
 * \class   DataFileWriter
 *
 * \brief   Abstraction for a append-only binary data file.
 *          Header is at the beginning of file, and T must be a POD type.
 *
 * \tparam  T   type of data, must be POD type
 * \tparam  H   type of header, must be POD type
 */
template <typename T, typename H = void>
class DataFileWriter {
    using HT = DUMMY_TYPE_IF_VOID(H);
#define HS SizeOfType<H>::value()

    static const size_t DEFAULT_BLOCK_SIZE = 128 * 1024 * 1024;
    OutputLocalFileStream file_;

 public:
    DataFileWriter() {}

    /*!
     * \fn  template<typename H_ = H> DataFileWriter::DataFileWriter(TYPE_IF_VOID(H_, const
     * std::string&) path, size_t block_size = DEFAULT_BLOCK_SIZE, size_t n_buffer = 1)
     *
     * \brief   Constructor.
     *
     * \param   path        Full pathname of the file.
     * \param   block_size  (Optional) Size of each block.
     * \param   n_buffer    (Optional) The number of buffers to use.
     */
    template <typename H_ = H>
    DataFileWriter(TYPE_IF_VOID(H_, const std::string&) path,
                   size_t block_size = DEFAULT_BLOCK_SIZE, size_t n_buffer = 1) {}

    /*!
     * \fn  template<typename H_ = H> DataFileWriter::DataFileWriter(const std::string& path,
     * typename std::enable_if<!std::is_void<H_>::value, const HT&>::type header, size_t block_size
     * = DEFAULT_BLOCK_SIZE, size_t n_buffer = 1)
     *
     * \brief   Constructor. Enabled only when H is not void
     *
     * \param   path        Full pathname of the file.
     * \param   header      Header of the datafile
     * \param   block_size  (Optional) Size of each block.
     * \param   n_buffer    (Optional) The number of buffers to use.
     */
    template <typename H_ = H>
    DataFileWriter(const std::string& path,
                   typename std::enable_if<!std::is_void<H_>::value, const HT&>::type header,
                   size_t block_size = DEFAULT_BLOCK_SIZE, size_t n_buffer = 1) {
        Open(path, header, block_size, n_buffer);
    }

    /*!
     * \fn  template<typename H_ = H> void DataFileWriter::Open(const std::string& path, typename
     * std::enable_if<!std::is_void<H_>::value, const HT&>::type header, size_t block_size =
     * DEFAULT_BLOCK_SIZE, size_t n_buffer = 1)
     *
     * \brief   Opens the given file. Enabled only when H is not void.
     *
     * \param   path        Full pathname of the file.
     * \param   header      Header of the datafile
     * \param   block_size  (Optional) Size of each block.
     * \param   n_buffer    (Optional) The number of buffers to use.
     */
    template <typename H_ = H>
    void Open(const std::string& path,
              typename std::enable_if<!std::is_void<H_>::value, const HT&>::type header,
              size_t block_size = DEFAULT_BLOCK_SIZE, size_t n_buffer = 1) {
        CheckNotOpen();
        file_.Open(path, block_size, std::ofstream::trunc);
        BinaryWrite(file_, header);
    }

    /*!
     * \fn  DataFileWriter::ENABLE_IF_VOID(H, void) Open(const std::string& path, size_t block_size
     * = DEFAULT_BLOCK_SIZE, size_t n_buffer = 1)
     *
     * \brief   Opens the given file. Enabled only when H is void.
     *
     * \param   path        Full pathname of the file.
     * \param   header      Header of the datafile
     * \param   block_size  (Optional) Size of each block.
     * \param   n_buffer    (Optional) The number of buffers to use.
     */
    ENABLE_IF_VOID(H, void)
    Open(const std::string& path, size_t block_size = DEFAULT_BLOCK_SIZE, size_t n_buffer = 1) {
        CheckNotOpen();
        file_.Open(path, block_size, std::ofstream::trunc);
    }

    /*!
     * \fn  bool DataFileWriter::IsOpen() const
     *
     * \brief   Query if this file is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return file_.Good(); }

    /*!
     * \fn  void DataFileWriter::Write(const T& d)
     *
     * \brief   Writes the given d into file.
     *
     * \param   d   The d to write.
     */
    void Write(const T& d) {
        CheckOpen();
        file_.Write(d);
    }

    /*!
     * \fn  void DataFileWriter::Write(const T* ptr, size_t size)
     *
     * \brief   Writes an array to data into the file.
     *
     * \param   ptr     The pointer.
     * \param   size    The size.
     */
    void Write(const T* ptr, size_t size) {
        CheckOpen();
        file_.Write(ptr, size * sizeof(T));
    }

    /*!
     * \fn  void DataFileWriter::Write(const std::vector<T>& v)
     *
     * \brief   Writes a vector into the file.
     *
     * \param   v   The vector to write.
     */
    void Write(const std::vector<T>& v) {
        CheckOpen();
        file_.Write(v.data(), v.size());
    }

    /*!
     * \fn  void DataFileWriter::Close()
     *
     * \brief   Closes this file.
     */
    void Close() { file_.Close(); }

 private:
    void CheckOpen() const {
        if (!IsOpen()) {
            throw std::runtime_error("Failed to open file " + file_.Path() + " for write.");
        }
    }

    void CheckNotOpen() const { FMA_ASSERT(!IsOpen()) << "DataFileWriter is already opened"; }
};
}  // namespace fma_common
