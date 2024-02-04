/**
 * Copyright 2024 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "fma-common/string_util.h"
#include "fma-common/utils.h"
#include "./unit_test_utils.h"

using namespace fma_common;

FMA_SET_TEST_PARAMS(FileSystem, "");

FMA_UNIT_TEST(FileSystem) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    Configuration config;
    std::string fs_type = "local";
    config.Add(fs_type, "fs,f", true)
        .Comment("Choose which type of file system to test")
        .SetPossibleValues({"local", "hdfs"});
    config.Parse(argc, argv);
    config.Finalize();

    {
#ifdef _WIN32
#else  // _WIN32
#if __cplusplus >= 201703L
        FMA_UT_CHECK_EQ(fma_common::file_system::JoinPath("/a", "b", "/c", "d"), "/c/d");
        FMA_UT_CHECK_EQ(fma_common::file_system::JoinPath("/a", "b", "c"), "/a/b/c");
        FMA_UT_CHECK_EQ(fma_common::file_system::JoinPath("./xx", "b", "./c", "d", "e"),
                     "./xx/b/./c/d/e");
#else  // __cplusplus >= 201703L
        FMA_UT_CHECK_EQ(fma_common::file_system::JoinPath("/a", "b", "/c"), "/a/b/c");
        FMA_UT_CHECK_EQ(fma_common::file_system::JoinPath("./xx", "b", "/c", "d", "e"),
                     "./xx/b/c/d/e");
#endif  // __cplusplus >= 201703L
#endif  // _WIN32
    }
    {
        auto path = FileSystem::GetExecutablePath();
        LOG_INFO() << "dir: " << path.Dir();
        LOG_INFO() << "name: " << path.Name();
        LOG_INFO() << FileSystem::GetWorkingDirectory();
        // by qinwei DirSpace
        LOG_INFO() << "dirSpace: " << GetDirSpace(path.Dir().c_str()) << " B";
        DiskInfo diskInfo;
#ifndef _WIN32
        GetDiskInfo(diskInfo, path.Dir().c_str());
#endif
        LOG_INFO() << "disk total: " << diskInfo.total << " B";
        LOG_INFO() << "disk avail: " << diskInfo.avail << " B";
    }
    std::string fname = LocalFileSystem::GetFileSystem().GetFileName(argv[0]);
#ifdef _WIN32
    if (!EndsWith(fname, ".exe", false)) fname += ".exe";
#endif
    FMA_UT_ASSERT(LocalFileSystem::GetFileSystem().FileExists(fname))
        << "File " << fname << " does not exist. We need it to do the test";
    if (fs_type == "local") {
        std::string dir1 = "kkkttt 1233";
        file_system::RemoveDir(dir1);
        std::string wdir = dir1 + LocalFileSystem::PATH_SEPERATOR() + "other";
        LocalFileSystem &fs = LocalFileSystem::GetFileSystem();
        fs.Mkdir(wdir);
        std::string file1 = wdir + LocalFileSystem::PATH_SEPERATOR() + fname;
        fs.CopyFromLocal(fname, wdir);
        FMA_UT_ASSERT(fs.FileExists(file1)) << "CopyFromLocal failed";
        std::string file2 = wdir + LocalFileSystem::PATH_SEPERATOR() + "argv0";
        fs.CopyToLocal(fname, file2);
        FMA_UT_ASSERT(fs.FileExists(file2)) << "CopyToLocal failed";
        FMA_UT_CHECK_EQ(fs.GetFileSize(file1), fs.GetFileSize(file2)) << "GetFileSize failed";
        FMA_UT_ASSERT(fs.IsDir(wdir)) << "IsDir(dir) failed";
        FMA_UT_ASSERT(!fs.IsDir(file2)) << "IsDir(file) failed";
        std::vector<size_t> sizes;
        auto files = fs.ListFiles(wdir, &sizes, true);
        FMA_UT_CHECK_EQ(files.size(), (size_t)2) << "ListFiles returned wrong result: "
            << ToString(files);
        FMA_UT_CHECK_EQ(sizes[0], fs.GetFileSize(file1));
        FMA_UT_CHECK_EQ(sizes[1], fs.GetFileSize(file2));
        auto dirs = fs.ListSubDirs(dir1);
        FMA_UT_CHECK_EQ(dirs.size(), (size_t)1) << "ListSubDirs returned wrong result: "
            << ToString(dirs);
        auto dirs2 = fs.ListSubDirs(wdir);
        FMA_UT_CHECK_EQ(dirs2.size(), (size_t)0)
            << "ListSubDirs for " << wdir << " returned wrong result: " << ToString(dirs2);
        auto sdir = file_system::JoinPath(wdir, "sub1", "sub2");
        fs.Mkdir(sdir);
        fs.CopyFromLocal(fname, sdir);
        FMA_UT_ASSERT(fs.FileExists(file_system::JoinPath(sdir, fname)));
        fs.RemoveDir(wdir);
        FMA_UT_ASSERT(!fs.IsDir(wdir));
        dirs2 = fs.ListSubDirs(dir1);
        FMA_UT_CHECK_EQ(dirs2.size(), 0);
        fs.RemoveDir(dir1);
        LOG_INFO() << "LocalFileSystem check passed";
    } else if (fs_type == "hdfs") {
        std::string hpdir = "test_file_system";
        std::string hcdir = "tmp";
        std::string wdir = hpdir + "/" + hcdir;
        HdfsFileSystem &hfs = HdfsFileSystem::GetFileSystem();
        hfs.Mkdir(wdir);
        std::string file1 = wdir + "/" + fname;
        hfs.CopyFromLocal(fname, file1);
        FMA_UT_ASSERT(hfs.FileExists(file1));
        std::string local2 = "local_copy_tmp";
        hfs.CopyToLocal(file1, local2);
        std::string file2 = wdir + "/local_copy_tmp";
        hfs.CopyFromLocal(local2, file2);
        FMA_UT_ASSERT(hfs.FileExists(file2));
        FMA_UT_CHECK_EQ(hfs.GetFileSize(file1), hfs.GetFileSize(file2));
        FMA_UT_ASSERT(hfs.IsDir(wdir));
        FMA_UT_ASSERT(!hfs.IsDir(file2));
        auto files = hfs.ListFiles(wdir);
        FMA_UT_CHECK_EQ(files.size(), (size_t)2) << "Got more than 2 files: " << ToString(files);
        hfs.Remove(files[0]);
        hfs.Remove(files[1]);
        FMA_UT_ASSERT(hfs.ListFiles(wdir).empty());
        auto dirs = hfs.ListSubDirs(hpdir);
        FMA_UT_CHECK_EQ(dirs.size(), (size_t)1);
        FMA_UT_ASSERT(hfs.ListSubDirs(wdir).empty());
        LOG_INFO() << "HdfsFileSystem check passed";
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
