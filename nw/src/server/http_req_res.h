#ifndef HTTP_REQ_RES_H
#define HTTP_REQ_RES_H

#include <string>
#include <unordered_map>
#include <sstream>

class HttpRequest {
public:
	std::string method;
	std::string uri;
	std::string version;
	std::unordered_map<std::string, std::string> headers;
	std::string body;

	// Parse raw HTTP request
	bool parse(const std::string& raw_request) {
		std::istringstream stream(raw_request);
		std::string line;

		// Parse request line
		if (!std::getline(stream, line) || line.empty()) return false;
		std::istringstream request_line(line);
		if (!(request_line >> method >> uri >> version)) return false;

		// Parse headers
		while (std::getline(stream, line) && !line.empty() && line != "\r") {
			auto colon_pos = line.find(':');
			if (colon_pos == std::string::npos) continue;
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);
			headers[key] = value;
		}

		// Parse body
		body = std::string(std::istreambuf_iterator<char>(stream), {});

		return true;
	}
};

class HttpResponse {
public:
	std::string version = "HTTP/1.1";
	int status_code = 200;
	std::string status_message = "OK";
	std::unordered_map<std::string, std::string> headers;
	std::string body;

	// Generate raw HTTP response
	std::string to_string() const {
		std::ostringstream response;
		response << version << " " << status_code << " " << status_message << "\r\n";
		for (const auto& [key, value] : headers) {
			response << key << ": " << value << "\r\n";
		}
		response << "\r\n" << body;
		return response.str();
	}
};

#endif // HTTP_REQ_RES_H