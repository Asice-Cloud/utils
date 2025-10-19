//
// Created by asice-cloud on 10/19/25.
//

#ifndef TASK_DO_EXECUTOR_H
#define TASK_DO_EXECUTOR_H

#include <coroutine>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <cstdio>
#include <algorithm>

// Forward declaration
template<typename T>
class task;

// Simple task executor with async task scheduling support
class executor {
public:
    explicit executor(size_t thread_count = std::thread::hardware_concurrency());
    ~executor();

    // Submit a coroutine handle to the execution queue
    void schedule(std::coroutine_handle<> handle);
    
    // Stop the executor
    void shutdown();
    
    // Get the number of pending tasks
    size_t pending_tasks() const;

private:
    void worker_thread();
    
    std::vector<std::thread> workers_;
    std::queue<std::coroutine_handle<>> task_queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stopped_{false};
};

// Global executor instance
inline executor& get_global_executor() {
    // With epoll-based I/O, we don't need excessive threads since I/O waiting
    // happens in the epoll reactor thread. Use hardware_concurrency directly,
    // or a small multiple for CPU-bound work bursts.
    static executor exec([]{
        unsigned hc = std::thread::hardware_concurrency();
        size_t threads = hc ? std::max<size_t>(4, hc) : 4;
        return threads;
    }());
    return exec;
}

// Awaitable type for switching to executor thread in coroutine
struct schedule_awaiter {
    executor& exec_;
    
    explicit schedule_awaiter(executor& exec) : exec_(exec) {}
    
    bool await_ready() const noexcept { return false; }
    
    void await_suspend(std::coroutine_handle<> handle) {
        exec_.schedule(handle);
    }
    
    void await_resume() noexcept {}
};

// Helper function: schedule current coroutine on executor
inline schedule_awaiter schedule_on(executor& exec) {
    return schedule_awaiter{exec};
}

// Simulate async operation: delayed execution
struct delay_awaiter {
    std::chrono::milliseconds duration_;
    
    explicit delay_awaiter(std::chrono::milliseconds duration) 
        : duration_(duration) {}
    
    bool await_ready() const noexcept { return duration_.count() == 0; }
    
    void await_suspend(std::coroutine_handle<> handle) {
        std::thread([handle, duration = duration_]() {
            std::this_thread::sleep_for(duration);
            get_global_executor().schedule(handle);
        }).detach();
    }
    
    void await_resume() noexcept {}
};

// Helper function: async delay
inline delay_awaiter async_delay(std::chrono::milliseconds ms) {
    return delay_awaiter{ms};
}

// Forward declarations for sync_wait
template<typename T>
T sync_wait_impl(task<T>&& t, std::false_type);

void sync_wait_impl(task<void>&& t, std::true_type);

template<typename T>
auto sync_wait(task<T>&& t) -> T;

// Run a coroutine directly on the executor (fire and forget)
// Warning: Exceptions will be silently ignored
template<typename Task>
void async_run(Task&& t) {
    auto wrapper = [](Task t) -> void {
        try {
            auto awaiter = std::move(t).operator co_await();
            auto handle = awaiter.await_suspend(std::noop_coroutine());
            get_global_executor().schedule(handle);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "[async_run] Exception during task launch: %s\n", e.what());
        } catch (...) {
            std::fprintf(stderr, "[async_run] Unknown exception during task launch\n");
        }
    };
    
    std::thread(wrapper, std::forward<Task>(t)).detach();
}

// Run a coroutine on the executor and get a future-like handle
class async_result {
public:
    async_result() : completed_(false), result_(0) {}
    
    void set_result(int value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            result_ = value;
            completed_ = true;
        }
        cv_.notify_all();
    }
    
    void set_exception(std::exception_ptr ex) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            exception_ = ex;
            completed_ = true;
        }
        cv_.notify_all();
    }
    
    int get() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return completed_; });
        
        if (exception_) {
            std::rethrow_exception(exception_);
        }
        
        return result_;
    }
    
    bool is_ready() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return completed_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool completed_;  // Changed from atomic to regular bool protected by mutex
    int result_;
    std::exception_ptr exception_;
};

// Run a coroutine and return a handle to get the result later
template<typename Task>
std::shared_ptr<async_result> async_spawn(Task&& t) {
    auto result = std::make_shared<async_result>();
    
    auto wrapper = [](Task t, std::shared_ptr<async_result> result) -> void {
        try {
            auto awaiter = std::move(t).operator co_await();
            auto handle = awaiter.await_suspend(std::noop_coroutine());
            get_global_executor().schedule(handle);
            
            while (!handle.done()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            int value = awaiter.await_resume();
            result->set_result(value);
        } catch (...) {
            result->set_exception(std::current_exception());
        }
    };
    
    std::thread(wrapper, std::forward<Task>(t), result).detach();
    return result;
}

#endif //TASK_DO_EXECUTOR_H
