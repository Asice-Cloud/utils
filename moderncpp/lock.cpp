#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

int main()
{
	std::vector<int> array;
	std::mutex mtx;
	std::thread t1(
		[&]
		{
			for (int i = 0; i < 100; ++i)
			{
				mtx.lock();
				array.push_back(i);
				std::cout << std::this_thread::get_id() << std::endl;
				mtx.unlock();
			}
		});
	std::thread t2(
		[&]
		{
			for (int i = 0; i < 100; ++i)
			{
				mtx.lock();
				array.push_back(i);
				std::cout << std::this_thread::get_id() << std::endl;
				mtx.unlock();
			}
		});
	t1.join();
	t2.join();

	// lock_guard: fit RAII
	std::vector<int> array_2;
	std::mutex mtx_2;
	std::jthread t3(
		[&]
		{
			for (int i = 0; i < 100; ++i)
			{
				std::lock_guard grd(mtx_2);
				array_2.push_back(i);
				std::cout << std::this_thread::get_id() << std::endl;
			}
		});
	std::jthread t4(
		[&]
		{
			for (int i = 0; i < 100; ++i)
			{
				std::lock_guard grd(mtx_2);
				array_2.push_back(i);
				std::cout << std::this_thread::get_id() << std::endl;
			}
		});

	// unique lock: more flexible, liberate the lock
	std::jthread t5(
		[&]
		{
			std::unique_lock grd(mtx_2);
			array_2.push_back(114514);
			grd.unlock();
			std::cout << "outside of lock\n";
			// if need, you can lock again
			// grd.lock();
		});
	// defer lock, so that unique_lock won't lock, you can lock it later by hand
	std::jthread t6(
		[&]
		{
			for (int i = 0; i < 100; i++)
			{
				std::unique_lock grd(mtx, std::defer_lock);
				std::cout << "before lock\n";
				grd.lock();
				array.push_back(1919810);
				grd.unlock();
				std::cout << "after unlock\n";
			}
		});
	// try_lock: try to lock, if failed, return false
	// tyr_lock_for: try to lock for a period of time
	std::jthread t7(
		[&]
		{
			for (int i = 0; i < 100; i++)
			{
				std::unique_lock grd(mtx, std::defer_lock);
				std::cout << "before lock\n";
				if (grd.try_lock())
				{
					array.push_back(1919810);
					grd.unlock();
				}
				else
				{
					std::cout << "failed to lock\n";
				}
				std::cout << "after unlock\n";
			}
		});

	// adopt_lock: lock the mutex by hand
	std::jthread t8(
		[&]
		{
			for (int i = 0; i < 100; i++)
			{
				std::unique_lock grd(mtx, std::adopt_lock);
				array.push_back(1919810);
				grd.unlock();
			}
		});
	return 0;
}
