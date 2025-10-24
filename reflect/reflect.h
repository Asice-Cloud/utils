#pragma once

#include <source_location>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <any>
#include <optional>
#include <functional>
#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <cmath>

#define Nothing_TODO_With_Member  [](const std::string &name, const std::any &value, std::string_view type){}
#define Nothing_TODO_With_Function  [](const std::string&, std::string_view, size_t, const std::vector<std::string>&) {}

// 成员变量信息结构体
template <typename T>
struct member_info
{
    const char *name;
    T *ptr;

    member_info(const char *n, T *p) : name(n), ptr(p) {}
};

// 成员函数信息结构体
template <typename FuncPtr>
struct function_info
{
    const char *name;
    FuncPtr ptr;

    function_info(const char *n, FuncPtr p) : name(n), ptr(p) {}
};

// 辅助宏用于创建成员信息
#define MEMBER(member) member_info(#member, &member)
#define FUNCTION(func) function_info(#func, &std::remove_reference_t<decltype(*this)>::func)

// 函数名解析器
class function_name_parser
{
public:
    static std::string extract_type_name(const std::source_location &loc)
    {
        std::string_view func_name = loc.function_name();
        size_t eq_pos = func_name.find("T = ");
        if (eq_pos != std::string_view::npos)
        {
            size_t start = eq_pos + 4;
            size_t end = func_name.find_first_of("];,)", start);
            if (end != std::string_view::npos)
            {
                std::string type_name = std::string(func_name.substr(start, end - start));
                return clean_type_name(type_name);
            }
        }
        return "unknown";
    }

    static std::string clean_type_name(const std::string &type_name)
    {
        std::string clean_name = type_name;
        if (clean_name.find("basic_string") != std::string::npos)
        {
            return "string";
        }
        if (clean_name == "int" || clean_name == "i")
            return "int";
        if (clean_name == "double" || clean_name == "d")
            return "double";
        if (clean_name == "float" || clean_name == "f")
            return "float";
        if (clean_name == "bool" || clean_name == "b")
            return "bool";
        if (clean_name == "char" || clean_name == "c")
            return "char";
        if (clean_name == "void" || clean_name == "v")
            return "void";
        return clean_name;
    }
};

// 属性基类
class property_base
{
public:
    virtual ~property_base() = default;
    virtual std::any get_value() const = 0;
    virtual void set_value(const std::any &value) = 0;
    virtual std::string_view get_type_name() const = 0;
    virtual size_t get_type_hash() const = 0;
};

// 函数基类
class function_base
{
public:
    virtual ~function_base() = default;
    virtual std::string_view get_name() const = 0;
    virtual std::string_view get_signature() const = 0;
    virtual std::any invoke(void *obj, const std::vector<std::any> &args) = 0;
    virtual size_t get_param_count() const = 0;
    virtual std::vector<std::string> get_param_types() const = 0;
};

// 具体属性实现
template <typename T>
class property : public property_base
{
private:
    T *ptr_;
    std::string type_name_;

public:
    property(T *ptr, const std::source_location &loc = std::source_location::current())
        : ptr_(ptr)
    {
        type_name_ = function_name_parser::extract_type_name(loc);
        if (type_name_ == "unknown")
        {
            type_name_ = function_name_parser::clean_type_name(typeid(T).name());
        }
    }

    std::any get_value() const override
    {
        return std::any(*ptr_);
    }

    void set_value(const std::any &value) override
    {
        *ptr_ = std::any_cast<T>(value); // 让异常传播到上层
    }

    std::string_view get_type_name() const override
    {
        return type_name_;
    }

    size_t get_type_hash() const override
    {
        return typeid(T).hash_code();
    }
};

// 成员函数实现 - 可变参数版本
template <typename Class, typename ReturnType, typename... Args>
class member_function : public function_base
{
private:
    std::string name_;
    std::string signature_;
    ReturnType (Class::*func_ptr_)(Args...);

    // 辅助函数：将参数包转换为类型名称
    std::string build_signature() const
    {
        std::string sig = name_ + "(";
        if constexpr (sizeof...(Args) > 0)
        {
            auto type_names = std::vector<std::string>{
                function_name_parser::clean_type_name(typeid(Args).name())...};
            for (size_t i = 0; i < type_names.size(); ++i)
            {
                if (i > 0)
                    sig += ", ";
                sig += type_names[i];
            }
        }
        sig += ") -> " + function_name_parser::clean_type_name(typeid(ReturnType).name());
        return sig;
    }

    // 辅助函数：从 std::any 提取参数，支持引用和 const 引用
    template<typename T>
    static T extract_arg(const std::any& a) {
        using base_t = std::remove_reference_t<T>;
        if constexpr (std::is_lvalue_reference_v<T>) {
            if (a.type() == typeid(std::reference_wrapper<base_t>))
                return static_cast<T>(std::any_cast<std::reference_wrapper<base_t>>(a).get());
            if (a.type() == typeid(base_t))
                return static_cast<T>(std::any_cast<base_t&>(const_cast<std::any&>(a)));
            return std::any_cast<T>(a);
        } else {
            return std::any_cast<T>(a);
        }
    }

    // 辅助函数：从 std::any 参数中提取参数并调用函数
    template <std::size_t... I>
    std::any invoke_impl(Class *obj, const std::vector<std::any> &args, std::index_sequence<I...>)
    {
        if constexpr (std::is_void_v<ReturnType>)
        {
            (obj->*func_ptr_)(extract_arg<Args>(args[I])...);
            return std::any{};
        }
        else
        {
            return std::any((obj->*func_ptr_)(extract_arg<Args>(args[I])...));
        }
    }

public:
    member_function(const std::string &name, ReturnType (Class::*func_ptr)(Args...))
        : name_(name), func_ptr_(func_ptr)
    {
        signature_ = build_signature();
    }

    std::string_view get_name() const override { return name_; }
    std::string_view get_signature() const override { return signature_; }
    size_t get_param_count() const override { return sizeof...(Args); }

    std::vector<std::string> get_param_types() const override
    {
        if constexpr (sizeof...(Args) == 0)
        {
            return {};
        }
        else
        {
            return {function_name_parser::clean_type_name(typeid(Args).name())...};
        }
    }

    std::any invoke(void *obj, const std::vector<std::any> &args) override
    {
        if (args.size() != sizeof...(Args))
        {
            throw std::invalid_argument("Function expects " + std::to_string(sizeof...(Args)) +
                                        " arguments, got " + std::to_string(args.size()));
        }

        Class *class_obj = static_cast<Class *>(obj);
        return invoke_impl(class_obj, args, std::index_sequence_for<Args...>{});
    }
};

// const member function
template<typename Class, typename ReturnType, typename... Args>
class const_member_function : public function_base {
private:
    std::string name_;
    std::string signature_;
    ReturnType (Class::*func_ptr_)(Args...) const;

    std::string build_signature() const {
        std::string sig = name_ + "(";
        if constexpr (sizeof...(Args) > 0) {
            auto type_names = std::vector<std::string>{
                function_name_parser::clean_type_name(typeid(Args).name())...};
            for (size_t i = 0; i < type_names.size(); ++i) {
                if (i > 0)
                    sig += ", ";
                sig += type_names[i];
            }
        }
        sig += ") -> " + function_name_parser::clean_type_name(typeid(ReturnType).name());
        return sig;
    }

    // 辅助函数：从 std::any 提取参数，支持引用和 const 引用
    template<typename T>
    static T extract_arg(const std::any& a) {
        using base_t = std::remove_reference_t<T>;
        if constexpr (std::is_lvalue_reference_v<T>) {
            if (a.type() == typeid(std::reference_wrapper<base_t>))
                return static_cast<T>(std::any_cast<std::reference_wrapper<base_t>>(a).get());
            if (a.type() == typeid(base_t))
                return static_cast<T>(std::any_cast<base_t&>(const_cast<std::any&>(a)));
            return std::any_cast<T>(a);
        } else {
            return std::any_cast<T>(a);
        }
    }

    template<std::size_t... I>
    std::any invoke_impl(const Class* obj, const std::vector<std::any>& args, std::index_sequence<I...>) {
        if constexpr (std::is_void_v<ReturnType>) {
            (obj->*func_ptr_)(extract_arg<Args>(args[I])...);
            return std::any{};
        } else {
            return std::any((obj->*func_ptr_)(extract_arg<Args>(args[I])...));
        }
    }
public:
    const_member_function(const std::string &name, ReturnType (Class::*func_ptr)(Args...) const)
        : name_(name), func_ptr_(func_ptr) {
        signature_ = build_signature();
    }


    std::string_view get_name() const override { return name_; }
    std::string_view get_signature() const override { return signature_; }
    size_t get_param_count() const override { return sizeof...(Args); }

    std::vector<std::string> get_param_types() const override {
        if constexpr (sizeof...(Args) == 0) {
            return {};
        } else {
            return {function_name_parser::clean_type_name(typeid(Args).name())...};
        }
    }

    std::any invoke(void* obj, const std::vector<std::any>& args) override {
        if (args.size() != sizeof...(Args)) {
            throw std::invalid_argument("Function expects " + std::to_string(sizeof...(Args)) +
                                        " arguments, got " + std::to_string(args.size()));
        }

        const Class* class_obj = static_cast<const Class*>(obj);
        return invoke_impl(class_obj, args, std::index_sequence_for<Args...>{});
    }
};

// 反射对象基类
class reflected_object
{
private:
    std::unordered_map<std::string, std::unique_ptr<property_base>> properties_;
    std::unordered_map<std::string, std::unique_ptr<function_base>> functions_;

    // 递归辅助函数用于变参模板
    template <typename T, typename... Rest>
    void set_properties_impl(const std::string &name, T &&value, Rest &&...rest)
    {
        set_property(name, std::forward<T>(value));
        if constexpr (sizeof...(rest) > 0)
        {
            set_properties_impl(std::forward<Rest>(rest)...);
        }
    }

protected:
    template <typename T>
    void register_member(const std::string &name, T *ptr)
    {
        properties_[name] = std::make_unique<property<T>>(ptr);
    }

    // 注册成员函数 - 可变参数版本
    template <typename Class, typename ReturnType, typename... Args>
    void register_function(const std::string &name, ReturnType (Class::*func_ptr)(Args...))
    {
        functions_[name] = std::make_unique<member_function<Class, ReturnType, Args...>>(name, func_ptr);
    }

    //overload
    template<typename Class, typename ReturnType, typename... Args>
    void register_function(const std::string &name, ReturnType (Class::*func_ptr)(Args...) const) {
        functions_[name] = std::make_unique<const_member_function<Class, ReturnType, Args...>>(name, func_ptr);
    }

    // 批量注册成员变量
    template <typename... Members>
    void register_all_members(Members... members)
    {
        // 使用fold expression (C++17)
        (register_member_helper(members), ...);
    }

    // 批量注册成员函数
    template <typename... Functions>
    void register_all_functions(Functions... functions)
    {
        // 使用fold expression (C++17)
        (register_function_helper(functions), ...);
    }

private:
    // 成员变量注册辅助函数
    template <typename T>
    void register_member_helper(T &&member_info)
    {
        register_member(member_info.name, member_info.ptr);
    }

    // 成员函数注册辅助函数
    template <typename T>
    void register_function_helper(T &&func_info)
    {
        register_function(func_info.name, func_info.ptr);
    }

public:
    // 获取属性值
    std::optional<std::any> get_property(const std::string &name) const
    {
        auto it = properties_.find(name);
        if (it != properties_.end())
        {
            return it->second->get_value();
        }
        return std::nullopt;
    }

    // 设置属性值
    bool set_property(const std::string &name, const std::any &value)
    {
        auto it = properties_.find(name);
        if (it != properties_.end())
        {
            try
            {
                it->second->set_value(value);
                return true;
            }
            catch (const std::bad_any_cast &)
            {
                // 类型转换失败
                std::cout << "Type conversion failed: bad any_cast" << std::endl;
                return false;
            }
        }
        return false;
    }

    // 获���所有属性名
    std::vector<std::string> get_property_names() const
    {
        std::vector<std::string> names;
        for (const auto &[name, _] : properties_)
        {
            names.push_back(name);
        }
        return names;
    }

    // 获取属性类型信息
    std::string_view get_property_type(const std::string &name) const
    {
        auto it = properties_.find(name);
        if (it != properties_.end())
        {
            return it->second->get_type_name();
        }
        return "unknown";
    }

    // 访问所有成员 - 使用函数对象访问每个属性
    template <typename Visitor>
    void visit_members(Visitor &&visitor) const
    {
        for (const auto &[name, property] : properties_)
        {
            visitor(name, property->get_value(), property->get_type_name());
        }
    }

    // 访问所有成员（属性和函数） - 使用函数对象访问每个成员
    template <typename PropertyVisitor, typename FunctionVisitor>
    void visit_all_members(PropertyVisitor &&prop_visitor, FunctionVisitor &&func_visitor) const
    {
        // 访问属性
        for (const auto &[name, property] : properties_)
        {
            prop_visitor(name, property->get_value(), property->get_type_name());
        }

        // 访问函数
        for (const auto &[name, function] : functions_)
        {
            func_visitor(name, function->get_signature(), function->get_param_count(), function->get_param_types());
        }
    } // 批量设置属性 - 使用变参模板（名称-值对）
    template <typename... Args>
    void set_properties_variadic(Args &&...args)
    {
        static_assert(sizeof...(args) % 2 == 0, "Arguments must come in name-value pairs");
        set_properties_impl(std::forward<Args>(args)...);
    }

    // 批量设置属性 - 使用 map
    void set_properties(const std::unordered_map<std::string, std::any> &props)
    {
        for (const auto &[name, value] : props)
        {
            set_property(name, value);
        }
    }

    // 批量设置属性 - 使用 vector of pairs
    void set_properties(const std::vector<std::pair<std::string, std::any>> &props)
    {
        for (const auto &[name, value] : props)
        {
            set_property(name, value);
        }
    }

    // 获取所有属性的键值对
    std::unordered_map<std::string, std::any> get_all_properties() const
    {
        std::unordered_map<std::string, std::any> result;
        for (const auto &[name, property] : properties_)
        {
            result[name] = property->get_value();
        }
        return result;
    }

    // 获取属性数量
    size_t property_count() const
    {
        return properties_.size();
    }

    // 检查是否存在某个属性
    bool has_property(const std::string &name) const
    {
        return properties_.find(name) != properties_.end();
    }

    // 调用成员函数
    std::any call_function(const std::string &name, const std::vector<std::any> &args = {})
    {
        auto it = functions_.find(name);
        if (it != functions_.end())
        {
            try
            {
                return it->second->invoke(this, args);
            }
            catch (const std::exception &e)
            {
                std::cout << "Function call failed: " << e.what() << std::endl;
                throw;
            }
        }
        throw std::runtime_error("Function '" + name + "' not found");
    }

    // 获取所有函数名
    std::vector<std::string> get_function_names() const
    {
        std::vector<std::string> names;
        for (const auto &[name, _] : functions_)
        {
            names.push_back(name);
        }
        return names;
    }

    // 获取函数签名
    std::string_view get_function_signature(const std::string &name) const
    {
        auto it = functions_.find(name);
        if (it != functions_.end())
        {
            return it->second->get_signature();
        }
        return "unknown";
    }

    // 检查是否存在某个函数
    bool has_function(const std::string &name) const
    {
        return functions_.find(name) != functions_.end();
    }

    // 获取函数参数数量
    size_t get_function_param_count(const std::string &name) const
    {
        auto it = functions_.find(name);
        if (it != functions_.end())
        {
            return it->second->get_param_count();
        }
        return 0;
    }

    // 获取函数参数类型
    std::vector<std::string> get_function_param_types(const std::string &name) const
    {
        auto it = functions_.find(name);
        if (it != functions_.end())
        {
            return it->second->get_param_types();
        }
        return {};
    }

    // 打印所有反射信息
    void print_reflection_info() const
    {
        std::cout << "=== Reflection Info ===\n";

        // 打印属性信息
        std::cout << "Properties:\n";
        for (const auto &prop_name : get_property_names())
        {
            auto value_opt = get_property(prop_name);
            auto type_name = get_property_type(prop_name);

            std::cout << "  " << prop_name << " (" << type_name << "): ";

            if (value_opt)
            {
                const auto &value = *value_opt;
                if (value.type() == typeid(int))
                {
                    std::cout << std::any_cast<int>(value);
                }
                else if (value.type() == typeid(std::string))
                {
                    std::cout << "\"" << std::any_cast<std::string>(value) << "\"";
                }
                else if (value.type() == typeid(double))
                {
                    std::cout << std::any_cast<double>(value);
                }
                else if (value.type() == typeid(float))
                {
                    std::cout << std::any_cast<float>(value);
                }
                else if (value.type() == typeid(bool))
                {
                    std::cout << (std::any_cast<bool>(value) ? "true" : "false");
                }
                else
                {
                    std::cout << "unknown type";
                }
            }
            else
            {
                std::cout << "null";
            }
            std::cout << "\n";
        }

        // 打印函数信息
        auto function_names = get_function_names();
        if (!function_names.empty())
        {
            std::cout << "Functions:\n";
            for (const auto &func_name : function_names)
            {
                std::cout << "  " << get_function_signature(func_name) << "\n";
            }
        }
    }
};

// 类型名获取模板
template <typename T>
std::string get_type_name(const std::source_location &loc = std::source_location::current())
{
    return function_name_parser::extract_type_name(loc);
}

// 源码位置打印函数
inline void print_source_location(const std::source_location &loc = std::source_location::current())
{
    std::cout << "=== Source Location ===\n";
    std::cout << "File: " << loc.file_name() << "\n";
    std::cout << "Function: " << loc.function_name() << "\n";
    std::cout << "Line: " << loc.line() << "\n";
    std::cout << "Column: " << loc.column() << "\n";
}

// 反射宏
#define REGISTER_MEMBER(member) register_member(#member, &member)
#define REGISTER_FUNCTION(func) register_function(#func, &std::remove_reference_t<decltype(*this)>::func)

// 简化的批量注册宏
#define REGISTER_MEMBERS(...)              \
    do                                     \
    {                                      \
        register_all_members(__VA_ARGS__); \
    } while (0)

#define REGISTER_FUNCTIONS(...)              \
    do                                       \
    {                                        \
        register_all_functions(__VA_ARGS__); \
    } while (0)

// 节点类 - 演示反射功能
class node : public reflected_object
{
public:
    int value;
    std::string name;
    double ratio;
    bool active;

    node(int v = 0) : value(v), name("default"), ratio(1.0), active(true)
    {
        // 注册属性
        REGISTER_MEMBER(value);
        REGISTER_MEMBER(name);
        REGISTER_MEMBER(ratio);
        REGISTER_MEMBER(active);

        // 注册函数 - 展示可变参数支持
        REGISTER_FUNCTION(process);      // 0 参数
        REGISTER_FUNCTION(get_info);     // 0 参数
        REGISTER_FUNCTION(set_value);    // 1 参数
        REGISTER_FUNCTION(calculate);    // 2 参数
        REGISTER_FUNCTION(complex_calc); // 5 参数
    }

    void print_source_location(const std::source_location &loc = std::source_location::current())
    {
        ::print_source_location(loc);
    }

    void process()
    {
        std::cout << "Processing node: " << name << " with value: " << value << std::endl;
    }

    std::string get_info()
    {
        return "Node '" + name + "' has value " + std::to_string(value) +
               ", ratio " + std::to_string(ratio) +
               ", active: " + (active ? "true" : "false");
    }

    void set_value(int new_value)
    {
        std::cout << "Setting value from " << value << " to " << new_value << std::endl;
        value = new_value;
    }

    double calculate(double multiplier, double offset)
    {
        double result = value * multiplier * ratio + offset;
        std::cout << "Calculation: " << value << " * " << multiplier << " * " << ratio
                  << " + " << offset << " = " << result << std::endl;
        return result;
    }

    // 演示5参数函数 - 展示可变参数模板的威力
    std::string complex_calc(int base, double factor, const std::string &prefix, bool round_result, float precision)
    {
        double result = (value + base) * factor * ratio;
        if (round_result)
        {
            result = std::round(result * precision) / precision;
        }

        std::string output = prefix + "_" + std::to_string(result);
        std::cout << "Complex calculation with 5 parameters: " << output << std::endl;
        return output;
    }
};
