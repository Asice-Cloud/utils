#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <tbb/blocked_range3d.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_invoke.h>
#include <tbb/task_group.h>
#include <thread>
#include <vector>

void download(std::string str) {
  for (int i = 0; i < 10; i++) {
    std::cout << "downloading " << i * 10 << "%" << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void doing() {
  std::string name;
  std::cin >> name;
  std::cout << "hello " << name << '\n';
}

int main() {

  tbb::task_group tg;

  tg.run([&] { download("test"); });
  tg.run([&] { doing(); });

  tg.wait();
  tbb::parallel_invoke([&] { download("test"); }, [&] { doing(); });

  size_t n = 10;
  std::vector<double> a(n * n * n); // Adjusted the size to n * n * n

  tbb::parallel_for(
      tbb::blocked_range3d<size_t>(0, n, 0, n, 0, n),
      [&](tbb::blocked_range3d<size_t> r) {
        for (size_t i = r.pages().begin(); i != r.pages().end(); ++i) {
          for (size_t j = r.cols().begin(); j != r.cols().end(); ++j) {
            for (size_t k = r.rows().begin(); k != r.rows().end(); ++k) {
              a[(i * n + j) * n + k] = std::sin(i) * std::sin(j) * std::sin(k);
            }
          }
        }
      });
  for (auto &i : a) {
    std::cout << i << '\n';
  }
  std::cout << "end";
  return 0;
}
