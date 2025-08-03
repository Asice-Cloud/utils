#include <source_location>
#include <iostream>
#include <string>
#include <type_traits>
#include <string_view>

template <typename T>
void log(const T &message, const std::source_location &location = std::source_location::current())
{
    std::string file = location.file_name();
    std::string parent_dir = file.substr(0, file.find_last_of('/'));
    std::string path = parent_dir.substr(parent_dir.find_last_of('/') + 1) + "/" + file.substr(file.find_last_of('/') + 1);
    std::cout << "[" << path
              << "(" << location.line()
              << " " << location.column() << ")]"
              << ": " << location.function_name()
              << ": " << message << "\n";
}

template <typename T>
std::string_view get_type_name()
{
    std::string_view sl = std::source_location::current().function_name();
    // std::string sl = __PRETTY_FUNCTION__;

    auto start = sl.find("T = ");
    start += 4;
    auto end = sl.find("]", start);
    return sl.substr(start, end - start);
}

template <typename T, T n>
std::string_view get_enum_name()
{
    std::string_view sl = std::source_location::current().function_name();

    auto start = sl.find("n = ");
    start += 4;
    auto end = sl.find_first_of("]", start);

    return sl.substr(start, end - start);
}

enum class Color
{
    Red = 1,
    Green = 2,
    Blue = 3,
    Yellow = 4,
    Cyan = 5,
};

template <int N>
struct int_constant
{
    static constexpr int value = N;
};

template <int start, int end, typename F>
void static_for(const F &f)
{
    if constexpr (start == end)
    {
        return;
    }
    else
    {
        // f(int_constant<start>());
        f(std::integral_constant<int, start>());
        static_for<start + 1, end>(f);
    }
}

template <int start = 0, int end = 10, typename T>
std::string_view get_enum_name_dynamic(T n)
{
    std::string_view ret;
    static_for<start, end + 1>([&](auto ic)
                               {
        constexpr auto i = ic.value;
        if (n == static_cast<T>(i))
        {
            ret = get_enum_name<T, static_cast<T>(i)>();
        } });

    if (ret.empty())
    {
        ret = "Unknown";
    }
    return ret;
}

template <typename T, int start = 0, int end = 10>
T enum_from_name(std::string_view name)
{
    for (int i = start; i <= end; ++i)
    {
        std::string_view enum_name = get_enum_name_dynamic((T)i);
        // Check if the enum name ends with the search name (handles "Color::Yellow" vs "Yellow")
        if (enum_name == name ||
            (enum_name.size() > name.size() &&
             enum_name.substr(enum_name.size() - name.size()) == name &&
             enum_name[enum_name.size() - name.size() - 1] == ':'))
        {
            return (T)i;
        }
    }

    throw std::runtime_error("Enum name not found");
}


// Add operator<< overload for Color enum to print the underlying value
std::ostream& operator<<(std::ostream& os, Color color) {
    os << static_cast<int>(color);
    return os;
}

int main()
{
    // log("Hello, World!");

    std::cout << get_type_name<std::string>() << '\n';

    std::cout << get_enum_name<Color, Color{4}>() << '\n';

    Color c = Color::Green;
    std::cout << get_enum_name_dynamic(c) << '\n';

    std::cout<<enum_from_name<Color>("Yellow") << '\n';
    return 0;
}