# Async Coroutine HTTP Server Example

这是一个基于 C++23 协程库构建的异步 HTTP 服务器示例，展示了如何在实际应用中使用我们的协程库。

## 特性

- ✨ **真正的异步处理**: 每个请求在协程中处理，不阻塞其他请求
- 🚀 **高并发**: 利用线程池处理多个并发请求
- 🎯 **简洁的 API**: 使用 `co_await` 实现异步 I/O 操作
- 🔄 **路由系统**: 支持多个 HTTP 路由和处理器
- 💾 **异步数据库**: 模拟异步数据库查询
- 🌐 **外部 API 调用**: 模拟异步 API 调用
- ⚡ **非阻塞**: 慢请求不会阻塞快请求

## 编译和运行

### 编译

```bash
cd /home/asice-cloud/projects/cppp/utils/task_do/build
cmake ..
cmake --build .
```

或者使用 Clang：

```bash
clang++ -std=c++23 -I.. \
    ../examples/http_server.cpp \
    ../task.cpp \
    ../executor.cpp \
    -lpthread -o http_server
```

### 运行

```bash
# 默认端口 8080
./http_server

# 指定端口
./http_server 9000
```

## 测试 API

服务器启动后，可以使用 curl 或浏览器测试：

### 1. 首页（HTML）

```bash
curl http://localhost:8080/
```

在浏览器中打开 `http://localhost:8080/` 可以看到一个简单的欢迎页面。

### 2. Hello API

```bash
curl http://localhost:8080/api/hello
```

返回：
```json
{
  "message": "Hello from async coroutine server!",
  "timestamp": "1729328400"
}
```

### 3. 数据库查询（模拟）

```bash
curl http://localhost:8080/api/db
```

模拟 50ms 的数据库延迟，返回：
```json
{
  "status": "success",
  "data": "Database result for: SELECT * FROM users"
}
```

### 4. 外部 API 调用（模拟）

```bash
curl http://localhost:8080/api/external
```

模拟 100ms 的网络延迟，返回：
```json
{
  "data": "API response from https://api.example.com/data"
}
```

### 5. 慢请求测试

```bash
curl http://localhost:8080/api/slow
```

等待 2 秒后返回：
```json
{
  "message": "Slow operation completed",
  "duration": "2s"
}
```

## 并发测试

使用多个并发请求测试服务器性能：

```bash
# 使用 ApacheBench (ab)
ab -n 1000 -c 10 http://localhost:8080/api/hello

# 使用 curl 并发
for i in {1..10}; do
    curl http://localhost:8080/api/slow &
done
wait
```

观察服务器输出，你会看到：
- 多个请求在不同的工作线程上并发处理
- 慢请求不会阻塞快请求
- 线程 ID 显示请求在线程池中的分布

## 架构说明

### 核心组件

1. **HttpRequest / HttpResponse**
   - 简单的 HTTP 请求/响应结构体
   - `HttpResponse::to_string()` 生成符合 HTTP/1.1 标准的响应

2. **异步 I/O 操作**
   ```cpp
   task async_read(int socket_fd, char* buffer, size_t size);
   task async_write(int socket_fd, const std::string& data);
   ```
   - 在协程中执行 socket I/O
   - 使用 `co_await` 实现非阻塞

3. **路由处理器**
   ```cpp
   task handle_home();
   task handle_hello();
   task handle_db();
   task handle_external();
   task handle_slow();
   task handle_not_found(const std::string& path);
   ```
   - 每个路由都是一个协程函数
   - 可以使用 `co_await` 调用其他异步操作

4. **请求分发器**
   ```cpp
   task handle_request(const HttpRequest& req);
   ```
   - 根据路径路由到相应的处理器
   - 打印请求信息和线程 ID

5. **连接处理**
   ```cpp
   task handle_client(int client_fd);
   ```
   - 异步读取请求
   - 异步处理请求
   - 异步发送响应

### 异步执行流程

```
客户端连接
    ↓
accept() - 主线程
    ↓
async_run(handle_client) - 提交到线程池
    ↓
工作线程接管
    ↓
async_read() - 读取请求数据
    ↓
handle_request() - 路由分发
    ↓
handle_xxx() - 具体处理器
    ↓  (可能包含)
query_database() / call_external_api() - 更多异步操作
    ↓
async_write() - 发送响应
    ↓
关闭连接
```

## 代码亮点

### 1. 真正的异步处理

```cpp
task handle_db() {
    co_await schedule_on(get_global_executor());
    
    // 异步数据库查询 - 不阻塞其他请求
    std::string db_result = co_await query_database("SELECT * FROM users");
    
    HttpResponse response;
    response.body = R"({"data": ")" + db_result + "\"}";
    co_return response;
}
```

### 2. 组合多个异步操作

```cpp
task handle_complex() {
    co_await schedule_on(get_global_executor());
    
    // 串行执行多个异步操作
    auto user = co_await query_database("SELECT * FROM users WHERE id=1");
    auto posts = co_await query_database("SELECT * FROM posts WHERE user_id=1");
    auto profile = co_await call_external_api("/profile/1");
    
    // 组合结果
    co_return build_response(user, posts, profile);
}
```

### 3. Fire-and-Forget 模式

```cpp
// 主线程不会等待连接处理完成
async_run(handle_client(client_fd));
```

## 扩展建议

### 1. 真实的异步 I/O

当前示例使用同步 socket 调用 + 线程池模拟异步。可以集成：
- **io_uring** (Linux)
- **epoll/kqueue**
- **ASIO** 库

### 2. 并行执行

实现 `when_all` 模式同时执行多个独立操作：

```cpp
task handle_parallel() {
    auto [user, posts, profile] = co_await when_all(
        query_database("SELECT * FROM users WHERE id=1"),
        query_database("SELECT * FROM posts WHERE user_id=1"),
        call_external_api("/profile/1")
    );
    co_return build_response(user, posts, profile);
}
```

### 3. 中间件系统

```cpp
task auth_middleware(HttpRequest& req) {
    if (!validate_token(req.headers["Authorization"])) {
        co_return HttpResponse{401, "Unauthorized"};
    }
    co_return std::nullopt; // Continue to handler
}
```

### 4. WebSocket 支持

添加 WebSocket 协议支持，实现双向通信。

### 5. 静态文件服务

```cpp
task serve_static_file(const std::string& path) {
    co_await schedule_on(get_global_executor());
    auto content = co_await async_read_file(path);
    co_return HttpResponse{200, "OK", {}, content};
}
```

## 性能说明

- **线程池大小**: 默认 4 个工作线程（在 `executor.cpp` 中配置）
- **连接队列**: 默认 10（在 `listen()` 调用中设置）
- **缓冲区大小**: 4KB（可根据需要调整）

### 调优建议

1. 根据 CPU 核心数调整线程池大小
2. 使用连接池管理数据库连接
3. 实现请求超时机制
4. 添加请求限流和负载保护

## 总结

这个示例展示了：
- ✅ 如何使用协程构建实用的异步应用
- ✅ 协程如何简化异步代码的编写
- ✅ 线程池执行器的实际应用
- ✅ 异步操作的组合和编排

相比传统的回调或 promise 链，协程让异步代码看起来像同步代码，但具有完全的异步性能！
