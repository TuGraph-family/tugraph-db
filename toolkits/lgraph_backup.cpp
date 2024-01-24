
/**
 * Copyright 2022 AntGroup CO., Ltd.
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

#include "tools/lgraph_log.h"
#include "fma-common/configuration.h"
#include "fma-common/file_system.h"

#include "core/killable_rw_lock.h"
#include "db/galaxy.h"

int main(int argc, char** argv) {
    std::string src, dst;
    bool compact = true;

    fma_common::Configuration config;
    config.ExitAfterHelp(true);
    config.Add(src, "s,src", false).Comment("Source data directory");
    config.Add(dst, "d,dst", false).Comment("Destination data directory");
    config.Add(compact, "c,compact", true)
        .Comment(
            "Whether to compact the DB during backup. Compaction results in smaller DB, but "
            "increases backup time");
    try {
        config.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        LOG_ERROR() << e.what();
        return -1;
    }

    LOG_INFO() << "Backing up data from [" << src << "] to [" << dst << "]";
    // check if src exists
    if (!fma_common::file_system::DirExists(src)) {
        LOG_ERROR() << "Source DB does not exist!";
        return -1;
    }

    fma_common::FileSystem& fs = fma_common::FileSystem::GetFileSystem(dst);
    // check if dst exists
    std::string dst_data_file = dst + "/.meta/data.lgr";
    if (fs.IsDir(dst) && fs.FileExists(dst_data_file)) {
        bool overwrite = true;
        std::cout << "Destination database already exists, override? [y/N]";
        bool should_break = false;
        while (!should_break) {
            switch (std::getchar()) {
            case 'y':
            case 'Y':
                overwrite = true;
                should_break = true;
                break;
            case 'n':
            case 'N':
            case '\r':
            case '\n':
                overwrite = false;
                should_break = true;
                break;
            }
        }
        if (!overwrite) {
            LOG_INFO() << "Program stops without modifying data.";
            return -1;
        } else {
            LOG_INFO() << "Overwriting all the data in " << dst;
            fs.RemoveDir(dst);
        }
    } else {
        fs.Mkdir(dst);
    }
    try {
        // lock the whole galaxy
        lgraph::Galaxy src_galaxy(src, false);
        _HoldWriteLock(src_galaxy.GetReloadLock());
        src_galaxy.Backup(dst, compact);
    } catch (std::exception& e) {
        LOG_ERROR() << "Failed to backup the db: " << e.what();
        return -1;
    }
    return 0;
}
