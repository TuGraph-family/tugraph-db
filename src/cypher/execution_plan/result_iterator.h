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

#pragma once
#include "graphdb/iterator.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/common/ObjectAllocator.h"
#include "cypher/execution_plan/execution_plan.h"
#include "common/result.h"
#include <string>

class ResultIterator : public graphdb::Iterator {
public:
    explicit ResultIterator(void* ctx, txn::Transaction* txn, std::string cypher);
    void Next() override;
    const std::vector<common::Result>& GetRecord() {
        assert(valid_);
        return record_;
    }
    std::vector<std::any> GetBoltRecord() {
        assert(valid_);
        std::vector<std::any> ret;
        ret.reserve(record_.size());
        for (const auto& item : record_) {
            ret.emplace_back(item.ToBolt());
        }
        return ret;
    }
    const std::vector<std::string>& GetHeader() {
        return header_;
    }
    void Consume() {
        while (Valid()) {
            Next();
        }
    }
private:
    void ReFillRecord();
    cypher::RTContext* ctx_ = nullptr;
    std::string cypher_;
    geax::common::ObjectArenaAllocator objAlloc_;
    cypher::ExecutionPlan execution_plan_v2_;
    cypher::OpBase* root_ = nullptr;
    cypher::OpBase::OpResult res_ = cypher::OpBase::OpResult::OP_ERR;
    std::vector<std::string> header_;
    std::vector<common::Result> record_;
};
