#include <iostream>
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>
// NOTE :condition variable, it should use with unique_lock

std::queue<int> q_product;
std::condition_variable cv;
std::mutex mtx;

void producer()
{
    for (int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::unique_lock<std::mutex> ulock(mtx);
        q_product.push(i);
        std::cout << "producer: " << i << '\n';
        cv.notify_one();
    }
}

void consumer()
{
    for (int i = 0; i < 10; i++)
    {
        std::unique_lock<std::mutex> ulock(mtx);
        cv.wait(ulock, []() -> bool
                { return !q_product.empty(); });
        int product = q_product.front();
        q_product.pop();
        std::cout << "consumer: " << product << '\n';
    }
}

int main()
{
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
    return 0;
}
