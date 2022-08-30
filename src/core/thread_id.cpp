/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "core/thread_id.h"

namespace lgraph {
thread_local ThreadIdFetcher ThreadLocalId::id_;
}
