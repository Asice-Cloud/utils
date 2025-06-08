#include "core/co.h"

// #include "core/thread_pool.h"
// #include "router/router.h"
// #include "util/Object.h"
//

Task<void> ling()
{
	debug, "00000";
	co_return ;
}

Task<std::string> hello_world()
{
	debug, "hello world";
	co_return "hello world";
}

Task<int> hello()
{

	ciallo ling();

	debug, "sleep 1";
	ciallo SleepFor(std::chrono::seconds(1));
	debug, "sleep 2";

	co_return 42;
}

int main()
{
	debug, "hello before";
	Task t1 = hello();
	Task t2 = hello_world();
	get_sleep_loop().add_task(t1);
	get_sleep_loop().add_task(t2);
	debug, "hello after";

	get_sleep_loop().run_all();

	debug, "result t1", t1.m_coroutine.promise().result();
	debug, "result t2", t2.m_coroutine.promise().result();
	return 0;
}

// int main() {
//   Router router;
//   router.add_router("^/api/v1/.*$", "v1_service");
//   router.add_router("^/api/v2/.*$", "v2_service");
//
//   std::string request1 = "/api/v1/resource";
//   std::string request2 = "/api/v2/resource";
//   std::string request3 = "/api/v3/resource";
//
//   std::cout << "Request: " << request1
//             << " -> Service: " << router.route(request1) << std::endl;
//   std::cout << "Request: " << request2
//             << " -> Service: " << router.route(request2) << std::endl;
//   std::cout << "Request: " << request3
//             << " -> Service: " << router.route(request3) << std::endl;
//
//   MyClass obj;
//
//   for (const auto &field : obj.getFields()) {
//     std::cout << "Field: " << field.name << " -> Value: " << field.getter(&obj)
//               << std::endl;
//   }
//
//   ThreadPool pool(4);
//   std::vector<std::future<int>> results;
//
//   for (int i = 0; i < 8; ++i) {
//     results.emplace_back(pool.en_queue([i] {
//       std::cout << "hello " << i << std::endl;
//       std::this_thread::sleep_for(std::chrono::seconds(1));
//       std::cout << "world " << i << std::endl;
//       return i * i;
//     }));
//   }
//
//   for (auto &&result : results)
//     std::cout << result.get() << ' ';
//   std::cout << std::endl;
//
//   return 0;
// }
