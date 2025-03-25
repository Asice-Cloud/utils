#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <type_traits>

std::vector<std::thread> lists;

template <typename T>
concept is_int =requires(T t){  std::is_same_v<std::decay_t<T>, int>;};

template <typename T>
void print_int(T t)
  requires is_int<T>
{
  std::cout << "it int" << '\n';
}

class test_da
{
  private:
  int value = 1;

  public:

 void set(int v)
  {
    value = v;
  }

  int get() const
  {
    return value;
  }
};

int main() {
  std::thread t1([] { std::cout << "1" << '\n'; });
  lists.emplace_back(std::move(t1));

  for (auto &i : lists) {
    i.join();
  }

  print_int(111);

  auto re = []() ->int
  {
    return 42;
  };
  std::cout<< re() <<std::endl;

  test_da td = test_da();
  std::cout<<td.get();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  return 0;
}
