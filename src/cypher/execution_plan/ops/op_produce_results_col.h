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

#pragma once

#include <regex>
#include "cypher/execution_plan/ops/op.h"
#include "lgraph/lgraph_result.h"
#include "lgraph/lgraph_types.h"
#include "lgraph_api/result_element.h"
#include "resultset/record.h"
#include "server/json_convert.h"
#include "server/bolt_session.h"
#include "boost/regex.hpp"
#include "cypher/execution_plan/ops/op_produce_results.h"

namespace cypher {

class ProduceResultsCol : public OpBase {
    enum {
        Uninitialized,
        RefreshAfterPass,
        Resetted,
        Consuming,
    } state_;
    lgraph_api::NODEMAP node_map_;
    lgraph_api::RELPMAP relp_map_;
    std::shared_ptr<DataChunk> final_r;

 public:
    ProduceResultsCol() : OpBase(OpType::PRODUCE_RESULTS, "Produce Results") {
        state_ = Uninitialized;
    }

    OpResult Initialize(RTContext *ctx) override {
        if (!children.empty()) {
            children[0]->Initialize(ctx);
        }
        columnar_ = std::make_shared<DataChunk>();
        final_r = std::make_shared<DataChunk>();
        return OP_OK;
    }

    /* ProduceResults next operation
     * called each time a new result record is required */
    OpResult RealConsume(RTContext *ctx) override {
        if (state_ == Uninitialized) {
            Initialize(ctx);
            state_ = Consuming;
        }
        if (children.empty()) return OP_DEPLETED;
        if (ctx->bolt_conn_) {
            if (ctx->bolt_conn_->has_closed()) {
                LOG_INFO() << "The bolt connection is closed, cancel the op execution.";
                return OP_ERR;
            }
            auto session = (bolt::BoltSession *)ctx->bolt_conn_->GetContext();
            while (session->state == bolt::SessionState::STREAMING && !session->streaming_msg) {
                session->streaming_msg = session->msgs.Pop(std::chrono::milliseconds(100));
                if (ctx->bolt_conn_->has_closed()) {
                    LOG_INFO() << "The bolt connection is closed, cancel the op execution.";
                    return OP_ERR;
                }
                if (!session->streaming_msg) {
                    continue;
                }
                if (session->streaming_msg.value().type == bolt::BoltMsg::PullN ||
                    session->streaming_msg.value().type == bolt::BoltMsg::DiscardN) {
                    const auto &fields = session->streaming_msg.value().fields;
                    if (fields.size() != 1) {
                        std::string err =
                            FMA_FMT("{} msg fields size error, size: {}",
                                    bolt::ToString(session->streaming_msg.value().type).c_str(),
                                    fields.size());
                        LOG_ERROR() << err;
                        bolt::PackStream ps;
                        ps.AppendFailure({{"code", "error"}, {"message", err}});
                        ctx->bolt_conn_->PostResponse(std::move(ps.MutableBuffer()));
                        session->state = bolt::SessionState::FAILED;
                        return OP_ERR;
                    }
                    auto &val =
                        std::any_cast<const std::unordered_map<std::string, std::any> &>(fields[0]);
                    auto n = std::any_cast<int64_t>(val.at("n"));
                    session->streaming_msg.value().n = n;
                } else if (session->streaming_msg.value().type == bolt::BoltMsg::Reset) {
                    LOG_INFO() << "Receive RESET, cancel the op execution.";
                    bolt::PackStream ps;
                    ps.AppendSuccess();
                    ctx->bolt_conn_->PostResponse(std::move(ps.MutableBuffer()));
                    session->state = bolt::SessionState::READY;
                    return OP_ERR;
                } else {
                    LOG_ERROR() << FMA_FMT(
                        "Unexpected msg:{} in STREAMING state, cancel the op execution, "
                        "close the connection.",
                        bolt::ToString(session->streaming_msg.value().type));
                    ctx->bolt_conn_->Close();
                    return OP_ERR;
                }
                break;
            }
            if (session->state == bolt::SessionState::INTERRUPTED) {
                LOG_WARN() << "The session state is INTERRUPTED, cancel the op execution.";
                return OP_ERR;
            } else if (session->state != bolt::SessionState::STREAMING) {
                LOG_ERROR() << "Unexpected state: {} in op execution, close the connection.";
                ctx->bolt_conn_->Close();
                return OP_ERR;
            } else if (session->streaming_msg.value().type != bolt::BoltMsg::PullN &&
                       session->streaming_msg.value().type != bolt::BoltMsg::DiscardN) {
                LOG_ERROR() << FMA_FMT("Unexpected msg: {} in op execution, "
                    "cancel the op execution, close the connection.",
                    bolt::ToString(session->streaming_msg.value().type));
                ctx->bolt_conn_->Close();
                return OP_ERR;
            }
            auto child = children[0];
            auto res = child->Consume(ctx);
            if (res != OP_OK) {
                if (ctx->result_->Size() > 0 &&
                    session->streaming_msg.value().type == bolt::BoltMsg::PullN) {
                    session->ps.AppendRecords(ctx->result_->BoltRecords());
                }
                session->ps.AppendSuccess();
                session->state = bolt::SessionState::READY;
                ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                session->ps.Reset();
                return res;
            }
            if (session->streaming_msg.value().type == bolt::BoltMsg::PullN) {
                auto record = ctx->result_->MutableRecord();
                RRecordToURecord(ctx->txn_.get(), ctx->result_->Header(), child->record,
                                *record, node_map_, relp_map_);
                session->ps.AppendRecords(ctx->result_->BoltRecords());
                ctx->result_->ClearRecords();
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
                    ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                    session->ps.Reset();
                }
            } else if (session->streaming_msg.value().type == bolt::BoltMsg::DiscardN) {
                if (--session->streaming_msg.value().n == 0) {
                    std::unordered_map<std::string, std::any> meta;
                    meta["has_more"] = true;
                    session->ps.AppendSuccess(meta);
                    session->state = bolt::SessionState::STREAMING;
                    session->streaming_msg.reset();
                    ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                    session->ps.Reset();
                }
            }
            return OP_OK;
        } else {
            auto child = children[0];
            auto res = child->Consume(ctx);
            if (res != OP_OK) return res;
            columnar_ = child->columnar_;
            final_r->Append(*columnar_);
            ctx->moveDataChunk(final_r);
            return OP_OK;
        }
    }

    /* Restart */
    OpResult ResetImpl(bool complete) override {
        if (complete) state_ = Uninitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
