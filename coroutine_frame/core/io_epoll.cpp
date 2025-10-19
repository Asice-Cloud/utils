#include "io_epoll.h"
#include <cstring>

namespace io {

EpollReactor& EpollReactor::instance() {
    static EpollReactor inst;
    return inst;
}

EpollReactor::EpollReactor() {
    ep_ = epoll_create1(EPOLL_CLOEXEC);
    if (ep_ < 0) {
        std::perror("epoll_create1");
        throw std::runtime_error("epoll_create1 failed");
    }
    th_ = std::thread([this]{ run(); });
}

EpollReactor::~EpollReactor() {
    shutdown();
}

void EpollReactor::shutdown() {
    bool expected = false;
    if (!stop_.compare_exchange_strong(expected, true)) return;
    if (ep_ >= 0) {
        // Wake epoll_wait by closing epoll fd in destructor path is risky; safer to use a timerfd
        // Here we add a 0ms timer to wake the loop
        int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (tfd >= 0) {
            itimerspec its{}; its.it_value.tv_nsec = 1; // wake asap
            timerfd_settime(tfd, 0, &its, nullptr);
            epoll_event ev{}; ev.events = EPOLLIN; ev.data.fd = tfd;
            epoll_ctl(ep_, EPOLL_CTL_ADD, tfd, &ev);
        }
    }
    if (th_.joinable()) th_.join();
    if (ep_ >= 0) { close(ep_); ep_ = -1; }
}

void EpollReactor::arm_fd(int fd, uint32_t ev) {
    epoll_event e{}; e.events = ev; e.data.fd = fd;
    if (epoll_ctl(ep_, EPOLL_CTL_MOD, fd, &e) < 0) {
        if (errno == ENOENT) {
            epoll_ctl(ep_, EPOLL_CTL_ADD, fd, &e);
        }
    }
}

void EpollReactor::disarm_fd_if_idle(int fd) {
    auto it = waiters_.find(fd);
    if (it == waiters_.end()) return;
    if (!it->second.rd && !it->second.wr) {
        epoll_ctl(ep_, EPOLL_CTL_DEL, fd, nullptr);
        waiters_.erase(it);
    }
}

void EpollReactor::wait_read(int fd, std::coroutine_handle<> h) {
    std::lock_guard lk(mu_);
    auto &w = waiters_[fd];
    w.rd = h;
    w.mask |= EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    arm_fd(fd, w.mask);
}

void EpollReactor::wait_write(int fd, std::coroutine_handle<> h) {
    std::lock_guard lk(mu_);
    auto &w = waiters_[fd];
    w.wr = h;
    w.mask |= EPOLLOUT | EPOLLHUP | EPOLLERR;
    arm_fd(fd, w.mask);
}

void EpollReactor::wait_timer(std::chrono::milliseconds ms, std::coroutine_handle<> h) {
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (tfd < 0) {
        // Fallback: schedule after delay on executor
        get_global_executor().schedule(h);
        return;
    }
    itimerspec its{};
    its.it_value.tv_sec = ms.count() / 1000;
    its.it_value.tv_nsec = (ms.count() % 1000) * 1000000;
    timerfd_settime(tfd, 0, &its, nullptr);
    {
        std::lock_guard lk(mu_);
        timer_waiters_[tfd] = h;
    }
    epoll_event ev{}; ev.events = EPOLLIN; ev.data.fd = tfd;
    epoll_ctl(ep_, EPOLL_CTL_ADD, tfd, &ev);
}

void EpollReactor::run() {
    constexpr int MAXE = 64;
    epoll_event events[MAXE];
    while (!stop_) {
        int n = epoll_wait(ep_, events, MAXE, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            uint32_t ev = events[i].events;

            // Timer?
            {
                std::coroutine_handle<> h{};
                {
                    std::lock_guard lk(mu_);
                    auto it = timer_waiters_.find(fd);
                    if (it != timer_waiters_.end()) { h = it->second; timer_waiters_.erase(it); }
                }
                if (h) {
                    uint64_t exp; ::read(fd, &exp, sizeof(exp));
                    close(fd);
                    get_global_executor().schedule(h);
                    continue;
                }
            }

            // I/O fd
            std::coroutine_handle<> rd{}, wr{};
            {
                std::lock_guard lk(mu_);
                auto it = waiters_.find(fd);
                if (it != waiters_.end()) {
                    if (ev & (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) { rd = it->second.rd; it->second.rd = nullptr; it->second.mask &= ~(EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR); }
                    if (ev & (EPOLLOUT | EPOLLHUP | EPOLLERR)) { wr = it->second.wr; it->second.wr = nullptr; it->second.mask &= ~(EPOLLOUT | EPOLLHUP | EPOLLERR); }
                    if (it->second.mask) arm_fd(fd, it->second.mask); else disarm_fd_if_idle(fd);
                }
            }
            if (rd) get_global_executor().schedule(rd);
            if (wr) get_global_executor().schedule(wr);
        }
    }
}

} // namespace io
