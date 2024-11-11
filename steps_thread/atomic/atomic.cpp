#include <iostream>
#include <thread>
#include <vector>

void atomic_add(int *ptr, int value)
{
    asm volatile(
        "lock xadd %1, %0"
        : "+m"(*ptr), "+r"(value)
        :
        : "memory");
}

void increment(int *shared)
{
    for (int i = 0; i < 100000; ++i)
    {
        atomic_add(shared, 1);
    }
}

int main()
{
    int shared = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back(increment, &shared);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "Final value of shared: " << shared << std::endl;

    return 0;
}