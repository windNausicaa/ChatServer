#include <gtest/gtest.h>
#include "usermodel.hpp"
#include "user.hpp"
#include "offlinemessagemodel.hpp"
#include "db.h"

TEST(UserModelTest, InsertAndQuery) {
    UserModel model;
    User testUser;
    testUser.setName("testuser");
    testUser.setPwd("testpwd");
    testUser.setState("offline");

    bool inserted = model.insert(testUser);
    ASSERT_TRUE(inserted) << "插入用户失败！";

    User queriedUser = model.query(testUser.getId());
    EXPECT_EQ(queriedUser.getName(), "testuser") << "用户名不对！";
    EXPECT_EQ(queriedUser.getPwd(), "testpwd") << "密码不对！";

    // 清理
    MySQL mysql;
    if (mysql.connect()) {
        char sql[1024] = {0};
        sprintf(sql, "DELETE FROM User WHERE id = %d", testUser.getId());
        mysql.update(sql);
    }
}

TEST(OfflineMsgModelTest, InsertAndQuery) {
    OfflineMsgModel model;
    int userId = 1;
    std::string msg = "test offline msg";

    model.insert(userId, msg);

    std::vector<std::string> msgs = model.query(userId);
    ASSERT_FALSE(msgs.empty()) << "没有离线消息！";
    EXPECT_EQ(msgs[0], msg) << "离线消息内容不对！";

    model.remove(userId);
}