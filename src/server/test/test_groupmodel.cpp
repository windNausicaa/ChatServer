#include <gtest/gtest.h>
#include "groupmodel.hpp"
#include "group.hpp"
#include "groupuser.hpp"
#include "user.hpp"
#include "db.h"

TEST(GroupModelTest, CreateGroup) {
    GroupModel model;
    Group testGroup;
    testGroup.setName("testgroup");
    testGroup.setDesc("test desc");

    bool created = model.createGroup(testGroup);
    ASSERT_TRUE(created) << "创建群失败！";
    EXPECT_GT(testGroup.getId(), 0) << "群ID无效！";

    // 清理
    MySQL mysql;
    if (mysql.connect()) {
        char sql[1024] = {0};
        sprintf(sql, "DELETE FROM AllGroup WHERE id = %d", testGroup.getId());
        mysql.update(sql);
    }
}

TEST(GroupModelTest, AddGroup) {
    GroupModel model;
    int userId = 1;
    int groupId = 1;  // 假设存在群id=1
    std::string role = "normal";

    model.addGroup(userId, groupId, role);

    std::vector<int> users = model.queryGroupUsers(userId + 100, groupId);
    bool found = false;
    for (int id : users) {
        if (id == userId) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "没有加入群！";

    // 清理
    MySQL mysql;
    if (mysql.connect()) {
        char sql[1024] = {0};
        sprintf(sql, "DELETE FROM GroupUser WHERE groupid = %d AND userid = %d", groupId, userId);
        mysql.update(sql);
    }
}

TEST(GroupModelTest, QueryGroups) {
    GroupModel model;
    int userId = 1;

    Group tempGroup;
    tempGroup.setName("querytestgroup");
    tempGroup.setDesc("query desc");
    model.createGroup(tempGroup);
    model.addGroup(userId, tempGroup.getId(), "normal");

    std::vector<Group> groups = model.queryGroups(userId);
    ASSERT_FALSE(groups.empty()) << "没有查到群！";
    EXPECT_EQ(groups[0].getName(), "querytestgroup") << "群名不对！";

    // 清理
    MySQL mysql;
    if (mysql.connect()) {
        char sql1[1024] = {0};
        sprintf(sql1, "DELETE FROM GroupUser WHERE groupid = %d", tempGroup.getId());
        mysql.update(sql1);

        char sql2[1024] = {0};
        sprintf(sql2, "DELETE FROM AllGroup WHERE id = %d", tempGroup.getId());
        mysql.update(sql2);
    }
}

TEST(GroupModelTest, QueryGroupUsers) {
    GroupModel model;
    int userId = 1;
    int groupId = 1;

    model.addGroup(2, groupId, "normal");

    std::vector<int> users = model.queryGroupUsers(userId, groupId);
    EXPECT_FALSE(users.empty()) << "没有成员！";
    bool found = false;
    for (int id : users) {
        if (id == 2) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "没有查到成员2！";

    // 清理
    MySQL mysql;
    if (mysql.connect()) {
        char sql[1024] = {0};
        sprintf(sql, "DELETE FROM GroupUser WHERE groupid = %d AND userid = 2", groupId);
        mysql.update(sql);
    }
}