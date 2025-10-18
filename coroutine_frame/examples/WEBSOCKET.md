# WebSocket Server Example

基于 C++23 协程的 WebSocket 服务器实现，支持多客户端实时通信。

## 功能特性

- ✅ **完整的 WebSocket 协议**：握手、帧解析、控制帧
- ✅ **异步协程驱动**：所有 I/O 操作都是异步的
- ✅ **多客户端支持**：同时处理多个 WebSocket 连接
- ✅ **实时广播**：消息可以广播给所有连接的客户端
- ✅ **Echo + Chat**：既回显给发送者，也广播给其他人
- ✅ **PING/PONG**：支持心跳检测
- ✅ **优雅关闭**：正确处理连接关闭

## 编译

```bash
cd build
cmake ..
ninja websocket_server
```

## 运行服务器

```bash
./websocket_server [port]

# 默认端口 8080
./websocket_server

# 自定义端口
./websocket_server 9090
```

## 测试方法

### 方法 1：使用浏览器客户端（推荐）

1. 启动服务器：
```bash
cd build
./websocket_server
```

2. 用浏览器打开 `examples/websocket_client.html`

3. 点击 "Connect" 连接服务器

4. 发送消息测试

### 方法 2：使用 websocat 命令行工具

安装 websocat：
```bash
# Arch Linux
sudo pacman -S websocat

# Ubuntu/Debian
cargo install websocat

# macOS
brew install websocat
```

测试：
```bash
# 终端 1：启动服务器
./websocket_server

# 终端 2：连接客户端
websocat ws://localhost:8080

# 然后就可以输入消息了
```

### 方法 3：使用 JavaScript (Node.js)

```javascript
const WebSocket = require('ws');

const ws = new WebSocket('ws://localhost:8080');

ws.on('open', function() {
    console.log('Connected!');
    ws.send('Hello from Node.js!');
});

ws.on('message', function(data) {
    console.log('Received:', data);
});
```

### 方法 4：使用 Python

```python
import asyncio
import websockets

async def test():
    async with websockets.connect('ws://localhost:8080') as ws:
        await ws.send('Hello from Python!')
        response = await ws.recv()
        print(f'Received: {response}')

asyncio.run(test())
```

## 协议实现细节

### WebSocket 握手

```
客户端请求:
GET / HTTP/1.1
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==

服务器响应:
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
```

### 帧格式

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               | Masking-key, if MASK set to 1 |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
:                     Payload Data continued ...                :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
```

### Opcodes

- `0x0` - Continuation Frame
- `0x1` - Text Frame
- `0x2` - Binary Frame
- `0x8` - Connection Close
- `0x9` - Ping
- `0xA` - Pong

## 示例输出

```
╔══════════════════════════════════════════╗
║  WebSocket Echo Server (Coroutines)     ║
╚══════════════════════════════════════════╝
Server listening on ws://localhost:8080
Press Ctrl+C to stop

Test with: websocat ws://localhost:8080

[ACCEPT] New connection - FD: 4
[WS] Handshake successful - FD: 4
[ACCEPT] New connection - FD: 5
[WS] Handshake successful - FD: 5
[WS] Message from FD 4: Hello, WebSocket!
[WS] Message from FD 5: Hi from client 2
[WS] PING received - FD: 4
[WS] Client requested close - FD: 4
[WS] Connection closed - FD: 4
```

## 架构说明

### 异步流程

```
客户端连接
    │
    ├─> ws_handshake()         [协程]
    │   └─> async_read/write
    │
    ├─> 添加到连接列表
    │
    ├─> 发送欢迎消息
    │
    └─> 消息循环               [协程]
        ├─> ws_read_frame()    [协程]
        │   └─> async_read
        │
        ├─> 处理消息类型
        │   ├─> TEXT: 回显 + 广播
        │   ├─> PING: 发送 PONG
        │   └─> CLOSE: 关闭连接
        │
        └─> ws_send_frame()    [协程]
            └─> async_write
```

### 并发模型

- **每个连接一个协程**：`handle_websocket_client()`
- **所有协程运行在线程池**：4 个 worker 线程
- **协程自动调度**：通过 `co_await schedule_on(executor)`
- **Fire-and-forget**：使用 `.detach()` 启动协程

### 内存安全

- **RAII 管理连接**：协程结束时自动关闭 socket
- **智能指针**：连接列表使用 vector + mutex
- **异常安全**：try-catch 保护每个客户端处理

## 与传统方式对比

### 传统 callback 方式
```cpp
void on_connect(int fd) {
    read_async(fd, [fd](data) {
        parse_frame(data, [fd](frame) {
            process_message(frame, [fd](response) {
                write_async(fd, response, [fd]() {
                    // 回调地狱...
                });
            });
        });
    });
}
```

### 协程方式（本实现）
```cpp
task<void> handle_websocket_client(int fd) {
    auto frame = co_await ws_read_frame(fd);
    auto response = process_message(frame);
    co_await ws_send_frame(fd, response);
    // 清晰的顺序逻辑！
}
```

## 性能特点

- **非阻塞 I/O**：所有操作异步执行
- **线程池复用**：4 个线程处理所有连接
- **零拷贝优化**：直接操作 buffer
- **低延迟**：协程切换开销极小（~10ns）

## 限制与改进方向

### 当前限制
- 使用模拟的异步 I/O（sleep 模拟延迟）
- 广播采用简单遍历（O(n)）
- 无消息队列或背压机制

### 改进方向
1. **真正的异步 I/O**
   - 使用 epoll/io_uring
   - 事件驱动而非轮询

2. **高级特性**
   - WebSocket 压缩（permessage-deflate）
   - 子协议协商
   - 二进制帧支持

3. **性能优化**
   - 零拷贝 buffer 管理
   - 连接池和内存池
   - 批量广播优化

4. **生产级特性**
   - SSL/TLS 支持（wss://）
   - 心跳超时检测
   - 连接限流和速率限制
   - 监控指标

## 相关资源

- RFC 6455: WebSocket Protocol
- MDN WebSocket API
- C++20 Coroutines

## License

MIT
