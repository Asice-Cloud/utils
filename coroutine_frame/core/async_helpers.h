//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_ASYNC_HELPERS_H
#define TASK_DO_ASYNC_HELPERS_H

#include "task.h"
#include "executor.h"
#include <type_traits>

// Convert a regular function to an async coroutine
// Automatically deduces return type from the function
// Note: Exceptions thrown by func() will be propagated through the task

// For functions that return a value
template<typename Func>
auto async_convert(Func&& func) -> task<std::invoke_result_t<Func>> {
    co_await schedule_on(get_global_executor());
    // Exception thrown here will be caught by promise_type::unhandled_exception()
    co_return func();
}

// Overload for void-returning functions
template<typename Func>
requires std::is_void_v<std::invoke_result_t<Func>>
task<void> async_convert_void(Func&& func) {
    co_await schedule_on(get_global_executor());
    func();
    co_return;
}

#endif //TASK_DO_ASYNC_HELPERS_H
