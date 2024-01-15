/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Copyright (c) "Neo4j"
 * Neo4j Sweden AB [https://neo4j.com]
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
 * written by botu.wzy, inspired by Neo4j Go Driver
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace bolt {

enum class StatementType : char {
    StatementTypeUnknown = 0,
    StatementTypeRead,
    StatementTypeReadWrite,
    StatementTypeWrite,
    StatementTypeSchemaWrite
};
#define NodesCreated         "nodes-created"
#define NodesDeleted         "nodes-deleted"
#define RelationshipsCreated "relationships-created"
#define RelationshipsDeleted "relationships-deleted"
#define PropertiesSet        "properties-set"
#define LabelsAdded          "labels-added"
#define LabelsRemoved        "labels-removed"
#define IndexesAdded         "indexes-added"
#define IndexesRemoved       "indexes-removed"
#define ConstraintsAdded     "constraints-added"
#define ConstraintsRemoved   "constraints-removed"
#define SystemUpdates        "system-updates"

struct Plan {
    // operation is the operation this plan is performing.
    std::string operation;
    // arguments for the operator.
    // Many operators have arguments defining their specific behavior.
    // This map contains those arguments.
    std::unordered_map<std::string, std::any> arguments;
    // List of identifiers used by this plan. identifiers used by this part of the plan.
    // These can be both identifiers introduced by you, or automatically generated.
    std::vector<std::string> identifiers;
    // Zero or more child plans. A plan is a tree, where each child is another plan.
    // The children are where this part of the plan gets its input records - unless
    // this is an operator that
    // introduces new records on its own.
    std::vector<Plan> children;
};

struct ProfiledPlan {
    // operation contains the operation this plan is performing.
    std::string operation;
    // arguments contains the arguments for the operator used.
    // Many operators have arguments defining their specific behavior.
    // This map contains those arguments.
    std::unordered_map<std::string, std::any> arguments;
    // identifiers contains a list of identifiers used by this plan.
    // identifiers used by this part of the plan.
    // These can be both identifiers introduced by you, or automatically generated.
    std::vector<std::string> identifiers;
    // dbHits contains the number of times this part of the plan touched the underlying data stores
    int64_t dbHits = 0;
    // records contains the number of records this part of the plan produced.
    int64_t records = 0;
    // children contains zero or more child plans. A plan is a tree,
    // where each child is another plan.
    // The children are where this part of the plan gets its input
    // records - unless this is an operator that
    // introduces new records on its own.
    std::vector<ProfiledPlan> children;
    int64_t pageCacheMisses = 0;
    int64_t pageCacheHits = 0;
    double  pageCacheHitRatio = 0;
    int64_t time = 0;
};

// InputPosition contains information about a specific position in a statement
struct InputPosition {
    // offset contains the character offset referred to by this position; offset numbers start at 0.
    int offset = 0;
    // line contains the line number referred to by this position; line numbers start at 1.
    int line = 0;
    // column contains the column number referred to by this position; column numbers start at 1.
    int column = 0;
};

struct Notification {
    std::string code;
    std::string title;
    std::string description;
    std::optional<InputPosition> position;
    std::string severity;
    std::string category;
};

struct ProtocolVersion {
    int major = 0;
    int minor = 0;
};

struct Summary  {
    std::string bookmark;
    StatementType stmntType;
    std::string serverName;
    std::string agent;
    int major = 0;
    int minor = 0;
    std::unordered_map<std::string, int> counters;
    int64_t tFirst = 0;
    int64_t tLast = 0;
    std::shared_ptr<Plan> plan;
    std::shared_ptr<ProfiledPlan> profiledPlan;
    std::vector<Notification> notifications;
    std::string database;
    std::optional<bool> containsSystemUpdates;
    std::optional<bool> containsUpdates;
};

}  // namespace bolt
