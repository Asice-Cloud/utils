# C++23 Coroutine Library

Lightweight async coroutine library with thread pool executor.

## Features

- ✅ Generic `task<T>` - supports any return type
- ✅ Thread pool executor with 4 workers
- ✅ `when_all` / `when_any` - **TRUE parallel** execution
- ✅ Timeout support - `with_timeout()` for task timeouts
- ✅ Error handling - `try_task`, `retry`, `fallback`, `unwrap_or`
- ✅ Cancellation tokens - cooperative cancellation
- ✅ `async_convert` - sync → async conversion
- ✅ Fire-and-forget with `detach()` - **memory safe**
- ✅ Single header - just `#include "core.h"`

## Quick Start

```cpp
#include "core.h"  // Single header includes everything!

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

## Core API

### Task Creation

```cpp
task<int> get_number();          // Returns int
task<std::string> get_text();    // Returns string  
task<void> do_work();            // Returns void
```

### Execution

```cpp
// Switch to executor thread
co_await schedule_on(get_global_executor());

// Wait synchronously (blocks)
T result = sync_wait(my_task());

// Fire-and-forget
my_task().detach();

// Delay
co_await async_delay(std::chrono::milliseconds(100));
```

### Concurrency

```cpp
// when_all: collect all results
std::vector<task<int>> tasks = {...};
auto results = co_await when_all(std::move(tasks));  // vector<int>

// when_all (variadic)
auto [id, name, score] = co_await when_all(
    fetch_id(),    // task<int>
    fetch_name(),  // task<string>  
    fetch_score()  // task<double>
);

// when_any: race for first result
auto [index, value] = co_await when_any(std::move(tasks));
```

### Cancellation

```cpp
cancellation_token token;

task<void> work(cancellation_token token) {
    while (!token.is_cancelled()) {
        co_await async_delay(100ms);
    }
}

token.cancel();  // Request cancellation
```

### Timeout

```cpp
// Timeout support
task<int> slow_task() {
    co_await async_delay(5s);
    co_return 42;
}

try {
    auto result = co_await with_timeout(slow_task(), 1s);
} catch (const timeout_error& e) {
    // Task timed out
}
```

### Error Handling

```cpp
// try_task: convert exceptions to optional
std::optional<int> result = co_await try_task(risky_task());
if (!result) { /* handle error */ }

// unwrap_or: provide default value on error
int value = co_await unwrap_or(risky_task(), 999);

// fallback: try primary, use backup on failure
int value = co_await fallback(primary_task(), backup_task());

// retry: retry with exponential backoff
int value = co_await retry(flaky_task(), 5, 10ms);
```

### Utilities

```cpp
// Convert sync function to async
auto result = co_await async_convert([]() { 
    return expensive_operation(); 
});
```

## Examples

```bash
# Build
mkdir build && cd build
cmake .. && ninja

# Run
./task_do              # Quick test
./basic_demo           # Thread switching, pipelines
./http_server          # Async HTTP server
./advanced_features    # when_all, when_any, cancellation
./core_features_test   # Core features test suite (detach, parallel, timeout, errors)
```

## How It Works

### Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                  User Coroutine                     │
│   task<T> my_task() {                               │
│       co_await schedule_on(executor);               │
│       co_return result;                             │
│   }                                                 │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│              promise_type (task<T>)                 │
│  • get_return_object() → creates task<T>            │
│  • initial_suspend() → suspend at start             │
│  • final_suspend() → suspend at end                 │
│  • return_value(T) / return_void()                  │
│  • unhandled_exception() → catches exceptions       │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│            coroutine_handle<promise_type>           │
│  • Represents suspended coroutine state             │
│  • Can be resumed with .resume()                    │
│  • Stores local variables & suspension point        │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│                Thread Pool Executor                 │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐          │
│  │ Worker 1 │  │ Worker 2 │  │ Worker 3 │  ...     │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘          │
│       │             │              │                 │
│       └─────────────┴──────────────┘                │
│                     │                                │
│              ┌──────▼──────┐                         │
│              │  Task Queue │                         │
│              │ (thread-safe)│                        │
│              └─────────────┘                         │
└─────────────────────────────────────────────────────┘
```

### Core Concepts

#### 1. **Coroutine State Machine**

When you write:
```cpp
task<int> compute() {
    int x = 10;              // State: initial
    co_await schedule_on(executor);  // Suspension point 1
    int y = x + 20;          // State: after schedule
    co_await async_delay(100ms);     // Suspension point 2
    co_return y;             // State: final
}
```

The compiler transforms it into a state machine:
```cpp
struct compute_coroutine {
    int x, y;  // Local variables stored in coroutine frame
    int state = 0;
    
    void resume() {
        switch(state) {
            case 0: x = 10; state = 1; suspend(); return;
            case 1: y = x + 20; state = 2; suspend(); return;
            case 2: return_value(y); state = 3; return;
        }
    }
};
```

#### 2. **Awaitable Pattern**

Every `co_await expr` requires `expr` to be an **awaitable** object with:
```cpp
struct my_awaitable {
    bool await_ready();              // Is result ready?
    void await_suspend(handle);      // What to do when suspending?
    T await_resume();                // Return value when resuming
};
```

Example - `schedule_on(executor)`:
```cpp
struct schedule_awaiter {
    bool await_ready() { return false; }  // Always suspend
    
    void await_suspend(coroutine_handle<> h) {
        executor.schedule(h);  // Put handle in queue
    }
    
    void await_resume() {}  // No return value
};
```

#### 3. **Execution Flow**

```
[Main Thread]                    [Worker Thread]
     │
     ├─ Create task<int> t
     │  (coroutine suspended at initial_suspend)
     │
     ├─ Call sync_wait(t)
     │     │
     │     ├─ Get awaiter from task
     │     ├─ Call await_suspend(handle)
     │     │      │
     │     │      └─ Schedule handle on executor ───┐
     │     │                                          │
     │     └─ Spin wait for completion               │
     │           │                                    │
     │           │                                    ▼
     │           │                            Executor picks up handle
     │           │                                    │
     │           │                                    ├─ handle.resume()
     │           │                                    ├─ Execute coroutine body
     │           │                                    ├─ Hit co_return
     │           │                                    └─ Set result, mark done
     │           │                                    
     │           ◄────────────────────────────────────┘
     │           (done flag = true)
     │
     ├─ Get result from awaiter
     └─ Return result
```

#### 4. **Thread Pool Design**

```cpp
class executor {
    queue<coroutine_handle<>> tasks_;  // Thread-safe queue
    vector<thread> workers_;
    atomic<bool> running_;
    
    void worker_thread() {
        while (running_) {
            auto handle = tasks_.pop();  // Block until task available
            if (handle) {
                handle.resume();  // Execute on this thread
            }
        }
    }
};
```

#### 5. **Task Coordination**

**when_all** - Wait for multiple tasks (TRUE PARALLEL):
```cpp
task<vector<T>> when_all(vector<task<T>> tasks) {
    // Launch all tasks concurrently with detach()
    auto state = make_shared<when_all_state<T>>();
    for (auto& t : tasks) {
        when_all_task(std::move(t), state).detach();
    }
    
    // Wait for all to complete
    while (state->completed < tasks.size()) {
        std::this_thread::sleep_for(1ms);
    }
    co_return state->results;
}
```

**when_any** - Race between tasks:
```cpp
task<pair<size_t, T>> when_any(vector<task<T>> tasks) {
    while (true) {
        for (size_t i = 0; i < tasks.size(); i++) {
            if (tasks[i].is_ready()) {
                T result = co_await tasks[i];
                co_return {i, result};
            }
        }
        co_await async_delay(1ms);  // Polling (not ideal)
    }
}
```

### Memory Management

```
task<T> object (stack/heap)
    │
    └─► coroutine_handle<promise_type>
            │
            └─► Coroutine Frame (heap-allocated by compiler)
                    ├─ promise_type object
                    ├─ Local variables
                    ├─ Suspension point state
                    └─ Parameter copies
```

**Lifetime:**
- Frame allocated when coroutine first called
- Frame destroyed when:
  - Coroutine runs to completion
  - task<T> destructor called (if not awaited)
  - `detach()` called and coroutine completes

**Problem with `detach()`:**
- Detached coroutines may outlive their captured references
- No ownership tracking → potential use-after-free

### Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| Create task | O(1) | Allocates coroutine frame |
| co_await | O(1) | State save + schedule |
| Schedule on executor | O(1) | Queue push |
| Worker pickup | O(1) amortized | Queue pop (blocking) |
| when_all(N tasks) | O(N) | Sequential await |
| when_any(N tasks) | O(N × polls) | Polling-based |

**Memory Overhead:**
- Each task: ~64-256 bytes (coroutine frame)
- Executor: 4 threads × stack size (~2MB each)
- Task queue: O(pending tasks)

## API Reference

### Core Types
| Type/Function | Description |
|---------------|-------------|
| `task<T>` | Coroutine return type |
| `co_await task` | Wait for task result |
| `task.detach()` | Fire-and-forget (memory safe) |
| `sync_wait(task)` | Block until complete |

### Execution
| Function | Description |
|----------|-------------|
| `schedule_on(executor)` | Switch to executor thread |
| `async_delay(duration)` | Async sleep |
| `get_global_executor()` | Get global thread pool |

### Concurrency
| Function | Description |
|----------|-------------|
| `when_all(tasks...)` | Wait all, TRUE parallel execution |
| `when_any(tasks)` | Get first completed task |

### Error Handling
| Function | Description |
|----------|-------------|
| `try_task(task)` | Convert exception to `optional<T>` |
| `unwrap_or(task, default)` | Return default on error |
| `fallback(primary, backup)` | Try primary, fallback to backup |
| `retry(task, count, delay)` | Retry with exponential backoff |
| `catch_and_log(task, logger)` | Log exceptions |
| `map_error(task, mapper)` | Transform exception types |

### Timeout & Cancellation
| Function | Description |
|----------|-------------|
| `with_timeout(task, duration)` | Timeout support |
| `cancellation_token` | Cooperative cancellation |
| `token.cancel()` | Request cancellation |
| `token.is_cancelled()` | Check if cancelled |

### Utilities
| Function | Description |
|----------|-------------|
| `async_convert(func)` | Convert sync function to async |

## Project Structure

```
task_do/
├── core.h                    # Single unified header (include this!)
├── core/                     # Framework implementation
│   ├── task.h                # Generic task<T> type
│   ├── executor.h/.cpp       # Thread pool (4 workers)
│   ├── executor_impl.inl     # sync_wait implementation
│   ├── async_helpers.h       # async_convert utility
│   ├── when_all.h            # Concurrent coordination (parallel)
│   ├── when_any.h            # Task racing
│   ├── cancellation_token.h  # Cancellation support
│   ├── timeout.h             # Timeout support
│   └── error_handling.h      # Error handling utilities
├── examples/                 # Demo programs
│   ├── basic_demo.cpp
│   ├── http_server.cpp
│   ├── advanced_features.cpp
│   └── core_features_test.cpp
└── main.cpp                  # Quick test
```

## Requirements

- C++23 (GCC 13+, Clang 16+)
- CMake 4.0+
- pthreads

## Known Limitations

- **Polling mechanism**: `when_all` uses 1ms polling instead of condition variables
- **Fixed thread pool**: 4 workers hardcoded, not configurable
- **Cooperative cancellation**: Cancellation is not preemptive
- **No I/O integration**: No built-in async file/network I/O support

## License

MIT
