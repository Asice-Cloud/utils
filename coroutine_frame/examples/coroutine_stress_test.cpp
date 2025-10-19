// Coroutine Framework Stress Test
// Spawns a large number of concurrent coroutines to test scheduling, context switching, and memory safety

#include <print>
#include <vector>
#include <chrono>
#include <atomic>
#include "../core.h"

using namespace std::chrono_literals;

// Simple async task: does a small delay, then increments a counter
task<void> simple_task(std::atomic<int>& counter, int delay_ms) {
    co_await schedule_on(get_global_executor());
    co_await async_delay(std::chrono::milliseconds(delay_ms));
    counter++;
}

task<void> stress_test(int total, int concurrency, int delay_ms) {
    std::atomic<int> counter{0};
    int launched = 0;
    auto start = std::chrono::steady_clock::now();
    while (launched < total) {
        task_group group;
        int batch = std::min(concurrency, total - launched);
        for (int i = 0; i < batch; ++i) {
            group.spawn(simple_task(counter, delay_ms));
        }
        launched += batch;
        co_await group.wait();
    }
    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();
    std::println("Launched {} coroutines, completed: {} in {:.3f} seconds", total, counter.load(), sec);
    std::println("Throughput: {:.1f} tasks/sec", total / sec);
}

int main(int argc, char* argv[]) {
    int total = 100000;
    int concurrency = 1000;
    int delay_ms = 1;
    if (argc > 1) total = std::atoi(argv[1]);
    if (argc > 2) concurrency = std::atoi(argv[2]);
    if (argc > 3) delay_ms = std::atoi(argv[3]);
    sync_wait(stress_test(total, concurrency, delay_ms));
    get_global_executor().shutdown();
    return 0;
}
