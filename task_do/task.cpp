#include "task.h"

// promise_type implementation
task::promise_type::promise_type() noexcept = default;

task::promise_type::~promise_type() = default;

task task::promise_type::get_return_object() noexcept {
    return task{std::coroutine_handle<promise_type>::from_promise(*this)};
}

std::suspend_always task::promise_type::initial_suspend() noexcept {
    return {};
}

task::promise_type::final_awaiter task::promise_type::final_suspend() noexcept {
    return {};
}

void task::promise_type::unhandled_exception() noexcept {
    result_ = std::current_exception();
}

void task::promise_type::return_value(int value) noexcept {
    result_ = value;
}

// final_awaiter implementation
bool task::promise_type::final_awaiter::await_ready() noexcept {
    return false;
}

std::coroutine_handle<> task::promise_type::final_awaiter::await_suspend(
    std::coroutine_handle<promise_type> h) noexcept {
    auto& promise = h.promise();
    if (promise.continuation_) {
        return promise.continuation_;
    }
    return std::noop_coroutine();
}

void task::promise_type::final_awaiter::await_resume() noexcept {}

// task implementation
task::task(std::coroutine_handle<promise_type> h) noexcept : coro_(h) {}

task::task(task&& t) noexcept : coro_(t.coro_) {
    t.coro_ = nullptr;
}

task::~task() {
    if (coro_) {
        coro_.destroy();
    }
}

task& task::operator=(task&& t) noexcept {
    if (this != &t) {
        if (coro_) {
            coro_.destroy();
        }
        coro_ = t.coro_;
        t.coro_ = nullptr;
    }
    return *this;
}

task::awaiter task::operator co_await() && noexcept {
    return awaiter{coro_};
}

// awaiter implementation
task::awaiter::awaiter(std::coroutine_handle<promise_type> handle) noexcept
    : coro_(handle) {}

bool task::awaiter::await_ready() noexcept {
    return false;
}

std::coroutine_handle<task::promise_type> task::awaiter::await_suspend(
    std::coroutine_handle<> awaiting) noexcept {
    coro_.promise().continuation_ = awaiting;
    return coro_;
}

int task::awaiter::await_resume() {
    auto& result = coro_.promise().result_;
    if (std::holds_alternative<std::exception_ptr>(result)) {
        std::rethrow_exception(std::get<std::exception_ptr>(result));
    }
    return std::get<int>(result);
}

