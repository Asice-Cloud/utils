#include <print>
#include "core.h"  // Single include!

// Simple hello world example
task<int> hello_async() {
    co_await schedule_on(get_global_executor());
    std::println("Hello from async coroutine!");
    co_return 42;
}

int main() {
    std::println("C++23 Coroutine Library - Quick Test\n");
    
    int result = sync_wait(hello_async());
    std::println("Result: {}\n", result);
    
    std::println("✓ Library works! See examples/ for more demos.");
    
    get_global_executor().shutdown();
    return 0;
}
