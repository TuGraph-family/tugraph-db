/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
