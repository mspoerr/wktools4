#include "class_wkmMacUpdate.h"

#ifdef WKTOOLS_MAPPER
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

MacUpdate::MacUpdate(std::string dbName)
{
	dieDB = new WkmDB(dbName, true);
	dieDB->query("DELETE FROM macOUI");

}


MacUpdate::~MacUpdate()
{
//	dieDB->close();
	delete dieDB;
}


int MacUpdate::doUpdate()
{
	// Return: 
	// 0 - OK
	// 1 - CONNECT FEHLER
	// 2 - HTTP Fehler
	// 3 - DB Fehler

	int ret = 0;
	boost::asio::io_service io_service;

	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_service);
	tcp::resolver::query query("www.ieee.org", "http");
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	// Try each endpoint until we successfully establish a connection.
	tcp::socket socket(io_service);
	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end)
	{
		socket.close();
		socket.connect(*endpoint_iterator++, error);
	}
	if (error)
	{
		return 1;
	}
	// Form the request. We specify the "Connection: close" header so that the
	// server will close the socket after transmitting the response. This will
	// allow us to treat all data up until the EOF as the content.
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << "GET " << "/oui.txt" << " HTTP/1.1\r\n";
	request_stream << "Host: " << "standards-oui.ieee.org" << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: keep-alive\r\n\r\n";
	// Send the request.
	boost::asio::write(socket, request);

	// Daten EMPFANGEN
	size_t bytes = 0;					// Anzahl der empfangenen Bytes
	std::string eBuf = "";				// Empfangsdaten in c++-std::string-Form
	boost::asio::streambuf iSBuf;
	std::istream iStream(&iSBuf);

	bytes = boost::asio::read_until(socket, iSBuf, "\r\n");

	// Check that response is OK.
	std::string http_version;
	iStream >> http_version;
	int status_code;
	iStream >> status_code;
	std::string status_message;
	getline(iStream, status_message);

	if (!iStream || http_version.substr(0, 5) != "HTTP/")
	{
		return 1;
	}
	if (status_code != 200)
	{
		return 2;
	}

	// Read the response headers, which are terminated by a blank line.
	bytes = boost::asio::read_until(socket, iSBuf, "\r\n\r\n");

	char *iBuf = new char[bytes+1];
	iStream.read(iBuf, bytes);
	iBuf[bytes] = 0x00;

	eBuf.assign(iBuf);
	delete[] iBuf;		// iBuf löschen

	std::string sbuf = "";
	
	// Read until EOF, writing data to output as we go.
	error = boost::asio::error::host_not_found;
	while (bytes = boost::asio::read(socket, iSBuf, boost::asio::transfer_at_least(1), error))
	{
//#ifdef _WINDOWS_
//		Sleep(10);
//#else
//		usleep(10000);
//#endif				
		iBuf = new char[bytes+1];
		iStream.read(iBuf, bytes);
		iBuf[bytes] = 0x00;

		eBuf.assign(iBuf);
		delete[] iBuf;		// iBuf löschen

		sbuf += eBuf;

		//boost::asio::socket_base::bytes_readable command(true);
		//socket.io_control(command);
		//size_t bytes_readable = command.get();

		//if (!bytes_readable)
		//{
		//	if (eBuf.find("</html>") != eBuf.npos || eBuf.find("</HTML>") != eBuf.npos || error == boost::asio::error::eof)
		//	{
		//		error = boost::asio::error::eof;
		//		break;
		//	}
		//}
	}
	if (error != boost::asio::error::eof)
	{
		return 2;
	}

	sbuf = sbuf.substr(sbuf.find("00-00-00"));

	// In DB schreiben
	std::string macZeile = "";
	int offsets[] = {8, 10, 50};
	size_t aktPos1 = 0;
	size_t aktPos2 = 0;

	std::string macAddr = "";
	std::string vendor = "";

	boost::offset_separator f(offsets, offsets+3, false);
	for (; aktPos2 < sbuf.npos;)
	{
		aktPos2 = sbuf.find("\n", aktPos1);
		macZeile = sbuf.substr(aktPos1, aktPos2 - aktPos1);

		// Zu klein
		if (macZeile.size() < 10)
		{
			aktPos1 = aktPos2+1;
			continue;
		}

		// Adressangabe beim Vendor
		if (macZeile[0] == '\t')
		{
			aktPos1 = aktPos2+1;
			continue;
		}

		// MAC Adresse im base-16 Format
		if (macZeile[2] != '-')
		{
			aktPos1 = aktPos2+1;
			continue;
		}

		aktPos1 = aktPos2;

		int i = 0;
		boost::tokenizer<boost::offset_separator> tok(macZeile, f);

		for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
		{
			switch (i)
			{
			case 0:
				macAddr = *beg;
				break;
			case 2:
				vendor = *beg;
				boost::algorithm::trim(vendor);
				break;
			default:
				break;
			}
			i++;
		}
		if (macAddr.find("-") != macAddr.npos)
		{
			boost::algorithm::to_lower(macAddr);
			macAddr = macAddr.erase(macAddr.find("-"), 1);
			macAddr = macAddr.replace(macAddr.find("-"), 1, ".");

			std::string sString = "INSERT INTO macOUI (oui_id,oui,vendor) VALUES (NULL,'";
			sString += macAddr + "','" + vendor + "');";
			dieDB->query(sString.c_str());
		}
		aktPos1 = aktPos2+1;

	}

	ret = 0;
	return ret;
}

#endif