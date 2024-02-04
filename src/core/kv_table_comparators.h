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

#include "core/data_type.h"

namespace lgraph {

typedef int (*KeySortFunc)(const MDB_val*, const MDB_val*);

struct ComparatorDesc {
    enum Type {
        BYTE_SEQ = 0,
        SINGLE_TYPE = 1,
        TYPE_AND_VID = 2,
        TYPE_AND_EUID = 3,
        GRAPH_KEY = 4,
        BOTH_SIDE_VID = 5,
        INVALID_TYPE = 255
    };

    Type comp_type = Type::BYTE_SEQ;
    FieldType data_type;

    static ComparatorDesc SingleDataComp(FieldType ft) {
        ComparatorDesc desc{Type::SINGLE_TYPE, ft};
        return desc;
    }

    static const ComparatorDesc& DefaultComparator() {
        static ComparatorDesc desc{Type::BYTE_SEQ, FieldType::NUL};
        return desc;
    }
};

KeySortFunc GetKeyComparator(const ComparatorDesc& desc);
}  // namespace lgraph
