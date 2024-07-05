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
#include "tools/lgraph_log.h"
#include "bolt/hydrator.h"
#include "bolt/messages.h"
#include "bolt/graph.h"
#include "bolt/path.h"
#include "bolt/spatial.h"
#include "bolt/temporal.h"
#include "fma-common/string_formatter.h"
#include "lgraph/lgraph_exceptions.h"

namespace bolt {
const char* containsSystemUpdatesKey = "contains-system-updates";
const char* containsUpdatesKey = "contains-updates";

std::unordered_map<std::string, int>  ExtractIntCounters(
    std::unordered_map<std::string, std::any> counters) {
    std::unordered_map<std::string, int> result;
    for (auto& pair : counters) {
        if (pair.first != containsSystemUpdatesKey && pair.first != containsUpdatesKey) {
            result[pair.first] = std::any_cast<int>(pair.second);
        }
    }
    return result;
}
std::optional<bool> ExtractBoolPointer(std::unordered_map<std::string, std::any> counters,
                                       const std::string& key) {
    auto iter = counters.find(key);
    if (iter == counters.end()) {
        return std::nullopt;
    }
    return std::any_cast<std::optional<bool>>(iter->second);
}

void Hydrator::SetErr(const std::string& err) {
    if (!err_) {
        err_ = err;
    }
}

void Hydrator::ClearErr() {
    err_.reset();
}

void Hydrator::UseUtc(bool use) {
    useUtc_ = use;
}

const std::optional<std::string>& Hydrator::GetErr() {
    if (unp_->Err()) {
        return unp_->Err();
    }
    return err_;
}

void Hydrator::AssertLength(const std::string& structType, uint32_t expected, uint32_t actual) {
    if (expected != actual) {
        SetErr(FMA_FMT("Invalid length of struct, expected {} but was {}", expected, actual));
    }
}

// Hydrate hydrates a top-level struct message
std::pair<std::any, std::optional<std::string>> Hydrator::Hydrate(std::string_view buf) {
    unp_ = &unpacker_;
    unp_->Reset(buf);
    unp_->Next();

    if (unp_->CurrentType() != PackType::Structure) {
        return {{}, "expected struct"};
    }
    std::pair<std::any, std::optional<std::string>> ret;
    auto n = unp_->Len();
    auto t = static_cast<BoltMsg>(unp_->StructTag());
    switch (t) {
    case BoltMsg::Success:
        ret.first = GetSuccess(n);
        break;
    case BoltMsg::Ignored:
        ret.first = GetIgnored(n);
        break;
    case BoltMsg::Failure:
        ret.first = GetFailure(n);
        break;
    case BoltMsg::Record:
        ret.first = GetRecord(n);
        break;
    default:
        return {{}, "unexpected tag at top level: %d"};
    }
    ret.second = GetErr();
    return ret;
}

Ignored* Hydrator::GetIgnored(uint32_t n) {
    AssertLength("ignored", 0, n);
    if (GetErr()) {
        return nullptr;
    }
    return &cachedIgnored_;
}

std::optional<Neo4jError> Hydrator::GetFailure(uint32_t n) {
    AssertLength("failure", 1, n);
    if (GetErr()) {
        return std::nullopt;
    }
    Neo4jError dberr;
    unp_->Next();  // Detect map
    for (auto maplen = unp_->Len(); maplen > 0; maplen--) {
        unp_->Next();
        auto key = unp_->String();
        unp_->Next();
        if (key == "code") {
            dberr.code = unp_->String();
        } else if (key == "message") {
            dberr.msg = unp_->String();
        } else {
            // Do not fail on unknown value in map
            Trash();
        }
    }
    return dberr;
}

Success* Hydrator::GetSuccess(uint32_t n) {
    AssertLength("Success", 1, n);
    if (GetErr()) {
        return nullptr;
    }
    // Use cached Success but clear it first
    cachedSuccess_ = {};
    cachedSuccess_.qid = -1;
    cachedSuccess_.tfirst = -1;
    cachedSuccess_.tlast = -1;
    auto succ = &cachedSuccess_;

    unp_->Next();  // Detect map
    n = unp_->Len();
    succ->num = n;
    for (; n > 0; n--) {
        // Key
        unp_->Next();
        auto key = unp_->String();
        // Value
        unp_->Next();
        if (key == "fields") {
            succ->fields = GetStrings();
        } else if (key == "t_first") {
            succ->tfirst = unp_->Int();
        } else if (key == "qid") {
            succ->qid = unp_->Int();
        } else if (key == "bookmark") {
            succ->bookmark = unp_->String();
        } else if (key == "connection_id") {
            succ->connectionId = unp_->String();
        } else if (key == "server") {
            succ->server = unp_->String();
        } else if (key == "has_more") {
            succ->hasMore = unp_->Bool();
        } else if (key == "t_last") {
            succ->tlast = unp_->Int();
        } else if (key == "type") {
            auto statementType = unp_->String();
            if (statementType ==  "routers")
                succ->qtype = StatementType::StatementTypeRead;
            else if (statementType == "w")
                succ->qtype = StatementType::StatementTypeWrite;
            else if (statementType == "rw")
                succ->qtype = StatementType::StatementTypeReadWrite;
            else if (statementType == "s")
                succ->qtype = StatementType::StatementTypeSchemaWrite;
            else
                SetErr("unrecognized Success statement type %s");
        } else if (key == "db") {
            succ->db = unp_->String();
        } else if (key == "stats") {
            succ->counters = GetSuccessStats();
        } else if (key == "plan") {
            auto m = GetAmap();
            succ->plan = std::make_shared<Plan>(ParsePlan(m));
        } else if (key == "profile") {
            auto m = GetAmap();
            succ->profile = std::make_shared<ProfiledPlan>(ParseProfile(m));
        } else if (key =="notifications") {
            auto l = GetArray();
            succ->notifications = ParseNotifications(l);
        } else if (key == "rt") {
            succ->routingTable = GetRoutingTable();
        } else if (key == "hints") {
            auto hints = GetAmap();
            succ->configurationHints = hints;
        } else if (key == "patch_bolt") {
            auto patches = GetStrings();
            succ->patches = patches;
        } else {
            // Unknown key, waste it
            Trash();
        }
    }
    return succ;
}

std::unordered_map<std::string, std::any> Hydrator::GetSuccessStats() {
    auto n = unp_->Len();
    if (n == 0) {
        return {};
    }
    std::unordered_map<std::string, std::any> counts;
    for (; n > 0; n--) {
        unp_->Next();
        auto key = unp_->String();
        unp_->Next();
        auto val = parseStatValue(key);
        counts[key] = val;
    }
    return counts;
}

std::any Hydrator::parseStatValue(const std::string& key) {
    if (key == containsSystemUpdatesKey || key == containsUpdatesKey) {
        return std::optional<bool>(unp_->Bool());
    } else {
        return (int)(unp_->Int());
    }
}

// routingTable parses a routing table sent from the server. This is done
// the 'hard' way to reduce number of allocations (would be easier to go via
// a map) since it is called in normal flow (not that frequent...).
std::unique_ptr<RoutingTable> Hydrator::GetRoutingTable() {
    auto rt = std::make_unique<RoutingTable>();
    // Length of map
    auto nkeys = unp_->Len();
    for (; nkeys > 0; nkeys--) {
        unp_->Next();
        auto key = unp_->String();
        unp_->Next();
        if (key ==  "ttl") {
            rt->timeToLive = int(unp_->Int());
        } else if (key == "servers") {
            auto nservers = unp_->Len();
            for (; nservers > 0; nservers--) {
                routingTableRole(rt.get());
            }
        } else if (key ==  "db") {
            rt->databaseName = unp_->String();
        } else {
            // Unknown key, waste the value
            Trash();
        }
    }
    return rt;
}

void Hydrator::routingTableRole(RoutingTable* rt) {
    unp_->Next();
    auto nkeys = unp_->Len();
    std::string role;
    std::vector<std::string> addresses;
    for (; nkeys > 0; nkeys--) {
        unp_->Next();
        auto key = unp_->String();
        unp_->Next();
        if (key == "role") {
            role = unp_->String();
        } else if (key == "addresses") {
            addresses = GetStrings();
        } else {
            // Unknown key, waste the value
            Trash();
        }
    }
    if (role == "READ") {
        rt->readers = addresses;
    } else if (role == "WRITE") {
        rt->writers = addresses;
    } else if (role == "ROUTE") {
        rt->routers = addresses;
    }
}

std::vector<std::string> Hydrator::GetStrings() {
    auto n = unp_->Len();
    std::vector<std::string> slice;
    for (uint32_t i = 0; i < n; i++) {
        unp_->Next();
        slice.push_back(unp_->String());
    }
    return slice;
}

std::unordered_map<std::string, std::any> Hydrator::GetAmap() {
    auto n = unp_->Len();
    std::unordered_map<std::string, std::any> m;
    for (; n > 0; n--) {
        unp_->Next();
        auto key = unp_->String();
        unp_->Next();
        m[key] = GetValue();
    }
    return m;
}

std::vector<std::any> Hydrator::GetArray() {
    auto n = unp_->Len();
    std::vector<std::any> ret;
    for (uint32_t i = 0; i < n; i++) {
        unp_->Next();
        ret.push_back(GetValue());
    }
    return ret;
}

std::optional<Record> Hydrator::GetRecord(uint32_t n) {
    AssertLength("record", 1, n);
    if (GetErr()) {
        return std::nullopt;
    }
    Record rec;
    unp_->Next();  // Detect array
    n = unp_->Len();
    for (uint32_t i = 0; i < n; i++) {
        unp_->Next();
        rec.values.push_back(GetValue());
    }
    return rec;
}

std::any Hydrator::GetValue() {
    auto valueType = unp_->CurrentType();
    switch (valueType) {
    case PackType::Integer:
        return unp_->Int();
    case PackType::Float:
        return unp_->Double();
    case PackType::String:
        return unp_->String();
    case PackType::Structure: {
        auto t = unp_->StructTag();
        auto n = unp_->Len();
        switch (t) {
            case 'N':
                if (boltMajor_ >= 5) {
                    return GetNodeWithElementId(n);
                }
                return GetNode(n);
            case 'R':
                if (boltMajor_ >= 5) {
                    return GetRelationshipWithElementId(n);
                }
                return GetRelationship(n);
            case 'r':
                if (boltMajor_ >= 5) {
                    return GetRelationnodeWithElementId(n);
                }
                return GetRelationnode(n);
            case 'P':
                return GetPath(n);
            case 'X':
                return GetPoint2D(n);
            case 'Y':
                return GetPoint3D(n);
            case 'F':
                if (useUtc_) {
                    return GetUnknownStructError(t);
                }
                return GetDateTimeOffset(n);
            case 'I':
                if (!useUtc_) {
                    return GetUnknownStructError(t);
                }
                return GetUtcDateTimeOffset(n);
            case 'f':
                if (useUtc_) {
                    return GetUnknownStructError(t);
                }
                return GetDateTimeNamedZone(n);
            case 'i':
                if (!useUtc_) {
                    return GetUnknownStructError(t);
                }
                return GetUtcDateTimeNamedZone(n);
            case 'd':
                return GetLocalDateTime(n);
            case 'D':
                return GetDate(n);
            case 'T':
                return GetTime(n);
            case 't':
                return GetLocalTime(n);
            case 'E':
                return GetDuration(n);
            default:
                return GetUnknownStructError(t);
        }
    }
    case PackType::Bytes:
        return unp_->ByteArray();
    case PackType::List:
        return GetArray();
    case PackType::Dictionary:
        return GetAmap();
    case PackType::Null:
        return {};
    case PackType::True:
        return true;
    case PackType::False:
        return false;
    default:
        SetErr("Received unknown packstream value type: %d");
        return {};
    }
}


// Trashes current value
void Hydrator::Trash() {
    GetValue();
}

std::any Hydrator::GetNode(uint32_t num) {
    AssertLength("node", 3, num);
    if (GetErr()) {
        return {};
    }
    Node n;
    unp_->Next();
    n.id = unp_->Int();
    unp_->Next();
    n.labels = GetStrings();
    unp_->Next();
    n.props = GetAmap();
    n.elementId = std::to_string(n.id);
    return n;
}

std::any Hydrator::GetNodeWithElementId(uint32_t num) {
    AssertLength("node", 4, num);
    if (GetErr()) {
        return {};
    }
    Node n;
    unp_->Next();
    n.id = unp_->Int();
    unp_->Next();
    n.labels = GetStrings();
    unp_->Next();
    n.props = GetAmap();
    unp_->Next();
    n.elementId = unp_->String();
    return n;
}

std::any Hydrator::GetRelationship(uint32_t n) {
    AssertLength("relationship", 5, n);
    if (GetErr()) {
        return {};
    }
    Relationship r;
    unp_->Next();
    r.id = unp_->Int();
    unp_->Next();
    r.startId = unp_->Int();
    unp_->Next();
    r.endId = unp_->Int();
    unp_->Next();
    r.type = unp_->String();
    unp_->Next();
    r.props = GetAmap();
    r.elementId = std::to_string(r.id);
    r.startElementId = std::to_string(r.startId);
    r.endElementId = std::to_string(r.endId);
    return r;
}

std::any Hydrator::GetRelationshipWithElementId(uint32_t n) {
    AssertLength("relationship", 8, n);
    if (GetErr()) {
        return {};
    }
    Relationship r;
    unp_->Next();
    r.id = unp_->Int();
    unp_->Next();
    r.startId = unp_->Int();
    unp_->Next();
    r.endId = unp_->Int();
    unp_->Next();
    r.type = unp_->String();
    unp_->Next();
    r.props = GetAmap();
    unp_->Next();
    r.elementId = unp_->String();
    unp_->Next();
    r.startElementId = unp_->String();
    unp_->Next();
    r.endElementId = unp_->String();
    return r;
}

std::any Hydrator::GetRelationnode(uint32_t n) {
    AssertLength("relationnode", 3, n);
    if (GetErr()) {
        return {};
    }
    RelNode r;
    unp_->Next();
    r.id = unp_->Int();
    unp_->Next();
    r.name = unp_->String();
    unp_->Next();
    r.props = GetAmap();
    r.elementId = std::to_string(r.id);
    return r;
}


std::any Hydrator::GetRelationnodeWithElementId(uint32_t n) {
    AssertLength("relationnode", 4, n);
    if (GetErr()) {
        return {};
    }
    RelNode r;
    unp_->Next();
    r.id = unp_->Int();
    unp_->Next();
    r.name = unp_->String();
    unp_->Next();
    r.props = GetAmap();
    unp_->Next();
    r.elementId = unp_->String();
    return r;
}

std::any Hydrator::GetPath(uint32_t n) {
    AssertLength("path", 3, n);
    if (GetErr()) {
        return {};
    }
    // List of nodes
    unp_->Next();
    auto num = unp_->Int();
    std::vector<Node> nodes;
    for (auto i = 0; i < num; i++) {
        unp_->Next();
        auto val = GetValue();
        auto p = std::any_cast<Node>(&val);
        if (!p) {
            SetErr("value could not be cast to Node");
            return {};
        }
        nodes.emplace_back(std::move(*p));
    }
    // List of relnodes
    unp_->Next();
    num = unp_->Int();
    std::vector<RelNode> rnodes;
    for (auto i = 0; i < num; i++) {
        unp_->Next();
        auto val = GetValue();
        auto p = std::any_cast<RelNode>(&val);
        if (!p) {
            SetErr("value could be not cast to RelNode");
            return {};
        }
        rnodes.emplace_back(std::move(*p));
    }
    // List of indexes
    unp_->Next();
    num = unp_->Int();

    std::vector<int> indexes;
    for (auto i = 0; i < num; i++) {
        unp_->Next();
        indexes.push_back(int(unp_->Int()));
    }

    if ((indexes.size() & 0x01) == 1) {
        SetErr("there should be an even number of indices, found %d");
        return {};
    }

    return BuildPath(std::move(nodes), std::move(rnodes), std::move(indexes));
}

std::any Hydrator::GetPoint2D(uint32_t n) {
    Point2D p;
    unp_->Next();
    p.spatialRefId = uint32_t(unp_->Int());
    unp_->Next();
    p.x = unp_->Double();
    unp_->Next();
    p.y = unp_->Double();
    return p;
}

std::any Hydrator::GetPoint3D(uint32_t n) {
    Point3D p;
    unp_->Next();
    p.spatialRefId = uint32_t(unp_->Int());
    unp_->Next();
    p.x = unp_->Double();
    unp_->Next();
    p.y = unp_->Double();
    unp_->Next();
    p.z = unp_->Double();
    return p;
}

std::any Hydrator::GetDateTimeOffset(uint32_t n) {
    unp_->Next();
    auto seconds = unp_->Int();
    unp_->Next();
    auto nanos = unp_->Int();
    unp_->Next();
    auto offset = unp_->Int();
    return LegacyDateTime{.seconds = seconds, .nanoseconds = nanos, .tz_offset_seconds = offset};
}

std::any Hydrator::GetUtcDateTimeOffset(uint32_t n) {
    unp_->Next();
    auto seconds = unp_->Int();
    unp_->Next();
    auto nanos = unp_->Int();
    unp_->Next();
    auto offset = unp_->Int();
    return DateTime{.seconds = seconds, .nanoseconds = nanos, .tz_offset_seconds = offset};
}


std::any Hydrator::GetDateTimeNamedZone(uint32_t n) {
    unp_->Next();
    auto seconds = unp_->Int();
    unp_->Next();
    auto nanos = unp_->Int();
    unp_->Next();
    auto zone = unp_->String();
    return LegacyDateTimeZoneId{.seconds = seconds, .nanoseconds = nanos, .tz_id = zone};
}

std::any Hydrator::GetUtcDateTimeNamedZone(uint32_t n) {
    unp_->Next();
    auto secs = unp_->Int();
    unp_->Next();
    auto nans = unp_->Int();
    unp_->Next();
    auto zone = unp_->String();
    return DateTimeZoneId{.seconds = secs, .nanoseconds = nans, .tz_id = zone};
}

std::any Hydrator::GetLocalDateTime(uint32_t n) {
    unp_->Next();
    auto secs = unp_->Int();
    unp_->Next();
    auto nans = unp_->Int();
    return LocalDateTime{.seconds = secs, .nanoseconds = nans};
}

std::any Hydrator::GetDate(uint32_t n) {
    unp_->Next();
    auto days = unp_->Int();
    return Date{.days = days};
}

std::any Hydrator::GetTime(uint32_t n) {
    unp_->Next();
    auto nans = unp_->Int();
    unp_->Next();
    auto offs = unp_->Int();
    return Time{.nanoseconds = nans, .tz_offset_seconds = offs};
}

std::any Hydrator::GetLocalTime(uint32_t n) {
    unp_->Next();
    auto nans = unp_->Int();
    return LocalTime{.nanoseconds = nans};
}

std::any Hydrator::GetDuration(uint32_t n) {
    unp_->Next();
    auto mon = unp_->Int();
    unp_->Next();
    auto day = unp_->Int();
    unp_->Next();
    auto sec = unp_->Int();
    unp_->Next();
    auto nan = unp_->Int();
    return Duration{.months = mon, .days = day, .seconds = sec, .nanos = nan};
}

std::vector<Notification> ParseNotifications(std::vector<std::any> notificationsx) {
    std::vector<Notification> notifications;
    for (auto& item : notificationsx) {
        auto notificationx =
            std::any_cast<std::unordered_map<std::string, std::any>>(&item);
        if (notificationx) {
            notifications.push_back(ParseNotification(std::move(*notificationx)));
        }
    }
    return notifications;
}

std::tuple<std::string,
        std::vector<std::string>,
        std::unordered_map<std::string , std::any>,
        std::vector<std::any>>
ParsePlanOpIdArgsChildren(std::unordered_map<std::string, std::any> planx) {
    std::string oper;
    {
        auto iter = planx.find("operatorType");
        if (iter != planx.end()) {
            oper = std::any_cast<std::string &&>(std::move(iter->second));
        }
    }
    std::vector<std::any> identifiersx;
    {
        auto iter = planx.find("identifiers");
        if (iter != planx.end()) {
            identifiersx = std::any_cast<std::vector<std::any>&&>(std::move(iter->second));
        }
    }
    std::unordered_map<std::string, std::any> arguments;
    {
        auto iter = planx.find("args");
        if (iter != planx.end()) {
            arguments = std::any_cast<std::unordered_map<std::string, std::any>&&>(
                std::move(iter->second));
        }
    }
    std::vector<std::any> childrenx;
    {
        auto iter = planx.find("children");
        if (iter != planx.end()) {
            childrenx = std::any_cast<std::vector<std::any>&&>(std::move(iter->second));
        }
    }
    std::vector<std::string> identifiers;
    identifiers.reserve(identifiersx.size());
    for (auto& iden : identifiersx) {
        identifiers.push_back(std::any_cast<std::string&&>(std::move(iden)));
    }
    return {std::move(oper), std::move(identifiers), std::move(arguments), std::move(childrenx)};
}

Plan ParsePlan(std::unordered_map<std::string, std::any> planx) {
    auto tmp = ParsePlanOpIdArgsChildren(std::move(planx));
    Plan plan;
    plan.operation = std::move(std::get<0>(tmp));
    plan.arguments = std::move(std::get<2>(tmp));
    plan.identifiers = std::move(std::get<1>(tmp));
    for (auto& item : std::get<3>(tmp)) {
        auto childPlanx =
            std::any_cast<std::unordered_map<std::string, std::any>&&>(std::move(item));
        if (!childPlanx.empty()) {
            auto childPlan = ParsePlan(std::move(childPlanx));
            plan.children.push_back(std::move(childPlan));
        }
    }
    return plan;
}

ProfiledPlan ParseProfile(std::unordered_map<std::string, std::any> profilex) {
    ProfiledPlan plan;
    plan.dbHits = std::any_cast<int64_t>(profilex.at("dbHits"));
    plan.records = std::any_cast<int64_t>(profilex.at("rows"));
    auto tmp = ParsePlanOpIdArgsChildren(profilex);
    plan.operation = std::move(std::get<0>(tmp));
    plan.arguments = std::move(std::get<2>(tmp));
    plan.identifiers = std::move(std::get<1>(tmp));
    for (auto& item : std::get<3>(tmp)) {
        auto childPlanx =
            std::any_cast<std::unordered_map<std::string, std::any>>(item);
        if (!childPlanx.empty()) {
            auto childPlan = ParseProfile(childPlanx);
            {
                auto iter = childPlanx.find("pageCacheMisses");
                if (iter != childPlanx.end()) {
                    childPlan.pageCacheMisses = std::any_cast<int64_t>(iter->second);
                }
            }
            {
                auto iter = childPlanx.find("pageCacheHits");
                if (iter != childPlanx.end()) {
                    childPlan.pageCacheHits = std::any_cast<int64_t>(iter->second);
                }
            }
            {
                auto iter = childPlanx.find("pageCacheHitRatio");
                if (iter != childPlanx.end()) {
                    childPlan.pageCacheHitRatio = std::any_cast<double>(iter->second);
                }
            }
            {
                auto iter = childPlanx.find("time");
                if (iter != childPlanx.end()) {
                    childPlan.time = std::any_cast<int64_t>(iter->second);
                }
            }
            plan.children.push_back(std::move(childPlan));
        }
    }
    return plan;
}

Notification ParseNotification(std::unordered_map<std::string, std::any> m) {
    Notification n;
    {
        auto iter = m.find("code");
        if (iter != m.end()) {
            n.code = std::any_cast<std::string>(iter->second);
        }
    }
    {
        auto iter = m.find("description");
        if (iter != m.end()) {
            n.description = std::any_cast<std::string>(iter->second);
        }
    }
    {
        auto iter = m.find("severity");
        if (iter != m.end()) {
            n.severity = std::any_cast<std::string>(iter->second);
        }
    }
    {
        auto iter = m.find("category");
        if (iter != m.end()) {
            n.category = std::any_cast<std::string>(iter->second);
        }
    }
    {
        auto iter = m.find("title");
        if (iter != m.end()) {
            n.title = std::any_cast<std::string>(iter->second);
        }
    }
    auto iter = m.find("position");
    if (iter != m.end()) {
        n.position = std::make_optional<InputPosition>();
        auto& posx =
            std::any_cast<const std::unordered_map<std::string, std::any>&>(iter->second);
        auto i = std::any_cast<int64_t>(posx.at("column"));
        n.position.value().column = int(i);
        i = std::any_cast<int64_t>(posx.at("line"));
        n.position.value().line = int(i);
        i = std::any_cast<int64_t>(posx.at("offset"));
        n.position.value().offset = int(i);
    }
    return n;
}

std::any Hydrator::GetUnknownStructError(uint8_t t) {
    this->SetErr("Received unknown struct tag: " + std::to_string(t));
    return {};
}

// Utility to test hydration
std::any ServerHydrator(Unpacker& unpacker) {
    switch (unpacker.CurrentType()) {
        case PackType::Integer:
            return unpacker.Int();
        case PackType::Float:
            return unpacker.Double();
        case PackType::String:
            return unpacker.String();
        case PackType::Structure: {
            THROW_CODE(BoltDataException, "No support for unpacking struct in server stub");
        }
        case PackType::Bytes:
            return unpacker.ByteArray();
        case PackType::List: {
            auto n = unpacker.Len();
            std::vector<std::any> a;
            for (uint32_t i = 0; i < n; i++) {
                unpacker.Next();
                a.push_back(ServerHydrator(unpacker));
            }
            return a;
        }
        case PackType::Dictionary: {
            auto n = unpacker.Len();
            std::unordered_map<std::string, std::any> m;
            for (; n > 0; n--) {
                unpacker.Next();
                auto key = unpacker.String();
                unpacker.Next();
                m[key] = ServerHydrator(unpacker);
            }
            return m;
        }
        case PackType::Null:
            return {};
        case PackType::True:
            return true;
        case PackType::False:
            return false;
        default: {
            THROW_CODE(BoltDataException, "Unsupported type to unpack: {}",
                       static_cast<int>(unpacker.CurrentType()));
        }
    }
}

}  // namespace bolt
