#include <iostream>
#include <type_traits>

template <typename T>
concept is_int = requires(T t) { std::is_same_v<std::decay_t<T>, int>; };

template <typename T>
void print_int(T t)
  requires is_int<T>
{
  std::cout << t << " " << "is int" << '\n';
}

int main() { print_int(11); }
