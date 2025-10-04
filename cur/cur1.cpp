#include <iostream>
#include <source_location>
#include <thread>

void do_work() { std::cout << "helloworld\n"; }

int main() {
    std::thread t1(do_work);

    t1.join();

    auto sl = std::source_location::current();
    std::cout << "file name: " << sl.file_name() << "\n";
    std::cout << "line: " << sl.line() << "\n";
    std::cout << "column: " << sl.column() << "\n";
    std::cout << "function name: " << sl.function_name() << "\n";
    return 0;
}
