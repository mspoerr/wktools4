
#include "request_handler.h"
#include <cpprest\filestream.h>

class static_file_handler : public request_handler
{
public:
	static_file_handler()
		: request_handler("/app/")
	{
	}

	virtual void handle(web::http::http_request& request);
};

