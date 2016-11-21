#include "stdwx.h"

#include "static_file_handler.h"

#include <filesystem>
#include <string>

void static_file_handler::handle(web::http::http_request& request)
{
	auto basePath = utility::conversions::to_string_t(std::tr2::sys::current_path<std::tr2::sys::path>().string());

	if (request.method() == web::http::methods::GET)
	{
		const auto appPrefix = utility::string_t(U("/app/"));
		const auto path = request.absolute_uri().path();
		auto fullPath = basePath + U("/") + path;
		concurrency::streams::file_stream<uint8_t>::open_istream(fullPath)
			.then([=](concurrency::streams::basic_istream<uint8_t> is){
			request.reply(web::http::status_codes::OK, is, U("text/html")).then([=](){
				is.close();
			});
		});
	}
}