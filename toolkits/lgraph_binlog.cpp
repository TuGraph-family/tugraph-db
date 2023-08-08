
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

#include <server/state_machine.h>
#include "fma-common/configuration.h"

#include "core/backup_log.h"
#include "core/data_type.h"
#include "protobuf/ha.pb.h"
#include "lgraph/lgraph_rpc_client.h"

void ProcessLogs(const std::vector<std::string>& files, int64_t beg_time, int64_t end_time,
                 int64_t limit, const std::unordered_set<int64_t>& skip_ids,
                 const std::function<bool(const lgraph::BackupLogEntry& l)>& sink,
                 const std::function<void()>& cleanup, bool print_stats = false) {
    int64_t n_processed = 0;
    double t1 = fma_common::GetTime();
    double tlast = t1;
    for (auto& fname : files) {
        fma_common::InputFmaStream in(fname, 4 << 20);
        if (!in.Good()) throw std::runtime_error("Failed to open file " + fname);
        lgraph::BackupLogEntry l;
        while (lgraph::BackupLog::ReadNextLogEntry(in, &l)) {
            if (l.time() < beg_time || l.time() >= end_time ||
                (!skip_ids.empty() && skip_ids.find(l.index()) != skip_ids.end())) {
                continue;
            }
            if (!sink(l)) {
                throw std::runtime_error("Failed to process log entry [" + l.DebugString() + "]");
            }
            if (++n_processed >= limit) break;
            if (print_stats && n_processed % 100 == 0) {
                double t2 = fma_common::GetTime();
                if (t2 - tlast >= 1) {
                    FMA_LOG() << "Processed " << n_processed << " log items at "
                              << (double)n_processed / (t2 - t1) << " tps";
                    tlast = t2;
                }
            }
        }
    }
    if (cleanup) cleanup();
    double t2 = fma_common::GetTime();
    if (print_stats) {
        FMA_LOG() << "Processed " << n_processed << " log items at "
                  << (double)n_processed / (t2 - t1) << " tps";
    }
}

std::string PrintLogEntry(const lgraph::BackupLogEntry& l) {
    return fma_common::StringFormatter::Format("index: {}\ntime: {}\nreq: \\{{}\\}", l.index(),
                                               lgraph::DateTime(l.time()).ToString(),
                                               l.req().DebugString());
}

int main(int argc, char** argv) {
    std::string files;  // files list, separated with comma, or `dir/*`
    std::string beg_time;
    std::string end_time;
    int64_t limit = -1;
    std::string skip_list;  // indexes to skip, separated with comma

    std::string action = "print";
    std::string host;
    uint16_t port = 7071;
    std::string dir;
    std::string user;
    std::string password;
    std::string output;

    try {
        fma_common::Configuration argparser;
        argparser.Add(files, "f,files", true)
            .Comment(
                "Binlog files as a list of strings separated with commas, or `dir/*` to use every "
                "file in a directory.");
        argparser.Add(beg_time, "begin_time", true)
            .Comment("Beginning time, a string in the format of `YYYY-MM-DD hh-mm-ss`.");
        argparser.Add(end_time, "end_time", true)
            .Comment("End time, a string in the format of `YYYY-MM-DD hh-mm-ss`.");
        argparser.Add(limit, "limit", true).Comment("Maximum number of log entries to print.");
        argparser.Add(skip_list, "skip_indexes", true)
            .Comment("Log indexes to skip, separated with commas.");
        argparser.Add(action, "a,action", true)
            .SetPossibleValues(std::vector<std::string>{"print", "restore"})
            .Comment(
                "Action to perform. `print` will print the log entries in human-readable format."
                " `restore` will apply the logs to a DB.");
        argparser.Add(host, "host", true)
            .Comment("Host address of server, if restoring to a remote server.");
        argparser.Add(port, "port", true)
            .Comment("RPC port of server, if restoring to a remote server.");
        argparser.Add(dir, "db_dir", true).Comment("Data diretory, if restoring to a local DB.");
        argparser.Add(user, "u,user", true).Comment("User name of DB, if restoring.");
        argparser.Add(password, "p,password", true)
            .Comment("Password of given user, if restoring.");
        argparser.Add(output, "o,output", true)
            .Comment(
                "Output file to print log to, if action==print. Empty value means printing to "
                "stdout.");
        argparser.ParseAndFinalize(argc, argv);

        // check validity of options
        // input files
        if (files.empty()) throw lgraph::InputError("Input file list cannot be empty.");
        std::vector<std::string> input_paths = fma_common::Split(files, ",");
        if (input_paths.size() == 1) {
            // could be dir/*
            if (input_paths[0].back() == '*') {
                std::string tmp_dir = fma_common::FilePath(input_paths[0]).Dir();
                input_paths = fma_common::file_system::ListFiles(tmp_dir, nullptr, true);
            }
        }
        std::vector<std::string> input_files = lgraph::BackupLog::SortLogFiles(input_paths);
        if (input_files.size() != input_paths.size()) {
            throw lgraph::InputError("Failed to sort input files. Did you renamed the files?");
        }
        // check that all files exist
        for (auto& f : input_paths) {
            if (!fma_common::file_system::FileExists(f))
                throw lgraph::InputError("File " + f + " does not exist.");
        }
        // check beg and end time
        int64_t min_time = std::numeric_limits<int64_t>::min();
        if (!beg_time.empty()) min_time = lgraph::DateTime(beg_time).MicroSecondsSinceEpoch();
        int64_t max_time = std::numeric_limits<int64_t>::max();
        if (!end_time.empty()) max_time = lgraph::DateTime(end_time).MicroSecondsSinceEpoch();
        if (limit <= 0) limit = std::numeric_limits<int64_t>::max();
        // check skip list
        std::unordered_set<int64_t> skip_ids;
        if (!skip_list.empty()) {
            auto strs = fma_common::Split(skip_list, ",");
            for (auto& s : strs) {
                int64_t id = -1;
                if (fma_common::TextParserUtils::ParseT(s, id) != s.size())
                    throw lgraph::InputError("Failed to parse skip id: " + std::to_string(id));
                skip_ids.insert(id);
            }
        }
        std::function<bool(const lgraph::BackupLogEntry& l)> log_sink;
        // take actions
        if (action == "restore") {
            if (host.empty() && dir.empty())
                throw lgraph::InputError(
                    "Remote address and local directory cannot be empty at the same time.");
            if (!host.empty() && !dir.empty())
                throw lgraph::InputError(
                    "Remote address and local directory cannot be specified at the same time.");
            if (user.empty() || password.empty())
                throw lgraph::InputError("User and password cannot be empty.");
            if (!host.empty()) {
                // writing to remote server
                lgraph::RpcClient client(host + ":" + std::to_string(port), user, password);
                size_t curr_packet_size = 0;
                std::vector<lgraph::BackupLogEntry> logs;
                log_sink = [&](const lgraph::BackupLogEntry& l) -> bool {
                    logs.push_back(l);
                    curr_packet_size += l.ByteSizeLong();
                    if (curr_packet_size >= 16 << 20) {
                        // send packet
                        client.Restore(logs);
                        logs.clear();
                        curr_packet_size = 0;
                    }
                    return true;
                };
                ProcessLogs(
                    input_files, min_time, max_time, limit, skip_ids, log_sink,
                    [&]() {
                        if (!logs.empty()) client.Restore(logs);
                    },
                    true);
            } else {
                // writing to local
                lgraph::StateMachine::Config config;
                config.db_dir = dir;
                config.durable = false;
                config.optimistic_txn = false;
                lgraph::StateMachine sm(config, nullptr);
                sm.Start();
                log_sink = [&](const lgraph::BackupLogEntry& l) -> bool {
                    try {
                        lgraph::LGraphResponse resp;
                        lgraph::LGraphRequest m_req;
                        m_req.CopyFrom(l.req());
                        m_req.release_user();
                        m_req.set_user(user);
                        sm.ApplyRequestDirectly(&m_req, &resp);
                        if (resp.error_code() !=
                            lgraph::LGraphResponse::ErrorCode::LGraphResponse_ErrorCode_SUCCESS) {
                            throw std::runtime_error(resp.error());
                        } else {
                            return true;
                        }
                    } catch (std::exception& e) {
                        std::throw_with_nested(
                            std::runtime_error("Error while applying request: " + l.DebugString()));
                    }
                };
                ProcessLogs(input_files, min_time, max_time, limit, skip_ids, log_sink, nullptr,
                            true);
                sm.Stop();
            }
        } else {
            FMA_CHECK_EQ(action, "print");
            if (output.empty()) {
                log_sink = [](const lgraph::BackupLogEntry& l) -> bool {
                    std::cout << PrintLogEntry(l) << "\n\n";
                    return true;
                };
                ProcessLogs(input_files, min_time, max_time, limit, skip_ids, log_sink, nullptr);
            } else {
                std::ofstream out(output);
                ProcessLogs(
                    input_files, min_time, max_time, limit, skip_ids,
                    [&](const lgraph::BackupLogEntry& l) -> bool {
                        out << PrintLogEntry(l) << "\n\n";
                        return true;
                    },
                    nullptr);
            }
        }
    } catch (std::exception& e) {
        FMA_LOG() << "An error occurred:\n" << lgraph::PrintNestedException(e, 1);
        return 1;
    }
#ifdef __SANITIZE_ADDRESS__
    // For address sanitizer: wait gc thread to release memory object
    fma_common::SleepS(2);
#endif
    return 0;
}
