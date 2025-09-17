// Chapter 2

#include <iostream>
#include <type_traits>
#include <memory>

template <typename T>
class wrapper
{
public:
    wrapper(T t) : value(t) {}
    T const &get() const { return value; }

    template <typename U>
    U as() const
    {
        return static_cast<U>(value);
    }

private:
    T value;
};

template <typename T, T... Args>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
struct value_sequence
{
    static constexpr size_t size = sizeof...(Args);
    using value_type = T;
    static void print()
    {
        std::cout << "Values: ";
        ((std::cout << Args << " "), ...);
        std::cout << std::endl;
    }

    static constexpr T sum()
        requires(std::is_arithmetic_v<T>)
    {
        return (Args + ...);
    }
};

struct base
{
    virtual void output() = 0; // 纯虚函数
    virtual ~base() = default; // 虚析构函数
};

template <void (*f)()>
struct function_wrapper : base
{
    void output() override
    {
        (*f)();
    }
};

template <typename Command, void (Command::*f)()>
struct command_wrapper : base
{
    void output() override
    {
        Command cmd;
        (cmd.*f)();
    }
};

void hello()
{
    std::cout << "Hello, World!" << std::endl;
}

struct say_hello
{
    void hello()
    {
        std::cout << "Hello, World!" << std::endl;
    }
};


int main()
{
    wrapper<int> intWrapper(42);
    std::cout << "Integer: " << intWrapper.get() << std::endl;
    
    wrapper<double> doubleWrapper(3.14);
    std::cout << "Double: " << doubleWrapper.get() << std::endl;
    
    std::cout << "As double: " << intWrapper.as<double>() << std::endl;

    using char_seq = value_sequence<char, 'a', 'b', 'c'>;
    char_seq::print();

    using int_seq = value_sequence<int, 0, 1, 2, 3>;
    int_seq::print();

    using bool_seq = value_sequence<bool, true, false, true>;
    bool_seq::print();

    auto w1 = function_wrapper<hello>();
    w1.output();

    auto w2 = command_wrapper<say_hello, &say_hello::hello>();
    w2.output();

    return 0;
}