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

#pragma once

#include <string>
#include <unordered_map>

#include "fma-common/text_parser.h"
#include "lgraph/lgraph_types.h"

#if defined(_WIN32) || defined(_WIN64)
#define LITTLE_ENDIAN 1234
#define BYTE_ORDER LITTLE_ENDIAN
#elif __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#ifdef _WIN32
#define LGRAPH_PSORT(...) std::stable_sort(__VA_ARGS__)
#elif __clang__
#include <experimental/algorithm>
#define LGRAPH_PSORT(...) std::stable_sort(__VA_ARGS__)
#else
#include <parallel/algorithm>
#include <lgraph/lgraph_types.h>  // NOLINT

#define LGRAPH_PSORT(...) __gnu_parallel::stable_sort(__VA_ARGS__)
#endif
#include "core/version.h"

#ifndef GIT_BRANCH
#define GIT_BRANCH "unknown"
#endif

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

#ifndef WEB_GIT_COMMIT_HASH
#define WEB_GIT_COMMIT_HASH "unknown"
#endif

#ifndef CXX_COMPILER_ID
#define CXX_COMPILER_ID "unknown"
#endif

#ifndef CXX_COMPILER_VERSION
#define CXX_COMPILER_VERSION "unknown"
#endif

#ifndef PYTHON_LIB_VERSION
#define PYTHON_LIB_VERSION "unknown"
#endif

#ifndef LGRAPH_PYTHON_PLUGIN_LIFETIME_S
#define LGRAPH_PYTHON_PLUGIN_LIFETIME_S 12 * 60 * 60
#endif

namespace lgraph {
/** internal tables in the db
 */
namespace _detail {
static const char* const DEFAULT_ADMIN_NAME = "admin";
static const char* const DEFAULT_ADMIN_PASS = "73@TuGraph";
static const char* const ADMIN_ROLE = "admin";
static const char* const ADMIN_ROLE_DESC = "Administrators. Builtin role, cannot be modified.";
static const char* const META_GRAPH = "@meta_graph@";
static const char* const BUILTIN_AUTH = "@builtin_auth@";
static const char* const PASSWORD_MD5_SALT = "We love salt.";
static const char* const DEFAULT_GRAPH_DB_NAME = "default";
// table names used in core
static const char* const META_TABLE = "_meta_";                  // stores version numbers
static const char* const BLOB_TABLE = "_blob_";                  // stores the blobs
static const char* const IP_WHITELIST_TABLE = "_ip_whitelist_";  // ip whitelist table
static const char* const GRAPH_TABLE = "_graph_";                // vid -> edge1, edge2, ...
static const char* const V_SCHEMA_TABLE = "_v_schema_";          // label -> schema
static const char* const E_SCHEMA_TABLE = "_e_schema_";          // label -> schema
static const char* const INDEX_TABLE = "_v_index_";              // index_name -> index properties
static const char* const PARTIAL_INDEX_TABLE = "_partial_index_";
static const char* const CPP_PLUGIN_TABLE = "_cpp_plugin_";        // plugin_name -> plugin_info
static const char* const PYTHON_PLUGIN_TABLE = "_python_plugin_";  // plugin_name -> plugin_info
static const char* const CPP_PLUGIN_DIR = "_cpp_plugin_";
static const char* const PYTHON_PLUGIN_DIR = "_python_plugin_";
static const char* const FULLTEXT_INDEX_DIR = "_fulltext_index_";
static const char* const NAME_SEPERATOR = "_@lgraph@_";
static const char* const VERTEX_FULLTEXT_INDEX = "vertex_fulltext";
static const char* const EDGE_FULLTEXT_INDEX = "edge_fulltext";
static const char* const VERTEX_INDEX = "vertex_index";
static const char* const EDGE_INDEX = "edge_index";
static const char* const USER_TABLE_NAME = "_user_table_";
static const char* const ROLE_TABLE_NAME = "_role_table_";
static const char* const GRAPH_CONFIG_TABLE_NAME = "_graph_config_table_";

// dynamic options that can be changed online
static const char* const OPT_TXN_OPTIMISTIC = "optimistic_txn";
static const char* const OPT_AUDIT_LOG_ENABLE = "enable_audit_log";
static const char* const OPT_DB_DURABLE = "durable";
static const char* const OPT_IP_CHECK_ENABLE = "enable_ip_check";
static const char* const RAFT_LOG_IDX = "_rlog_idx_";  // key to store raft log index in db

// keys to store version info in db
static const char* const VER_MAJOR_KEY = "ver_major";
static const char* const VER_MINOR_KEY = "ver_minor";
static const char* const VER_PATCH_KEY = "ver_patch";

// lgraph meta table keys
static const char* const DB_SECRET_KEY = "_db_secret_";  // key to store db secret in meta table
static const char* const NEXT_VID_KEY = "_next_vid_";    // key to store next vid in meta table

// real-time count of vertex and edge
static const char* const VERTEX_COUNT_PREFIX = "_vertex_count_";
static const char* const EDGE_COUNT_PREFIX = "_edge_count_";

// store property data in a separate kv table
static const char* const VERTEX_PROPERTY_TABLE_PREFIX = "_vertex_property_";
static const char* const EDGE_PROPERTY_TABLE_PREFIX = "_edge_property_";

// version info
static const int VER_MAJOR = LGRAPH_VERSION_MAJOR;
static const int VER_MINOR = LGRAPH_VERSION_MINOR;
static const int VER_PATCH = LGRAPH_VERSION_PATCH;

// limits
static const size_t MAX_NUM_FIELDS = 1024;  // max number of fields in vertex/edge property
static const size_t MAX_COMPILE_TIME_MS = 1000 * 1000;  // max compile time when loading plugin
static const size_t MAX_UNZIP_TIME_MS = 100 * 1000;     // max unzip time when loading plugin

static const size_t MAX_NUM_USERS = 65536;
static const size_t MAX_NUM_GRAPHS = 4096;
#ifdef _WIN32
static const size_t DEFAULT_GRAPH_SIZE = (size_t)1 << 30;
static const size_t MAX_GRAPH_SIZE = (size_t)1 << 40;
#else
static const size_t DEFAULT_GRAPH_SIZE = (size_t)1 << 42;  // 4TB
static const size_t MAX_GRAPH_SIZE = (size_t)1 << 44;
#endif
static const size_t GRAPH_SUBDIR_NAME_LEN = 8;

static const size_t MAX_NAME_LEN = 64;
static const size_t MAX_PASSWORD_LEN = 64;
static const size_t MAX_DESC_LEN = 512;

static const size_t DEFAULT_MEM_LIMIT = (size_t)2 << 40;
}  // namespace _detail

namespace plugin {
enum class Type { CPP = 1, PYTHON = 2, JAVA = 3, ANY = 4 };
static const char* const PLUGIN_VERSION_1 = "v1";
static const char* const PLUGIN_VERSION_2 = "v2";
static const char* const PLUGIN_VERSION_ANY = "any";
static const char* const PLUGIN_LANG_TYPE_CPP = "cpp";
static const char* const PLUGIN_LANG_TYPE_PYTHON = "python";
static const char* const PLUGIN_LANG_TYPE_JAVA = "java";
static const char* const PLUGIN_LANG_TYPE_ANY = "any";
static const char* const PLUGIN_CODE_TYPE_CPP = "cpp";
static const char* const PLUGIN_CODE_TYPE_SO = "so";
static const char* const PLUGIN_CODE_TYPE_ZIP = "zip";
static const char* const PLUGIN_CODE_TYPE_PY = "py";

typedef ::lgraph_api::PluginCodeType CodeType;
}  // namespace plugin

// check if name is a valid user name
// User names must be of length (0, 64].
inline bool IsValidLGraphName(const std::string& name) {
    if (name.empty() || name.size() > _detail::MAX_NAME_LEN) return false;
    if (fma_common::TextParserUtils::IsDigits(name.front())) return false;
    for (auto& c : name) {
        if ((uint8_t)c < 128 && !fma_common::TextParserUtils::IsValidNameCharacter(c)) return false;
    }
    return true;
}

inline bool IsValidUserName(const std::string& name) { return IsValidLGraphName(name); }

inline bool IsValidGraphName(const std::string& name) { return IsValidLGraphName(name); }

// check if pass is a valid password
// Passwords must be of length (0, 64].
inline bool IsValidPassword(const std::string& pass) {
    return !pass.empty() && pass.size() <= _detail::MAX_PASSWORD_LEN;
}
}  // namespace lgraph
