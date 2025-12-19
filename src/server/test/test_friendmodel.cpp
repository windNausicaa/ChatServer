#include <gtest/gtest.h>
#include "friendmodel.hpp"
#include "user.hpp"
#include "db.h"

// 测试插入好友
TEST(FriendModelTest, Insert) {
    FriendModel model;
    int userId = 1;     // 假设数据库有user id=1
    int friendId = 2;   // 假设有friend id=2

    model.insert(userId, friendId);

    // 检查是否插入成功
    std::vector<User> friends = model.query(userId);
    bool found = false;
    for (User& f : friends) {
        if (f.getId() == friendId) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "好友没有插入成功！";

    // 清理测试数据
    MySQL mysql;
    if (mysql.connect()) {
        char sql[1024] = {0};
        sprintf(sql, "DELETE FROM Friend WHERE userid = %d AND friendid = %d", userId, friendId);
        mysql.update(sql);
    }
}

// 测试查询好友列表
TEST(FriendModelTest, Query) {
    FriendModel model;
    int userId = 1;

    // 先插入测试数据
    model.insert(userId, 3);

    std::vector<User> friends = model.query(userId);
    ASSERT_FALSE(friends.empty()) << "没有查询到好友！";
    EXPECT_EQ(friends[0].getId(), 3) << "好友ID不对！";

    // 清理
    MySQL mysql;
    if (mysql.connect()) {
        char sql[1024] = {0};
        sprintf(sql, "DELETE FROM Friend WHERE userid = %d AND friendid = 3", userId);
        mysql.update(sql);
    }
}