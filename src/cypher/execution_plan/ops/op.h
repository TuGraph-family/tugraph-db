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
// Created by wt on 6/12/18.
//
#pragma once

#include <string>
#include <vector>
#include "core/lightning_graph.h"
#include "filter/iterator.h"
#include "graph/graph.h"
#include "parser/data_typedef.h"
#include "execution_plan/runtime_context.h"
#include "cypher/cypher_types.h"
#include "execution_plan/visitor/visitor.h"
#include "monitor/memory_monitor_allocator.h"

namespace cypher {
enum OpType {
    AGGREGATE,
    ALL_NODE_SCAN,
    ALL_NODE_SCAN_DYNAMIC,
    NODE_BY_LABEL_SCAN_DYNAMIC,
    EXPAND_ALL,
    EXPAND_INTO,
    TRAVERSAl,
    REVERSED_EXPAND_ALL,
    VAR_LEN_EXPAND,
    VAR_LEN_EXPAND_INTO,
    VAR_LEN_REV_EXPAND,
    FILTER,
    NODE_BY_LABEL_SCAN,
    NODE_INDEX_SEEK,
    NODE_INDEX_SEEK_DYNAMIC,
    PRODUCE_RESULTS,
    PROJECT,
    DISTINCT,
    SORT,
    SKIP,
    LIMIT,
    OPTIONAL_,
    CREATE,
    UPDATE,
    DELETE_,
    REMOVE,
    STANDALONE_CALL,
    INQUERY_CALL,
    CARTESIAN_PRODUCT,
    ARGUMENT,
    APPLY,
    UNWIND,
    RELATIONSHIP_COUNT,
    MERGE,
    TOPN,
    UNION,
    NODE_BY_ID_SEEK,
    // TODO(lingsu): the operator and ast will be decoupled in the future, and ast will generate
    // symbolic information and expression, then the operator will complete the calculation through
    // the symbolic information and expression. and then the operator will be unified, without
    // distinguishing gql or cypher
    GQL_STANDALONE_CALL,
    GQL_CREATE,
    GQL_DELETE,
    GQL_UPDATE,
    GQL_INQUERY_CALL,
    GQL_MERGE
};

struct OpStats {
    size_t profileRecordCount = 0;  // Number of records generated.
    double profileExecTime = .0;    // Operation total execution time in ms.
};

struct OpBase {
    OpType type;                        // Type of operation
    std::string name;                   // Operation name.
    std::vector<std::string> modifies;  // List of aliases, this op modifies.
    std::vector<OpBase *> children;     // Child operations.
    OpBase *parent;                     // Parent operations.
    enum StreamState { StreamUnInitialized, StreamConsuming, StreamDepleted } state;
    /* Stream state. */              // TODO(anyone) remove
    std::shared_ptr<Record> record;  // Result of consume.
    OpStats stats;                   // Profiling statistics.
    enum OpResult {
        OP_DEPLETED = 1,
        OP_REFRESH = 2,
        OP_OK = 4,
        OP_ERR = 8,
    };

    OpBase() = default;

    OpBase(OpType t, const std::string &n)
        : type(t), name(n), parent(nullptr), state(StreamUnInitialized) {}

    virtual ~OpBase() = default;

    /* This method will initialize children, so make sure it's
     * called after the operators tree built.  */
    virtual OpResult Initialize(RTContext *ctx) = 0;

    virtual OpResult RealConsume(RTContext *ctx) = 0;

    virtual OpResult Consume(RTContext *ctx) {
        auto r = RealConsume(ctx);
        // Stop timer and accumulate.
        if (r == OP_OK) stats.profileRecordCount++;
        return r;
    }

    /* reset level:
     *   0: ready for re-consume
     *   1: reset to unitialized state (undo initialize)
     * */
    OpResult Reset(bool complete = false) { return ResetImpl(complete); }

    virtual std::string ToString() const = 0;

    // non-virtual methods
    bool ContainChild(OpBase *node) const {
        // TODO(anyone) optimize
        for (auto c : children) {
            if (c == node) return true;
        }
        return false;
    }

    void AddChild(OpBase *child) {
        // Add child to parent
        children.emplace_back(child);

        // Add parent to child
        child->parent = this;
    }

    void InsertChild(const std::vector<OpBase *>::const_iterator &position, OpBase *child) {
        children.insert(position, child);

        child->parent = this;
    }

    // Removes node child and update child parent lists
    void RemoveChild(OpBase *child) {
        for (auto it = children.begin(); it != children.end(); it++) {
            if (*it == child) {
                // remove parent from child
                child->parent = nullptr;
                // remove child from parent
                children.erase(it);
                break;
            }
        }
    }

    void PushInBetween(OpBase *onlyChild) {
        /* Disconnect every child from parent
         * Add each parent's child to only child. */
        for (auto child : children) {
            onlyChild->AddChild(child);
        }
        children.clear();
        AddChild(onlyChild);
    }

    void PushBelow(OpBase *op) {
        /* op is a new operation. */
        if (!op->children.empty() || op->parent || !parent) {
            throw lgraph::CypherException("internal error");
        }
        /* Replace A's former parent. */
        OpBase *a_former_parent = parent;
        /* Disconnect A from its former parent. */
        a_former_parent->RemoveChild(this);
        /* Add A's former parent as parent of B. */
        a_former_parent->AddChild(op);
        /* Add A as a child of B. */
        op->AddChild(this);
    }

    bool IsTap() const {
        return type == OpType::ALL_NODE_SCAN || type == OpType::NODE_BY_LABEL_SCAN ||
               type == OpType::NODE_INDEX_SEEK ||
               type == OpType::CREATE
               //|| type == OpType::UNWIND
               || type == OpType::STANDALONE_CALL;
    }

    bool IsScan() const {
        return type == OpType::ALL_NODE_SCAN || type == OpType::NODE_BY_LABEL_SCAN ||
               type == OpType::NODE_INDEX_SEEK || type == OpType::ARGUMENT;
    }

    bool IsDynamicScan() const {
        return type == OpType::NODE_INDEX_SEEK_DYNAMIC || type == OpType::ALL_NODE_SCAN_DYNAMIC ||
               type == OpType::NODE_BY_LABEL_SCAN_DYNAMIC;
    }

    bool IsStreamRoot() const {
        return type == OpType::PROJECT ||
               type == OpType::AGGREGATE
               // TODO(anyone) replace create/delete/set with 'empty project'
               || type == OpType::CREATE || type == OpType::DELETE_ || type == OpType::UPDATE ||
               type == OpType::GQL_CREATE || type == OpType::GQL_DELETE ||
               type == OpType::GQL_UPDATE;
    }

    // static methods
    static void ResetStream(OpBase *op, bool complete = false) {
        op->Reset(complete);
        if (complete) op->state = OpBase::StreamUnInitialized;

        for (auto child : op->children) {
            ResetStream(child, complete);
        }
    }

    static void FreeStream(OpBase *op) {
        if (!op) return;
        // Free child ops
        for (auto child : op->children) {
            FreeStream(child);
        }
        // Free op
        delete op;
    }

    static void DumpStream(OpBase *op, int indent, bool statistics, std::string &s) {
        if (!op) return;
        for (int i = 0; i < indent; i++) s.append(" ");
        s.append(op->ToString());
        if (statistics) {
            s.append(" (").append(std::to_string(op->stats.profileRecordCount)).append(" rows)");
        }
        s.append("\n");
        for (auto child : op->children) {
            DumpStream(child, indent + 4, statistics, s);
        }
    }

    virtual void Accept(Visitor *visitor) = 0;

    virtual void Accept(Visitor *visitor) const = 0;

 protected:
    // complete reset: set operation to uninitialized state.
    virtual OpResult ResetImpl(bool complete) = 0;
};

}  // namespace cypher
