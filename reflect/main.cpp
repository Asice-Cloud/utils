#include "reflect.h"
#include <iostream>
#include <string>

// 演示反射功能的人员类
class person : public reflected_object {
  public:
    std::string name;
    int age;
    double height;
    bool is_employed;

    person(const std::string &n = "Unknown", int a = 0)
        : name(n), age(a), height(170.0), is_employed(false) {
        // 批量注册所有成员变量和函数
        REGISTER_MEMBERS(MEMBER(name), MEMBER(age), MEMBER(height),
                         MEMBER(is_employed));
        REGISTER_FUNCTIONS(FUNCTION(introduce), FUNCTION(get_info),
                           FUNCTION(set_age), FUNCTION(calculate_bmi));
    }

    void introduce() {
        std::cout << "Hi, I'm " << name << ", " << age << " years old, "
                  << height << "cm tall, "
                  << (is_employed ? "employed" : "unemployed") << std::endl;
    }

    std::string get_info() {
        return name + " (Age: " + std::to_string(age) +
               ", Height: " + std::to_string(height) + "cm)";
    }

    void set_age(int new_age) {
        std::cout << "Age updated: " << age << " -> " << new_age << std::endl;
        age = new_age;
    }

    // 新增：演示多参数函数
    double calculate_bmi(double weight, bool use_metric = true) {
        double height_m = use_metric ? height / 100.0 : height; // 转换为米
        double bmi = weight / (height_m * height_m);
        std::cout << "BMI calculation: " << weight << "kg / (" << height_m
                  << "m)² = " << bmi << std::endl;
        return bmi;
    }
};

// 演示可变参数功能的测试类
class variadic_demo : public reflected_object {
  public:
    std::string name;
    int value;
    double ratio;

    variadic_demo(const std::string &n = "demo", int v = 42)
        : name(n), value(v), ratio(1.0) {
        // 注册属性
        REGISTER_MEMBER(name);
        REGISTER_MEMBER(value);
        REGISTER_MEMBER(ratio);

        // 注册不同参数数量的函数 - 展示可变参数模板的威力
        REGISTER_FUNCTION(func0); // 0 参数
        REGISTER_FUNCTION(func1); // 1 参数
        REGISTER_FUNCTION(func2); // 2 参数
        REGISTER_FUNCTION(func3); // 3 参数
        REGISTER_FUNCTION(func4); // 4 参数
        REGISTER_FUNCTION(func5); // 5 参数
    }

    // 0 参数函数
    void func0() { std::cout << "func0() - no parameters" << std::endl; }

    // 1 参数函数
    int func1(int x) {
        std::cout << "func1(" << x << ") - returns " << (x * 2) << std::endl;
        return x * 2;
    }

    // 2 参数函数
    std::string func2(int x, const std::string &s) {
        std::string result = s + "_" + std::to_string(x);
        std::cout << "func2(" << x << ", \"" << s << "\") - returns \""
                  << result << "\"" << std::endl;
        return result;
    }

    // 3 参数函数
    double func3(int a, double b, bool c) {
        double result = c ? (a + b) : (a - b);
        std::cout << "func3(" << a << ", " << b << ", "
                  << (c ? "true" : "false") << ") - returns " << result
                  << std::endl;
        return result;
    }

    // 4 参数函数
    void func4(int a, double b, const std::string &c, bool d) {
        std::cout << "func4(" << a << ", " << b << ", \"" << c << "\", "
                  << (d ? "true" : "false") << ")";
        if (d) {
            std::cout << " - processed: " << c << "_" << (a + b);
        }
        std::cout << std::endl;
    }

    // 5 参数函数
    std::string func5(int a, double b, const std::string &c, bool d, float e) {
        std::string result = c + "_" + std::to_string(a) + "_" +
                             std::to_string(b) + "_" + std::to_string(e);
        if (d)
            result += "_enabled";
        std::cout << "func5(5 params) - returns \"" << result << "\""
                  << std::endl;
        return result;
    }
};

// 分割线函数
void print_separator(const std::string &title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "=== " << title << " ===" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

int main() {
    std::cout << "🚀 C++ Variadic Template Reflection System Demo 🚀"
              << std::endl;

    // ===== 基础反射功能演示 =====
    print_separator("Basic Reflection Features");

    person p("Alice", 25);
    p.print_reflection_info();

    std::cout << "\n--- Property Modification ---" << std::endl;
    p.set_property("name", std::string("Bob"));
    p.set_property("age", 30);
    p.set_property("is_employed", true);

    std::cout << "\nAfter modifications:" << std::endl;
    std::cout << "Name: " << std::any_cast<std::string>(*p.get_property("name"))
              << std::endl;
    std::cout << "Age: " << std::any_cast<int>(*p.get_property("age"))
              << std::endl;
    std::cout << "Employed: "
              << (std::any_cast<bool>(*p.get_property("is_employed")) ? "Yes"
                                                                      : "No")
              << std::endl;

    std::cout << "\n--- Function Calls ---" << std::endl;
    p.call_function("introduce");
    auto info = p.call_function("get_info");
    std::cout << "Info: " << std::any_cast<std::string>(info) << std::endl;

    p.call_function("set_age", {35});
    auto bmi = p.call_function("calculate_bmi", {70.5, true});
    std::cout << "BMI: " << std::any_cast<double>(bmi) << std::endl;

    // ===== 可变参数模板演示 =====
    print_separator("Variadic Template Functions (0-5 Parameters)");

    variadic_demo demo("test_obj", 100);
    demo.print_reflection_info();

    std::cout << "\n--- Testing All Parameter Counts ---" << std::endl;

    try {
        // 0 参数
        std::cout << "🔹 0 parameters: ";
        demo.call_function("func0");

        // 1 参数
        std::cout << "🔹 1 parameter: ";
        auto result1 = demo.call_function("func1", {42});

        // 2 参数
        std::cout << "🔹 2 parameters: ";
        auto result2 = demo.call_function("func2", {123, std::string("hello")});

        // 3 参数
        std::cout << "🔹 3 parameters: ";
        auto result3 = demo.call_function("func3", {10, 3.14, true});

        // 4 参数
        std::cout << "🔹 4 parameters: ";
        demo.call_function("func4", {5, 2.5, std::string("test"), true});

        // 5 参数
        std::cout << "🔹 5 parameters: ";
        auto result5 = demo.call_function(
            "func5", {7, 1.5, std::string("complex"), false, 9.9f});
    } catch (const std::exception &e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
    }

    // ===== Node类演示 =====
    print_separator("Built-in Node Class Demo");

    node n(42);
    n.name = "demo_node";
    n.ratio = 1.5;
    n.active = true;

    n.print_reflection_info();

    std::cout << "\n--- Node Function Tests ---" << std::endl;
    try {
        n.call_function("process");

        auto node_info = n.call_function("get_info");
        std::cout << "Node info: " << std::any_cast<std::string>(node_info)
                  << std::endl;

        n.call_function("set_value", {200});

        auto calc_result = n.call_function("calculate", {2.0, 50.0});
        std::cout << "Calculation result: "
                  << std::any_cast<double>(calc_result) << std::endl;

        // 5参数复杂计算
        auto complex_result =
            n.call_function("complex_calc",
                            {
                                25,                    // int base
                                1.2,                   // double factor
                                std::string("result"), // string prefix
                                true,                  // bool round_result
                                10.0f                  // float precision
                            });
        std::cout << "Complex result: \""
                  << std::any_cast<std::string>(complex_result) << "\""
                  << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
    }

    // ===== 访问者模式演示 =====
    print_separator("Visitor Pattern Demo");

    std::cout << "Using visit_all_members to inspect person object:"
              << std::endl;
    p.visit_all_members(
        // 属性访问者
        [](const std::string &name, const std::any &value,
           std::string_view type) {
            std::cout << "  📋 [Property] " << name << " (" << type << ") = ";
            if (type == "int") {
                std::cout << std::any_cast<int>(value);
            } else if (type == "string") {
                std::cout << "\"" << std::any_cast<std::string>(value) << "\"";
            } else if (type == "double") {
                std::cout << std::any_cast<double>(value);
            } else if (type == "bool") {
                std::cout << (std::any_cast<bool>(value) ? "true" : "false");
            } else {
                std::cout << "unknown";
            }
            std::cout << std::endl;
        },
        // 函数访问者
        [](const std::string &name, std::string_view signature,
           size_t param_count, const std::vector<std::string> &param_types) {
            std::cout << "  [Function] " << name << " -> " << signature
                      << " (params: " << param_count << ")" << std::endl;
        });

    // ===== 总结 =====
    print_separator("Summary");

    std::cout << "✅ Property reflection: Dynamic get/set with type safety"
              << std::endl;
    std::cout << "✅ Function reflection: Support for 0-N parameters with "
                 "variadic templates"
              << std::endl;
    std::cout << "✅ Unified API: Single register_function for all "
                 "parameter counts"
              << std::endl;
    std::cout << "✅ Type safety: Compile-time type checking with runtime "
                 "conversion"
              << std::endl;
    std::cout << "✅ Visitor pattern: Traverse all members programmatically"
              << std::endl;
    std::cout << "✅ Batch registration: "
                 "REGISTER_MEMBERS/REGISTER_FUNCTIONS macros"
              << std::endl;

    std::cout << "\n🎉 Reflection system demo completed successfully!"
              << std::endl;

    return 0;
}
