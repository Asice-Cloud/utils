//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_WHEN_ANY_H
#define TASK_DO_WHEN_ANY_H

#include "task.h"
#include <vector>
#include <atomic>
#include <optional>
#include <mutex>

// when_any: Wait for the first task to complete and return its result
// Returns a task that yields a pair<size_t, T> where size_t is the index
template<typename T>
task<std::pair<size_t, T>> when_any(std::vector<task<T>>&& tasks) {
    co_await schedule_on(get_global_executor());
    
    if (tasks.empty()) {
        throw std::invalid_argument("when_any: empty task list");
    }
    
    std::atomic<bool> completed{false};
    std::mutex result_mutex;
    std::optional<std::pair<size_t, T>> result;
    
    // Start all tasks
    for (auto& t : tasks) {
        t.resume();
    }
    
    // Check tasks in order until one completes
    // This is a simplified implementation; a production version would use proper event notification
    while (!completed.load()) {
        for (size_t i = 0; i < tasks.size(); ++i) {
            if (tasks[i].done()) {
                if (!completed.exchange(true)) {
                    // We're the first to notice completion
                    T value = co_await std::move(tasks[i]);
                    co_return std::make_pair(i, std::move(value));
                }
            }
        }
        // Small delay to avoid busy waiting
        co_await async_delay(std::chrono::milliseconds(1));
    }
    
    // Should never reach here
    throw std::runtime_error("when_any: unexpected state");
}

// Simplified when_any that just returns the first result (without index)
template<typename T>
task<T> when_any_value(std::vector<task<T>>&& tasks) {
    auto [index, value] = co_await when_any(std::move(tasks));
    co_return value;
}

#endif //TASK_DO_WHEN_ANY_H
