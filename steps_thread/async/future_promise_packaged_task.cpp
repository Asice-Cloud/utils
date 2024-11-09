#include <iostream>
#include <thread>
#include <future>
#include <chrono>

// 使用 std::async
int asyncFunction()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 10;
}

// 使用 std::promise 和 std::future
void promiseFunction(std::promise<int> &prom)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    prom.set_value(20);
}

// 使用 std::packaged_task
int packagedTaskFunction(int x)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return x * x;
}

int main()
{
    // 使用 std::async
    std::future<int> asyncFuture = std::async(std::launch::async, asyncFunction);
    std::cout << "Async result: " << asyncFuture.get() << std::endl;

    // 使用 std::promise 和 std::future
    std::promise<int> prom;
    std::future<int> promFuture = prom.get_future();
    std::thread promThread(promiseFunction, std::ref(prom));
    std::cout << "Promise result: " << promFuture.get() << std::endl;
    promThread.join();

    // 使用 std::packaged_task
    std::packaged_task<int(int)> task(packagedTaskFunction);
    std::future<int> taskFuture = task.get_future();
    std::thread taskThread(std::move(task), 5);
    std::cout << "Packaged task result: " << taskFuture.get() << std::endl;
    taskThread.join();

    return 0;
}