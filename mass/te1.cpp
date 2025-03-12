// #include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <type_traits>

template <typename F>
constexpr bool is_function = std::is_invocable_r_v<void, F, int>;

template <typename T>
concept Function = requires(T t) { t(1); };

template <typename Func, typename... Ts>
std::enable_if_t<is_function<Func>, void> callback(Func &&f, Ts... ts) {
  std::forward<Func>(f)(ts...);
}

template <typename F>
  requires Function<F>
void callback2(F &&f, int i) {
  std::forward<F>(f)(i);
}

int main() {
  std::thread t1([] { std::cout << "char" << "1" << '\n'; });
  callback([](int i) { std::cout << i << '\n'; }, 1);
  t1.join();

  callback2([](int i) { std::cout << "char" << i << '\n'; }, 1);
}
