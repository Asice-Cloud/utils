# C++ 可变参数模板反射系统

## 核心原理

- 利用 C++20/23 可变参数模板和 `std::index_sequence`，实现任意参数数量的成员函数注册与调用。
- 属性和函数均以字符串为 key 注册到基类的 map 中，支持运行时动态访问。
- 参数传递统一用 `std::any`，类型安全由模板和 `std::any_cast` 保证。
- 支持 const 引用参数时，需用 `std::cref(obj)` 包装，否则类型不匹配。

## 常见用法与注意事项

- **const 引用参数**：如成员函数参数为 `const T&`，调用时需用 `std::cref(obj)`，如 `call_function("foo", {std::cref(obj)})`。
- **不可拷贝类型**（如含 unique_ptr 的类）：只能用引用方式传递，不能直接传值。
- **类型安全**��`set_property`/`call_function` 参数类型需与注册类型严格匹配，否则抛出 `std::bad_any_cast`。

## 简明示例

```cpp
#include "reflect.h"
struct Point : reflected_object {
    int x, y;
    Point(int a, int b) : x(a), y(b) {
        REGISTER_MEMBERS(MEMBER(x), MEMBER(y));
        REGISTER_FUNCTIONS(FUNCTION(inner_product), FUNCTION(inner_with_other));
    }
    int inner_product(int a, int b) const { return x * a + y * b; }
    int inner_with_other(const Point& other) const { return x * other.x + y * other.y; }
};

int main() {
    Point p(1, 2), p2(7, 8);
    p.set_property("x", 10);
    std::cout << std::any_cast<int>(p.get_property("x").value()) << std::endl;
    std::cout << std::any_cast<int>(p.call_function("inner_product", {3, 4})) << std::endl;
    std::cout << std::any_cast<int>(p.call_function("inner_with_other", {std::cref(p2)})) << std::endl;
}
```

## ✨ 主要特性

### 🚀 核心功能

- **属性反射**: 运行时查询、修改对象属性，完整类型安全
- **可变参数函数反射**: **单一模板**支持0到∞个参数的函数调用  
- **统一注册接口**: 无需为不同参数数量编写不同的注册代码
- **访问者模式**: 遍历所有属性和函数，支持元编程
- **类型安全**: 编译时类型检查，运行时自动类型转换
- **简洁API**: 批量注册宏，一行代码注册所有成员

### 🔥 技术亮点

- **可变参数模板**: 使用 `template<typename... Args>` 替代多个重载
- **完美转发**: `std::index_sequence` 实现参数包展开
- **类型推导**: `std::source_location` 自动提取类型信息
- **零开销抽象**: 编译时优化，运行时高效

## 🚀 快速开始

```cpp
#include "reflect.h"

class Demo : public reflected_object {
public:
    std::string name;
    int value;
    
    Demo(const std::string& n, int v) : name(n), value(v) {
        // 批量注册 - 超简洁
        REGISTER_MEMBERS(MEMBER(name), MEMBER(value));  
        REGISTER_FUNCTIONS(FUNCTION(func0), FUNCTION(func1), FUNCTION(func5));
    }
    
    // 可变参数模板自动支持所有这些函数！
    void func0() { }                                           // 0 参数
    int func1(int x) { return x * 2; }                        // 1 参数  
    std::string func5(int a, double b, const std::string& c,  // 5 参数
                     bool d, float e) { return "result"; }
};

int main() {
    Demo obj("test", 42);
    
    // 属性反射
    obj.set_property("name", std::string("updated"));
    auto name = obj.get_property("name");
    
    // 函数反射 - 同一个API支持不同参数数量！
    obj.call_function("func0");                               // 0 参数
    auto r1 = obj.call_function("func1", {100});             // 1 参数  
    auto r5 = obj.call_function("func5", {1, 2.0, std::string("hi"), true, 3.14f}); // 5 参数
    
    return 0;
}
```

## 📊 性能对比

| 传统方案 | 本系统 |
|---------|--------|
| 为每个参数数量写一个类 | **单一可变参数模板** |
| 多个 register_function 重载 | **统一注册接口** |
| 代码重复，维护困难 | **DRY原则，易维护** |
| 限制参数数量 | **支持任意参数数量** |

## 🛠 API 参考

### 注册宏

#### 属性注册

```cpp
// 单个注册
REGISTER_MEMBER(member_name)

// 批量注册
REGISTER_MEMBERS(
    MEMBER(member1),
    MEMBER(member2),
    MEMBER(member3)
)
```

#### 函数注册

```cpp
// 单个注册
REGISTER_FUNCTION(function_name)

// 批量注册
REGISTER_FUNCTIONS(
    FUNCTION(func1),
    FUNCTION(func2),
    FUNCTION(func3)
)
```

### 属性反射 API

#### 属性访问

```cpp
// 获取属性值
std::optional<std::any> get_property(const std::string& name) const

// 设置属性值
bool set_property(const std::string& name, const std::any& value)

// 获取属性类型
std::string_view get_property_type(const std::string& name) const

// 检查属性是否存在
bool has_property(const std::string& name) const
```

#### 属性查询

```cpp
// 获取所有属性名
std::vector<std::string> get_property_names() const

// 获取所有属性键值对
std::unordered_map<std::string, std::any> get_all_properties() const

// 获取属性数量
size_t property_count() const
```

#### 批量属性操作

```cpp
// 使用 map 批量设置
void set_properties(const std::unordered_map<std::string, std::any>& props)

// 使用 vector 批量设置
void set_properties(const std::vector<std::pair<std::string, std::any>>& props)

// 使用变参模板批量设置（名称-值对）
template<typename... Args>
void set_properties_variadic(Args&&... args)
```

### 函数反射 API

#### 函数调用

```cpp
// 调用函数（支持任意数量参数和返回值）
std::any call_function(const std::string& name, const std::vector<std::any>& args = {})

// 示例 - 支持不同参数数量
obj.call_function("func0");                                    // 0 参数
obj.call_function("func1", {42});                              // 1 参数  
obj.call_function("func2", {123, std::string("hello")});       // 2 参数
obj.call_function("func3", {10, 3.14, true});                  // 3 参数
obj.call_function("func4", {1, 2.0, std::string("test"), false}); // 4 参数
// ... 支持任意数量参数
```

#### 可变参数模板支持

本系统使用**单一的可变参数模板**实现函数反射，自动支持任意数量的参数：

```cpp
// 内部实现 - 统一的注册接口
template<typename Class, typename ReturnType, typename... Args>
void register_function(const std::string& name, ReturnType (Class::*func_ptr)(Args...))

// 支持的函数签名示例：
void func0()                                    // 0 参数
int func1(int x)                               // 1 参数
std::string func2(int x, const std::string& s) // 2 参数
double func3(int a, double b, bool c)          // 3 参数
void func4(int a, double b, const std::string& c, bool d) // 4 参数
// ... 等等，无限制
```

#### 函数查询

```cpp
// 获取所有函数名
std::vector<std::string> get_function_names() const

// 获取函数签名
std::string_view get_function_signature(const std::string& name) const

// 获取函数参数数量
size_t get_function_param_count(const std::string& name) const

// 获取函数参数类型列表
std::vector<std::string> get_function_param_types(const std::string& name) const

// 检查函数是否存在
bool has_function(const std::string& name) const
```

### 访问者模式 API

#### 访问属性

```cpp
template<typename Visitor>
void visit_members(Visitor&& visitor) const

// visitor 函数签名: void(const std::string& name, const std::any& value, std::string_view type)
```

#### 访问所有成员（属性和函数）

```cpp
template<typename PropertyVisitor, typename FunctionVisitor>
void visit_all_members(PropertyVisitor&& prop_visitor, FunctionVisitor&& func_visitor) const

// prop_visitor 函数签名: void(const std::string& name, const std::any& value, std::string_view type)
// func_visitor 函数签名: void(const std::string& name, std::string_view signature, size_t param_count, const std::vector<std::string>& param_types)
```

## 🔧 技术实现

### 可变参数模板核心

```cpp
// 统一的成员函数实现 - 支持任意参数数量
template <typename Class, typename ReturnType, typename... Args>
class member_function : public function_base {
private:
    ReturnType (Class::*func_ptr_)(Args...);
    
    // 使用 index_sequence 展开参数包
    template <std::size_t... I>
    std::any invoke_impl(Class *obj, const std::vector<std::any> &args, 
                        std::index_sequence<I...>) {
        if constexpr (std::is_void_v<ReturnType>) {
            (obj->*func_ptr_)(std::any_cast<Args>(args[I])...);
            return std::any{};
        } else {
            return std::any((obj->*func_ptr_)(std::any_cast<Args>(args[I])...));
        }
    }
    
public:
    // 统一注册接口
    std::any invoke(void *obj, const std::vector<std::any> &args) override {
        Class *class_obj = static_cast<Class *>(obj);
        return invoke_impl(class_obj, args, std::index_sequence_for<Args...>{});
    }
    
    size_t get_param_count() const override { return sizeof...(Args); }
};

// 统一的函数注册方法 - 替代多个重载
template <typename Class, typename ReturnType, typename... Args>
void register_function(const std::string &name, ReturnType (Class::*func_ptr)(Args...)) {
    functions_[name] = std::make_unique<member_function<Class, ReturnType, Args...>>(name, func_ptr);
}
```

### 优势分析

| 特性 | 传统实现 | 可变参数模板实现 |
|------|----------|------------------|
| **代码量** | N个类 × 平均50行 = 大量重复 | 1个模板类 ≈ 80行 |
| **维护性** | 每增加参数数量需新增类 | 自动支持任意参数数量 |
| **编译时间** | 多个类实例化 | 单一模板，更快编译 |
| **类型安全** | 每个类单独实现 | 统一模板，类型推导 |
| **扩展性** | 有限制（预定义数量） | 无限制（编译器限制内） |

### 工具函数

```cpp
// 打印所有反射信息（调试用）
void print_reflection_info() const
```

## 使用示例

### 属性操作示例

```cpp
Person p("Alice", 25);

// 单个属性操作
p.set_property("name", std::string("Bob"));
auto age = p.get_property("age");
std::cout << "Age: " << std::any_cast<int>(*age) << std::endl;

// 批量属性操作
p.set_properties({
    {"name", std::string("Charlie")},
    {"age", 30},
    {"employed", true}
});

// 检查属性
if (p.has_property("salary")) {
    auto salary = p.get_property("salary");
}

// 获取所有属性名
auto prop_names = p.get_property_names();
for (const auto& name : prop_names) {
    std::cout << "Property: " << name << std::endl;
}
```

### 函数调用示例

```cpp
Person p("Alice", 25);

// 无参数函数
p.call_function("introduce");

// 有参数函数
p.call_function("set_age", {30});

// 有返回值的函数
auto info = p.call_function("get_info");
std::cout << std::any_cast<std::string>(info) << std::endl;

// 查询函数信息
if (p.has_function("calculate")) {
    auto signature = p.get_function_signature("calculate");
    auto param_count = p.get_function_param_count("calculate");
    auto param_types = p.get_function_param_types("calculate");
  
    std::cout << "Function: " << signature << std::endl;
    std::cout << "Parameters: " << param_count << std::endl;
}
```

### 访问者模式示例

```cpp
Person p("Alice", 25);

// 访问所有属性
p.visit_members([](const std::string& name, const std::any& value, std::string_view type) {
    std::cout << name << " (" << type << "): ";
    if (value.type() == typeid(int)) {
        std::cout << std::any_cast<int>(value);
    } else if (value.type() == typeid(std::string)) {
        std::cout << "\"" << std::any_cast<std::string>(value) << "\"";
    }
    std::cout << std::endl;
});

// 访问所有成员（属性和函数）
p.visit_all_members(
    // 属性访问者
    [](const std::string& name, const std::any& value, std::string_view type) {
        std::cout << "[Property] " << name << " (" << type << ")" << std::endl;
    },
    // 函数访问者
    [](const std::string& name, std::string_view signature, size_t param_count, 
       const std::vector<std::string>& param_types) {
        std::cout << "[Function] " << name << " -> " << signature 
                  << " (params: " << param_count << ")" << std::endl;
    }
);
```

## 📋 完整示例

查看 `main.cpp` 了解完整的演示，包括：

- ✅ 基础属性反射（get/set）
- ✅ 0-5参数函数调用演示  
- ✅ 内置node类测试
- ✅ 访问者模式使用
- ✅ 错误处理示例

## 🚀 编译和运行

```bash
# 方法1: 直接编译
g++ -std=c++20 -Wall -Wextra -O2 main.cpp -o demo
./demo

# 方法2: 使用CMake
cmake -B build
cmake --build build
./build/reflect
```

### 系统要求

- **C++20/23**: 支持 `std::source_location`, `if constexpr`, 折叠表达式
- **编译器**: GCC 10+ 或 Clang++ 13+
- **CMake**: 3.20+ (可选)

## 🎯 输出示例

```
🚀 C++ Variadic Template Reflection System Demo 🚀

=== Variadic Template Functions (0-5 Parameters) ===
🔹 0 parameters: func0() - no parameters
🔹 1 parameter: func1(42) - returns 84  
🔹 2 parameters: func2(123, "hello") - returns "hello_123"
🔹 3 parameters: func3(10, 3.14, true) - returns 13.14
🔹 4 parameters: func4(5, 2.5, "test", true) - processed: test_7.5
🔹 5 parameters: func5(5 params) - returns "complex_result"

=== Summary ===
✅ Property reflection: Dynamic get/set with type safety
✅ Function reflection: Support for 0-N parameters with variadic templates  
✅ Unified API: Single register_function for all parameter counts
✅ Type safety: Compile-time type checking with runtime conversion
🎉 Reflection system demo completed successfully!
```

## 🏆 总结

这个反射系统展示了现代C++模板元编程的威力：

- **单一可变参数模板** 替代多个重复的类定义
- **统一API** 简化用户接口，提高开发效率  
- **类型安全** 编译时检查+运行时转换，最佳实践
- **零运行时开销** 模板在编译时完全展开
- **无限扩展性** 理论上支持任意数量参数（受编译器限制）

通过可变参数模板，我们用**80行代码**实现了原本需要**数百行重复代码**的功能！

=== Function Reflection ===
Hi, I'm Bob, 30 years old
Age updated: 30 -> 35

=== Visitor Pattern ===
  [Property] name (string) = "Bob"
  [Property] age (int) = 35
  [Property] employed (bool) = true
  [Function] introduce -> introduce() -> void (params: 0)
  [Function] set_age -> set_age(int) -> void (params: 1)
```

## 技术实现

- **C++23 source_location**: 编译时类型推导
- **std::any**: 类型擦除和动态存储
- **模板元编程**: 灵活的函数参数处理
- **RTTI**: 运行时类型信息
- **访问者模式**: 统一的成员遍历接口

这个反射系统提供了完整的运行时内省能力，适用于序列化、配置系统、调试工具等场景。
