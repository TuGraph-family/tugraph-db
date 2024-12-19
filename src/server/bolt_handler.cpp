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

//
// Created by botu.wzy
//

#include <spdlog/fmt/fmt.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/chrono.h>
#include "common/logger.h"

#include <boost/algorithm/string.hpp>

#include "bolt/bolt_server.h"
#include "bolt/connection.h"
#include "common/exceptions.h"
#include "cypher/execution_plan/result_iterator.h"
#include "cypher/execution_plan/runtime_context.h"
#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/common/ObjectAllocator.h"
#include "server/bolt_session.h"

using namespace bolt;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
DECLARE_bool(enable_query_log);
namespace server {

geax::frontend::Expr* ConvertParameters(geax::common::ObjectArenaAllocator &obj_alloc_,
                                        std::any data) {
    geax::frontend::Expr* ret;
    if (data.type() == typeid(std::string)) {
        ret = obj_alloc_.allocate<geax::frontend::VString>();
        auto& str = std::any_cast<std::string&>(data);
        ((geax::frontend::VString*)ret)->setVal(std::move(str));
    } else if (data.type() == typeid(int64_t)) {
        ret = obj_alloc_.allocate<geax::frontend::VInt>();
        ((geax::frontend::VInt*)ret)->setVal(std::any_cast<int64_t>(data));
    } else if (data.type() == typeid(double)) {
        ret = obj_alloc_.allocate<geax::frontend::VDouble>();
        ((geax::frontend::VDouble*)ret)->setVal(std::any_cast<double>(data));
    } else if (data.type() == typeid(bool)) {
        ret = obj_alloc_.allocate<geax::frontend::VBool>();
        ((geax::frontend::VBool*)ret)->setVal(std::any_cast<bool>(data));
    } else if (data.type() == typeid(void)) {
        ret = obj_alloc_.allocate<geax::frontend::VNull>();
    } else if (data.type() == typeid(std::unordered_map<std::string, std::any>)) {
        ret = obj_alloc_.allocate<geax::frontend::MkMap>();
        auto& map =  std::any_cast<std::unordered_map<std::string, std::any>&>(data);
        for (auto& pair : map) {
            auto key = obj_alloc_.allocate<geax::frontend::VString>();
            std::string key_val = pair.first;
            key->setVal(std::move(key_val));
            ((geax::frontend::MkMap*)ret)->appendElem(key, ConvertParameters(
                                                               obj_alloc_, std::move(pair.second)));
        }
    } else if (data.type() == typeid(std::vector<std::any>)) {
        ret = obj_alloc_.allocate<geax::frontend::MkList>();
        auto& list =  std::any_cast<std::vector<std::any>&>(data);
        for (auto& item : list) {
            ((geax::frontend::MkList*)ret)->appendElem(ConvertParameters(
                obj_alloc_, std::move(item)));
        }
    } else {
        THROW_CODE(InputError,
                   "Unexpected cypher parameter type : {}", data.type().name());
    }
    return ret;
}

static bool SendRecord(BoltConnection* conn, BoltSession* session, const std::vector<std::any>& record) {
    if (conn->has_closed()) {
        LOG_INFO("The bolt connection is closed, cancel the op execution.");
        return false;
    }
    while (session->state == bolt::SessionState::STREAMING && !session->streaming_msg) {
        session->streaming_msg = session->msgs.Pop(std::chrono::milliseconds(100));
        if (conn->has_closed()) {
            LOG_INFO("The bolt connection is closed, cancel the op execution.");
            return false;
        }
        if (!session->streaming_msg) {
            continue;
        }
        if (session->streaming_msg.value().type == bolt::BoltMsg::PullN ||
            session->streaming_msg.value().type == bolt::BoltMsg::DiscardN) {
            const auto &fields = session->streaming_msg.value().fields;
            if (fields.size() != 1) {
                std::string err =
                    fmt::format("{} msg fields size error, size: {}",
                            bolt::ToString(session->streaming_msg.value().type).c_str(),
                            fields.size());
                LOG_ERROR(err);
                bolt::PackStream ps;
                ps.AppendFailure({{"code", "error"}, {"message", err}});
                conn->PostResponse(std::move(ps.MutableBuffer()));
                session->state = bolt::SessionState::FAILED;
                return false;
            }
            auto &val =
                std::any_cast<const std::unordered_map<std::string, std::any> &>(fields[0]);
            auto n = std::any_cast<int64_t>(val.at("n"));
            session->streaming_msg.value().n = n;
        } else if (session->streaming_msg.value().type == bolt::BoltMsg::Reset) {
            LOG_INFO("Receive RESET, cancel the op execution.");
            bolt::PackStream ps;
            ps.AppendSuccess();
            conn->PostResponse(std::move(ps.MutableBuffer()));
            session->state = bolt::SessionState::READY;
            return false;
        } else {
            LOG_ERROR(
                "Unexpected msg:{} in STREAMING state, cancel the op execution, "
                "close the connection.",
                bolt::ToString(session->streaming_msg.value().type));
            conn->Close();
            return false;
        }
        break;
    }
    if (session->state == bolt::SessionState::INTERRUPTED) {
        LOG_WARN("The session state is INTERRUPTED, cancel the op execution.");
        return false;
    } else if (session->state != bolt::SessionState::STREAMING) {
        LOG_ERROR("Unexpected state: {} in op execution, close the connection.");
        conn->Close();
        return false;
    } else if (session->streaming_msg.value().type != bolt::BoltMsg::PullN &&
               session->streaming_msg.value().type != bolt::BoltMsg::DiscardN) {
        LOG_ERROR("Unexpected msg: {} in op execution, "
            "cancel the op execution, close the connection.",
            bolt::ToString(session->streaming_msg.value().type));
        conn->Close();
        return false;
    }
    if (session->streaming_msg.value().type == bolt::BoltMsg::PullN) {
        session->ps.AppendRecord(record);
        bool sync = false;
        if (--session->streaming_msg.value().n == 0) {
            std::unordered_map<std::string, std::any> meta;
            meta["has_more"] = true;
            session->ps.AppendSuccess(meta);
            session->state = bolt::SessionState::STREAMING;
            session->streaming_msg.reset();
            sync = true;
        }
        if (sync || session->ps.ConstBuffer().size() > 1024) {
            conn->PostResponse(std::move(session->ps.MutableBuffer()));
            session->ps.Reset();
        }
    } else if (session->streaming_msg.value().type == bolt::BoltMsg::DiscardN) {
        if (--session->streaming_msg.value().n == 0) {
            std::unordered_map<std::string, std::any> meta;
            meta["has_more"] = true;
            session->ps.AppendSuccess(meta);
            session->state = bolt::SessionState::STREAMING;
            session->streaming_msg.reset();
            conn->PostResponse(std::move(session->ps.MutableBuffer()));
            session->ps.Reset();
        }
    }
    return true;
}

void BoltFSM(std::shared_ptr<BoltConnection> conn) {
    pthread_setname_np(pthread_self(), "bolt_fsm");
    auto conn_id = conn->conn_id();
    LOG_DEBUG("bolt fsm thread[conn_id:{}] start.", conn_id);
    auto session = (BoltSession*)conn->GetContext();
    auto RespondFailure = [&conn, &session](ErrorCode code, const std::string& msg){
        bolt::PackStream ps;
        ps.AppendFailure(
            {{"code", ErrorCodeToString(code)},
             {"message", msg}});
        conn->PostResponse(std::move(ps.MutableBuffer()));
        session->state = SessionState::FAILED;
    };
    while (!conn->has_closed()) {
        auto msg = session->msgs.Pop(std::chrono::milliseconds(50));
        if (!msg) {  // msgs pop timeout
            continue;
        }
        auto& fields = msg.value().fields;
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
                LOG_ERROR("Unexpected msg:{} in FAILED state, "
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
                LOG_ERROR("Unexpected msg:{} in INTERRUPTED state, "
                    "close the connection", ToString(type));
                conn->Close();
                return;
            }
        } else if (session->state == SessionState::READY) {
            if (type == bolt::BoltMsg::Begin) {
                std::string err = fmt::format("Receive {}, but explicit transactions are "
                    "not currently supported.", ToString(type));
                LOG_ERROR(err);
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
                        THROW_CODE(InputError,
                            "Run msg fields size error, size: {}", fields.size());
                    }
                    spdlog::stopwatch sw;
                    auto& cypher = std::any_cast<const std::string&>(fields[0]);
                    auto& extra = std::any_cast<
                        const std::unordered_map<std::string, std::any>&>(fields[2]);
                    std::string graph;
                    auto db_iter = extra.find("db");
                    if (db_iter != extra.end()) {
                        graph = std::any_cast<const std::string&>(db_iter->second);
                    }
                    auto& field1 = std::any_cast<
                        std::unordered_map<std::string, std::any>&>(fields[1]);
                    cypher::RTContext ctx(g_galaxy.get(), session->user, graph);
                    for (auto& pair : field1) {
                        ctx.bolt_parameters_.emplace("$" + pair.first,
                            ConvertParameters(ctx.obj_alloc_, std::move(pair.second)));
                    }
                    session->streaming_msg.reset();
                    auto graph_db = g_galaxy->OpenGraph(graph);
                    auto txn = graph_db->BeginTransaction();
                    txn->SetConn(conn);
                    LOG_DEBUG("Execute {}", cypher.substr(0, 256));
                    auto res_iter = txn->Execute(&ctx, cypher);
                    auto header = res_iter->GetHeader();

                    std::unordered_map<std::string, std::any> meta;
                    meta["fields"] = header;
                    bolt::PackStream ps;
                    ps.AppendSuccess(meta);
                    conn->PostResponse(std::move(ps.MutableBuffer()));
                    session->state = bolt::SessionState::STREAMING;
                    bool success = true;
                    for (; res_iter->Valid(); res_iter->Next()) {
                        auto record = res_iter->GetBoltRecord();
                        if (!SendRecord(conn.get(), session, record)) {
                            success = false;
                            break;
                        }
                    }
                    res_iter.reset();
                    txn->Commit();
                    if (success) {
                        session->ps.AppendSuccess();
                        session->state = bolt::SessionState::READY;
                        conn->PostResponse(std::move(session->ps.MutableBuffer()));
                        session->ps.Reset();
                    }
                    LOG_DEBUG("Cypher execution completed");
                    QUERY_LOG("{} {} {}", graph, duration_cast<milliseconds>(sw.elapsed()), cypher.substr(0, 256));
                } catch (const LgraphException& e) {
                    LOG_ERROR(e.what());
                    RespondFailure(e.code(), e.msg());
                } catch (std::exception& e) {
                    LOG_ERROR(e.what());
                    RespondFailure(ErrorCode::UnknownError, e.what());
                }
            }
        }
    }
    LOG_DEBUG("bolt fsm thread[conn_id:{}] exit.", conn_id);
}

std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg,
                   std::vector<std::any> fields)> g_bolt_handler =
[](BoltConnection& conn, BoltMsg msg, std::vector<std::any> fields) {
    if (msg == BoltMsg::Hello) {
        if (fields.size() != 1) {
            LOG_ERROR("Hello msg fields size error, size: {}", fields.size());
            bolt::PackStream ps;
            ps.AppendFailure({{"code", "error"},
                              {"message", "Hello msg fields size error"}});
            conn.Respond(std::move(ps.MutableBuffer()));
            conn.Close();
            return;
        }
        auto& val = std::any_cast<const std::unordered_map<std::string, std::any>&>(fields[0]);
        if (!val.count("principal") || !val.count("credentials")) {
            std::string err = "Miss 'principal' or 'credentials' in Hello msg";
            LOG_ERROR(err);
            bolt::PackStream ps;
            ps.AppendFailure({{"code", "error"},
                              {"message", err}});
            conn.Respond(std::move(ps.MutableBuffer()));
            conn.Close();
            return;
        }
        auto& principal = std::any_cast<const std::string&>(val.at("principal"));
        auto& credentials = std::any_cast<const std::string&>(val.at("credentials"));
        /*auto galaxy = BoltServer::Instance().StateMachine()->GetGalaxy();
        if (!galaxy->ValidateUser(principal, credentials)) {
            LOG_ERROR("Bolt authentication failed");
            bolt::PackStream ps;
            ps.AppendFailure({{"code", "error"},
                              {"message", "Authentication failed"}});
            conn.Respond(std::move(ps.MutableBuffer()));
            conn.Close();
            return;
        }*/
        std::unordered_map<std::string, std::any> meta;
        meta["connection_id"] = std::string("bolt") + std::to_string(conn.conn_id());
        // Neo4j python client check that the returned server info must start with 'Neo4j/'
        meta["server"] = "Neo4j/tugraph-db";
        auto session = std::make_shared<BoltSession>();
        if (val.count("user_agent")) {
            auto& user_agent = std::any_cast<const std::string&>(val.at("user_agent"));
            if (boost::algorithm::starts_with(user_agent, "neo4j-python")) {
                session->python_driver = true;
            }
        }
        if (val.count("patch_bolt")) {
            auto& patch = std::any_cast<const std::vector<std::any>&>(val.at("patch_bolt"));
            if (patch.size() == 1) {
                auto item = std::any_cast<std::string>(patch[0]);
                if (item == "utc") {
                    session->utc_patch = true;
                    meta["patch_bolt"] = std::vector<std::string>{"utc"};
                }
            }
        }
        session->state = SessionState::READY;
        session->user = principal;
        conn.SetContext(session);
        session->fsm_thread = std::thread(BoltFSM, conn.shared_from_this());
        session->fsm_thread.detach();
        bolt::PackStream ps;
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
        LOG_WARN("receive unknown bolt message: {}", ToString(msg));
        conn.Close();
    }
};
}  // namespace bolt
