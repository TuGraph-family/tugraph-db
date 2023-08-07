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

#include "fma-common/logging.h"
#include "gtest/gtest.h"

#include "core/value.h"
#include "./ut_utils.h"

class TestValue : public TuGraphTest {};

template <typename T>
inline T MaxMinus2() {
    return std::numeric_limits<T>::max() - 2;
}

template <typename T>
void TestUnalignedGetAndSet() {
    using namespace fma_common;
    using namespace lgraph;
    using namespace lgraph::_detail;
    int64_t buf[2];
    char* p = (char*)buf + 3;
    T data = MaxMinus2<T>();
    UnalignedSet(p, data);
    T read;
    UnalignedGet(p, read);
    UT_EXPECT_EQ(read, data);
    UT_EXPECT_EQ(UnalignedGet<T>(p), data);
}

template <typename T>
void CheckCopyT() {
    using namespace fma_common;
    using namespace lgraph;
    using namespace lgraph::_detail;
    {
        Value v;
        T d = MaxMinus2<T>();
        v.Copy(d);
        UT_EXPECT_EQ(v.AsType<T>(), d);
    }
    {
        Value v(32);
        T d = MaxMinus2<T>();
        v.Copy(d);
        UT_EXPECT_EQ(v.AsType<T>(), d);
    }
    {
        Value v(1024);
        T d = MaxMinus2<T>();
        v.Copy(d);
        UT_EXPECT_EQ(v.AsType<T>(), d);
    }
}

using namespace fma_common;
using namespace lgraph;
using namespace lgraph::_detail;

TEST_F(TestValue, ValueGetAndSet) {
    TestUnalignedGetAndSet<int8_t>();
    TestUnalignedGetAndSet<uint8_t>();
    TestUnalignedGetAndSet<int16_t>();
    TestUnalignedGetAndSet<uint16_t>();
    TestUnalignedGetAndSet<int32_t>();
    TestUnalignedGetAndSet<uint32_t>();
    TestUnalignedGetAndSet<int64_t>();
    TestUnalignedGetAndSet<uint64_t>();
    TestUnalignedGetAndSet<float>();
    TestUnalignedGetAndSet<double>();
}

TEST_F(TestValue, ValueConstruct) {
    Value v(1024);
    Value v2;
    char* buf = (char*)LBMalloc(1024);
    Value const_ref_to_buf(buf, 1024);
    v2.AssignConstRef(buf, 1024);
    Value v3 = v2;
    Value v4 = std::move(v2);
    Value v5(v);
    Value small_value(32);
    Value v6 = small_value;
    Value v7 = std::move(v);
    Value v8 = std::move(v6);
    v8 = v8;
    v8 = std::move(v8);
    v8 = v5;
    v8 = v5;
    Value v9;
    v9 = v3;
    v9 = std::move(v3);
    v9 = std::move(v8);
    v9 = std::move(Value(32));
    Value v10;
    v9 = v10;
    v9 = std::move(v10);
    LBFree(buf);
}

TEST_F(TestValue, ValueAssignMent) {
    // operator =
    Value v(63);
    Value v_tmp = v;
    Value v1(1024);
    v_tmp = v1;
    Value v2(63);
    v_tmp = v2;
    // value(&)
    Value v4(v);
    Value v5(v1);
    // left value
    Value v_tmp1 = std::move(v4);
    Value v_tmp3(std::move(v5));

    v = v2;
    v.Resize(20, 10);
    v1.Resize(1025, 10);
    Value v3;
    Value v6(v3);
    v3.Resize(20, 10);
}

TEST_F(TestValue, ValueMethonds) {
    Value large_v(1024);
    Value small_v(9);
    // Copy
    {
        Value v;
        v.Copy(large_v);
        v.Copy(small_v);
    }
    {
        Value v;
        v.Copy(small_v);
        v.Copy(large_v);
    }
    // MakeCopy
    {  // small value
        {
            Value v(32);
            char* p = v.Data();
            for (size_t i = 0; i < v.Size(); i++) {
                p[i] = (char)i;
            }
            Value v2 = Value::MakeCopy(v);
            UT_EXPECT_EQ(v.AsString(), v2.AsString());
        }
    }
}

TEST_F(TestValue, ValueCopy) {
    // large value
    {
        Value v(128);
        char* p = v.Data();
        for (size_t i = 0; i < v.Size(); i++) {
            p[i] = (char)i;
        }
        Value v2 = Value::MakeCopy(v);
        UT_EXPECT_EQ(v.AsString(), v2.AsString());
    }
    // Copy<T>(const T& d)
    {
        CheckCopyT<int8_t>();
        CheckCopyT<uint8_t>();
        CheckCopyT<int16_t>();
        CheckCopyT<uint16_t>();
        CheckCopyT<int32_t>();
        CheckCopyT<uint32_t>();
        CheckCopyT<int64_t>();
        CheckCopyT<uint64_t>();
        // Copy(buf, size)
        {
            Value v;
            std::string s("hello, world");
            v.Copy(s.data(), s.size());
            UT_EXPECT_EQ(v.AsString(), s);
        }
        // Copy(string)
        {
            Value v;
            std::string s("hello, world");
            v.Copy(s);
            UT_EXPECT_EQ(v.AsString(), s);
        }
    }
    // ConstRef
    {
        UT_EXPECT_EQ(Value::ConstRef("hello").AsString(), "hello");
        std::string s("hello world");
        UT_EXPECT_EQ(Value::ConstRef(s).AsString(), s);
        UT_EXPECT_EQ(Value::ConstRef(s.c_str()).AsString(), s);
        Value v(s);
        UT_EXPECT_EQ(Value::ConstRef(v).AsString(), s);
    }
    {
        std::string s("hello, world");
        auto v = Value::MakeCopy(s.data(), s.size());
        UT_EXPECT_TRUE(v.IsSlice());
        std::string b(100, 'a');
        v = Value::MakeCopy(b.data(), b.size());
        UT_EXPECT_FALSE(v.IsSlice());
    }
}
