#include "executor.h"
#include "task.h"  // Required for sync_wait implementation
#include "executor_impl.inl"  // sync_wait implementation (needs complete task<T>)
#include <cstdio>
#include <exception>

executor::executor(size_t thread_count) {
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { worker_thread(); });
    }
}

executor::~executor() {
    shutdown();
}

void executor::schedule(std::coroutine_handle<> handle) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_) {
            return;
        }
        task_queue_.push(handle);
    }
    cv_.notify_one();
}

void executor::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    cv_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

size_t executor::pending_tasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return task_queue_.size();
}

void executor::worker_thread() {
    while (true) {
        std::coroutine_handle<> handle;
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { 
                return stopped_ || !task_queue_.empty(); 
            });
            
            if (stopped_ && task_queue_.empty()) {
                return;
            }
            
            if (!task_queue_.empty()) {
                handle = task_queue_.front();
                task_queue_.pop();
            }
        }
        
        if (handle) {
            try {
                handle.resume();
            } catch (const std::exception& e) {
                // Log exception but don't crash the worker thread
                std::fprintf(stderr, "[executor] Unhandled exception in coroutine: %s\n", e.what());
            } catch (...) {
                std::fprintf(stderr, "[executor] Unknown exception in coroutine\n");
            }
        }
    }
}
