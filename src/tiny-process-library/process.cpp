/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2018 Ole Christian Eidheim
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "tiny-process-library/process.hpp"

namespace TinyProcessLib {

Process::Process(const string_type &command, const string_type &path,
                 std::function<void(const char *bytes, size_t n)> read_stdout,
                 std::function<void(const char *bytes, size_t n)> read_stderr, bool open_stdin,
                 size_t buffer_size) noexcept
    : closed(true),
      read_stdout(std::move(read_stdout)),
      read_stderr(std::move(read_stderr)),
      open_stdin(open_stdin),
      buffer_size(buffer_size) {
    open(command, path);
    async_read();
}

Process::~Process() noexcept { close_fds(); }

Process::id_type Process::get_id() const noexcept { return data.id; }

bool Process::write(const std::string &data) { return write(data.c_str(), data.size()); }

}  // namespace TinyProcessLib
