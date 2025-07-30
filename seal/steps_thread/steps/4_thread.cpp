#include <iostream>
#include <mutex>
#include <thread>

// NOTE : lock_guard and unique_lock
/*
 * lock_guard: it will lock the mutex(when invoke construction) and unlock
 it(deconstruction)
 *  -------------it could not be copied and moved, just use in its localscope
 * unique_lock: lock() ,try_lock(), try_lock_for(), try_lock_until(), unlock(
 )
 * */

std::mutex mtx;
int common = 0;

void func1() {
  for (int i = 0; i < 10000; i++) {
    // we do not need to write lock and unlock anymore
    std::lock_guard<std::mutex> lg(mtx);
    common += 1;
  }
}

std::timed_mutex mtx_t;
void func2() {
  // std::unique_lock<std::mutex> ul(mtx); // lock
  std::unique_lock<std::timed_mutex> ul2(
      mtx_t, std::defer_lock); // defer, it will not lock when construction
  std::this_thread::sleep_for(std::chrono::seconds(4));
  // ul2.lock();
  if (ul2.try_lock_for(std::chrono::seconds(5))) { // just wait for 5s
    common += 1;
  }
  // ul2.unlock();
}

int main() {
  std::thread t1(func1);
  std::thread t2(func1);
  t1.join();
  t2.join();
  std::cout << common << '\n';

  std::thread t3(func2);
  t3.join();
  std::cout << common << '\n';
  return 0;
}
