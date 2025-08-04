#include "reflect.h"
#include <iostream>

// 演示反射功能的人员类
class person : public reflected_object
{
public:
    std::string name;
    int age;
    double height;
    bool is_employed;

    person(const std::string &n = "Unknown", int a = 0)
        : name(n), age(a), height(170.0), is_employed(false)
    {
        // 批量注册所有成员变量和函数
        REGISTER_MEMBERS(MEMBER(name), MEMBER(age), MEMBER(height), MEMBER(is_employed));
        REGISTER_FUNCTIONS(FUNCTION(introduce), FUNCTION(get_info), FUNCTION(set_age));
    }

    void introduce()
    {
        std::cout << "Hi, I'm " << name << ", " << age << " years old, "
                  << height << "cm tall, " << (is_employed ? "employed" : "unemployed") << std::endl;
    }

    std::string get_info()
    {
        return name + " (Age: " + std::to_string(age) + ", Height: " + std::to_string(height) + "cm)";
    }

    void set_age(int new_age)
    {
        std::cout << "Age updated: " << age << " -> " << new_age << std::endl;
        age = new_age;
    }
};

int main()
{
    std::cout << "=== C++ Reflection Demo ===\n\n";

    person p("Alice", 25);

    // 属性反射演示
    std::cout << "=== Property Reflection ===\n";
    p.print_reflection_info();

    p.set_property("name", std::string("Bob"));
    p.set_property("age", 30);
    p.set_property("is_employed", true);
    std::cout << "\nAfter modification:\n";
    p.print_reflection_info();

    // 函数反射演示
    std::cout << "\n=== Function Reflection ===\n";
    p.call_function("introduce");

    auto info = p.call_function("get_info");
    std::cout << "Info: " << std::any_cast<std::string>(info) << std::endl;

    p.call_function("set_age", {35});

    // 访问者模式演示
    std::cout << "\n=== Visitor Pattern ===\n";
    p.visit_all_members(
        [](const std::string &name, const std::any &value, std::string_view type)
        {
            std::cout << "  [Property] " << name << " (" << type << ") = ";
            if (value.type() == typeid(int))
                std::cout << std::any_cast<int>(value);
            else if (value.type() == typeid(std::string))
                std::cout << "\"" << std::any_cast<std::string>(value) << "\"";
            else if (value.type() == typeid(double))
                std::cout << std::any_cast<double>(value);
            else if (value.type() == typeid(bool))
                std::cout << (std::any_cast<bool>(value) ? "true" : "false");
            std::cout << "\n";
        },
        [](const std::string &name, std::string_view signature, size_t param_count,
           const std::vector<std::string> &param_types)
        {
            std::cout << "  [Function] " << name << " -> " << signature << " (params: " << param_count << ")\n";
        });

    std::cout << "\n=== Demo completed! ===\n";
    return 0;
}
