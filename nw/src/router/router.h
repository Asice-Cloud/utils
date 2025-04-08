//
// Created by asice-cloud on 3/25/25.
//

#ifndef ROUTER_H
#define ROUTER_H
#include <string>
#include <unordered_map>

class Router
{
public:
	void add_router(const std::string& pattern, const std::string& service);
	std::string route(const std::string& request) const;

private:
	std::unordered_map<std::string, std::string> routers;
};


#endif //ROUTER_H
