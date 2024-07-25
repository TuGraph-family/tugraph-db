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
#include <tuple>
#include <unordered_map>
#include <any>
#include "tools/json.hpp"
#include "bolt/summary.h"
#include "bolt/pack.h"
#include "bolt/record.h"
#include "bolt/errors.h"
#include "bolt/spatial.h"

namespace bolt {

struct Ignored {
};

struct RoutingTable {
    int timeToLive = 0;
    std::string databaseName;
    std::vector<std::string> routers;
    std::vector<std::string> readers;
    std::vector<std::string> writers;
};

struct Success {
    std::vector<std::string> fields;
    int64_t tfirst = 0;
    int64_t qid = 0;
    std::string bookmark;
    std::string connectionId;
    std::string server;
    std::string db;
    bool hasMore = false;
    int64_t tlast = 0;
    StatementType qtype;
    std::unordered_map<std::string, std::any> counters;
    std::shared_ptr<Plan> plan;
    std::shared_ptr<ProfiledPlan> profile;
    std::vector<Notification> notifications;
    std::unique_ptr<RoutingTable> routingTable;
    uint32_t num = 0;
    std::unordered_map<std::string, std::any> configurationHints;
    std::vector<std::string> patches;
};

class Hydrator {
 public:
    void ClearErr();
    void UseUtc(bool use);
    std::pair<std::any, std::optional<std::string>> Hydrate(std::string_view buf);

 private:
    void SetErr(const std::string& err);
    const std::optional<std::string>& GetErr();
    void AssertLength(const std::string& structType, uint32_t expected, uint32_t actual);
    Ignored* GetIgnored(uint32_t n);
    std::optional<Neo4jError> GetFailure(uint32_t n);
    Success* GetSuccess(uint32_t n);
    std::unordered_map<std::string, std::any> GetSuccessStats();
    std::any parseStatValue(const std::string& key);
    std::unique_ptr<RoutingTable> GetRoutingTable();
    void routingTableRole(RoutingTable* rt);
    std::vector<std::string> GetStrings();
    std::unordered_map<std::string, std::any> GetAmap();
    std::vector<std::any> GetArray();
    std::optional<Record> GetRecord(uint32_t n);
    std::any GetValue();
    void Trash();
    std::any GetNode(uint32_t num);
    std::any GetNodeWithElementId(uint32_t num);
    std::any GetRelationship(uint32_t n);
    std::any GetRelationshipWithElementId(uint32_t n);
    std::any GetRelationnode(uint32_t n);
    std::any GetRelationnodeWithElementId(uint32_t n);
    std::any GetPath(uint32_t n);
    std::any GetPoint2D(uint32_t n);
    std::any GetPoint3D(uint32_t n);
    std::any GetDateTimeOffset(uint32_t n);
    std::any GetUtcDateTimeOffset(uint32_t n);
    std::any GetDateTimeNamedZone(uint32_t n);
    std::any GetUtcDateTimeNamedZone(uint32_t n);
    std::any GetLocalDateTime(uint32_t n);
    std::any GetDate(uint32_t n);
    std::any GetTime(uint32_t n);
    std::any GetLocalTime(uint32_t n);
    std::any GetDuration(uint32_t n);
    std::any GetUnknownStructError(uint8_t t);

    Unpacker unpacker_;
    Unpacker* unp_ = nullptr;
    std::optional<std::string> err_;
    Ignored cachedIgnored_;
    Success cachedSuccess_;
    std::string logId_;
    int boltMajor_ = 0;
    bool useUtc_ = false;
};
std::unordered_map<std::string, int>
    ExtractIntCounters(std::unordered_map<std::string, std::any> counters);
std::optional<bool> ExtractBoolPointer(std::unordered_map<std::string, std::any> counters,
                                       const std::string& key);
std::vector<Notification> ParseNotifications(std::vector<std::any> notificationsx);
std::tuple<std::string,
    std::vector<std::string>,
        std::unordered_map<std::string , std::any>,
        std::vector<std::any>>
ParsePlanOpIdArgsChildren(std::unordered_map<std::string, std::any> planx);
Plan ParsePlan(std::unordered_map<std::string, std::any> planx);
ProfiledPlan ParseProfile(std::unordered_map<std::string, std::any> profilex);
Notification ParseNotification(std::unordered_map<std::string, std::any> m);
std::any ServerHydrator(Unpacker& unpacker);
}  // namespace bolt

