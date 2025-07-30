// NOTE :asynchronous
#include <future>
#include <iostream>

int func()
{
  int i = 0;
  for (int j = 0; j < 10000; j++)
  {
    i++;
  }
  return i;
}

// packaged_task: could encapsulate a invocable object into an asynchronous
// task. could not be copied, just use in its localscope or move

int multiply(int x, int y) { return x * y; }

// promise: create a value in thread and get its future
void set_value(std::promise<int> &pp) { pp.set_value(112233); }

int main()
{
  std::future<int> f = std::async(std::launch::async, func);
  // f.wait();
  std::cout << func() << '\n';
  std::cout << f.get() << '\n';

  std::packaged_task<int(int, int)> task(multiply);
  std::future<int> ft = task.get_future();
  std::thread tt(std::move(task), 234, 567);
  tt.join();
  std::cout << ft.get() << '\n';

  std::promise<int> ppt;
  std::future<int> ff1 = ppt.get_future();
  std::thread tt1(set_value, std::ref(ppt));
  // std::thread tt1(std::move(set_value), std::ref(ppt));
  tt1.join();
  std::cout << ff1.get() << '\n';

  return 0;
}
