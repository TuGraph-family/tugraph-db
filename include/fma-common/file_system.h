//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\file_system.h.
 *
 * \brief    Declares the file system class.
 *
 * \author    Chuntao Hong
 *
 * \last_modified    2017/5/27.
 */
#pragma once

#include <cstdio>
#include <cstring>

#include "tools/lgraph_log.h"
#include "fma-common/pipe_stream.h"
#include "fma-common/string_formatter.h"
#include "fma-common/string_util.h"
#include "fma-common/text_parser.h"

#if FMA_HAS_LIBHDFS
#include "hdfs/hdfs.h"
#endif

#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#include <filesystem>
#define _STD_FS std::filesystem
#define _RMDIR std::filesystem::remove_all
#else  // if __cplusplu >= 201703L
#if (defined(__GNUC__)) && (__GNUC__ < 4 || __GNUC__ == 4 && __GNUC_MINOR__ <= 8)
#define FMA_USE_BOOST_FS 1
#else
#include <experimental/filesystem>
#define _STD_FS std::experimental::filesystem
#ifndef _WIN32
#include <stdio.h>
#include <ftw.h>
static inline bool _RmDir(const std::string& dir, std::error_code& ec);

static inline int _RmFiles_(const char* pathname, const struct stat* sbuf, int type,
                            struct FTW* ftwb) {
    std::error_code ec;
    if (!_STD_FS::remove(pathname, ec)) return -1;
    return 0;
}

static inline bool _RmDir(const std::string& dir, std::error_code& ec) {
    if (nftw(dir.c_str(), _RmFiles_, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) == 0) return true;
    return _STD_FS::remove_all(dir, ec);
}
#define _RMDIR _RmDir
#else
#define _RMDIR _STD_FS::remove_all
#endif  // ifndef _WIN32
#endif
#endif

#if FMA_USE_BOOST_FS
#include <boost/filesystem.hpp>
#define _STD_FS boost::filesystem
#define _RMDIR _STD_FS::remove_all
#define _EC_TYPE_ boost::system::error_code
#else
#define _EC_TYPE_ std::error_code
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <Windows.h>
#include <direct.h>
#include <codecvt>
#include <locale>
#else  // ifdef _WIN32
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#endif  // ifdef _WIN32

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace fma_common {
/*!
 * \class   FilePath
 *
 * \brief   Representation of a file uri, including local file uri and HDFS URI
 */
class FilePath {
 public:
    enum SchemeType { LOCAL = 0, HDFS = 1, MYSQL = 2, SQLSERVER = 3 };
    static constexpr const char* HDFS_PATH_START = "hdfs://";
    static const int HDFS_SCHEME_LEN = 7;
    static constexpr const char* WEBHDFS_PATH_START = "webhdfs://";
    static const int WEBHDFS_SCHEME_LEN = 10;
    static constexpr const char* MYSQL_PATH_START = "mysql://";
    static const int MYSQL_SCHEME_LEN = 8;
    static constexpr const char* SQLSERVER_PATH_START = "sqlserver://";
    static const int SQLSERVER_SCHEME_LEN = 12;

    FilePath() {}

    /*!
     * \fn  FilePath::FilePath(const std::string& uri)
     *
     * \brief   Construct a FilePath from local file uri or HDFS URI.
     *
     * \param   uri    Full pathname of the file, HDFS URI MUST start with hdfs://.
     */
    explicit FilePath(const std::string& uri) { FromString(uri); }

    /*!
     * \fn  bool FilePath::FromString(const std::string& uri)
     *
     * \brief   Read uri from the string
     *
     * \param   uri    string containing a uri, can either be a local uri or a HDFS URI
     *                  An HDFS URI must look like hdfs://host:port/... or webhdfs://host:port/....
     *                  You can leave out host:port, so it looks like hdfs:///... or webhdfs:///...
     *                  in which case we will use the default configuration.
     *
     * \return  True if the uri can be successfully parsed, otherwise false
     */
    bool FromString(const std::string& uri) {
        uri_ = uri;
        if (StartsWith(uri, HDFS_PATH_START, false) || StartsWith(uri, WEBHDFS_PATH_START, false)) {
            scheme_ = SchemeType::HDFS;
        } else if (StartsWith(uri, MYSQL_PATH_START, false)) {
            scheme_ = SchemeType::MYSQL;
        } else if (StartsWith(uri, SQLSERVER_PATH_START, false)) {
            scheme_ = SchemeType::SQLSERVER;
        } else {
            scheme_ = SchemeType::LOCAL;
        }
        if (scheme_ == SchemeType::LOCAL) {
            path_ = uri;
        } else if (scheme_ == SchemeType::HDFS) {
            int scheme_size =
                StartsWith(uri, HDFS_PATH_START, false) ? HDFS_SCHEME_LEN : WEBHDFS_SCHEME_LEN;
            auto p1 = uri.find("/", scheme_size);
            if (p1 == uri.npos) {
                LOG_WARN() << "failed to parse hdfs file uri: " << uri;
                return false;
            } else {
                std::string nn = uri.substr(scheme_size, p1 - scheme_size);
                port_ = 0;
                auto p2 = nn.find(":");
                host_ = nn.substr(0, p2);
                if (host_ == "") host_ = "default";
                if (p2 != nn.npos) {
                    port_ = atoi(nn.substr(p2 + 1).c_str());
                }
            }
            path_ = uri.substr(p1);
        }
        return true;
    }

    /*!
     * \fn    const std::string& FilePath::Uri() const
     *
     * \brief    Gets the URI of the path, e.g. hdfs://host:port/path/to/file
     *
     * \return    A reference to a const std::string.
     */
    const std::string& Uri() const { return uri_; }

    /*!
     * \fn  SchemeType FilePath::Scheme() const
     *
     * \brief   Get the scheme of the uri, either LOCAL or HDFS
     *
     * \return  SchemeType of the uri, either LOCAL or HDFS.
     */
    SchemeType Scheme() const { return scheme_; }

    /*!
     * \fn  const std::string& FilePath::Path() const
     *
     * \brief   Get the uri of the file, excluding the scheme
     *
     * \return  Path string of the file, excluding the scheme
     */
    const std::string& Path() const { return path_; }

    /*!
     * \fn  const std::string& FilePath::Host() const
     *
     * \brief   Get the host of HDFS uri, otherwise empty string
     *
     * \return  Host of the hdfs uri excluding port. Returns empty if
     *          the uri is not a HDFS uri.
     */
    const std::string& Host() const { return host_; }

    /*!
     * \fn  uint16_t FilePath::Port() const
     *
     * \brief   Get the port of HDFS scheme, otherwise 0
     *
     * \note    HDFS uri can also return port=0 if default configuration
     *          is used.
     *
     * \return  Port number if the uri is an HDFS uri. Returns 0 if the
     *          uri is not HDFS, or we are using default HDFS configuration.
     */
    uint16_t Port() const { return port_; }

    std::string Dir() const {
#ifdef _WIN32
        auto pos = uri_.find_last_of("/\\");
#else
        auto pos = uri_.find_last_of("/");
#endif
        if (pos == uri_.npos) {
            return "";
        } else {
            return uri_.substr(0, pos);
        }
    }

    std::string Name() const {
#ifdef _WIN32
        auto pos = uri_.find_last_of("/\\");
#else
        auto pos = uri_.find_last_of("/");
#endif
        if (pos == uri_.npos) {
            return uri_;
        } else {
            return uri_.substr(pos + 1);
        }
    }

    static std::string GetFileNameFromPath(const std::string& path) {
#ifdef _WIN32
        auto pos = path.find_last_of("/\\");
#else
        auto pos = path.find_last_of("/");
#endif
        if (pos == path.npos) {
            return path;
        } else {
            return path.substr(pos + 1);
        }
    }

 private:
    SchemeType scheme_;
    std::string uri_;
    std::string path_;
    std::string host_;
    uint16_t port_ = 0;
};

class FileSystem {
 protected:
    FileSystem() {}

 public:
    // get current executable path
    static inline FilePath GetExecutablePath() {
        std::string buf(FILENAME_MAX, 0);
#ifdef _WIN32
        auto size = GetModuleFileName(NULL, &buf[0], (DWORD)(buf.size()));
#elif __APPLE__
        uint32_t size = FILENAME_MAX;
        auto res = _NSGetExecutablePath(&buf[0], &size);
        if (res == -1) throw std::runtime_error("Error getting executable path.");
#else
        auto size = readlink("/proc/self/exe", &buf[0], buf.size());
        if (size == -1) throw std::runtime_error("Error getting executable path.");
#endif
        buf.resize(size);
        return FilePath(buf);
    }

    static inline std::string GetWorkingDirectory() {
        char buff[FILENAME_MAX];
#ifdef _WIN32
        char* ret = _getcwd(buff, FILENAME_MAX);
#else
        char* ret = getcwd(buff, FILENAME_MAX);
#endif
        if (!ret) throw std::runtime_error("Failed to get current working directory.");
        return buff;
    }

    DISABLE_COPY(FileSystem);

    /*!
     * \fn    static FileSystem* FileSystem::GetFileSystem(const std::string& uri);
     *
     * \brief    Gets the file system instance corresponding to the given uri.
     *
     * \param    uri    URI of the document.
     *
     * \return    Null if it fails, else the file system.
     */
    inline static FileSystem& GetFileSystem(const std::string& uri);

    /*!
     * \fn    static FileSystem* FileSystem::GetFileSystem(FilePath::SchemeType scheme);
     *
     * \brief    Gets the file system instance corresponding to the given scheme.
     *
     * \param    scheme    The scheme.
     *
     * \return    Null if it fails, else the file system.
     */
    inline static FileSystem& GetFileSystem(FilePath::SchemeType scheme);

    /*!
     * \fn    virtual bool FileSystem::CopyToLocal(const std::string& src, const std::string& dst)
     * const = 0;
     *
     * \brief    Copies a file to local.
     *
     * \param    src    Path of the source file.
     * \param    dst    Path of destination.
     *
     * \return    True if it succeeds, false if it fails.
     */
    virtual bool CopyToLocal(const std::string& src, const std::string& dst) const = 0;

    /*!
     * \fn    virtual bool FileSystem::CopyFromLocal(const std::string& src, const std::string& dst)
     * const = 0;
     *
     * \brief    Copies from local.
     *
     * \param    src    Path of the source file.
     * \param    dst    Path of destination.
     *
     * \return    True if it succeeds, false if it fails.
     */
    virtual bool CopyFromLocal(const std::string& src, const std::string& dst) const = 0;

    /*!
     * \fn    virtual bool FileSystem::FileExists(const std::string& file) const = 0;
     *
     * \brief    Queries if a given file exists.
     *
     * \param    file    Path of the file.
     *
     * \return    True if the path exists, and it is a file.
     */
    virtual bool FileExists(const std::string& file) const = 0;

    /*!
     * \fn    virtual bool FileSystem::Mkdir(const std::string& dir) const = 0;
     *
     * \brief    Makes the given dir, creating parent if necessary.
     *
     * \param    dir    The dir.
     *
     * \return    True if it succeeds, false if it fails.
     */
    virtual bool Mkdir(const std::string& dir) const = 0;

    /*!
     * \fn    virtual bool FileSystem::Remove(const std::string& file) const = 0;
     *
     * \brief    Removes the given file.
     *
     * \param    file    The file to remove.
     *
     * \return    True if it succeeds, false if it fails.
     */
    virtual bool Remove(const std::string& file) const = 0;

    virtual bool RemoveDir(const std::string& file) const = 0;

    /*!
     * \fn    virtual size_t FileSystem::GetFileSize(const std::string& file) const = 0;
     *
     * \brief    Gets file size.
     *
     * \param    file    The file.
     *
     * \return    The file size.
     */
    virtual size_t GetFileSize(const std::string& file) const = 0;

    /*!
     * \fn    virtual bool FileSystem::IsDir(const std::string& path) const = 0;
     *
     * \brief    Query if 'path' is a directory.
     *
     * \param    path    The path.
     *
     * \return    True if it is a dir, false if not.
     */
    virtual bool IsDir(const std::string& path) const = 0;

    /**
     * List all files under the directory.
     *
     * \param           dir     The dir.
     * \param [in,out]  sizes   If non-null, returns the sizes of the files.
     *
     * \return  A std::vector&lt;std::string&gt;
     */
    virtual std::vector<std::string> ListFiles(const std::string& dir,
                                               std::vector<size_t>* sizes = nullptr,
                                               bool full_path = true) const = 0;

    /*!
     * \fn    virtual std::vector<std::string> FileSystem::ListSubDirs(const std::string& dir) const
     * = 0;
     *
     * \brief    List sub directories under the directory.
     *
     * \param    dir    The dir.
     *
     * \return    A std::vector&lt;std::string&gt;
     */
    virtual std::vector<std::string> ListSubDirs(const std::string& dir,
                                                 bool full_path = true) const = 0;

    /*!
     * \fn    virtual std::string FileSystem::GetFileName(const std::string& path) const = 0;
     *
     * \brief    Gets the file name in the path. That is, the last part of
     *             the path.
     *
     * \param    path    The path.
     *
     * \return    The file name.
     */
    virtual std::string GetFileName(const std::string& path) const {
#ifdef _WIN32
        const char* seps = "\\/";
#else
        const char* seps = "/";
#endif
        size_t pos = path.find_last_of(seps);
        if (pos == path.npos) {
            return path;
        } else {
            return path.substr(pos + 1);
        }
    }

    virtual std::string PathSeparater() const { return "/"; }
};

class LocalFileSystem : public FileSystem {
 public:
    /*!
     * \fn  static inline const std::string& FileUtils::PATH_SEPERATOR()
     *
     * \brief   Path seperator.
     *
     * \return  Path seperator, "\\" for windows, "/" for linux
     */
    static inline const std::string& PATH_SEPERATOR() {
#ifdef _WIN32
        static const std::string r = "\\";
#else
        static const std::string r = "/";
#endif
        return r;
    }

    inline static LocalFileSystem& GetFileSystem() {
        static LocalFileSystem fs;
        return fs;
    }

    // throws exception on error
    bool CopyToLocal(const std::string& src, const std::string& dst) const override {
        _EC_TYPE_ ec;
#if FMA_USE_BOOST_FS
        if ((_STD_FS::is_regular_file(src) || _STD_FS::is_symlink(src)) &&
            _STD_FS::is_directory(dst)) {
            _STD_FS::copy(src, _STD_FS::path(dst) / _STD_FS::path(src).filename(), ec);
        } else {
            _STD_FS::copy(src, dst, ec);
        }
#else
        _STD_FS::copy(src, dst, _STD_FS::copy_options::recursive, ec);
#endif
        return !ec;
    }

    bool CopyFromLocal(const std::string& src, const std::string& dst) const override {
        return CopyToLocal(src, dst);
    }

    bool FileExists(const std::string& file) const override {
        _EC_TYPE_ ec;
        return _STD_FS::exists(file, ec);
    }

    bool Mkdir(const std::string& dir) const override {
        _EC_TYPE_ ec;
        _STD_FS::create_directories(dir, ec);
        return _STD_FS::is_directory(dir, ec);
    }

    bool Remove(const std::string& path) const override {
        _EC_TYPE_ ec;
        return _STD_FS::remove(path.c_str(), ec);
    }

    bool RemoveDir(const std::string& path) const override {
        _EC_TYPE_ ec;
        return _RMDIR(path, ec);
    }

    size_t GetFileSize(const std::string& file) const override {
        std::ifstream in(file, std::ifstream::binary);
        std::streampos beg = in.tellg();
        in.seekg(0, std::ifstream::end);
        return in.tellg() - beg;
    }

    bool IsDir(const std::string& path) const override {
        return _STD_FS::is_directory(path);
    }

    std::vector<std::string> ListFiles(const std::string& dir,
                                               std::vector<size_t>* f_sizes = nullptr,
                                               bool full_path = true) const override {
        std::vector<std::string> ret;
#ifdef _WIN32
        std::string search_path = dir + PATH_SEPERATOR() + "*";
        WIN32_FIND_DATA fd;
        HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                // read all (real) files in current folder
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
                    std::string fn = fd.cFileName;
                    ret.push_back(full_path ? _STD_FS::path(dir).append(fn).string() : fn);
                    if (f_sizes) {
                        uint64_t fsize = fd.nFileSizeHigh;
                        fsize = (fsize << 32) + fd.nFileSizeLow;
                        f_sizes->push_back(fsize);
                    }
                }
            } while (::FindNextFile(hFind, &fd));
            ::FindClose(hFind);
        }
#else
        struct dirent* entry;
        DIR* d = opendir(dir.c_str());
        if (d != NULL) {
            while ((entry = readdir(d)) != NULL) {
                if (entry->d_type == DT_REG) {
                    if (entry->d_name[0] == '.') continue;
                    std::string p = dir + PATH_SEPERATOR() + entry->d_name;
                    ret.push_back(full_path ? p : entry->d_name);
                    if (f_sizes) f_sizes->push_back(GetFileSize(p));
                }
            }
            closedir(d);
        }
#endif
        return ret;
    }

    std::vector<std::string> ListSubDirs(const std::string& dir,
                                                 bool full_path = true) const override {
        std::vector<std::string> ret;
#ifdef _WIN32
        std::string search_path = dir + PATH_SEPERATOR() + "*";
        WIN32_FIND_DATA fd;
        HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                // read all dirs in current folder
                std::string fn = fd.cFileName;
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && fn != "." && fn != "..") {
                    ret.push_back(full_path ? dir + PATH_SEPERATOR() + fn : fn);
                }
            } while (::FindNextFile(hFind, &fd));
            ::FindClose(hFind);
        }
#else
        struct dirent* entry;
        DIR* d = opendir(dir.c_str());
        if (d != NULL) {
            while ((entry = readdir(d)) != NULL) {
                if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
                    ret.push_back(full_path ? dir + PATH_SEPERATOR() + entry->d_name
                                            : entry->d_name);
                }
            }
            closedir(d);
        }
#endif
        return ret;
    }

    std::string PathSeparater() const override {
#ifdef _WIN32
        return "\\";
#else
        return "/";
#endif
    }
};

class HdfsFileSystem : public FileSystem {
 public:
    inline static HdfsFileSystem& GetFileSystem() {
#ifdef _WIN32
        static bool HdfsCmdOk = CheckHdfsCmd();
#else
        static bool HdfsCmdOk __attribute__((used)) = CheckHdfsCmd();
#endif
        static HdfsFileSystem fs;
        return fs;
    }

    bool CopyToLocal(const std::string& src, const std::string& dst) const override {
        if (LocalFileSystem::GetFileSystem().FileExists(dst)) {
            LocalFileSystem::GetFileSystem().Remove(dst);
        }
        return ExecCmd(StringFormatter::Format("{} -get \"{}\" \"{}\"", HDFS_CMD(), src, dst)) == 0;
    }

    bool CopyFromLocal(const std::string& local, const std::string& remote) const override {
        return ExecCmd(StringFormatter::Format("{} -put -f \"{}\" \"{}\"", HDFS_CMD(), local,
                                               remote)) == 0;
    }

    bool FileExists(const std::string& file) const override {
        return ExecCmd(StringFormatter::Format("{} -test -f \"{}\"", HDFS_CMD(), file)) == 0;
    }

    bool Mkdir(const std::string& dir) const override {
        return ExecCmd(StringFormatter::Format("{} -mkdir -p \"{}\"", HDFS_CMD(), dir)) == 0;
    }

    bool Remove(const std::string& path) const override {
        return ExecCmd(StringFormatter::Format("{} -rm -f \"{}\"", HDFS_CMD(), path)) == 0;
    }

    bool RemoveDir(const std::string& path) const override {
        return ExecCmd(StringFormatter::Format("{} -rm -rf \"{}\"", HDFS_CMD(), path)) == 0;
    }

    size_t GetFileSize(const std::string& file) const override {
        InputPipeStream stream(StringFormatter::Format("{} -ls \"{}\"", HDFS_CMD(), file));
        StreamLineReader reader(stream);
        auto lines = reader.ReadAllLines();
        if (lines.empty()) return 0;
        if (StartsWith(lines[0], "Found ") || StartsWith(lines[0], "ls: ")) return 0;
        auto fields = Split(lines[0]);
        if (fields.size() < 8) {
            LOG_ERROR() << "Error parsing the command line output when trying to "
                         "get the size of file "
                      << file << ", output is: " << lines[0];
            return 0;
        }
        size_t s = 0;
        ParseString(fields[4], s);
        return s;
    }

    bool IsDir(const std::string& path) const override {
#if _WIN32
        int r = ExecCmd(StringFormatter::Format("call {} -test -d \"{}\"", HDFS_CMD(), path));
#else
        int r = ExecCmd(StringFormatter::Format("{} -test -d \"{}\"", HDFS_CMD(), path));
#endif
        return r == 0;
    }

    std::vector<std::string> ListFiles(const std::string& dir,
                                               std::vector<size_t>* f_sizes = nullptr,
                                               bool full_path = true) const override {
#if FMA_HAS_LIBHDFS
        auto file_path_ = FilePath(dir);
        std::vector<std::string> ret;
        hdfsFS hdfs_fs_ = nullptr;
        hdfs_fs_ = hdfsConnect(file_path_.Host().c_str(), file_path_.Port());
        if (!hdfs_fs_) {
            LOG_WARN() << "failed to connect to name node: " << file_path_.Host() << ":"
                       << file_path_.Port() << "\n";
            return ret;
        }
        int num_entries_ = 0;
        hdfsFileInfo* hf_info = hdfsListDirectory(hdfs_fs_, dir.c_str(), &num_entries_);
        for (int i = 0; i < num_entries_; ++i) {
            if (hf_info[i].mKind == kObjectKindFile) {
                ret.push_back(full_path ? std::string(hf_info[i].mName)
                                        : FilePath::GetFileNameFromPath(hf_info[i].mName));
            }
        }
        hdfsFreeFileInfo(hf_info);
        return ret;
#else
        std::vector<std::string> ret;
        InputPipeStream stream(StringFormatter::Format("{} -ls \"{}\"", HDFS_CMD(), dir));
        StreamLineReader reader(stream);
        auto lines = reader.ReadAllLines();
        if (lines.empty()) return ret;
        if (StartsWith(lines[0], "ls: ")) return ret;
        if (StartsWith(lines[0], "Found ")) lines.erase(lines.begin());
        for (auto& line : lines) {
            if (line.empty()) continue;
            // each line is in format like this:
            //     -rw-r--r--   1 hct supergroup      15429 2017-05-31 13:43 /LICENSE.txt
            char type;
            int64_t size;
            std::string path;
            TextParserUtils::DropField _;
            size_t s;
            bool success;
            std::tie(s, success) = TextParserUtils::ParseAsTuple(&line[0], &line[line.size()], _,
                                                                 type, _, _, size, _, _, path);
            if (!success) {
                LOG_ERROR() << "Error parsing output of hdfs dfs -ls command "
                          << " when trying to list files in dir " << dir
                          << ": line is mis-formed: " << line;
                break;
            }
            if (type != '-') {
                if (f_sizes) f_sizes->push_back(size);
                ret.push_back(full_path ? path : FilePath::GetFileNameFromPath(path));
            }
        }
        return ret;
#endif
    }

    std::vector<std::string> ListSubDirs(const std::string& dir,
                                                 bool full_path = true) const override {
#if FMA_HAS_LIBHDFS
        auto file_path_ = FilePath(dir);
        std::vector<std::string> ret;
        hdfsFS hdfs_fs_ = nullptr;
        hdfs_fs_ = hdfsConnect(file_path_.Host().c_str(), file_path_.Port());
        if (!hdfs_fs_) {
            LOG_WARN() << "failed to connect to name node: " << file_path_.Host() << ":"
                       << file_path_.Port() << "\n";
            return ret;
        }
        int num_entries_ = 0;
        hdfsFileInfo* hf_info = hdfsListDirectory(hdfs_fs_, dir.c_str(), &num_entries_);
        for (int i = 0; i < num_entries_; ++i) {
            if (hf_info[i].mKind == kObjectKindDirectory) {
                ret.push_back(full_path ? std::string(hf_info[i].mName)
                                        : FilePath::GetFileNameFromPath(hf_info[i].mName));
            }
        }
        hdfsFreeFileInfo(hf_info);
        return ret;
#else
        std::vector<std::string> ret;
        InputPipeStream stream(StringFormatter::Format("{} -ls \"{}\"", HDFS_CMD(), dir));
        StreamLineReader reader(stream);
        auto lines = reader.ReadAllLines();
        if (lines.empty()) return ret;
        if (StartsWith(lines[0], "ls: ")) return ret;
        if (StartsWith(lines[0], "Found ")) lines.erase(lines.begin());
        for (auto& line : lines) {
            if (line.empty()) continue;
            // each line is in format like this:
            //     -rw-r--r--   1 hct supergroup      15429 2017-05-31 13:43 /LICENSE.txt
            char type;
            int64_t size;
            std::string path;
            TextParserUtils::DropField _;
            size_t s;
            bool success;
            std::tie(s, success) = TextParserUtils::ParseAsTuple(&line[0], &line[line.size()], _,
                                                                 type, _, _, size, _, _, path);
            if (!success) {
                LOG_ERROR() << "Error parsing output of hdfs dfs -ls command "
                          << " when trying to list files in dir " << dir
                          << ": line is mis-formed: " << line;
                break;
            }
            if (type == '-') {
                ret.push_back(full_path ? path : FilePath::GetFileNameFromPath(path));
            }
        }
        return ret;
#endif
    }

 private:
    static bool CheckHdfsCmd() {
#if FMA_HAS_LIBHDFS
        return true;
#else
        int r = ExecCmd(HDFS_CMD() + " -ls");
        FMA_ASSERT(r == 0) << "Cannot execute hdfs command, invalid path?\n"
                           << "fma-common/predefs.h::HDFS_CMD()=" << HDFS_CMD();
        return true;
#endif
    }
};

class SQLFileSystem : public FileSystem {
 public:
    inline static SQLFileSystem& GetFileSystem() {
        static SQLFileSystem fs;
        return fs;
    }

    bool CopyToLocal(const std::string& src, const std::string& dst) const override {
        throw std::runtime_error(
            "SQLFileSystem::CopyToLocal(const std::string& src, const std::string& dst) "
            "is not implemented for SQLFileSystem");
    }

    bool CopyFromLocal(const std::string& src, const std::string& dst) const override {
        throw std::runtime_error(
            "SQLFileSystem::CopyFromLocal(const std::string& src, const std::string& dst) "
            "is not implemented for SQLFileSystem");
    }

    bool FileExists(const std::string& file) const override {
        // WARN() << "SQLFileSystem::FileExists(const std::string& file) return true directly";
        bool fake_res = true;
        return fake_res;
    }

    bool Mkdir(const std::string& dir) const override {
        throw std::runtime_error(
            "SQLFileSystem::FileExists(const std::string& file) "
            "is not implemented for SQLFileSystem");
    }

    bool Remove(const std::string& path) const override {
        throw std::runtime_error(
            "SQLFileSystem::Remove(const std::string& path) "
            "is not implemented for SQLFileSystem");
    }

    bool RemoveDir(const std::string& path) const override {
        throw std::runtime_error(
            "SQLFileSystem::Remove(const std::string& path) "
            "is not implemented for SQLFileSystem");
    }

    size_t GetFileSize(const std::string& file) const override {
        // WARN() << "SQLFileSystem::GetFileSize(const std::string& file) return 0 directly";
        size_t fake_size = 1;
        return fake_size;
    }

    bool IsDir(const std::string& path) const override {
        throw std::runtime_error(
            "SQLFileSystem::IsDir(const std::string& path) "
            "is not implemented for SQLFileSystem");
    }

    std::vector<std::string> ListFiles(const std::string& dir,
                                               std::vector<size_t>* f_sizes = nullptr,
                                               bool full_path = true) const override {
        throw std::runtime_error(
            "ListFiles(const std::string& dir, "
            "std::vector<size_t>* f_sizes = nullptr) const override "
            "is not implemented for SQLFileSystem");
    }

    std::vector<std::string> ListSubDirs(const std::string& dir,
                                                 bool full_path = true) const override {
        throw std::runtime_error(
            "SQLFileSystem::ListSubDirs(const std::string& dir) "
            "is not implemented for SQLFileSystem");
    }
};

inline FileSystem& FileSystem::GetFileSystem(const std::string& uri) {
    FilePath::SchemeType scheme = FilePath(uri).Scheme();
    return FileSystem::GetFileSystem(scheme);
}

inline FileSystem& FileSystem::GetFileSystem(FilePath::SchemeType scheme) {
    if (scheme == FilePath::SchemeType::LOCAL) {
        return LocalFileSystem::GetFileSystem();
    } else if (scheme == FilePath::SchemeType::HDFS) {
        return HdfsFileSystem::GetFileSystem();
    } else if (scheme == FilePath::SchemeType::MYSQL || scheme == FilePath::SchemeType::SQLSERVER) {
        return SQLFileSystem::GetFileSystem();
    } else {
        LOG_ERROR() << "Unsupported file system type: " << (int)scheme;
        return LocalFileSystem::GetFileSystem();
    }
}

struct DiskInfo {  // unit: B
    uint64_t total;
    uint64_t avail;
};

// by qinwei GetDirSpace
#ifdef _WIN32
inline size_t GetDirSpace(const char* pathname) {
    WIN32_FIND_DATAA data;
    HANDLE sh = NULL;
    std::string _path = pathname;
    size_t totalSize = 0;
    sh = FindFirstFileA((_path + "\\*").c_str(), &data);
    if (sh == INVALID_HANDLE_VALUE) return 0;
    do {
        if (std::string(data.cFileName).compare(".") != 0 &&
            std::string(data.cFileName).compare("..") != 0) {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
                totalSize += GetDirSpace((_path + "\\" + data.cFileName).c_str());
            } else {
                totalSize +=
                    ((size_t)(data.nFileSizeHigh) * (MAXDWORD) + (size_t)(data.nFileSizeLow));
            }
        }
    } while (FindNextFileA(sh, &data));  // do
    FindClose(sh);
    return totalSize;
}

inline void GetDiskInfo(struct DiskInfo& diskInfo, const char* pathname) {
    diskInfo.total = 0;
    diskInfo.avail = 0;
}
#else
inline size_t GetDirSpace(const char* pathname) {
    if (pathname == NULL) {
        printf("Erorr: pathname is NULL\n");
        return 0;
    }
    struct stat stats;
    if (lstat(pathname, &stats) == 0) {
        if (S_ISREG(stats.st_mode)) {
            return stats.st_size;
        }
    } else {
        printf("lstat error\n");
        return 0;
    }
    DIR* dir = opendir(pathname);
    if (dir == NULL) {
        printf("Open Error\n");
        return 0;
    }
    struct dirent* dirEntry;
    size_t totalSize = 4096;
    for (dirEntry = readdir(dir); dirEntry != NULL; dirEntry = readdir(dir)) {
        size_t pathLength = sizeof(char) * (strlen(pathname) + strlen(dirEntry->d_name) + 2);
        char* name = (char*)malloc(pathLength);
        snprintf(name, pathLength, "%s/%s", pathname, dirEntry->d_name);
        if (dirEntry->d_type == DT_DIR) {
            if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0) {
                totalSize += GetDirSpace(name);
            }
        } else {
            int status = lstat(name, &stats);
            if (status == 0) {
                totalSize += stats.st_size;
            } else {
                printf("lstat\n");
            }
        }
        free(name);
    }
    closedir(dir);
    return totalSize;
}

inline void GetDiskInfo(struct DiskInfo& diskInfo, const char* pathname) {
    diskInfo.total = 0;
    diskInfo.avail = 0;
    if (pathname == NULL) {
        printf("Erorr: pathname is NULL\n");
        return;
    }
    struct statvfs fiData;
    if ((statvfs(pathname, &fiData)) < 0) {
        printf("Failed to stat: %s\n", pathname);
    } else {
        diskInfo.total = fiData.f_bsize * fiData.f_blocks;
        diskInfo.avail = fiData.f_bsize * fiData.f_bavail;
    }
}
#endif

namespace file_system {
static inline bool RemoveDir(const std::string& dir) {
    return FileSystem::GetFileSystem(dir).RemoveDir(dir);
}

static inline bool RemoveFile(const std::string& file) {
    return FileSystem::GetFileSystem(file).Remove(file);
}

static inline bool DirExists(const std::string& dir) {
    return FileSystem::GetFileSystem(dir).IsDir(dir);
}

static inline bool FileExists(const std::string& file) {
    return FileSystem::GetFileSystem(file).FileExists(file);
}

static inline bool MkDir(const std::string& dir) {
    return FileSystem::GetFileSystem(dir).Mkdir(dir);
}

static inline size_t GetFileSize(const std::string& file) {
    return FileSystem::GetFileSystem(file).GetFileSize(file);
}

static inline std::vector<std::string> ListFiles(const std::string& dir, std::vector<size_t>* sizes,
                                                 bool full_path = true) {
    return FileSystem::GetFileSystem(dir).ListFiles(dir, sizes, full_path);
}

static inline std::vector<std::string> ListSubDirs(const std::string& dir, bool full_path = true) {
    return FileSystem::GetFileSystem(dir).ListSubDirs(dir, full_path);
}

static std::string
#ifdef __GNUC__
    __attribute__((__unused__))
#endif
    _JoinPath_(const std::string& first) {
    return first;
}

template <typename... T>
static std::string _JoinPath_(const std::string& first, const T&... parts) {
    return (_STD_FS::path(first) / _JoinPath_(parts...)).string();
}

template <typename... T>
static std::string JoinPath(const T&... parts) {
    return _JoinPath_(parts...);
}
}  // namespace file_system
}  // namespace fma_common
