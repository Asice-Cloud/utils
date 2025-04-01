//
// Created by asice-cloud on 4/1/25.
//

#include "../src/debug.hpp"

int main()
{
	debug(), "hello; hi";

	debug()<<"hi";

	debug(), []->std::runtime_error{return std::runtime_error("hello");}();

	// int a =1 ;
	// debug() >> a>2;
}