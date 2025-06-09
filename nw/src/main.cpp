// #include "core/thread_pool.h"
// #include "router/router.h"
// #include "util/Object.h"

#include "util/debug.h"
#include "core/co/task.h"
#include "core/co/filesystem.h"
#include "core/co/socket.h"
#include "core/co/epoll_loop.h"
#include "core/co/stream.h"

using namespace co_async;
using namespace std::literals;

EpollLoop loop;

Task<> handle_connection(FileStream s, IpAddress addr) {
    debug(), "收到了来自", addr, "的连接";

    debug(), "发来的消息头如下：";
    while (true) {
        auto l = co_await s.getline("\r\n");
        debug(), l;
        if (l.empty()) {
            break;
        }
    }
    co_await s.puts("HTTP/1.1 200 OK\r\n");
    co_await s.puts("Content-Type: text/plain\r\n");
    co_await s.puts("Content-Length: 12\r\n");
    co_await s.puts("\r\n");
    co_await s.puts("Hello, world");
    co_await s.flush();
    co_return;
}

Task<> amain() {
    auto serv = co_await create_tcp_server(
        loop, socket_address(ip_address("127.0.0.1"), 8080));
    socket_listen(serv);

    while (true) {
        auto [conn, addr] = co_await socket_accept<IpAddress>(loop, serv);
        FileStream s(loop, std::move(conn));
        run_task(loop, handle_connection(std::move(s), addr));
    }
}

int main() {
    run_task(loop, amain());
    return 0;
}


// int main() {
// Router router;
// router.add_router("^/api/v1/.*$", "v1_service");
// router.add_router("^/api/v2/.*$", "v2_service");
//
// std::string request1 = "/api/v1/resource";
// std::string request2 = "/api/v2/resource";
// std::string request3 = "/api/v3/resource";
//
// std::cout << "Request: " << request1
//           << " -> Service: " << router.route(request1) << std::endl;
// std::cout << "Request: " << request2
//           << " -> Service: " << router.route(request2) << std::endl;
// std::cout << "Request: " << request3
//           << " -> Service: " << router.route(request3) << std::endl;
//
// MyClass obj;
//
// for (const auto &field : obj.getFields()) {
//   std::cout << "Field: " << field.name << " -> Value: " << field.getter(&obj)
//             << std::endl;
// }
//
// ThreadPool pool(4);
// std::vector<std::future<int>> results;
//
// for (int i = 0; i < 8; ++i) {
//   results.emplace_back(pool.en_queue([i] {
//     std::cout << "hello " << i << std::endl;
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     std::cout << "world " << i << std::endl;
//     return i * i;
//   }));
// }
//
// for (auto &&result : results)
//   std::cout << result.get() << ' ';
// std::cout << std::endl;
//
// return 0;
// }
