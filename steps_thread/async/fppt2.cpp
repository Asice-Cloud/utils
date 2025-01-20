#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <utility>

class Async {
public:
  template <typename Func, typename... Args>
  static auto run(Func &&func, Args &&...args)
      -> std::future<typename std::result_of<Func(Args...)>::type> {
    using DecayType = typename std::decay<Func>::type;
    using ReturnType = typename std::result_of<DecayType(
        typename std::decay<Args>::type...)>::type;

    // 创建一个 packaged_task
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

    // 获取 future 对象
    std::future<ReturnType> result = task->get_future();

    // 启动一个线程来执行任务
    std::thread([task]() { (*task)(); }).detach();

    return result;
  }
};

int asyncFunction(int x) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return x * 2;
}

int main() {
  // 使用封装的 Async 工具类
  std::future<int> futureResult = Async::run(asyncFunction, 10);

  // 获取并输出结果
  std::cout << "Async result: " << futureResult.get() << std::endl;

  return 0;
}
