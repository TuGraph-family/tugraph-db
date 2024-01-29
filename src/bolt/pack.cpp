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
#include <iostream>
#include <boost/endian/conversion.hpp>
#include "bolt/pack.h"

namespace bolt {

Marker markers[0x100];

void Packer::ListHeader(int ll, uint8_t shortOffset, uint8_t longOffset) {
    auto l = int64_t(ll);
    std::string hdr;
    if (l < 0x10) {
        hdr.push_back(shortOffset + uint8_t(l));
    } else {
        if (l < 0x100) {
            hdr.push_back(longOffset);
            hdr.push_back(uint8_t(l));
        } else if (l < 0x10000) {
            hdr.push_back(longOffset + 1);
            auto num = boost::endian::native_to_big(uint16_t(l));
            hdr.append((const char*)&num, 2);
        } else if (l < std::numeric_limits<uint32_t>::max()) {
            hdr.push_back(longOffset + 2);
            auto num = boost::endian::native_to_big(uint32_t(l));
            hdr.append((const char *) &num, 4);
        } else {
            err_ = "Trying to pack too large list of size " + std::to_string(l);
            return;
        }
    }
    buf_->append(hdr);
}

void Packer::Int64(int64_t i) {
    if (-int64_t(0x10) <= i && i < int64_t(0x80)) {
        buf_->push_back((uint8_t)(i));
    } else if (-int64_t(0x80) <= i && i < -int64_t(0x10)) {
        buf_->push_back((uint8_t)0xc8);
        buf_->push_back((uint8_t)i);
    } else if (-int64_t(0x8000) <= i && i < int64_t(0x8000)) {
        buf_->push_back((uint8_t)0xc9);
        auto num = boost::endian::native_to_big(uint16_t(i));
        buf_->append((const char*)&num, 2);
    } else if (-int64_t(0x80000000) <= i && i < int64_t(0x80000000)) {
        buf_->push_back((uint8_t)0xca);
        auto num = boost::endian::native_to_big(uint32_t(i));
        buf_->append((const char*)&num, 4);
    } else {
        buf_->push_back((uint8_t)0xcb);
        auto num = boost::endian::native_to_big(uint64_t(i));
        buf_->append((const char*)&num, 8);
    }
}

void Packer::Bytes(const std::string& b) {
    std::string hdr;
    auto l = int64_t(b.size());
    if (l < 0x100) {
        hdr.push_back(uint8_t(0xcc));
        hdr.push_back(uint8_t(l));
    } else if (l < 0x10000) {
        hdr.push_back(uint8_t(0xcd));
        auto num = boost::endian::native_to_big(uint16_t(l));
        hdr.append((const char*)&num, sizeof(uint16_t));
    } else if (l < 0x100000000) {
        hdr.push_back(uint8_t(0xce));
        auto num = boost::endian::native_to_big(uint32_t(l));
        hdr.append((const char*)&num, sizeof(uint32_t));
    } else {
        err_ = "Trying to pack too large byte array of size " + std::to_string(l);
        return;
    }
    buf_->append(hdr);
    buf_->append(b);
}

void Unpacker::Reset(std::string_view buffer) {
    buf_ = buffer;
    off_ = 0;
    len_ = buf_.size();
    err_.reset();
    mrk_.typ = PackType::Undef;
    curr_ = PackType::Undef;
}

void Unpacker::Next() {
    auto i = Pop();
    mrk_ = markers[i];
    curr_ = mrk_.typ;
}

uint32_t Unpacker::Len() {
    if (mrk_.numlenbytes == 0) {
        return uint32_t(mrk_.shortlen);
    }
    return ReadLen(uint32_t(mrk_.numlenbytes));
}

int64_t Unpacker::Int() {
    auto n = mrk_.numlenbytes;
    if (n == 0) {
        return int64_t(mrk_.shortlen);
    }

    auto end = off_ + uint32_t(n);
    if (end > len_) {
        SetErr("IO Error");
        return 0;
    }
    int64_t i = 0;
    switch (n) {
    case 1:
        i = *((int8_t *) (buf_.data() + off_));
        break;
    case 2:
        i = boost::endian::big_to_native(*((int16_t *) (buf_.data() + off_)));
        break;
    case 4:
        i = boost::endian::big_to_native(*((int32_t *) (buf_.data() + off_)));
        break;
    case 8:
        i = boost::endian::big_to_native(*((int64_t *) (buf_.data() + off_)));
        break;
    default:
        SetErr("Illegal int length: " + std::to_string(i));
        return 0;
    }
    off_ = end;
    return i;
}

double Unpacker::Double() {
    char buffer[8] = {0};
    Read(8, buffer);
    if (err_) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    boost::endian::big_to_native_inplace(*(uint64_t*)buffer);
    return *(double*)buffer;
}

std::string Unpacker::String() {
    auto n = uint32_t(mrk_.numlenbytes);
    if (n == 0) {
        n = uint32_t(mrk_.shortlen);
    } else {
        n = ReadLen(n);
    }
    return Read(n);
}

bool Unpacker::Bool() {
    switch (curr_) {
    case PackType::True:
        return true;
    case PackType::False:
        return false;
    default:
        SetErr("Illegal value for bool");
        return false;
    }
}

uint8_t Unpacker::Pop() {
    if (off_ < len_) {
        auto x = buf_[off_];
        off_ += 1;
        return x;
    }
    SetErr("IO Error");
    return 0;
}

std::string Unpacker::Read(uint32_t n) {
    auto start = off_;
    auto end = off_ + n;
    if (end > len_) {
        SetErr("IO Error");
        return {};
    }
    off_ = end;
    return {buf_.data() + start, n};
}

void Unpacker::Read(uint32_t n, void* p) {
    auto start = off_;
    auto end = off_ + n;
    if (end > len_) {
        SetErr("IO Error");
        return;
    }
    off_ = end;
    memcpy(p, buf_.data() + start, n);
}

uint32_t Unpacker::ReadLen(uint32_t n) {
    auto end = off_ + n;
    if (end > len_) {
        SetErr("IO Error");
        return 0;
    }
    uint32_t l = 0;
    switch (n) {
    case 1:
        l = *((uint8_t *) (buf_.data() + off_));
        break;
    case 2:
        l = boost::endian::big_to_native(*((uint16_t *) (buf_.data() + off_)));
        break;
    case 4:
        l = boost::endian::big_to_native(*((uint32_t *) (buf_.data() + off_)));
        break;
    default:
        SetErr("Illegal length: " + std::to_string(n) + ", current type: " +
               std::to_string(static_cast<uint8_t>(CurrentType())));
        break;
    }
    off_ = end;
    return l;
}

void MarkersInit() {
    int i = 0;
    // Tiny int
    for (; i < 0x80; i++) {
        markers[i] = Marker{.typ = PackType::Integer, .shortlen = int8_t(i)};
    }
    // Tiny string
    for (; i < 0x90; i++) {
        markers[i] = Marker{.typ = PackType::String, .shortlen = int8_t(i-0x80)};
    }
    // Tiny array
    for (; i < 0xa0; i++) {
        markers[i] = Marker{.typ = PackType::List, .shortlen = int8_t(i-0x90)};
    }
    // Tiny map
    for (; i < 0xb0; i++) {
        markers[i] = Marker{.typ = PackType::Dictionary, .shortlen = int8_t(i-0xa0)};
    }
    // Structure
    for (; i < 0xc0; i++) {
        markers[i] = Marker{.typ = PackType::Structure, .shortlen = int8_t(i-0xb0)};
    }

    markers[0xc0] = Marker{.typ = PackType::Null};
    markers[0xc1] = Marker{.typ = PackType::Float, .numlenbytes = 8};
    markers[0xc2] = Marker{.typ = PackType::False};
    markers[0xc3] = Marker{.typ = PackType::True};

    markers[0xc8] = Marker{.typ = PackType::Integer, .numlenbytes = 1};
    markers[0xc9] = Marker{.typ = PackType::Integer, .numlenbytes = 2};
    markers[0xca] = Marker{.typ = PackType::Integer, .numlenbytes = 4};
    markers[0xcb] = Marker{.typ = PackType::Integer, .numlenbytes = 8};

    markers[0xcc] = Marker{.typ = PackType::Bytes, .numlenbytes = 1};
    markers[0xcd] = Marker{.typ = PackType::Bytes, .numlenbytes = 2};
    markers[0xce] = Marker{.typ = PackType::Bytes, .numlenbytes = 4};

    markers[0xd0] = Marker{.typ = PackType::String, .numlenbytes = 1};
    markers[0xd1] = Marker{.typ = PackType::String, .numlenbytes = 2};
    markers[0xd2] = Marker{.typ = PackType::String, .numlenbytes = 4};

    markers[0xd4] = Marker{.typ = PackType::List, .numlenbytes = 1};
    markers[0xd5] = Marker{.typ = PackType::List, .numlenbytes = 2};
    markers[0xd6] = Marker{.typ = PackType::List, .numlenbytes = 4};

    markers[0xd8] = Marker{.typ = PackType::Dictionary, .numlenbytes = 1};
    markers[0xd9] = Marker{.typ = PackType::Dictionary, .numlenbytes = 2};
    markers[0xda] = Marker{.typ = PackType::Dictionary, .numlenbytes = 4};

    for (i = 0xf0; i < 0x100; i++) {
        markers[i] = Marker{.typ = PackType::Integer, .shortlen = int8_t(i-0x100)};
    }
}

}  // namespace bolt
