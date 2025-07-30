#include <iostream>

template <typename... Ts>
void magic(Ts... args)
{
	// get length of args
	std::cout << sizeof...(Ts) << std::endl;
}

// recursion
//
template <typename T0>
void print_all(T0 t)
{
	std::cout << t << std::endl;
}
template <typename T, typename... Ts>
void print_all(T value, Ts... args)
{
	std::cout << value << std::endl;
	if constexpr (sizeof...(Ts) > 0)
	{

		print_all(args...);
	}
}

// expand
template <typename T, typename... Ts>
void print_all2(T value, Ts... args)
{
	std::cout << value << std::endl;
	(void)std::initializer_list<T>{([&args] { std::cout << args << std::endl; }(), value)...};
}

template <typename... Ts>
auto sum(Ts... args)
{
	std::cout << (args + ... + 1) << '\n';
	std::cout << (1 + ... + args) << '\n';
	return (args + ...);
}


auto main() -> int
{
	magic(0);
	magic(1, 2, 3, 4, 5);
	std::cout << '\n';

	print_all(1, 2, 34, 5, 6, 7, 8);

	std::cout << '\n';

	print_all2(1, 2, 34, 5, 6, 7, 8);

	std::cout << sum(1, 2, 3, 4, 5, 7) << '\n';

	return 0;
}
