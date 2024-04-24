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

#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "lgraph/lgraph_utils.h"

class TestLGraphUtils : public TuGraphTest {};

TEST_F(TestLGraphUtils, LGraphUtils) {
    using namespace lgraph_api;
    std::string code64 =
        "ZGVmIFByb2Nlc3MoZGIsIGlucHV0KToKICAgIHR4biA9IGRiLkNyZWF0ZVJlYWRUeG4oKQogICAgaXQgPSB0eG4uR2"
        "V0VmVydGV4SXRlcmF0b3IoKQogICAgbiA9IDAKICAgIHdoaWxlIGl0LklzVmFsaWQoKToKICAgICAgICBpZiBpdC5H"
        "ZXRMYWJlbCgpID09ICdQZXJzb24nOgogICAgICAgICAgICBuID0gbiArIDEKICAgICAgICBpdC5OZXh0KCkKICAgIH"
        "JldHVybiAoVHJ1ZSwgc3RyKG4pKQ==";
    std::vector<std::string> str_vec;
    std::string src = "test1,test2,test3,test4";
    split_string(src, str_vec, ",");
    UT_EXPECT_EQ(str_vec[0], "test1");
    std::string key = "my_key";
    std::string encrypt = rc4(src, key, "encrypt");
    std::string decrypt = rc4(encrypt, key, "decrypt");
    UT_EXPECT_EQ(src, decrypt);
    std::string decode = decode_base64(code64);
    std::string encode = encode_base64(decode);
    UT_EXPECT_EQ(encode, code64);
    size_t* addr = (size_t*)alloc_buffer(100);
    UT_EXPECT_TRUE(addr);
    dealloc_buffer(addr, 100);
}
