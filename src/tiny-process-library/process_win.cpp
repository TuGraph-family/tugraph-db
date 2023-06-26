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

/**
 * Modified by chuntao: Add GetLastErrorAsString
 * */
#include "tiny-process-library/process.hpp"
#include <windows.h>
#include <cstring>
#include <TlHelp32.h>
#include <stdexcept>
#include <iostream>

namespace TinyProcessLib {

Process::Data::Data() noexcept : id(0), handle(NULL) {}

// Simple HANDLE wrapper to close it automatically from the destructor.
class Handle {
 public:
    Handle() noexcept : handle(INVALID_HANDLE_VALUE) {}
    ~Handle() noexcept { close(); }
    void close() noexcept {
        if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
    }
    HANDLE detach() noexcept {
        HANDLE old_handle = handle;
        handle = INVALID_HANDLE_VALUE;
        return old_handle;
    }
    operator HANDLE() const noexcept { return handle; }
    HANDLE *operator&() noexcept { return &handle; }  // NOLINT

 private:
    HANDLE handle;
};

static std::string GetLastErrorAsString() {
    // Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) return std::string();  // No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0,
        NULL);

    std::string message(messageBuffer, size);

    // Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

// Based on the discussion thread:
// https://www.reddit.com/r/cpp/comments/3vpjqg/a_new_platform_independent_process_library_for_c11/cxq1wsj
std::mutex create_process_mutex;

// Based on the example at
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx.
Process::id_type Process::open(const string_type &command, const string_type &path) noexcept {
    if (open_stdin) stdin_fd = std::unique_ptr<fd_type>(new fd_type(NULL));
    if (read_stdout) stdout_fd = std::unique_ptr<fd_type>(new fd_type(NULL));
    if (read_stderr) stderr_fd = std::unique_ptr<fd_type>(new fd_type(NULL));

    Handle stdin_rd_p;
    Handle stdin_wr_p;
    Handle stdout_rd_p;
    Handle stdout_wr_p;
    Handle stderr_rd_p;
    Handle stderr_wr_p;

    SECURITY_ATTRIBUTES security_attributes;

    security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.bInheritHandle = TRUE;
    security_attributes.lpSecurityDescriptor = nullptr;

    std::lock_guard<std::mutex> lock(create_process_mutex);
    if (stdin_fd) {
        if (!CreatePipe(&stdin_rd_p, &stdin_wr_p, &security_attributes, 0) ||
            !SetHandleInformation(stdin_wr_p, HANDLE_FLAG_INHERIT, 0))
            return 0;
    }
    if (stdout_fd) {
        if (!CreatePipe(&stdout_rd_p, &stdout_wr_p, &security_attributes, 0) ||
            !SetHandleInformation(stdout_rd_p, HANDLE_FLAG_INHERIT, 0)) {
            return 0;
        }
    }
    if (stderr_fd) {
        if (!CreatePipe(&stderr_rd_p, &stderr_wr_p, &security_attributes, 0) ||
            !SetHandleInformation(stderr_rd_p, HANDLE_FLAG_INHERIT, 0)) {
            return 0;
        }
    }

    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;

    ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&startup_info, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.hStdInput = stdin_rd_p;
    startup_info.hStdOutput = stdout_wr_p;
    startup_info.hStdError = stderr_wr_p;
    if (stdin_fd || stdout_fd || stderr_fd) startup_info.dwFlags |= STARTF_USESTDHANDLES;

    string_type process_command = command;
#ifdef MSYS_PROCESS_USE_SH
    size_t pos = 0;
    while ((pos = process_command.find('\\', pos)) != string_type::npos) {
        process_command.replace(pos, 1, "\\\\\\\\");
        pos += 4;
    }
    pos = 0;
    while ((pos = process_command.find('\"', pos)) != string_type::npos) {
        process_command.replace(pos, 1, "\\\"");
        pos += 2;
    }
    process_command.insert(0, "sh -c \"");
    process_command += "\"";
#endif

    BOOL bSuccess = CreateProcess(
        nullptr, process_command.empty() ? nullptr : &process_command[0], nullptr, nullptr, TRUE, 0,
        nullptr, path.empty() ? nullptr : path.c_str(), &startup_info, &process_info);

    if (!bSuccess) {
        std::cerr << "Error creating process: " << GetLastErrorAsString() << std::endl;
        return 0;
    } else {
        CloseHandle(process_info.hThread);
    }

    if (stdin_fd) *stdin_fd = stdin_wr_p.detach();
    if (stdout_fd) *stdout_fd = stdout_rd_p.detach();
    if (stderr_fd) *stderr_fd = stderr_rd_p.detach();

    closed = false;
    data.id = process_info.dwProcessId;
    data.handle = process_info.hProcess;
    return process_info.dwProcessId;
}

void Process::async_read() noexcept {
    if (data.id == 0) return;

    if (stdout_fd) {
        stdout_thread = std::thread([this]() {
            DWORD n;
            std::unique_ptr<char[]> buffer(new char[buffer_size]);
            for (;;) {
                BOOL bSuccess = ReadFile(*stdout_fd, static_cast<CHAR *>(buffer.get()),
                                         static_cast<DWORD>(buffer_size), &n, nullptr);
                if (!bSuccess || n == 0) break;
                read_stdout(buffer.get(), static_cast<size_t>(n));
            }
        });
    }
    if (stderr_fd) {
        stderr_thread = std::thread([this]() {
            DWORD n;
            std::unique_ptr<char[]> buffer(new char[buffer_size]);
            for (;;) {
                BOOL bSuccess = ReadFile(*stderr_fd, static_cast<CHAR *>(buffer.get()),
                                         static_cast<DWORD>(buffer_size), &n, nullptr);
                if (!bSuccess || n == 0) break;
                read_stderr(buffer.get(), static_cast<size_t>(n));
            }
        });
    }
}

int Process::get_exit_status() noexcept {
    if (data.id == 0) return -1;

    DWORD exit_status;
    WaitForSingleObject(data.handle, INFINITE);
    if (!GetExitCodeProcess(data.handle, &exit_status)) exit_status = -1;
    {
        std::lock_guard<std::mutex> lock(close_mutex);
        CloseHandle(data.handle);
        closed = true;
    }
    close_fds();

    return static_cast<int>(exit_status);
}

bool Process::try_get_exit_status(int &exit_status) noexcept {
    if (data.id == 0) return false;

    DWORD wait_status = WaitForSingleObject(data.handle, 0);

    if (wait_status == WAIT_TIMEOUT) return false;

    DWORD exit_status_win;
    if (!GetExitCodeProcess(data.handle, &exit_status_win)) exit_status_win = -1;
    {
        std::lock_guard<std::mutex> lock(close_mutex);
        CloseHandle(data.handle);
        closed = true;
    }
    close_fds();

    exit_status = static_cast<int>(exit_status_win);
    return true;
}

void Process::close_fds() noexcept {
    if (stdout_thread.joinable()) stdout_thread.join();
    if (stderr_thread.joinable()) stderr_thread.join();

    if (stdin_fd) close_stdin();
    if (stdout_fd) {
        if (*stdout_fd != NULL) CloseHandle(*stdout_fd);
        stdout_fd.reset();
    }
    if (stderr_fd) {
        if (*stderr_fd != NULL) CloseHandle(*stderr_fd);
        stderr_fd.reset();
    }
}

bool Process::write(const char *bytes, size_t n) {
    if (!open_stdin)
        throw std::invalid_argument(
            "Can't write to an unopened stdin pipe. Please set open_stdin=true when constructing "
            "the process.");

    std::lock_guard<std::mutex> lock(stdin_mutex);
    if (stdin_fd) {
        DWORD written;
        BOOL bSuccess = WriteFile(*stdin_fd, bytes, static_cast<DWORD>(n), &written, nullptr);
        if (!bSuccess || written == 0) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}

void Process::close_stdin() noexcept {
    std::lock_guard<std::mutex> lock(stdin_mutex);
    if (stdin_fd) {
        if (*stdin_fd != NULL) CloseHandle(*stdin_fd);
        stdin_fd.reset();
    }
}

// Based on http://stackoverflow.com/a/1173396
void Process::kill(bool /*force*/) noexcept {
    std::lock_guard<std::mutex> lock(close_mutex);
    if (data.id > 0 && !closed) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot) {
            PROCESSENTRY32 process;
            ZeroMemory(&process, sizeof(process));
            process.dwSize = sizeof(process);
            if (Process32First(snapshot, &process)) {
                do {
                    if (process.th32ParentProcessID == data.id) {
                        HANDLE process_handle =
                            OpenProcess(PROCESS_TERMINATE, FALSE, process.th32ProcessID);
                        if (process_handle) {
                            TerminateProcess(process_handle, 2);
                            CloseHandle(process_handle);
                        }
                    }
                } while (Process32Next(snapshot, &process));
            }
            CloseHandle(snapshot);
        }
        TerminateProcess(data.handle, 2);
    }
}

// Based on http://stackoverflow.com/a/1173396
void Process::kill(id_type id, bool /*force*/) noexcept {
    if (id == 0) return;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot) {
        PROCESSENTRY32 process;
        ZeroMemory(&process, sizeof(process));
        process.dwSize = sizeof(process);
        if (Process32First(snapshot, &process)) {
            do {
                if (process.th32ParentProcessID == id) {
                    HANDLE process_handle =
                        OpenProcess(PROCESS_TERMINATE, FALSE, process.th32ProcessID);
                    if (process_handle) {
                        TerminateProcess(process_handle, 2);
                        CloseHandle(process_handle);
                    }
                }
            } while (Process32Next(snapshot, &process));
        }
        CloseHandle(snapshot);
    }
    HANDLE process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, id);
    if (process_handle) TerminateProcess(process_handle, 2);
}

}  // namespace TinyProcessLib
