//
// Created by asice-cloud on 4/2/25.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>


using P_Task = std::function<void()>;

class ThreadPool{
    private:
	std::vector<std::thread> workers;
	std::queue<P_Task> tasks;
	std::mutex mtx;
	std::condition_variable cv;
	std::atomic_bool stop;

public:
	explicit ThreadPool(size_t size);

	template<typename F, typename... Args>
	auto en_queue(F&&f, Args&&... args) -> std::future<std::result_of_t<F(Args...)>>;
	~ThreadPool();

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;

};

inline ThreadPool::ThreadPool(size_t size) :stop(false)
{
	for (size_t i = 0; i < size; ++i)
	{
		workers.emplace_back([this]()
		{
			for (;;)
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->mtx);
					this->cv.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}
				task();
			}
		});
	}
}

template <typename F, typename... Args>
auto ThreadPool::en_queue(F&& f, Args&&... args) -> std::future<std::result_of_t<F(Args...)>>
{
	using return_type = std::result_of_t<F(Args...)>;
	auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
	);

	std::future<return_type> res = task->get_future();
	{
        std::unique_lock<std::mutex> lock(mtx);
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task]() { (*task)(); });
    }
	cv.notify_one();
	return res;
}

inline ThreadPool::~ThreadPool()
{
	{
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();
    for (std::thread& worker : workers)
        worker.join();
}


#endif //THREAD_POOL_H
