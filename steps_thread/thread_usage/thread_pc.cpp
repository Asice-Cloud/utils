#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class Producer {
public:
  void produce(int item) {
    std::cout << "Producing item: " << item << std::endl;
  }
};

class Consumer {
public:
  void consume(int item) {
    std::cout << "Consuming item: " << item << std::endl;
  }
};

int main() {
  std::vector<std::thread> threads;

  Producer producer;
  Consumer consumer;

  std::queue<int> queue;
  std::condition_variable cv;
  std::mutex mutex;
  bool done = false;

  threads.emplace_back([&] {
    for (int i = 0; i < 10; ++i) {
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
    }
  });

  threads.emplace_back([&] {
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(3));
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [&] { return !queue.empty() || done; });

      while (!queue.empty()) {
        int item = queue.front();
        queue.pop();
        lock.unlock();
        consumer.consume(item);
        lock.lock();
      }

      if (done && queue.empty())
        break;
    }
  });

  for (auto &thread : threads)
    thread.join();
  return 0;
}
