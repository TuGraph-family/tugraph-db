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

/*
 * written by botu.wzy
 */

#pragma once
#include <gflags/gflags.h>

DECLARE_string(mode);
DECLARE_string(data_path);
DECLARE_string(pid_file);

DECLARE_string(log_path);
DECLARE_string(log_level);
DECLARE_uint64(log_max_size);
DECLARE_uint32(log_max_files);

DECLARE_bool(enable_query_log);
DECLARE_string(query_log_path);
DECLARE_uint64(query_log_max_size);
DECLARE_uint32(query_log_max_files);

DECLARE_uint32(log_flush_interval);

DECLARE_string(host);
DECLARE_uint32(bolt_port);
DECLARE_uint32(bolt_io_thread_num);

DECLARE_uint64(block_cache);
DECLARE_uint64(row_cache);
DECLARE_uint64(ft_commit_interval);