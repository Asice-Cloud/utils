// #include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <type_traits>

template <typename F>
constexpr bool is_function = std::is_invocable_r_v<void,F,int>;

template <typename Func, typename... Ts>
std::enable_if_t<is_function<Func>, void> callback(Func &&f, Ts... ts) {
  std::forward<Func>(f)(ts...);
}

int main() {
  std::thread t1([] { std::cout << "char" << "1" << '\n'; });
  callback([](int i) { std::cout << i << '\n'; }, 1);
  t1.join();
}
