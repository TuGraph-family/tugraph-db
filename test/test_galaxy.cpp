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
#include "server/galaxy.h"
#include "transaction/transaction.h"
#include <filesystem>
#include "test_util.h"
namespace fs = std::filesystem;
std::string test_galaxy = "test_galaxy";
TEST(Galaxy, basic) {
    fs::remove_all(test_galaxy);
    auto galaxy = server::Galaxy::Open(test_galaxy, {});
    {
        auto graphDB = galaxy->OpenGraph("default");
        EXPECT_TRUE(graphDB != nullptr);
    }
    galaxy->CreateGraph("graph1");
    galaxy.reset(nullptr);
    galaxy = server::Galaxy::Open(test_galaxy, {});
    EXPECT_TRUE(galaxy->OpenGraph("default") != nullptr);
    EXPECT_TRUE(galaxy->OpenGraph("graph1") != nullptr);
    galaxy->DeleteGraph("graph1");
    EXPECT_THROW_CODE(galaxy->OpenGraph("graph1"), NoSuchGraph);
    galaxy.reset();
    galaxy = server::Galaxy::Open(test_galaxy, {});
    EXPECT_TRUE(galaxy->OpenGraph("default") != nullptr);
    EXPECT_THROW_CODE(galaxy->OpenGraph("graph1"), NoSuchGraph);
    galaxy.reset();
}

TEST(Galaxy, DISABLED_createGraph) {
    fs::remove_all(test_galaxy);
    auto galaxy = server::Galaxy::Open(test_galaxy, {});
    for (int i = 0; i < 10000; i++) {
        galaxy->CreateGraph("graph" + std::to_string(i));
    }
    std::this_thread::sleep_for(std::chrono::seconds(30));
}
