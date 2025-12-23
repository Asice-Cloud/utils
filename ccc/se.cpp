#include <system_error>

std::error_code make_error_code(int ev) {
    return std::error_code(ev, std::generic_category());
}

std::error_condition make_error_condition(int ev) {
    return std::error_condition(ev, std::generic_category());
}

// Example usage
#include <iostream>

int main() {
    std::error_code ec = make_error_code(1);
    std::error_condition cond = make_error_condition(2);

    std::cout << "Error Code: " << ec.value() << ", Category: " << ec.category().name() << std::endl;
    std::cout << "Error Condition: " << cond.value() << ", Category: " << cond.category().name() << std::endl;

    return 0;
}
