#include <fstream>;
#include <functional>
#include <iostream>

using CallBack = std::function<void(const std::string &)>;

void async_read(const std::string &filename, CallBack cb) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    cb("error: unable to open");
    return;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  cb(content);
}

void async_write(const std::string &filename, const std::string &content,
                 CallBack cb) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    cb("Error: unable to open file");
    return;
  }

  file << content;
  file.close();

  cb("Write successful");
}

int main() {
  // 异步读取文件
  async_read("example.txt", [](const std::string &content) {
    std::cout << "Read content: " << content << std::endl;
  });

  // 异步写入文件
  async_write("example.txt", "Hello, world!", [](const std::string &result) {
    std::cout << "Write result: " << result << std::endl;
  });

  return 0;
}
