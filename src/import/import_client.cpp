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

#include "import/import_client.h"
#include "import/file_cutter.h"
#include "client/cpp/restful/rest_client.h"
#include "fma-common/encrypt.h"

std::atomic<bool> lgraph::import_v2::OnlineImportClient::exit_flag_;

lgraph::import_v2::OnlineImportClient::OnlineImportClient(const Config& config) : config_(config) {
    fma_common::encrypt::Base64 base64;
    config_.progress_log_file = FMA_FMT(".prograss_{}.log", base64.Encode(config_.config_file));
}

void lgraph::import_v2::OnlineImportClient::DoImport() {
    double t1 = fma_common::GetTime();
    std::ifstream ifs(config_.config_file);
    nlohmann::json conf;
    ifs >> conf;

    if (conf.contains("schema")) {
        RestClient client(config_.url);
        if (config_.username.size()) {
            client.Login(config_.username, config_.password);
        }
        nlohmann::json schema_conf;
        schema_conf["schema"] = conf["schema"];
        std::string log = client.SendSchema(config_.graph_name, schema_conf.dump(4));
        if (log.size()) {
            FMA_WARN() << log;
        }
    }

    std::vector<CsvDesc> data_files = ImportConfParser::ParseFiles(conf);
    if (data_files.empty()) {
        double t2 = fma_common::GetTime();
        FMA_LOG() << "Import finished in " << t2 - t1 << " seconds.";
        return;
    }

    // vertex before edge
    std::stable_sort(data_files.begin(), data_files.end(), [](const CsvDesc& a, const CsvDesc& b) {
        return a.is_vertex_file > b.is_vertex_file;
    });

    size_t bytes_total = 0;
    for (const CsvDesc& fd : data_files) bytes_total += fd.size;
    FMA_LOG() << "Total file size " << FormatBytes((double)bytes_total, 6);

    exit_flag_ = false;
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    std::ifstream iplfs(config_.progress_log_file);
    FMA_LOG() << "config_.progress_log_file " << config_.progress_log_file;
    size_t packages_processed = ReadProcessedPackages(iplfs);
    std::ofstream oplfs(config_.progress_log_file);

    size_t bytes_cut = 0, bytes_sent = 0, skip_packages;
    if (config_.breakpoint_continue) {
        skip_packages =
            config_.skip_packages > packages_processed ? config_.skip_packages : packages_processed;
    } else {
        skip_packages = config_.skip_packages;
    }
    packages_processed = skip_packages;
    time_t start_time = time(nullptr);
    for (CsvDesc& fd : data_files) {
        const auto& filename = fd.path;
        std::string desc = fd.Dump();

        bool is_first_package = true;
        char *begin, *end;
        FileCutter cutter(filename);
        for (; cutter.Cut(begin, end); is_first_package = false) {
            if (exit_flag_) {
                exit(1);
            }
            bytes_cut += end - begin;
            if (skip_packages > 0) {
                FMA_LOG() << "skipping package " << packages_processed - skip_packages + 1;
                --skip_packages;
                continue;
            }
            ++packages_processed;
            FMA_LOG() << fma_common::StringFormatter::Format("importing ({}) packages",
                                                             packages_processed);
            if (is_first_package) {
                if (fd.n_header_line >
                    static_cast<size_t>(std::count(begin, end, '\n')) + (end[-1] != '\n'))
                    FMA_ERR() << "HEADER too large";
            } else {
                fd.n_header_line = 0;
                desc = fd.Dump();
            }
            // create a new client for each package
            // the client is not reused because one client cannot process
            // total data of several GBs
            // TODO: optimize, reuse the client each GB processed // NOLINT
            RestClient client(config_.url);
            if (config_.username.size()) {
                client.Login(config_.username, config_.password);
            }
            std::string log =
                client.SendImportData(config_.graph_name, desc, std::string(begin, end),
                                      config_.continue_on_error, config_.delimiter);
            WriteProcessedLinePackages(oplfs, packages_processed);
            if (log.size()) {
                FMA_WARN() << log;
            }
            bytes_sent += end - begin;
            double bytes_per_sec = bytes_sent * 1. / (time(nullptr) - start_time);
            FMA_LOG() << fma_common::StringFormatter::Format(
                "imported {} packages ({}, {}) at {}/s, approx. {} s remaining", packages_processed,
                FormatBytes((double)bytes_cut, 6),
                FormatPercent((double)bytes_cut * 1. / bytes_total), FormatBytes(bytes_per_sec),
                (size_t)((bytes_total - bytes_cut) / bytes_per_sec));
        }
    }
    iplfs.close();
    oplfs.close();
    remove(config_.progress_log_file.c_str());
    double t2 = fma_common::GetTime();
    FMA_LOG() << "Import finished in " << t2 - t1 << " seconds.";
}

void lgraph::import_v2::OnlineImportClient::SignalHandler(int signum) {
    exit_flag_ = true;
    FMA_WARN() << "signal received, exiting......";
}

// convert 0 <= p <= 1 to percentage "12.34%"
std::string lgraph::import_v2::OnlineImportClient::FormatPercent(double p) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << p * 100 << '%';
    return ss.str();
}

// convert n bytes to human readable string with nums digits
// FormatBytes(16385, 3) -> "16.0 KB"
std::string lgraph::import_v2::OnlineImportClient::FormatBytes(double n, int nums) {
    if (n < 0 || n > 1e20) return std::to_string(n) + " B";
    int i = 0;
    while (n >= 1024) {
        ++i;
        n /= 1024;
    }
    int precision = nums - (n < 10 ? 1 : (n < 100 ? 2 : 3));
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << n << ' ';
    if (i) ss << "_KMGTPEZYB"[i];
    ss << 'B';
    return ss.str();
}

size_t lgraph::import_v2::OnlineImportClient::ReadProcessedPackages(std::ifstream& is) {
    try {
        nlohmann::json conf;
        is >> conf;
        if (conf.contains("ProcessedPackages")) {
            return conf["ProcessedPackages"];
        }
    } catch (...) {
        return 0;
    }
    return 0;
}
void lgraph::import_v2::OnlineImportClient::WriteProcessedLinePackages(std::ofstream& os,
                                                                       size_t packages) {
    nlohmann::json conf;
    conf["ProcessedPackages"] = packages;
    os.seekp(std::ios_base::beg);
    os << conf;
    os.flush();
}
