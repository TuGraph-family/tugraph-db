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
namespace bolt {
extern boost::asio::io_service workers;
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
        conn.SetContext(std::move(session));
        ps.AppendSuccess(meta);
        conn.Respond(std::move(ps.MutableBuffer()));
    } else if (msg == BoltMsg::Run) {
        auto session = (BoltSession*)conn.GetContext();
        session->state = SessionState::STREAMING;
        // Now only implicit transactions are supported
        workers.post([conn = conn.shared_from_this(),
                      fields = std::move(fields)](){
            if (fields.size() < 3) {
                LOG_ERROR() << "Run msg fields size error, size: " << fields.size();
                bolt::PackStream ps;
                ps.AppendFailure({{"code", "error"},
                                  {"message", "Run msg fields size error"}});
                ps.AppendIgnored();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                return;
            }
            auto session = (BoltSession*)conn->GetContext();
            auto& cypher = std::any_cast<const std::string&>(fields[0]);
            auto& extra = std::any_cast<
                const std::unordered_map<std::string, std::any>&>(fields[2]);
            auto db_iter = extra.find("db");
            if (db_iter == extra.end()) {
                LOG_ERROR() << "Missing 'db' item in the 'extra' info of 'Run' msg";
                bolt::PackStream ps;
                ps.AppendFailure(
                    {{"code", "error"},
                     {"message", "Missing 'db' item in the 'extra' info of 'Run' msg"}});
                ps.AppendIgnored();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                return;
            }
            auto& graph = std::any_cast<const std::string&>(db_iter->second);
            auto sm = BoltServer::Instance().StateMachine();
            cypher::RTContext ctx(sm, sm->GetGalaxy(), session->user, graph);
            ctx.SetBoltConnection(conn.get());
            cypher::ElapsedTime elapsed;
            LOG_DEBUG() << "Bolt run " << cypher;
            try {
                sm->GetCypherScheduler()->Eval(&ctx, lgraph_api::GraphQueryType::CYPHER, cypher,
                                               elapsed);
            } catch (const std::exception& ex) {
                LOG_ERROR() << "Cypher execution meets exception: " << ex.what();
                bolt::PackStream ps;
                ps.AppendFailure({{"code", "error"},
                                  {"message", ex.what()}});
                ps.AppendIgnored();
                conn->PostResponse(std::move(ps.MutableBuffer()));
                return;
            }
            LOG_DEBUG() << "Cypher execution completed";
            session->state = SessionState::READY;
        });
    } else if (msg == BoltMsg::PullN) {
        if (fields.size() != 1) {
            LOG_ERROR() << "PullN msg fields size error, size: " << fields.size();
            ps.AppendFailure({{"code", "error"},
                              {"message", "PullN msg fields size error"}});
            conn.Respond(std::move(ps.MutableBuffer()));
            conn.Close();
            return;
        }
        auto& val = std::any_cast<const std::unordered_map<std::string, std::any>&>(fields[0]);
        auto n = std::any_cast<int64_t>(val.at("n"));
        auto session = (BoltSession*)conn.GetContext();
        session->msgs.Push({BoltMsg::PullN, n});
    } else if (msg == BoltMsg::DiscardN) {
        if (fields.size() != 1) {
            LOG_ERROR() << "DiscardN msg fields size error, size: " << fields.size();
            ps.AppendFailure({{"code", "error"},
                              {"message", "DiscardN msg fields size error"}});
            conn.Respond(std::move(ps.MutableBuffer()));
            conn.Close();
            return;
        }
        auto& val = std::any_cast<const std::unordered_map<std::string, std::any>&>(fields[0]);
        auto n = std::any_cast<int64_t>(val.at("n"));
        auto session = (BoltSession*)conn.GetContext();
        session->msgs.Push({BoltMsg::DiscardN, n});
    } else if (msg == BoltMsg::Begin ||
               msg == BoltMsg::Commit ||
               msg == BoltMsg::Rollback) {
        // Explicit transactions are not currently supported
        ps.Reset();
        ps.AppendSuccess();
        conn.Respond(std::move(ps.MutableBuffer()));
    } else if (msg == BoltMsg::Reset) {
        ps.Reset();
        ps.AppendSuccess();
        conn.Respond(std::move(ps.MutableBuffer()));
    } else if (msg == BoltMsg::Goodbye) {
        conn.Close();
    } else {
        LOG_WARN() << "receive unknown bolt message: " << ToString(msg);
        conn.Close();
    }
};
}  // namespace bolt
