/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "gtest/gtest.h"
#include "cpprest/json.h"
#include "cpprest/asyncrt_utils.h"
#include "./ut_utils.h"

#include "fma-common/logger.h"

#ifdef _WIN32
#include <codecvt>
#include <locale>
#define _U(str) utility::conversions::to_string_t(str)
#define _S(str) utility::conversions::to_utf8string(str)
#else
#define _U(buf) buf
#define _S(buf) buf
#endif

class TestCppRestSdkJson : public TuGraphTest {};

TEST_F(TestCppRestSdkJson, CppRestSdkJson) {
    using namespace fma_common;
    using namespace web;
    std::string binary(256, 0);
    for (size_t i = 0; i < binary.size(); i++) binary[i] = (char)i;
    std::vector<unsigned char> uchars(binary.begin(), binary.end());
    json::value v;
    v[_U("code")] = json::value(utility::conversions::to_base64(uchars));
    UT_LOG() << _S(v.serialize());
    std::vector<unsigned char> bytes = utility::conversions::from_base64(v[_U("code")].as_string());
    std::string ret(bytes.begin(), bytes.end());
    UT_EXPECT_EQ(binary, ret);
    for (size_t i = 0; i < ret.size(); i++) {
        int rets = ret[i] & 0xff;
        UT_EXPECT_EQ(rets, i);
    }
}
