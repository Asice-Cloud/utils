#include <iostream>
#include <source_location>
#include <string_view>


void log(const std::string_view mes, const std::source_location loc = std::source_location::current())
{

	std::cout << "file:" << loc.file_name() << "\nline:" << loc.line() << "\ncol:" << loc.column()
			  << "\nfunc:" << loc.function_name() << "\nmes:" << mes << '\n';
}

int main() { log("hello"); }
