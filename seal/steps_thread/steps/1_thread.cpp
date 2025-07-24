#include <iostream>
#include <thread>

// NOTE :how to create a thread
void printf_hello(std::string name) { std::cout << "hello " << name << '\n'; }

int main() {
  std::thread t1([] { std::cout << "hello" << '\n'; });
  // t1.join();
  bool ok = t1.joinable();
  if (ok) {
    std::cout << "thread is joinable\n";
    t1.join();
  }

  // detach will run in the background(beyond even main exit) without waiting
  std::thread t2(printf_hello, "world");
  // t2.detach();
  t2.join();

  return 0;
}
