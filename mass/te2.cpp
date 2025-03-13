#include <iostream>
#include <thread>
#include <vector>

std::vector<std::thread> lists;

int main() {
  std::thread t1([] { std::cout << "1"; });
  lists.emplace_back(std::move(t1));

  for (auto &i : lists) {
    i.join();
  }

  return 0;
}
