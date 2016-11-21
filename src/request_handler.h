#pragma once

#include <cpprest\http_listener.h>
#include <string>

class request_handler
{
private:
	std::string prefix;
public:
	request_handler(std::string prefix)
		: prefix(prefix)
	{
	}

	std::string get_prefix()
	{
		return this->prefix;
	}

	virtual void handle(web::http::http_request& request) = 0;
};

