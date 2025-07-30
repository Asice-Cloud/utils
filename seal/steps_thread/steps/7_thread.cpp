#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  ThreadPool(size_t numThreads) : done(false) {
    for (size_t i = 0; i < numThreads; ++i) {
      threads.emplace_back([this]() {
        while (true) {
          std::function<void()> task;
          std::unique_lock<std::mutex> ulock(mtx);
          cv.wait(ulock, [this] { return !tasks.empty() || done; });
          if (done && tasks.empty()) {
            return;
          }

          task = std::move(tasks.front());
          tasks.pop();
          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> ulock(mtx);
      done = true;
    }
    cv.notify_all();
    for (std::thread &thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  template <class Func, class... Args> void en_queue(Func &&f, Args &&...args) {
    std::unique_lock<std::mutex> ulock(mtx);
    std::function<void()> task =
        std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
    tasks.emplace(std::move(task));
    cv.notify_one();
  }

private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;
  std::mutex mtx;
  std::condition_variable cv;
  bool done;
};

int main() {
  ThreadPool pool(4);
  for (int i = 0; i < 10; i++) {
    pool.en_queue([i]() {
      std::cout << "task:" << i << " hello" << '\n';
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "task" << i << " world" << '\n';
    });
  }

  return 0;
}
