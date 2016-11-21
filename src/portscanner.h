#pragma once

#include "stdwx.h"

using boost::asio::ip::tcp;
using boost::asio::ip::icmp;

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>


class PortScanner
{
public:
	PortScanner();
	~PortScanner();
	bool ping(								// IP pingen
		std::string ipa							// IP Adresse
		);
	bool connect(							// Verbindung aufbauen
		std::string port,						// TCP Port
		std::string ipa							// IP Adresse
		);
private:

	static unsigned short get_identifier()
	{
#if defined(BOOST_WINDOWS)
		return static_cast<unsigned short>(::GetCurrentProcessId());
#else
		return static_cast<unsigned short>(::getpid());
#endif
	}

};