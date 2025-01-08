//
// Created by asice-cloud on 1/8/25.
//

#include <iostream>
#include <coroutine>
#include <thread>
#include <stdexcept>

//implement switch among threads

// coro_ret must definite promise_type
// coro_ret is return value of a coroutine function
template <typename T>
struct coro_ret
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	handle_type coro_handle_;

	coro_ret(handle_type h): coro_handle_(h){};

	coro_ret(const coro_ret&)=delete;
	coro_ret(coro_ret&&s):coro_handle_(s.coro_handle_)
	{
		s.coro_handle_=nullptr;
	}

	~coro_ret()
	{
		if (coro_handle_) coro_handle_.destroy();
	}

	coro_ret& operator=(const coro_ret&)=delete;
	coro_ret& operator=(coro_ret&&s)
	{
		coro_handle_=s.coro_handle_;
		s.coro_handle_=nullptr;
		return *this;
	}

	bool move_next()
	{
		 coro_handle_.resume();
		return coro_handle_.done();
	}

	T get()
	{
		return coro_handle_.promise().return_data_;
	}

	struct promise_type
	{
		promise_type()=default;
		~promise_type()=default;

		auto get_return_object()
		{
			return coro_ret<T>{handle_type::from_promise(*this)};
		}

		auto initial_suspend() //this is returned the awaiter
		{
			return std::suspend_never{};
			// return std::suspend_always{};
		}

		void return_value(T v)
		{
			return_data_ = v;
			return;
		}

		auto yield_value(T v)
		{
			std::cout<<"yield_value invoke"<<std::endl;
			return_data_=v;
			return std::suspend_never{};
		}

		auto final_suspend() noexcept
		{
			std::cout<<"final_suspend invoke"<<std::endl;
			return std::suspend_always{};
		}

		void unhandled_exception()
		{
			std::exit(1);
		}

		T return_data_;
	};
};


// in out *7
coro_ret<int> coroutine_7i7o()
{
	std::cout<<"coroutine co_await std::suspend_never"<<'\n';
	co_await std::suspend_never{};

	std::cout<< "coroutine co_await std::suspend_always\n";
	co_await std::suspend_always{};

	std::cout<<"Coroutine stage 1, co_yield"<<'\n';
	co_yield 101;
	std::cout<<"Coroutine stage 2, co_yield"<<'\n';
	co_yield 202;
	std::cout<<"Coroutine stage 3, co_yield"<<'\n';
	co_yield 303;
	std::cout<<"Coroutine stage 4, co_yield"<<'\n';
	co_yield 404;
}

int main()
{
	bool done=false;
	std::cout<<"Start coroutine_7i7o"<<'\n';

	auto c_r=coroutine_7i7o();
	//first return is suspend always, now not enter stage 1

	std::cout<<"Coroutine "<<(done?" done":"not done")<<"ret= "<<c_r.get()<<'\n';
	done=c_r.move_next();
	// now suspend always

	//now stage 1
	std::cout<<"Coroutine "<<(done?"done":"not done")<<"ret="<<c_r.get()<<'\n';
	done=c_r.move_next();

	std::cout<<"Coroutine "<<(done?"done":"not done")<<"ret="<<c_r.get()<<'\n';
	return 0;
}