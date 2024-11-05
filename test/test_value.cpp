/**
* Copyright 2024 AntGroup CO., Ltd.
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

#include <gtest/gtest.h>
#include "common/value.h"
#include <date/tz.h>
#include <boost/endian/conversion.hpp>

TEST(Value, basic) {
    Value v1(std::unordered_map<std::string, Value>{{"key1",Value::Integer(1)}, {"key2",Value::Double(2.1)}});
    Value v2(std::unordered_map<std::string, Value>{{"key3",Value::Bool(true)}, {"key4",Value::String("dddd")}});
    Value v3(std::vector<Value>{Value::Integer(100), Value::String("str"), v1, v2});
    EXPECT_EQ(v3.ToString(), R"([100, "str", {key2:2.100000, key1:1}, {key4:"dddd", key3:true}])");
}

TEST(Endian, big) {
    int64_t first = 0;
    std::string last, now;
    auto big = boost::endian::native_to_big(first);
    last.append((const char*)&big, sizeof(big));
    for (int64_t i = 1; i < 10000000; i++) {
        auto c = boost::endian::native_to_big(i);
        now.append((const char*)&c, sizeof(c));
        EXPECT_TRUE(now > last);
        last = now;
        now.clear();
    }
    int a = 1;
    int b = -1;
    auto big_a = boost::endian::native_to_big(a);
    auto big_b = boost::endian::native_to_big(b);
    std::string a_str, b_str;
    a_str.append((const char*)&big_a, sizeof(big_a));
    b_str.append((const char*)&big_b, sizeof(big_b));
    EXPECT_TRUE(a_str < b_str);
}