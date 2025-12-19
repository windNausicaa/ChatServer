add_subdirectory(server)
add_subdirectory(client)

# 添加测试文件
find_package(GTest REQUIRED)

set(TEST_SOURCES
    server/test/test_usermodel.cpp
    server/test/test_friendmodel.cpp
    server/test/test_groupmodel.cpp
    server/db/db.cpp
    server/model/usermodel.cpp
    server/model/friendmodel.cpp
    server/model/groupmodel.cpp
    server/model/offlinemessagemodel.cpp
)

add_executable(unit_tests  ${TEST_SOURCES})  # 新测试文件

find_package(GTest REQUIRED)

target_link_libraries(unit_tests GTest::GTest GTest::Main
    mysqlclient  
    hiredis      
    muduo_net 
    muduo_base  
    pthread      
    OpenSSL::SSL OpenSSL::Crypto 
)