/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 6/15/18.
//

#include "iterator.h"

int64_t lgraph::EIter::HashId() const {
    EuidHash th;
    return th(GetUid());
}
