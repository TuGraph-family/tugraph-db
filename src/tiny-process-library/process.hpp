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


#ifndef TINY_PROCESS_LIBRARY_HPP_
#define TINY_PROCESS_LIBRARY_HPP_

#include <string>
#include <functional>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace TinyProcessLib {

/// Platform independent class for creating processes
class Process {
 public:
#ifdef _WIN32
    typedef unsigned long id_type;  // Process id type
    typedef void *fd_type;          // File descriptor type
#ifdef UNICODE
    typedef std::wstring string_type;
#else
    typedef std::string string_type;
#endif
#else
    typedef pid_t id_type;
    typedef int fd_type;
    typedef std::string string_type;
#endif
 private:
    class Data {
     public:
        Data() noexcept;
        id_type id;
#ifdef _WIN32
        void *handle;
#endif
    };

 public:
    /// Note on Windows: it seems not possible to specify which pipes to redirect.
    /// Thus, at the moment, if read_stdout==nullptr, read_stderr==nullptr and open_stdin==false,
    /// the stdout, stderr and stdin are sent to the parent process instead.
    Process(const string_type &command, const string_type &path = string_type(),
            std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
            std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
            bool open_stdin = false, size_t buffer_size = 131072) noexcept;
#ifndef _WIN32
    /// Supported on Unix-like systems only.
    Process(std::function<void()> function,
            std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
            std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
            bool open_stdin = false, size_t buffer_size = 131072) noexcept;
#endif
    ~Process() noexcept;

    /// Get the process id of the started process.
    id_type get_id() const noexcept;
    /// Wait until process is finished, and return exit status.
    int get_exit_status() noexcept;
    /// If process is finished, returns true and sets the exit status. Returns false otherwise.
    bool try_get_exit_status(int &exit_status) noexcept;
    /// Write to stdin.
    bool write(const char *bytes, size_t n);
    /// Write to stdin. Convenience function using write(const char *, size_t).
    bool write(const std::string &data);
    /// Close stdin. If the process takes parameters from stdin, use this to notify that all
    /// parameters have been sent.
    void close_stdin() noexcept;

    /// Kill the process. force=true is only supported on Unix-like systems.
    void kill(bool force = false) noexcept;
    /// Kill a given process id. Use kill(bool force) instead if possible. force=true is only
    /// supported on Unix-like systems.
    static void kill(id_type id, bool force = false) noexcept;

 private:
    Data data;
    bool closed;
    std::mutex close_mutex;
    std::function<void(const char *bytes, size_t n)> read_stdout;
    std::function<void(const char *bytes, size_t n)> read_stderr;
    std::thread stdout_thread, stderr_thread;
    bool open_stdin;
    std::mutex stdin_mutex;
    size_t buffer_size;

    std::unique_ptr<fd_type> stdout_fd, stderr_fd, stdin_fd;

    id_type open(const string_type &command, const string_type &path) noexcept;
#ifndef _WIN32
    id_type open(std::function<void()> function) noexcept;
#endif
    void async_read() noexcept;
    void close_fds() noexcept;
};

}  // namespace TinyProcessLib

#endif  // TINY_PROCESS_LIBRARY_HPP_
