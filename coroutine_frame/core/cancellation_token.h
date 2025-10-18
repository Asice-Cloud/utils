//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_CANCELLATION_TOKEN_H
#define TASK_DO_CANCELLATION_TOKEN_H

#include "task.h"
#include <atomic>
#include <memory>
#include <stdexcept>

// Exception thrown when a task is cancelled
class task_cancelled : public std::exception {
public:
    const char* what() const noexcept override {
        return "Task was cancelled";
    }
};

// Cancellation token that can be checked by coroutines
class cancellation_token {
public:
    cancellation_token() : cancelled_(std::make_shared<std::atomic<bool>>(false)) {}
    
    // Request cancellation
    void cancel() {
        cancelled_->store(true, std::memory_order_release);
    }
    
    // Check if cancellation was requested
    bool is_cancelled() const {
        return cancelled_->load(std::memory_order_acquire);
    }
    
    // Throw if cancelled
    void throw_if_cancelled() const {
        if (is_cancelled()) {
            throw task_cancelled();
        }
    }
    
private:
    std::shared_ptr<std::atomic<bool>> cancelled_;
};

// Awaitable that checks for cancellation
class cancellation_check {
public:
    explicit cancellation_check(const cancellation_token& token) 
        : token_(token) {}
    
    bool await_ready() const noexcept {
        // If already cancelled, throw immediately
        // Otherwise, don't suspend - this is just a check point
        return true;  // Don't suspend, check immediately
    }
    
    void await_suspend(std::coroutine_handle<> h) const noexcept {
        // Never called since await_ready returns true
    }
    
    void await_resume() const {
        token_.throw_if_cancelled();
    }
    
private:
    cancellation_token token_;
};

// Helper function to check cancellation in coroutines
inline cancellation_check check_cancelled(const cancellation_token& token) {
    return cancellation_check(token);
}

// Cancellable delay
inline task<void> cancellable_delay(
    std::chrono::milliseconds duration,
    const cancellation_token& token
) {
    co_await schedule_on(get_global_executor());
    
    auto start = std::chrono::steady_clock::now();
    auto end = start + duration;
    
    while (std::chrono::steady_clock::now() < end) {
        // Check for cancellation
        token.throw_if_cancelled();
        
        // Sleep in small intervals
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    co_return;
}

#endif //TASK_DO_CANCELLATION_TOKEN_H
