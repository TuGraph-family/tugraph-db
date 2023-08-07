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

#include <random>

#include "fma-common/encrypt.h"
#include "fma-common/logger.h"
#include "./unit_test_utils.h"

FMA_SET_TEST_PARAMS(Encrypt, "");

FMA_UNIT_TEST(Encrypt) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    using namespace fma_common;
    size_t n = 1024;
    size_t max_size = 1024;

    FMA_LOG() << "Testing blowfish";
    encrypt::Encryptor<encrypt::Blowfish> enc("random_pass");
    std::random_device rnd;
    std::default_random_engine eng;
    std::uniform_int_distribution<size_t> gen(1, max_size);
    for (size_t i = 0; i < n; i++) {
        size_t s = gen(eng);
        std::string orig(s, 0);
        for (auto &c : orig) c = gen(eng) % 256;
        std::string encrypted = enc.Encrypt(orig);
        std::string decrypted = enc.Decrypt(encrypted);
        FMA_UT_CHECK_EQ(decrypted, orig);
        if (i == 0) {
            encrypt::Encryptor<encrypt::Blowfish> wrong_pass("wrong_pass");
            try {
                std::string w = wrong_pass.Decrypt(encrypted);
            } catch (std::exception &e) {
                FMA_LOG() << "Expected exception: decrypting with wrong password. Exception info: "
                          << e.what();
            }
        }
    }

    FMA_LOG() << "Testing md5";
    FMA_UT_CHECK_EQ(encrypt::MD5::Encrypt("hello, world"),
                 encrypt::Base16::Decode("e4d7f1b4ed2e42d15898f4b27b019da4"));
    FMA_UT_CHECK_EQ(encrypt::MD5::Encrypt("What does MD5 mean?"),
                 encrypt::Base16::Decode("99a5a46cee444fafac4a8f03c3aab8d8"));
    FMA_CHECK_NEQ(encrypt::MD5::Encrypt("What does MD5 mean?", "salt"),
                  encrypt::Base16::Decode("99a5a46cee444fafac4a8f03c3aab8d8"));

    FMA_LOG() << "Testing hex16";
    std::string orig;
    for (size_t i = 0; i < 255; i++) orig.push_back((char)i);
    std::string encrypted = encrypt::Base16::Encode(orig);
    std::string decrypted = encrypt::Base16::Decode(encrypted);

    FMA_EXPECT_EXCEPTION(encrypt::Base16::Decode("kkk"));

    FMA_UT_CHECK_EQ(orig, decrypted);
    for (size_t i = 0; i < n; i++) {
        size_t s = gen(eng);
        std::string orig(s, 0);
        for (auto &c : orig) c = gen(eng) % 256;
        std::string encrypted = encrypt::Base16::Encode(orig);
        std::string decrypted = encrypt::Base16::Decode(encrypted);
        FMA_UT_CHECK_EQ(orig, decrypted);
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
