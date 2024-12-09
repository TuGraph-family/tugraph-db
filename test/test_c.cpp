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

#include <cstdint>
#include <cstring>
#include "lgraph/c.h"
#include "fma-common/utils.h"
#include "fma-common/file_system.h"
#include "./ut_utils.h"
#include "lgraph/lgraph_types.h"

#include "gtest/gtest.h"

class TestC : public TuGraphTest {};

struct RemoveDirGuard {
    const char* dir_ = nullptr;
    ~RemoveDirGuard() { fma_common::file_system::RemoveDir(dir_); }
};

#define CHECK_AND_FREESTR(func, res) {auto p = func; ASSERT_STREQ(p, res); free((void*)p);}

TEST_F(TestC, LGraphTypes) {
    lgraph_api_date_time_t* dt = lgraph_api_create_date_time();
    ASSERT_NE(dt, nullptr);
    lgraph_api_date_time_t* dt2 = lgraph_api_create_date_time_ymdhms(2021, 1, 1, 0, 0, 0);
    ASSERT_NE(dt2, nullptr);
    lgraph_api_date_time_t* dt3 = lgraph_api_create_date_time_seconds(1609459200);
    ASSERT_NE(dt3, nullptr);
    ASSERT_EQ(lgraph_api_date_time_seconds_since_epoch(dt), 0);
    ASSERT_EQ(lgraph_api_date_time_seconds_since_epoch(dt2), 1609459200000000);
    ASSERT_EQ(lgraph_api_date_time_seconds_since_epoch(dt3), 1609459200);
    lgraph_api_date_time_destroy(dt);
    lgraph_api_date_time_destroy(dt2);
    lgraph_api_date_time_destroy(dt3);

    lgraph_api_date_t* date = lgraph_api_create_date();
    ASSERT_NE(date, nullptr);
    lgraph_api_date_t* date2 = lgraph_api_create_date_ymd(2021, 1, 1);
    ASSERT_NE(date2, nullptr);
    lgraph_api_date_t* date3 = lgraph_api_create_date_days(18628);
    ASSERT_NE(date3, nullptr);
    ASSERT_EQ(lgraph_api_date_days_since_epoch(date), 0);
    ASSERT_EQ(lgraph_api_date_days_since_epoch(date2), 18628);
    ASSERT_EQ(lgraph_api_date_days_since_epoch(date3), 18628);
    lgraph_api_date_destroy(date);
    lgraph_api_date_destroy(date2);
    lgraph_api_date_destroy(date3);
}

TEST_F(TestC, EdgeUid) {
    lgraph_api_edge_uid_t* euid = lgraph_api_create_edge_euid(1, 2, 3, 4, 5);
    ASSERT_NE(euid, nullptr);
    ASSERT_EQ(lgraph_api_edge_euid_get_src(euid), 1);
    ASSERT_EQ(lgraph_api_edge_euid_get_dst(euid), 2);
    ASSERT_EQ(lgraph_api_edge_euid_get_lid(euid), 3);
    ASSERT_EQ(lgraph_api_edge_euid_get_tid(euid), 4);
    ASSERT_EQ(lgraph_api_edge_euid_get_eid(euid), 5);

    lgraph_api_edge_euid_set_src(euid, 6);
    lgraph_api_edge_euid_set_dst(euid, 7);
    lgraph_api_edge_euid_set_lid(euid, 8);
    lgraph_api_edge_euid_set_tid(euid, 9);
    lgraph_api_edge_euid_set_eid(euid, 10);
    ASSERT_EQ(lgraph_api_edge_euid_get_src(euid), 6);
    ASSERT_EQ(lgraph_api_edge_euid_get_dst(euid), 7);
    ASSERT_EQ(lgraph_api_edge_euid_get_lid(euid), 8);
    ASSERT_EQ(lgraph_api_edge_euid_get_tid(euid), 9);
    ASSERT_EQ(lgraph_api_edge_euid_get_eid(euid), 10);
    lgraph_api_edge_euid_destroy(euid);

    lgraph_api_edge_uid_t* euid1 = lgraph_api_create_edge_euid(1, 2, 3, 4, 5);
    lgraph_api_edge_uid_t* euid2 = lgraph_api_create_edge_euid(2, 1, 3, 4, 5);
    lgraph_api_edge_uid_t* euid3 = lgraph_api_create_edge_euid(2, 3, 3, 4, 5);
    lgraph_api_edge_uid_t* euid4 = lgraph_api_create_edge_euid(3, 2, 4, 4, 5);
    lgraph_api_edge_euid_reverse(euid1);
    ASSERT_TRUE(lgraph_api_edge_euid_eq(euid1, euid2));
    ASSERT_TRUE(lgraph_api_edge_euid_out_less(euid3, euid4));
    ASSERT_TRUE(lgraph_api_edge_euid_in_less(euid4, euid3));
    char* euid1_str = lgraph_api_edge_euid_to_string(euid1);
    ASSERT_STREQ(euid1_str, "2_1_3_4_5");
    lgraph_api_edge_euid_destroy(euid1);
    lgraph_api_edge_euid_destroy(euid2);
    lgraph_api_edge_euid_destroy(euid3);
    lgraph_api_edge_euid_destroy(euid4);
    free(euid1_str);
}

TEST_F(TestC, FieldData) {
    lgraph_api_field_data_t* null_fd = lgraph_api_create_field_data();
    ASSERT_NE(null_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(null_fd), lgraph_api_field_type_null);
    ASSERT_TRUE(lgraph_api_field_data_is_null(null_fd));
    lgraph_api_field_data_destroy(null_fd);

    lgraph_api_field_data_t* int8_fd = lgraph_api_create_field_data_int8(1);
    ASSERT_NE(int8_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(int8_fd), lgraph_api_field_type_int8);
    ASSERT_FALSE(lgraph_api_field_data_is_null(int8_fd));
    ASSERT_FALSE(lgraph_api_field_data_is_real(int8_fd));
    ASSERT_EQ(lgraph_api_field_data_as_int8(int8_fd), 1);
    lgraph_api_field_data_destroy(int8_fd);

    lgraph_api_field_data_t* int16_fd = lgraph_api_create_field_data_int16(1);
    ASSERT_NE(int16_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(int16_fd), lgraph_api_field_type_int16);
    ASSERT_FALSE(lgraph_api_field_data_is_null(int16_fd));
    ASSERT_FALSE(lgraph_api_field_data_is_real(int16_fd));
    ASSERT_EQ(lgraph_api_field_data_as_int16(int16_fd), 1);
    lgraph_api_field_data_destroy(int16_fd);

    lgraph_api_field_data_t* int32_fd = lgraph_api_create_field_data_int32(1);
    ASSERT_NE(int32_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(int32_fd), lgraph_api_field_type_int32);
    ASSERT_FALSE(lgraph_api_field_data_is_null(int32_fd));
    ASSERT_FALSE(lgraph_api_field_data_is_real(int32_fd));
    ASSERT_EQ(lgraph_api_field_data_as_int32(int32_fd), 1);
    lgraph_api_field_data_destroy(int32_fd);

    lgraph_api_field_data_t* int64_fd = lgraph_api_create_field_data_int64(1);
    ASSERT_NE(int64_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(int64_fd), lgraph_api_field_type_int64);
    ASSERT_FALSE(lgraph_api_field_data_is_null(int64_fd));
    ASSERT_FALSE(lgraph_api_field_data_is_real(int64_fd));
    ASSERT_EQ(lgraph_api_field_data_as_int64(int64_fd), 1);
    lgraph_api_field_data_destroy(int64_fd);

    lgraph_api_field_data_t* float_fd = lgraph_api_create_field_data_float(1.0);
    ASSERT_NE(float_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(float_fd), lgraph_api_field_type_float);
    ASSERT_FALSE(lgraph_api_field_data_is_null(float_fd));
    ASSERT_TRUE(lgraph_api_field_data_is_real(float_fd));
    ASSERT_EQ(lgraph_api_field_data_as_float(float_fd), 1.0);
    lgraph_api_field_data_destroy(float_fd);

    lgraph_api_field_data_t* double_fd = lgraph_api_create_field_data_double(1.0);
    ASSERT_NE(double_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(double_fd), lgraph_api_field_type_double);
    ASSERT_FALSE(lgraph_api_field_data_is_null(double_fd));
    ASSERT_TRUE(lgraph_api_field_data_is_real(double_fd));
    ASSERT_EQ(lgraph_api_field_data_as_double(double_fd), 1.0);
    lgraph_api_field_data_destroy(double_fd);

    lgraph_api_date_t* date = lgraph_api_create_date_ymd(2020, 1, 1);
    lgraph_api_field_data_t* date_fd = lgraph_api_create_field_data_date(date);
    ASSERT_NE(date_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(date_fd), lgraph_api_field_type_date);
    ASSERT_FALSE(lgraph_api_field_data_is_null(date_fd));
    lgraph_api_date_t* as_date = lgraph_api_field_data_as_date(date_fd);
    ASSERT_EQ(lgraph_api_date_days_since_epoch(date), lgraph_api_date_days_since_epoch(as_date));
    lgraph_api_date_destroy(date);
    lgraph_api_date_destroy(as_date);
    lgraph_api_field_data_destroy(date_fd);

    lgraph_api_date_time_t* date_time = lgraph_api_create_date_time_ymdhms(2020, 1, 1, 0, 0, 0);
    lgraph_api_field_data_t* date_time_fd = lgraph_api_create_field_data_date_time(date_time);
    ASSERT_NE(date_time_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(date_time_fd), lgraph_api_field_type_datetime);
    ASSERT_FALSE(lgraph_api_field_data_is_null(date_time_fd));
    lgraph_api_date_time_t* as_date_time = lgraph_api_field_data_as_date_time(date_time_fd);
    ASSERT_EQ(lgraph_api_date_time_seconds_since_epoch(date_time),
              lgraph_api_date_time_seconds_since_epoch(as_date_time));
    lgraph_api_date_time_destroy(date_time);
    lgraph_api_date_time_destroy(as_date_time);
    lgraph_api_field_data_destroy(date_time_fd);

    lgraph_api_field_data_t* string_fd = lgraph_api_create_field_data_str("hello");
    ASSERT_NE(string_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(string_fd), lgraph_api_field_type_string);
    ASSERT_FALSE(lgraph_api_field_data_is_null(string_fd));
    ASSERT_TRUE(lgraph_api_field_data_is_buf(string_fd));
    CHECK_AND_FREESTR(lgraph_api_field_data_as_str(string_fd), "hello");

    lgraph_api_field_data_destroy(string_fd);

    lgraph_api_field_data_t* bool_fd = lgraph_api_create_field_data_bool(true);
    ASSERT_NE(bool_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(bool_fd), lgraph_api_field_type_bool);
    ASSERT_FALSE(lgraph_api_field_data_is_null(bool_fd));
    ASSERT_EQ(lgraph_api_field_data_as_bool(bool_fd), true);
    lgraph_api_field_data_destroy(bool_fd);

    lgraph_api_field_data_t* blob_fd = lgraph_api_create_field_data_blob((uint8_t*)"hello", 5);
    ASSERT_NE(blob_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(blob_fd), lgraph_api_field_type_blob);
    ASSERT_TRUE(lgraph_api_field_data_is_buf(blob_fd));
    ASSERT_FALSE(lgraph_api_field_data_is_null(blob_fd));
    char* as_blob = lgraph_api_field_data_as_blob(blob_fd);
    ASSERT_EQ(strlen(as_blob), 5);
    ASSERT_EQ(strcmp(as_blob, "hello"), 0);
    lgraph_api_field_data_destroy(blob_fd);
    free(as_blob);

    // encode "hello" using base64
    lgraph_api_field_data_t* base64_blob_fd =
        lgraph_api_create_field_data_base64_blob((uint8_t*)"aGVsbG8=", 8);
    ASSERT_NE(base64_blob_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(base64_blob_fd), lgraph_api_field_type_blob);
    ASSERT_TRUE(lgraph_api_field_data_is_buf(base64_blob_fd));
    ASSERT_FALSE(lgraph_api_field_data_is_null(base64_blob_fd));
    char* as_base64_blob = lgraph_api_field_data_as_blob(base64_blob_fd);
    ASSERT_EQ(strlen(as_base64_blob), 5);
    ASSERT_EQ(strcmp(as_base64_blob, "hello"), 0);
    lgraph_api_field_data_destroy(base64_blob_fd);
    free(as_base64_blob);

    lgraph_api_field_data_t* origin_fd = lgraph_api_create_field_data_str("hello");
    lgraph_api_field_data_t* cloned_fd = lgraph_api_create_field_data_clone(origin_fd);
    ASSERT_NE(cloned_fd, nullptr);
    ASSERT_EQ(lgraph_api_field_data_get_type(cloned_fd), lgraph_api_field_type_string);
    ASSERT_FALSE(lgraph_api_field_data_is_null(cloned_fd));
    CHECK_AND_FREESTR(lgraph_api_field_data_as_str(cloned_fd), "hello");
    lgraph_api_field_data_t* new_fd = lgraph_api_create_field_data_str("world");
    lgraph_api_create_field_data_clone_from(cloned_fd, new_fd);
    CHECK_AND_FREESTR(lgraph_api_field_data_as_str(cloned_fd), "world");

    lgraph_api_field_data_destroy(origin_fd);
    lgraph_api_field_data_destroy(cloned_fd);
    lgraph_api_field_data_destroy(new_fd);
}

TEST_F(TestC, FieldSpec) {
    lgraph_api_field_spec_t* fs = lgraph_api_create_field_spec();
    ASSERT_NE(fs, nullptr);
    CHECK_AND_FREESTR(lgraph_api_field_spec_get_name(fs), "");
    ASSERT_EQ(lgraph_api_field_spec_get_type(fs), lgraph_api_field_type_null);
    ASSERT_FALSE(lgraph_api_field_spec_get_optional(fs));
    lgraph_api_field_spec_set_name(fs, "hello");
    CHECK_AND_FREESTR(lgraph_api_field_spec_get_name(fs), "hello");
    lgraph_api_field_spec_set_type(fs, lgraph_api_field_type_bool);
    ASSERT_EQ(lgraph_api_field_spec_get_type(fs), lgraph_api_field_type_bool);
    lgraph_api_field_spec_set_optional(fs, true);
    ASSERT_TRUE(lgraph_api_field_spec_get_optional(fs));
    CHECK_AND_FREESTR(
        lgraph_api_field_spec_to_string(fs),
        "lgraph_api::FieldSpec(name=[hello],type=BOOL),optional=1,fieldid=0,isDeleted=0");

    lgraph_api_field_spec_t* fs2 =
        lgraph_api_create_field_spec_name_type_optional("hello", lgraph_api_field_type_bool, true);
    ASSERT_TRUE(lgraph_api_field_spec_eq(fs, fs2));
    lgraph_api_field_spec_destroy(fs);
    lgraph_api_field_spec_destroy(fs2);
}

TEST_F(TestC, IndexSpec) {
    lgraph_api_index_spec_t* is = lgraph_api_create_index_spec();
    ASSERT_NE(is, nullptr);
    CHECK_AND_FREESTR(lgraph_api_index_spec_get_label(is), "");
    CHECK_AND_FREESTR(lgraph_api_index_spec_get_field(is), "");

    ASSERT_FALSE(lgraph_api_index_spec_get_unique(is));
    lgraph_api_index_spec_set_label(is, "hello");
    CHECK_AND_FREESTR(lgraph_api_index_spec_get_label(is), "hello");

    lgraph_api_index_spec_set_field(is, "world");
    CHECK_AND_FREESTR(lgraph_api_index_spec_get_field(is), "world");
    lgraph_api_index_spec_set_type(is, lgraph_api_global_unique_index_type);
    ASSERT_TRUE(lgraph_api_index_spec_get_unique(is));
    lgraph_api_index_spec_destroy(is);
}

TEST_F(TestC, UserInfo) {
    lgraph_api_user_info_t* ui = lgraph_api_create_user_info();
    ASSERT_NE(ui, nullptr);
    CHECK_AND_FREESTR(lgraph_api_user_info_get_desc(ui), "");
    ASSERT_EQ(lgraph_api_user_info_get_roles(ui, nullptr), 0);
    ASSERT_FALSE(lgraph_api_user_info_get_disable(ui));
    ASSERT_EQ(lgraph_api_user_info_get_memory_limit(ui), 0);
    lgraph_api_user_info_set_desc(ui, "hello");
    CHECK_AND_FREESTR(lgraph_api_user_info_get_desc(ui), "hello");
    lgraph_api_user_info_set_disable(ui, true);
    ASSERT_TRUE(lgraph_api_user_info_get_disable(ui));
    lgraph_api_user_info_set_memory_limit(ui, 1024);
    ASSERT_EQ(lgraph_api_user_info_get_memory_limit(ui), 1024);
    const char* roles[] = {"hello", "world"};
    lgraph_api_user_info_set_roles(ui, roles, 2);
    char** roles2 = nullptr;
    size_t n = lgraph_api_user_info_get_roles(ui, &roles2);
    ASSERT_EQ(n, 2);
    ASSERT_STREQ(roles2[0], "hello");
    ASSERT_STREQ(roles2[1], "world");
    lgraph_api_user_info_destroy_roles(roles2, n);
    lgraph_api_user_info_destroy(ui);
}

TEST_F(TestC, RoleInfo) {
    bool ret = true;
    lgraph_api_role_info_t* ri = lgraph_api_create_role_info();
    ASSERT_NE(ri, nullptr);
    CHECK_AND_FREESTR(lgraph_api_role_info_get_desc(ri), "");
    ASSERT_EQ(lgraph_api_role_info_get_access_level(ri, "hello"), -1);
    ASSERT_EQ(lgraph_api_role_info_get_graph_access(ri, nullptr, nullptr), 0);
    ASSERT_FALSE(lgraph_api_role_info_get_disabled(ri));
    lgraph_api_role_info_set_desc(ri, "hello");
    CHECK_AND_FREESTR(lgraph_api_role_info_get_desc(ri), "hello");
    lgraph_api_role_info_add_access_level(ri, "hello", lgraph_api_access_level_read);
    ASSERT_EQ(lgraph_api_role_info_get_access_level(ri, "hello"), lgraph_api_access_level_read);
    lgraph_api_role_info_add_access_level(ri, "world", lgraph_api_access_level_write);
    ASSERT_EQ(lgraph_api_role_info_get_access_level(ri, "world"), lgraph_api_access_level_write);
    ret = lgraph_api_role_info_add_access_level(ri, "world", lgraph_api_access_level_full);
    ASSERT_FALSE(ret);
    ASSERT_EQ(lgraph_api_role_info_get_access_level(ri, "world"), lgraph_api_access_level_write);
    char** graph_names = nullptr;
    int* access_levels = nullptr;
    size_t n = lgraph_api_role_info_get_graph_access(ri, &graph_names, &access_levels);
    ASSERT_EQ(n, 2);
    ASSERT_STREQ(graph_names[0], "hello");
    ASSERT_STREQ(graph_names[1], "world");
    ASSERT_EQ(access_levels[0], lgraph_api_access_level_read);
    ASSERT_EQ(access_levels[1], lgraph_api_access_level_write);
    lgraph_api_role_info_destroy_graph_access(graph_names, access_levels, n);
    // create new graph access
    graph_names = new char*[2];
    graph_names[0] = strdup("nice");
    graph_names[1] = strdup("world");
    access_levels = new int[2];
    access_levels[0] = lgraph_api_access_level_read;
    access_levels[1] = lgraph_api_access_level_full;
    lgraph_api_role_info_set_graph_access(ri, graph_names, access_levels, n);
    lgraph_api_role_info_destroy_graph_access(graph_names, access_levels, n);
    // check new graph access
    n = lgraph_api_role_info_get_graph_access(ri, &graph_names, &access_levels);
    ASSERT_EQ(n, 2);
    ASSERT_STREQ(graph_names[0], "nice");
    ASSERT_STREQ(graph_names[1], "world");
    ASSERT_EQ(access_levels[0], lgraph_api_access_level_read);
    ASSERT_EQ(access_levels[1], lgraph_api_access_level_full);
    lgraph_api_role_info_destroy_graph_access(graph_names, access_levels, n);
    // delete access level
    ASSERT_EQ(lgraph_api_role_info_del_access_level(ri, "nice"), 1);
    ASSERT_EQ(lgraph_api_role_info_get_access_level(ri, "nice"), -1);
    ASSERT_EQ(lgraph_api_role_info_get_access_level(ri, "world"), lgraph_api_access_level_full);
    // set disabled
    lgraph_api_role_info_set_disabled(ri, true);
    ASSERT_TRUE(lgraph_api_role_info_get_disabled(ri));
    lgraph_api_role_info_destroy(ri);
}

TEST_F(TestC, Galaxy) {
    // write unit tests of above functions
    const char* dir = "./c_test_galaxy";
    fma_common::file_system::RemoveDir(dir);
    RemoveDirGuard rdg{dir};
    char* errptr = nullptr;
    bool ret = true;

    lgraph_api_galaxy_t* galaxy = lgraph_api_galaxy_create(dir, true, true, &errptr);
    ASSERT_NE(galaxy, nullptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");

    // set current user to admin
    lgraph_api_galaxy_set_current_user(galaxy, "admin", "73@TuGraph", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");

    // =====================
    // test user functions
    // =====================
    lgraph_api_galaxy_create_user(galaxy, "user1", "password1", "desc1", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    lgraph_api_galaxy_create_user(galaxy, "user2", "password2", "desc2", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    lgraph_api_galaxy_set_current_user(galaxy, "user2", "password2", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    lgraph_api_galaxy_set_user(galaxy, "user1", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // change user1 desc
    lgraph_api_galaxy_set_user_desc(galaxy, "user1", "desc1_changed", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // change user1 password
    lgraph_api_galaxy_set_password(galaxy, "user1", "password1", "password1_changed", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // set current to admin
    lgraph_api_galaxy_set_current_user(galaxy, "admin", "73@TuGraph", &errptr);
    // disable user2
    lgraph_api_galaxy_disable_user(galaxy, "user2", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // enable user1
    lgraph_api_galaxy_enable_user(galaxy, "user1", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // list users
    char** user_names = nullptr;
    lgraph_api_user_info_t** user_infos;
    size_t num_users = lgraph_api_galaxy_list_users(galaxy, &user_names, &user_infos, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(num_users, 3);
    ASSERT_STREQ(user_names[1], "user1");
    ASSERT_STREQ(user_names[2], "user2");
    CHECK_AND_FREESTR(lgraph_api_user_info_get_desc(user_infos[1]), "desc1_changed");
    CHECK_AND_FREESTR(lgraph_api_user_info_get_desc(user_infos[2]), "desc2");

    ASSERT_EQ(lgraph_api_user_info_get_disable(user_infos[1]), false);
    ASSERT_EQ(lgraph_api_user_info_get_disable(user_infos[2]), true);
    for (auto i = 0; i < num_users; i++) {
        lgraph_api_user_info_destroy(user_infos[i]);
        free(user_names[i]);
    }
    delete[] user_infos;
    delete[] user_names;
    // delete all users
    ret = lgraph_api_galaxy_delete_user(galaxy, "user1", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    ret = lgraph_api_galaxy_delete_user(galaxy, "user2", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);

    // =============================
    // test graph functions
    // =============================
    ret = lgraph_api_galaxy_create_graph(galaxy, "graph1", "desc1", 1 << 20, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // list graph
    char** graph_names = nullptr;
    char** graph_descs = nullptr;
    size_t* graph_sizes = nullptr;
    size_t num_graphs =
        lgraph_api_galaxy_list_graphs(galaxy, &graph_names, &graph_descs, &graph_sizes, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(num_graphs, 2);
    ASSERT_STREQ(graph_names[0], "default");
    ASSERT_STREQ(graph_names[1], "graph1");
    ASSERT_STREQ(graph_descs[0], "");
    ASSERT_STREQ(graph_descs[1], "desc1");
    ASSERT_EQ(graph_sizes[1], 1 << 20);
    // free graph_names and graph_descs
    for (size_t i = 0; i < num_graphs; ++i) {
        free(graph_names[i]);
        free(graph_descs[i]);
    }
    free(graph_names);
    free(graph_descs);
    free(graph_sizes);

    // modify graph
    lgraph_api_galaxy_mod_graph(galaxy, "graph1", true, "desc1_changed", true, 2 << 20, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // list graph
    num_graphs =
        lgraph_api_galaxy_list_graphs(galaxy, &graph_names, &graph_descs, &graph_sizes, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(num_graphs, 2);
    ASSERT_STREQ(graph_descs[1], "desc1_changed");
    ASSERT_EQ(graph_sizes[1], 2 << 20);
    // free graph_names and graph_descs
    for (size_t i = 0; i < num_graphs; ++i) {
        free(graph_names[i]);
        free(graph_descs[i]);
    }
    free(graph_names);
    free(graph_descs);
    free(graph_sizes);

    // open graph and destroy it
    lgraph_api_graph_db_t* graph = lgraph_api_galaxy_open_graph(galaxy, "graph1", true, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_NE(graph, nullptr);

    // delete graph1
    ret = lgraph_api_galaxy_delete_graph(galaxy, "graph1", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // list graph
    num_graphs =
        lgraph_api_galaxy_list_graphs(galaxy, &graph_names, &graph_descs, &graph_sizes, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(num_graphs, 1);
    ASSERT_STREQ(graph_names[0], "default");
    ASSERT_STREQ(graph_descs[0], "");
    // free graph_names and graph_descs
    for (size_t i = 0; i < num_graphs; ++i) {
        free(graph_names[i]);
        free(graph_descs[i]);
    }
    free(graph_names);
    free(graph_descs);
    free(graph_sizes);
    // destroy graph
    lgraph_api_graph_db_destroy(graph);

    // =========================
    // test role functions
    // =========================
    ret = lgraph_api_galaxy_create_graph(galaxy, "graph1", "desc1", 1 << 20, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // create role1
    ret = lgraph_api_galaxy_create_role(galaxy, "role1", "desc1", &errptr);
    ASSERT_TRUE(ret);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // change role1 desc
    lgraph_api_galaxy_set_role_desc(galaxy, "role1", "desc1_changed", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // set role1 access rights on default
    const char* tmp_graph_names1[1] = {"default"};
    ret = lgraph_api_galaxy_set_role_access_rights(
        galaxy, "role1", tmp_graph_names1, (const int[]){lgraph_api_access_level_read}, 1, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // set role1 access rights on graph1 incrementally
    const char* tmp_graph_names2[2] = {"default", "graph1"};
    ret = lgraph_api_galaxy_set_role_access_rights_incremental(
        galaxy, "role1", tmp_graph_names2,
        (const int[]){lgraph_api_access_level_write, lgraph_api_access_level_full}, 2, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // create role2
    ret = lgraph_api_galaxy_create_role(galaxy, "role2", "desc2", &errptr);
    ASSERT_TRUE(ret);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // disable role1
    lgraph_api_galaxy_disable_role(galaxy, "role1", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // list roles
    char** role_names = nullptr;
    lgraph_api_role_info_t** role_infos;
    size_t num_roles = lgraph_api_galaxy_list_roles(galaxy, &role_names, &role_infos, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(num_roles, 3);
    ASSERT_STREQ(role_names[0], "admin");
    ASSERT_STREQ(role_names[1], "role1");
    ASSERT_STREQ(role_names[2], "role2");
    ASSERT_EQ(lgraph_api_role_info_get_disabled(role_infos[1]), true);
    ASSERT_EQ(lgraph_api_role_info_get_disabled(role_infos[2]), false);
    // get graph access of role1
    ASSERT_EQ(lgraph_api_role_info_get_access_level(role_infos[1], "default"),
              lgraph_api_access_level_write);
    ASSERT_EQ(lgraph_api_role_info_get_access_level(role_infos[1], "graph1"),
              lgraph_api_access_level_full);
    // destroy role_infos
    for (size_t i = 0; i < num_roles; ++i) {
        lgraph_api_role_info_destroy(role_infos[i]);
        free(role_names[i]);
    }
    delete[] role_infos;
    delete[] role_names;
    // delete role2
    ret = lgraph_api_galaxy_delete_role(galaxy, "role2", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // enable role1
    ret = lgraph_api_galaxy_enable_role(galaxy, "role1", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // list roles
    num_roles = lgraph_api_galaxy_list_roles(galaxy, &role_names, &role_infos, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(num_roles, 2);
    ASSERT_EQ(lgraph_api_role_info_get_disabled(role_infos[1]), false);
    // destroy role_infos
    for (size_t i = 0; i < num_roles; ++i) {
        lgraph_api_role_info_destroy(role_infos[i]);
        free(role_names[i]);
    }
    delete[] role_infos;
    delete[] role_names;

    // =========================
    // test user roles functions
    // =========================
    // create user1
    ret = lgraph_api_galaxy_create_user(galaxy, "user1", "password1", "desc1", &errptr);
    ASSERT_TRUE(ret);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");

    // set user1 with role1
    const char* tmp_roles[1] = {"role1"};
    ret = lgraph_api_galaxy_set_user_roles(galaxy, "user1", tmp_roles, 1, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(ret, true);
    // list users
    num_users = lgraph_api_galaxy_list_users(galaxy, &user_names, &user_infos, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_EQ(num_users, 2);
    ASSERT_STREQ(user_names[0], "admin");
    ASSERT_STREQ(user_names[1], "user1");
    // get roles of user1
    char** roles = nullptr;
    num_roles = lgraph_api_user_info_get_roles(user_infos[1], &roles);
    ASSERT_EQ(num_roles, 2);
    ASSERT_STREQ(roles[0], "role1");
    ASSERT_STREQ(roles[1], "user1");
    // free roles
    lgraph_api_user_info_destroy_roles(roles, num_roles);
    // destroy user_infos
    for (size_t i = 0; i < num_users; ++i) {
        lgraph_api_user_info_destroy(user_infos[i]);
        free(user_names[i]);
    }
    delete[] user_names;
    delete[] user_infos;

    // =========================
    // close galaxy
    // =========================
    lgraph_api_galaxy_close(galaxy, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");

    // destroy galaxy
    lgraph_api_galaxy_destroy(galaxy);
}

TEST_F(TestC, Graph) {
    char* errptr = nullptr;
    bool ret = true;
    const char* dir = "c_test_graph";
    // remove the directory if it exists
    fma_common::file_system::RemoveDir(dir);
    RemoveDirGuard rdg{dir};

    // create a galaxy on c_test_graph
    lgraph_api_galaxy_t* galaxy = lgraph_api_galaxy_create(dir, true, true, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_NE(galaxy, nullptr);
    // set current user admin
    lgraph_api_galaxy_set_current_user(galaxy, "admin", "73@TuGraph", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");

    // open default graph
    lgraph_api_graph_db_t* graphdb =
        lgraph_api_galaxy_open_graph(galaxy, "default", false, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_NE(graphdb, nullptr);
    // now we are going to create a relationship graph
    //
    // Person --knows--> Person
    //     vuid1               vuid2               vuid3             vuid4
    //  ----------  euid1   ----------  euid2    ---------  euid3     -----------
    //  |  John  |--knows-->|  Mary  |--knows--> |  Bob  |--knows-->|  Alice  |
    //  ----------          ----------           ---------          -----|-----
    //                          ^                                        |
    //                          |-----------------knows------------------|
    //                                            euid4

    // Firstly, add a vertex label "Person" with 2 fields
    lgraph_api_field_spec_t* fields[2];
    fields[0] = lgraph_api_create_field_spec_name_type_optional(
        "name", lgraph_api_field_type_string, false);
    fields[1] =
        lgraph_api_create_field_spec_name_type_optional("age", lgraph_api_field_type_int8, false);
    ret = lgraph_api_graph_db_add_vertex_label(graphdb, "Person", fields, 2, "name", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(ret);
    lgraph_api_field_spec_destroy(fields[0]);
    lgraph_api_field_spec_destroy(fields[1]);

    // Secondly, add a edge label "knows" with 1 field
    lgraph_api_field_spec_t* fields2[1];
    fields2[0] =
        lgraph_api_create_field_spec_name_type_optional("years", lgraph_api_field_type_int8, false);
    // create constraints from "Person" to "Person"
    const char* first_constraints[1] = {"Person"};
    const char* second_contraints[1] = {"Person"};
    ret = lgraph_api_graph_db_add_edge_label(graphdb, "knows", fields2, 1, "",
                                             first_constraints, second_contraints, 1, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(ret);
    lgraph_api_field_spec_destroy(fields2[0]);

    // create write transaction
    lgraph_api_transaction_t* wtxn = lgraph_api_graph_db_create_write_txn(graphdb, true, &errptr);
    // create a "Person" vertex with "name" called "John" and "age" 20
    lgraph_api_field_data_t* field_data[2];
    field_data[0] = lgraph_api_create_field_data_str("John");
    field_data[1] = lgraph_api_create_field_data_int8(20);
    const char* person_field_names[2] = {"name", "age"};
    int64_t vuid1 = lgraph_api_transaction_add_vertex_with_field_data(
        wtxn, "Person", person_field_names, 2, field_data, 2, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    for (int i = 0; i < 2; i++) {
        lgraph_api_field_data_destroy(field_data[i]);
    }

    // create a "Person" vertex with "name" called "Mary" and "age" 30
    field_data[0] = lgraph_api_create_field_data_str("Mary");
    field_data[1] = lgraph_api_create_field_data_int8(30);
    int64_t vuid2 = lgraph_api_transaction_add_vertex_with_field_data(
        wtxn, "Person", person_field_names, 2, field_data, 2, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    for (int i = 0; i < 2; i++) {
        lgraph_api_field_data_destroy(field_data[i]);
    }

    // create a "Person" vertex with "name" called "Bob" and "age" 40
    field_data[0] = lgraph_api_create_field_data_str("Bob");
    field_data[1] = lgraph_api_create_field_data_int8(40);
    int64_t vuid3 = lgraph_api_transaction_add_vertex_with_field_data(
        wtxn, "Person", person_field_names, 2, field_data, 2, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    for (int i = 0; i < 2; i++) {
        lgraph_api_field_data_destroy(field_data[i]);
    }

    // create a "Person" vertex with "name" called "Alice" and "age" 50
    field_data[0] = lgraph_api_create_field_data_str("Alice");
    field_data[1] = lgraph_api_create_field_data_int8(50);
    int64_t vuid4 = lgraph_api_transaction_add_vertex_with_field_data(
        wtxn, "Person", person_field_names, 2, field_data, 2, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    for (int i = 0; i < 2; i++) {
        lgraph_api_field_data_destroy(field_data[i]);
    }

    // create a "knows" edge between "John" and "Mary" with "years" 10
    const char* knows_field_names[1] = {"years"};
    field_data[0] = lgraph_api_create_field_data_int8(10);
    lgraph_api_edge_uid_t* euid1 = lgraph_api_transaction_add_edge_with_field_data(
        wtxn, vuid1, vuid2, "knows", knows_field_names, 1, field_data, 1, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    lgraph_api_field_data_destroy(field_data[0]);

    // create a "knows" edge between "Mary" and "Bob" with "years" 20
    field_data[0] = lgraph_api_create_field_data_int8(20);
    lgraph_api_edge_uid_t* euid2 = lgraph_api_transaction_add_edge_with_field_data(
        wtxn, vuid2, vuid3, "knows", knows_field_names, 1, field_data, 1, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    lgraph_api_field_data_destroy(field_data[0]);

    // create a "knows" edge between "Bob" and "Alice" with "years" 30
    field_data[0] = lgraph_api_create_field_data_int8(30);
    lgraph_api_edge_uid_t* euid3 = lgraph_api_transaction_add_edge_with_field_data(
        wtxn, vuid3, vuid4, "knows", knows_field_names, 1, field_data, 1, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    lgraph_api_field_data_destroy(field_data[0]);

    // create a "knows" edge between "Alice" and "Mary" with "years" 40
    field_data[0] = lgraph_api_create_field_data_int8(40);
    lgraph_api_edge_uid_t* euid4 = lgraph_api_transaction_add_edge_with_field_data(
        wtxn, vuid4, vuid2, "knows", knows_field_names, 1, field_data, 1, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy field_data
    lgraph_api_field_data_destroy(field_data[0]);

    // commit the transaction
    lgraph_api_transaction_commit(wtxn, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy the transaction
    lgraph_api_transaction_destroy(wtxn);

    // build a index of "age" field for "Person" vertex
    ret = lgraph_api_graph_db_add_vertex_index(graphdb, "Person", "age",
                                               lgraph_api_normal_index_type, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(ret);
    // test "age" field is indexed
    ret = lgraph_api_graph_db_is_vertex_indexed(graphdb, "Person", "age", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(ret);

    // build a index of "years" for "knows" edge
    ret = lgraph_api_graph_db_add_edge_index(graphdb, "knows", "years",
                                             lgraph_api_normal_index_type, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(ret);
    // test "years" field is indexed
    ret = lgraph_api_graph_db_is_edge_indexed(graphdb, "knows", "years", &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(ret);

    // create read transaction
    lgraph_api_transaction_t* rtxn = lgraph_api_graph_db_create_read_txn(graphdb, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(rtxn != nullptr);

    // query all "Person" vertex with "age" >= 30
    lgraph_api_field_data_t* key_start = lgraph_api_create_field_data_int8(30);
    lgraph_api_field_data_t* key_end = lgraph_api_create_field_data_int8(100);
    lgraph_api_vertex_index_iterator_t* it =
        lgraph_api_transaction_get_vertex_index_iterator_by_data(rtxn, "Person", "age", key_start,
                                                                 key_end, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(it != nullptr);

    // iterate all "Person" vertex with "age" >= 30
    int64_t vuid;
    // expected vuid2, vuid3, vuid4
    int64_t expected[] = {vuid2, vuid3, vuid4};
    int i = 0;
    while (lgraph_api_vertex_index_iterator_is_valid(it)) {
        vuid = lgraph_api_vertex_index_iterator_get_vid(it, &errptr);
        ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
        ASSERT_EQ(vuid, expected[i++]);
        lgraph_api_vertex_index_iterator_next(it, &errptr);
    }
    // destroy it
    lgraph_api_vertex_index_iterator_destroy(it);
    // destroy key_start, key_end
    lgraph_api_field_data_destroy(key_start);
    lgraph_api_field_data_destroy(key_end);


    // query all "knows" edge with "years" >= 30
    key_start = lgraph_api_create_field_data_int8(20);
    key_end = lgraph_api_create_field_data_int8(100);
    lgraph_api_edge_index_iterator_t* eit = lgraph_api_transaction_get_edge_index_iterator_by_data(
        rtxn, "knows", "years", key_start, key_end, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    ASSERT_TRUE(eit != nullptr);

    // iterate all "knows" edge with "years" >= 30
    lgraph_api_edge_uid_t* euid;
    // expected euid2, euid3, euid4
    lgraph_api_edge_uid_t* expected_euid[] = {euid2, euid3, euid4};
    i = 0;
    while (lgraph_api_edge_index_iterator_is_valid(eit)) {
        euid = lgraph_api_edge_index_iterator_get_uid(eit, &errptr);
        ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
        ASSERT_TRUE(lgraph_api_edge_euid_eq(euid, expected_euid[i++]));
        lgraph_api_edge_euid_destroy(euid);
        lgraph_api_edge_index_iterator_next(eit, &errptr);
    }
    // destroy eit
    lgraph_api_edge_index_iterator_destroy(eit);
    // destroy key_start, key_end
    lgraph_api_field_data_destroy(key_start);
    lgraph_api_field_data_destroy(key_end);

    // abort rtxn
    lgraph_api_transaction_abort(rtxn, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy rtxn
    lgraph_api_transaction_destroy(rtxn);

    // destroy euids
    lgraph_api_edge_euid_destroy(euid1);
    lgraph_api_edge_euid_destroy(euid2);
    lgraph_api_edge_euid_destroy(euid3);
    lgraph_api_edge_euid_destroy(euid4);

    // close graphdb
    lgraph_api_graph_db_close(graphdb, &errptr);
    ASSERT_EQ(std::string(errptr == nullptr ? "" : errptr), "");
    // destroy graphdb
    lgraph_api_graph_db_destroy(graphdb);
    // destroy galaxy
    lgraph_api_galaxy_destroy(galaxy);
}
