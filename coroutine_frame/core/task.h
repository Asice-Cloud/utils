//
// Created by asice-cloud on 10/18/25.
//

#ifndef TASK_DO_TASK_H
#define TASK_DO_TASK_H
#include <coroutine>
#include <variant>
#include <exception>
#include <utility>
#include <type_traits>

// Generic coroutine task class supporting any return type T
// Specialization for void is provided below
template<typename T = int>
class task {
public:
    struct awaiter;

    struct promise_type {
        promise_type() noexcept = default;
        ~promise_type() = default;

        struct final_awaiter {
            bool await_ready() noexcept { return false; }
            
            std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                // If detached, destroy the coroutine now that it's done
                if (h.promise().detached_) {
                    h.destroy();
                    return std::noop_coroutine();
                }
                
                if (h.promise().continuation_) {
                    return h.promise().continuation_;
                }
                return std::noop_coroutine();
            }
            
            void await_resume() noexcept {}
        };

        task get_return_object() noexcept {
            return task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        final_awaiter final_suspend() noexcept { return {}; }

        void unhandled_exception() noexcept {
            result_ = std::current_exception();
        }

        void return_value(T value) noexcept {
            result_ = std::move(value);
        }

        std::coroutine_handle<> continuation_;
        std::variant<std::monostate, T, std::exception_ptr> result_;
        bool detached_ = false;  // Track if task was detached
    };

    task(const task&) = delete;
    task& operator=(const task&) = delete;

    task(task&& t) noexcept : coro_(t.coro_) {
        t.coro_ = nullptr;
    }

    ~task() {
        if (coro_) {
            coro_.destroy();
        }
    }

    task& operator=(task&& t) noexcept {
        if (this != &t) {
            if (coro_) {
                coro_.destroy();
            }
            coro_ = t.coro_;
            t.coro_ = nullptr;
        }
        return *this;
    }

    struct awaiter {
        explicit awaiter(std::coroutine_handle<promise_type> handle) noexcept 
            : coro_(handle) {}

        bool await_ready() noexcept {
            return coro_.done();
        }

        std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<> awaiting) noexcept {
            coro_.promise().continuation_ = awaiting;
            return coro_;
        }

        T await_resume() {
            auto& result = coro_.promise().result_;
            
            if (std::holds_alternative<std::exception_ptr>(result)) {
                std::rethrow_exception(std::get<std::exception_ptr>(result));
            }
            
            return std::get<T>(std::move(result));
        }

    private:
        std::coroutine_handle<promise_type> coro_;
    };

    awaiter operator co_await() && noexcept {
        return awaiter{coro_};
    }

    bool done() const noexcept {
        return coro_.done();
    }

    void resume() {
        if (!coro_.done()) {
            coro_.resume();
        }
    }

    // Detach: Fire-and-forget execution
    // The coroutine will self-destruct when it completes
    // SAFETY: Task MUST be scheduled on executor and run to completion
    void detach() noexcept {
        if (coro_) {
            coro_.promise().detached_ = true;
            // Schedule it if not already running
            coro_.resume();
            coro_ = nullptr;  // Release ownership
        }
    }

private:
    explicit task(std::coroutine_handle<promise_type> h) noexcept 
        : coro_(h) {}

    std::coroutine_handle<promise_type> coro_;
};

// Specialization for void return type
template<>
class task<void> {
public:
    struct awaiter;

    struct promise_type {
        promise_type() noexcept = default;
        ~promise_type() = default;

        struct final_awaiter {
            bool await_ready() noexcept { return false; }
            
            std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                // If detached, destroy the coroutine now that it's done
                if (h.promise().detached_) {
                    h.destroy();
                    return std::noop_coroutine();
                }
                
                if (h.promise().continuation_) {
                    return h.promise().continuation_;
                }
                return std::noop_coroutine();
            }
            
            void await_resume() noexcept {}
        };

        task get_return_object() noexcept {
            return task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        final_awaiter final_suspend() noexcept { return {}; }

        void unhandled_exception() noexcept {
            exception_ = std::current_exception();
        }

        void return_void() noexcept {}

        std::coroutine_handle<> continuation_;
        std::exception_ptr exception_;
        bool detached_ = false;  // Track if task was detached
    };

    task(const task&) = delete;
    task& operator=(const task&) = delete;

    task(task&& t) noexcept : coro_(t.coro_) {
        t.coro_ = nullptr;
    }

    ~task() {
        if (coro_) {
            coro_.destroy();
        }
    }

    task& operator=(task&& t) noexcept {
        if (this != &t) {
            if (coro_) {
                coro_.destroy();
            }
            coro_ = t.coro_;
            t.coro_ = nullptr;
        }
        return *this;
    }

    struct awaiter {
        explicit awaiter(std::coroutine_handle<promise_type> handle) noexcept 
            : coro_(handle) {}

        bool await_ready() noexcept {
            return coro_.done();
        }

        std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<> awaiting) noexcept {
            coro_.promise().continuation_ = awaiting;
            return coro_;
        }

        void await_resume() {
            if (coro_.promise().exception_) {
                std::rethrow_exception(coro_.promise().exception_);
            }
        }

    private:
        std::coroutine_handle<promise_type> coro_;
    };

    awaiter operator co_await() && noexcept {
        return awaiter{coro_};
    }

    bool done() const noexcept {
        return coro_ && coro_.done();
    }

    void resume() {
        if (coro_ && !coro_.done()) {
            coro_.resume();
        }
    }

    // Detach: Fire-and-forget execution
    // The coroutine will self-destruct when it completes
    // SAFETY: Task MUST be scheduled on executor and run to completion
    void detach() noexcept {
        if (coro_) {
            coro_.promise().detached_ = true;
            // Schedule it if not already running
            coro_.resume();
            coro_ = nullptr;  // Release ownership
        }
    }

private:
    explicit task(std::coroutine_handle<promise_type> h) noexcept 
        : coro_(h) {}

    std::coroutine_handle<promise_type> coro_;
};

#endif //TASK_DO_TASK_H