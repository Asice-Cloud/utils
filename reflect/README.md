# C++ 反射系统

基于 C++23 `std::source_location` 的现代反射系统，支持属性和函数的运行时访问。

## 特性

### 🚀 核心功能

- **属性反射**: 运行时查询、修改对象属性
- **函数反射**: 动态调用成员函数，支持参数和返回值
- **访问者模式**: 遍历所有属性和函数
- **类型安全**: 编译时类型检查，运行时类型转换
- **简洁API**: 批量注册，一行代码注册所有成员

## 快速开始

```cpp
#include "reflect.h"

class Person : public reflected_object {
public:
    std::string name;
    int age;
    bool employed;
  
    Person(const std::string& n, int a) : name(n), age(a), employed(false) {
        // 批量注册
        REGISTER_MEMBERS(MEMBER(name), MEMBER(age), MEMBER(employed));
        REGISTER_FUNCTIONS(FUNCTION(introduce), FUNCTION(set_age));
    }
  
    void introduce() {
        std::cout << "Hi, I'm " << name << ", " << age << " years old\n";
    }
  
    void set_age(int new_age) { age = new_age; }
};

int main() {
    Person p("Alice", 25);
  
    // 属性反射
    p.set_property("employed", true);
    auto name = p.get_property("name");
  
    // 函数反射
    p.call_function("introduce");
    p.call_function("set_age", {30});
  
    return 0;
}
```

## API 参考

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
// 调用函数（支持参数和返回值）
std::any call_function(const std::string& name, const std::vector<std::any>& args = {})
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

## 支持的类型

### 基本类型

- `int`, `double`, `float`, `bool`
- `std::string`（显示为 `string`）

### 函数类型

- **无参数函数**: `void func()`
- **单参数函数**: `ReturnType func(ParamType)`
- **双参数函数**: `ReturnType func(Param1, Param2)`
- **返回值类型**: `void` 和各种基本类型

## 编译和运行

```bash
# 编译
cmake -B build
cmake --build build

# 运行演示
./build/reflect
```

### 要求

- **C++23** 支持 `std::source_location`
- **CMake** 3.31+
- **编译器**: Clang++ 或 GCC

## 输出示例

```
=== Property Reflection ===
Properties:
  name (string): "Bob"
  age (int): 30
  employed (bool): true

Functions:
  introduce() -> void
  set_age(int) -> void

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
