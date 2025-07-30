//
// Created by asice-cloud on 10/20/24.
//
#include<rttr/registration.h>
#include<rttr/type.h>
#include<iostream>

using namespace rttr;

struct Student
{
	int code;
	mutable const char* name;

	static void func(int x)
	{
		std::cout<< x<<'\n';
	};
};

RTTR_REGISTRATION
{
	registration::class_<Student>("Student")
		.constructor<>()
		.property("code", &Student::code)
		.property("name", &Student::name)
		.method("func", &Student::func);
}

int main()
{
	type t = type::get<Student>();
	for (auto& prop : t.get_properties())
		std::cout << "name: " << prop.get_name() << '\n';

	for (auto& meth : t.get_methods())
		std::cout << "name: " << meth.get_name() << '\n';

	type t1 = type::get_by_name("Student");
	variant var = t1.create(); // will invoke the previously registered ctor

	constructor ctor = t1.get_constructor(); // 2nd way with the constructor class
	var = ctor.invoke();
	std::cout << var.get_type().get_name()<<'\n'; // prints 'MyStruct'


	Student obj;
	property prop = type::get(obj).get_property("code");
	prop.set_value(obj, 42);

	variant var_prop = prop.get_value(obj);
	std::cout << var_prop.to_int()<<'\n'; // prints '23'
	method meth = type::get(obj).get_method("func");
	meth.invoke(obj, 42);

	variant var1 = type::get(obj).create();
	meth.invoke(var1, 42.0);
}
