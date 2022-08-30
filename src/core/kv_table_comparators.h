/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
