//
// Created by asice-cloud on 6/8/25.
//

#ifndef SLEEP_H
#define SLEEP_H

#include "promise.h"
#include <chrono>
#include <deque>
#include <queue>
#include <thread>

struct SleepLoop
{
	SleepLoop& operator=(SleepLoop&&) = delete;

	void add_task(std::coroutine_handle<> t)
	{
		m_ready.push_front(t);
	}

	void run_all()
	{
		while (!m_ex_times.empty()||!m_ready.empty())
		{
			while (!m_ready.empty())
			{
				auto ready_task = m_ready.front();
				debug, "pop";
				m_ready.pop_front();
				ready_task.resume();
			}
			if (!m_ex_times.empty())
			{
				auto now = std::chrono::system_clock::now();
				auto timer =std::move( m_ex_times.top());
				if (now >= timer.expire)
				{
					debug, "timer expired";
					m_ex_times.pop();
					timer.coroutine.resume();
				}else
				{
					std::this_thread::sleep_until(timer.expire);
				}
			}
		}
	}

	void add_timer(std::chrono::system_clock::time_point expire, std::coroutine_handle<> coroutine)
	{
		m_ex_times.push({expire, coroutine});
	}

	struct Timer
	{
		std::chrono::system_clock::time_point expire;
		std::coroutine_handle<> coroutine;
		bool operator<(const Timer& other) const
		{
			return expire < other.expire;
		}
	};

	std::deque<std::coroutine_handle<>> m_ready;
	std::priority_queue<Timer> m_ex_times;
};

SleepLoop& get_sleep_loop(){
	static SleepLoop loop;
	return loop;
}

struct SleepAwaiter
{
	bool await_ready() const
	{
		return std::chrono::system_clock::now() >= m_expire;
	}

	void await_suspend(std::coroutine_handle<> coroutine) const noexcept
	{
		get_sleep_loop().add_timer(m_expire,coroutine);
	}

	void await_resume() const noexcept
	{
	}

	std::chrono::system_clock::time_point m_expire;
};

Task<void> SleepFor(std::chrono::system_clock::duration duration)
{
	co_await SleepAwaiter{std::chrono::system_clock::now() + duration};
	co_return;
}

Task<void> SleepUntil(std::chrono::system_clock::time_point tp)
{
	co_await SleepAwaiter(tp);
	co_return;
}


#endif //SLEEP_H
