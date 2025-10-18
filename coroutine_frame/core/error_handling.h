//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_ERROR_HANDLING_H
#define TASK_DO_ERROR_HANDLING_H

#include "task.h"
#include <optional>
#include <functional>
#include <string>
#include <sstream>

// Error context for better debugging
struct error_context {
    std::string function;
    std::string file;
    int line;
    std::string message;
    
    std::string to_string() const {
        std::ostringstream oss;
        oss << file << ":" << line << " in " << function << ": " << message;
        return oss.str();
    }
};

// Exception with context
class task_error : public std::runtime_error {
public:
    explicit task_error(const error_context& ctx) 
        : std::runtime_error(ctx.to_string()), context_(ctx) {}
    
    const error_context& context() const { return context_; }
    
private:
    error_context context_;
};

// Macro for creating error context
#define TASK_ERROR_CONTEXT(msg) \
    error_context{__FUNCTION__, __FILE__, __LINE__, msg}

// try_task: Convert exceptions to std::optional
// Returns std::nullopt on error
template<typename T>
task<std::optional<T>> try_task(task<T> t) {
    try {
        T result = co_await std::move(t);
        co_return std::optional<T>{std::move(result)};
    } catch (...) {
        co_return std::nullopt;
    }
}

// try_task with error handler
template<typename T, typename ErrorHandler>
task<std::optional<T>> try_task(task<T> t, ErrorHandler&& on_error) {
    try {
        T result = co_await std::move(t);
        co_return std::optional<T>{std::move(result)};
    } catch (const std::exception& e) {
        on_error(e);
        co_return std::nullopt;
    } catch (...) {
        on_error(std::runtime_error("Unknown error"));
        co_return std::nullopt;
    }
}

// catch_and_log: Catch errors and log them
template<typename T>
task<T> catch_and_log(task<T> t, const char* task_name) {
    try {
        co_return co_await std::move(t);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "[ERROR] Task '%s' failed: %s\n", task_name, e.what());
        throw;
    } catch (...) {
        std::fprintf(stderr, "[ERROR] Task '%s' failed with unknown exception\n", task_name);
        throw;
    }
}

// retry: Retry a task on failure
template<typename TaskFactory>
auto retry(TaskFactory&& factory, int max_attempts, std::chrono::milliseconds delay) 
    -> decltype(factory()) {
    
    std::exception_ptr last_exception;
    
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        try {
            auto result = co_await factory();
            co_return result;
        } catch (const std::exception& e) {
            last_exception = std::current_exception();
            if (attempt == max_attempts) {
                std::fprintf(stderr, "[RETRY] All %d attempts failed. Last error: %s\n", 
                           max_attempts, e.what());
                std::rethrow_exception(last_exception);
            }
            std::fprintf(stderr, "[RETRY] Attempt %d/%d failed: %s. Retrying in %ldms...\n",
                       attempt, max_attempts, e.what(), delay.count());
            // Move delay outside try-catch
        }
        // Delay between retries
        co_await async_delay(delay);
    }
    
    throw std::runtime_error("retry: unreachable");
}

// fallback: Provide a fallback task if primary fails
template<typename T>
task<T> fallback(task<T> primary, task<T> backup) {
    std::exception_ptr primary_exception;
    try {
        auto result = co_await std::move(primary);
        co_return result;
    } catch (...) {
        primary_exception = std::current_exception();
    }
    
    // If we got here, primary failed, try backup
    auto result = co_await std::move(backup);
    co_return result;
}

// unwrap_or: Get result or default value
template<typename T>
task<T> unwrap_or(task<T> t, T default_value) {
    try {
        co_return co_await std::move(t);
    } catch (...) {
        co_return default_value;
    }
}

// map_error: Transform exception types
template<typename T, typename From, typename To>
task<T> map_error(task<T> t) {
    try {
        co_return co_await std::move(t);
    } catch (const From& e) {
        throw To(e.what());
    }
}

#endif //TASK_DO_ERROR_HANDLING_H
