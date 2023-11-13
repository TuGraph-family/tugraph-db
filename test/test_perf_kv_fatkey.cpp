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
#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/utils.h"
#include "fma-common/string_util.h"

#include "core/kv_store.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

class TestPerfKvFk : public TuGraphTest {};

using namespace lgraph;
using namespace fma_common;

static int power = 8;                  // power of nk
static int64_t nk = 1LL << power;      // number of keys
static int64_t txn_batch = 1LL << 10;  // txn batch
static int prop_size = 8;              // value size
static const int key_size = sizeof(int64_t) * 2;

static char prop[] = {/* 64 * 8 */
                      "0A0B0C0D0E0F0G0H0I0J0K0L0M0N0O0P0Q0R0S0T0U0V0W0X0Y0Z0a0b0c0d0e0f"
                      "1A1B1C1D1E1F1G1H1I1J1K1L1M1N1O1P1Q1R1S1T1U1V1W1X1Y1Z1a1b1c1d1e1f"
                      "2A2B2C2D2E2F2G2H2I2J2K2L2M2N2O2P2Q2R2S2T2U2V2W2X2Y2Z2a2b2c2d2e2f"
                      "3A3B3C3D3E3F3G3H3I3J3K3L3M3N3O3P3Q3R3S3T3U3V3W3X3Y3Z3a3b3c3d3e3f"
                      "4A4B4C4D4E4F4G4H4I4J4K4L4M4N4O4P4Q4R4S4T4U4V4W4X4Y4Z4a4b4c4d4e4f"
                      "5A5B5C5D5E5F5G5H5I5J5K5L5M5N5O5P5Q5R5S5T5U5V5W5X5Y5Z5a5b5c5d5e5f"
                      "6A6B6C6D6E6F6G6H6I6J6K6L6M6N6O6P6Q6R6S6T6U6V6W6X6Y6Z6a6b6c6d6e6f"
                      "7A7B7C7D7E7F7G7H7I7J7K7L7M7N7O7P7Q7R7S7T7U7V7W7X7Y7Z7a7b7c7d7e7f"};

static void DumpTable(KvTable& tab, KvTransaction& txn, int64_t b, int64_t e,
                      const std::function<std::string(const Value&)>& print_key,
                      const std::function<std::string(const Value&)>& print_value) {
    int64_t bk[2] = {b, 0};
    auto it = tab.GetIterator(txn, Value(bk, key_size) /*key*/);
    if (b < 0) {
        it = tab.GetIterator(txn);
        it->GotoFirstKey();
    }
    int count = 0;
    while (it->IsValid()) {
        std::string line;
        line = line + "[" + print_key(it->GetKey()) + "]: {" + print_value(it->GetValue());
        line += "}";
        UT_LOG() << line;
        it->Next();
        if (b >= 0 && ++count > e - b) break;
    }
}

static void DumpTable(KvTable& tab, KvTransaction& txn) {
    DumpTable(
        tab, txn, txn_batch / 2 - 1, txn_batch / 2 + 1,
        [](const Value& v) {
            int64_t* p = (int64_t*)v.Data();
            return ToString(*p) + " " + ToString(*(p + 1));
        },
        [](const Value& v) { return v.AsString().substr(0, 2) + "@" + ToString(v.Size()); });
}

static void InsertFidProp(void* fid, void* prop, int value_size, KvTable& tab, KvTransaction& txn) {
    tab.SetValue(txn, Value(fid, key_size), Value(prop, value_size));
}

int TestPerfKvFidProp(bool durable, int power, int prop_size) {
    UT_LOG() << "begin test " << __func__;
    nk = power > 31 ? 1LL << 31 : 1LL << power;
    prop_size = prop_size > 512 ? 512 : prop_size;
    txn_batch = txn_batch > nk ? nk : txn_batch;

    double t1, t2, t_add_kv;
    auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << 40, durable);
    // Start test, see if we already has a db
    auto txn = store->CreateWriteTxn();
    std::vector<std::string> tables = store->ListAllTables(*txn);
    UT_LOG() << "Current tables in the kvstore: " << ToString(tables);
    if (!tables.empty()) store->DropAll(*txn);
    txn->Commit();

    // now create tables
    txn = store->CreateWriteTxn();
    auto fid_prop =
        store->OpenTable(*txn, "fid_prop", true, ComparatorDesc::DefaultComparator());
    txn->Commit();

    // add kv
    int64_t id[2] = {0, 0};
    t1 = fma_common::GetTime();
    for (int k = 0; k < nk / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (int i = 0; i < txn_batch; i++) {
            id[0]++;
            InsertFidProp(id, prop, prop_size, *fid_prop, *txn);
        }
        txn->Commit();
    }
    t2 = fma_common::GetTime();
    t_add_kv = t2 - t1;
    UT_LOG() << "add kv done";
    txn = store->CreateReadTxn();
    DumpTable(*fid_prop, *txn);
    size_t mem_size = 0, height = 0;
    store->DumpStat(*txn, mem_size, height);
    UT_LOG() << "store DumpStat: (mem_size, height) = " << mem_size << ", " << height;
    txn->Abort();

    UT_LOG() << "(durable, nk, txn_batch, key_size, prop_size, log_space) = (" << durable << ", "
             << nk << ", " << txn_batch << ", " << key_size << ", " << prop_size << ", "
             << (key_size + prop_size) * nk << ")";
    UT_LOG() << "(t_add_kv) = (" << t_add_kv << ") s";
    UT_LOG() << "FOR STATISTICS: " << (double)(key_size + prop_size) * nk / (1 << 20) << ", "
             << (double)mem_size / (1 << 20) << ", "
             << "disk_size, " << t_add_kv;

    return 0;
}

static int my_key_cmp(const MDB_val* a, const MDB_val* b) {
    // ASSERT(a->mv_size == 8 && b->mv_size ==8);
    int64_t al = *(int64_t*)a->mv_data;
    int64_t bl = *(int64_t*)b->mv_data;
    int64_t ah = *((int64_t*)a->mv_data + 1);
    int64_t bh = *((int64_t*)b->mv_data + 1);
    return ah < bh ? -1 : ah > bh ? 1 : al < bl ? -1 : al > bl ? 1 : 0;
}

// static int neg_key_cmp(const MDB_val* a, const MDB_val* b) {
//     int ret = my_key_cmp(a, b);
//     return -ret;
// }

int TestPerfKvFidPropWithCmpFunc(bool durable, int power, int prop_size) {
    UT_LOG() << "begin test " << __func__;
    nk = power > 31 ? 1LL << 31 : 1LL << power;
    prop_size = prop_size > 512 ? 512 : prop_size;
    txn_batch = txn_batch > nk ? nk : txn_batch;

    double t1, t2, t_add_kv;
    auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << 40, durable);
    // Start test, see if we already has a db
    auto txn = store->CreateWriteTxn();
    std::vector<std::string> tables = store->ListAllTables(*txn);
    UT_LOG() << "Current tables in the kvstore: " << ToString(tables);
    if (!tables.empty()) store->DropAll(*txn);
    txn->Commit();

    // now create tables
    KeySortFunc key_cmp;
    key_cmp = my_key_cmp;
    // key_cmp = neg_key_cmp;
    txn = store->CreateWriteTxn();
    auto fid_prop = store->_OpenTable_(*txn, "fid_prop", true, key_cmp);
    txn->Commit();

    // add kv
    int64_t id[2] = {0, 0};
    t1 = fma_common::GetTime();
    for (int k = 0; k < nk / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (int i = 0; i < txn_batch; i++) {
            id[0]++;
            InsertFidProp(id, prop, prop_size, *fid_prop, *txn);
        }
        txn->Commit();
    }
    t2 = fma_common::GetTime();
    t_add_kv = t2 - t1;
    UT_LOG() << "add kv done";
    txn = store->CreateReadTxn();
    DumpTable(*fid_prop, *txn);
    size_t mem_size = 0, height = 0;
    store->DumpStat(*txn, mem_size, height);
    UT_LOG() << "store DumpStat: (mem_size, height) = " << mem_size << ", " << height;
    txn->Abort();

    UT_LOG() << "(durable, nk, txn_batch, key_size, prop_size, log_space) = (" << durable << ", "
             << nk << ", " << txn_batch << ", " << key_size << ", " << prop_size << ", "
             << (key_size + prop_size) * nk << ")";
    UT_LOG() << "(t_add_kv) = (" << t_add_kv << ") s";
    UT_LOG() << "FOR STATISTICS: " << (double)(key_size + prop_size) * nk / (1 << 20) << ", "
             << (double)mem_size / (1 << 20) << ", "
             << "disk_size, " << t_add_kv;

    return 0;
}

int TestPerfKvFidPropWithCmpFuncShuffle(bool durable, int power, int prop_size) {
    UT_LOG() << "begin test " << __func__;
    nk = power > 31 ? 1LL << 31 : 1LL << power;
    prop_size = prop_size > 512 ? 512 : prop_size;
    txn_batch = txn_batch > nk ? nk : txn_batch;

    // gen shuffle id
    std::vector<int64_t> shuffle_id(nk);
    for (int i = 0; i < nk; i++) shuffle_id[i] = i + 1;
    // randomly shuffle the ids using std::shuffle
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffle_id.begin(), shuffle_id.end(), g);

    double t1, t2, t_add_kv;
    auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << 40, durable);
    // Start test, see if we already has a db
    auto txn = store->CreateWriteTxn();
    std::vector<std::string> tables = store->ListAllTables(*txn);
    UT_LOG() << "Current tables in the kvstore: " << ToString(tables);
    if (!tables.empty()) store->DropAll(*txn);
    txn->Commit();

    // now create tables
    KeySortFunc key_cmp = my_key_cmp;
    txn = store->CreateWriteTxn();
    auto fid_prop = store->_OpenTable_(*txn, "fid_prop", true, key_cmp);
    txn->Commit();

    // add kv
    int64_t count = 0;
    int64_t id[2] = {0, 0};
    t1 = fma_common::GetTime();
    for (int k = 0; k < nk / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (int i = 0; i < txn_batch; i++) {
            id[0] = shuffle_id[count++];
            InsertFidProp(id, prop, prop_size, *fid_prop, *txn);
        }
        txn->Commit();
    }
    t2 = fma_common::GetTime();
    t_add_kv = t2 - t1;
    UT_LOG() << "add kv done";
    txn = store->CreateReadTxn();
    DumpTable(*fid_prop, *txn);
    size_t mem_size = 0, height = 0;
    store->DumpStat(*txn, mem_size, height);
    UT_LOG() << "store DumpStat: (mem_size, height) = " << mem_size << ", " << height;
    txn->Abort();

    UT_LOG() << "(durable, nk, txn_batch, key_size, prop_size, log_space) = (" << durable << ", "
             << nk << ", " << txn_batch << ", " << key_size << ", " << prop_size << ", "
             << (key_size + prop_size) * nk << ")";
    UT_LOG() << "(t_add_kv) = (" << t_add_kv << ") s";
    UT_LOG() << "FOR STATISTICS: " << (double)(key_size + prop_size) * nk / (1 << 20) << ", "
             << (double)mem_size / (1 << 20) << ", "
             << "disk_size, " << t_add_kv;

    return 0;
}

TEST_F(TestPerfKvFk, PerfKvFk) {
    int test_case = 1;
    bool durable = false;
    if (test_case & 0x1)
        TestPerfKvFidProp(durable, power, prop_size);
    else if (test_case & 0x2)
        TestPerfKvFidPropWithCmpFunc(durable, power, prop_size);
    else if (test_case & 0x4)
        TestPerfKvFidPropWithCmpFuncShuffle(durable, power, prop_size);
}
