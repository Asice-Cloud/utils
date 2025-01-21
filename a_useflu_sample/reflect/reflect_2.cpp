#include <iostream>
#include <string>
#include <tuple>

// 定义结构体
struct Person
{
	std::string name;
	int age;
	double height;
	Person(std::string _name, int _age, double _height) : name(_name), age(_age), height(_height) {};

	// 返回字段和值作为元组
	auto GetFields() const { return std::tie(name, age, height); }

	// 返回字段名称作为元组
	static auto GetFieldNames() { return std::make_tuple("Name", "Age", "Height"); }
};

// 递归模板打印元组内容
template <std::size_t Index = 0, typename T, typename... Args>
typename std::enable_if<Index == sizeof...(Args), void>::type PrintFields(const T &, const std::tuple<Args...> &)
{
}

template <std::size_t Index = 0, typename T, typename... Args>
	typename std::enable_if <
	Index<sizeof...(Args), void>::type PrintFields(const T &obj, const std::tuple<Args...> &fields)
{
	auto fieldNames = T::GetFieldNames();
	std::cout << std::get<Index>(fieldNames) << ": " << std::get<Index>(fields) << std::endl;
	PrintFields<Index + 1>(obj, fields);
}

int main()
{
	Person person{"Bob", 25, 180.2};
	auto fields = person.GetFields();
	PrintFields(person, fields);
	return 0;
}
