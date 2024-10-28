#include <condition_variable>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

// 读写状态
enum class ReadWriteState {
  SYNC, // 同步读写
  ASYNC // 异步读写
};

class ReadWriteManager {
public:
  ReadWriteManager() : state_(ReadWriteState::SYNC) {}

  // 同步读写操作
  void syncReadWrite(const std::string &filename, const std::string &content) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != ReadWriteState::SYNC) {
      // 如果当前状态不是同步读写，切换到异步读写
      state_ = ReadWriteState::ASYNC;
      asyncReadWrite(filename, content);
      return;
    }

    // 执行同步读写操作
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << "Error: unable to open file" << std::endl;
      return;
    }

    std::string data((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
      std::cout << "Error: unable to open file" << std::endl;
      return;
    }

    outFile << content;
    outFile.close();
  }

  // 异步读写操作
  void asyncReadWrite(const std::string &filename, const std::string &content) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != ReadWriteState::ASYNC) {
      // 如果当前状态不是异步读写，切换到同步读写
      state_ = ReadWriteState::SYNC;
      syncReadWrite(filename, content);
      return;
    }

    // 执行异步读写操作
    std::thread([this, filename, content]() {
      std::ifstream file(filename);
      if (!file.is_open()) {
        std::cout << "Error: unable to open file" << std::endl;
        return;
      }

      std::string data((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
      file.close();

      std::ofstream outFile(filename);
      if (!outFile.is_open()) {
        std::cout << "Error: unable to open file" << std::endl;
        return;
      }

      outFile << content;
      outFile.close();

      // 通知读写操作完成
      cv_.notify_all();
    }).detach();
  }

  // 等待读写操作完成
  void waitForReadWrite() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return state_ == ReadWriteState::SYNC; });
  }

private:
  ReadWriteState state_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

int main() {
  ReadWriteManager manager;
  manager.syncReadWrite("example.txt", "Hello, world!");
  manager.waitForReadWrite();
  return 0;
}
