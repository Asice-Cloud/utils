//
// Created by asice-cloud on 8/24/25.
//

#include "reflect.h"

struct Point :public reflected_object{
    int x;
    int y;

    Point(int a,int b): x(a), y(b) {
        REGISTER_MEMBERS(MEMBER(x), MEMBER(y));
        REGISTER_FUNCTIONS(FUNCTION(inner_product),FUNCTION(inner_with_other));
    }
    Point(const Point&) = delete;
    Point& operator=(const Point&) = delete;

    int inner_product(int a, int b) const {
        return x * a + y * b;
    }

    int inner_with_other(const Point& other) const {
        return x * other.x + y * other.y;
    }
};

int main() {
    auto p= Point{1,2};
    p.print_reflection_info();
    auto result = p.call_function("inner_product", {3, 4});
    std::cout << "Inner product result: " << std::any_cast<int>(result) << std::endl;

    // 测试 const Point& 参数的反射调用
    Point p2(7, 8);
    auto result2 = p.call_function("inner_with_other", {std::cref(p2)});
    std::cout << "Inner with other (ref) result: " << std::any_cast<int>(result2) << std::endl;

    p.visit_all_members(
     [](const std::string& name, const std::any& value, const std::string_view type) {
         std::cout << "Member: " << name << ", Type: " << type << '\n';
     },
     [](const std::string& name, std::string_view signature, size_t param_count, const std::vector<std::string>& param_types) {
         std::cout << "Function: " << name << ", Signature: " << signature << ", Param count: " << param_count << '\n';
     }
 );

    return 0;
}