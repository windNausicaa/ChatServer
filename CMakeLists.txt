cmake_minimum_required(VERSION 3.20)
project(ChatServer LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

# --- 2. 配置编译选项 ---
# 添加调试信息 -g 和警告
add_compile_options(-Wall -Wextra -g)

# --- 3. 配置路径 ---
# 配置最终的可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/server
    ${PROJECT_SOURCE_DIR}/include/server/db
    ${PROJECT_SOURCE_DIR}/include/server/model
    ${PROJECT_SOURCE_DIR}/include/server/redis
    ${PROJECT_SOURCE_DIR}/thirdparty
)

# 库文件搜索路径 (MySQL)
link_directories(/usr/lib64/mysql)

# --- 4. 查找依赖 ---
# OpenSSL 安全防护
find_package(OpenSSL REQUIRED)

# --- 5. 加载子目录 (核心逻辑) ---
# 注意：这一步必须在 target_link_libraries 之前
# 假设 add_executable(ChatServer ...) 是在 src/CMakeLists.txt 中定义的
add_subdirectory(src)


target_link_libraries(ChatServer OpenSSL::SSL OpenSSL::Crypto)