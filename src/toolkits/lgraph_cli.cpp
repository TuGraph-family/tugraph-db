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

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/asio.hpp>
#include "bolt/hydrator.h"
#include "bolt/pack_stream.h"
#include "bolt/to_string.h"
#include "toolkits/linenoise/linenoise.h"
#include "tabulate/table.hpp"
#include <gflags/gflags.h>
using namespace boost;
using namespace boost::endian;
DEFINE_string(format, "table", "format");
DEFINE_string(ip, "127.0.0.1", "ip");
DEFINE_int32(port, 7687, "port");
DEFINE_string(graph, "default", "graph");
DEFINE_string(user, "admin", "user");
DEFINE_string(password, "73@TuGraph", "password");

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
        big_to_native_inplace(size);
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
        throw std::runtime_error(fmt::format(
            "Failed to parse bolt message, error: {}", ret.second.value()));
    }
    return ret.first;
}

bool FetchRecords(asio::ip::tcp::socket& socket, bolt::Hydrator& hydrator, OutputFormat of) {
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
                LOG_FATAL("mismatched data, header column size: {}, "
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
                    LOG_INFO(boost::algorithm::join(strs, ","));
                }
            } else {
                LOG_INFO(values.dump());
            }
        } else if (msg.type() == typeid(bolt::Success*)) {
            auto success = std::any_cast<bolt::Success*>(msg);
            if (!header) {
                header = success->fields;
                if (of == OutputFormat::TABLE) {
                    table.add_row({header.value().begin(), header.value().end()});
                } else if (of == OutputFormat::CSV) {
                    LOG_INFO(boost::algorithm::join(header.value(), ","));
                } else {
                    nlohmann::json j = header.value();
                    LOG_INFO(j.dump());
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
            error = fmt::format("Unexpected message: {}", msg.type().name());
            break;
        }
    }
    if (error.empty()) {
        if (of == OutputFormat::TABLE) {
            if (table.size() > 0) {
                if (table.row(0).size()) {
                    LOG_INFO("{}\n", table.str());
                }
                LOG_INFO("{} rows\n", table.size() - 1);
            } else {
                LOG_INFO("{} rows\n", table.size());
            }
        }
        return true;
    } else {
        LOG_ERROR("{}\n", error);
        return false;
    }
}

std::string ReadStatement() {
    bool is_multiline = false;
    std::string statement;
    while (true) {
        char* line = linenoise(is_multiline ? "      -> " : fmt::format("TuGraph[{}]> ", FLAGS_graph).c_str());
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
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    spdlog::set_pattern("%v");
    bool is_terminal = false;
    if (isatty(STDIN_FILENO)) {
        is_terminal = true;
    }

    OutputFormat of = OutputFormat::TABLE;
    if (FLAGS_format == "csv") {
        of = OutputFormat::CSV;
    } else if (FLAGS_format == "json") {
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
        asio::connect(socket, resolver.resolve(FLAGS_ip, std::to_string(FLAGS_port)));
        asio::write(socket, asio::const_buffer(handshake, sizeof(handshake)));
        uint8_t accepted_version[4];
        asio::read(socket, asio::buffer(accepted_version, sizeof(accepted_version)));
        if (accepted_version[2] != 4 || accepted_version[3] != 4) {
            LOG_ERROR("Unexpected accepted version");
            return -1;
        }
        meta = {{"scheme", "basic"}, {"principal", FLAGS_user}, {"credentials", FLAGS_password},
                {"patch_bolt", std::vector<std::string>{"utc"}}};
        ps.AppendHello(meta);
        asio::write(socket, asio::const_buffer(ps.ConstBuffer().data(), ps.ConstBuffer().size()));
        auto msg = ReadMessage(socket, hydrator);
        if (msg.type() == typeid(bolt::Success*)) {
            auto success = std::any_cast<bolt::Success*>(msg);
            auto iter = success->server.find("tugraph-db");
            if (iter == std::string::npos) {
                LOG_ERROR("The server is not tugraph-db");
                return -1;
            }
            if (success->patches.size() == 1 && success->patches[0] == "utc") {
                hydrator.UseUtc(true);
            }
        } else if (msg.type() == typeid(std::optional<bolt::Neo4jError>)) {
            const auto& error = std::any_cast<std::optional<bolt::Neo4jError>&>(msg);
            LOG_ERROR(error.value().msg);
            return -1;
        } else {
            LOG_ERROR("Unexpected message for authentication");
            return -1;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("{}:{} {}", FLAGS_ip, FLAGS_port, e.what());
        return -1;
    }
    if (is_terminal) {
        LOG_INFO("Welcome to the TuGraph console client. Commands end with ';'.");
        LOG_INFO("Copyright(C) 2018-2023 Ant Group. All rights reserved.");
        LOG_INFO("Type 'exit', 'quit' or Ctrl-C to exit.\n");
    }
    while (true) {
        std::string statement = ReadStatement();
        try {
            ps.Reset();
            meta.clear();
            meta = {{"db", FLAGS_graph}};
            ps.AppendRun(statement, {}, meta);
            ps.AppendPullN(-1);
            asio::write(socket,
                        asio::const_buffer(ps.ConstBuffer().data(), ps.ConstBuffer().size()));
            bool ret = FetchRecords(socket, hydrator, of);
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
                        LOG_ERROR("unexpected bolt msg after reset");
                        return -1;
                    }
                }
            }
            if (is_terminal && ret) {
                linenoiseHistoryAdd(statement.c_str());
                linenoiseHistorySave(history_file);
            }
        } catch (std::exception& e) {
            LOG_ERROR(e.what());
        }
    }
}
