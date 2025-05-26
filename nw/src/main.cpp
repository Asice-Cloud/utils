#include "router/router.h"
#include "util/Object.h"
#include "core/thread_pool.h"
#include "server/http_req_res.h"
#include "server/tcp_server.h"


int main() {
	// Router router;
	// router.add_router("^/api/v1/.*$", "v1_service");
	// router.add_router("^/api/v2/.*$", "v2_service");
	//
	// std::string request1 = "/api/v1/resource";
	// std::string request2 = "/api/v2/resource";
	// std::string request3 = "/api/v3/resource";
	//
	// std::cout << "Request: " << request1 << " -> Service: " << router.route(request1) << std::endl;
	// std::cout << "Request: " << request2 << " -> Service: " << router.route(request2) << std::endl;
	// std::cout << "Request: " << request3 << " -> Service: " << router.route(request3) << std::endl;
	//
	//
	// MyClass obj;
	//
	// for (const auto& field : obj.getFields()) {
	// 	std::cout << "Field: " << field.name << " -> Value: " << field.getter(&obj) << std::endl;
	// }
	//
	//
	// ThreadPool pool(4);
	// std::vector< std::future<int> > results;
	//
	// for(int i = 0; i < 8; ++i) {
	// 	results.emplace_back(
	// 		pool.en_queue([i] {
	// 			std::cout << "hello " << i << std::endl;
	// 			std::this_thread::sleep_for(std::chrono::seconds(1));
	// 			std::cout << "world " << i << std::endl;
	// 			return i*i;
	// 		})
	// 	);
	// }
	//
	// for(auto && result: results)
	// 	std::cout << result.get() << ' ';
	// std::cout << std::endl;


	// asio::io_context io_context;
	// asio::steady_timer timer(io_context, std::chrono::seconds(1));
	//
	// int count = 0;
	// timer.async_wait([&timer, &count](const asio::error_code& error) {
	// 	if (!error && count < 5) {
	// 		std::cout << "Timer expired: " << count << std::endl;
	// 		count++;
	//
	// 		timer.expires_at(timer.expiry() + std::chrono::seconds(1));
	// 		timer.async_wait([&timer, &count](const asio::error_code& error) {
	// 			if (!error) {
	// 				std::cout << "Timer callback executed: " << count << std::endl;
	// 				count++;
	// 			}
	// 		});
	// 	}
	// });
	// io_context.run();
	// std::this_thread::sleep_for(std::chrono::seconds(1));

	try
	{
		asio::io_context io_context;
		tcp_server server(io_context);
		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;

}