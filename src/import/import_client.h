/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <signal.h>
#include <string>

#include "fma-common/fma_stream.h"

#include "import/import_config_parser.h"

namespace lgraph {
namespace import_v2 {

class OnlineImportClient {
 public:
    struct Config {
        std::string config_file = "";
        std::string url = "http://127.0.0.1:7071/";
        std::string username = "";
        std::string password = "";
        bool continue_on_error = false;
        size_t skip_packages = 0;
        std::string graph_name = "default";
        std::string delimiter = ",";
        std::string progress_log_file;
        bool breakpoint_continue = false;
    };
    explicit OnlineImportClient(const Config& config);
    void DoImport();

 protected:
    Config config_;

    static std::atomic<bool> exit_flag_;
    static void SignalHandler(int signum);

    static std::string FormatPercent(double p);
    static std::string FormatBytes(double n_bytes, int nums = 3);

 private:
    size_t ReadProcessedPackages(std::ifstream& is);
    void WriteProcessedLinePackages(std::ofstream& os, size_t packages);
};

}  // namespace import_v2
}  // namespace lgraph
