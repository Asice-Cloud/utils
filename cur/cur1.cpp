#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

void do_work(int i) { std::cout << "helloworld from do work," << i << "\n"; }

class thread_g {
    std::thread t;

  public:
    explicit thread_g(std::thread &to) : t(std::move(to)) {

        if (!t.joinable()) {
            throw std::logic_error("no thread");
        }
    }
    ~thread_g() { t.join(); }
};

struct func {
    void operator()(int a, std::string const &s) {
        std::cout << "helloworld from func\n";
        std::cout << "a=" << a << " s=" << s << "\n";
    }
    int &i;
    func(int &ii) : i(ii) {
        do_work(1);
        ++i;
    }
};

int main() {

    std::thread tt(do_work, 42);
    tt.detach();

    int locat_state = 0;
    func myf(locat_state);
    std::string s = "hello";
    std::thread t1(myf, 42, std::ref(s));
    std::thread t2 = std::move(t1);
    thread_g g(t2);
    std::cout << "locat_state=" << locat_state << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
