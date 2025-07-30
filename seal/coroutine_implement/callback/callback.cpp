#include <fstream>
#include <functional>
#include <future>
#include <iostream>

using CallBack = std::function<void(const std::string &)>;

void async_read(const std::string &filename, CallBack cb) {
  std::async(std::launch::async, [filename, cb]() {
    std::ifstream file(filename);
    if (!file.is_open()) {
      cb("error: unable to open");
      return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    cb(content);
  });
}

void async_write(const std::string &filename, const std::string &content,
                 CallBack cb) {
  std::async(std::launch::async, [filename, content, cb]() {
    std::ofstream file(filename);
    if (!file.is_open()) {
      cb("Error: unable to open file");
      return;
    }

    file << content;
    file.close();
    cb("Write successful");
  });
}

int main() {
  async_read("callback.cpp", [](const std::string &content) {
    std::cout << "Read successful\n";
    std::cout << content << std::endl;
  });

  async_write("callback.txt", "Hello, callback",
              [](const std::string &msg) { std::cout << msg << std::endl; });

  std::this_thread::sleep_for(std::chrono::seconds(1));
  return 0;
}
