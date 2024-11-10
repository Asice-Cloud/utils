#include <iostream>
#include <mutex>
#include <thread>

// NOTE: :mutual variable
// WARN: :dead lock

int common = 0;
std::mutex mtx;

void func1() {
  for (int i = 0; i < 10000; i++) {
    mtx.lock();
    common += 1;
    mtx.unlock();
  }
}

// std::mutex mt1, mt2;
//
// void dead1() {
//   while (1) {
//     mt1.lock();
//     mt2.lock();
//     mt1.unlock();
//     mt2.unlock();
//   }
// }
// void dead2() {
//   while (1) {
//     mt2.lock();
//     mt1.lock();
//     mt1.unlock();
//     mt2.unlock();
//   }
// }

int main() {
  std::thread t1(func1);
  std::thread t2(func1);
  t1.join();
  t2.join();

  std::cout << common << '\n';

  // std::thread t3(dead1);
  // std::thread t4(dead2);
  // t3.join();
  // t4.join();
  // std::cout << "done\n" << '\n';

  return 0;
}
