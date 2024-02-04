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
#pragma once
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace bolt {
// Neo4jError is created when the database server failed to fulfill request.
struct Neo4jError {
    std::string code;
    std::string msg;
    bool parsed = false;
    std::string classification;
    std::string category;
    std::string title;
    bool retriable = false;
    std::string Error() {
        return "Neo4jError: " + code + " (" + msg + ")";
    }

    void Reclassify() {
        if (code == "Neo.TransientError.Transaction.LockClientStopped") {
            code = "Neo.ClientError.Transaction.LockClientStopped";
        } else if (code == "Neo.TransientError.Transaction.Terminated") {
            code = "Neo.ClientError.Transaction.Terminated";
        }
    }

    void Parse() {
        if (parsed) {
            return;
        }
        parsed = true;
        Reclassify();
        std::vector<std::string> parts;
        boost::split(parts, code, boost::is_any_of("."));
        if (parts.size() != 4) {
            return;
        }
        classification = parts[1];
        category = parts[2];
        title = parts[3];
    }

    bool HasSecurityCode() {
        return boost::algorithm::istarts_with(code, "Neo.ClientError.Security.");
    }

    bool IsAuthenticationFailed() {
        return code == "Neo.ClientError.Security.Unauthorized";
    }

    bool IsRetriableTransient() {
        Parse();
        return classification == "TransientError";
    }

    bool IsRetriableCluster() {
        if (code == "Neo.ClientError.Cluster.NotALeader" ||
            code == "Neo.ClientError.General.ForbiddenOnReadOnlyDatabase") {
            return true;
        } else {
            return false;
        }
    }

    void MarkRetriable() {
        retriable = true;
    }

    bool IsRetriable() {
        return retriable ||
            IsRetriableTransient() ||
            IsRetriableCluster() ||
            code == "Neo.ClientError.Security.AuthorizationExpired";
    }
};

}  // namespace bolt
