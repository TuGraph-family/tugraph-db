/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "./test_tools.h"
#include <gtest/gtest.h>
#include <any>
#include "bolt/pack.h"
#include "bolt/messages.h"
#include "bolt/hydrator.h"
#include "bolt/spatial.h"
#include "bolt/graph.h"

using namespace bolt;

class TestBoltHydrator : public TuGraphTest {};

struct hydratorTestCase {
    std::string name;
    std::function<void()> build;
    std::any x;
    std::optional<std::string> err;
    bool useUtc = false;
};

bool operator==(const bolt::Success& lhs, const bolt::Success& rhs);
bool operator==(const Plan& lhs, const Plan& rhs);
bool operator==(const ProfiledPlan& lhs, const ProfiledPlan& rhs);
bool operator==(const Notification& lhs, const Notification& rhs);
bool operator==(const InputPosition& lhs, const InputPosition& rhs);
bool operator==(const bolt::RoutingTable& lhs, const bolt::RoutingTable& rhs);
bool operator==(const  Record& lhs, const Record& rhs);
bool operator==(const Point2D& lhs, const Point2D& rhs);
bool operator==(const  Point3D& lhs, const Point3D& rhs);

// for test
bool operator==(const bolt::Success& lhs, const bolt::Success& rhs) {
    if (lhs.fields != rhs.fields) {
        return false;
    }
    if (lhs.tfirst != rhs.tfirst) {
        return false;
    }
    if (lhs.qid != rhs.qid) {
        return false;
    }
    if (lhs.bookmark != rhs.bookmark) {
        return false;
    }
    if (lhs.connectionId != rhs.connectionId) {
        return false;
    }
    if (lhs.server != rhs.server) {
        return false;
    }
    if (lhs.db != rhs.db) {
        return false;
    }
    if (lhs.hasMore != rhs.hasMore) {
        return false;
    }
    if (lhs.tlast != rhs.tlast) {
        return false;
    }
    if (lhs.qtype != rhs.qtype) {
        return false;
    }

    if ((lhs.plan && !rhs.plan) || (!lhs.plan && rhs.plan)) {
        return false;
    }
    if (lhs.plan && rhs.plan) {
        if (!(*lhs.plan == *rhs.plan)) {
            return false;
        }
    }

    if ((lhs.profile && !rhs.profile) || (!lhs.profile && rhs.profile)) {
        return false;
    }
    if (lhs.profile && rhs.profile) {
        if (!(*lhs.profile == *rhs.profile)) {
            return false;
        }
    }

    if (lhs.notifications.size() != rhs.notifications.size()) {
        return false;
    }
    for (auto i = 0; i < lhs.notifications.size(); i++) {
        if (!(lhs.notifications[i] == rhs.notifications[i])) {
            return false;
        }
    }

    if ((lhs.routingTable && !rhs.routingTable) || (!lhs.routingTable && rhs.routingTable)) {
        return false;
    }
    if (lhs.routingTable && rhs.routingTable) {
        if (!(*lhs.routingTable == *rhs.routingTable)) {
            return false;
        }
    }

    if (lhs.num != rhs.num) {
        return false;
    }
    if (lhs.patches != rhs.patches) {
        return false;
    }
    return true;
}

// for test
inline bool operator==(const Plan& lhs, const Plan& rhs) {
    if (lhs.operation != rhs.operation) {
        return false;
    }
    if (lhs.arguments.size() != rhs.arguments.size()) {
        return false;
    }
    for (auto& pair : lhs.arguments) {
        auto iter = rhs.arguments.find(pair.first);
        if (iter == rhs.arguments.end()) {
            return false;
        }
        if (pair.second.type() != iter->second.type()) {
            return false;
        }
        if (pair.second.type() == typeid(int64_t)) {
            if (std::any_cast<int64_t>(pair.second) != std::any_cast<int64_t>(iter->second)) {
                return false;
            }
        } else {
            // unexpected type
            std::exit(-1);
        }
    }
    if (lhs.identifiers != rhs.identifiers) {
        return false;
    }
    if (lhs.children.size() != rhs.children.size()) {
        return false;
    }
    for (auto i = 0; i < lhs.children.size(); i++) {
        if (!(lhs.children[i] == rhs.children[i])) {
            return false;
        }
    }
    return true;
}

// for test
bool operator==(const ProfiledPlan& lhs, const ProfiledPlan& rhs) {
    if (lhs.operation != rhs.operation) {
        return false;
    }
    if (lhs.arguments.size() != rhs.arguments.size()) {
        return false;
    }
    for (auto& pair : lhs.arguments) {
        auto iter = rhs.arguments.find(pair.first);
        if (iter == rhs.arguments.end()) {
            return false;
        }
        if (pair.second.type() != iter->second.type()) {
            return false;
        }
        if (pair.second.type() == typeid(int64_t)) {
            if (std::any_cast<int64_t>(pair.second) != std::any_cast<int64_t>(iter->second)) {
                return false;
            }
        } else {
            // unexpected type
            std::exit(-1);
        }
    }
    if (lhs.identifiers != rhs.identifiers) {
        return false;
    }
    if (lhs.dbHits != rhs.dbHits) {
        return false;
    }
    if (lhs.records != rhs.records) {
        return false;
    }
    if (lhs.children.size() != rhs.children.size()) {
        return false;
    }
    if (lhs.pageCacheMisses != rhs.pageCacheMisses) {
        return false;
    }
    if (lhs.pageCacheHits != rhs.pageCacheHits) {
        return false;
    }
    if (lhs.pageCacheHitRatio != rhs.pageCacheHitRatio) {
        return false;
    }
    if (lhs.time != rhs.time) {
        return false;
    }
    for (auto i = 0; i < lhs.children.size(); i++) {
        if (!(lhs.children[i] == rhs.children[i])) {
            return false;
        }
    }
    return true;
}
// for test
bool operator==(const InputPosition& lhs, const InputPosition& rhs) {
    if (lhs.offset != rhs.offset) {
        return false;
    }
    if (lhs.line != rhs.line) {
        return false;
    }
    if (lhs.column != rhs.column) {
        return false;
    }
    return true;
}
// for test
bool operator==(const Notification& lhs, const Notification& rhs) {
    if (lhs.code != rhs.code) {
        return false;
    }
    if (lhs.title != rhs.title) {
        return false;
    }
    if (lhs.description != rhs.description) {
        return false;
    }
    if ((lhs.position && !rhs.position) || (!lhs.position && rhs.position)) {
        return false;
    }
    if (lhs.position && rhs.position) {
        if (!(lhs.position.value() == rhs.position.value())) {
            return false;
        }
    }
    if (lhs.severity != rhs.severity) {
        return false;
    }
    if (lhs.category != rhs.category) {
        return false;
    }
    return true;
}
// for test
bool operator==(const bolt::RoutingTable& lhs, const bolt::RoutingTable& rhs) {
    if (lhs.databaseName != rhs.databaseName) {
        return false;
    }
    if (lhs.timeToLive != rhs.timeToLive) {
        return false;
    }
    if (lhs.routers != rhs.routers) {
        return false;
    }
    if (lhs.readers != rhs.readers) {
        return false;
    }
    if (lhs.writers != rhs.writers) {
        return false;
    }
    return true;
}
// for test
bool operator==(const Point2D& lhs, const Point2D& rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.spatialRefId == rhs.spatialRefId);
}
// for test
bool operator==(const Point3D& lhs, const Point3D& rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z) &&
           (lhs.spatialRefId == rhs.spatialRefId);
}
// for test
bool operator==(const Record& lhs, const Record& rhs) {
    if (lhs.keys != rhs.keys) {
        return false;
    }
    if (lhs.values.size() != rhs.values.size()) {
        return false;
    }
    for (auto i = 0; i < lhs.values.size(); i++) {
        if (lhs.values[i].type() != rhs.values[i].type()) {
            return false;
        }
        if (lhs.values[i].type() == typeid(int64_t)) {
            if (std::any_cast<int64_t>(lhs.values[i]) !=
                std::any_cast<int64_t>(rhs.values[i])) {
                return false;
            }
        } else if (lhs.values[i].type() == typeid(Point2D)) {
            if (!(std::any_cast<const Point2D&>(lhs.values[i]) ==
                std::any_cast<const Point2D&>(rhs.values[i]))) {
                return false;
            }
        } else if (lhs.values[i].type() == typeid(Point3D)) {
            if (!(std::any_cast<const Point3D&>(lhs.values[i]) ==
                std::any_cast<const Point3D&>(rhs.values[i]))) {
                return false;
            }
        } else {
            // unexpected type
            std::exit(-1);
        }
    }
    return true;
}

#define Nanosecond (int64_t(1))
#define Microsecond  (1000 * Nanosecond)
#define Millisecond  (1000 * Microsecond)
#define Second       (1000 * Millisecond)
#define Minute       (60 * Second)
#define Hour         (60 * Minute)

TEST_F(TestBoltHydrator, all) {
    bolt::MarkersInit();
    Packer packer;
    std::vector<hydratorTestCase> cases = {
            {
                .name = "Ignored",
                .build = [&](){
                    packer.StructHeader(BoltMsg::Ignored, 0);
                },
                .x = new Ignored
            },
            {
                .name = "Error",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Failure, 0);
                    packer.MapHeader(3);
                    packer.String("code");
                    packer.String("the code");
                    packer.String("message");
                    packer.String("mess");
                    packer.String("extra key");  // Should be ignored
                    packer.Int(1);
                },
                .err = "ProtocolError"
            },
            {
                .name = "Success hello response",
                .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(3);
                        packer.String("connection_id");
                        packer.String("connid");
                        packer.String("server");
                        packer.String("srv");
                        packer.String("details");  // Should be ignored
                        packer.Int8(1);
                },
                .x = new bolt::Success{.tfirst = -1, .qid = -1, .connectionId = "connid",
                                   .server = "srv", .tlast = -1, .num = 3},
            },
            {
                .name = "Success commit/rollback/Reset response",
                .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(0);
                },
                .x = new bolt::Success{.tfirst = -1, .qid = -1, .tlast = -1, .num = 0},
            },
            {
                .name = "Success run response",
                .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(3);
                        packer.String("unknown");  // Should be ignored
                        packer.Int64(666);
                        packer.String("fields");
                        packer.ListHeader(2);   // >> fields array
                        packer.String("field1");
                        packer.String("field2");  // << fields array
                        packer.String("t_first");
                        packer.Int64(10000);
                },
                .x = new bolt::Success{.fields = {"field1", "field2"}, .tfirst = 10000,
                                   .qid = -1, .tlast = -1, .num = 3},
            },
            {
                    .name = "Success run response with qid",
                    .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(4);
                        packer.String("unknown");  // Should be ignored
                        packer.Int64(666);
                        packer.String("fields");
                        packer.ListHeader(2);   // >> fields array
                        packer.String("field1");
                        packer.String("field2");  // << fields array
                        packer.String("t_first");
                        packer.Int64(10000);
                        packer.String("qid");
                        packer.Int64(777);
                    },
                .x = new bolt::Success{.fields = {"field1", "field2"}, .tfirst = 10000,
                                   .qid = int64_t(777), .tlast = -1, .num = 4},
            },
            {
                    .name = "Success discard/End of page response with more data",
                    .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(1);
                        packer.String("has_more");
                        packer.Bool(true);
                    },
                    .x = new bolt::Success{ .tfirst = -1, .qid = -1, .hasMore = true,
                                   .tlast = -1, .num = 1},
            },
            {
                    .name = "Success discard response with no more data",
                    .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(4);
                        packer.String("has_more");
                        packer.Bool(false);
                        packer.String("whatever");  // >> Whatever array to ignore
                        packer.ListHeader(2);
                        packer.Int(1);
                        packer.Int(2);  // << Whatever array
                        packer.String("bookmark");
                        packer.String("bm");
                        packer.String("db");
                        packer.String("sys");
                    },
                .x = new bolt::Success{.tfirst = -1, .qid = -1, .bookmark = "bm", .db = "sys",
                                   .tlast = -1, .num = 4},
            },
            {
                    .name = "Success pull response, write with db",
                    .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(4);
                        packer.String("bookmark");
                        packer.String("b");
                        packer.String("t_last");
                        packer.Int64(124);
                        packer.String("type");
                        packer.String("w");
                        packer.String("db");
                        packer.String("s");
                    },
                    .x = new bolt::Success{.tfirst = -1, .qid = -1, .bookmark = "b",
                                   .db = "s", .tlast = 124,
                                   .qtype = StatementType::StatementTypeWrite, .num = 4},
            },
            {
                    .name = "Success summary with plan",
                    .build = [&]() {
                        packer.StructHeader(BoltMsg::Success, 1);
                        packer.MapHeader(4);
                        packer.String("has_more");
                        packer.Bool(false);
                        packer.String("bookmark");
                        packer.String("bm");
                        packer.String("db");
                        packer.String("sys");
                        packer.String("plan");  // Plan map
                        packer.MapHeader(4);
                        packer.String("operatorType");
                        packer.String("opType");
                        packer.String("identifiers");  // array
                        packer.ListHeader(2);
                        packer.String("id1");
                        packer.String("id2");
                        packer.String("args");  // map
                        packer.MapHeader(1);
                        packer.String("arg1");
                        packer.Int(1001);
                        packer.String("children");  // array of maps
                        packer.ListHeader(1);
                        packer.MapHeader(2);  // Another plan map
                        packer.String("operatorType");
                        packer.String("cop");
                        packer.String("identifiers");  // array
                        packer.ListHeader(1);
                        packer.String("cid");
                },
            .x = new bolt::Success{.tfirst = -1, .qid = -1,  .bookmark = "bm",
                                   .db = "sys", .tlast = -1,
                       .plan = std::make_shared<Plan>(Plan{
                           .operation = "opType",
                           .arguments = {{"arg1", int64_t(1001)}},
                           .identifiers = {"id1", "id2"},
                           .children = {Plan{.operation = "cop", .identifiers = {"cid"}}},
                       }), .num = 4
                    }
            },
            {
                .name = "Success summary with profile",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Success, 1);
                    packer.MapHeader(4);
                    packer.String("has_more");
                    packer.Bool(false);
                    packer.String("bookmark");
                    packer.String("bm");
                    packer.String("db");
                    packer.String("sys");
                    packer.String("profile");  // Profile map
                    packer.MapHeader(6);
                    packer.String("operatorType");
                    packer.String("opType");
                    packer.String("dbHits");
                    packer.Int(7);
                    packer.String("rows");
                    packer.Int(4);
                    packer.String("identifiers");  // array
                    packer.ListHeader(2);
                    packer.String("id1");
                    packer.String("id2");
                    packer.String("args");  // map
                    packer.MapHeader(1);
                    packer.String("arg1");
                    packer.Int(1001);
                    packer.String("children");  // array of maps
                    packer.ListHeader(1);
                    packer.MapHeader(4);  // Another profile map
                    packer.String("operatorType");
                    packer.String("cop");
                    packer.String("identifiers");  // array
                    packer.ListHeader(1);
                    packer.String("cid");  // << array
                    packer.String("dbHits");
                    packer.Int(1);
                    packer.String("rows");
                    packer.Int(2);
                },
                .x = new bolt::Success{.tfirst = -1, .qid = -1, .bookmark = "bm",
                                   .db = "sys", .tlast = -1,
                        .profile =  std::make_shared<ProfiledPlan>(ProfiledPlan{
                            .operation = "opType",
                            .arguments = {{"arg1", int64_t(1001)}},
                            .identifiers = {"id1", "id2"},
                            .dbHits =   int64_t(7),
                            .records =  int64_t(4),
                            .children = {
                                ProfiledPlan{.operation = "cop",
                                                 .identifiers = {"cid"},
                                                 .dbHits = int64_t(1),
                                                 .records = int64_t(2)},
                             },
                        }),
                        .num =  4
                }
            },
            {
                .name = "Success summary with notifications",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Success, 1);
                    packer.MapHeader(4);
                    packer.String("has_more");
                    packer.Bool(false);
                    packer.String("bookmark");
                    packer.String("bm");
                    packer.String("db");
                    packer.String("sys");
                    packer.String("notifications");  // List
                    packer.ListHeader(2);
                    packer.MapHeader(5);  // Notification map
                    packer.String("code");
                    packer.String("c1");
                    packer.String("title");
                    packer.String("t1");
                    packer.String("description");
                    packer.String("d1");
                    packer.String("severity");
                    packer.String("s1");
                    packer.String("position");
                    packer.MapHeader(3);
                    packer.String("offset");
                    packer.Int(1);
                    packer.String("line");
                    packer.Int(2);
                    packer.String("column");
                    packer.Int(3);
                    packer.MapHeader(4);  // Notification map
                    packer.String("code");
                    packer.String("c2");
                    packer.String("title");
                    packer.String("t2");
                    packer.String("description");
                    packer.String("d2");
                    packer.String("severity");
                    packer.String("s2");
                },
                .x = new bolt::Success{ .tfirst = -1, .qid = -1, .bookmark = "bm",
                                   .db = "sys", .tlast = -1,
                    .notifications =  {
                        Notification{.code = "c1", .title = "t1",
                                         .description = "d1", .position = InputPosition{
                                                        .offset = 1, .line = 2, .column = 3},
                                                    .severity = "s1"},
                        Notification{.code = "c2", .title = "t2",
                                         .description = "d2", .severity = "s2"},
                    },
                .num =  4},
            },
            {
                .name = "Success pull response Read no db",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Success, 1);
                    packer.MapHeader(4);
                    packer.String("bookmark");
                    packer.String("b1");
                    packer.String("t_last");
                    packer.Int64(7);
                    packer.String("type");
                    packer.String("routers");
                    packer.String("has_more");
                    packer.Bool(false);
                },
                .x = new bolt::Success{ .tfirst = -1, .qid = -1, .bookmark = "b1", .tlast = 7,
                                   .qtype = StatementType::StatementTypeRead, .num = 4},
            },
            {
                .name = "Success route response",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Success, 1);
                    packer.MapHeader(1);
                    packer.String("rt");
                    packer.MapHeader(3);
                    packer.String("ttl");
                    packer.Int(1001);
                    packer.String("db");
                    packer.String("dbname");
                    packer.String("servers");
                    packer.ListHeader(3);
                    // Routes
                    packer.MapHeader(2);
                    packer.String("role");
                    packer.String("ROUTE");
                    packer.String("addresses");
                    packer.ListHeader(2);
                    packer.String("router1");
                    packer.String("router2");
                    // readers
                    packer.MapHeader(2);
                    packer.String("role");
                    packer.String("READ");
                    packer.String("addresses");
                    packer.ListHeader(3);
                    packer.String("reader1");
                    packer.String("reader2");
                    packer.String("reader3");
                    // writers
                    packer.MapHeader(2);
                    packer.String("role");
                    packer.String("WRITE");
                    packer.String("addresses");
                    packer.ListHeader(1);
                    packer.String("writer1");
                },
                .x = new bolt::Success{ .tfirst = -1, .qid = -1, .tlast = -1,
                    .routingTable = std::make_unique<bolt::RoutingTable>(
                        bolt::RoutingTable{
                            .timeToLive = 1001,
                            .databaseName = "dbname",
                            .routers = {"router1", "router2"},
                            .readers = {"reader1", "reader2", "reader3"},
                            .writers = {"writer1"}}),
                            .num = 1,
                },
            },
            {
                .name = "Success route response no database name(<4.4)",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Success, 1);
                    packer.MapHeader(1);
                    packer.String("rt");
                    packer.MapHeader(2);
                    packer.String("ttl");
                    packer.Int(1001);
                    packer.String("servers");
                    packer.ListHeader(3);
                    // Routes
                    packer.MapHeader(2);
                    packer.String("role");
                    packer.String("ROUTE");
                    packer.String("addresses");
                            packer.ListHeader(2);
                    packer.String("router1");
                    packer.String("router2");
                    // readers
                    packer.MapHeader(2);
                    packer.String("role");
                    packer.String("READ");
                    packer.String("addresses");
                            packer.ListHeader(3);
                    packer.String("reader1");
                    packer.String("reader2");
                    packer.String("reader3");
                    // writers
                    packer.MapHeader(2);
                    packer.String("role");
                    packer.String("WRITE");
                    packer.String("addresses");
                    packer.ListHeader(1);
                    packer.String("writer1");
                },
                .x = new bolt::Success{.tfirst = -1, .qid = -1,  .tlast = -1,
                   .routingTable =  std::make_unique<bolt::RoutingTable>(bolt::RoutingTable{
                        .timeToLive =  1001,
                        .routers =  {"router1", "router2"},
                        .readers =  {"reader1", "reader2", "reader3"},
                        .writers =  {"writer1"}}),
                   .num =  1
                }
            },

            {
                .name = "Success route response extras",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Success, 1);
                    packer.MapHeader(2);
                    packer.String("extra1");
                    packer.ListHeader(2);
                    packer.Int(1);
                    packer.Int(2);
                    packer.String("rt");
                    packer.MapHeader(2);
                    packer.String("ttl");
                    packer.Int(1001);
                    packer.String("servers");
                    packer.ListHeader(1);
                    // Routes
                    packer.MapHeader(3);
                    packer.String("extra2");
                    packer.ListHeader(1);
                    packer.String("extraval2");
                    packer.String("role");
                    packer.String("ROUTE");
                    packer.String("addresses");
                    packer.ListHeader(1);
                    packer.String("router1");
                },
                .x = new bolt::Success{ .tfirst = -1, .qid = -1, .tlast = -1,
                        .routingTable = std::make_unique<bolt::RoutingTable>(bolt::RoutingTable{
                            .timeToLive = 1001, .routers = {"router1"}}),
                        .num = 2
                }
            },
            {
                .name = "Record of ints",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Record, 1);
                    packer.ListHeader(5);
                    packer.Int(1);
                    packer.Int(2);
                    packer.Int(3);
                    packer.Int(4);
                    packer.Int(5);
                },
                .x = std::make_optional<Record>(Record{.values = {int64_t(1),
                                                              int64_t(2), int64_t(3),
                                                              int64_t(4), int64_t(5)}})
            },
            {
                .name = "Record of spatials",
                .build = [&]() {
                    packer.StructHeader(BoltMsg::Record, 1);
                    packer.ListHeader(2);
                    packer.StructHeader('X', 3);  // Point2D
                    packer.Int64(1);
                    packer.Double(7.123);
                    packer.Double(123.7);
                    packer.StructHeader('Y', 4);  // Point3D
                    packer.Int64(2);
                    packer.Double(0.123);
                    packer.Double(23.71);
                    packer.Double(3.712);
                },
                .x = std::make_optional<Record>(Record{.values = {
                    Point2D{.x = 7.123, .y = 123.7, .spatialRefId = 1},
                    Point3D{.x = 0.123, .y = 23.71, .z = 3.712, .spatialRefId = 2}
                    }
                }),
            }
    };
    bolt::Hydrator hydrator;
    for (auto& c : cases) {
        hydrator.ClearErr();
        hydrator.UseUtc(c.useUtc);
        std::string buffer;
        packer.Begin(&buffer);
        c.build();
        auto err = packer.Err();
        UT_EXPECT_FALSE(err);
        auto hydrator_ret = hydrator.Hydrate(buffer);
        if (c.err) {
            UT_EXPECT_TRUE(hydrator_ret.second);
            continue;
        }
        UT_EXPECT_FALSE(hydrator_ret.second);
        UT_EXPECT_EQ(hydrator_ret.first.type(), c.x.type());
        if (hydrator_ret.first.type() == typeid(bolt::Success*)) {
            auto l = std::any_cast<bolt::Success*>(hydrator_ret.first);
            auto r = std::any_cast<bolt::Success*>(c.x);
            UT_EXPECT_TRUE(*l == *r);
            delete r;
        } else if (hydrator_ret.first.type() == typeid(std::optional<Record>)) {
            auto& l = std::any_cast<std::optional<Record>&>(hydrator_ret.first);
            auto& r = std::any_cast<std::optional<Record>&>(c.x);
            UT_EXPECT_EQ(l.has_value(), r.has_value());
            if (l.has_value()) {
                UT_EXPECT_TRUE(l.value() == r.value());
            }
        } else if (hydrator_ret.first.type() == typeid(bolt::Ignored*)) {
            auto r = std::any_cast<bolt::Ignored*>(c.x);
            delete r;
        }
    }
}
