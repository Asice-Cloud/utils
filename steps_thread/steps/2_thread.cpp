#include <iostream>
#include <memory>
#include <thread>
// NOTE::error of thread
// undefined action

int a = 1;
std::thread t1;
void foo(int &x) { x += 1; }

void test1() {
  // send a by reference
  // int a = 1; // address of a will be destroy when test1 return.
  t1 = std::thread(foo, std::ref(a));
  t1.join();
}

class my_error_1 {
public:
  void func() { std::cout << "11111" << '\n'; }
};

class my_error_2 {
private:
  friend void fff_friend();
  void fff() { std::cout << "22222" << '\n'; }
};

void fff_friend() {
  std::shared_ptr<my_error_2> mee = std::make_shared<my_error_2>();
  std::thread t2(&my_error_2::fff, mee);
  t2.join();
}

int main() {
  test1();
  // t1.join();  //used for debug((()))
  std::cout << a << '\n';

  // my_error_1 me;
  // std::thread t2(&my_error_1::func, &me); // similar as above.
  std::shared_ptr<my_error_1> me = std::make_shared<my_error_1>();
  std::thread t2(&my_error_1::func, me);
  t2.join();

  fff_friend();
  return 0;
}
