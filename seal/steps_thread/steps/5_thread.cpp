#include <iostream>
#include <mutex>
#include <string>
#include <thread>

// NOTE :singleton in multip-thread, call_once

class Log
{
public:
  static Log &getInstance()
  {
    std::call_once(once, init);
    return *instance;
  }
  Log() = default;
  Log(const Log &) = delete;
  Log &operator=(const Log &) = delete;

  static void init()
  {
    if (!instance)
    {
      instance = new Log();
      std::cout << "initializing!" << '\n';
    }
  }

  void print_log(std::string msg)
  {
    std::cout << __LINE__ << " " << msg << '\n';
  }

private:
  static Log *instance;
  static std::once_flag once;
};
Log *Log::instance = nullptr;
std::once_flag Log::once;

void print_error() { Log::getInstance().print_log("error"); }

int main()
{

  std::thread t1(print_error);
  std::thread t2(print_error);
  t1.join();
  t2.join();

  return 0;
}
