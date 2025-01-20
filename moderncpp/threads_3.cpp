#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

int main()
{
	std::queue<int> produce_nums;
	std::mutex mux;
	std::condition_variable cv;

	auto producor = [&]()
	{
		for (int i = 0; i < 20; ++i)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			{
				std::unique_lock<std::mutex> lk(mux);

				// produce one
				std::cout << "produce: " << i << '\n';
				produce_nums.push(i);
				cv.notify_one();
			}
		}
	};

	auto consumer = [&]()
	{
		while (true)
		{
			// consume one
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::unique_lock<std::mutex> lk(mux);
			// while (produce_nums.empty())
			// {
			// 	cv.wait(lk);
			// }
			// same as:
			// cv.wait(lk, [&]() { return !produce_nums.empty(); });

			bool is = cv.wait_for(lk, std::chrono::seconds(3), [&]() { return !produce_nums.empty(); });
			if (!is)
			{
				break;
			}
			std::cout << "consume: " << produce_nums.front() << '\n';
			produce_nums.pop();
		}
	};

	std::thread t_p = std::thread(producor);
	std::thread t_s[2];
	for (int i = 0; i < 2; ++i)
	{
		t_s[i] = std::thread(consumer);
	}
	t_p.join();
	for (int i = 0; i < 2; ++i)
	{
		t_s[i].join();
	}
	return 0;
}
