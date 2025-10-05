#include <mutex>
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <vector>

class Locking {
private : 
    std::mutex mtx;
    std::shared_mutex sh_mtx;
    std::condition_variable cv;
    std::vector<int> data;
    int counter = 0;

public:
    // 1. lock_guard 示例 - 最常用
    void add_data(int value) {
        std::lock_guard<std::mutex> lock(mtx);  // 自动加锁
        data.push_back(value);
        counter++;
        std::cout << "添加数据: " << value << ", 总数: " << counter << std::endl;
        // 自动解锁
    }

    // 2. unique_lock 示例 - 灵活控制
    void process_data() {
        std::unique_lock<std::mutex> lock(mtx);  // 加锁
        
        if (data.empty()) {
            lock.unlock();  // 手动解锁
            std::cout << "数据为空，跳过处理" << std::endl;
            return;
        }
        
        int value = data.back();
        data.pop_back();
        counter--;
        lock.unlock();  // 手动解锁
        
        // 在锁外进行耗时操作
        std::cout << "处理数据: " << value << std::endl;
    }

    // 3. shared_lock 示例 - 读锁
    void read_data() {
        std::shared_lock<std::shared_mutex> lock(sh_mtx);  // 读锁
        std::cout << "读取数据，当前数量: " << data.size() << std::endl;
        // 多个线程可以同时读
    }

    // 4. unique_lock + shared_mutex 示例 - 写锁
    void clear_data() {
        std::unique_lock<std::shared_mutex> lock(sh_mtx);  // 写锁
        data.clear();
        counter = 0;
        std::cout << "清空所有数据" << std::endl;
        // 独占访问
    }

    // 5. 条件变量示例 - 生产者消费者
    void wait_for_data() {
        std::unique_lock<std::mutex> lock(mtx);  // 必须用unique_lock
        cv.wait(lock, [this] { return !data.empty(); });  // 等待条件满足
        
        int value = data.back();
        data.pop_back();
        counter--;
        std::cout << "等到数据: " << value << std::endl;
    }

    void notify_data_ready() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            data.push_back(999);
            counter++;
            std::cout << "通知: 数据准备好了!" << std::endl;
        }
        cv.notify_one();  // 唤醒一个等待线程
    }

    // 6. 带超时的等待
    bool wait_for_data_timeout(int timeout_ms) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
                       [this] { return !data.empty(); })) {
            int value = data.back();
            data.pop_back();
            counter--;
            std::cout << "超时前获得数据: " << value << std::endl;
            return true;
        } else {
            std::cout << "等待超时，没有数据" << std::endl;
            return false;
        }
    }
};

int main() {
    Locking demo;
    
    std::cout << "=== 基础锁演示 ===" << std::endl;
    // 测试 lock_guard
    demo.add_data(1);
    demo.add_data(2);
    demo.add_data(3);
    
    // 测试 unique_lock
    demo.process_data();
    
    std::cout << "\n=== 读写锁演示 ===" << std::endl;
    // 测试读写锁
    std::thread reader1([&]() { demo.read_data(); });
    std::thread reader2([&]() { demo.read_data(); });
    std::thread writer([&]() { 
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        demo.clear_data(); 
    });
    
    reader1.join();
    reader2.join();
    writer.join();
    
    std::cout << "\n=== 条件变量演示 ===" << std::endl;
    
    // 1. 基本条件变量使用
    std::thread waiter([&]() {
        std::cout << "等待线程: 开始等待数据..." << std::endl;
        demo.wait_for_data();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        demo.notify_data_ready();
    });
    
    waiter.join();
    notifier.join();
    
    // 2. 超时等待演示
    std::cout << "\n=== 超时等待演示 ===" << std::endl;
    
    std::thread timeout_waiter([&]() {
        std::cout << "超时等待: 等待300ms..." << std::endl;
        demo.wait_for_data_timeout(300);  // 等待300ms
    });
    
    // 不发送通知，让它超时
    timeout_waiter.join();
    
    std::cout << "\n程序结束" << std::endl;
    return 0;
}