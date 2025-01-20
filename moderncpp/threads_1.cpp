#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

double cal_time()
{
	/*
	 * duration_cast 可以在任意的duration类型之间转换
	 * duration<T,R>表示用T类型表示，时间单位是R
	 * R省略不写就是秒，milli是毫秒，micro是微秒
	 * seconds是duration<int64_t>别名
	 * */
	auto t0 = std::chrono::steady_clock::now();
	for (int i = 0; i < 10000000; i++)
		;
	// sleep in all platform;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	// std::this_thread::sleep_until(t) //set sleep untile a time point;
	auto t1 = std::chrono::steady_clock::now();
	auto dt1 = t1 - t0;
	using double_ms = std::chrono::duration<double, std::milli>;
	double ms = std::chrono::duration_cast<double_ms>(dt1).count();
	return ms;
}

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

void interact()
{
	std::cout << "input a name:" << '\n';
	std::string name;
	std::cin >> name;
	std::cout << "Hi, " << name << '\n';
}

void myfunc()
{
	std::thread tf([&] { download("hello.zip"); });
	// thread会在tf所在函数退出时调用结构函数销毁
	// 如果不用thread对象管理生命周期:
	tf.detach();
}

// 使用线程池管理线程生命周期
// std::vector<std::thread> pool;

// better pool: we do not need to use join() anymore;
class ThreadPool
{
	std::vector<std::thread> _pool;

public:
	void push_back(std::thread thr) { _pool.push_back(std::move(thr)); }
	~ThreadPool()
	{
		for (auto &t : _pool)
			t.join();
	}
};

ThreadPool pool;

void myfunc_2()
{
	std::thread tm([&] { download("world.zip"); });
	pool.push_back(std::move(tm));
}

int main()
{
	auto t0 = std::chrono::steady_clock::now();
	auto t1 = t0 + std::chrono::seconds(30);
	auto dt = t1 - t0;
	int64_t sec = std::chrono::duration_cast<std::chrono::seconds>(dt).count();
	std::cout << sec << '\n';

	std::cout << cal_time() << "ms" << '\n';

	std::thread th1([&] { download("hello.zip"); });
	myfunc();
	interact();
	// main should wait for thread end
	th1.join();

	myfunc_2();
	// wait for all thread end in pool;
	// for (auto &t : pool)
	// t.join();

	return 0;
}
