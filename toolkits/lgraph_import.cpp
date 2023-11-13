
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

#include "fma-common/check_date.h"
#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/timed_task.h"
#include "fma-common/hardware_info.h"
#include "import/import_v2.h"
#include "import/import_v3.h"
#include "import/import_client.h"
#include "import/parse_delimiter.h"

using namespace fma_common;
using namespace lgraph;
using namespace import_v2;

int main(int argc, char** argv) {
    Importer::Config import_config;
    import_v3::Importer::Config import_config_v3;
    OnlineImportClient::Config online_import_config;

    std::string log_dir;
    int verbose_level = 1;
    bool memory_profile = false;

    bool online = false;
    bool v3 = true;
    {
        fma_common::Configuration config;
        config.Add(online, "online", true).Comment("Whether to import online");
        config.Add(v3, "v3", true).Comment("Whether to use lgraph import V3");
        config.ParseAndRemove(&argc, &argv);
        config.ExitAfterHelp(false);
        config.Finalize();
    }

    if (online) {
        fma_common::Configuration config;
        config.Add(log_dir, "log", true).Comment("Log dir to use, empty means stderr");
        config.Add(verbose_level, "v,verbose", true)
            .Comment("Verbose level to use, higher means more verbose");
        config.Add(online_import_config.config_file, "c,config_file", false)
            .Comment("Config file path");
        config.Add(online_import_config.url, "r,url", false).Comment("DB REST API address");
        config.Add(online_import_config.username, "u,user", false).Comment("DB username");
        config.Add(online_import_config.password, "p,password", false).Comment("DB password");
        config.Add(online_import_config.continue_on_error, "i,continue_on_error", true)
            .Comment("When we hit a duplicate uid or missing uid, should we continue or abort");
        config.Add(online_import_config.graph_name, "g,graph", true)
            .Comment("The name of the graph to import into");
        config.Add(online_import_config.skip_packages, "skip_packages", true)
            .Comment("How many packages should we skip");
        config.Add(online_import_config.delimiter, "delimiter", true)
            .Comment("Delimiter used in the CSV files");
        config.Add(online_import_config.breakpoint_continue, "breakpoint_continue", true)
            .Comment("Delimiter used in the CSV files");
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    } else if (!v3) {
        fma_common::Configuration config;
        config.Add(log_dir, "log", true).Comment("Log dir to use, empty means stderr");
        config.Add(verbose_level, "v,verbose", true)
            .Comment("Verbose level to use, higher means more verbose");
        config.Add(import_config.config_file, "c,config_file", false).Comment("Config file path");
        config.Add(import_config.bucket_size_mb, "bucket_memory_mb", true)
            .SetMin(1)
            .SetMax(1 << 20)
            .Comment("Average memory size of each bucket in MB");
        config.Add(import_config.continue_on_error, "i,continue_on_error", true)
            .Comment("When we hit a duplicate uid or missing uid, should we continue or abort");
        config.Add(import_config.db_dir, "d,dir", true).Comment("The DB data directory");
        config.Add(import_config.user, "u,user", true).Comment("DB username.");
        config.Add(import_config.password, "p,password", true).Comment("DB password.");
        config.Add(import_config.graph, "g,graph", true)
            .Comment("The name of the graph to import into");
        config.Add(import_config.delete_if_exists, "overwrite", true)
            .Comment("Whether to overwrite the existing DB if it already exists");
        config.Add(import_config.dry_run, "dry_run", true)
            .Comment("A dry run simulates the import process without actually writing the DB");
        config.Add(import_config.intermediate_buf_size, "ibuf_size", true)
            .SetMin(1024)
            .SetMax(1 << 30)
            .Comment("Buffer size for each intermediate file in bytes");
        config.Add(import_config.intermediate_dir, "idir", true)
            .Comment("Directory used to store intermediate files");
        config.Add(import_config.n_parser_threads, "n_parser", true)
            .SetMin(1)
            .SetMax(100)
            .Comment("Number of text parser threads to use");
        config.Add(import_config.n_stichers, "n_stitcher", true)
            .SetMin(1)
            .SetMax(100)
            .Comment("Number of stitcher threads to use");
        /* not useful
        config.Add(import_config.parse_block_size, "parse_block_size", true)
            .SetMin(1024)
            .Comment("Size of each text block");
            */
        config.Add(import_config.n_ireaders, "n_ireader", true)
            .SetMin(1)
            .SetMax(100)
            .Comment("Number of threads to use when reading intermediate files");
        config.Add(import_config.n_sorters, "n_sorter", true)
            .SetMin(1)
            .SetMax(100)
            .Comment("Number of threads to use when sorting edge data");
        config.Add(import_config.n_packers, "n_packer", true)
            .SetMin(1)
            .SetMax(100)
            .Comment("Number of threads to use when packing values into KVs");
        config.Add(import_config.keep_intermediate_files, "k,keep_ifiles", true)
            .Comment("Whether to keep intermediate files after import");
        config.Add(import_config.quiet, "quiet", true)
            .Comment("Do not print error message when continue_on_error==true");
        config.Add(memory_profile, "memory_profile", true)
            .Comment("Whether to dump memory size info while doing import");
        config.Add(import_config.delimiter, "delimiter", true)
            .Comment("Delimiter used in the CSV files");
        config.Add(import_config.enable_fulltext_index, "enable_fulltext_index", true)
                .Comment("Whether to enable fulltext index");
        config.Add(import_config.fulltext_index_analyzer, "fulltext_index_analyzer", true)
                .SetPossibleValues({"SmartChineseAnalyzer", "StandardAnalyzer"})
                .Comment("fulltext index analyzer");
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    } else {
        fma_common::Configuration config;
        config.Add(log_dir, "log", true).Comment("Log dir to use, empty means stderr");
        config.Add(verbose_level, "v,verbose", true)
            .Comment("Verbose level to use, higher means more verbose");
        config.Add(import_config_v3.config_file, "c,config_file", false)
            .Comment("Config file path");
        config.Add(import_config_v3.continue_on_error, "i,continue_on_error", true);
        config.Add(import_config_v3.db_dir, "d,dir", true).Comment("The DB data directory");
        config.Add(import_config_v3.user, "u,user", true).Comment("DB username.");
        config.Add(import_config_v3.password, "p,password", true).Comment("DB password.");
        config.Add(import_config_v3.graph, "g,graph", true);
        config.Add(import_config_v3.delete_if_exists, "overwrite", true)
            .Comment("Whether to overwrite the existing DB if it already exists");
        config.Add(import_config_v3.quiet, "quiet", true)
            .Comment("Do not print error message when continue_on_error==true");
        config.Add(import_config_v3.delimiter, "delimiter", true)
            .Comment("Delimiter used in the CSV files");
        config.Add(import_config_v3.parse_block_size, "parse_block_size", true)
            .Comment("Block size per parse");
        config.Add(import_config_v3.parse_block_threads, "parse_block_threads", true)
            .Comment("How many threads to parse the data block");
        config.Add(import_config_v3.parse_file_threads, "parse_file_threads", true)
            .Comment("How many threads to parse the files");
        config.Add(import_config_v3.generate_sst_threads, "generate_sst_threads", true)
            .Comment("How many threads to generate sst files");
        config.Add(import_config_v3.read_rocksdb_threads, "read_rocksdb_threads", true)
            .Comment("How many threads to read rocksdb in the final stage");
        config.Add(import_config_v3.vid_num_per_reading, "vid_num_per_reading", true)
            .Comment("How many vertex data to read each time");
        config.Add(import_config_v3.max_size_per_reading, "max_size_per_reading", true)
            .Comment("Maximum size of kvs per reading");
        config.Add(import_config_v3.compact, "compact", true)
            .Comment("Whether to compact");
        config.Add(import_config_v3.keep_vid_in_memory, "keep_vid_in_memory", true)
            .Comment("Whether to keep vids in memory");
        config.Add(import_config_v3.enable_fulltext_index, "enable_fulltext_index", true)
            .Comment("Whether to enable fulltext index");
        config.Add(import_config_v3.fulltext_index_analyzer, "fulltext_index_analyzer", true)
            .SetPossibleValues({"SmartChineseAnalyzer", "StandardAnalyzer"})
            .Comment("fulltext index analyzer");
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    }

    // Setup lgraph log
    lgraph_log::severity_level vlevel;
    switch (verbose_level) {
    case 0:
        vlevel = lgraph_log::ERROR;
        break;
    case 1:
        vlevel = lgraph_log::INFO;
        break;
    case 2:
    default:
        vlevel = lgraph_log::DEBUG;
        break;
    }
    lgraph_log::LoggerManager::GetInstance().Init(log_dir, vlevel);

    try {
        if (online) {
            FMA_LOG() << "Importing ONLINE: "
                      << "\n\tfrom:                " << online_import_config.config_file
                      << "\n\tto:                  " << online_import_config.url
                      << "\n\tverbose:             " << verbose_level
                      << "\n\tlog_dir:             " << log_dir;
            online_import_config.delimiter = lgraph::ParseDelimiter(online_import_config.delimiter);
            OnlineImportClient client(online_import_config);
            client.DoImport();
        } else if (!v3) {
            FMA_LOG() << "Importing FROM SCRATCH: "
                      << "\n\tfrom:                " << import_config.config_file
                      << "\n\tto:                  " << import_config.db_dir
                      << "\n\tverbose:             " << verbose_level
                      << "\n\tlog_dir:             " << log_dir;
            import_config.delimiter = lgraph::ParseDelimiter(import_config.delimiter);
            fma_common::TimedTaskScheduler scheduler;
            if (memory_profile) {
                scheduler.ScheduleReccurringTask(1000, [](TimedTask*) {
                    FMA_LOG() << "Current available memory: "
                              << fma_common::HardwareInfo::GetAvailableMemory() / 1024 / 1024 / 1024
                              << "GB";
                });
            }
            Importer importer(import_config);
            importer.DoImportOffline();
        } else {
            FMA_LOG() << "Importing FROM SCRATCH:   "
                      << "\n\tfrom:                 " << import_config_v3.config_file
                      << "\n\tto:                   " << import_config_v3.db_dir
                      << "\n\tverbose:              " << verbose_level
                      << "\n\tlog_dir:              " << log_dir
                      << "\n\tkeep_vid_in_memory:   " << import_config_v3.keep_vid_in_memory
                      << "\n\tparse_file_threads:   " << import_config_v3.parse_file_threads
                      << "\n\tparse_block_threads:  " << import_config_v3.parse_block_threads
                      << "\n\tparse_block_size:     " << import_config_v3.parse_block_size
                      << "\n\tgenerate_sst_threads: " << import_config_v3.generate_sst_threads
                      << "\n\tread_rocksdb_threads: " << import_config_v3.read_rocksdb_threads
                      << "\n\tvid_num_per_reading:  " << import_config_v3.vid_num_per_reading
                      << "\n\tmax_size_per_reading: " << import_config_v3.max_size_per_reading;
            import_config_v3.delimiter = lgraph::ParseDelimiter(import_config_v3.delimiter);
            import_v3::Importer importer(import_config_v3);
            importer.DoImportOffline();
        }
    } catch (std::exception& e) {
        FMA_LOG() << "An error occurred during import:\n" << PrintNestedException(e, 1);
        return 1;
    }
    fma_common::SleepS(3);  // waiting for memory reclaiming by async task
    return 0;
}
