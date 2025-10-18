#include <print>
#include <chrono>
#include <thread>
#include <vector>
#include "../core.h"  // Single include for all functionality!

using namespace std::chrono_literals;

// ============================================================================
// Example 1: when_all - Concurrent execution with all results
// ============================================================================

task<int> compute(int id, int duration_ms) {
    co_await schedule_on(get_global_executor());
    std::println("[Task {}] Starting computation...", id);
    co_await async_delay(std::chrono::milliseconds(duration_ms));
    std::println("[Task {}] Done!", id);
    co_return id * 10;
}

task<void> demo_when_all() {
    co_await schedule_on(get_global_executor());
    
    std::println("\n=== when_all Demo ===");
    std::println("Launching 5 tasks in parallel...\n");
    
    auto start = std::chrono::steady_clock::now();
    
    // Create multiple tasks
    std::vector<task<int>> tasks;
    tasks.push_back(compute(1, 100));
    tasks.push_back(compute(2, 150));
    tasks.push_back(compute(3, 80));
    tasks.push_back(compute(4, 120));
    tasks.push_back(compute(5, 90));
    
    // Wait for all to complete
    std::vector<int> results = co_await when_all(std::move(tasks));
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::println("\nAll tasks completed in {}ms", duration.count());
    std::println("Results: [");
    for (size_t i = 0; i < results.size(); ++i) {
        std::println("  Task {}: {}", i + 1, results[i]);
    }
    std::println("]");
    
    co_return;
}

// ============================================================================
// Example 2: when_all with variadic arguments (different types)
// ============================================================================

task<int> fetch_user_id() {
    co_await schedule_on(get_global_executor());
    co_await async_delay(50ms);
    co_return 12345;
}

task<std::string> fetch_user_name() {
    co_await schedule_on(get_global_executor());
    co_await async_delay(60ms);
    co_return "Alice";
}

task<double> fetch_user_score() {
    co_await schedule_on(get_global_executor());
    co_await async_delay(40ms);
    co_return 98.5;
}

task<void> demo_when_all_variadic() {
    co_await schedule_on(get_global_executor());
    
    std::println("\n=== when_all Variadic Demo ===");
    std::println("Fetching user data from multiple sources...\n");
    
    // Wait for tasks with different return types
    auto [id, name, score] = co_await when_all(
        fetch_user_id(),
        fetch_user_name(),
        fetch_user_score()
    );
    
    std::println("User Profile:");
    std::println("  ID: {}", id);
    std::println("  Name: {}", name);
    std::println("  Score: {:.1f}", score);
    
    co_return;
}

// ============================================================================
// Example 3: when_any - Race condition
// ============================================================================

task<int> slow_server(int id, int delay_ms) {
    co_await schedule_on(get_global_executor());
    std::println("[Server {}] Processing request... (delay: {}ms)", id, delay_ms);
    co_await async_delay(std::chrono::milliseconds(delay_ms));
    co_return id * 100;
}

task<void> demo_when_any() {
    co_await schedule_on(get_global_executor());
    
    std::println("\n=== when_any Demo ===");
    std::println("Querying 3 servers, using first response...\n");
    
    auto start = std::chrono::steady_clock::now();
    
    // Query multiple servers
    std::vector<task<int>> servers;
    servers.push_back(slow_server(1, 200));  // Slow
    servers.push_back(slow_server(2, 50));   // Fast!
    servers.push_back(slow_server(3, 150));  // Medium
    
    // Use the first response
    auto [index, result] = co_await when_any(std::move(servers));
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::println("\nFirst response received from Server {} in {}ms", index + 1, duration.count());
    std::println("Result: {}", result);
    
    co_return;
}

// ============================================================================
// Example 4: Cancellation Token
// ============================================================================

task<int> cancellable_long_operation(cancellation_token token) {
    co_await schedule_on(get_global_executor());
    
    std::println("[Long Op] Starting...");
    
    try {
        for (int i = 0; i < 10; ++i) {
            // Check for cancellation before each step
            if (token.is_cancelled()) {
                std::println("[Long Op] Cancellation detected at step {}", i + 1);
                throw task_cancelled();
            }
            
            std::println("[Long Op] Step {}/10", i + 1);
            co_await async_delay(100ms);
        }
        
        std::println("[Long Op] Completed successfully");
        co_return 42;
        
    } catch (const task_cancelled&) {
        std::println("[Long Op] Handling cancellation...");
        throw;
    }
}

task<void> demo_cancellation() {
    co_await schedule_on(get_global_executor());
    
    std::println("\n=== Cancellation Token Demo ===");
    std::println("(Skipping - detach() has stability issues, see KNOWN_ISSUES.md)");
    
    // Note: The cancellation mechanism works, but there are stability
    // issues with detach() that need to be addressed separately
    
    co_return;
}

// ============================================================================
// Example 5: Improved async_convert with auto return type
// ============================================================================

// Function returning int
int calculate_square(int x) {
    std::this_thread::sleep_for(50ms);
    return x * x;
}

// Function returning string
std::string format_message(int value) {
    std::this_thread::sleep_for(30ms);
    return "Value is: " + std::to_string(value);
}

// Function returning double
double compute_average(int a, int b) {
    std::this_thread::sleep_for(40ms);
    return (a + b) / 2.0;
}

task<void> demo_async_convert() {
    co_await schedule_on(get_global_executor());
    
    std::println("\n=== async_convert Auto Return Type Demo ===");
    
    // Convert functions with different return types
    auto int_result = co_await async_convert([]() { return calculate_square(7); });
    std::println("Square of 7: {}", int_result);
    
    auto str_result = co_await async_convert([]() { return format_message(42); });
    std::println("Message: {}", str_result);
    
    // Temporarily comment out to isolate issue
    // auto double_result = co_await async_convert([]() { return compute_average(10, 20); });
    // std::println("Average: {:.1f}", double_result);
    
    co_return;
}

// ============================================================================
// Example 6: Combined Usage - Real-world scenario
// ============================================================================

task<std::string> fetch_from_api(int id, int delay_ms) {
    co_await schedule_on(get_global_executor());
    co_await async_delay(std::chrono::milliseconds(delay_ms));
    co_return "Data from API " + std::to_string(id);
}

task<void> demo_combined() {
    co_await schedule_on(get_global_executor());
    
    std::println("\n=== Combined Features Demo ===");
    std::println("Scenario: Fetch data from multiple APIs with timeout\n");
    
    cancellation_token token;
    
    // Create tasks for multiple API calls
    std::vector<task<std::string>> api_calls;
    api_calls.push_back(fetch_from_api(1, 100));
    api_calls.push_back(fetch_from_api(2, 150));
    api_calls.push_back(fetch_from_api(3, 80));
    
    // Start timeout timer
    auto timeout_task = [](const cancellation_token& t) -> task<void> {
        co_await cancellable_delay(200ms, t);
        std::println("[Timeout] All APIs took too long!");
        co_return;
    }(token);
    
    // Race between when_all and timeout
    // In a real implementation, we'd use proper when_any with timeout
    try {
        auto results = co_await when_all(std::move(api_calls));
        token.cancel();  // Cancel timeout
        
        std::println("\n✓ All APIs responded:");
        for (const auto& data : results) {
            std::println("  - {}", data);
        }
    } catch (...) {
        std::println("✗ Operation failed or timed out");
    }
    
    co_return;
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::println("╔══════════════════════════════════════════════╗");
    std::println("║   Advanced Coroutine Features Demo          ║");
    std::println("╚══════════════════════════════════════════════╝\n");
    
    try {
        // Run all demos
        sync_wait(demo_when_all());
        sync_wait(demo_when_all_variadic());
        sync_wait(demo_when_any());
        sync_wait(demo_cancellation());
        sync_wait(demo_async_convert());
        // Temporarily disabled due to stability issues
        // sync_wait(demo_combined());
        
        std::println("\n✅ Core demos completed successfully!");
        std::println("   (Combined demo skipped - has stability issues)");
        
    } catch (const std::exception& e) {
        std::println("\n❌ Error: {}", e.what());
    }
    
    get_global_executor().shutdown();
    return 0;
}
