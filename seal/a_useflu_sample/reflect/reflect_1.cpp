#include <functional>
#include <iostream>
#include <string>
#include <tuple>

// 一个宏用于定义成员的名字和引用
#define REFLECTABLE(...)                                                                                               \
	static auto GetFields() { return std::make_tuple(__VA_ARGS__); }

struct Person
{
	std::string name;
	int age;
	double height;

	// 使用宏定义元数据
	REFLECTABLE(std::make_pair("name", &Person::name), std::make_pair("age", &Person::age),
				std::make_pair("height", &Person::height))
};

// 遍历结构体字段和值的通用函数
template <typename T>
void PrintFields(T &obj)
{
	auto fields = T::GetFields();

	std::apply([&](auto &&...field) { (..., (std::cout << field.first << ": " << obj.*(field.second) << std::endl)); },
			   fields);
}

int main()
{
	Person person{"Alice", 30, 165.5};
	PrintFields(person); // 打印字段和值
	return 0;
}
