﻿/**
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
// Created by wt on 6/14/18.
//
#pragma once

#include "op_all_node_scan.h"
#include "op_node_by_label_scan.h"
#include "op_node_index_seek.h"
#include "op_produce_results.h"
#include "op_filter.h"
#include "op_expand_all.h"
#include "op_create.h"
#include "op_standalone_call.h"
#include "op_var_len_expand.h"
#include "op_var_len_expand_into.h"
#include "op_inquery_call.h"
#include "op_aggregate.h"
#include "op_project.h"
#include "op_sort.h"
#include "op_limit.h"
#include "op_cartesian_product.h"
#include "op_set.h"
#include "op_delete.h"
#include "op_optional.h"
#include "op_argument.h"
#include "op_apply.h"
#include "op_unwind.h"
#include "op_node_index_seek_dynamic.h"
#include "op_relationship_count.h"
#include "op_distinct.h"
#include "op_merge.h"
#include "op_remove.h"
#include "op_skip.h"
#include "op_all_node_scan_dynamic.h"
#include "op_node_by_label_scan_dynamic.h"
#include "op_topn.h"
#include "op_union.h"
#include "op_node_by_id_seek.h"
#include "op_traversal.h"
