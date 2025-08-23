//
// Created by asice-cloud on 8/24/25.
//

#include "reflect.h"

struct Point :public reflected_object{
    int x;
    int y;

    Point(int a,int b): x(a), y(b) {
        REGISTER_MEMBERS(MEMBER(x), MEMBER(y));
        REGISTER_FUNCTIONS(FUNCTION(inner_product));
    }

    int inner_product(int a, int b) const {
        return x * a + y * b;
    }
};

int main() {
    auto p= Point{1,2};
    p.print_reflection_info();
    auto result = p.call_function("inner_product", {3, 4});
    std::cout << "Inner product result: " << std::any_cast<int>(result) << std::endl;
    return 0;
}