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

/*
 * written by botu.wzy
 */

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/asio.hpp>
#include "fma-common/configuration.h"
#include "tools/lgraph_log.h"
#include "bolt/hydrator.h"
#include "bolt/pack_stream.h"
#include "bolt/to_string.h"
#include "linenoise/linenoise.h"
#include "tabulate/table.hpp"
using namespace boost;

enum class OutputFormat {
    TABLE = 0,
    CSV,
    JSON
};

std::any ReadMessage(asio::ip::tcp::socket& socket, bolt::Hydrator& hydrator) {
    hydrator.ClearErr();
    uint16_t size = 0;
    std::vector<uint8_t> buffer;
    while (true) {
        asio::read(socket, asio::buffer((void*)(&size), sizeof(size)));
        boost::endian::big_to_native_inplace(size);
        if (size == 0) {
            if (!buffer.empty()) {
                break;
            }
            continue;
        }
        auto old_size = buffer.size();
        buffer.resize(old_size + size);
        asio::read(socket, asio::buffer((void*)(buffer.data() + old_size), size));
    }
    auto ret = hydrator.Hydrate({(const char*)buffer.data(), buffer.size()});
    if (ret.second) {
        throw std::runtime_error(FMA_FMT(
            "Failed to parse bolt message, error: {}", ret.second.value()));
    }
    return ret.first;
}

bool FetchRecords(asio::ip::tcp::socket& socket, bolt::Hydrator& hydrator,
                  OutputFormat of, bool print_time) {
    auto start = std::chrono::high_resolution_clock::now();
    std::string error;
    std::optional<std::vector<std::string>> header;
    tabulate::Table table;
    table.format().trim_mode(tabulate::Format::TrimMode::kNone);
    while (true) {
        auto msg = ReadMessage(socket, hydrator);
        if (msg.type() == typeid(std::optional<bolt::Record>)) {
            const auto& val = std::any_cast<const std::optional<bolt::Record>&>(msg);
            nlohmann::json values = nlohmann::json::array();
            for (auto& item : val.value().values) {
                values.push_back(bolt::ToJson(item));
            }
            if (values.size() != header.value().size()) {
                LOG_FATAL() << FMA_FMT("mismatched data, header column size: {}, "
                    "record column size: {}", header.value().size(), values.size());
            }
            if (of != OutputFormat::JSON) {
                std::vector<std::string> strs;
                for (const auto& v : values) {
                    if (v.is_string()) {
                        strs.push_back(v.get<std::string>());
                    } else {
                        strs.push_back(v.dump());
                    }
                }
                if (of == OutputFormat::TABLE) {
                    table.add_row({strs.begin(), strs.end()});
                } else {
                    LOG_INFO() << boost::algorithm::join(strs, ",");
                }
            } else {
                LOG_INFO() << values.dump();
            }
        } else if (msg.type() == typeid(bolt::Success*)) {
            auto success = std::any_cast<bolt::Success*>(msg);
            if (!header) {
                header = success->fields;
                if (of == OutputFormat::TABLE) {
                    table.add_row({header.value().begin(), header.value().end()});
                } else if (of == OutputFormat::CSV) {
                    LOG_INFO() << boost::algorithm::join(header.value(), ",");
                } else {
                    nlohmann::json j = header.value();
                    LOG_INFO() << j.dump();
                }
            } else {
                break;
            }
        } else if (msg.type() == typeid(std::optional<bolt::Neo4jError>)) {
            const auto& ne = std::any_cast<std::optional<bolt::Neo4jError>&>(msg);
            error = ne.value().msg;
            break;
        } else if (msg.type() == typeid(bolt::Ignored*)) {
            continue;
        } else {
            error = FMA_FMT("Unexpected message: {}", msg.type().name());
            break;
        }
    }

    if (error.empty()) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        if (of == OutputFormat::TABLE) {
            if (table.size() > 0) {
                LOG_INFO() << table << "\n";
                if (print_time) {
                    LOG_INFO() << FMA_FMT("{} rows ({} ms)", table.size() - 1,
                                          static_cast<int>(elapsed.count())) << "\n";
                } else {
                    LOG_INFO() << FMA_FMT("{} rows", table.size() - 1) << "\n";
                }
            } else {
                if (print_time) {
                    LOG_INFO() << FMA_FMT("{} rows ({} ms)", table.size(),
                                          static_cast<int>(elapsed.count())) << "\n";
                } else {
                    LOG_INFO() << FMA_FMT("{} rows", table.size()) << "\n";
                }
            }
        }
        return true;
    } else {
        LOG_ERROR() << error << "\n";
        return false;
    }
}

std::string ReadStatement() {
    bool is_multiline = false;
    std::string statement;
    while (true) {
        char* line = linenoise(is_multiline ? "      -> " : "TuGraph> ");
        if (line == nullptr) {
            std::exit(0);
        }
        std::unique_ptr<char[], void(*)(char*)> auto_free_line(line, [](char* p) {
            free(p);
        });
        if (line[0] == '\0') {
            continue;
        }
        if (!is_multiline) {
            if (strcasecmp(line, "quit") == 0 || strcasecmp(line, "exit") == 0) {
                std::exit(0);
            }
        }
        statement.append(line);
        if (statement.back() == ';') {
            break;
        } else {
            is_multiline = true;
            statement.append(" ");
        }
    }
    return statement;
}

void completion(const char *buf, linenoiseCompletions *lc) {
    // TODO(botu.wzy)
}

char* hints(const char* buf, int* color, int* bold) {
    // TODO(botu.wzy)
    return nullptr;
}

int main(int argc, char** argv) {
    fma_common::Configuration config;
    std::string format = "table";
    std::string ip = "127.0.0.1";
    int port = 7687;
    std::string graph = "default";
    std::string username = "admin";
    std::string password = "73@TuGraph";
    bool print_time = true;
    config.Add(format, "format", true).
        Comment("output format (table, csv, json)").
        SetPossibleValues({"table", "csv", "json"});
    config.Add(ip, "ip", true).Comment("TuGraph bolt protocol ip");
    config.Add(port, "port", true).Comment("TuGraph bolt protocol port");
    config.Add(graph, "graph", true).Comment("Graph to use");
    config.Add(username, "user", true).Comment("User to login");
    config.Add(password, "password", true).Comment("Password to use when connecting to server");
    config.Add(print_time, "print_time", true).Comment(
        "Whether to print the execution time of the query");
    try {
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        LOG_ERROR() << e.what();
        return -1;
    }

    bool is_terminal = false;
    if (isatty(STDIN_FILENO)) {
        is_terminal = true;
    }

    OutputFormat of = OutputFormat::TABLE;
    if (format == "csv") {
        of = OutputFormat::CSV;
    } else if (format == "json") {
        of = OutputFormat::JSON;
    }

    const char* history_file = ".lgraphcli_history";
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);
    linenoiseSetMultiLine(1);
    if (is_terminal) {
        linenoiseHistoryLoad(history_file);
    }

    bolt::MarkersInit();
    uint8_t handshake[] = {
        0x60, 0x60, 0xb0, 0x17,
        0x00, 0x00, 0x04, 0x04,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };

    asio::io_context io_context;
    asio::ip::tcp::socket socket(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    bolt::PackStream ps;
    std::unordered_map<std::string, std::any> meta;
    bolt::Hydrator hydrator;
    try {
        asio::connect(socket, resolver.resolve(ip, std::to_string(port)));
        asio::write(socket, asio::const_buffer(handshake, sizeof(handshake)));
        uint8_t accepted_version[4];
        asio::read(socket, asio::buffer(accepted_version, sizeof(accepted_version)));
        if (accepted_version[2] != 4 || accepted_version[3] != 4) {
            LOG_ERROR() << "Unexpected accepted version";
            return -1;
        }
        meta = {{"scheme", "basic"}, {"principal", username}, {"credentials", password}};
        ps.AppendHello(meta);
        asio::write(socket, asio::const_buffer(ps.ConstBuffer().data(), ps.ConstBuffer().size()));
        auto msg = ReadMessage(socket, hydrator);
        if (msg.type() == typeid(bolt::Success*)) {
            auto success = std::any_cast<bolt::Success*>(msg);
            auto iter = success->server.find("tugraph-db");
            if (iter == std::string::npos) {
                LOG_ERROR() << "The server is not tugraph-db";
                return -1;
            }
        } else if (msg.type() == typeid(std::optional<bolt::Neo4jError>)) {
            const auto& error = std::any_cast<std::optional<bolt::Neo4jError>&>(msg);
            LOG_ERROR() << error.value().msg;
            return -1;
        } else {
            LOG_ERROR() << "Unexpected message for authentication";
            return -1;
        }
    } catch (const std::exception& e) {
        LOG_ERROR() << FMA_FMT("{}:{} ", ip, port) << e.what();
        return -1;
    }
    if (is_terminal) {
        LOG_INFO() << "Welcome to the TuGraph console client. Commands end with ';'.";
        LOG_INFO() << "Copyright(C) 2018-2023 Ant Group. All rights reserved.";
        LOG_INFO() << "Type 'exit', 'quit' or Ctrl-C to exit.\n";
    }
    while (true) {
        std::string statement = ReadStatement();
        try {
            ps.Reset();
            meta.clear();
            meta = {{"db", graph}};
            ps.AppendRun(statement, {}, meta);
            ps.AppendPullN(-1);
            asio::write(socket,
                        asio::const_buffer(ps.ConstBuffer().data(), ps.ConstBuffer().size()));
            bool ret = FetchRecords(socket, hydrator, of, print_time);
            if (!ret) {
                // reset connection
                ps.Reset();
                ps.AppendReset();
                asio::write(socket, asio::const_buffer(
                                        ps.ConstBuffer().data(), ps.ConstBuffer().size()));
                while (true) {
                    auto m = ReadMessage(socket, hydrator);
                    if (m.type() == typeid(bolt::Success*)) {
                        break;
                    } else if (m.type() == typeid(bolt::Ignored*)) {
                        continue;
                    } else {
                        LOG_ERROR() << "unexpected bolt msg after reset";
                        return -1;
                    }
                }
            }
            if (is_terminal) {
                linenoiseHistoryAdd(statement.c_str());
                linenoiseHistorySave(history_file);
            }
        } catch (std::exception& e) {
            LOG_ERROR() << e.what();
        }
    }
}
