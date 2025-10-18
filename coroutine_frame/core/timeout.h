//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_TIMEOUT_H
#define TASK_DO_TIMEOUT_H

#include "task.h"
#include "cancellation_token.h"
#include "when_any.h"
#include <chrono>
#include <stdexcept>

// Timeout exception
class timeout_error : public std::runtime_error {
public:
    timeout_error() : std::runtime_error("Task timed out") {}
    explicit timeout_error(const char* msg) : std::runtime_error(msg) {}
};

// Timeout task that throws after duration
template<typename Duration>
task<void> timeout_task(Duration duration, cancellation_token token) {
    co_await schedule_on(get_global_executor());
    
    auto start = std::chrono::steady_clock::now();
    auto end = start + duration;
    
    while (std::chrono::steady_clock::now() < end) {
        // Check if cancelled (task completed before timeout)
        if (token.is_cancelled()) {
            co_return;  // Success, task completed in time
        }
        co_await async_delay(std::chrono::milliseconds(10));
    }
    
    // Timeout reached
    throw timeout_error();
}

// with_timeout: Run a task with a timeout
// Throws timeout_error if task doesn't complete in time
template<typename T, typename Duration>
task<T> with_timeout(task<T> t, Duration duration) {
    co_await schedule_on(get_global_executor());
    
    cancellation_token timeout_cancel;
    
    // Create timeout task
    auto timeout = timeout_task(duration, timeout_cancel);
    
    // Race between the task and timeout
    std::vector<task<int>> race;
    race.push_back([](task<T> inner_task) -> task<int> {
        co_await std::move(inner_task);
        co_return 0;  // Task completed
    }(std::move(t)));
    
    race.push_back([](task<void> inner_timeout) -> task<int> {
        co_await std::move(inner_timeout);
        co_return 1;  // Timeout reached
    }(std::move(timeout)));
    
    auto [index, _] = co_await when_any(std::move(race));
    
    if (index == 1) {
        throw timeout_error("Task exceeded timeout");
    }
    
    // Cancel timeout task since main task completed
    timeout_cancel.cancel();
    
    // This is hacky - we need to refactor when_any to return actual results
    // For now, we'll re-execute the task (not ideal)
    // TODO: Improve this implementation
    throw std::runtime_error("with_timeout needs implementation improvement");
}

// Simpler version for void tasks
template<typename Duration>
task<void> with_timeout(task<void> t, Duration duration) {
    co_await schedule_on(get_global_executor());
    
    cancellation_token timeout_cancel;
    std::atomic<bool> task_done{false};
    std::atomic<bool> timed_out{false};
    std::exception_ptr task_exception;
    
    // Wrapper for main task
    auto task_wrapper = [&]() -> task<void> {
        try {
            co_await std::move(t);
            task_done = true;
            timeout_cancel.cancel();  // Cancel timeout
        } catch (...) {
            task_exception = std::current_exception();
            task_done = true;
            timeout_cancel.cancel();
        }
    };
    
    // Wrapper for timeout
    auto timeout_wrapper = [&](Duration dur) -> task<void> {
        try {
            co_await timeout_task(dur, timeout_cancel);
            timed_out = true;
        } catch (const task_cancelled&) {
            // Normal cancellation, task completed in time
        }
    };
    
    // Launch both
    task_wrapper().detach();
    timeout_wrapper(duration).detach();
    
    // Wait for one to complete
    while (!task_done && !timed_out) {
        co_await async_delay(std::chrono::milliseconds(1));
    }
    
    if (timed_out && !task_done) {
        throw timeout_error("Task exceeded timeout");
    }
    
    if (task_exception) {
        std::rethrow_exception(task_exception);
    }
    
    co_return;
}

#endif //TASK_DO_TIMEOUT_H
