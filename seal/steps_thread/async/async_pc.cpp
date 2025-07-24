#include <iostream>
#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <queue>

class Producer
{
public:
    void produce(int item)
    {
        std::cout << "Producing item: " << item << std::endl;
    }
};

class Consumer
{
public:
    void consume(int item)
    {
        std::cout << "Consuming item: " << item << std::endl;
    }
};

int main()
{
    Producer producer;
    Consumer consumer;

    std::queue<int> queue;
    std::condition_variable cv;
    std::mutex mutex;
    bool done = false;

    std::future<void> producerFuture = std::async(std::launch::async, [&]
                                                  {
        for (int i = 0; i < 10; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::unique_lock<std::mutex> lock(mutex);
                producer.produce(i);
                queue.push(i);
            }
            cv.notify_one();
            std::this_thread::yield(); // 让出线程，给消费者机会
        }
        {
            std::unique_lock<std::mutex> lock(mutex);
            done = true;
            cv.notify_all();
        } });

    std::future<void> consumerFuture = std::async(std::launch::async, [&]
                                                  {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&]()->bool{ return !queue.empty() || done; });

            while (!queue.empty())
            {
                int item = queue.front();
                queue.pop();
                lock.unlock();
                consumer.consume(item);
                lock.lock();
            }

            if (done && queue.empty())
                break;
        } });

    producerFuture.get();
    consumerFuture.get();

    return 0;
}