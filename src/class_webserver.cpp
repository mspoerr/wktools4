#include "class_webserver.h"

#include <vector>
#include <string>

#include "request_handler.h"
#include "static_file_handler.h"
#include "tasks_handler.h"


Webserver::Webserver(const http::uri& url) : listener(http_listener(url))
{
	listener.support(methods::GET,
		std::tr1::bind(&Webserver::handle_get,
		this,
		std::tr1::placeholders::_1));
	//listener.support(methods::PUT,
	//	std::tr1::bind(&Webserver::handle_put,
	//	this,
	//	std::tr1::placeholders::_1));
	//listener.support(methods::POST,
	//	std::tr1::bind(&Webserver::handle_post,
	//	this,
	//	std::tr1::placeholders::_1));
	//listener.support(methods::DEL,
	//	std::tr1::bind(&Webserver::handle_delete,
	//	this,
	//	std::tr1::placeholders::_1));
}


Webserver::~Webserver()
{
	listener.close();
}


void Webserver::handle_get(http_request message)
{
	message.reply(status_codes::OK, U("wktools version 5"));
};