//
// Created by asice-cloud on 1/8/25.
//

#include <coroutine>
#include <iostream>

struct Generator{
	struct promise_type
	{
		int current_value;
		Generator get_return_object()
		{
			return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
		}
		std::suspend_always initial_suspend()
		{
			return {};
		}
		std::suspend_always final_suspend() noexcept
		{
			return {};
		}
		void unhandled_exception()
		{
			std::terminate();
		}
		std::suspend_always yield_value(int value)
		{
			current_value=value;
			return {};
		}
		void return_void(){};
	};

	// create a instance
	std::coroutine_handle<promise_type> coro_handle;
	//raii
	explicit Generator(std::coroutine_handle<promise_type> handle):coro_handle(handle){}

	~Generator()
	{
		if (coro_handle) coro_handle.destroy();
	}

	Generator(const Generator&)=delete;
	Generator& operator=(const Generator&)=delete;

	Generator(Generator&& other) noexcept:coro_handle(other.coro_handle)
	{
		other.coro_handle=nullptr;
	}
	Generator& operator=(Generator&& other)noexcept
	{
		if (this!=&other)
		{
			coro_handle=other.coro_handle;
			other.coro_handle=nullptr;
		}
		return *this;
	}

	bool move_next()
	{
		if (!coro_handle.done())
		{
			coro_handle.resume();
			return true;
		}
		return false;
	}

	int current_value() const
	{
		return coro_handle.promise().current_value;
	}
};

Generator counter()
{
	for (int i=0;i<100;++i)
	{
		co_yield i;
	}
	co_return;
}

int main()
{
	Generator gen = counter();
	while (gen.move_next())
	{
		std::cout<<gen.current_value()<<'\n';
	}
	return 0;
}