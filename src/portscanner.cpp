#include "portscanner.h"
#include "icmp_header.hpp"
#include "ipv4_header.hpp"

PortScanner::PortScanner()
{

}


PortScanner::~PortScanner()
{

}

bool PortScanner::connect(std::string port, std::string ipa)
{
	boost::asio::io_service ioserv;				// IO Service für ASIO Sockets
	boost::asio::ip::tcp::resolver resolver(ioserv);
	boost::asio::ip::tcp::resolver::query query(ipa, port);
	boost::system::error_code errorcode = boost::asio::error::host_not_found;
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, errorcode);
	boost::asio::ip::tcp::resolver::iterator end;

	tcp::socket verbindungsSock(ioserv);			// Socket für Scanner

	if (errorcode)
	{
		return false;
	}

	// Herstellen einer Verbindung
	while (endpoint_iterator != end)
	{
		verbindungsSock.close();
		verbindungsSock.connect(*endpoint_iterator++, errorcode);
	}
	if (errorcode)
	{
		// Fehlerausgabe
		return false;
	}
	else
	{
		verbindungsSock.close();
		return true;
	}

	return true;
}


bool PortScanner::ping(std::string ipa)
{
	try
	{
		boost::asio::io_service ioserv;				// IO Service für ASIO Sockets

		icmp::socket icmpSock(ioserv, icmp::v4());

		std::string body("\"Hello!\" from wktools.");

		icmp::endpoint icmpDest;
		unsigned short sequence_number_ = 0;
		boost::asio::streambuf reply_buffer_;

		icmp::resolver icmpResolver(ioserv);
		icmp::resolver::query query(icmp::v4(), ipa, "");
		icmpDest = *icmpResolver.resolve(query);

		// Create an ICMP header for an echo request.
		icmp_header echo_request;
		echo_request.type(icmp_header::echo_request);
		echo_request.code(0);
		echo_request.identifier(get_identifier());
		echo_request.sequence_number(++sequence_number_);
		compute_checksum(echo_request, body.begin(), body.end());

		// Encode the request packet.
		boost::asio::streambuf request_buffer;
		std::ostream os(&request_buffer);
		os << echo_request << body;

		// Send the request.
		icmpSock.send_to(request_buffer.data(), icmpDest);
		int wcounter = 0;
		size_t bytes = 0;

		boost::system::error_code error;

		while (1)
		{
			size_t bytes_readable = icmpSock.available(error);
			if (bytes_readable)
			{
				// TODO: Check, ob ICMP Echo Reply, oder was anderes
				char d1[256];
				bytes = icmpSock.receive_from(boost::asio::buffer(d1), icmpDest);

				if ((d1[20] == 0x00) && (d1[21] == 0x00))
				{
					return true;
				}
				else
				{
					return false;
				}

				break;

			}
			else
			{
				// Falls 3 Sekunden (30x100ms) nichts empfangen wurde, dann soll die Verbindung abgebrochen werden
				if (wcounter > 60)
				{
					return false;
					break;
				}

#ifdef _WINDOWS_
				Sleep(50);
#else
				usleep(50000);
#endif				
				wcounter++;
			}
		}

	}
	catch (std::exception& e)
	{
		return true;
	}


	return true;
}

