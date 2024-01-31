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
#include "core/lmdb_store.h"
#endif
