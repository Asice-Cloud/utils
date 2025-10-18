//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_WHEN_ALL_H
#define TASK_DO_WHEN_ALL_H

#include "task.h"
#include <vector>
#include <tuple>
#include <utility>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>

// when_all: Wait for all tasks to complete and return all results
// TRUE PARALLEL VERSION: All tasks start concurrently

namespace detail {
    // Shared state for coordinating multiple tasks
    template<typename T>
    struct when_all_state {
        std::vector<T> results;
        std::atomic<size_t> completed{0};
        size_t total;
        std::mutex mutex;
        std::condition_variable cv;
        std::exception_ptr exception;
        
        when_all_state(size_t n) : total(n) {
            results.resize(n);
        }
        
        void set_result(size_t index, T&& value) {
            std::lock_guard lock(mutex);
            results[index] = std::move(value);
            completed.fetch_add(1, std::memory_order_release);
            cv.notify_one();
        }
        
        void set_exception(std::exception_ptr ex) {
            std::lock_guard lock(mutex);
            if (!exception) {
                exception = ex;
            }
            completed.fetch_add(1, std::memory_order_release);
            cv.notify_one();
        }
        
        bool is_done() const {
            return completed.load(std::memory_order_acquire) >= total;
        }
    };
    
    // Wrapper task that stores result in shared state
    template<typename T>
    task<void> when_all_task(task<T> t, std::shared_ptr<when_all_state<T>> state, size_t index) {
        try {
            T result = co_await std::move(t);
            state->set_result(index, std::move(result));
        } catch (...) {
            state->set_exception(std::current_exception());
        }
    }
}

template<typename T>
task<std::vector<T>> when_all(std::vector<task<T>>&& tasks) {
    co_await schedule_on(get_global_executor());
    
    if (tasks.empty()) {
        co_return std::vector<T>{};
    }
    
    // Create shared state
    auto state = std::make_shared<detail::when_all_state<T>>(tasks.size());
    
    // Launch all tasks concurrently
    for (size_t i = 0; i < tasks.size(); ++i) {
        detail::when_all_task(std::move(tasks[i]), state, i).detach();
    }
    
    // Wait for all tasks to complete
    while (!state->is_done()) {
        co_await async_delay(std::chrono::milliseconds(1));
    }
    
    // Check for exceptions
    if (state->exception) {
        std::rethrow_exception(state->exception);
    }
    
    co_return std::move(state->results);
}

// when_all for void tasks: Wait for all void tasks to complete
inline task<void> when_all_void(std::vector<task<void>>&& tasks) {
    co_await schedule_on(get_global_executor());
    
    if (tasks.empty()) {
        co_return;
    }
    
    std::atomic<size_t> completed{0};
    size_t total = tasks.size();
    std::exception_ptr exception;
    std::mutex mutex;
    
    auto wrapper = [&](task<void> t) -> task<void> {
        try {
            co_await std::move(t);
        } catch (...) {
            std::lock_guard lock(mutex);
            if (!exception) {
                exception = std::current_exception();
            }
        }
        completed.fetch_add(1, std::memory_order_release);
    };
    
    // Launch all tasks
    for (auto& t : tasks) {
        wrapper(std::move(t)).detach();
    }
    
    // Wait for completion
    while (completed.load(std::memory_order_acquire) < total) {
        co_await async_delay(std::chrono::milliseconds(1));
    }
    
    if (exception) {
        std::rethrow_exception(exception);
    }
    
    co_return;
}

// when_all for variadic tasks with different types
// Returns a tuple of results
// NOTE: This version evaluates tasks sequentially (left-to-right)
// For true concurrent execution, consider using when_all with vector
namespace detail {
    template<typename... Ts>
    task<std::tuple<Ts...>> when_all_variadic_impl(task<Ts>&&... tasks) {
        co_await schedule_on(get_global_executor());
        
        // Await each task and construct tuple from results
        // C++17 guaranteed left-to-right evaluation in braced-init-list
        co_return std::tuple<Ts...>{co_await std::move(tasks)...};
    }
}

// Variadic when_all: when_all(task1, task2, task3, ...)
template<typename... Ts>
task<std::tuple<Ts...>> when_all(task<Ts>&&... tasks) {
    return detail::when_all_variadic_impl(std::move(tasks)...);
}

#endif //TASK_DO_WHEN_ALL_H
