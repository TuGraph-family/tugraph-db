//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#ifdef _WIN32
#ifdef LGRAPH_DLL
#ifdef LGRAPH_LIBRARY_EXPORTS
#define LGRAPH_LIBRARY_API __declspec(dllexport)
#else
#define LGRAPH_LIBRARY_API __declspec(dllimport)
#endif
#else
#define LGRAPH_LIBRARY_API
#endif
#else
#define LGRAPH_LIBRARY_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct lgraph_api_date_t lgraph_api_date_t;
typedef struct lgraph_api_date_time_t lgraph_api_date_time_t;
typedef struct lgraph_api_field_data_t lgraph_api_field_data_t;
typedef struct lgraph_api_field_spec_t lgraph_api_field_spec_t;
typedef struct lgraph_api_index_spec_t lgraph_api_index_spec_t;
typedef struct lgraph_api_edge_uid_t lgraph_api_edge_uid_t;
typedef struct lgraph_api_user_info_t lgraph_api_user_info_t;
typedef struct lgraph_api_role_info_t lgraph_api_role_info_t;
typedef struct lgraph_api_out_edge_iterator_t lgraph_api_out_edge_iterator_t;
typedef struct lgraph_api_in_edge_iterator_t lgraph_api_in_edge_iterator_t;
typedef struct lgraph_api_vertex_iterator_t lgraph_api_vertex_iterator_t;
typedef struct lgraph_api_edge_index_iterator_t lgraph_api_edge_index_iterator_t;
typedef struct lgraph_api_vertex_index_iterator_t lgraph_api_vertex_index_iterator_t;
typedef struct lgraph_api_transaction_t lgraph_api_transaction_t;
typedef struct lgraph_api_graph_db_t lgraph_api_graph_db_t;
typedef struct lgraph_api_galaxy_t lgraph_api_galaxy_t;

/* DateTime(not exhausted)*/
extern LGRAPH_LIBRARY_API lgraph_api_date_time_t* lgraph_api_create_date_time();
extern LGRAPH_LIBRARY_API lgraph_api_date_time_t* lgraph_api_create_date_time_ymdhms(
    int year, unsigned month, unsigned day, unsigned hour, unsigned minute, unsigned second);
extern LGRAPH_LIBRARY_API lgraph_api_date_time_t* lgraph_api_create_date_time_seconds(
    int64_t seconds_since_epoch);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_date_time_seconds_since_epoch(lgraph_api_date_time_t* dt);
extern LGRAPH_LIBRARY_API void lgraph_api_date_time_destroy(lgraph_api_date_time_t* dt);

/* Date(not exhausted) */
extern LGRAPH_LIBRARY_API lgraph_api_date_t* lgraph_api_create_date();
extern LGRAPH_LIBRARY_API lgraph_api_date_t* lgraph_api_create_date_ymd(int year, unsigned month,
                                                                        unsigned day);
extern LGRAPH_LIBRARY_API lgraph_api_date_t* lgraph_api_create_date_days(int32_t day);
extern LGRAPH_LIBRARY_API int32_t lgraph_api_date_days_since_epoch(lgraph_api_date_t* date);
extern LGRAPH_LIBRARY_API void lgraph_api_date_destroy(lgraph_api_date_t* dt);

/* FieldData */
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data();
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_bool(bool v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_int8(int8_t v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_int16(int16_t v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_int32(int32_t v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_int64(int64_t v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_float(float v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_double(double v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_date(
    lgraph_api_date_t* v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_date_time(
    lgraph_api_date_time_t* v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_str(const char* v);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_str_len(
    const char* v, size_t len);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_blob(
    const uint8_t* v, size_t len);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_base64_blob(
    const uint8_t* v, size_t len);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_create_field_data_clone(
    lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API void lgraph_api_create_field_data_clone_from(
    lgraph_api_field_data_t* fd, lgraph_api_field_data_t* other_fd);
extern LGRAPH_LIBRARY_API void lgraph_api_field_data_destroy(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_field_data_integer(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API double lgraph_api_field_data_real(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API const char* lgraph_api_field_data_str(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_as_bool(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API int8_t lgraph_api_field_data_as_int8(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API int16_t lgraph_api_field_data_as_int16(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API int32_t lgraph_api_field_data_as_int32(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_field_data_as_int64(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API float lgraph_api_field_data_as_float(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API double lgraph_api_field_data_as_double(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API lgraph_api_date_t* lgraph_api_field_data_as_date(
    lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API lgraph_api_date_time_t* lgraph_api_field_data_as_date_time(
    lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API char* lgraph_api_field_data_as_str(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API char* lgraph_api_field_data_as_blob(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API char* lgrpah_api_field_data_as_blob_base64(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API char* lgraph_api_field_data_to_string(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_eq(const lgraph_api_field_data_t* fd,
                                                        const lgraph_api_field_data_t* other_fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_not_eq(
    const lgraph_api_field_data_t* fd, const lgraph_api_field_data_t* other_fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_greater_than(
    const lgraph_api_field_data_t* fd, const lgraph_api_field_data_t* other_fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_greater_eq_than(
    const lgraph_api_field_data_t* fd, const lgraph_api_field_data_t* other_fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_less_than(
    const lgraph_api_field_data_t* fd, const lgraph_api_field_data_t* other_fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_less_eq_than(
    const lgraph_api_field_data_t* fd, const lgraph_api_field_data_t* other_fd);
enum {
    lgraph_api_field_type_null = 0,
    lgraph_api_field_type_bool = 1,
    lgraph_api_field_type_int8 = 2,
    lgraph_api_field_type_int16 = 3,
    lgraph_api_field_type_int32 = 4,
    lgraph_api_field_type_int64 = 5,
    lgraph_api_field_type_float = 6,
    lgraph_api_field_type_double = 7,
    lgraph_api_field_type_date = 8,
    lgraph_api_field_type_datetime = 9,
    lgraph_api_field_type_string = 10,
    lgraph_api_field_type_blob = 11,
};
extern LGRAPH_LIBRARY_API int lgraph_api_field_data_get_type(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_null(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_buf(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_empty_buf(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_bool(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_blob(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_string(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_int8(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_int16(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_int32(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_int64(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_float(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_double(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_real(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_date(lgraph_api_field_data_t* fd);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_data_is_date_time(lgraph_api_field_data_t* fd);

/* FieldSpec */
extern LGRAPH_LIBRARY_API lgraph_api_field_spec_t* lgraph_api_create_field_spec();
extern LGRAPH_LIBRARY_API lgraph_api_field_spec_t* lgraph_api_create_field_spec_name_type_optional(
    const char* name, int type, bool optional);
extern LGRAPH_LIBRARY_API void lgraph_api_field_spec_destroy(lgraph_api_field_spec_t* fs);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_spec_eq(const lgraph_api_field_spec_t* fs,
                                                        const lgraph_api_field_spec_t* other_fs);
extern LGRAPH_LIBRARY_API char* lgraph_api_field_spec_to_string(lgraph_api_field_spec_t* fs);
extern LGRAPH_LIBRARY_API const char* lgraph_api_field_spec_get_name(lgraph_api_field_spec_t* fs);
extern LGRAPH_LIBRARY_API void lgraph_api_field_spec_set_name(lgraph_api_field_spec_t* fs,
                                                              const char* name);
extern LGRAPH_LIBRARY_API int lgraph_api_field_spec_get_type(lgraph_api_field_spec_t* fs);
extern LGRAPH_LIBRARY_API void lgraph_api_field_spec_set_type(lgraph_api_field_spec_t* fs,
                                                              int type);
extern LGRAPH_LIBRARY_API bool lgraph_api_field_spec_get_optional(lgraph_api_field_spec_t* fs);
extern LGRAPH_LIBRARY_API void lgraph_api_field_spec_set_optional(lgraph_api_field_spec_t* fs,
                                                                  bool optional);
enum {
    lgraph_api_normal_index_type = 0,
    lgraph_api_global_unique_index_type = 1,
    lgraph_api_pair_unique_index_type = 2
};
/* IndexSpec */
extern LGRAPH_LIBRARY_API lgraph_api_index_spec_t* lgraph_api_create_index_spec();
extern LGRAPH_LIBRARY_API void lgraph_api_index_spec_destroy(lgraph_api_index_spec_t* is);
extern LGRAPH_LIBRARY_API const char* lgraph_api_index_spec_get_label(lgraph_api_index_spec_t* is);
extern LGRAPH_LIBRARY_API const char* lgraph_api_index_spec_get_field(lgraph_api_index_spec_t* is);
extern LGRAPH_LIBRARY_API bool lgraph_api_index_spec_get_unique(lgraph_api_index_spec_t* is);
extern LGRAPH_LIBRARY_API void lgraph_api_index_spec_set_label(lgraph_api_index_spec_t* is,
                                                               const char* label);
extern LGRAPH_LIBRARY_API void lgraph_api_index_spec_set_field(lgraph_api_index_spec_t* is,
                                                               const char* field);
extern LGRAPH_LIBRARY_API void lgraph_api_index_spec_set_type(lgraph_api_index_spec_t* is,
                                                              int type);

/* EdgeUid */
extern LGRAPH_LIBRARY_API lgraph_api_edge_uid_t* lgraph_api_create_edge_euid(
    int64_t src, int64_t dst, uint16_t lid, int64_t tid, int64_t eid);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_euid_destroy(lgraph_api_edge_uid_t* euid);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_edge_euid_get_src(lgraph_api_edge_uid_t* euid);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_edge_euid_get_dst(lgraph_api_edge_uid_t* euid);
extern LGRAPH_LIBRARY_API uint16_t lgraph_api_edge_euid_get_lid(lgraph_api_edge_uid_t* euid);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_edge_euid_get_tid(lgraph_api_edge_uid_t* euid);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_edge_euid_get_eid(lgraph_api_edge_uid_t* euid);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_euid_set_src(lgraph_api_edge_uid_t* euid,
                                                            int64_t src);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_euid_set_dst(lgraph_api_edge_uid_t* euid,
                                                            int64_t dst);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_euid_set_lid(lgraph_api_edge_uid_t* euid,
                                                            uint16_t lid);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_euid_set_tid(lgraph_api_edge_uid_t* euid,
                                                            int64_t tid);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_euid_set_eid(lgraph_api_edge_uid_t* euid,
                                                            int64_t eid);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_euid_reverse(lgraph_api_edge_uid_t* euid);
extern LGRAPH_LIBRARY_API bool lgraph_api_edge_euid_eq(const lgraph_api_edge_uid_t* euid,
                                                       const lgraph_api_edge_uid_t* other_euid);
extern LGRAPH_LIBRARY_API bool lgraph_api_edge_euid_out_less(
    const lgraph_api_edge_uid_t* euid, const lgraph_api_edge_uid_t* other_euid);
extern LGRAPH_LIBRARY_API bool lgraph_api_edge_euid_in_less(
    const lgraph_api_edge_uid_t* euid, const lgraph_api_edge_uid_t* other_euid);
extern LGRAPH_LIBRARY_API char* lgraph_api_edge_euid_to_string(lgraph_api_edge_uid_t* euid);

extern LGRAPH_LIBRARY_API lgraph_api_user_info_t* lgraph_api_create_user_info();
extern LGRAPH_LIBRARY_API void lgraph_api_user_info_destroy(lgraph_api_user_info_t* ui);
extern LGRAPH_LIBRARY_API const char* lgraph_api_user_info_get_desc(lgraph_api_user_info_t* ui);
extern LGRAPH_LIBRARY_API size_t lgraph_api_user_info_get_roles(lgraph_api_user_info_t* ui,
                                                                char*** roles);
extern LGRAPH_LIBRARY_API void lgraph_api_user_info_destroy_roles(char** roles, size_t n);
extern LGRAPH_LIBRARY_API bool lgraph_api_user_info_get_disable(lgraph_api_user_info_t* ui);
extern LGRAPH_LIBRARY_API size_t lgraph_api_user_info_get_memory_limit(lgraph_api_user_info_t* ui);
extern LGRAPH_LIBRARY_API void lgraph_api_user_info_set_desc(lgraph_api_user_info_t* ui,
                                                             const char* desc);
extern LGRAPH_LIBRARY_API void lgraph_api_user_info_add_role(lgraph_api_user_info_t* ui,
                                                             const char* role);
extern LGRAPH_LIBRARY_API void lgraph_api_user_info_set_roles(lgraph_api_user_info_t* ui,
                                                              const char** roles, size_t n);
/* return deleted roles */
extern LGRAPH_LIBRARY_API int lgraph_api_user_info_del_role(lgraph_api_user_info_t* ui,
                                                            const char* role);
extern LGRAPH_LIBRARY_API void lgraph_api_user_info_set_disable(lgraph_api_user_info_t* ui,
                                                                bool disable);
extern LGRAPH_LIBRARY_API void lgraph_api_user_info_set_memory_limit(lgraph_api_user_info_t* ui,
                                                                     size_t memory_limit);

extern LGRAPH_LIBRARY_API lgraph_api_role_info_t* lgraph_api_create_role_info();
extern LGRAPH_LIBRARY_API void lgraph_api_role_info_destroy(lgraph_api_role_info_t* ri);
extern LGRAPH_LIBRARY_API const char* lgraph_api_role_info_get_desc(lgraph_api_role_info_t* ri);
enum {
    lgraph_api_access_level_none = 0,
    lgraph_api_access_level_read = 1,
    lgraph_api_access_level_write = 2,
    lgraph_api_access_level_full = 3,
};
/* if graph_name not exist, return -1 */
extern LGRAPH_LIBRARY_API int lgraph_api_role_info_get_access_level(lgraph_api_role_info_t* ri,
                                                                    const char* graph_name);
extern LGRAPH_LIBRARY_API size_t lgraph_api_role_info_get_graph_access(lgraph_api_role_info_t* ri,
                                                                       char*** graph_names,
                                                                       int** access_levels);
extern LGRAPH_LIBRARY_API void lgraph_api_role_info_destroy_graph_access(char** graph_names,
                                                                         int* access_levels,
                                                                         size_t n);
extern LGRAPH_LIBRARY_API bool lgraph_api_role_info_get_disabled(lgraph_api_role_info_t* ri);
extern LGRAPH_LIBRARY_API void lgraph_api_role_info_set_desc(lgraph_api_role_info_t* ri,
                                                             const char* desc);
extern LGRAPH_LIBRARY_API bool lgraph_api_role_info_add_access_level(lgraph_api_role_info_t* ri,
                                                                     const char* graph,
                                                                     int access_level);
extern LGRAPH_LIBRARY_API void lgraph_api_role_info_set_graph_access(lgraph_api_role_info_t* ri,
                                                                     char** graph_names,
                                                                     int* access_levels, size_t n);
/* if graph_name not exist, return -1 */
extern LGRAPH_LIBRARY_API int lgraph_api_role_info_del_access_level(lgraph_api_role_info_t* ri,
                                                                    const char* graph_name);
extern LGRAPH_LIBRARY_API void lgraph_api_role_info_set_disabled(lgraph_api_role_info_t* ri,
                                                                 bool disabled);

/* OutEdgeIterator */
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_destroy(
    lgraph_api_out_edge_iterator_t* it);
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_close(
    lgraph_api_out_edge_iterator_t* it);
extern LGRAPH_LIBRARY_API bool lgraph_api_out_edge_iterator_goto(lgraph_api_out_edge_iterator_t* it,
                                                                 const lgraph_api_edge_uid_t* euid,
                                                                 bool nearest, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_out_edge_iterator_is_valid(
    lgraph_api_out_edge_iterator_t* it);
extern LGRAPH_LIBRARY_API bool lgraph_api_out_edge_iterator_next(lgraph_api_out_edge_iterator_t* it,
                                                                 char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_edge_uid_t* lgraph_api_out_edge_iterator_get_uid(
    lgraph_api_out_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_out_edge_iterator_get_dst(lgraph_api_out_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_out_edge_iterator_get_edge_id(lgraph_api_out_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_out_edge_iterator_get_temporal_id(lgraph_api_out_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_out_edge_iterator_get_src(lgraph_api_out_edge_iterator_t* it, char** errptr);
const char* lgraph_api_out_edge_iterator_get_label(lgraph_api_out_edge_iterator_t* it,
                                                   char** errptr);
extern LGRAPH_LIBRARY_API uint16_t
lgraph_api_out_edge_iterator_get_label_id(lgraph_api_out_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_out_edge_iterator_get_fields_by_names(
    lgraph_api_out_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_out_edge_iterator_get_field_by_name(
    lgraph_api_out_edge_iterator_t* it, const char* field_name, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_out_edge_iterator_get_fields_by_ids(
    lgraph_api_out_edge_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_out_edge_iterator_get_field_by_id(
    lgraph_api_out_edge_iterator_t* it, size_t field_id, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_out_edge_iterator_get_all_fields(lgraph_api_out_edge_iterator_t* it, char*** field_names,
                                            lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_field_names_destroy(char** field_names, size_t n);
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_set_field_by_name(
    lgraph_api_out_edge_iterator_t* it, const char* field_name,
    const lgraph_api_field_data_t* field_value, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_set_field_by_id(
    lgraph_api_out_edge_iterator_t* it, size_t field_id, const lgraph_api_field_data_t* field_value,
    char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_set_fields_by_value_strings(
    lgraph_api_out_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const char* const* field_value_strings, size_t field_value_strings_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_set_fields_by_data(
    lgraph_api_out_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_set_fields_by_ids(
    lgraph_api_out_edge_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_out_edge_iterator_delete(
    lgraph_api_out_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API char* lgraph_api_out_edge_iterator_to_string(
    lgraph_api_out_edge_iterator_t* it, char** errptr);

/* InEdgeIterator */
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_destroy(
    lgraph_api_in_edge_iterator_t* it);
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_close(lgraph_api_in_edge_iterator_t* it);
extern LGRAPH_LIBRARY_API bool lgraph_api_in_edge_iterator_next(lgraph_api_in_edge_iterator_t* it,
                                                                char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_in_edge_iterator_goto(lgraph_api_in_edge_iterator_t* it,
                                                                const lgraph_api_edge_uid_t* euid,
                                                                bool nearest, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_edge_uid_t* lgraph_api_in_edge_iterator_get_uid(
    lgraph_api_in_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_in_edge_iterator_get_src(lgraph_api_in_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_in_edge_iterator_get_dst(lgraph_api_in_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_in_edge_iterator_get_edge_id(lgraph_api_in_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_in_edge_iterator_get_temporal_id(lgraph_api_in_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_in_edge_iterator_is_valid(
    lgraph_api_in_edge_iterator_t* it);
extern LGRAPH_LIBRARY_API const char* lgraph_api_in_edge_iterator_get_label(
    lgraph_api_in_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API uint16_t
lgraph_api_in_edge_iterator_get_label_id(lgraph_api_in_edge_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_in_edge_iterator_get_fields_by_names(
    lgraph_api_in_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_in_edge_iterator_get_field(
    lgraph_api_in_edge_iterator_t* it, const char* field_name, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_in_edge_iterator_get_fields_by_ids(
    lgraph_api_in_edge_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_in_edge_iterator_get_field_by_id(
    lgraph_api_in_edge_iterator_t* it, size_t field_id, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_in_edge_iterator_get_field_by_name(
    lgraph_api_in_edge_iterator_t* it, const char* field_name, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_in_edge_iterator_get_all_fields(lgraph_api_in_edge_iterator_t* it, char*** field_names,
                                           lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_set_field_by_name(
    lgraph_api_in_edge_iterator_t* it, const char* field_name,
    const lgraph_api_field_data_t* field_value, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_set_field_by_id(
    lgraph_api_in_edge_iterator_t* it, size_t field_id, const lgraph_api_field_data_t* field_value,
    char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_set_fields_by_value_strings(
    lgraph_api_in_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const char* const* field_value_strings, size_t field_value_strings_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_set_fields_by_data(
    lgraph_api_in_edge_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_set_fields_by_ids(
    lgraph_api_in_edge_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_in_edge_iterator_delete(lgraph_api_in_edge_iterator_t* it,
                                                                  char** errptr);
extern LGRAPH_LIBRARY_API char* lgraph_api_in_edge_iterator_to_string(
    lgraph_api_in_edge_iterator_t* it, char** errptr);

extern LGRAPH_LIBRARY_API bool lgraph_api_out_edge_iterator_eq(
    const lgraph_api_out_edge_iterator_t* lhs, const lgraph_api_out_edge_iterator_t* rhs);
extern LGRAPH_LIBRARY_API bool LGraphApi_OutInEdgeIterator_eq(
    const lgraph_api_out_edge_iterator_t* lhs, const lgraph_api_in_edge_iterator_t* rhs);
extern LGRAPH_LIBRARY_API bool lgraph_api_in_edge_iterator_eq(
    const lgraph_api_in_edge_iterator_t* lhs, const lgraph_api_in_edge_iterator_t* rhs);

/* VertexIterator */
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_destroy(lgraph_api_vertex_iterator_t* it);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_close(lgraph_api_vertex_iterator_t* it);
extern LGRAPH_LIBRARY_API bool lgraph_api_vertex_iterator_next(lgraph_api_vertex_iterator_t* it,
                                                               char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_vertex_iterator_goto(lgraph_api_vertex_iterator_t* it,
                                                               int64_t vid, bool nearest,
                                                               char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_vertex_iterator_get_id(lgraph_api_vertex_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_out_edge_iterator_t*
lgraph_api_vertex_iterator_get_out_edge_iterator(lgraph_api_vertex_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_out_edge_iterator_t*
lgraph_api_vertex_iterator_get_out_edge_iterator_by_euid(lgraph_api_vertex_iterator_t* it,
                                                         const lgraph_api_edge_uid_t* euid,
                                                         bool nearest, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_in_edge_iterator_t*
lgraph_api_vertex_iterator_get_in_edge_iterator(lgraph_api_vertex_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_in_edge_iterator_t*
lgraph_api_vertex_iterator_get_in_edge_iterator_by_euid(lgraph_api_vertex_iterator_t* it,
                                                        const lgraph_api_edge_uid_t* euid,
                                                        bool nearest, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_vertex_iterator_is_valid(
    lgraph_api_vertex_iterator_t* it);
const char* lgraph_api_vertex_iterator_get_label(lgraph_api_vertex_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API uint16_t
lgraph_api_vertex_iterator_get_label_id(lgraph_api_vertex_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_vertex_iterator_get_fields_by_names(
    lgraph_api_vertex_iterator_t* it, const char* const* field_names, size_t field_names_len,
    lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_vertex_iterator_get_field_by_name(
    lgraph_api_vertex_iterator_t* it, const char* field_name, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_vertex_iterator_get_fields_by_ids(
    lgraph_api_vertex_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_vertex_iterator_get_field_by_id(
    lgraph_api_vertex_iterator_t* it, size_t field_id, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_vertex_iterator_get_all_fields(lgraph_api_vertex_iterator_t* it, char*** field_names,
                                          lgraph_api_field_data_t*** field_data, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_set_field_by_name(
    lgraph_api_vertex_iterator_t* it, const char* field_name,
    const lgraph_api_field_data_t* field_value, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_set_field_by_id(
    lgraph_api_vertex_iterator_t* it, size_t field_id, const lgraph_api_field_data_t* field_value,
    char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_set_fields_by_value_strings(
    lgraph_api_vertex_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const char* const* field_value_strings, size_t field_value_strings_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_set_fields_by_data(
    lgraph_api_vertex_iterator_t* it, const char* const* field_names, size_t field_names_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_set_fields_by_ids(
    lgraph_api_vertex_iterator_t* it, const size_t* field_ids, size_t field_ids_len,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_len, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_vertex_iterator_list_src_vids(lgraph_api_vertex_iterator_t* it, size_t n_limit,
                                         bool* more_to_go, int64_t** vids, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_vertex_iterator_list_dst_vids(lgraph_api_vertex_iterator_t* it, size_t n_limit,
                                         bool* more_to_go, int64_t** vids, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_list_vids_destroy(int64_t* vids);
extern LGRAPH_LIBRARY_API size_t lgraph_api_vertex_iterator_get_num_in_edges(
    lgraph_api_vertex_iterator_t* it, size_t n_limit, bool* more_to_go, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_vertex_iterator_get_num_out_edges(
    lgraph_api_vertex_iterator_t* it, size_t n_limit, bool* more_to_go, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_iterator_delete(lgraph_api_vertex_iterator_t* it,
                                                                 size_t* n_in_edges,
                                                                 size_t* n_out_edges,
                                                                 char** errptr);
extern LGRAPH_LIBRARY_API char* lgraph_api_vertex_iterator_to_string(
    lgraph_api_vertex_iterator_t* it);

// binding types in lgraph_vertex_index_iterator.h

extern LGRAPH_LIBRARY_API void lgraph_api_vertex_index_iterator_destroy(
    lgraph_api_vertex_index_iterator_t* it);
extern LGRAPH_LIBRARY_API void lgraph_api_vertex_index_iterator_close(
    lgraph_api_vertex_index_iterator_t* it);
extern LGRAPH_LIBRARY_API bool lgraph_api_vertex_index_iterator_is_valid(
    lgraph_api_vertex_index_iterator_t* it);
extern LGRAPH_LIBRARY_API bool lgraph_api_vertex_index_iterator_next(
    lgraph_api_vertex_index_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_vertex_index_iterator_get_index_value(
    lgraph_api_vertex_index_iterator_t* it, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_vertex_index_iterator_get_vid(lgraph_api_vertex_index_iterator_t* it, char** errptr);

/* EdgeIndexIterator */
extern LGRAPH_LIBRARY_API void lgraph_api_edge_index_iterator_close(
    lgraph_api_edge_index_iterator_t* iter);
extern LGRAPH_LIBRARY_API void lgraph_api_edge_index_iterator_destroy(
    lgraph_api_edge_index_iterator_t* iter);
extern LGRAPH_LIBRARY_API bool lgraph_api_edge_index_iterator_is_valid(
    lgraph_api_edge_index_iterator_t* iter);
extern LGRAPH_LIBRARY_API bool lgraph_api_edge_index_iterator_next(
    lgraph_api_edge_index_iterator_t* iter, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_field_data_t* lgraph_api_edge_index_iterator_get_index_value(
    lgraph_api_edge_index_iterator_t* iter, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_edge_uid_t* lgraph_api_edge_index_iterator_get_uid(
    lgraph_api_edge_index_iterator_t* iter, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_edge_index_iterator_get_src(lgraph_api_edge_index_iterator_t* iter, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_edge_index_iterator_get_dst(lgraph_api_edge_index_iterator_t* iter, char** errptr);
extern LGRAPH_LIBRARY_API uint16_t
lgraph_api_edge_index_iterator_get_label_id(lgraph_api_edge_index_iterator_t* iter, char** errptr);
extern LGRAPH_LIBRARY_API int64_t
lgraph_api_edge_index_iterator_get_edge_id(lgraph_api_edge_index_iterator_t* iter, char** errptr);

/* Transaction */

extern LGRAPH_LIBRARY_API void lgraph_api_transaction_destroy(lgraph_api_transaction_t* txn);
extern LGRAPH_LIBRARY_API void lgraph_api_transaction_commit(lgraph_api_transaction_t* txn,
                                                             char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_transaction_abort(lgraph_api_transaction_t* txn,
                                                            char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_transaction_is_valid(lgraph_api_transaction_t* txn);
extern LGRAPH_LIBRARY_API bool lgraph_api_transaction_is_read_only(lgraph_api_transaction_t* txn);
extern LGRAPH_LIBRARY_API lgraph_api_vertex_iterator_t* lgraph_api_transaction_get_vertex_iterator(
    lgraph_api_transaction_t* txn, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_vertex_iterator_t*
lgraph_api_transaction_get_vertex_iterator_by_vid(lgraph_api_transaction_t* txn, int64_t vid,
                                                  bool nearest, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_out_edge_iterator_t*
lgraph_api_transaction_get_out_edge_iterator_by_euid(lgraph_api_transaction_t* txn,
                                                     const lgraph_api_edge_uid_t* euid,
                                                     bool nearest, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_out_edge_iterator_t*
lgraph_api_transaction_get_out_edge_iterator_by_src_dst_lid(lgraph_api_transaction_t* txn,
                                                            int64_t src, int64_t dst, int16_t lid,
                                                            char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_in_edge_iterator_t*
lgraph_api_transaction_get_in_edge_iterator_by_euid(lgraph_api_transaction_t* txn,
                                                    const lgraph_api_edge_uid_t* euid, bool nearest,
                                                    char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_in_edge_iterator_t*
lgraph_api_transaction_get_in_edge_iterator_by_src_dst_lid(lgraph_api_transaction_t* txn,
                                                           int64_t src, int64_t dst, int16_t lid,
                                                           char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_transaction_get_num_vertex_labels(lgraph_api_transaction_t* txn, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_transaction_get_num_edge_labels(lgraph_api_transaction_t* txn, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_list_vertex_labels(
    lgraph_api_transaction_t* txn, char*** labels, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_list_edge_labels(
    lgraph_api_transaction_t* txn, char*** labels, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_transaction_list_labels_destroy(char** labels, size_t n);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_get_vertex_label_id(
    lgraph_api_transaction_t* txn, const char* label, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_get_edge_label_id(
    lgraph_api_transaction_t* txn, const char* label, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_transaction_get_vertex_schema(lgraph_api_transaction_t* txn, const char* label,
                                         lgraph_api_field_spec_t*** field_specs, char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_transaction_get_edge_schema(lgraph_api_transaction_t* txn, const char* label,
                                       lgraph_api_field_spec_t*** field_specs, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_get_vertex_field_id(
    lgraph_api_transaction_t* txn, size_t label_id, const char* field_name, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_get_vertex_field_ids(
    lgraph_api_transaction_t* txn, size_t label_id, const char* const* field_names,
    size_t field_names_size, size_t** field_ids, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_get_edge_field_id(
    lgraph_api_transaction_t* txn, size_t label_id, const char* field_name, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_get_edge_field_ids(
    lgraph_api_transaction_t* txn, size_t label_id, const char** field_names,
    size_t field_names_size, size_t** field_ids, char** errptr);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_transaction_add_vertex_with_value_strings(
    lgraph_api_transaction_t* txn, const char* label_name, const char* const* field_names,
    size_t field_names_size, const char* const* field_value_strings,
    size_t field_value_strings_size, char** errptr);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_transaction_add_vertex_with_field_data(
    lgraph_api_transaction_t* txn, const char* label_name, const char* const* field_names,
    size_t field_names_size, const lgraph_api_field_data_t* const* field_values,
    size_t field_values_size, char** errptr);
extern LGRAPH_LIBRARY_API int64_t lgraph_api_transaction_add_vertex_with_field_data_and_id(
    lgraph_api_transaction_t* txn, size_t label_id, const size_t* field_ids, size_t field_ids_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_edge_uid_t* lgraph_api_transaction_add_edge_with_field_data(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char* const* field_names, size_t field_names_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_edge_uid_t* lgraph_api_transaction_add_edge_with_value_strings(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char* const* field_names, size_t field_names_size, const char* const* field_value_strings,
    size_t field_value_strings_size, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_edge_uid_t*
lgraph_api_transaction_add_edge_with_field_data_and_id(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, size_t label_id,
    const size_t* field_ids, size_t field_ids_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_transaction_upsert_edge_with_value_strings(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char* const* field_names, size_t field_names_size, const char* const* field_value_strings,
    size_t field_value_strings_size, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_transaction_upsert_edge_with_field_data(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, const char* label,
    const char* const* field_names, size_t field_names_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_transaction_upsert_edge_with_field_data_and_id(
    lgraph_api_transaction_t* txn, int64_t src, int64_t dst, size_t label_id,
    const size_t* field_ids, size_t field_ids_size,
    const lgraph_api_field_data_t* const* field_values, size_t field_values_size, char** errptr);

extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_list_vertex_indexes(
    lgraph_api_transaction_t* txn, lgraph_api_index_spec_t*** indexes, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_transaction_list_edge_indexes(
    lgraph_api_transaction_t* txn, lgraph_api_index_spec_t*** indexes, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_list_indexes_detroy(lgraph_api_index_spec_t** indexes,
                                                              size_t n);

extern LGRAPH_LIBRARY_API lgraph_api_vertex_index_iterator_t*
lgraph_api_transaction_get_vertex_index_iterator_by_id(lgraph_api_transaction_t* txn,
                                                       size_t label_id, size_t field_id,
                                                       const lgraph_api_field_data_t* key_start,
                                                       const lgraph_api_field_data_t* key_end,
                                                       char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_edge_index_iterator_t*
lgraph_api_transaction_get_edge_index_iterator_by_id(lgraph_api_transaction_t* txn, size_t label_id,
                                                     size_t field_id,
                                                     const lgraph_api_field_data_t* key_start,
                                                     const lgraph_api_field_data_t* key_end,
                                                     char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_vertex_index_iterator_t*
lgraph_api_transaction_get_vertex_index_iterator_by_data(lgraph_api_transaction_t* txn,
                                                         const char* label, const char* field,
                                                         const lgraph_api_field_data_t* key_start,
                                                         const lgraph_api_field_data_t* key_end,
                                                         char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_edge_index_iterator_t*
lgraph_api_transaction_get_edge_index_iterator_by_data(lgraph_api_transaction_t* txn,
                                                       const char* label, const char* field,
                                                       const lgraph_api_field_data_t* key_start,
                                                       const lgraph_api_field_data_t* key_end,
                                                       char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_vertex_index_iterator_t*
lgraph_api_transaction_get_vertex_index_iterator_by_value_string(
    lgraph_api_transaction_t* txn, const char* label, const char* field, const char* key_start,
    const char* key_end, char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_edge_index_iterator_t*
lgraph_api_transaction_get_edge_index_iterator_by_value_string(lgraph_api_transaction_t* txn,
                                                               const char* label, const char* field,
                                                               const char* key_start,
                                                               const char* key_end, char** errptr);

extern LGRAPH_LIBRARY_API bool lgraph_api_transaction_is_vertex_indexed(
    lgraph_api_transaction_t* txn, const char* label, const char* field, char** errptr);

extern LGRAPH_LIBRARY_API bool lgraph_api_transaction_is_edge_indexed(lgraph_api_transaction_t* txn,
                                                                      const char* label,
                                                                      const char* field,
                                                                      char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_vertex_iterator_t*
lgraph_api_transaction_get_vertex_by_unique_index_value_string(lgraph_api_transaction_t* txn,
                                                               const char* label_name,
                                                               const char* field_name,
                                                               const char* field_value_string,
                                                               char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_out_edge_iterator_t*
lgraph_api_transaction_get_edge_by_unique_index_value_string(lgraph_api_transaction_t* txn,
                                                             const char* label_name,
                                                             const char* field_name,
                                                             const char* field_value_string,
                                                             char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_vertex_iterator_t*
lgraph_api_transaction_get_vertex_by_unique_index_with_data(
    lgraph_api_transaction_t* txn, const char* label_name, const char* field_name,
    const lgraph_api_field_data_t* field_value, char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_out_edge_iterator_t*
lgraph_api_transaction_get_edge_by_unique_index_with_data(
    lgraph_api_transaction_t* txn, const char* label_name, const char* field_name,
    const lgraph_api_field_data_t* field_value, char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_vertex_iterator_t*
lgraph_api_transaction_get_vertex_by_unique_index_with_id(
    lgraph_api_transaction_t* txn, size_t label_id, size_t field_id,
    const lgraph_api_field_data_t* field_value, char** errptr);

extern LGRAPH_LIBRARY_API lgraph_api_out_edge_iterator_t*
lgraph_api_transaction_get_edge_by_unique_index_with_id(lgraph_api_transaction_t* txn,
                                                        size_t label_id, size_t field_id,
                                                        const lgraph_api_field_data_t* field_value,
                                                        char** errptr);

extern LGRAPH_LIBRARY_API size_t
lgraph_api_transaction_get_num_vertices(lgraph_api_transaction_t* txn, char** errptr);

extern LGRAPH_LIBRARY_API const char* lgraph_api_transaction_get_vertex_primary_field(
    lgraph_api_transaction_t* txn, const char* label, char** errptr);

/* GraphDB */
extern LGRAPH_LIBRARY_API void lgraph_api_graph_db_close(lgraph_api_graph_db_t* graphdb,
                                                         char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_graph_db_destroy(lgraph_api_graph_db_t* graphdb);
extern LGRAPH_LIBRARY_API lgraph_api_transaction_t* lgraph_api_graph_db_create_read_txn(
    lgraph_api_graph_db_t* graphdb, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_transaction_t* lgraph_api_graph_db_create_write_txn(
    lgraph_api_graph_db_t* graphdb, bool optimistic, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_transaction_t* lgraph_api_graph_db_fork_txn(
    lgraph_api_graph_db_t* graphdb, lgraph_api_transaction_t* txn, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_graph_db_flush(lgraph_api_graph_db_t* graphdb,
                                                         char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_graph_db_drop_all_data(lgraph_api_graph_db_t* graphdb,
                                                                 char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_graph_db_drop_all_vertex(lgraph_api_graph_db_t* graphdb,
                                                                   char** errptr);
extern LGRAPH_LIBRARY_API size_t
lgraph_api_graph_db_estimate_num_vertices(lgraph_api_graph_db_t* graphdb, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_add_vertex_label(
    lgraph_api_graph_db_t* graphdb, const char* label, const lgraph_api_field_spec_t* const* fds,
    size_t fds_len, const char* primary_field, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_delete_vertex_label(
    lgraph_api_graph_db_t* graphdb, const char* label, size_t* n_modified, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_alter_vertex_label_del_fields(
    lgraph_api_graph_db_t* graphdb, const char* label, const char* const* del_fields,
    size_t del_fields_len, size_t* n_modified, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_alter_vertex_label_add_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* add_fields, size_t add_fields_len,
    const lgraph_api_field_data_t* const* default_values, size_t default_values_len,
    size_t* n_modified, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_alter_vertex_label_mod_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* mod_fields, size_t mod_fields_len, size_t* n_modified,
    char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_add_edge_label(
    lgraph_api_graph_db_t* graphdb, const char* label, const lgraph_api_field_spec_t* const* fds,
    size_t fds_len, const char* temporal_field, const char* const* first_edge_constraints,
    const char* const* second_edge_constraints, size_t edge_constraints_len, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_delete_edge_label(lgraph_api_graph_db_t* graphdb,
                                                                     const char* label,
                                                                     size_t* n_modified,
                                                                     char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_alter_label_mod_edge_constraints(
    lgraph_api_graph_db_t* graphdb, const char* label, const char* const* first_edge_constraints,
    const char* const* second_edge_constraints, size_t edge_constraints_len, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_alter_edge_label_del_fields(
    lgraph_api_graph_db_t* graphdb, const char* label, const char* const* del_fields,
    size_t del_fields_len, size_t* n_modified, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_alter_edge_label_add_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* add_fields, size_t add_fields_len,
    const lgraph_api_field_data_t* const* default_values, size_t default_values_len,
    size_t* n_modified, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_alter_edge_label_mod_fields(
    lgraph_api_graph_db_t* graphdb, const char* label,
    const lgraph_api_field_spec_t* const* mod_fields, size_t mod_fields_len, size_t* n_modified,
    char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_add_vertex_index(lgraph_api_graph_db_t* graphdb,
                                                                    const char* label,
                                                                    const char* field,
                                                                    int type,
                                                                    char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_add_edge_index(lgraph_api_graph_db_t* graphdb,
                                                                  const char* label,
                                                                  const char* field,
                                                                  int type,
                                                                  char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_is_vertex_indexed(lgraph_api_graph_db_t* graphdb,
                                                                     const char* label,
                                                                     const char* field,
                                                                     char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_is_edge_indexed(lgraph_api_graph_db_t* graphdb,
                                                                   const char* label,
                                                                   const char* field,
                                                                   char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_delete_vertex_index(
    lgraph_api_graph_db_t* graphdb, const char* label, const char* field, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_delete_edge_index(lgraph_api_graph_db_t* graphdb,
                                                                     const char* label,
                                                                     const char* field,
                                                                     char** errptr);
const char* lgraph_api_graph_db_get_description(lgraph_api_graph_db_t* graphdb, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_graph_db_get_max_size(lgraph_api_graph_db_t* graphdb,
                                                                  char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_add_vertex_full_text_index(
    lgraph_api_graph_db_t* graphdb, const char* vertex_label, const char* field, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_add_edge_full_text_index(
    lgraph_api_graph_db_t* graphdb, const char* edge_label, const char* field, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_delete_vertex_full_text_index(
    lgraph_api_graph_db_t* graphdb, const char* vertex_label, const char* field, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_graph_db_delete_edge_full_text_index(
    lgraph_api_graph_db_t* graphdb, const char* edge_label, const char* field, char** errptr);

extern LGRAPH_LIBRARY_API void lgraph_api_graph_db_rebuild_full_text_index(
    lgraph_api_graph_db_t* graphdb, const char* const* vertex_labels, size_t vertex_labels_len,
    const char* const* edge_labels, size_t edge_labels_len, char** errptr);

extern LGRAPH_LIBRARY_API size_t lgraph_api_graph_db_list_full_text_indexes(
    lgraph_api_graph_db_t* graphdb, bool** is_vertex, char*** label_names, char*** property_names,
    char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_graph_db_list_full_text_indexes_destroy(
    bool** is_vertex, char*** label_names, char*** property_names, size_t n);

extern LGRAPH_LIBRARY_API size_t lgraph_api_graph_db_query_vertex_by_full_text_index(
    lgraph_api_graph_db_t* graphdb, const char* label, const char* query, int top_n, int64_t** vids,
    float** scores, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_graph_db_query_edge_by_full_text_index(
    lgraph_api_graph_db_t* graphdb, const char* label, const char* query, int top_n,
    lgraph_api_edge_uid_t*** euids, float** scores, char** errptr);

/* Galaxy */

extern LGRAPH_LIBRARY_API lgraph_api_galaxy_t* lgraph_api_galaxy_create(const char* dir,
                                                                        bool durable,
                                                                        bool create_if_not_exist,
                                                                        char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_galaxy_t* lgraph_api_galaxy_create_with_user(
    const char* dir, const char* user, const char* password, bool durable, bool create_if_not_exist,
    char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_galaxy_destroy(lgraph_api_galaxy_t* galaxy);
extern LGRAPH_LIBRARY_API void lgraph_api_galaxy_set_current_user(lgraph_api_galaxy_t* galaxy,
                                                                  const char* user,
                                                                  const char* password,
                                                                  char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_galaxy_set_user(lgraph_api_galaxy_t* galaxy,
                                                          const char* user, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_create_graph(lgraph_api_galaxy_t* galaxy,
                                                              const char* graph_name,
                                                              const char* description,
                                                              size_t max_size, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_delete_graph(lgraph_api_galaxy_t* galaxy,
                                                              const char* graph_name,
                                                              char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_mod_graph(lgraph_api_galaxy_t* galaxy,
                                                           const char* graph_name, bool mod_desc,
                                                           const char* desc, bool mod_size,
                                                           size_t new_max_size, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_galaxy_list_graphs(lgraph_api_galaxy_t* galaxy,
                                                               char*** graph_names,
                                                               char*** graph_descs,
                                                               size_t** graph_sizes, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_create_user(lgraph_api_galaxy_t* galaxy,
                                                             const char* user, const char* password,
                                                             const char* desc, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_delete_user(lgraph_api_galaxy_t* galaxy,
                                                             const char* user, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_set_password(lgraph_api_galaxy_t* galaxy,
                                                              const char* user,
                                                              const char* old_password,
                                                              const char* new_password,
                                                              char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_set_user_desc(lgraph_api_galaxy_t* galaxy,
                                                               const char* user, const char* desc,
                                                               char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_set_user_roles(lgraph_api_galaxy_t* galaxy,
                                                                const char* user,
                                                                const char* const* roles,
                                                                size_t num_roles, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_set_user_graph_access(lgraph_api_galaxy_t* galaxy,
                                                                       const char* user,
                                                                       const char* graph,
                                                                       int access, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_disable_user(lgraph_api_galaxy_t* galaxy,
                                                              const char* user, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_enable_user(lgraph_api_galaxy_t* galaxy,
                                                             const char* user, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_galaxy_list_users(lgraph_api_galaxy_t* galaxy,
                                                              char*** user_names,
                                                              lgraph_api_user_info_t*** user_infos,
                                                              char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_galaxy_list_users_destroy(
    char** user_names, lgraph_api_user_info_t** user_infos, size_t num_users);
extern LGRAPH_LIBRARY_API lgraph_api_user_info_t* lgraph_api_galaxy_get_user_info(
    lgraph_api_galaxy_t* galaxy, const char* user, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_create_role(lgraph_api_galaxy_t* galaxy,
                                                             const char* role, const char* desc,
                                                             char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_delete_role(lgraph_api_galaxy_t* galaxy,
                                                             const char* role, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_disable_role(lgraph_api_galaxy_t* galaxy,
                                                              const char* role, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_enable_role(lgraph_api_galaxy_t* galaxy,
                                                             const char* role, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_set_role_desc(lgraph_api_galaxy_t* galaxy,
                                                               const char* role, const char* desc,
                                                               char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_set_role_access_rights(
    lgraph_api_galaxy_t* galaxy, const char* role, const char* const* graph_names,
    const int* access_levels, size_t num_graphs, char** errptr);
extern LGRAPH_LIBRARY_API bool lgraph_api_galaxy_set_role_access_rights_incremental(
    lgraph_api_galaxy_t* galaxy, const char* role, const char* const* graph_names,
    const int* access_levels, size_t num_graphs, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_role_info_t* lgraph_api_galaxy_get_role_info(
    lgraph_api_galaxy_t* galaxy, const char* role, char** errptr);
extern LGRAPH_LIBRARY_API size_t lgraph_api_galaxy_list_roles(lgraph_api_galaxy_t* galaxy,
                                                              char*** role_names,
                                                              lgraph_api_role_info_t*** role_info,
                                                              char** errptr);
extern LGRAPH_LIBRARY_API int lgraph_api_galaxy_get_access_level(lgraph_api_galaxy_t* galaxy,
                                                                 const char* user,
                                                                 const char* graph, char** errptr);
extern LGRAPH_LIBRARY_API lgraph_api_graph_db_t* lgraph_api_galaxy_open_graph(
    lgraph_api_galaxy_t* galaxy, const char* graph, bool read_only, char** errptr);
extern LGRAPH_LIBRARY_API void lgraph_api_galaxy_close(lgraph_api_galaxy_t* galaxy, char** errptr);
#ifdef __cplusplus
}
#endif
