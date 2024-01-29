//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\text_writer.h.
 *
 * \brief   Declares the text writer class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "fma-common/file_stream.h"
#include "fma-common/pipeline.h"
#include "fma-common/string_formatter.h"
#include "fma-common/string_util.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

namespace fma_common {
namespace _CsvWriterDetail {
template <size_t N, typename... T>
struct WriteTuple {
    template <typename FUNC, typename... A>
    static inline void Write(FUNC&& f, std::string& str, const std::tuple<const T*...>& ptrs,
                             size_t off, A&&... a) {
        WriteTuple<N - 1, T...>::Write(std::forward<FUNC>(f), str, ptrs, off,
                                       std::get<N - 1>(ptrs)[off], std::forward<A>(a)...);
    }
};

template <typename... T>
struct WriteTuple<0, T...> {
    template <typename FUNC, typename... A>
    static inline void Write(FUNC&& f, std::string& str, const std::tuple<const T*...>& ptrs,
                             size_t off, A&&... a) {
        f(str, std::forward<A>(a)...);
    }
};

}  // namespace _CsvWriterDetail

/*!
 * \class   TextFileFormatter
 *
 * \brief   A text formatter which can be used like Python formatters.
 *          Example:
 *              std::string buf;
 *              buf.reserve(1024);
 *              TextFileFormatter.format(buf, "{} {} {}", a, b, c);
 */
class TextFileFormatter {
    std::string buf_;

 public:
    template <typename... Ts>
    size_t Write(OutputFileStream& out, const char* format, const Ts&... data) {
        buf_.clear();
        StringFormatter::Format(buf_, format, data...);
        out.Write(&buf_[0], buf_.size());
        return buf_.size();
    }
};

template <char DELIM, typename... Ts>
class DeliminatedFormatter {
    std::string format_;

 public:
    DeliminatedFormatter() {
        int n_fields = sizeof...(Ts);
        for (int i = 0; i < n_fields; i++) {
            if (i != 0) format_ += DELIM;
            format_ += "{}";
        }
        format_.push_back('\n');
        format_.push_back('\0');
    }

    void operator()(std::string& buf, const Ts&... data) const {
        StringFormatter::Append(buf, &format_[0], data...);
    }
};

class TextWriterBase {
 public:
    /*! \brief   When using multiple threads, the data will be divided into
     *           multiple chunks of this size and processed in parallel
     */
    static const size_t DEFAULT_CHUNK_SIZE = 1 << 16;
};

/*!
 * \class   TextWriter
 *
 * \brief   A text writer that can write multiple vectors into a text file.
 *
 * \tparam  F   Function to append a line to text file
 * \tparam  T   List of types of the fields
 *
 * \note    F must be of type void F(std::string& buf, const T&... ts). It
 *          must APPEND to the string buf. And it must append proper line
 *          breaks ('\n' or '\r\n') at the end of each line.
 */
template <typename F, typename... T>
class TextWriter : public TextWriterBase {
    /*! \brief   Number of fields */
    static const int n_fields_ = sizeof...(T);

    OutputFileStream& file_;
    F write_line_;
    int n_writers_ = 0;
    size_t chunk_size_ = DEFAULT_CHUNK_SIZE;

    /*! \brief   Pointer to the arrays currently being written */
    std::tuple<const T*...> curr_ptrs_;
    struct DataPart {
        size_t off;   // offset from begining of the array
        size_t size;  // number of elements to write
        std::string buf;
    };
    std::vector<DataPart> workspace_;
    PipelineStage<int, int> textify_stage_;
    PipelineStage<int, int> file_write_stage_;
    BoundedQueue<int> free_spaces_;

 public:
    DISABLE_COPY(TextWriter);
    DISABLE_MOVE(TextWriter);

    /*!
     * \fn  TextWriter::TextWriter(OutputFileStream& out, const F& write_line = F(), int n_writers =
     * 1)
     *
     * \brief   Constructor.
     *
     * \param [in,out]  out         The file to write to.
     * \param           write_line  (Optional) The write line function.
     * \param           n_writers   (Optional) The number of worker threads used
     *                              to convert data into text.
     *
     * \note    write_line must be of type void F(std::string& buf, const T&... ts).
     *          It must APPEND to the string buf. And it must append proper line
     *          breaks ('\n' or '\r\n') at the end of each line.
     */
    TextWriter(OutputFileStream& out, const F& write_line = F(), int n_writers = 1,
               size_t chunk_size = DEFAULT_CHUNK_SIZE)
        : file_(out),
          write_line_(write_line),
          n_writers_(n_writers),
          chunk_size_(chunk_size),
          textify_stage_([this](int i) -> int { return PartToText(i); }, nullptr, 0, n_writers,
                         n_writers),
          file_write_stage_([this](int i) -> int { return WriteToFile(i); }, nullptr, 0, 1) {
        workspace_.resize(n_writers_);
        textify_stage_.SetNextStage(&file_write_stage_);
        file_write_stage_.SetNextStage(&free_spaces_);
        for (int i = 0; i < n_writers_; i++) free_spaces_.Push(std::move(i));
    }

    ~TextWriter() {
        textify_stage_.Stop();
        file_write_stage_.Stop();
    }

    /*!
     * \fn  void TextWriter::Write(size_t n, const T*... data)
     *
     * \brief   Writes arrays to type T... and size n into the file.
     *
     * \param   n       Number of elements in each data array
     * \param   data    Pointer to each array
     */
    void Write(size_t n, const T*... data) {
        curr_ptrs_ = std::make_tuple(data...);
        for (size_t i = 0; i < n; i += chunk_size_) {
            int b;
            if (!free_spaces_.Pop(b)) break;
            DataPart& p = workspace_[b];
            p.off = i;
            p.size = std::min(chunk_size_, n - i);
            textify_stage_.Push(std::move(b));
        }
        textify_stage_.WaitTillClear();
        file_write_stage_.WaitTillClear();
    }

    /*!
     * \fn  void TextWriter::Write(const std::vector<T>&... data)
     *
     * \brief   Writes the vectors into CSV file
     *
     * \param   data    Vectors of type T... to be written
     */
    void Write(const std::vector<T>&... data) {
        curr_ptrs_ = std::make_tuple(&data[0]...);
        auto t = std::forward_as_tuple(data...);
        size_t n = std::get<0>(t).size();
        Write(n, &data[0]...);
    }

 private:
    int WriteToFile(int b) {
        DataPart& p = workspace_[b];
        file_.Write(p.buf.data(), p.buf.size());
        p.buf.clear();
        return b;
    }

    int PartToText(int b) {
        DataPart& p = workspace_[b];
        for (size_t i = 0; i < p.size; i++) {
            _CsvWriterDetail::WriteTuple<n_fields_, T...>::Write(write_line_, p.buf, curr_ptrs_,
                                                                 i + p.off);
        }
        return b;
    }
};

namespace _detail {
template <char DELIM, typename X_, typename... A_>
struct LineWriter {
    static inline void Write(std::string& buf, const X_& x, const A_&... a) {
        buf += ToString(x);
        buf += DELIM;
        LineWriter<DELIM, A_...>::Write(buf, a...);
    }
};

template <char DELIM, typename X_>
struct LineWriter<DELIM, X_> {
    static inline void Write(std::string& buf, const X_& x) {
        buf += ToString(x);
        buf += '\n';
    }
};

template <char DELIM, typename... T>
struct DefaultFormatter {
    void operator()(std::string& buf, const T&... d) { LineWriter<DELIM, T...>::Write(buf, d...); }
};
};  // namespace _detail

/*!
 * \fn  template<typename F, typename... T> inline void WriteText(OutputFileStream& file, F&&
 * write_line_func, int n_threads, const std::vector<T>&... vs)
 *
 * \brief   Write a list of vectors out as text using the function write_line_func
 *          to write each line.
 *
 * \tparam  F   Type of the function to write a text line.
 * \tparam  T   Types of the vector elements
 *
 * \param [in,out]  file            An OutputFileStream to write to
 * \param [in,out]  write_line_func function used to write a line, in the form of
 *                                  void F(std::string& buf, const T&...)
 *                                  buf MUST be properly ended with line break
 * \param           n_threads       Number of parallel threads to use for writing
 * \param               chunk_size           (Optional) Number of lines to process as a block.
 * \param           vs              vectors of data
 *
 * \note    F must be of type void F(std::string& buf, const T&... ts). It
 *          must APPEND to the string buf. And it must append proper line
 *          breaks ('\n' or '\r\n') at the end of each line.
 */
template <typename F, typename... T>
inline void WriteText(OutputFileStream& file, F&& write_line_func, int n_threads, size_t chunk_size,
                      const std::vector<T>&... vs) {
    TextWriter<typename std::decay<F>::type, T...> writer(file, write_line_func, n_threads);
    writer.Write(vs...);
}

/*!
 * \fn  template<typename... T> inline void WriteDelimitedText(OutputFileStream& file, char delim,
 * int n_threads, const std::vector<T>&... vs)
 *
 * \brief   Write a list of vectors out as text using default formatter.
 *          That is, each element is converted into string using std::to_string()
 *
 * \tparam  T   Generic type parameter.
 *
 * \param [in,out]  file        An OutputFileStream to write to.
 * \param           delim       The delimiter to separate fields.
 * \param           n_threads   Number of parallel threads to use for writing.
 * \param               chunk_size           (Optional) Number of lines to process as a block.
 * \param           vs          vectors of data.
 */
template <char DELIM, typename... T>
inline void WriteDelimitedText(OutputFileStream& file, int n_threads, size_t chunk_size,
                               const std::vector<T>&... vs) {
    typedef _detail::DefaultFormatter<DELIM, T...> Formatter;
    Formatter formatter;
    TextWriter<Formatter, T...> writer(file, formatter, n_threads);
    writer.Write(vs...);
}

/*!
 * \fn  template<typename... T> inline void WriteCsv(OutputFileStream& file, int n_threads, const
 * std::vector<T>&... vs)
 *
 * \brief   Write a list of vectors out as CSV file (fields separated with
 *          comma).
 *
 * \tparam  T   Generic type parameter.
 *
 * \param [in,out]  file        An OutputFileStream to write to.
 * \param           delim       The delimiter to separate fields.
 * \param           n_threads   Number of parallel threads to use for writing.
 * \param               chunk_size           (Optional) Number of lines to process as a block.
 * \param           vs          vectors of data.
 */
template <typename... T>
inline void WriteCsv(OutputFileStream& file, int n_threads, size_t chunk_size,
                     const std::vector<T>&... vs) {
    WriteDelimitedText<',', T...>(file, n_threads, chunk_size, vs...);
}

/*!
 * \fn  template<typename... T> inline void WriteTsv(OutputFileStream& file, int n_threads, const
 * std::vector<T>&... vs)
 *
 * \brief   Write a list of vectors out as TSV file (fields separated with
 *          tab).
 *
 * \tparam  T   Generic type parameter.
 * \param [in,out]  file        The file.
 * \param           n_threads   The threads.
 * \param               chunk_size           (Optional) Number of lines to process as a block.
 * \param           vs          Variable arguments providing the vs.
 */
template <typename... T>
inline void WriteTsv(OutputFileStream& file, int n_threads, size_t chunk_size,
                     const std::vector<T>&... vs) {
    WriteDelimitedText<'\t', T...>(file, n_threads, chunk_size, vs...);
}

/*!
 * \fn    template<typename... T> inline TextWriter<std::function<void(std::string&, const T&...)>,
 * T...> MakeTextWriter(OutputFileStream& file, const std::function<void(std::string&, const
 * T&...)>& write_line_func, int n_threads = 1, size_t chunk_size =
 * TextWriterBase::DEFAULT_CHUNK_SIZE)
 *
 * \brief    Get a TextWriter object You can later call writer.Write(v1, v2,...) to write contents.
 *
 * \tparam    T    Generic type parameter.
 *
 * \param [in,out]    file               An OutputFileStream to write to.
 * \param [in,out]    write_line_func    function used to write a line, in the form of void
 *                                          F(std::string&amp; buf, const T&amp;...)
 *                                          buf MUST be properly ended with line break.
 * \param             n_threads          (Optional) Number of parallel threads to use for writing.
 * \param             chunk_size         (Optional) Number of lines to process as a block.
 *
 * \return    A TextWriter&lt;std::function&lt;void(std::string&amp;,const T&amp;...)&gt;,T...&gt;
 *
 * \note    write_line_func must be of type void F(std::string&amp; buf, const T&amp;...
 *             ts). It must APPEND to the string buf. And it must append proper line breaks ('\n' or
 *             '\r\n') at the end of each line.
 */
template <typename... T>
inline std::shared_ptr<TextWriter<std::function<void(std::string&, const T&...)>, T...>>
MakeTextWriter(OutputFileStream& file,
               const std::function<void(std::string&, const T&...)>& write_line_func,
               int n_threads = 1, size_t chunk_size = TextWriterBase::DEFAULT_CHUNK_SIZE) {
    return std::make_shared<TextWriter<std::function<void(std::string&, const T&...)>, T...>>(
        file, write_line_func, n_threads, chunk_size);
}
}  // namespace fma_common
