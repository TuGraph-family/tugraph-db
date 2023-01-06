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

//
// Created by wt on 2019/12/12.
//
#pragma once

namespace cypher {

struct ResultSetStatistics {
    int labels_added = 0;     /* Number of labels added as part of a create query. */
    int vertices_created = 0; /* Number of nodes created as part of a create query. */
    int properties_set = 0;   /* Number of properties created as part of a create query. */
    int edges_created = 0;    /* Number of edges created as part of a create query. */
    int vertices_deleted = 0; /* Number of nodes removed as part of a delete query.*/
    int edges_deleted = 0;    /* Number of edges removed as part of a delete query.*/

    bool Modified() const {
        return labels_added > 0 || vertices_created > 0 || properties_set > 0 ||
               edges_created > 0 || vertices_deleted > 0 || edges_deleted > 0;
    }
};

}  // namespace cypher
