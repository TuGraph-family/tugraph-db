/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#if LGRAPH_USE_MOCK_KV
#include "core/mock_kv.h"
namespace lgraph {
typedef MockKvStore KvStore;
typedef MockKvIterator KvIterator;
typedef MockKvTransaction KvTransaction;
typedef MockKvTable KvTable;
}  // namespace lgraph
#else
#include "core/kv_store_mdb.h"
#endif
