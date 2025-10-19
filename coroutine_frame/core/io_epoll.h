// Epoll-based reactor and coroutine awaiters for true async I/O on Linux
// Minimal, one-shot awaiters: read_ready, write_ready, timer

#ifndef TASK_DO_IO_EPOLL_H
#define TASK_DO_IO_EPOLL_H

#include <coroutine>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#include "executor.h"

namespace io {

class EpollReactor {
public:
    static EpollReactor& instance();

    void wait_read(int fd, std::coroutine_handle<> h);
    void wait_write(int fd, std::coroutine_handle<> h);
    void wait_timer(std::chrono::milliseconds ms, std::coroutine_handle<> h);
    void shutdown();

private:
    EpollReactor();
    ~EpollReactor();
    void run();
    void arm_fd(int fd, uint32_t ev);
    void disarm_fd_if_idle(int fd);

    struct Waiters { std::coroutine_handle<> rd{nullptr}; std::coroutine_handle<> wr{nullptr}; uint32_t mask{0}; };

    int ep_ = -1;
    std::thread th_;
    std::atomic<bool> stop_{false};
    std::mutex mu_;
    std::unordered_map<int, Waiters> waiters_;
    std::unordered_map<int, std::coroutine_handle<>> timer_waiters_; // timerfd->handle
};

struct read_ready_awaiter {
    int fd;
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) { EpollReactor::instance().wait_read(fd, h); }
    void await_resume() noexcept {}
};

struct write_ready_awaiter {
    int fd;
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) { EpollReactor::instance().wait_write(fd, h); }
    void await_resume() noexcept {}
};

struct timer_awaiter {
    std::chrono::milliseconds ms;
    bool await_ready() const noexcept { return ms.count() <= 0; }
    void await_suspend(std::coroutine_handle<> h) { EpollReactor::instance().wait_timer(ms, h); }
    void await_resume() noexcept {}
};

inline read_ready_awaiter read_ready(int fd) { return read_ready_awaiter{fd}; }
inline write_ready_awaiter write_ready(int fd) { return write_ready_awaiter{fd}; }
inline timer_awaiter async_sleep(std::chrono::milliseconds ms) { return timer_awaiter{ms}; }

} // namespace io

#endif // TASK_DO_IO_EPOLL_H
