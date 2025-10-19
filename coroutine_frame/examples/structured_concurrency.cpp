// Structured Concurrency Example
// Demonstrates task_group for safe parallel execution

#include <print>
#include "../core.h"

using namespace std::chrono_literals;

// Simulate async operations
task<int> fetch_user(int id) {
    co_await schedule_on(get_global_executor());
    std::println("[fetch_user] Fetching user {}...", id);
    co_await async_delay(100ms + std::chrono::milliseconds(id * 50));
    std::println("[fetch_user] User {} fetched!", id);
    co_return id * 100;
}

task<std::string> fetch_profile(int user_id) {
    co_await schedule_on(get_global_executor());
    std::println("[fetch_profile] Fetching profile for user {}...", user_id);
    co_await async_delay(150ms);
    co_return "Profile_" + std::to_string(user_id);
}

task<void> risky_task(int id, bool should_fail) {
    co_await schedule_on(get_global_executor());
    std::println("[risky_task] Task {} starting...", id);
    co_await async_delay(50ms);
    
    if (should_fail) {
        std::println("[risky_task] Task {} THROWING exception!", id);
        throw std::runtime_error("Task " + std::to_string(id) + " failed!");
    }
    
    std::println("[risky_task] Task {} completed successfully", id);
}

// Example 1: Basic parallel execution
task<void> example_basic_parallel() {
    std::println("\n=== Example 1: Basic Parallel Execution ===");
    
    task_group group;
    
    // Spawn multiple tasks
    group.spawn(fetch_user(1), "user_1");
    group.spawn(fetch_user(2), "user_2");
    group.spawn(fetch_user(3), "user_3");
    
    std::println("All tasks spawned. Waiting for completion...");
    
    // Wait for all to complete
    co_await group.wait();
    
    std::println("All tasks completed! Group size: {}, Completed: {}", 
                group.size(), group.completed_count());
}

// Example 2: Scoped task group (RAII)
task<void> example_scoped_group() {
    std::println("\n=== Example 2: Scoped Task Group (RAII) ===");
    {
        scoped_task_group group;
        group.spawn(fetch_user(10));
        group.spawn(fetch_user(20));
        std::println("Tasks spawned in scoped group...");
        // Group automatically waits on scope exit
    }  // <-- Blocks here until all tasks complete
    std::println("Scope exited - all tasks guaranteed to be done!");
    co_return;
}

// Example 3: Exception handling
task<void> example_exception_handling() {
    std::println("\n=== Example 3: Exception Handling ===");
    
    task_group group;
    
    group.spawn(risky_task(1, false));  // Will succeed
    group.spawn(risky_task(2, true));   // Will throw
    group.spawn(risky_task(3, false));  // Will succeed
    
    try {
        co_await group.wait();
        std::println("All tasks completed successfully");
    } catch (const std::exception& e) {
        std::println("Caught exception from task group: {}", e.what());
    }
    co_return;
}

// Example 4: Cancellation
task<void> long_running_task(int id, std::shared_ptr<cancellation_token> token) {
    co_await schedule_on(get_global_executor());
    
    for (int i = 0; i < 20; i++) {
        if (token->is_cancelled()) {
            std::println("[Task {}] Cancelled at iteration {}", id, i);
            co_return;
        }
        
        std::println("[Task {}] Working... {}/20", id, i + 1);
        co_await async_delay(100ms);
    }
    
    std::println("[Task {}] Completed all iterations", id);
    co_return;  // Add explicit co_return for void task
}

task<void> example_cancellation() {
    std::println("\n=== Example 4: Cancellation ===");
    
    task_group group;
    auto token = group.get_token();
    
    group.spawn(long_running_task(1, token));
    group.spawn(long_running_task(2, token));
    group.spawn(long_running_task(3, token));
    
    // Let them run a bit
    co_await async_delay(500ms);
    
    std::println("\n>>> Cancelling all tasks! <<<\n");
    group.cancel_all();
    
    // Wait for graceful shutdown
    co_await group.wait();
    
    std::println("All tasks cancelled and cleaned up");
}

// Example 5: Nested task groups
task<void> nested_worker(int worker_id, int num_subtasks) {
    co_await schedule_on(get_global_executor());
    std::println("[Worker {}] Starting with {} subtasks", worker_id, num_subtasks);
    
    {
        scoped_task_group inner;
        
        for (int i = 0; i < num_subtasks; i++) {
            inner.spawn([](int wid, int sid) -> task<void> {
                co_await schedule_on(get_global_executor());
                std::println("  [W{}-S{}] Executing...", wid, sid);
                co_await async_delay(50ms);
                std::println("  [W{}-S{}] Done", wid, sid);
            }(worker_id, i));
        }
        
        // Inner group waits here
    }
    
    std::println("[Worker {}] All subtasks completed", worker_id);
}

task<void> example_nested_groups() {
    std::println("\n=== Example 5: Nested Task Groups ===");
    
    task_group outer;
    
    outer.spawn(nested_worker(1, 3));
    outer.spawn(nested_worker(2, 2));
    
    co_await outer.wait();
    
    std::println("Outer group completed - all nested tasks done!");
}

// Main
int main() {
    std::println("╔════════════════════════════════════════════╗");
    std::println("║   Structured Concurrency Examples         ║");
    std::println("╚════════════════════════════════════════════╝\n");
    
    try {
        sync_wait(example_basic_parallel());
        sync_wait(example_scoped_group());
        sync_wait(example_exception_handling());
        sync_wait(example_cancellation());
        sync_wait(example_nested_groups());
        
        std::println("\n✅ All examples completed!");
        
    } catch (const std::exception& e) {
        std::println("Fatal error: {}", e.what());
    }
    
    get_global_executor().shutdown();
    return 0;
}
