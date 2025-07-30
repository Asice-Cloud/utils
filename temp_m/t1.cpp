// Chapter 1

#include <string>
#include <iostream>
#include <type_traits>
#include <vector>
#include <list>

template <typename T>
constexpr T NewLine = T('\n');

template <typename T>
void print_type(T value)
{
    static_assert(std::is_same_v<T, decltype(value)>);
    std::cout << "Type: " << typeid(T).name() << ", Value: " << value << std::e ndl;
}

// 模板模板参数示例 1：容器适配器
template <template <typename, typename> class Container>
class ContainerAdapter
{
public:
    template <typename T>
    using container_type = Container<T, std::allocator<T>>;

    template <typename T>
    void demonstrate()
    {
        container_type<T> container;
        std::cout << "Using container: " << typeid(container).name() << std::endl;
    }
};

// 模板模板参数示例 2：策略模式
template <template <typename> class Policy>
class ConfigurableClass
{
public:
    template <typename T>
    void apply_policy(T value)
    {
        Policy<T> policy;
        policy.execute(value);
    }
};

// 策略类示例
template <typename T>
class PrintPolicy
{
public:
    void execute(T value)
    {
        std::cout << "Printing: " << value << std::endl;
    }
};

template <typename T>
class DoublePolicy
{
public:
    void execute(T value)
    {
        std::cout << "Doubling: " << value * 2 << std::endl;
    }
};

int main()
{
    print_type<int>(42);

    // 原有的代码
    constexpr auto lineChar = NewLine<char>;
    constexpr auto lineWChar = NewLine<wchar_t>;

    static_assert(lineChar == '\n', "lineChar should be newline character");
    static_assert(lineWChar == L'\n', "lineWChar should be wide newline character");

    std::wstring wstr = L"Hello, World!";
    wstr += lineWChar;
    std::wcout << wstr << std::endl;

    // 模板模板参数演示
    std::cout << "\n=== Template Template Parameters Demo ===\n";

    // 使用不同的容器类型
    ContainerAdapter<std::vector> vectorAdapter;
    ContainerAdapter<std::list> listAdapter;

    vectorAdapter.demonstrate<int>();
    listAdapter.demonstrate<double>();

    // 使用不同的策略
    ConfigurableClass<PrintPolicy> printer;
    ConfigurableClass<DoublePolicy> doubler;

    printer.apply_policy(10);
    doubler.apply_policy(15);

    return 0;
}