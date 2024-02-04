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
#include <unordered_map>
#include <optional>
#include <boost/endian/conversion.hpp>
#include "bolt/messages.h"

namespace bolt {

enum class PackType {
    Undef = 0,
    Integer,
    Float,
    String,
    Structure,
    Bytes,
    List,
    Dictionary,
    Null,
    True,
    False,
};

struct Marker {
    PackType typ = PackType::Undef;
    int8_t shortlen = 0;
    uint8_t numlenbytes = 0;
};

class Packer  {
 public:
    void Reset() {
        buf_ = nullptr;
        err_.reset();
    }
    void Begin(std::string* buf) {
        buf_ = buf;
        err_.reset();
    }
    const std::optional<std::string>& Err() {
        return err_;
    }
    void setErr(const std::string& err) {
        if (!err_) {
            err_ = err;
        }
    }
    void StructHeader(uint8_t tag, int num) {
        if (num > 0x0f) {
            setErr("Trying to pack struct with too many fields");
            return;
        }
        buf_->push_back((uint8_t)(0xb0) + uint8_t(num));
        buf_->push_back(tag);
    }
    void StructHeader(BoltMsg msg, int num) {
        StructHeader(static_cast<uint8_t>(msg), num);
    }
    void Int64(int64_t i);
    void Int32(int32_t i) {
        Int64(int64_t(i));
    }
    void Int16(int16_t i) {
        Int64(int64_t(i));
    }
    void Int8(int8_t i) {
        Int64(int64_t(i));
    }
    void Int(int i) {
        Int64(int64_t(i));
    }
    void Uint64(uint64_t i) {
        checkOverflowInt(i);
        Int64(int64_t(i));
    }
    void Uint32(uint32_t i) {
        Int64(int64_t(i));
    }
    void Uint16(uint16_t i) {
        Int64(int64_t(i));
    }
    void Uint8(uint8_t i) {
        Int64(int64_t(i));
    }
    void Double(double f) {
        buf_->push_back((uint8_t)0xc1);
        uint64_t t = 0;
        memcpy(&t, &f, sizeof(f));
        boost::endian::native_to_big_inplace(t);
        buf_->append((const char*)&t, sizeof(t));
    }
    void Float(float f) {
        Double(double(f));
    }
    void String(const std::string& s) {
        ListHeader(s.size(), 0x80, 0xd0);
        buf_->append(s);
    }
    void String(const char* s) {
        ListHeader(std::strlen(s), 0x80, 0xd0);
        buf_->append(s);
    }
    void Strings(const std::vector<std::string>& ss) {
        ListHeader(ss.size(), 0x90, 0xd4);
        for (auto& s : ss) {
            String(s);
        }
    }
    void Ints(const std::vector<int>& ii) {
        ListHeader(ii.size(), 0x90, 0xd4);
        for (auto& i : ii) {
            Int(i);
        }
    }
    void Int64s(const std::vector<int64_t>& ii) {
        ListHeader(ii.size(), 0x90, 0xd4);
        for (auto& i : ii) {
            Int64(i);
        }
    }
    void Doubles(const std::vector<double>& ii) {
        ListHeader(ii.size(), 0x90, 0xd4);
        for (auto& i : ii) {
            Double(i);
        }
    }
    void ListHeader(int l) {
        ListHeader(l, 0x90, 0xd4);
    }
    void MapHeader(int l) {
        ListHeader(l, 0xa0, 0xd8);
    }
    void IntMap(const std::unordered_map<std::string, int>& m) {
        ListHeader(m.size(), 0xa0, 0xd8);
        for (auto& pair : m) {
            String(pair.first);
            Int(pair.second);
        }
    }
    void StringMap(const std::unordered_map<std::string, std::string>& m) {
        ListHeader(m.size(), 0xa0, 0xd8);
        for (auto& pair : m) {
            String(pair.first);
            String(pair.second);
        }
    }
    void Bytes(const std::string& b);
    void Bool(bool b) {
        if (b) {
            buf_->push_back((uint8_t)0xc3);
            return;
        }
        buf_->push_back((uint8_t)0xc2);
    }
    void Null() {
        buf_->push_back((uint8_t)0xc0);
    }

 private:
    void ListHeader(int ll, uint8_t shortOffset, uint8_t longOffset);
    void checkOverflowInt(uint64_t i) {
        if (i > std::numeric_limits<int64_t>::max()) {
            err_ = "Trying to pack uint64 that doesn't fit into int64";
        }
    }

    std::string* buf_ = nullptr;
    std::optional<std::string> err_;
};

struct Unpacker {
 public:
    void Reset(std::string_view buf);
    void Next();
    uint32_t Len();
    int64_t Int();
    double Double();
    uint8_t StructTag() {
        return Pop();
    }
    std::string String();
    bool Bool();
    std::string ByteArray() {
        return Read(Len());
    }
    const std::optional<std::string>& Err() {
        return err_;
    }
    PackType CurrentType() {
        return curr_;
    }

 private:
    void SetErr(const std::string& err) {
        if (!err_) {
            err_ = err;
        }
    }
    uint8_t Pop();
    std::string Read(uint32_t n);
    void Read(uint32_t n, void* p);
    uint32_t ReadLen(uint32_t n);

    std::string_view buf_;
    uint32_t off_ = 0;
    uint32_t len_ = 0;
    Marker mrk_;
    std::optional<std::string> err_;
    PackType curr_ = PackType::Undef;
};

void MarkersInit();

}  // namespace bolt
