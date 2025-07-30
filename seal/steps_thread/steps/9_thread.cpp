// atomic operations
#include <iostream>
#include <thread>
#include <atomic>

int ss1 = 0;
std::mutex mtx;
void fu()
{
    for (int i = 0; i < 100000; i++)
    {
        mtx.lock();
        ss1++;
        mtx.unlock();
    }
}

std::atomic<int> shared = 0;
void func()
{
    for (int i = 0; i < 100000; i++)
    {
        shared++;
    }
}

int main()
{
    auto ti1 = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::thread t1(fu);
    std::thread t11(fu);
    auto ti2 = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    t1.join();
    t11.join();
    std::cout << "time: " << ti2 - ti1 << '\n';
    auto ti3 = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::thread t2(func);
    std::thread t22(func);
    auto ti4 = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    t2.join();
    t22.join();
    std::cout << "time: " << ti4 - ti3 << std::endl;

    std::cout << shared << std::endl;
    return 0;
}