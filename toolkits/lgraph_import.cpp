
/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/check_date.h"
#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/timed_task.h"
#include "fma-common/hardware_info.h"
#include "import/import_v2.h"
#include "import/import_client.h"
#include "import/parse_delimiter.h"

using namespace fma_common;
using namespace lgraph;
using namespace import_v2;

int main(int argc, char** argv) {
    Importer::Config import_config;
    OnlineImportClient::Config online_import_config;

    std::string log_file;
    int verbose_level = 1;
    bool memory_profile = false;

    bool online = false;
    {
        fma_common::Configuration config;
        config.Add(online, "online", true).Comment("Whether to import online");
        config.ParseAndRemove(&argc, &argv);
        config.ExitAfterHelp(false);
        config.Finalize();
    }

    if (online) {
        fma_common::Configuration config;
        config.Add(log_file, "log", true).Comment("Log file to use, empty means stderr");
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
    } else {
        fma_common::Configuration config;
        config.Add(log_file, "log", true).Comment("Log file to use, empty means stderr");
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
    }

    // setup logging
    fma_common::LogLevel llevel;
    switch (verbose_level) {
    case 0:
        llevel = fma_common::LogLevel::LL_ERROR;
        break;
    case 1:
        llevel = fma_common::LogLevel::LL_INFO;
        break;
    default:
        memory_profile = true;
        llevel = fma_common::LogLevel::LL_DEBUG;
    }
    fma_common::Logger::Get().SetLevel(llevel);
    if (!log_file.empty()) {
        fma_common::Logger::Get().SetDevice(
            std::shared_ptr<fma_common::LogDevice>(new fma_common::FileLogDevice(log_file)));
    }
    fma_common::Logger::Get().SetFormatter(
        std::shared_ptr<fma_common::LogFormatter>(new fma_common::TimedModuleLogFormatter()));

    try {
        if (online) {
            FMA_LOG() << "Importing ONLINE: "
                      << "\n\tfrom:                " << online_import_config.config_file
                      << "\n\tto:                  " << online_import_config.url
                      << "\n\tverbose:             " << verbose_level
                      << "\n\tlog:                 " << log_file;
            online_import_config.delimiter = lgraph::ParseDelimiter(online_import_config.delimiter);
            OnlineImportClient client(online_import_config);
            client.DoImport();
        } else {
            FMA_LOG() << "Importing FROM SCRATCH: "
                      << "\n\tfrom:                " << import_config.config_file
                      << "\n\tto:                  " << import_config.db_dir
                      << "\n\tverbose:             " << verbose_level
                      << "\n\tlog:                 " << log_file;
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
        }
    } catch (std::exception& e) {
        FMA_LOG() << "An error occurred during import:\n" << PrintNestedException(e, 1);
        return 1;
    }
    return 0;
}
