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
#include "tools/lgraph_log.h"
#include "bolt/connection.h"
#include "server/bolt_server.h"
#include "server/bolt_session.h"
#include "db/galaxy.h"
using namespace lgraph_api;
namespace bolt {
extern boost::asio::io_service workers;

parser::Expression ConvertParameters(std::any data) {
    parser::Expression ret;
    if (data.type() == typeid(std::string)) {
        ret.type = parser::Expression::STRING;
        auto& str = std::any_cast<std::string&>(data);
        ret.data = std::make_shared<std::string>(std::move(str));
    } else if (data.type() == typeid(int64_t)) {
        ret.type = parser::Expression::INT;
        ret.data = std::any_cast<int64_t>(data);
    } else if (data.type() == typeid(double)) {
        ret.type = parser::Expression::DOUBLE;
        ret.data = std::any_cast<double>(data);
    } else if (data.type() == typeid(bool)) {
        ret.type = parser::Expression::BOOL;
        ret.data = std::any_cast<bool>(data);
    } else if (data.type() == typeid(void)) {
        ret.type = parser::Expression::NULL_;
    } else if (data.type() == typeid(std::unordered_map<std::string, std::any>)) {
        ret.type = parser::Expression::MAP;
        std::map<std::string, parser::Expression> map_exp;
        auto& map =  std::any_cast<std::unordered_map<std::string, std::any>&>(data);
        for (auto& pair : map) {
            map_exp.emplace(pair.first, ConvertParameters(std::move(pair.second)));
        }
        ret.data = std::make_shared<std::map<
            std::string, parser::Expression>>(std::move(map_exp));
    } else if (data.type() == typeid(std::vector<std::any>)) {
        ret.type = parser::Expression::LIST;
        std::vector<parser::Expression> list_exp;
        auto& list =  std::any_cast<std::vector<std::any>&>(data);
        for (auto& item : list) {
            list_exp.emplace_back(ConvertParameters(std::move(item)));
        }
        ret.data = std::make_shared<std::vector<parser::Expression>>(std::move(list_exp));
    } else {
        THROW_CODE(InputError,
                   "Unexpected cypher parameter type : {}", data.type().name());
    }
    return ret;
}

void BoltFSM(std::shared_ptr<BoltConnection> conn) {
    pthread_setname_np(pthread_self(), "bolt_fsm");
    auto conn_id = conn->conn_id();
    LOG_DEBUG() << FMA_FMT("bolt fsm thread[conn_id:{}] start.", conn_id);
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
        auto msg = session->msgs.Pop(std::chrono::milliseconds(100));
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
                        THROW_CODE(InputError,
                            "Run msg fields size error, size: {}", fields.size());
                    }
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
                    auto sm = BoltServer::Instance().StateMachine();
                    cypher::RTContext ctx(sm, sm->GetGalaxy(), session->user, graph);
                    ctx.SetBoltConnection(conn.get());
                    ctx.bolt_parameters_ = std::make_shared<std::unordered_map<
                        std::string, parser::Expression>>();
                    for (auto& pair : field1) {
                        ctx.bolt_parameters_->emplace("$" + pair.first,
                                                      ConvertParameters(std::move(pair.second)));
                    }
                    session->streaming_msg.reset();
                    cypher::ElapsedTime elapsed;
                    LOG_DEBUG() << "Bolt run " << cypher;
                    sm->GetCypherScheduler()->Eval(&ctx, lgraph_api::GraphQueryType::CYPHER,
                                                   cypher, elapsed);
                    LOG_DEBUG() << "Cypher execution completed";
                } catch (const lgraph_api::LgraphException& e) {
                    LOG_ERROR() << e.what();
                    RespondFailure(e.code(), e.msg());
                } catch (std::exception& e) {
                    LOG_ERROR() << e.what();
                    RespondFailure(ErrorCode::UnknownError, e.what());
                }
            }
        }
    }
    LOG_DEBUG() << FMA_FMT("bolt fsm thread[conn_id:{}] exit.", conn_id);
}

std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg,
                   std::vector<std::any> fields)> BoltHandler =
[](BoltConnection& conn, BoltMsg msg, std::vector<std::any> fields) {
    if (msg == BoltMsg::Hello) {
        if (fields.size() != 1) {
            LOG_ERROR() << "Hello msg fields size error, size: " << fields.size();
            bolt::PackStream ps;
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
            bolt::PackStream ps;
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
        if (val.count("user_agent")) {
            auto& user_agent = std::any_cast<const std::string&>(val.at("user_agent"));
            if (fma_common::StartsWith(user_agent, "neo4j-python", false)) {
                session->python_driver = true;
            }
        }
        if (principal == lgraph::_detail::DEFAULT_ADMIN_NAME &&
            credentials == lgraph::_detail::DEFAULT_ADMIN_PASS) {
            session->using_default_user_password = true;
        }
        session->state = SessionState::READY;
        session->user = principal;
        session->fsm_thread = std::thread(BoltFSM, conn.shared_from_this());
        session->fsm_thread.detach();
        conn.SetContext(std::move(session));
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
        LOG_WARN() << "receive unknown bolt message: " << ToString(msg);
        conn.Close();
    }
};
}  // namespace bolt
