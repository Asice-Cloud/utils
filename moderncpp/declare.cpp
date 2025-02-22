#include <iostream>
#include <vector>

int main()
{

	std::vector<int> vec = {1, 2, 3, 45, 6};

	for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		std::cout << *it << std::endl;
	}

	// auto [k, v] = std::make_pair(1, 2);
	// auto [f, s, t] = std::make_tuple(1, 2, 3);


	return 0;
}
