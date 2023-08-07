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

#include "fma-common/utils.h"
#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"
#include "fma-common/file_system.h"
#include "gtest/gtest.h"

#include "core/lmdb/lmdb.h"
#include "./random_port.h"
#include "./test_tools.h"
#include "./ut_utils.h"

#define CHECK_MDB(stmt)                                                        \
    do {                                                                       \
        int ec = (stmt);                                                       \
        if (ec != MDB_SUCCESS) UT_ERR() << "MDB_ERROR: " << mdb_strerror(ec); \
    } while (0)

class TestMDB : public TuGraphTest {};

std::string PrintStat(const MDB_stat &stat) {
    return fma_common::StringFormatter::Format(
        "\t\tdepth={}, \tentries={}, \tbr_page={}, \tlf_page={}, \tofpage={}", stat.ms_depth,
        stat.ms_entries, stat.ms_branch_pages, stat.ms_leaf_pages, stat.ms_overflow_pages);
}

static std::string GetAsString(const MDB_val &v) {
    return std::string((const char *)v.mv_data, v.mv_size);
}

TEST_F(TestMDB, MdbTool) {
    using namespace fma_common;
    std::string path = "./db";
    auto src = {"create", "replace", "stat", "delete"};
    int nk = 1 << 20;
    size_t v_size = 20;
    int percent = 10;
    int random_seed = 0;
    size_t batch_size = 2000;

    {
        using namespace lgraph;
        AutoCleanDir cleaner(path);
        MDB_env *env;
        CHECK_MDB(mdb_env_create(&env));
        CHECK_MDB(mdb_env_set_mapsize(env, (size_t)1 << 30));
        CHECK_MDB(mdb_env_set_maxdbs(env, 255));
        unsigned int flags = MDB_NOMEMINIT | MDB_NORDAHEAD | MDB_NOSYNC;
        CHECK_MDB(mdb_env_open(env, path.c_str(), flags, 0664));

        MDB_txn *txn;
        CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_NOSYNC, &txn));
        MDB_dbi dbi;
        CHECK_MDB(mdb_dbi_open(txn, "db1", MDB_CREATE, &dbi));
        mdb_txn_abort(txn);

        CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_NOSYNC, &txn));
        CHECK_MDB(mdb_dbi_open(txn, "db1", MDB_CREATE, &dbi));
        MDB_cursor *cursor;
        CHECK_MDB(mdb_cursor_open(txn, dbi, &cursor));
        MDB_val key, value;
        int r = mdb_cursor_get(cursor, &key, &value, MDB_FIRST);
        UT_EXPECT_TRUE(r == MDB_SUCCESS || r == MDB_NOTFOUND);
        mdb_txn_abort(txn);
        mdb_env_close(env);
    }
    lgraph::AutoCleanDir cleaner(path);
    for (auto act : src) {
        if (strcmp(act, "create") == 0) {
            MDB_env *env;
            CHECK_MDB(mdb_env_create(&env));
            CHECK_MDB(mdb_env_set_mapsize(env, (size_t)1 << 40));
            CHECK_MDB(mdb_env_set_maxdbs(env, 255));
            unsigned int flags = MDB_NOMEMINIT | MDB_NORDAHEAD | MDB_NOSYNC;
            CHECK_MDB(mdb_env_open(env, path.c_str(), flags, 0664));

            MDB_txn *txn;
            CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_NOSYNC, &txn));
            MDB_dbi dbi;
            CHECK_MDB(mdb_dbi_open(txn, "db1", MDB_INTEGERKEY | MDB_CREATE, &dbi));
            MDB_cursor *cursor;
            CHECK_MDB(mdb_cursor_open(txn, dbi, &cursor));
            MDB_val key, value;
            mdb_cursor_get(cursor, &key, &value, MDB_FIRST);
            {
                std::string k1 = "key1";
                std::string v1 = "value1";
                key.mv_data = (void *)k1.data();
                key.mv_size = k1.size();
                value.mv_data = (void *)v1.data();
                value.mv_size = v1.size();
                CHECK_MDB(mdb_cursor_put(cursor, &key, &value, MDB_APPEND));
                std::string k2 = "key2";
                std::string v2 = "value2";
                key.mv_data = (void *)k2.data();
                key.mv_size = k2.size();
                value.mv_data = (void *)v2.data();
                value.mv_size = v2.size();
                CHECK_MDB(mdb_cursor_put(cursor, &key, &value, MDB_APPEND));
                MDB_cursor *c;
                CHECK_MDB(mdb_cursor_open(txn, dbi, &c));
                MDB_val ok, ov;
                mdb_cursor_get(c, &ok, &ov, MDB_FIRST);
                UT_EXPECT_EQ(GetAsString(ok), "key1");
                UT_EXPECT_EQ(GetAsString(ov), "value1");
                CHECK_MDB(mdb_cursor_get(cursor, &key, &value, MDB_FIRST));
                UT_EXPECT_EQ(GetAsString(key), "key1");
                UT_EXPECT_EQ(GetAsString(value), "value1");
                CHECK_MDB(mdb_cursor_del(cursor, 0));
                CHECK_MDB(mdb_cursor_get(c, &ok, &ov, MDB_GET_CURRENT));
                UT_EXPECT_EQ(GetAsString(ok), "key2");
                UT_EXPECT_EQ(GetAsString(ov), "value2");
                CHECK_MDB(mdb_cursor_del(c, 0));
            }
            std::string vbuf(v_size, 'v');
            value.mv_size = v_size;
            value.mv_data = (void *)vbuf.data();
            for (int i = 0; i < nk; i++) {
                key.mv_data = &i;
                key.mv_size = sizeof(i);
                CHECK_MDB(mdb_cursor_put(cursor, &key, &value, MDB_APPEND));
            }
            mdb_cursor_close(cursor);
            CHECK_MDB(mdb_txn_commit(txn));
            mdb_env_close(env);
        } else if (strcmp(act, "replace") == 0) {
            MDB_env *env;
            CHECK_MDB(mdb_env_create(&env));
            CHECK_MDB(mdb_env_set_mapsize(env, (size_t)1 << 40));
            CHECK_MDB(mdb_env_set_maxdbs(env, 255));
            unsigned int flags = MDB_NOMEMINIT | MDB_NORDAHEAD | MDB_NOSYNC;
            CHECK_MDB(mdb_env_open(env, path.c_str(), flags, 0664));

            srand(random_seed);
            MDB_txn *txn;
            CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_NOSYNC, &txn));
            MDB_dbi dbi;
            CHECK_MDB(mdb_dbi_open(txn, "db1", MDB_INTEGERKEY | MDB_CREATE, &dbi));
            MDB_val key, value;
            // get max id
            {
                MDB_cursor *cursor;
                CHECK_MDB(mdb_cursor_open(txn, dbi, &cursor));
                CHECK_MDB(mdb_cursor_get(cursor, &key, &value, MDB_LAST));
                CHECK_MDB(mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT));
                UT_EXPECT_EQ(key.mv_size, sizeof(int));
                nk = *(int *)key.mv_data;
                mdb_cursor_close(cursor);
            }
            // replace
            size_t current_batch = 0;
            for (int i = 0; i < nk * percent / 100; i++) {
                int k = myrand() % nk;
                key.mv_data = &k;
                key.mv_size = sizeof(k);

                CHECK_MDB(mdb_del(txn, dbi, &key, &value));
                CHECK_MDB(mdb_put(txn, dbi, &key, &value, MDB_NOOVERWRITE));
                current_batch++;
                if (current_batch >= batch_size) {
                    CHECK_MDB(mdb_txn_commit(txn));
                    CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_NOSYNC, &txn));
                    UT_LOG() << "Committed a batch of " << current_batch;
                    current_batch = 0;
                }
            }
            CHECK_MDB(mdb_txn_commit(txn));
            mdb_env_close(env);
        } else if (strcmp(act, "delete") == 0) {
            MDB_env *env;
            CHECK_MDB(mdb_env_create(&env));
            CHECK_MDB(mdb_env_set_mapsize(env, (size_t)1 << 40));
            CHECK_MDB(mdb_env_set_maxdbs(env, 255));
            unsigned int flags = MDB_NOMEMINIT | MDB_NORDAHEAD | MDB_NOSYNC;
            CHECK_MDB(mdb_env_open(env, path.c_str(), flags, 0664));

            srand(random_seed);
            MDB_txn *txn;
            CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_NOSYNC, &txn));
            MDB_dbi dbi;
            CHECK_MDB(mdb_dbi_open(txn, "db1", MDB_INTEGERKEY | MDB_CREATE, &dbi));
            MDB_val key, value;
            // get max id
            {
                MDB_cursor *cursor;
                CHECK_MDB(mdb_cursor_open(txn, dbi, &cursor));
                CHECK_MDB(mdb_cursor_get(cursor, &key, &value, MDB_LAST));
                CHECK_MDB(mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT));
                UT_EXPECT_EQ(key.mv_size, sizeof(int));
                nk = *(int *)key.mv_data;
                mdb_cursor_close(cursor);
            }
            // delete
            size_t current_batch = 0;
            for (int i = 0; i < nk * percent / 100; i++) {
                int k = myrand() % nk;
                key.mv_data = &k;
                key.mv_size = sizeof(k);

                mdb_del(txn, dbi, &key, &value);
                current_batch++;
                if (current_batch >= batch_size) {
                    CHECK_MDB(mdb_txn_commit(txn));
                    CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_NOSYNC, &txn));
                    UT_LOG() << "Committed a batch of " << current_batch;
                    current_batch = 0;
                }
            }
            CHECK_MDB(mdb_txn_commit(txn));
            mdb_env_close(env);
        } else if (strcmp(act, "stat") == 0) {
            MDB_env *env;
            CHECK_MDB(mdb_env_create(&env));
            CHECK_MDB(mdb_env_set_mapsize(env, (size_t)1 << 40));
            CHECK_MDB(mdb_env_set_maxdbs(env, 255));
            unsigned int flags = MDB_NOMEMINIT | MDB_NORDAHEAD | MDB_NOSYNC;
            CHECK_MDB(mdb_env_open(env, path.c_str(), flags, 0664));

            MDB_txn *txn;
            CHECK_MDB(mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn));
            MDB_dbi dbi;
            CHECK_MDB(mdb_dbi_open(txn, nullptr, 0, &dbi));
            MDB_stat stat;
            CHECK_MDB(mdb_stat(txn, dbi, &stat));
            UT_LOG() << "root: \t" << PrintStat(stat);
            size_t total_pages = 0;
            total_pages += stat.ms_branch_pages;
            total_pages += stat.ms_leaf_pages;
            MDB_cursor *cursor;
            CHECK_MDB(mdb_cursor_open(txn, dbi, &cursor));
            MDB_val key, value;
            CHECK_MDB(mdb_cursor_get(cursor, &key, &value, MDB_FIRST));
            while (mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT) == MDB_SUCCESS) {
                MDB_dbi dbi;
                std::string name((const char *)key.mv_data, key.mv_size);
                CHECK_MDB(mdb_dbi_open(txn, name.c_str(), 0, &dbi));
                MDB_stat stat;
                CHECK_MDB(mdb_stat(txn, dbi, &stat));
                UT_LOG() << name << ": \t" << PrintStat(stat);
                total_pages += stat.ms_branch_pages;
                total_pages += stat.ms_leaf_pages;
                if (mdb_cursor_get(cursor, &key, &value, MDB_NEXT) != MDB_SUCCESS) break;
            }
            mdb_cursor_close(cursor);
            mdb_txn_abort(txn);
            mdb_env_close(env);
            UT_LOG() << "Number of pages: " << total_pages;
        }
    }
}
