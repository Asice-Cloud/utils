//
// Created by asice-cloud on 10/1/24.
//
#include <iostream>
#include <rttr/registration>

struct Person
{
	std::string name;
	int age;
	double height;
};

// 注册结构体
RTTR_REGISTRATION
{
	rttr::registration::class_<Person>("Person")
		.property("name", &Person::name)
		.property("age", &Person::age)
		.property("height", &Person::height);
}

void PrintFields(rttr::instance obj)
{
	rttr::type type = obj.get_type();

	for (auto &prop : type.get_properties())
	{
		rttr::variant value = prop.get_value(obj);
		std::cout << prop.get_name() << ": " << value.to_string() << std::endl;
	}
}

int main()
{
	Person person{"Charlie", 28, 172.5};
	PrintFields(person);
	return 0;
}
