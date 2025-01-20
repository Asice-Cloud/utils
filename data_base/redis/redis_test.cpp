#include <iostream>
#include <sw/redis++/redis++.h>

using namespace sw::redis;

int main() {
  auto redis = Redis("tcp://127.0.0.1:6379");
  std::cout << redis.ping() << '\n';
}
