//
// Created by asice-cloud on 10/19/25.
//
// Core header - includes all essential coroutine library components
// Usage: #include "core.h"

#ifndef TASK_DO_CORE_H
#define TASK_DO_CORE_H

// ============================================================================
// Core Types
// ============================================================================
#include "core/task.h"              // Generic task<T> coroutine type
#include "core/executor.h"          // Thread pool executor
#include "core/executor_impl.inl"   // sync_wait implementation (must be included!)

// ============================================================================
// Utilities
// ============================================================================
#include "core/async_helpers.h"     // async_convert - sync to async conversion

// ============================================================================
// Concurrency Primitives
// ============================================================================
#include "core/when_all.h"          // Wait for all tasks concurrently
#include "core/when_any.h"          // Race between tasks
#include "core/task_group.h"        // Structured concurrency

// ============================================================================
// Cancellation & Timeout
// ============================================================================
#include "core/cancellation_token.h"  // Cooperative task cancellation
#include "core/timeout.h"             // Timeout support for tasks

// ============================================================================
// Error Handling
// ============================================================================
#include "core/error_handling.h"    // Error handling utilities (try_task, retry, etc.)

// ============================================================================
// I/O (Linux epoll backend)
// =========================================================================
#if defined(__linux__)
#include "core/io_epoll.h"           // read_ready, write_ready, async_sleep
#endif

// ============================================================================
// Commonly used namespaces and types
// ============================================================================
using namespace std::chrono_literals;  // For 100ms, 1s, etc.

// ============================================================================
// Quick Start Example:
// ============================================================================
/*
    #include "core.h"

    task<int> my_async_function() {
        co_await schedule_on(get_global_executor());
        co_await async_delay(100ms);
        co_return 42;
    }

    int main() {
        int result = sync_wait(my_async_function());
        get_global_executor().shutdown();
        return 0;
    }
*/

#endif //TASK_DO_CORE_H
