# C++23 Coroutine Async Library

## API Reference

| Type/Function                | Description                                      |
|------------------------------|--------------------------------------------------|
| `task<T>`                    | Coroutine return type (any T/void)               |
| `co_await task`              | Await task result                                |
| `task.detach()`              | Fire-and-forget, memory safe                     |
| `sync_wait(task)`            | Block until task completes                       |
| `task_group`                 | Structured concurrency: manage child tasks       |
| `scoped_task_group`          | RAII: auto-wait for all tasks on scope exit      |
| `when_all(tasks...)`         | Wait all, true parallel execution                |
| `when_any(tasks)`            | Get first completed task                         |
| `schedule_on(executor)`      | Switch to executor thread                        |
| `async_delay(duration)`      | Async sleep (thread-based)                       |
| `io::read_ready(fd)`         | Await fd readable (epoll, Linux)                 |
| `io::write_ready(fd)`        | Await fd writable (epoll, Linux)                 |
| `io::async_sleep(duration)`  | Event-driven timer (Linux)                       |
| `with_timeout(task, dur)`    | Timeout support                                  |
| `cancellation_token`         | Cooperative cancellation                         |
| `try_task(task)`             | Convert exception to optional<T>                 |
| `unwrap_or(task, def)`       | Return default on error                          |
| `fallback(primary, backup)`  | Try primary, fallback to backup                  |
| `retry(task, n, delay)`      | Retry with backoff                               |
| `async_convert(func)`        | Convert sync function to async                   |

## Features

- ✅ C++23 coroutine/await, any return type
- ✅ True async I/O (epoll reactor, Linux)
- ✅ Thread pool executor, auto-scaled
- ✅ Structured concurrency (`task_group`)
- ✅ when_all / when_any, parallel aggregation
- ✅ Timeout, cancellation, error handling
- ✅ Single header include: `#include "core.h"`

## Quick Start

```cpp
#include "core.h"

task<int> compute() {
    co_await schedule_on(get_global_executor());
    co_return 42;
}

int main() {
    int result = sync_wait(compute());
    get_global_executor().shutdown();
    return 0;
}
```

## Architecture & Principles

- **Event-driven I/O**: `io::read_ready`/`write_ready`/`async_sleep` use epoll, no thread blocking, 10k+ connections per thread.
- **Thread pool executor**: CPU-bound tasks run on auto-sized thread pool, I/O waits are event-driven.
- **Structured concurrency**: `task_group`/`scoped_task_group` ensure all child tasks complete or cancel before parent continues, exceptions/cancellation propagate.
- **Memory safety**: `detach()` is safe, coroutine frame auto-destroyed, no leaks.
- **Error/cancel/timeout**: Unified awaitable error/timeout/cancel handling, see API.

## Example: Structured Concurrency

```cpp
#include "core.h"

task<void> example() {
    task_group group;
    group.spawn(fetch_user(1));
    group.spawn(fetch_user(2));
    co_await group.wait();
}
```

## Example: True Async HTTP Server

- See `examples/http_server.cpp` for a fully event-driven, coroutine-based HTTP server.
- See `examples/websocket_server.cpp` for a true async WebSocket chat demo.

---
MIT License. Linux, C++23, GCC 13+/Clang 16+ required.
