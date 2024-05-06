//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#if FMA_HAS_LIBHDFS
/*!
 * \file    fma-common\libhdfs_stream.h.
 *
 * \brief   Declares the hdfs stream class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include "libhdfs3/hdfs.h"

#include "fma-common/file_stream.h"
#include "fma-common/string_util.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"
#define CREATE_HDFS_INSTANCE 0

namespace fma_common {
/*!
 * \class   HdfsFile
 *
 * \brief   Representation of a HDFS file, provides basic open/close and read/write.
 *
 * \note    This class uses libhdfs to access HDFS. So it requires linking
 *          with libhdfs and libjvm. Also, libhdfs requires setting
 *          CLASSPATH to the .jar files of hadoop.
 */
class HdfsFile {
 public:
    DISABLE_COPY(HdfsFile);
    HdfsFile() {}

    ~HdfsFile() { Close(); }

    /*!
     * \fn  HdfsFile::HdfsFile(const std::string& uri, int flags)
     *
     * \brief   Open an hdfs file stored in uri, with POSIX flags
     *
     * \param   uri     URI of the file
     *                  Must start with hdfs://host:port/ or webhdfs://host:port/
     * \param   flags   POSIX file flags. Following flags are supported:
     *                      O_RDONLY
     *                      O_WRONLY(meaning create or overwrite i.e., implies O_TRUNCAT)
     *                      O_WRONLY | O_APPEND
     *                  Other flags are generally ignored other than(O_RDWR || (O_EXCL & O_CREAT))
     *                  which return NULL and set errno equal ENOTSUP.
     * \param   block_size  Block size to use for the file. Pass 0 if you want
     *                      to use the default configured value in HDFS.
     */
    HdfsFile(const std::string& uri, int flags, int block_size = 0) {
        Open(uri, flags, block_size);
    }

    /*!
     * \fn  bool HdfsFile::Open(const std::string& uri, int flags)
     *
     * \brief   Open an hdfs file stored in uri, with POSIX flags
     *
     * \param   uri     URI of the file
     *                  Must start with hdfs://host:port/ or webhdfs://host:port/
     * \param   flags   POSIX file flags. Following flags are supported:
     *                      O_RDONLY
     *                      O_WRONLY(meaning create or overwrite i.e., implies O_TRUNCAT)
     *                      O_WRONLY | O_APPEND
     *                  Other flags are generally ignored other than(O_RDWR || (O_EXCL & O_CREAT))
     *                  which return NULL and set errno equal ENOTSUP.
     * \param   block_size  Block size to use for the file. Pass 0 if you want
     *                      to use the default configured value in HDFS.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Open(const std::string& uri, int flags, int block_size = 0) {
        Close();
        if (!path_.FromString(uri)) return false;
#if CREATE_HDFS_INSTANCE
        hdfs_fs_ = hdfsConnectNewInstance(path_.Host().c_str(), path_.Port());
#else
        hdfs_fs_ = hdfsConnect(path_.Host().c_str(), path_.Port());
#endif
        if (!hdfs_fs_) {
            FMA_WARN() << "failed to connect to name node: " << path_.Host() << ":" << path_.Port();
            return false;
        }
        hdfs_file_ = hdfsOpenFile(hdfs_fs_, path_.Path().c_str(), flags, 0, 0, 0);
        if (!hdfs_file_) {
            FMA_WARN() << "failed to open file: " << path_.Path();
            return false;
        }
        hdfsFileInfo* info = hdfsGetPathInfo(hdfs_fs_, path_.Path().c_str());
        block_size_ = (size_t)info->mBlockSize;
        hdfsFreeFileInfo(info, 1);
        return true;
    }

    /*!
     * \fn  void HdfsFile::Close()
     *
     * \brief   Closes this file.
     */
    void Close() {
        if (hdfs_file_ && hdfs_fs_) hdfsCloseFile(hdfs_fs_, hdfs_file_);
#if CREATE_HDFS_INSTANCE
        if (hdfs_fs_) hdfsDisconnect(hdfs_fs_);
#endif
        hdfs_fs_ = nullptr;
        hdfs_file_ = nullptr;
    }

    /*!
     * \fn  bool HdfsFile::Good() const
     *
     * \brief   Whether the file is opened properly.
     *
     * \return  True if file is opened correctly.
     */
    bool Good() const { return hdfs_fs_ != nullptr && hdfs_file_ != nullptr; }

    /*!
     * \fn  size_t HdfsFile::Size() const
     *
     * \brief   Gets the size of the file.
     *
     * \return  Size of the file.
     */
    size_t Size() const {
        CheckOpen();
        hdfsFileInfo* info = hdfsGetPathInfo(hdfs_fs_, path_.Path().c_str());
        size_t size = (size_t)info->mSize;
        hdfsFreeFileInfo(info, 1);
        return size;
    }

    /*!
     * \fn  const std::string& HdfsFile::Path() const
     *
     * \brief   Gets the path of the file.
     *
     * \return  A reference to the path string.
     */
    const std::string& Path() const { return path_.Path(); }

    /*!
     * \fn  bool HdfsFile::Seek(size_t pos)
     *
     * \brief   Seek to a position, only applicable for read.
     *          Hdfs files are append only, so you cannot seek to a position
     *          and write to it.
     *
     * \param   pos The position.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Seek(size_t pos) {
        CheckOpen();
        FMA_ASSERT((uint64_t)pos < (uint64_t)std::numeric_limits<tOffset>::max())
            << "offset too large, offset=" << pos;
        return hdfsSeek(hdfs_fs_, hdfs_file_, pos) == 0;
    }

    /*!
     * \fn  virtual size_t HdfsFile::Read(void* buf, size_t size) = 0;
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buffer, size_t size) {
        CheckOpen();
        if (size == 0) return 0;
        size_t bytes_read = 0;
        while (bytes_read < size) {
            size_t bytes_remaining = size - bytes_read;
            tSize bytes_to_read = (tSize)std::min<size_t>(bytes_remaining, block_size_);
            tSize r = hdfsRead(hdfs_fs_, hdfs_file_, (char*)buffer + bytes_read, bytes_to_read);
            if (r == 0) {
                // WARN() << "Read stopped with 0 bytes read: file=" << path_.Path()
                //    << ", bytes_read=" << bytes_read << ", size=" << size;
                break;
            } else if (r == -1) {
                FMA_ERR() << "Read stopped with -1: file=" << path_.Path()
                          << ", bytes_read=" << bytes_read;
                break;
            }
            bytes_read += r;
        }
        return bytes_read;
    }
#if 0
    /*!
     * \fn  size_t HdfsFile::Read(void* buffer, size_t size, size_t offset)
     *
     * \brief   Read a block of data, starting from offset, into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     * \param   offset  Offset in the file
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buffer, size_t size, size_t offset) {
        CheckOpen();
        size_t bytes_read = 0;
        while (bytes_read < size) {
            size_t bytes_remaining = size - bytes_read;
            tSize bytes_to_read = (tSize)std::min<size_t>(bytes_remaining, block_size_);
            tSize r = hdfsPread(hdfs_fs_, hdfs_file_, offset + bytes_read,
                                (char*)buffer + bytes_read, bytes_to_read);
            if (r == 0) {
                // WARN() << "Read stopped with 0 bytes read: file=" << path_.Path()
                //    << ", bytes_read=" << bytes_read << ", size=" << size;
                break;
            } else if (r == -1) {
                FMA_ERR() << "Read stopped with -1: file=" << path_.Path()
                          << ", bytes_read=" << bytes_read;
                break;
            }
            bytes_read += r;
        }
        return bytes_read;
    }
#endif
    /*!
     * \fn  void HdfsFile::Append(const void* buffer, size_t size)
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Append(const void* buffer, size_t size, size_t buffer_size = 64 << 20) {
        CheckOpen();
        size_t bytes_per_write = std::min<size_t>(buffer_size, block_size_);
        size_t bytes_written = 0;
        while (bytes_written < size) {
            size_t bytes_remaining = size - bytes_written;
            tSize bytes_to_write = (tSize)std::min<size_t>(bytes_remaining, bytes_per_write);
            tSize r =
                hdfsWrite(hdfs_fs_, hdfs_file_, (char*)buffer + bytes_written, bytes_to_write);
            if (r == 0) {
                FMA_WARN() << "Write stopped with 0 bytes read: file=" << path_.Path()
                           << ", bytes_written=" << bytes_written << ", size=" << size;
                break;
            } else if (r == -1) {
                FMA_ERR() << "Read stopped with -1: file=" << path_.Path()
                          << ", bytes_written=" << bytes_written;
                break;
            }
            bytes_written += r;
        }

        FMA_CHECK_EQ(bytes_written, size) << "error appending to file: file=" << path_.Path()
                                          << ", size=" << size << ", written=" << bytes_written;
    }

    /*!
     * \fn  static bool HdfsFile::Exists(const std::string& path)
     *
     * \brief   Determine if the file exists.
     *
     * \param   path    URI of the file
     *
     * \return  True if the file exists, otherwise false.
     */
    static bool Exists(const std::string& path) {
        FilePath fp;
        if (!fp.FromString(path)) return false;
#if CREATE_HDFS_INSTANCE
        hdfsFS fs = hdfsConnectNewInstance(fp.Host().c_str(), fp.Port());
#else
        hdfsFS fs = hdfsConnect(fp.Host().c_str(), fp.Port());
#endif
        if (!fs) {
            FMA_WARN() << "failed to connect to name node: " << fp.Host() << ":" << fp.Port();
            return false;
        }
        bool ret = (hdfsExists(fs, fp.Path().c_str()) == 0);
#if CREATE_HDFS_INSTANCE
        hdfsDisconnect(fs);
#endif
        return ret;
    }

    /**
     * Gets the offset
     *
     * \return  A size_t.
     */
    size_t Offset() const {
        CheckOpen();
        size_t off = (size_t)hdfsTell(hdfs_fs_, hdfs_file_);
        return off;
    }

 private:
    void CheckOpen() const {
        if (hdfs_fs_ == nullptr || hdfs_file_ == nullptr) {
            throw std::runtime_error("Failed to open file " + file_.Path() + " for write.");
        }
    }

    FilePath path_;
    hdfsFS hdfs_fs_ = nullptr;
    hdfsFile hdfs_file_ = nullptr;
    size_t block_size_ = 64 << 20;
};

/*!
 * \class   UnbufferedInputHdfsStream
 *
 * \brief   An input stream reading from HDFS file
 */
class UnbufferedInputHdfsStream : public InputFileStream {
    HdfsFile file_;

 public:
    DISABLE_COPY(UnbufferedInputHdfsStream);
    UnbufferedInputHdfsStream() {}

    /*!
     * \fn  UnbufferedInputHdfsStream::UnbufferedInputHdfsStream(const std::string& path, size_t
     * buf_size = 0)
     *
     * \brief   Constructor.
     *
     * \param   path        URI of the HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes. This is ignored.
     */
    explicit UnbufferedInputHdfsStream(const std::string& path, size_t buf_size = 0) {
        Open(path, buf_size);
    }

    virtual ~UnbufferedInputHdfsStream() {}

    /*!
     * \fn  virtual void UnbufferedInputHdfsStream::Open(const std::string& path, size_t buf_size =
     * 0) override
     *
     * \brief   Opens the given file.
     *
     * \param   path        URI of the HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes. This is ignored.
     */
    void Open(const std::string& path, size_t buf_size = 0) override {
        if (!file_.Open(path, O_RDONLY)) {
            FMA_ERR() << "Failed to open file " << path << " for read";
        }
    }

    /*!
     * \fn  size_t UnbufferedInputHdfsStream::Read(void* buf, size_t size) override
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buf, size_t size) override { return file_.Read(buf, size); }

    /*!
     * \fn  bool UnbufferedInputHdfsStream::Good() const override
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Good() const override { return file_.Good(); }

    /*!
     * \fn  void UnbufferedInputHdfsStream::Close() override
     *
     * \brief   Closes this file.
     */
    void Close() override { file_.Close(); }

    /*!
     * \fn  size_t UnbufferedInputHdfsStream::Size() const override
     *
     * \brief   Gets the size of the file
     *
     * \return  Size of the file
     */
    size_t Size() const override { return file_.Size(); }

    /*!
     * \fn  bool UnbufferedInputHdfsStream::Seek(size_t offset)
     *
     * \brief   Seeks to a position in the file
     *
     * \param   offset  Offset from the beginning of the file
     *
     * \return  true if operation is successful, otherwise false
     *          Some streams like snappy streams cannot read from arbitrary offsets,
     *          and they will always return false.
     */
    bool Seek(size_t offset) { return file_.Seek(offset); }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return file_.Path(); }

    /**
     * Gets the offset
     *
     * \return  A size_t.
     */
    size_t Offset() const override { return file_.Offset(); }
};

/*!
 * \class   UnbufferedOutputHdfsStream
 *
 * \brief   Output stream for HDFS file
 */
class UnbufferedOutputHdfsStream : public OutputFileStream {
    HdfsFile file_;
    size_t size_;

 public:
    DISABLE_COPY(UnbufferedOutputHdfsStream);
    UnbufferedOutputHdfsStream() {}

    /*!
     * \fn  UnbufferedOutputHdfsStream::UnbufferedOutputHdfsStream(const std::string& path, size_t
     * buf_size = 0, std::ofstream::openmode mode = std::ofstream::trunc)
     *
     * \brief   Construct an output stream from a hdfs file
     *
     * \param   path        URI of the HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes. This is ignored.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    UnbufferedOutputHdfsStream(const std::string& path, size_t buf_size = 0,
                               std::ofstream::openmode mode = std::ofstream::trunc) {
        Open(path, buf_size, mode);
    }

    ~UnbufferedOutputHdfsStream() { Close(); }

    /*!
     * \fn  void UnbufferedOutputHdfsStream::Open(const std::string& path, size_t buf_size = 0,
     * std::ofstream::openmode mode = std::ofstream::trunc) override
     *
     * \brief   Opens the given file.
     *
     * \param   path        URI of the HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes. This is ignored.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    void Open(const std::string& path, size_t buf_size = 0,
              std::ofstream::openmode mode = std::ofstream::trunc) override {
        int flags = O_WRONLY;
        if (mode & std::ofstream::app) flags |= O_APPEND;
        if (!file_.Open(path, flags)) {
            FMA_ERR() << "Failed to open file " << path << " for write";
        }
        size_ = file_.Size();
    }

    /*!
     * \fn  void UnbufferedOutputHdfsStream::Write(const void* p, size_t size)
     *
     * \brief   Writes.
     *
     * \param   buf     Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Write(const void* buf, size_t size) {
        file_.Append(buf, size);
        size_ += size;
    }

    /*!
     * \fn  bool UnbufferedOutputHdfsStream::Good() const
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file opened correctly, otherwise false
     */
    bool Good() const { return file_.Good(); }

    /*!
     * \fn  void UnbufferedOutputHdfsStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() {
        file_.Close();
        size_ = 0;
    }

    virtual size_t Size() const { return size_; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return file_.Path(); }
};

typedef InputBufferedFileStream<UnbufferedInputHdfsStream, PrefetchingtStreamBuffer>
    InputHdfsStream;
typedef OutputBufferedFileStream<UnbufferedOutputHdfsStream, ThreadedOutputStreamBuffer>
    OutputHdfsStream;
}  // namespace fma_common
#endif
