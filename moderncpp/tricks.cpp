#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
// 1.swap two variable:
// 2.construct function: aggregate initialization
// 3.use atr() rather than [] to find an element
// 4.insert std::pair< , > into map, use {} rather than std::make_pair()
// 5.insert won't not replace existent value
// 6.do not use std::endl anymore! std::endl will flush twice!
// 7.how to delete in mao: use erase_if
// 8.delete in vector

int main()
{
	{ // 1 6
		int a = 42;
		int b = 24;
		std::swap(a, b);
		std::cout << a << " " << b << '\n';
	}
	{ // 2
		struct Student
		{
			std::string name;
			int age;
			int id = 9999;
		};
		Student stu{"he", 24};
		// = Student stu {"he",24,9999};
		Student stu2{.name = "she", .age = 17, .id = 0};
	}
	{ //
		std::map<std::string, int> table;
		table.insert({"he", 11});
		table.at("he") = 24;
		std::cout << table.at("he") << '\n';
	}
	{ // 4 5
		std::map<std::string, int> table;
		table.insert(std::make_pair("he", 24));
		table.insert({"she", 17});
		table.insert({"she", 24});
		std::cout << table.at("she") << '\n';
	}
	{ // 6
		std::map<std::string, int> table;
		for (auto it = table.begin(); it != table.end();)
		{
			if (it->second < 0)
			{
				it = table.erase(it);
			}
			else
			{
				++it;
			}
		}
		// or you can use(in cpp 23):
		erase_if(table, [](std::pair<std::string, int> it) { return it.second < 0; });
	}
	{ // 7
		std::vector<int> vec{5, 4, 8, 9, 1, 6, -4, -6, 1};
		vec.erase(remove(vec.begin(), vec.end(), 5), vec.end());
		vec.erase(std::remove_if(vec.begin(), vec.end(), [](int x) { return x > 0; }), vec.end());
		for (auto i : vec)
		{
			std::cout << i << " ";
		}
		std::cout << '\n';
	}
}
