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

#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "./ut_utils.h"


#include "gtest/gtest.h"

#include "db/galaxy.h"
#include "lgraph/lgraph.h"

#include "./test_tools.h"

class TestGalaxy : public TuGraphTest {};

TEST_F(TestGalaxy, Galaxy) {
    const std::string testdir = "./testdb";
    const std::string new_graph = "new_graph";

    const std::string new_user = "new_user";
    const std::string new_user_desc = "description of the new user";
    const std::string new_pass = "new_pass";
    const std::string admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string admin_pass = lgraph::_detail::DEFAULT_ADMIN_PASS;

    const std::string new_role = "new_role";
    const std::string new_role_desc = "description of the new role";
    const std::string admin_role = lgraph::_detail::ADMIN_ROLE;
    const std::string admin_role_desc = lgraph::_detail::ADMIN_ROLE_DESC;
    using namespace lgraph_api;
    auto& fs = fma_common::FileSystem::GetFileSystem("./");
    _ParseTests(&_ut_argc, &_ut_argv);

    DefineTest("Open") {
        MarkTestBegin("open");
        fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
        {
            UT_LOG() << "testdb does not exist";
            UT_EXPECT_ANY_THROW(Galaxy galaxy("./testdb", false, false));

            UT_LOG() << "testdb exists, but .meta does not exist";
            fma_common::file_system::MkDir("./testdb");
            UT_EXPECT_ANY_THROW(Galaxy galaxy("./testdb", false, false));

            UT_LOG() << "testdb exists, but .meta is a file";
            std::ofstream of("./testdb/.meta");
            of.write("a", 1);
            of.close();
            UT_EXPECT_ANY_THROW(Galaxy galaxy("./testdb", false, false));
            UT_EXPECT_ANY_THROW(Galaxy galaxy("./testdb", false, false));

            UT_LOG() << "testdb exists, but .meta is empty";
            fma_common::file_system::RemoveDir("./testdb/.meta");
            fma_common::file_system::MkDir("./testdb/.meta");
            UT_EXPECT_ANY_THROW(Galaxy galaxy("./testdb", false, false));

            UT_LOG() << "testdb/.meta exists, but data.lgr is invalid";
            of.open("./testdb/.meta/data.lgr");
            of.write("a", 1);
            of.close();
            UT_EXPECT_ANY_THROW(Galaxy galaxy("./testdb", false, false));
            UT_EXPECT_ANY_THROW(Galaxy galaxy("./testdb", false, false));

            UT_LOG() << "empty /testdb/.meta should be ok when create_if_not_exist==true";
            fma_common::file_system::RemoveFile("./testdb/.meta/data.lgr");
            { Galaxy galaxy("./testdb", false, true); }

            UT_LOG() << "emtpy /testdb should be ok when create_if_not_exist==true";
            fma_common::file_system::RemoveDir("./testdb");
            fma_common::file_system::MkDir("./testdb");
            { Galaxy galaxy("./testdb", false, true); }

            UT_LOG() << "no testdb should be ok when create_if_not_exist==true";
            fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
            { Galaxy galaxy("./testdb", false, true); }

            UT_LOG() << "should be able to move directory";
            fma_common::file_system::RemoveDir("./testdb2");
            fs.CopyFromLocal("./testdb", "./testdb2");
            fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
            { Galaxy galaxy("./testdb2", false, false); }
        }
        {
            fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
            lgraph::Galaxy::Config conf;
            conf.dir = "./testdb";
            auto global_config = std::make_shared<lgraph::GlobalConfig>();
            global_config->ft_index_options.enable_fulltext_index = true;
            lgraph::Galaxy galaxy(conf, true, global_config);
            lgraph::DBConfig db_conf;
            db_conf.db_size = 1 << 30;
            galaxy.CreateGraph("admin", "graph1", db_conf);
            lgraph::GraphManager::ModGraphActions act;
            act.mod_size = true;
            act.max_size = (size_t)1 << 31;
            galaxy.ModGraph("admin", "graph1", act);
        }
    }

    DefineTest("AddDeleteGraph") {
        MarkTestBegin("add and delete graph");
        fma_common::file_system::RemoveDir(testdir);

#ifdef _WIN32
        static const size_t big_graph_size = (size_t)1 << 32;
#else
        static const size_t big_graph_size = (size_t)1 << 42;
#endif
        {
            Galaxy galaxy("./testdb");
            UT_LOG() << "normal operations";
            galaxy.SetCurrentUser("admin", "73@TuGraph");
            UT_EXPECT_TRUE(galaxy.CreateGraph("graph1T"));
            UT_EXPECT_TRUE(galaxy.CreateGraph("graph4T", "this is graph4T", big_graph_size));
            UT_EXPECT_TRUE(galaxy.CreateGraph("graph1G", "this is graph1G", (size_t)1 << 30));
            auto graphs = galaxy.ListGraphs();
            UT_EXPECT_EQ(graphs.size(), 4);
            UT_EXPECT_EQ(graphs["graph1G"].first, "this is graph1G");
            UT_EXPECT_EQ(graphs["graph1G"].second, (size_t)1 << 30);
            UT_EXPECT_EQ(graphs["graph4T"].first, "this is graph4T");
            UT_EXPECT_EQ(graphs["graph4T"].second, big_graph_size);
        }
        {
            Galaxy galaxy("./testdb");
            galaxy.SetCurrentUser("admin", "73@TuGraph");
            galaxy.ModGraph("graph1G", true, "modified graph1G", true, (size_t)1 << 32);
            auto graphs = galaxy.ListGraphs();
            UT_EXPECT_EQ(graphs["graph1G"].first, "modified graph1G");
            UT_EXPECT_EQ(graphs["graph1G"].second, (size_t)1 << 32);
            UT_EXPECT_EQ(graphs["graph4T"].first, "this is graph4T");
            UT_EXPECT_EQ(graphs["graph4T"].second, big_graph_size);
        }
    }

    auto OpenNewGalaxy = [&]() {
        fma_common::file_system::RemoveDir(testdir);
        Galaxy galaxy(testdir, false, true);
        galaxy.SetCurrentUser(admin_user, admin_pass);
        return galaxy;
    };
    auto OpenExistingGalaxy = [&](bool default_admin_user) {
        Galaxy galaxy(testdir, false, false);
        if (default_admin_user) galaxy.SetCurrentUser(admin_user, admin_pass);
        return galaxy;
    };
    auto NewGalaxyWithNewUser = [&](bool set_to_new_user = true) {
        auto galaxy = OpenNewGalaxy();
        galaxy.CreateUser(new_user, new_pass);
        if (set_to_new_user) galaxy.SetCurrentUser(new_user, new_pass);
        return galaxy;
    };
    auto ExistingGalaxyWithNewUser = [&](bool login_as_new_user) {
        auto galaxy = OpenExistingGalaxy(true);
        if (login_as_new_user) galaxy.SetCurrentUser(new_user, new_pass);
        return galaxy;
    };
    auto NewGalaxyWithNewRoleAndUser = [&](bool use_new_user = true) {
        auto g = NewGalaxyWithNewUser(false);
        g.CreateRole(new_role, new_role_desc);
        if (use_new_user) g.SetCurrentUser(new_user, new_pass);
        return g;
    };
    auto NewGalaxyWithNewRoleUserAndGraph = [&](bool use_new_user = true) {
        auto g = NewGalaxyWithNewRoleAndUser(false);
        g.CreateGraph(new_graph, "desc", 1 << 30);
        g.SetUserRoles(new_user, {new_role});
        if (use_new_user) g.SetCurrentUser(new_user, new_pass);
        return g;
    };

    DefineTest("DoubleOpen") {
        MarkTestBegin("DoubleOpen");
        auto g1 = OpenNewGalaxy();
        UT_EXPECT_NO_THROW(OpenExistingGalaxy(true));
    }

    DefineTest("SetPassword") {
        MarkTestBegin("SetPassword");
        // test with admin
        auto g = OpenNewGalaxy();
        UT_EXPECT_TRUE(g.SetPassword(admin_user, admin_pass, "admin123456"));
        UT_EXPECT_THROW_REGEX(g.SetPassword(admin_user, admin_pass, "new"),
                            "(Incorrect).*(password).");
        UT_EXPECT_THROW_REGEX(g.SetPassword(admin_user, admin_pass, ""), "Invalid password");
        UT_EXPECT_THROW_REGEX(g.SetPassword(admin_user, admin_pass,
                                          std::string(lgraph::_detail::MAX_PASSWORD_LEN + 1, 'a')),
                            "Invalid password");
        UT_EXPECT_TRUE(!g.SetPassword("non_existing_user", "old", "new"));
        UT_EXPECT_THROW_REGEX(g.SetPassword("", "old", "new"), "Invalid user name");
        g.Close();
        // test persistence
        g = OpenExistingGalaxy(false);
        g.SetCurrentUser(admin_user, "admin123456");
        g.Close();
        // test with non-admin
        g = NewGalaxyWithNewUser();
        UT_EXPECT_THROW_REGEX(g.SetPassword(new_user, "wrong_pass", "newpass"),
                            "(Incorrect).*(password).");
        UT_EXPECT_THROW_REGEX(g.SetPassword(admin_user, "pp", "pp"),
                              "Non-admin user cannot.*");
        UT_EXPECT_TRUE(g.SetPassword(new_user, new_pass, "renewed"));
        g.Close();
        // test persistence
        g = OpenExistingGalaxy(false);
        g.SetCurrentUser(new_user, "renewed");
    }

    DefineTest("SetUserDesc") {
        MarkTestBegin("SetUserDesc");
        auto g = OpenNewGalaxy();
        UT_EXPECT_TRUE(g.CreateUser(new_user, new_pass));
        UT_EXPECT_EQ(g.ListUsers().size(), 2);

        // admin can set hiself description
        UT_EXPECT_TRUE(g.SetUserDesc(admin_user, new_user_desc));
        UT_EXPECT_EQ(g.GetUserInfo(admin_user).desc, new_user_desc);

        // switch to new_user
        g.SetCurrentUser(new_user, new_pass);

        // new_user cannot set others' description
        UT_EXPECT_THROW_REGEX(g.SetUserDesc(admin_user, "description setted by new_user"),
                            "(Non-admin).*(cannot).*(modify)");

        // new_user can set hiself description
        UT_EXPECT_TRUE(g.SetUserDesc(new_user, new_user_desc));
        UT_EXPECT_EQ(g.GetUserInfo(new_user).desc, new_user_desc);

        // switch to admin
        g.SetCurrentUser(admin_user, admin_pass);
        const std::string& admin_setted_desc = "description setted by admin";
        // admin can set others' description
        UT_EXPECT_TRUE(g.SetUserDesc(new_user, admin_setted_desc));
        UT_EXPECT_EQ(g.GetUserInfo(new_user).desc, admin_setted_desc);

        g.Close();

        // test persistence
        g = OpenExistingGalaxy(true);
        UT_EXPECT_EQ(g.GetUserInfo(admin_user).desc, new_user_desc);
        UT_EXPECT_EQ(g.GetUserInfo(new_user).desc, admin_setted_desc);
        g.Close();
    }

    DefineTest("CreateDeleteUser") {
        MarkTestBegin("CreateUser/DeleteUser");
        // test with admin
        auto g = OpenNewGalaxy();
        UT_EXPECT_EQ(g.ListUsers().size(), 1);
        UT_EXPECT_TRUE(g.CreateUser(new_user, new_pass));
        UT_EXPECT_EQ(g.ListUsers().size(), 2);
        UT_EXPECT_TRUE(!g.CreateUser(new_user, new_pass));  // already exists
        UT_EXPECT_THROW_REGEX(g.CreateUser("@invalid_name", "new_pass"), "Invalid user name");
        UT_EXPECT_THROW_REGEX(g.CreateUser("", "new_pass"), "Invalid user name");
        UT_EXPECT_THROW_REGEX(g.CreateUser(std::string(1024, 'a'), "new_pass"),
                              "Invalid user name");
        UT_EXPECT_THROW_REGEX(g.CreateUser("ok_name", ""), "Invalid password");
        UT_EXPECT_THROW_REGEX(g.CreateUser("ok_name", std::string(1024, 1)), "Invalid password");
        g.SetCurrentUser(new_user, new_pass);
        g.Close();
        // test persistence
        g = OpenExistingGalaxy(false);
        g.SetCurrentUser(new_user, new_pass);
        g.Close();
        // test with non-admin
        g = NewGalaxyWithNewUser();
        UT_EXPECT_THROW_REGEX(g.CreateUser("new", "new"), "Non-admin user cannot.*");
        g.Close();
        g = OpenExistingGalaxy(true);
        UT_EXPECT_EQ(g.ListUsers().size(), 2);
        g.Close();
        // test DeleteUser
        g = NewGalaxyWithNewUser();
        g.SetCurrentUser(admin_user, admin_pass);
        UT_EXPECT_TRUE(g.DeleteUser(new_user));
        UT_EXPECT_TRUE(!g.DeleteUser("non_existing_user"));
        UT_EXPECT_THROW_REGEX(g.DeleteUser(""), "Invalid user name");
        UT_EXPECT_THROW_REGEX(g.DeleteUser(admin_user), "cannot delete itself");
        UT_EXPECT_THROW_REGEX(g.SetCurrentUser(new_user, new_pass), "Bad user");
        g.Close();
        g = OpenExistingGalaxy(true);
        UT_EXPECT_THROW_REGEX(g.SetCurrentUser(new_user, new_pass), "Bad user");
        UT_EXPECT_EQ(g.ListUsers().size(), 1);
        g.Close();
        g = NewGalaxyWithNewUser();
        UT_EXPECT_THROW_REGEX(g.DeleteUser(admin_user), "Non-admin user cannot.*");
        // try to delete admin
        g.Close();
        g = NewGalaxyWithNewUser(false);
        g.SetUserRoles(new_user, {admin_role});
        g.SetCurrentUser(new_user, new_pass);
        UT_EXPECT_THROW_REGEX(g.DeleteUser(admin_user), "Builtin .* cannot .* ");
    }

    DefineTest("DisableEnableUser") {
        MarkTestBegin("DisableUser/EnableUser");
        // testing disable
        auto g = NewGalaxyWithNewUser();
        g.SetCurrentUser(admin_user, admin_pass);
        UT_EXPECT_TRUE(g.DisableUser(new_user));
        UT_EXPECT_TRUE(!g.DisableUser("non_existing_user"));
        UT_EXPECT_THROW_REGEX(g.DisableUser(""), "Invalid user name");
        UT_EXPECT_THROW_REGEX(g.DisableUser(admin_user), "cannot disable itself");
        UT_EXPECT_THROW_REGEX(g.SetCurrentUser(new_user, new_pass), "Bad user");
        g.Close();
        // check disable, testing enable
        g = OpenExistingGalaxy(true);
        UT_EXPECT_THROW_REGEX(g.SetCurrentUser(new_user, new_pass), "Bad user");
        UT_EXPECT_TRUE(g.EnableUser(new_user));
        UT_EXPECT_THROW_REGEX(g.EnableUser(""), "Invalid user name");
        UT_EXPECT_TRUE(!g.EnableUser("non_existing_user"));
        g.SetCurrentUser(new_user, new_pass);
        g.Close();
        // check enable, and check non-admin user
        g = ExistingGalaxyWithNewUser(true);
        UT_EXPECT_THROW_REGEX(g.DisableUser(new_user), "cannot disable itself");
        UT_EXPECT_THROW_REGEX(g.DisableUser(admin_user), "Non-admin user cannot.*");
    }

    DefineTest("SetUserGraphAccess") {
        MarkTestBegin("SetUserGraphAccess");
        auto g = NewGalaxyWithNewUser();
        UT_EXPECT_ANY_THROW(g.OpenGraph("default"));  // no access
        g.SetCurrentUser(admin_user, admin_pass);
        g.SetUserGraphAccess(new_user, "default", AccessLevel::FULL);
        UT_EXPECT_THROW_REGEX(
            g.SetUserGraphAccess(new_user, "non_existing_graph", AccessLevel::FULL),
            "Graph .* does not exist");
        // validate that user have full access
        g.SetCurrentUser(new_user, new_pass);
        auto gr = g.OpenGraph("default");
        gr.DropAllData();
        // set no access
        g.SetCurrentUser(admin_user, admin_pass);
        g.SetUserGraphAccess(new_user, "default", AccessLevel::NONE);
        g.SetCurrentUser(new_user, new_pass);
        UT_EXPECT_ANY_THROW(g.OpenGraph("default"));  // no access
        UT_EXPECT_THROW_REGEX(g.SetUserGraphAccess(new_user, "default", AccessLevel::FULL),
                            "Non-admin user cannot");  // non-admin
    }

    DefineTest("SetUserRoles") {
        MarkTestBegin("SetUserRoles");
        auto Test = [&](const std::function<void(Galaxy&)>& setup,
                        const std::function<void(Galaxy&)>& validate) {
            auto g = NewGalaxyWithNewUser(false);
            setup(g);
            validate(g);
            g.Close();
            g = ExistingGalaxyWithNewUser(false);
            validate(g);
        };

        // setting roles without primary role
        Test([&](Galaxy& g) { g.SetUserRoles(new_user, {admin_role}); },
             [&](Galaxy& g) {
                 UT_EXPECT_TRUE(g.GetUserInfo(new_user).roles ==
                                std::set<std::string>({new_user, admin_role}));
             });

        // setting to empty roles
        Test([&](Galaxy& g) { g.SetUserRoles(new_user, {}); },
             [&](Galaxy& g) {
                 UT_EXPECT_TRUE(g.GetUserInfo(new_user).roles == std::set<std::string>({new_user}));
             });

        // setting admin roles
        Test([&](Galaxy& g) { g.SetUserRoles(admin_user, {}); },
             [&](Galaxy& g) {
                 UT_EXPECT_TRUE(g.GetUserInfo(admin_user).roles ==
                                std::set<std::string>({admin_role}));
             });

        Test([&](Galaxy& g) { g.SetUserRoles(admin_user, {new_user}); },
             [&](Galaxy& g) {
                 UT_EXPECT_TRUE(g.GetUserInfo(admin_user).roles ==
                                std::set<std::string>({admin_role, new_user}));
             });

        auto g = NewGalaxyWithNewUser(false);
        UT_EXPECT_THROW_REGEX(g.SetUserRoles(new_user, {new_role, "new_role2", "no_such_role"}),
                            "Role .* does not exist");
        UT_EXPECT_TRUE(g.SetUserRoles(new_user, {admin_role}));
        UT_EXPECT_TRUE(!g.SetUserRoles("non_existing_user", {admin_role}));
        auto uinfo = g.GetUserInfo(new_user);
        UT_EXPECT_TRUE(uinfo.roles == std::set<std::string>({admin_role, new_user}));
        UT_EXPECT_THROW_REGEX(g.SetUserRoles("@illegal_user", {}), "Invalid user name");
        UT_EXPECT_THROW_REGEX(g.SetUserRoles(new_user, {"@Illegal_role"}), "Invalid role name");
    }

    DefineTest("GetUserInfoListUsers") {
        MarkTestBegin("GetUserInfo/ListUsers");
        // get with admin
        auto g = NewGalaxyWithNewUser(false);
        auto uinfo = g.GetUserInfo(admin_user);
        UT_EXPECT_TRUE(!uinfo.disabled);
        UT_EXPECT_EQ(uinfo.roles.size(), 1);
        UT_EXPECT_TRUE(g.GetUserInfo(new_user).roles == std::set<std::string>({new_user}));
        UT_EXPECT_THROW_REGEX(g.GetUserInfo("non_existing_user"), "User.* does not exist");
        UT_EXPECT_THROW_REGEX(g.GetUserInfo(""), "Invalid user name");
        UT_EXPECT_EQ(g.ListUsers().size(), 2);
        g.Close();
        // get with non-admin
        g = NewGalaxyWithNewUser(true);
        UT_EXPECT_TRUE(!g.GetUserInfo(new_user).disabled);
        UT_EXPECT_THROW_REGEX(g.GetUserInfo(admin_user), "Non-admin user cannot.*");
        UT_EXPECT_THROW_REGEX(g.ListUsers(), "Non-admin user cannot.*");
    }

    DefineTest("CreateRole") {
        MarkTestBegin("CreateRole");
        // create with admin
        auto g = OpenNewGalaxy();
        UT_EXPECT_EQ(g.ListRoles().size(), 1);
        UT_EXPECT_TRUE(g.CreateRole(new_role, new_role_desc));
        UT_EXPECT_TRUE(!g.CreateRole(new_role, new_role_desc));    // already exists
        UT_EXPECT_TRUE(!g.CreateRole(admin_role, new_role_desc));  // already exists
        UT_EXPECT_THROW_REGEX(g.CreateRole("nnn", std::string(8192, 'a')),
                            "Role description too long.");
        UT_EXPECT_THROW_REGEX(g.CreateRole("", ""), "Invalid role name.");
        UT_EXPECT_THROW_REGEX(g.CreateRole("#invalid", ""), "Invalid role name.");
        UT_EXPECT_THROW_REGEX(g.CreateRole(std::string(1024, 'a'), ""), "Invalid role name.");
        UT_EXPECT_EQ(g.ListRoles().size(), 2);
        UT_EXPECT_EQ(g.ListRoles()[new_role].desc, new_role_desc);
        UT_EXPECT_EQ(g.ListRoles()[new_role].disabled, false);
        UT_EXPECT_TRUE(g.ListRoles()[new_role].graph_access.empty());
        g.Close();
        // test persistence
        g = OpenExistingGalaxy(true);
        UT_EXPECT_TRUE(!g.CreateRole(new_role, new_role_desc));  // already exists
        UT_EXPECT_EQ(g.ListRoles().size(), 2);
        g.Close();
        // create with non-admin
        g = NewGalaxyWithNewUser(true);
        UT_EXPECT_THROW_REGEX(g.CreateRole(new_role, new_role_desc), "Non-admin user cannot.*");
    }

    DefineTest("DeleteRole") {
        MarkTestBegin("DeleteRole");
        // delete with admin
        auto g = NewGalaxyWithNewRoleAndUser(false);
        UT_EXPECT_EQ(g.ListRoles().size(), 3);
        UT_EXPECT_TRUE(g.DeleteRole(new_role));
        UT_EXPECT_TRUE(!g.DeleteRole(new_role));                     // not exist
        UT_EXPECT_TRUE(!g.DeleteRole("non_existing_role"));          // not exist
        UT_EXPECT_THROW_REGEX(g.DeleteRole(""), "Invalid role name");  // not exist
        UT_EXPECT_THROW_REGEX(g.DeleteRole(admin_role), "Builtin roles cannot be deleted");
        g.Close();
        // test persistence
        g = OpenExistingGalaxy(true);
        UT_EXPECT_EQ(g.ListRoles().size(), 2);
        UT_EXPECT_TRUE(!g.DeleteRole(new_role));  // not exist
        g.Close();
        // delete with non-admin
        g = NewGalaxyWithNewRoleAndUser(true);
        UT_EXPECT_THROW_REGEX(g.DeleteRole(new_role), "Non-admin user cannot.*");
    }

    DefineTest("DeletePrimaryRole") {
        MarkTestBegin("DeletePrimaryRole");
        auto g = NewGalaxyWithNewUser(false);
        const std::string user2 = "user2";
        g.CreateUser(user2, "pass2");
        UT_EXPECT_THROW_REGEX(g.DeleteRole(user2), "primary role");
        g.SetUserRoles(new_user, {user2});
        UT_EXPECT_TRUE(g.GetUserInfo(new_user).roles == std::set<std::string>({new_user, user2}));
        g.DeleteUser(user2);
        // role "user2" still in use
        UT_EXPECT_TRUE(g.GetUserInfo(new_user).roles == std::set<std::string>({new_user, user2}));
        UT_EXPECT_TRUE(!g.GetRoleInfo(user2).disabled);
        // now role "user2" can be safely deleted
        UT_EXPECT_TRUE(g.DeleteRole(user2));
        auto roles = g.ListRoles();
        UT_EXPECT_TRUE(roles.find(user2) == roles.end());
        UT_EXPECT_TRUE(g.GetUserInfo(new_user).roles == std::set<std::string>({new_user}));
        // try deleting role "new_user"
        UT_EXPECT_TRUE(g.DeleteUser(new_user));
        roles = g.ListRoles();
        UT_EXPECT_TRUE(roles.find(new_user) == roles.end());
    }

    DefineTest("SetRoleDesc") {
        MarkTestBegin("SetRoleDesc");
        const std::string& modified_desc = "modified role desc";
        // set with admin
        auto g = NewGalaxyWithNewRoleAndUser(false);
        UT_EXPECT_TRUE(g.SetRoleDesc(new_role, modified_desc));  // ok
        UT_EXPECT_EQ(g.GetRoleInfo(new_role).desc, modified_desc);
        UT_EXPECT_TRUE(!g.SetRoleDesc("non_existing_role", ""));  // not found
        UT_EXPECT_THROW_REGEX(g.SetRoleDesc("", ""), "Invalid role name.");
        UT_EXPECT_THROW_REGEX(g.SetRoleDesc(admin_role, modified_desc),
                            "Builtin roles cannot be modified");  // builtin roles
        UT_EXPECT_THROW_REGEX(g.SetRoleDesc(new_role, std::string(8192, 'a')),
                            "Role description too long.");  // desc too long
        g.Close();
        // test persistence
        g = OpenExistingGalaxy(true);
        UT_EXPECT_EQ(g.GetRoleInfo(new_role).desc, modified_desc);
        g.Close();
        // set with non-admin
        g = ExistingGalaxyWithNewUser(true);
        UT_EXPECT_THROW_REGEX(g.SetRoleDesc(new_role, "must not succeed"),
                              "Non-admin user cannot.*");
    }

    DefineTest("SetRoleAccessRights") {
        MarkTestBegin("SetRoleAccessRights");
        // set with admin
        auto g = NewGalaxyWithNewRoleUserAndGraph(false);
        UT_EXPECT_TRUE(g.GetRoleInfo(new_role).graph_access[new_graph] ==
                       lgraph_api::AccessLevel::NONE);
        UT_EXPECT_TRUE(
            g.SetRoleAccessRights(new_role, {{new_graph, lgraph_api::AccessLevel::FULL}}));
        UT_EXPECT_TRUE(g.GetRoleInfo(new_role).graph_access[new_graph] ==
                       lgraph_api::AccessLevel::FULL);
        UT_EXPECT_TRUE(!g.SetRoleAccessRights(
            "non_existing_user", {{new_graph, lgraph_api::AccessLevel::FULL}}));  // not found
        UT_EXPECT_THROW_REGEX(
                       g.SetRoleAccessRights(new_user,
                                  {{"non_existing_graph", lgraph_api::AccessLevel::FULL}}),
                            "Graph .* does not exist");  // not found
        UT_EXPECT_THROW_REGEX(g.SetRoleAccessRights("", {}), "Invalid role name");
        UT_EXPECT_THROW_REGEX(
            g.SetRoleAccessRights(admin_role, {{new_graph, lgraph_api::AccessLevel::FULL}}),
            "Builtin roles cannot be modified");
        g.Close();
        // test persistence
        g = OpenExistingGalaxy(true);
        UT_EXPECT_TRUE(g.GetRoleInfo(new_role).graph_access[new_graph] ==
                       lgraph_api::AccessLevel::FULL);
        g.Close();
        // test with non-admin
        g = ExistingGalaxyWithNewUser(true);
        UT_EXPECT_THROW_REGEX(g.SetRoleAccessRights(new_role, {}), "Non-admin user cannot.*");
        auto gr = g.OpenGraph(new_graph);  // validate that user has full control of the graph
        gr.DropAllData();
    }

    DefineTest("GetRoleInfo") {
        MarkTestBegin("GetRoleInfo");
        // get with admin
        auto g = NewGalaxyWithNewRoleUserAndGraph(false);
        UT_EXPECT_TRUE(
            g.SetRoleAccessRights(new_role, {{new_graph, lgraph_api::AccessLevel::FULL}}));
        auto rinfo = g.GetRoleInfo(new_role);
        UT_EXPECT_EQ(rinfo.desc, new_role_desc);
        UT_EXPECT_TRUE(!rinfo.disabled);
        UT_EXPECT_EQ(rinfo.graph_access.size(), 1);
        UT_EXPECT_THROW_REGEX(g.GetRoleInfo("non_existing_role"), "Role .* does not exist");
        UT_EXPECT_THROW_REGEX(g.GetRoleInfo(""), "Invalid role name");
        g.Close();
        // get with non-admin
        g = ExistingGalaxyWithNewUser(true);
        UT_EXPECT_THROW_REGEX(g.GetRoleInfo(admin_role),
                            "User .* not .* admin .* not have .* specified role");
    }

    DefineTest("ListRoles") {
        MarkTestBegin("ListRoles");
        // list with admin
        auto g = NewGalaxyWithNewRoleUserAndGraph(false);
        auto roles = g.ListRoles();
        UT_EXPECT_EQ(roles.size(), 3);
        g.Close();
        // list with non-admin
        g = ExistingGalaxyWithNewUser(true);
        UT_EXPECT_THROW_REGEX(g.ListRoles(), "Non-admin user.*");
    }

    DefineTest("DisableEnableRole") {
        MarkTestBegin("Disable/EnableRole");
        auto g = NewGalaxyWithNewRoleUserAndGraph(false);
        UT_EXPECT_TRUE(g.DisableRole(new_role));
        UT_EXPECT_TRUE(g.GetRoleInfo(new_role).disabled);
        UT_EXPECT_TRUE(!g.DisableRole("non_existing_role"));  // not found
        UT_EXPECT_THROW_REGEX(g.DisableRole(""), "Invalid role name");
        g.Close();
        // test persistence
        g = ExistingGalaxyWithNewUser(false);
        UT_EXPECT_TRUE(g.GetRoleInfo(new_role).disabled);
        UT_EXPECT_TRUE(g.EnableRole(new_role));
        UT_EXPECT_TRUE(!g.EnableRole("non_existing_role"));  // not found
        UT_EXPECT_THROW_REGEX(g.EnableRole(""), "Invalid role name");
    }

    DefineTest("Issue126") {
        MarkTestBegin("Issue126");
        auto g = NewGalaxyWithNewUser(false);
        UT_EXPECT_THROW_REGEX(g.DeleteRole(new_user), "primary role");
        g.SetUserRoles(new_user, {admin_role});
    }

    DefineTest("Issue149") {
        MarkTestBegin("Issue149");
        auto g = OpenNewGalaxy();
        lgraph_api::GraphDB db = g.OpenGraph("default");

        std::string node_label = "data";
        std::vector<std::string> node_schema = {
            "guid", "name", "entity_type", "props", "last_access_time", "gdb_timestamp", "deleted"};
        // Add Label
        std::vector<lgraph_api::FieldSpec> fds;
        fds.push_back(lgraph_api::FieldSpec("guid", lgraph_api::FieldType::STRING, false));
        fds.push_back(lgraph_api::FieldSpec("name", lgraph_api::FieldType::STRING, false));
        fds.push_back(lgraph_api::FieldSpec("entity_type", lgraph_api::FieldType::STRING, false));
        fds.push_back(lgraph_api::FieldSpec("props", lgraph_api::FieldType::STRING, false));
        fds.push_back(
            lgraph_api::FieldSpec("last_access_time", lgraph_api::FieldType::INT64, false));
        fds.push_back(lgraph_api::FieldSpec("gdb_timestamp", lgraph_api::FieldType::INT64, false));
        fds.push_back(lgraph_api::FieldSpec("deleted", lgraph_api::FieldType::INT64, false));
        db.AddVertexLabel("data", fds, VertexOptions("guid"));
        fds.resize(0);
        fds.push_back(lgraph_api::FieldSpec("props", lgraph_api::FieldType::STRING, false));
        fds.push_back(
            lgraph_api::FieldSpec("last_access_time", lgraph_api::FieldType::INT64, false));
        fds.push_back(lgraph_api::FieldSpec("gdb_timestamp", lgraph_api::FieldType::INT64, false));
        fds.push_back(lgraph_api::FieldSpec("deleted", lgraph_api::FieldType::INT64, false));
        db.AddEdgeLabel("da_depend_field", fds, {});

        auto Merge = [&](lgraph_api::Transaction& txn, const std::string& guid,
                         const std::string& name, const std::string& entity_type,
                         const std::string& props, int64_t last_access_time, int64_t gdb_timestamp,
                         int64_t deleted) -> int {
            auto vit = txn.GetVertexIterator();
            auto iit = txn.GetVertexIndexIterator(node_label, node_schema[0], FieldData(guid),
                                                  FieldData(guid));
            std::vector<FieldData> field_data;
            field_data.emplace_back(guid);
            field_data.emplace_back(name);
            field_data.emplace_back(entity_type);
            field_data.emplace_back(props);
            field_data.emplace_back(last_access_time);
            field_data.emplace_back(gdb_timestamp);
            field_data.emplace_back(deleted);
            if (iit.IsValid()) {
                vit.Goto(iit.GetVid());
                auto ts = vit.GetField(node_schema[5]).AsInt64();
                if (gdb_timestamp > ts) {
                    vit.SetFields(node_schema, field_data);
                    return 0;
                }
                return 1;
            }
            txn.AddVertex(node_label, node_schema, field_data);
            return 2;
        };

        {
            auto txn = db.CreateWriteTxn();
            Merge(txn, "d2.104817823", "001", "001", "001", 10, 11, 0);
            Merge(txn, "004", "001", "001", "001", 10, 11, 0);
        }
    }

    DefineTest("ACLComplex") {
        MarkTestBegin("ACL complex");
        auto g = NewGalaxyWithNewRoleUserAndGraph(false);
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::NONE);
        UT_EXPECT_TRUE(
            g.SetRoleAccessRights(new_role, {{new_graph, lgraph_api::AccessLevel::WRITE}}));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::WRITE);
        UT_EXPECT_TRUE(g.SetUserRoles(new_user, {}));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::NONE);
        UT_EXPECT_TRUE(g.CreateRole("new_role2", ""));
        UT_EXPECT_TRUE(
            g.SetRoleAccessRights("new_role2", {{new_graph, lgraph_api::AccessLevel::FULL}}));
        UT_EXPECT_TRUE(g.SetUserRoles(new_user, {new_role, "new_role2"}));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::FULL);
        UT_EXPECT_TRUE(g.DisableRole("new_role2"));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::WRITE);
        UT_EXPECT_TRUE(g.DisableRole(new_role));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::NONE);
        UT_EXPECT_TRUE(g.EnableRole(new_role));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::WRITE);
        UT_EXPECT_TRUE(g.SetUserRoles(new_user, {new_role, "new_role2", admin_role}));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::FULL);
        g.Close();
        g = ExistingGalaxyWithNewUser(true);
        // now new_user has admin rights
        UT_EXPECT_TRUE(g.DeleteRole(new_role));
        UT_EXPECT_TRUE(g.EnableRole("new_role2"));
        UT_EXPECT_TRUE(g.GetAccessLevel(new_user, new_graph) == lgraph_api::AccessLevel::FULL);
        UT_EXPECT_THROW_REGEX(g.SetUserRoles(new_user, {}), "User cannot remove itself from.*");
    }

    {
        MarkTestBegin("snapshot");
        fma_common::file_system::RemoveDir(testdir);
        { lgraph::Galaxy galaxy("./testdb", true); }
        lgraph::Galaxy galaxy("./testdb", false);
        auto graphs = galaxy.ListGraphs("admin");
        UT_LOG() << fma_common::ToString(graphs);
        auto check_graphs_eq = [](const std::map<std::string, lgraph::DBConfig>& lhs,
                                  const std::map<std::string, lgraph::DBConfig>& rhs) {
            UT_EXPECT_EQ(lhs.size(), rhs.size());
            for (auto& kv : lhs) {
                auto kv2 = rhs.find(kv.first);
                UT_EXPECT_TRUE(kv2 != rhs.end());
                UT_EXPECT_EQ(kv.second.db_size, kv2->second.db_size);
                UT_EXPECT_EQ(kv.second.name, kv2->second.name);
            }
        };
        {
            UT_LOG() << "\tTesting with empty snapshot dir";
            // start with empty snapshot dir
            fma_common::file_system::RemoveDir("./snapshot");
            UT_EXPECT_TRUE(!galaxy.SaveSnapshot("snapshot").empty());
#ifndef _WIN32
            lgraph::Galaxy g2("./snapshot", false);
            auto graphs2 = g2.ListGraphs("admin");
            check_graphs_eq(graphs, graphs2);
            lgraph::DBConfig conf;
            conf.db_size = 1 << 30;
            g2.CreateGraph("admin", "should_not_exist", conf);
#endif
        }
        {
            UT_LOG() << "\tTesting with existing snapshot dir";
            // start with non-empty snapshot dir
            UT_EXPECT_TRUE(!galaxy.SaveSnapshot("snapshot").empty());
#ifndef _WIN32
            lgraph::Galaxy g2("./snapshot", false);
            auto graphs2 = g2.ListGraphs("admin");
            check_graphs_eq(graphs, graphs2);
#endif
        }
    }
}
