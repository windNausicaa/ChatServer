# ChatServer

# 基于Muduo网络库的集群聊天服务器

[![Build Status](https://github.com/your_username/ChatClusterServer/actions/workflows/ci.yml/badge.svg)](https://github.com/your_username/ChatClusterServer/actions)  <!-- CI徽章，从GitHub Actions抄 -->

## 项目概述
这是一个C++实现的分布式聊天系统，支持用户注册/登录、好友/群聊、离线消息。使用Muduo处理网络、Nginx负载均衡、Redis发布订阅、MySQL存储。适合学习网络编程和分布式系统。

### 技术栈（从你的PDF）
- Json序列化和反序列化（nlohmann/json）
- Muduo网络库（Reactor模型，多线程）
- Nginx TCP负载均衡（一致性哈希）
- Redis中间件（发布-订阅）
- MySQL数据库编程
- CMake构建、GitHub管理

### 架构图
![Muduo Reactor模型](docs/reactor_diagram.png)  <!-- 你用draw.io画PDF第9页的图，存docs/ -->
![集群设计](docs/cluster_diagram.png)  <!-- 画PDF第12页的Redis集群图 -->

## 安装和运行
1. 克隆仓库：`git clone https://github.com/your_username/ChatClusterServer.git`
2. 安装依赖：`sudo yum install muduo nginx redis mysql cmake`
3. 构建：`cmake . && make`
4. 配置Nginx：复制nginx.conf到/etc/nginx/
5. 运行：`docker-compose up`（见docker-compose.yml）
6. 客户端：跑./bin/ChatClient 127.0.0.1 8000
7. 服务端：跑./bin/ChatServer 127.0.0.1 6000 / ./bin/ChatServer 127.0.0.1 6002

### 功能演示
![登录截屏](docs/login_screenshot.png)  <!-- 用手机拍命令行或Qt界面 -->
![聊天GIF](docs/chat_demo.gif)  <!-- 用screenrecorder录视频转GIF -->
