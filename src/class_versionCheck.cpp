#include "class_versionCheck.h"
#include "class_wkLog.h"
#include "version.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include <wx/sstream.h>
#include <wx/tokenzr.h>


// Konstruktor:
//*************
// Log File muss angegeben werden
VersionCheck::VersionCheck(WkLog *lAsg) : wxThread()
{
	logAusgabe = lAsg;
}


// Destruktor:
//************
VersionCheck::~VersionCheck()
{

}


void *VersionCheck::Entry()
{
	int ret = 0;		// Wert, der mit der Nachricht mitgeschickt wird
						// 0: Aktuell
						// 1: Neuere Version verfügbar
						// 2: Fehler

	try
	{
		boost::asio::io_service io_service;

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
		tcp::resolver::query query("www.spoerr.org", "http");
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
			throw boost::system::system_error(error);

		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		boost::asio::streambuf request;
		std::ostream request_stream(&request);
		request_stream << "GET " << "/wktools/update/version4.txt" << " HTTP/1.0\r\n";
		request_stream << "Host: " << "www.spoerr.org" << "\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";

		// Send the request.
		boost::asio::write(socket, request);

		// Read the response status line.
		boost::asio::streambuf response;

		// Read until EOF, writing data to output as we go.
		while (boost::asio::read(socket, response,	boost::asio::transfer_at_least(1), error))
		{

		}
		
		if (error != boost::asio::error::eof)
		{
			throw boost::system::system_error(error);
		}
		
		// Bis zum Schluss alles verwerfen. Die Version steht ganz zum Schluss vom buffer
		std::string sbuf;
		std::istream response_stream(&response);
		while (!response_stream.eof())
		{
			response_stream >> sbuf;
		}

		// Versionen vergleichen
		if (vglVersion(sbuf, WKTOOLSVERSION))
		{
			// Neuere Version verfügbar
			std::string meldung = "Version Check: Newer Version available: ";
			meldung += sbuf;
			meldung += "\r\n";
			ret = 1;
			schreibeLog(WkLog::WkLog_ZEIT, meldung, "", 
				WkLog::WkLog_TUERKIS, WkLog::WkLog_FETT);
		}
		else
		{
			// Neueste Version wird verwendet
			schreibeLog(WkLog::WkLog_ZEIT, "Version Check: Latest Version in use\r\n", "", 
				WkLog::WkLog_TUERKIS, WkLog::WkLog_FETT);
		}

	}
	catch (std::exception& e)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "Version Check Error! - Visit www.spoerr.org/wktools for newer versions\r\n", "", 
			WkLog::WkLog_TUERKIS, WkLog::WkLog_FETT);
		ret = 2;
	}

	wxCommandEvent event(wkEVT_VCHECK_FERTIG);
	event.SetInt(ret);
	wxPostEvent(wxTheApp->GetTopWindow(), event);

	return NULL;
}


void VersionCheck::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
	WkLog::evtData evtDat;
	evtDat.farbe = farbe;
	evtDat.format = format;
	evtDat.groesse = groesse;
	evtDat.logEintrag = logEintrag;
	evtDat.logEintrag2 = log2;
	evtDat.type = type;

	LogEvent evt(EVT_LOG_MELDUNG);
	evt.SetData(evtDat);

	if (logAusgabe != NULL)
	{
		wxPostEvent(logAusgabe, evt);
	}
}


// Vergleiche die zwei Versionen
// v1: version4.txt Version
// v2: tatsächliche Programmversion
// true: wenn v1 größer
// false: wenn gleich
bool VersionCheck::vglVersion(wxString v1, wxString v2)
{
	int ver1[4];
	int ver2[4];

	for (int i = 0; i < 4; i++)
	{
		ver1[i] = 0;
		ver2[i] = 0;
	}

	// die Versionsstrings in INTs umwandeln
	wxStringTokenizer tkz1(wxT(v1), ".");
	for (int i = 0; tkz1.HasMoreTokens(); i++)
	{
		wxString token = tkz1.GetNextToken();

		ver1[i] = wxAtoi(token);
	}
	
	wxStringTokenizer tkz2(wxT(v2), ".");
	for (int i = 0; tkz2.HasMoreTokens(); i++)
	{
		wxString token = tkz2.GetNextToken();

		ver2[i] = wxAtoi(token);
	}


	// die Version INTs vergleichen
	for (int i = 0; i < 4; i++)
	{
		if (ver1[i] > ver2[i])
		{
			return true;
		}
		else if (ver2[i] > ver1[i])
		{
			return false;
		}
	}
	
	return false;
}