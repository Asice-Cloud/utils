//
// Created by asice-cloud on 10/19/25.
//
// Structured concurrency support - task_group
// Ensures all child tasks complete before parent scope exits

#ifndef TASK_DO_TASK_GROUP_H
#define TASK_DO_TASK_GROUP_H

#include "task.h"
#include "executor.h"
#include "cancellation_token.h"
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <exception>
#include <chrono>
using namespace std::chrono_literals;

// Task group for structured concurrency
// All spawned tasks must complete before the group is destroyed
class task_group {
public:
    task_group() : token_(std::make_shared<cancellation_token>()) {}
    
    // Spawn a task into this group (non-blocking)
    template<typename T>
    void spawn(task<T>&& t, std::string name = "") {
        auto state = std::make_shared<task_state>();
        state->name = name;
        
        // Wrapper task that catches exceptions and marks completion
        auto wrapper = [](task<T> t, 
                         std::shared_ptr<task_state> s,
                         std::shared_ptr<cancellation_token> tok) -> task<void> {
            try {
                // Check cancellation before starting
                if (tok->is_cancelled()) {
                    s->cancelled = true;
                    s->completed.store(true);
                    co_return;
                }
                
                // Execute the task
                if constexpr (std::is_void_v<T>) {
                    co_await std::move(t);
                } else {
                    [[maybe_unused]] auto result = co_await std::move(t);
                }
                
                s->completed.store(true);
            } catch (...) {
                s->exception = std::current_exception();
                s->completed.store(true);
            }
        };
        
        // Launch wrapper task
        wrapper(std::move(t), state, token_).detach();
        
        // Track in group
        std::lock_guard lock(mutex_);
        tasks_.push_back(state);
    }
    
    // Wait for all tasks to complete
    task<void> wait() {
        co_await schedule_on(get_global_executor());
        
        while (true) {
            bool all_done = true;
            std::vector<std::exception_ptr> exceptions;
            
            {
                std::lock_guard lock(mutex_);
                for (auto& s : tasks_) {
                    if (!s->completed.load()) {
                        all_done = false;
                        break;
                    }
                    if (s->exception) {
                        exceptions.push_back(s->exception);
                    }
                }
            }
            
            if (all_done) {
                // Rethrow first exception if any
                if (!exceptions.empty()) {
                    std::rethrow_exception(exceptions[0]);
                }
                co_return;
            }
            
            // Yield and check again
            co_await async_delay(1ms);
        }
    }
    
    // Cancel all tasks in the group
    void cancel_all() {
        token_->cancel();
    }
    
    // Get number of tasks
    size_t size() const {
        std::lock_guard lock(mutex_);
        return tasks_.size();
    }
    
    // Get number of completed tasks
    size_t completed_count() const {
        std::lock_guard lock(mutex_);
        size_t count = 0;
        for (auto& s : tasks_) {
            if (s->completed.load()) count++;
        }
        return count;
    }
    
    // Check if all tasks are done
    bool is_done() const {
        std::lock_guard lock(mutex_);
        for (auto& s : tasks_) {
            if (!s->completed.load()) return false;
        }
        return true;
    }
    
    // Get shared cancellation token (for child tasks to check)
    std::shared_ptr<cancellation_token> get_token() const {
        return token_;
    }
    
    // Destructor: in debug mode, assert all tasks completed
    // In production, you might want to wait synchronously (blocking)
    ~task_group() {
        #ifdef NDEBUG
        // Production: optionally wait for tasks (blocking)
        // Uncomment if you want strict structured concurrency:
        // if (!is_done()) {
        //     cancel_all();
        //     sync_wait(wait());
        // }
        #else
        // Debug: assert all tasks completed
        if (!is_done()) {
            std::fprintf(stderr, "[task_group] Warning: Destroying task_group with %zu incomplete tasks!\n", 
                        size() - completed_count());
        }
        #endif
    }
    
    // Delete copy/move to ensure clear ownership
    task_group(const task_group&) = delete;
    task_group& operator=(const task_group&) = delete;
    task_group(task_group&&) = delete;
    task_group& operator=(task_group&&) = delete;

private:
    struct task_state {
        std::string name;
        std::atomic<bool> completed{false};
        bool cancelled{false};
        std::exception_ptr exception;
    };
    
    std::vector<std::shared_ptr<task_state>> tasks_;
    std::shared_ptr<cancellation_token> token_;
    mutable std::mutex mutex_;
};

// RAII helper: auto-wait on scope exit
class scoped_task_group {
public:
    scoped_task_group() = default;
    
    template<typename T>
    void spawn(task<T>&& t, std::string name = "") {
        group_.spawn(std::move(t), name);
    }
    
    void cancel_all() {
        group_.cancel_all();
    }
    
    task_group& group() { return group_; }
    
    // On destruction, synchronously wait for all tasks
    ~scoped_task_group() {
        if (!group_.is_done()) {
            // Block until all complete
            sync_wait(group_.wait());
        }
    }
    
private:
    task_group group_;
};

#endif // TASK_DO_TASK_GROUP_H
