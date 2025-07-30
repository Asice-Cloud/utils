// 更好的多线程对象：jthread
#include <future>
#include <iostream>
#include <thread>
#include <vector>

std::vector<std::jthread> pool;

// multuply thread
void download(std::string file)
{
	for (int i = 0; i < 10; i++)
	{
		std::cout << "download..." << file << "(" << i * 10 << "%)..." << '\n';
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cout << "Done!" << file << '\n';
}

void myfunc()
{
	// jthread is better and more convenient, fit for RAII
	std::jthread t1([&] { download("ubuntu.osi"); });

	// 移交控制权到全局的pool列表，以延长t1的生命周期
	pool.emplace_back(std::move(t1));
}

// synchronous and asynchronous
// std::async

int GetFile(std::string file)
{
	for (int i = 0; i < 10; i++)
	{
		std::cout << "getting..." << file << "(" << i * 10 << "%}" << '\n';
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}
	std::cout << "Done!" << '\n';
	return 404;
}

int main()
{
	myfunc();

	// future: std::async receive a lambda,return a std::future
	// lambda will run in another thread
	// get() will wait this function done and get the return of function;
	// or you can use wait(),but won't return value
	std::future<int> fret = std::async([&] { return GetFile("meme.zip"); });
	// also, use std::launch::deferred in async(),it won't create a thread,
	// just delay this function until invoking future's get();
	std::cout << "no return" << '\n';

	// fret.wait();
	// wait() will wait until it done, ues wait_for() target a max waitting time;
	// if it not done which more than time, give up waitting,return future_status::timeout
	// if successful, return future_status::ready
	while (true)
	{
		auto status = fret.wait_for(std::chrono::milliseconds(1000));
		if (status == std::future_status::ready)
		{
			std::cout << "future is Done" << '\n';
			break;
		}
		else
		{
			std::cout << "future not ready!" << '\n';
		}
	}
	std::cout << "will return" << '\n';
	int ret = fret.get();
	std::cout << "get result :" << ret << '\n';

	// base in async: promise:
	//  create thread by hand:
	std::promise<int> pret;
	std::thread t1(
		[&]
		{
			auto ret = GetFile("fuck.zip");
			pret.set_value(ret);
		});
	std::future<int> rere = pret.get_future();
	std::cout << "new result:" << rere.get() << '\n';
	t1.join();

	return 0;
}
