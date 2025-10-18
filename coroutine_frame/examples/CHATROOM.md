# 💬 WebSocket 聊天室

基于 C++23 协程的实时多用户聊天室，支持用户昵称、在线列表、消息广播等功能。

## ✨ 功能特性

- 👥 **多用户聊天**：支持多个用户同时在线
- 📝 **自定义昵称**：用户可以设置自己的昵称
- 📊 **在线用户列表**：实时显示所有在线用户
- 🔔 **加入/离开通知**：用户进出聊天室时自动通知
- 💬 **实时消息**：消息即时广播给所有用户
- 🎨 **漂亮的 UI**：现代化的聊天界面

## 🚀 快速开始

### 1. 编译服务器

```bash
cd build
cmake ..
ninja websocket_server
```

### 2. 启动服务器

```bash
./websocket_server

# 输出:
╔══════════════════════════════════════════╗
║  💬 WebSocket Chat Room (C++23)         ║
╚══════════════════════════════════════════╝
🚀 Server listening on ws://localhost:8080
📝 HTTP UI: http://localhost:8080
Press Ctrl+C to stop
```

### 3. 打开聊天室

**方法 1：使用浏览器（推荐）**

用浏览器打开 `examples/chatroom.html`，或者直接访问 `http://localhost:8080`

**方法 2：多个浏览器窗口测试**

1. 打开第一个浏览器窗口，输入昵称 "Alice"
2. 打开第二个浏览器窗口，输入昵称 "Bob"
3. 开始聊天！

## 📸 界面展示

```
┌─────────────────────────────────────────────────┐
│  Sidebar        │  Chat Area                    │
│  ┌───────────┐  │  ┌─────────────────────────┐ │
│  │ 💬 Chat   │  │  │ 🚀 Welcome to Chat Room │ │
│  │ Room      │  │  └─────────────────────────┘ │
│  │           │  │                               │
│  │ ● Connected│ │  [Messages Area]             │
│  ├───────────┤  │  Alice: Hello!               │
│  │ 👤 Alice  │  │  Bob: Hi there!              │
│  │ 👤 Bob    │  │  System: Charlie joined      │
│  │ 👤 Charlie│  │                               │
│  ├───────────┤  │  [Input]                     │
│  │ 3 users   │  │  Type message... [Send]      │
│  │ online    │  │                               │
│  └───────────┘  │                               │
└─────────────────────────────────────────────────┘
```

## 🔧 协议说明

### 消息格式

所有消息使用 **JSON** 格式：

#### 1. 系统消息
```json
{
    "type": "system",
    "message": "Welcome to Chat Room!"
}
```

#### 2. 普通聊天消息
```json
{
    "type": "message",
    "user": "Alice",
    "message": "Hello everyone!"
}
```

#### 3. 用户加入
```json
{
    "type": "join",
    "user": "Bob"
}
```

#### 4. 用户离开
```json
{
    "type": "leave",
    "user": "Charlie"
}
```

#### 5. 用户列表更新
```json
{
    "type": "userlist",
    "users": ["Alice", "Bob", "Charlie"]
}
```

## 🎮 使用流程

### 客户端连接流程

```
1. 连接 WebSocket
   ws://localhost:8080
   
2. 收到欢迎消息
   {"type":"system","message":"Welcome..."}
   
3. 发送昵称
   → "Alice"
   
4. 收到确认
   ← {"type":"system","message":"Welcome, Alice!"}
   
5. 收到用户列表
   ← {"type":"userlist","users":["Alice"]}
   
6. 发送聊天消息
   → "Hello everyone!"
   
7. 所有人收到消息
   ← {"type":"message","user":"Alice","message":"Hello everyone!"}
```

## 📊 服务器日志

```bash
[ACCEPT] New connection - FD: 4
[CHAT] User registered: Alice (FD: 4)
[CHAT] Alice: Hello everyone!
[ACCEPT] New connection - FD: 5
[CHAT] User registered: Bob (FD: 5)
[CHAT] Bob: Hi Alice!
[CHAT] Alice left the chat
```

## 🏗️ 架构说明

### 并发模型

```
每个用户 → 独立协程 (task<void>)
           ↓
    协程调度器 (executor)
           ↓
    线程池 (4 workers)
           ↓
    异步 I/O (async_read/write)
```

### 数据结构

```cpp
struct ChatUser {
    int fd;                      // Socket 文件描述符
    std::string nickname;        // 用户昵称
    std::chrono::time_point join_time;  // 加入时间
};

std::vector<ChatUser> chat_users;  // 所有在线用户
std::mutex users_mutex;            // 线程安全锁
```

### 关键函数

```cpp
// 处理 WebSocket 客户端
task<void> handle_websocket_client(int client_fd);

// 广播消息给所有用户
task<void> broadcast_message(const std::string& message, int exclude_fd = -1);

// 广播用户列表更新
task<void> broadcast_user_list();
```

## 🎯 测试场景

### 场景 1：基本聊天

1. Alice 加入聊天室
2. Bob 加入聊天室
3. Alice 发送: "Hello Bob!"
4. Bob 收到消息并回复: "Hi Alice!"
5. 双方都能看到对方的消息

### 场景 2：多人聊天

1. 打开 3 个浏览器窗口
2. 分别以 Alice、Bob、Charlie 的身份加入
3. 任何人发送的消息，所有人都能看到
4. 侧边栏显示 3 个在线用户

### 场景 3：用户进出

1. Alice 已在线
2. Bob 加入 → Alice 收到 "Bob joined the chat"
3. Charlie 加入 → Alice 和 Bob 都收到通知
4. Bob 离开 → Alice 和 Charlie 收到 "Bob left the chat"

## 🔍 调试技巧

### 1. 浏览器开发者工具

按 F12 打开控制台，查看 WebSocket 消息：

```javascript
// 查看所有 WebSocket 消息
// Network → WS → Messages
```

### 2. 命令行测试

使用 `websocat` 测试：

```bash
# 终端 1
./websocket_server

# 终端 2
websocat ws://localhost:8080
> Alice                    # 输入昵称
< {"type":"system","message":"Welcome, Alice!"}
> Hello from terminal!     # 发送消息
```

### 3. 多客户端测试

```bash
# 同时启动多个 websocat
websocat ws://localhost:8080 &
websocat ws://localhost:8080 &
websocat ws://localhost:8080 &
```

## 🚀 性能特点

- **异步非阻塞**：所有 I/O 操作都是异步的
- **协程驱动**：每个用户一个轻量级协程
- **低延迟**：消息广播几乎实时
- **高并发**：理论支持上千并发连接

## 💡 扩展想法

### 功能扩展

1. **私聊功能**
   ```json
   {"type":"private","to":"Bob","message":"Secret!"}
   ```

2. **聊天室房间**
   ```json
   {"type":"join_room","room":"general"}
   ```

3. **消息历史**
   - 保存最近 100 条消息
   - 新用户加入时发送历史记录

4. **表情支持**
   ```
   :smile: → 😊
   :heart: → ❤️
   ```

5. **文件分享**
   - Base64 编码小文件
   - 文件上传/下载链接

6. **在线状态**
   ```json
   {"type":"status","user":"Alice","status":"typing"}
   ```

### 性能优化

1. **消息队列**
   - 批量发送消息
   - 减少系统调用

2. **连接池**
   - 复用 WebSocket 连接
   - 减少握手开销

3. **压缩**
   - WebSocket permessage-deflate
   - 减少带宽使用

## 📝 常见问题

### Q: 为什么用户列表没更新？
A: 检查浏览器控制台，确保收到 `userlist` 消息。

### Q: 消息发送失败？
A: 确保 WebSocket 已连接（状态显示 "● Connected"）。

### Q: 支持多少用户？
A: 理论上支持上千用户，实际取决于服务器性能。

### Q: 消息有延迟吗？
A: 几乎没有延迟，消息通过协程立即广播。

## 📚 相关文档

- [WebSocket Protocol RFC 6455](https://tools.ietf.org/html/rfc6455)
- [C++20 Coroutines](https://en.cppreference.com/w/cpp/language/coroutines)
- [JSON Format](https://www.json.org/)

## 🎉 开始聊天吧！

启动服务器，打开 `chatroom.html`，邀请朋友一起聊天！

```bash
./websocket_server
# 然后打开浏览器访问 http://localhost:8080
```

Have fun! 💬🚀
