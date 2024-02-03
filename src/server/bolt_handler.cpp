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

/*
 * written by botu.wzy
 */
#include "tools/lgraph_log.h"
#include "bolt/connection.h"
#include "server/bolt_server.h"
#include "server/bolt_session.h"
#include "db/galaxy.h"
namespace bolt {
extern boost::asio::io_service workers;
std::unordered_map<std::string, cypher::FieldData> ConvertParameters(
    const std::unordered_map<std::string, std::any>& parameters) {
    std::unordered_map<std::string, cypher::FieldData> ret;
    for (auto& pair : parameters) {
        if (pair.second.type() == typeid(std::string)) {
            ret.emplace("$" + pair.first, lgraph_api::FieldData::String(
                                              std::any_cast<const std::string&>(pair.second)));
        } else if (pair.second.type() == typeid(int64_t)) {
            ret.emplace("$" + pair.first, lgraph_api::FieldData::Int64(
                                              std::any_cast<int64_t>(pair.second)));
        } else if (pair.second.type() == typeid(double)) {
            ret.emplace("$" + pair.first, lgraph_api::FieldData::Double(
                                              std::any_cast<double>(pair.second)));
        } else if (pair.second.type() == typeid(bool)) {
            ret.emplace("$" + pair.first, lgraph_api::FieldData::Bool(
                                              std::any_cast<bool>(pair.second)));
        } else if (pair.second.type() == typeid(void)) {
            ret.emplace("$" + pair.first, lgraph_api::FieldData());
        } else {
            throw lgraph_api::InputError(FMA_FMT(
                "Unexpected cypher parameter type, parameter: {}", pair.first));
        }
    }
    return ret;
}

void BoltFSM(std::shared_ptr<BoltConnection> conn) {
    pthread_setname_np(pthread_self(), "bolt_fsm");
    auto conn_id = conn->conn_id();
    LOG_DEBUG() << FMA_FMT("bolt fsm thread[conn_id:{}] start.", conn_id);
    auto session = (BoltSession*)conn->GetContext();
    while (!conn->has_closed()) {
        auto msg = session->msgs.Pop(std::chrono::milliseconds(100));
        if (!msg) {  // msgs pop timeout
            continue;
        }
        const auto& fields = msg.value().fields;
        auto type = msg.value().type;
        if (session->state == SessionState::FAILED) {
            if (type == bolt::BoltMsg::Run ||
                type == bolt::BoltMsg::PullN ||
                type == bolt::BoltMsg::DiscardN) {
                bolt::PackStream ps;
                ps.AppendIgnored();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                continue;
            } else if (type == bolt::BoltMsg::Reset) {
                bolt::PackStream ps;
                ps.AppendSuccess();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                session->state = SessionState::READY;
            } else {
                LOG_ERROR() << FMA_FMT("Unexpected msg:{} in FAILED state, "
                    "close the connection", ToString(type));
                conn->Close();
                return;
            }
        } else if (session->state == SessionState::INTERRUPTED) {
            if (type == bolt::BoltMsg::Run ||
                type == bolt::BoltMsg::PullN ||
                type == bolt::BoltMsg::DiscardN ||
                type == bolt::BoltMsg::Begin ||
                type == bolt::BoltMsg::Commit ||
                type == bolt::BoltMsg::Rollback) {
                bolt::PackStream ps;
                ps.AppendIgnored();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                continue;
            } else if (type == bolt::BoltMsg::Reset) {
                bolt::PackStream ps;
                ps.AppendSuccess();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                session->state = SessionState::READY;
                continue;
            } else {
                LOG_ERROR() << FMA_FMT("Unexpected msg:{} in INTERRUPTED state, "
                    "close the connection", ToString(type));
                conn->Close();
                return;
            }
        } else if (session->state == SessionState::READY) {
            if (type == bolt::BoltMsg::Begin) {
                std::string err = FMA_FMT("Receive {}, but explicit transactions are "
                    "not currently supported.", ToString(type));
                LOG_ERROR() << err;
                bolt::PackStream ps;
                ps.AppendFailure({{"code", "error"},
                                  {"message", err}});
                conn->PostResponse(std::move(ps.MutableBuffer()));
                session->state = SessionState::FAILED;
                continue;
            } else if (type == bolt::BoltMsg::Reset) {
                bolt::PackStream ps;
                ps.AppendSuccess();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                session->state = SessionState::READY;
                continue;
            } else if (type == bolt::BoltMsg::Run) {
                try {
                    if (fields.size() < 3) {
                        throw lgraph_api::InputError(FMA_FMT(
                            "Run msg fields size error, size: {}", fields.size()));
                    }
                    auto& cypher = std::any_cast<const std::string&>(fields[0]);
                    auto& extra = std::any_cast<
                        const std::unordered_map<std::string, std::any>&>(fields[2]);
                    auto db_iter = extra.find("db");
                    if (db_iter == extra.end()) {
                        throw lgraph_api::InputError(
                            "Missing 'db' item in the 'extra' info of 'Run' msg");
                    }
                    auto& graph = std::any_cast<const std::string&>(db_iter->second);
                    auto& field1 = std::any_cast<
                        const std::unordered_map<std::string, std::any>&>(fields[1]);
                    auto parameter = ConvertParameters(field1);
                    auto sm = BoltServer::Instance().StateMachine();
                    cypher::RTContext ctx(sm, sm->GetGalaxy(), session->user, graph);
                    ctx.SetBoltConnection(conn.get());
                    ctx.param_tab_ = std::move(parameter);
                    session->streaming_msg.reset();
                    cypher::ElapsedTime elapsed;
                    LOG_DEBUG() << "Bolt run " << cypher;
                    sm->GetCypherScheduler()->Eval(&ctx, lgraph_api::GraphQueryType::CYPHER,
                                                   cypher, elapsed);
                    LOG_DEBUG() << "Cypher execution completed";
                } catch (std::exception& e) {
                    LOG_ERROR() << e.what();
                    bolt::PackStream ps;
                    ps.AppendFailure({{"code", "error"},
                                      {"message", e.what()}});
                    conn->PostResponse(std::move(ps.MutableBuffer()));
                    session->state = SessionState::FAILED;
                }
            }
        }
    }
    LOG_DEBUG() << FMA_FMT("bolt fsm thread[conn_id:{}] exit.", conn_id);
}

std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg,
                   std::vector<std::any> fields)> BoltHandler =
[](BoltConnection& conn, BoltMsg msg, std::vector<std::any> fields) {
    static bolt::PackStream ps;
    if (msg == BoltMsg::Hello) {
        ps.Reset();
        if (fields.size() != 1) {
            LOG_ERROR() << "Hello msg fields size error, size: " << fields.size();
            ps.AppendFailure({{"code", "error"},
                              {"message", "Hello msg fields size error"}});
            conn.Respond(std::move(ps.MutableBuffer()));
            conn.Close();
            return;
        }
        auto& val = std::any_cast<const std::unordered_map<std::string, std::any>&>(fields[0]);
        auto& principal = std::any_cast<const std::string&>(val.at("principal"));
        auto& credentials = std::any_cast<const std::string&>(val.at("credentials"));
        auto galaxy = BoltServer::Instance().StateMachine()->GetGalaxy();
        if (!galaxy->ValidateUser(principal, credentials)) {
            LOG_ERROR() << "Bolt authentication failed";
            ps.AppendFailure({{"code", "error"},
                              {"message", "Authentication failed"}});
            conn.Respond(std::move(ps.MutableBuffer()));
            conn.Close();
            return;
        }
        std::unordered_map<std::string, std::any> meta;
        meta["connection_id"] = std::string("bolt") + std::to_string(conn.conn_id());
        // Neo4j python client check that the returned server info must start with 'Neo4j/'
        meta["server"] = "Neo4j/tugraph-db";
        auto session = std::make_shared<BoltSession>();
        session->state = SessionState::READY;
        session->user = principal;
        session->fsm_thread = std::thread(BoltFSM, conn.shared_from_this());
        session->fsm_thread.detach();
        conn.SetContext(std::move(session));
        ps.AppendSuccess(meta);
        conn.Respond(std::move(ps.MutableBuffer()));
    } else if (msg == BoltMsg::Run ||
               msg == BoltMsg::PullN ||
               msg == BoltMsg::DiscardN ||
               msg == BoltMsg::Begin ||
               msg == BoltMsg::Commit ||
               msg == BoltMsg::Rollback) {
        auto session = (BoltSession*)conn.GetContext();
        session->msgs.Push({msg, std::move(fields)});
    } else if (msg == BoltMsg::Reset) {
        auto session = (BoltSession*)conn.GetContext();
        session->state = SessionState::INTERRUPTED;
        session->msgs.Push({BoltMsg::Reset, std::move(fields)});
    } else if (msg == BoltMsg::Goodbye) {
        conn.Close();
    } else {
        LOG_WARN() << "receive unknown bolt message: " << ToString(msg);
        conn.Close();
    }
};
}  // namespace bolt
