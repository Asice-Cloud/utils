//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_WHEN_ANY_H
#define TASK_DO_WHEN_ANY_H

#include "executor.h"
#include "task.h"
#include <atomic>
#include <mutex>
#include <optional>
#include <vector>

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
    std::condition_variable cv;
    std::optional<std::pair<size_t, T>> result;

    // Launch all tasks with completion notification
    for (size_t i = 0; i < tasks.size(); ++i) {
        auto wrapper = [i, &completed, &result, &result_mutex, &cv](task<T> t) -> task<void> {
            try {
                T value = co_await std::move(t);
                std::lock_guard lock(result_mutex);
                if (!completed.exchange(true)) {
                    result = std::make_pair(i, std::move(value));
                    cv.notify_all();
                }
            } catch (...) {
                // Ignore exceptions for when_any; could be extended to propagate
            }
        };
        wrapper(std::move(tasks[i])).detach();
    }

    // Wait for any task to complete (event-driven)
    while (!completed.load()) {
        std::unique_lock lock(result_mutex);
        if (!completed.load()) {
            cv.wait(lock);
        }
    }
    if (result) {
        co_return std::move(*result);
    }
    throw std::runtime_error("when_any: unexpected state");
}

// Simplified when_any that just returns the first result (without index)
template<typename T>
task<T> when_any_value(std::vector<task<T>>&& tasks) {
    auto [index, value] = co_await when_any(std::move(tasks));
    co_return value;
}

#endif //TASK_DO_WHEN_ANY_H
