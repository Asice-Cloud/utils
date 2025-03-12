#include <variant>

template <typename... T>
concept is_int = requires{
  std::is_same_v<std::variant<T...>,std::variant<int>>;
}

int main() {}
