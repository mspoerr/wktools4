#pragma once
#include "stdwx.h"


#include <cpprest\http_listener.h>
#include <cpprest\filestream.h>
#include <cpprest\http_headers.h>

using namespace web::http::experimental::listener;
using namespace web::http;
using namespace web;

class Webserver
{
private:
	http_listener listener;

	void handle_get(http_request request);
	//void handle_put(http_request request);
	//void handle_post(http_request request);
	//void handle_delete(http_request request);

public:
	Webserver(				// Konstruktor
		const http::uri& url
		);			
	~Webserver();			// Destruktor

};





