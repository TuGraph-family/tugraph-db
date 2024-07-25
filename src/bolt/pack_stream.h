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
#include <iostream>
#include <any>
#include "tools/lgraph_log.h"
#include "fma-common/string_formatter.h"
#include "bolt/pack.h"
#include "bolt/messages.h"
#include "bolt/graph.h"
#include "bolt/path.h"
#include "bolt/temporal.h"

namespace bolt {

struct Chunker {
    std::string buf;
    // std::vector<int> sizes;
    int offset = 0;
    void BeginMessage() {
        // Space for length of next message
        buf.append(2, 0);
        offset = buf.size();
    }
    void EndMessage() {
        auto size = buf.size() - offset;
        // optimize
        while (size > 0xffff) {
            auto p = (uint16_t *) (buf.data() + offset - 2);
            *p = 0xffff;
            buf.insert(offset + 0xffff, 2, 0);
            offset += 0xffff + 2;
            size -= 0xffff;
        }
        auto p = (uint16_t *) (buf.data() + offset - 2);
        *p = size;
        boost::endian::native_to_big_inplace(*p);

        // Add zero chunk to mark End of message
        buf.append(2, 0);
    }
};

class PackStream {
 public:
    void Reset() {
        chunker_.buf.clear();
        chunker_.offset = 0;
        packer_.Reset();
    }
    void Begin() {
        chunker_.BeginMessage();
        packer_.Begin(&chunker_.buf);
    }
    void End() {
        const auto& err = packer_.Err();
        if (err) {
            LOG_FATAL() << FMA_FMT("packer meet error {}", err.value());
        }
        chunker_.EndMessage();
    }

    void PackDate(const bolt::Date& m) {
        packer_.StructHeader('D', 1);
        packer_.Int64(m.days);
    }
    void PackLocalDateTime(const bolt::LocalDateTime& m) {
        packer_.StructHeader('d', 2);
        packer_.Int64(m.seconds);
        packer_.Int64(m.nanoseconds);
    }

    void PackX(const std::any& x) {
        if (!x.has_value()) {
            packer_.Null();
            return;
        }
        auto& type = x.type();
        if (type == typeid(bool)) {
            packer_.Bool(std::any_cast<bool>(x));
        } else if (type == typeid(int8_t)) {
            packer_.Int8(std::any_cast<int8_t>(x));
        } else if (type == typeid(int16_t)) {
            packer_.Int16(std::any_cast<int16_t>(x));
        } else if (type == typeid(int32_t)) {
            packer_.Int32(std::any_cast<int32_t>(x));
        } else if (type == typeid(int64_t)) {
            packer_.Int64(std::any_cast<int64_t>(x));
        } else if (type == typeid(float)) {
            packer_.Float(std::any_cast<float>(x));
        } else if (type == typeid(double)) {
            packer_.Double(std::any_cast<double>(x));
        } else if (type == typeid(std::string)) {
            packer_.String(std::any_cast<const std::string&>(x));
        } else if (type == typeid(const char*)) {
            packer_.String(std::any_cast<const char*>(x));
        } else if (type == typeid(std::vector<std::string>)) {
            packer_.Strings(std::any_cast<const std::vector<std::string>&>(x));
        } else if (type == typeid(bolt::Node)) {
            PackNode(std::any_cast<const bolt::Node&>(x));
        } else if (type == typeid(bolt::Relationship)) {
            PackRelationship(std::any_cast<const bolt::Relationship&>(x));
        } else if (type == typeid(bolt::InternalPath)) {
            PackInternalPath(std::any_cast<const bolt::InternalPath&>(x));
        } else if (type == typeid(std::unordered_map<std::string, std::any>)) {
            PackMap(std::any_cast<const std::unordered_map<std::string, std::any>&>(x));
        } else if (type == typeid(std::vector<std::any>)) {
            PackList(std::any_cast<const std::vector<std::any>&>(x));
        } else if (type == typeid(bolt::Date)) {
            PackDate(std::any_cast<const bolt::Date&>(x));
        } else if (type == typeid(bolt::LocalDateTime)) {
            PackLocalDateTime(std::any_cast<const bolt::LocalDateTime&>(x));
        } else {
            LOG_FATAL() << FMA_FMT("PackX meet unexpected type {}", type.name());
        }
    }

    void PackString(const std::string& str) {
        packer_.String(str);
    }

    void PackMap(const std::unordered_map<std::string, std::any>& m) {
        packer_.MapHeader(m.size());
        for (auto& pair : m) {
            packer_.String(pair.first);
            PackX(pair.second);
        }
    }
    void PackList(const std::vector<std::any>& m) {
        packer_.ListHeader(m.size());
        for (auto& item : m) {
            PackX(item);
        }
    }

    void PackNode(const bolt::Node& node) {
        packer_.StructHeader('N', 3);
        packer_.Int64(node.id);
        packer_.ListHeader(node.labels.size());
        for (auto& label : node.labels) {
            packer_.String(label);
        }
        PackMap(node.props);
    }

    void PackRelationship(const bolt::Relationship& rel) {
        packer_.StructHeader('R', 5);
        packer_.Int64(rel.id);
        packer_.Int64(rel.startId);
        packer_.Int64(rel.endId);
        packer_.String(rel.type);
        PackMap(rel.props);
    }

    void PackUnboundRelationship(const bolt::RelNode& rel) {
        packer_.StructHeader('r', 3);
        packer_.Int64(rel.id);
        packer_.String(rel.name);
        PackMap(rel.props);
    }

    void PackInternalPath(const bolt::InternalPath& path) {
        packer_.StructHeader('P', 3);
        packer_.ListHeader(path.nodes.size());
        for (auto& node : path.nodes) {
            PackNode(node);
        }
        packer_.ListHeader(path.rels.size());
        for (auto& rel : path.rels) {
            PackUnboundRelationship(rel);
        }
        packer_.ListHeader(path.indices.size());
        for (auto index : path.indices) {
            packer_.Int64(index);
        }
    }

    void AppendStructMessage(BoltMsg type, const std::unordered_map<std::string, std::any>& meta) {
        Begin();
        packer_.StructHeader(type, 1);
        PackMap(meta);
        End();
    }
    void AppendStructMessage(BoltMsg type) {
        Begin();
        packer_.StructHeader(type, 0);
        End();
    }

    void AppendHello(const std::unordered_map<std::string, std::any>& meta) {
        AppendStructMessage(BoltMsg::Hello, meta);
    }

    void AppendSuccess(const std::unordered_map<std::string, std::any>& meta) {
        AppendStructMessage(BoltMsg::Success, meta);
    }

    void AppendSuccess() {
        AppendStructMessage(BoltMsg::Success, {});
    }

    void AppendIgnored() {
        AppendStructMessage(BoltMsg::Ignored);
    }

    void AppendFailure(const std::unordered_map<std::string, std::any>& meta) {
        AppendStructMessage(BoltMsg::Failure, meta);
    }

    void AppendRun(const std::string& cypher,
                   const std::unordered_map<std::string, std::any>& params,
                   const std::unordered_map<std::string, std::any>& meta) {
        Begin();
        packer_.StructHeader(BoltMsg::Run, 3);
        PackString(cypher);
        PackMap(params);
        PackMap(meta);
        End();
    }

    void AppendPullN(int64_t n) {
        AppendStructMessage(BoltMsg::PullN, {{"n", n}});
    }

    void AppendRecord(const std::vector<std::any>& fields) {
        Begin();
        packer_.StructHeader(BoltMsg::Record, 1);
        PackList(fields);
        End();
    }

    void AppendRecords(const std::vector<std::vector<std::any>>& records) {
        for (auto& r : records) {
            AppendRecord(r);
        }
    }

    const std::string& ConstBuffer() const {
        return chunker_.buf;
    }
    std::string& MutableBuffer() {
        return chunker_.buf;
    }

 private:
    Chunker chunker_;
    Packer packer_;
};

}  // namespace bolt
