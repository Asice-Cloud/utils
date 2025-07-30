//
// Created by asice-cloud on 3/25/25.
//

#include "router.h"
#include <regex>

void Router::add_router(const std::string& pattern, const std::string& service)
{
	routers[pattern] = service;
}

std::string Router::route(const std::string& request) const
{
	for (const auto& router : routers)
	{
		std::regex pattern(router.first);
		if (std::regex_match(request, pattern))
		{
			return router.second;
		}
	}
	return "404 Not Found";
}