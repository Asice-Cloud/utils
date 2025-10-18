// executor_impl.inl
// Implementation of sync_wait that requires complete task<T> type definition
// This file should be included AFTER task.h is fully defined

#ifndef TASK_DO_EXECUTOR_IMPL_INL
#define TASK_DO_EXECUTOR_IMPL_INL

#include <thread>
#include <atomic>
#include <exception>

// Helper for non-void sync_wait
template<typename T>
T sync_wait_impl(task<T>&& t, std::false_type /* is_void */) {
    std::atomic<bool> completed{false};
    T result{};
    std::exception_ptr exception;
    
    auto wrapper = [](task<T> task_obj, std::atomic<bool>& completed, T& result, std::exception_ptr& exception) -> void {
        try {
            auto awaiter = std::move(task_obj).operator co_await();
            auto handle = awaiter.await_suspend(std::noop_coroutine());
            get_global_executor().schedule(handle);
            
            while (!handle.done()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            result = awaiter.await_resume();
        } catch (...) {
            exception = std::current_exception();
        }
        completed = true;
    };
    
    std::thread worker(wrapper, std::move(t), std::ref(completed), std::ref(result), std::ref(exception));
    worker.join();
    
    if (exception) {
        std::rethrow_exception(exception);
    }
    
    return result;
}

// Helper for void sync_wait
inline void sync_wait_impl(task<void>&& t, std::true_type /* is_void */) {
    std::atomic<bool> completed{false};
    std::exception_ptr exception;
    
    auto wrapper = [](task<void> task_obj, std::atomic<bool>& completed, std::exception_ptr& exception) -> void {
        try {
            auto awaiter = std::move(task_obj).operator co_await();
            auto handle = awaiter.await_suspend(std::noop_coroutine());
            get_global_executor().schedule(handle);
            
            while (!handle.done()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            awaiter.await_resume();
        } catch (...) {
            exception = std::current_exception();
        }
        completed = true;
    };
    
    std::thread worker(wrapper, std::move(t), std::ref(completed), std::ref(exception));
    worker.join();
    
    if (exception) {
        std::rethrow_exception(exception);
    }
}

// Synchronously wait for a task to complete
// Exceptions from the task will be re-thrown to the caller
template<typename T>
auto sync_wait(task<T>&& t) -> T {
    return sync_wait_impl(std::move(t), std::is_void<T>{});
}

#endif // TASK_DO_EXECUTOR_IMPL_INL
