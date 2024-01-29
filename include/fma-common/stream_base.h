//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace fma_common {
class InputStreamBase {
 public:
    virtual ~InputStreamBase() {}
    virtual size_t Read(void* buf, size_t size) = 0;
    virtual size_t Offset() const = 0;
    virtual bool Good() const = 0;
};

class OutputStreamBase {
 public:
    virtual ~OutputStreamBase() {}
    virtual void Write(const void* buf, size_t size) = 0;
    virtual bool Good() const = 0;
};

class InputMemoryStream : public InputStreamBase {
    std::string buf_;
    const char* beg_;
    size_t size_;
    size_t offset_;

 public:
    InputMemoryStream(void* p, size_t s) : beg_((const char*)p), size_(s), offset_(0) {}
    template <typename T>
    explicit InputMemoryStream(T&& str)
        : buf_(std::forward<T>(str)), beg_(buf_.data()), size_(buf_.size()), offset_(0) {}

    virtual ~InputMemoryStream() {}
    virtual size_t Read(void* buf, size_t size) {
        if (offset_ >= size_) return 0;
        size_t rs = std::min<size_t>(size_ - offset_, size);
        memcpy(buf, beg_ + offset_, rs);
        offset_ += rs;
        return rs;
    }
    virtual size_t Offset() const { return offset_; }
    virtual bool Good() const { return offset_ < size_; }
};

/*!
 * \class    StreamLineReader
 *
 * \brief    A reader that enables reading lines from a InputStream
 */
class StreamLineReader {
    std::string buf_;
    size_t begin_ = 0;
    size_t end_ = 0;
    size_t capacity_ = 1024;
    InputStreamBase& stream_;

 public:
    /*!
     * \fn    StreamLineReader::StreamLineReader(InputStream& stream, size_t line_size_hint = 1024)
     *
     * \brief    Constructor.
     *
     * \param [in,out]    stream              The input stream.
     * \param               line_size_hint    (Optional) A hint to tell the reader
     *                                     how large the read buffer should be.
     */
    explicit StreamLineReader(InputStreamBase& stream, size_t line_size_hint = 1024)
        : stream_(stream) {
        capacity_ = line_size_hint;
        buf_.resize(capacity_);
    }

    /*!
     * \fn    void StreamLineReader::ReadLine(std::string& line)
     *
     * \brief    Reads a line into the given string.
     *
     * \param [in,out]    line            The location where the line should be stored.
     *                  strip_endings   (Optional) Whether we should remove
     *                                  line endings (\r\n) from the line
     */
    void ReadLine(std::string& line, bool strip_endings = true) {
        line.clear();
        bool line_ends = false;
        while (!line_ends) {
            if (begin_ >= end_) {
                // empty buffer, read from stream
                size_t read = stream_.Read(&buf_[0], capacity_);
                begin_ = 0;
                end_ = read;
                if (read == 0) return;
            }
            size_t end = buf_.find('\n', begin_);
            if (end == buf_.npos || end >= end_) {
                line_ends = false;
                end = end_;
            } else {
                end++;
                line_ends = true;
            }
            line.append(&buf_[begin_], &buf_[end]);
            begin_ = end;
        }
        if (strip_endings) {
            size_t pos = line.find_last_not_of("\r\n");
            if (pos != line.npos) {
                line.resize(pos + 1);
            }
        }
    }

    /*!
     * \fn    std::string StreamLineReader::ReadLine()
     *
     * \brief    Reads a line from the input stream and return it.
     *
     * \param   strip_endings   (Optional) Whether we should remove
     *                          line endings (\r\n) from the line
     *
     * \return    The line.
     */
    std::string ReadLine(bool strip_endings = true) {
        std::string ret;
        ReadLine(ret, strip_endings);
        return ret;
    }

    /*!
     * \fn    std::vector<std::string> StreamLineReader::ReadAllLines()
     *
     * \brief    Reads all lines from the input stream.
     *
     * \param   strip_endings   (Optional) Whether we should remove
     *                          line endings (\r\n) from the line
     *
     * \return    all lines.
     */
    std::vector<std::string> ReadAllLines(bool strip_endings = true) {
        std::vector<std::string> ret;
        while (true) {
            std::string line = ReadLine(strip_endings);
            if (line.empty()) break;
            ret.push_back(std::move(line));
        }
        return ret;
    }
};
}  // namespace fma_common
