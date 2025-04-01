#include <iostream>
#include "router.h"

int main() {
	Router router;
	router.add_router("^/api/v1/.*$", "v1_service");
	router.add_router("^/api/v2/.*$", "v2_service");

	std::string request1 = "/api/v1/resource";
	std::string request2 = "/api/v2/resource";
	std::string request3 = "/api/v3/resource";

	std::cout << "Request: " << request1 << " -> Service: " << router.route(request1) << std::endl;
	std::cout << "Request: " << request2 << " -> Service: " << router.route(request2) << std::endl;
	std::cout << "Request: " << request3 << " -> Service: " << router.route(request3) << std::endl;

	return 0;
}