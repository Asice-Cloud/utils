//
// Created by asice-cloud on 24-8-31.
//
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <variant>
#include <vector>
#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

template <class T>
T twice(T t)
{
	return t * 2;
}

// specialization, it used to handle the case that the template function can't handle
std::string twice(const std::string &s) { return s + ' ' + s; }

// default template parameter
template <class T = int>
T two(T t)
{
	return t << 1;
}

// any integer can use as a parameter
// more parameters:
// here N is a static constant (in compiling period)
// could used for optimizing
template <int N, class T>
void show_msg(T msg)
{
	for (int i = 0; i < N; i++)
	{
		std::cout << msg << '\n';
	}
}

// only some parameters specialization
/*
func(T t) 完全让参数类型取决于调用者。
func(vector<T> t) 这样则可以限定仅仅为 vector 类型的参数
 */
template <class T>
T sum(const std::vector<T> &arr)
{
	T res = 0;
	for (auto i : arr)
	{
		res += i;
	}
	return res;
}

// optimizing:
int sumto(int n, bool debug)
{
	int res = 0;
	for (int i = 0; i < n; i++)
	{
		res += i;
		if (debug)
		{
			std::cout << i << "-th: " << res << '\n';
		}
	}
	return res;
}
// change to:
template <bool debug>
int sumto(int n)
{
	int res = 0;
	for (int i = 0; i < n; i++)
	{
		res += i;
		// use constexpr to promise compiling branch
		if constexpr (debug)
		{
			std::cout << i << "-th: " << res << '\n';
		}
	}
	return res;
}

constexpr bool is_debug(int n) { return n % 2 == 0; }

// inertia(lazy) / multiply compile:
template <class T>
void print_add(const std::vector<T> &arr)
{
	std::cout << '{';
	for (size_t i = 0; i < arr.size(); i++)
	{
		std::cout << arr[i];
		if (i != arr.size() - 1)
		{
			std::cout << ", ";
		}
	}
	std::cout << "}\n";
}

/*
类型作为参数：template <class T>
整数值作为参数：template <int N>
定义默认参数：template <int N = 0, class T = int>
使用模板函数：myfunc<T, N>(...)
模板函数可以自动推断类型，从而参与重载
模板具有惰性、多次编译的特点
 */

// auto type derive:
// could declare a class member by auto
// auto could used to function's return; multiply return must keep same type
// reference: auto& or auto const&

// singleton: lazy man
auto &product_table()
{
	static std::map<std::string, int> instance;
	return instance;
}

// show type name:
template <class T>
std::string cpp_show_name()
{
	const char *name = typeid(T).name();
#if defined(__GNUC__) || defined(__clang__)
	int status;
	char *p = abi::__cxa_demangle(name, 0, 0, &status);
	std::string s = p;
	std::free(p);
#else
	std::string s = name;
#endif
	if (std::is_const_v<std::remove_reference_t<T>>)
		s += " const";
	if (std::is_volatile_v<std::remove_reference_t<T>>)
		s += " volatile";
	if (std::is_lvalue_reference_v<T>)
		s += " &";
	if (std::is_rvalue_reference_v<T>)
		s += " &&";
	return s;
}

int t;
int func_ref() { return t; }
int const &func_cref() { return t; }
int func_val() { return t; }

// using is similar as typedef, could make an alias of variable;
template <class T1, class T2>
auto add(std::vector<T1> const &a, std::vector<T2> const &b)
{
	using T0 = decltype(T1{} + T2{});
	std::vector<T0> ret;
	for (size_t i = 0; i < std::min(a.size(), b.size()); i++)
	{
		ret.push_back(a[i] + b[i]);
	}
	return ret;
}

// lambda expression in template
template <class Func>
void call_twice(Func const &func)
{
	std::cout << func(0) << '\n';
	std::cout << func(1) << '\n';
	std::cout << "size of Func" << sizeof(Func) << '\n';
}

auto make_twice(int fac)
{
	// could not use & because address has returned/resumed
	return [=](int n) { return n * fac; };
}

// avoid to use template in lambda:
// std::function<Return_type(Args)> func = ? ;
void call_third(std::function<int(int)> const &func)
{
	std::cout << func(0) << '\n';
	std::cout << func(1) << '\n';
	std::cout << func(2) << '\n';
	std::cout << "size of Func" << sizeof(func) << '\n';
}
std::function<int(int)> make_third(int fac)
{
	return [=](int n) { return n * fac; };
}

// another way: use function pointer
void call_forth(int func(int))
{
	std::cout << func(0) << '\n';
	std::cout << func(1) << '\n';
	std::cout << func(2) << '\n';
	std::cout << func(3) << '\n';
	std::cout << "size of Func" << sizeof(func) << '\n';
}

// yield mode:
template <class Func>
void fetch_data(Func const &func)
{
	for (int i = 0; i < 32; i++)
	{
		func(i);
		func(i + 0.5f);
	}
}

// static polymorphism
void print(std::variant<int, float> const &v)
{
	// match multiply
	std::visit([&](auto const &t) { std::cout << t << " -> " << cpp_show_name<decltype(t)>() << '\n'; }, v);
}
auto add(std::variant<int, float> const &v1, std::variant<int, float> const &v2)
{
	/* more parameters in visit
	std::variant<int, float> ret;
	std::visit([&](auto const &t1, auto const &t2) { ret = t1 + t2; }, v1, v2);
	return ret;
	 * */

	// also could return value;
	return std::visit([&](auto const &t1, auto const &t2) -> std::variant<int, float> { return t1 + t2; }, v1, v2);
}

int main()
{
	std::cout << twice(5) << '\n';
	std::cout << twice(5.5) << '\n';
	std::cout << twice(std::string("hello")) << '\n';

	std::cout << two(10) << '\n';

	show_msg<3>("I love you");
	show_msg<2>(17);

	std::cout << sumto(2, true) << '\n';
	// which in <> must be a compiling constant
	// and right must is a compiling constant too
	constexpr bool debug = false;
	std::cout << sumto<debug>(3) << '\n';
	std::cout << sumto<true>(3) << '\n';

	constexpr bool debug2 = is_debug(12);
	std::cout << sumto<debug2>(3) << '\n';

	std::vector<int> a1 = {1, 2, 3, 4, 5};
	std::vector<float> a2 = {1.1, 2.2, 3.3, 4.4, 5.5};
	print_add(a1);
	print_add(a2);

	auto map = product_table().emplace("Peqi", 100);
	product_table().emplace("Gorge", 200);

	std::cout << '\n';
	std::cout << cpp_show_name<int>() << '\n';
	std::cout << cpp_show_name<const int &>() << '\n'; // int const &

	// use decltype get type when one variable was declared
	std::cout << '\n';
	std::cout << "decltype" << '\n';
	std::cout << decltype(1 + 1)() << '\n'; // 0
	std::cout << cpp_show_name<decltype(1)>() << '\n'; // int
	std::cout << cpp_show_name<decltype(1.5 + 1)>() << '\n'; // double
	int a, *p;
	int arr[10];
	// decltype(variable) != decltype(expression), use decltype(()) to force compile later,get:
	std::cout << cpp_show_name<decltype(a)>() << '\n'; // int
	std::cout << cpp_show_name<decltype((a))>() << '\n'; // int&
	std::cout << cpp_show_name<decltype(p)>() << '\n'; // int*
	std::cout << cpp_show_name<decltype(p[0])>() << '\n'; // int&
	std::cout << cpp_show_name<decltype(arr)>() << '\n'; // int[10]
	std::cout << cpp_show_name<decltype("hello world")>() << '\n'; // char[12] const&

	// omnipotent auto derive: decltype(auto)
	decltype(auto) au1 = func_ref();
	decltype(auto) au2 = func_cref();
	decltype(auto) au3 = func_val();
	std::cout << "au1: " << cpp_show_name<decltype(au1)>() << " au2: " << cpp_show_name<decltype(au2)>()
			  << " au3: " << cpp_show_name<decltype(au3)>() << '\n';

	std::cout << '\n';
	std::vector<int> va = {2, 1, 7, 6, 3, 6, 8};
	std::vector<double> vb = {0.2f, 0.7f, 0.5f, 0.0f};
	auto vc = add(va, vb);
	for (size_t i = 0; i < vc.size(); i++)
	{
		std::cout << vc[i] << '\n';
	}

	std::cout << '\n';
	auto twice = make_twice(2);
	call_twice(twice);

	// function container
	std::cout << '\n';
	auto third = make_third(3);
	call_third(third);

	// function pointer
	call_forth([](int n) { return n * 4; });

	// duoble happy, you can use call_twice(auto const& func)
	auto twice_2 = []<class T>(T m) { return m * 2; };
	call_twice(twice_2);

	// yield mode:
	std::cout << '\n';
	std::vector<int> res_i;
	std::vector<float> res_f;
	fetch_data(
		[&res_i, &res_f](auto const &x)
		{
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, int>)
			{
				res_i.emplace_back(x);
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				res_f.emplace_back(x);
			}
		});

	for (auto &i : res_i)
	{
		std::cout << i << ' ';
	}
	std::cout << '\n';
	for (auto &i : res_f)
	{
		std::cout << i << ' ';
	}
	std::cout << '\n';

	// static polymorphism
	std::cout << '\n';
	std::variant<int, float> v = 3;
	// get value:
	std::cout << std::get<int>(v) << '\n';
	std::cout << std::get<0>(v) << '\n';
	if (std::holds_alternative<int>(v)) // judge type
	{
		std::cout << "now is int type" << '\n';
	}
	else if (v.index() == 1) // judge in another way, by return index == your wonder type's index
	{
		std::cout << "now is float type" << '\n';
	}
	else
	{
		std::cout << "what fuck?" << '\n';
	}
	print(v);
	v = 3.14f;
	print(v);

	std::cout << '\n';
	std::variant<int, float> vv = 42;
	print(add(vv, 42.24f));

	return 0;
}
