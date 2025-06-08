//
// Created by asice-cloud on 6/8/25.
//

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

#define debug debug1()

#define ciallo co_await

class debug1 {
public:
	template <typename T> debug1 &operator,(const T &t) {
		std::cout << "  " << t;
		return *this;
	}

	~debug1() { std::cout << std::endl; }
};


#endif //DEBUG_H
