#include <chrono>
#include <future>
#include <iostream>
#include <thread>

// 使用 std::async
int asyncFunction() {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return 10;
}

// 使用 std::promise 和 std::future
void promiseFunction(std::promise<int> &prom) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  prom.set_value(20);
}

// 使用 std::packaged_task
int packagedTaskFunction(int x) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return x * x;
}

int main() {
  // 使用 std::async
  std::future<int> asyncFuture = std::async(std::launch::async, asyncFunction);
  std::cout << "Async result: " << asyncFuture.get() << std::endl;

  // 使用 std::promise 和 std::future
  // std::promise 是一个可以存放结果的对象，可以复制和移动
  // std::future 是一个可以获取结果的对象
  std::promise<int> prom;
  std::future<int> promFuture = prom.get_future();
  std::thread promThread(std::move(promiseFunction), std::ref(prom));
  std::cout << "Promise result: " << promFuture.get() << std::endl;
  promThread.join();

  // 使用 std::packaged_task, std::packaged_task
  // 是一个可调用对象，它可以包装任何可调用目标， 它的返回值可以通过 std::future
  // 获取, 不可以复制，只能移动
  std::packaged_task<int(int)> task(packagedTaskFunction);
  std::future<int> taskFuture = task.get_future();
  std::thread taskThread(std::move(task), 5);
  std::cout << "Packaged task result: " << taskFuture.get() << std::endl;
  taskThread.join();

  return 0;
}
