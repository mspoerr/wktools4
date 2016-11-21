#include "class_ipl.h"
#include "portscanner.h"
#include "version.h"

#include <boost/bind.hpp>
#include <boost/threadpool.hpp>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

// Konstruktor:
//*************
// Log File muss angegeben werden
IPL::IPL(std::string logFile, WkLog *logA, Wkl *wkL)
{
	wkListe = wkL;
	logAusgabe = logA;

	//	logAusgabeDatei.rdbuf()->open(logFile.c_str(), std::ios_base::out | std::ios_base::app | std::ios_base::binary);
}


// Destruktor:
//************
IPL::~IPL()
{
}


// erstelleIPL:
//*************
// Funktion zum Erstellen der IP Liste
// Es werden alle Dateien mit einer bestimmten Dateierweiterung (derw, auch BLANK möglich) 
// in einem Verzeichnis mit Unterverzeichnissen nach einer IP Adresse im Interface intf durchsucht.
uint IPL::erstelleIPL(std::string pfad, std::string ausgabeVz, std::string kriterien, std::string intf)
{
	// Suchkriterien einstellen
	// Als erstes werden die strings getrennt und in einen std::vector geschrieben
	std::vector<std::string> exps;
	size_t pos1 = 0;
	size_t pos2 = 0;
	while (pos2 != kriterien.npos)
	{
		pos2 = kriterien.find(";", pos1);
		std::string ph = kriterien.substr(pos1, pos2-pos1);
		exps.push_back(ph);
		pos1 = pos2+1;
	}
	size_t laenge = exps.size();

	// String, zur Debug Ausgabe
	std::string dbgA;

	// Initialisierung eines Strings, der alle IP Adressen aufnehmen soll.
	std::string alleIP;

	const fs::path dir_path(pfad);
	if (!sdir(dir_path))
	{
		return 0;
	}

	while (!files.empty())
	{
		std::string dateiname = files.front().string();
		files.pop();

		bool vorhanden = false;
		
		// Check, ob der Search Pattern im Filenamen vorkommt
		for (size_t i = 0; i < laenge; i++)
		{
			if (dateiname.find(exps[i]) != dateiname.npos)
			{
				vorhanden = true;
				break;
			}
		}
		if (vorhanden)
		{
			std::ifstream konfigDatei(dateiname.c_str(), std::ios_base::in);
			std::string ganzeDatei;
			
			dbgA = "4610: Searching file " + dateiname + "\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "4610", WkLog::WkLog_BLAU);

			// File einlesen			
			getline(konfigDatei, ganzeDatei, '\0');
			konfigDatei.close();

			std::string dasInterface = "\ninterface " + intf;

			// Wenn die Datei leer ist, wird dieser Durchlauf beendet.
			if (ganzeDatei.empty())
			{
				dbgA = "4611: " + dateiname + "is empty\n";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "4611", WkLog::WkLog_ROT);
			}
			else
			{
				std::string::size_type pos1 = 0;
				std::string::size_type pos2 = 0;

				pos1 = ganzeDatei.find(dasInterface, pos2);
				if  (pos1 == ganzeDatei.npos)
				{
					dbgA = "4403: Interface " + intf + " not found in " + dateiname + "\n";
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "4403", WkLog::WkLog_ROT);
				}
				else
				{
					pos2 = ganzeDatei.find("\n!", pos1);
					std::string::size_type pos3 = 0;
					
					// Feststellen, ob secondary IP Adressen vorhanden sind
					while (1)
					{
						pos3 = ganzeDatei.find("secondary", pos1);
						if (pos3 < pos2)
						{
							pos1 = pos3 + 1;
						}
						else
						{
							break;
						}
					}
					if ((pos1 = ganzeDatei.find("ip address ", pos1)) < pos2)
					{
						pos1 += 11;
						pos2 = ganzeDatei.find(" ", pos1);
						std::string ipadd = ganzeDatei.substr(pos1, pos2-pos1);
						ipadd += ";\t";
						alleIP += ipadd;
						alleIP += "<<";
						alleIP += intf;
						alleIP += ">>\t";
						alleIP += dateiname;
						alleIP += "\n";
						// IP Adresse ausschneiden und einfügen
					}
					else
					{
						dbgA = "4404: No IP address found at Interface in " + dateiname + "\n";
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "4404", WkLog::WkLog_ROT);
					}
				}
			}
		}
	}
	
	dbgA = "\n\n4612: --- FINSHED ---\n\n";
	schreibeLog(WkLog::WkLog_ABSATZ, dbgA, "4612", WkLog::WkLog_GRUEN);

	// Ausgabedatei für die IP Liste:
	std::string ausgabeDatei = ausgabeVz + "iplist.txt";
	std::ofstream ausgabe(ausgabeDatei.c_str(), std::ios_base::out | std::ios_base::binary);
	ausgabe << alleIP;
	ausgabe.close();
	return 1;
}


void IPL::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
	if (logAusgabe != NULL)
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
		wxPostEvent(logAusgabe, evt);
	}
}


// schreibeLogfile:
//*****************
// Zum Beschreiben des Log Files
void IPL::schreibeLogfile(std::string logInput)
{
	//logAusgabeDatei << logInput;
}


bool IPL::sdir(fs::path pfad)
{
	if ( !exists( pfad ) ) 
	{
		return false;
	}

	std::string dbgA = "4609: Searching Directory " + pfad.string() + "\n";
	schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "4609", WkLog::WkLog_BLAU);

	fs::directory_iterator end_itr; // default construction yields past-the-end
	for ( fs::directory_iterator itr( pfad ); itr != end_itr; ++itr )
	{
		if ( fs::is_directory(itr->status()) )
		{
			// Pfad gefunden
			sdir(itr->path());
		}
		else
		{
			// File gefunden
			files.push(itr->path());
		}
	}
	return true;
}


void IPL::startScanner(std::size_t i)
{
	bool erfolgreich = false;

	try
	{
		PortScanner *derScanner = new PortScanner();
		
		// Wenn das Tool nicht als Admin gestartet wird, dann funktionieren keine Raw Sockets
		try
		{
			erfolgreich = derScanner->ping(ips[i]);
		}
		catch (std::exception& e)
		{
			erfolgreich = true;
		}

		if (erfolgreich)
		{
			for (std::size_t j = 0; j < ports.size(); j++)
			{
				erfolgreich = derScanner->connect(ports[j], ips[i]);
				if (erfolgreich)
				{
					boost::mutex::scoped_lock lock(io_mutex);
					ipQueue.push(ips[i]);
					portQueue.push(ports[j]);
					break;
				}
			}
		}

		delete derScanner;
	}
	catch (std::exception& e)
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, e.what(), "", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	
}


uint IPL::erstelleIPPL(std::string ipRange, std::string ausgabeVz, std::string dummy, std::string dummy2)
{

//	ipRange = "192.168.83.1-192.168.83.20;192.168.80.1-192.168.83.30;192.168.83.254";
	if (ipRange[ipRange.size()] != ';' )
	{
		ipRange += ";";
	}
	std::size_t spPos1 = 0;
	std::size_t spPos2 = ipRange.find(";");
	if (spPos2 == ipRange.npos)
	{
		spPos2 = ipRange.size();
	}

	while (spPos2 != ipRange.npos)
	{
		std::string sTeil = ipRange.substr(spPos1, spPos2-spPos1);
		std::size_t dPos = sTeil.find("-");
		if (dPos != sTeil.npos)
		{
			// Prüfen, ob Range Eingabe korrekt
			std::string ip1 = sTeil.substr(0,dPos);
			std::string ip2 = sTeil.substr(dPos+1, ipRange.size()-dPos-1);

			boost::system::error_code errorcode = boost::asio::error::host_not_found;
			boost::asio::ip::address_v4 ipa1 = boost::asio::ip::address_v4::from_string(ip1, errorcode);
			if (errorcode)
			{
				schreibeLog(WkLog::WkLog_NORMALTYPE, "4201: IP Range Input error: " + ip1, "4201", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				return 0;
			}

			boost::asio::ip::address_v4 ipa2 = boost::asio::ip::address_v4::from_string(ip2, errorcode);
			if (errorcode)
			{
				schreibeLog(WkLog::WkLog_NORMALTYPE, "4201: IP Range Input error: " + ip2, "4201", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				return 0;
			}

			// Umwandeln der IPs in ulong und den kompletten Range in den vector einlesen
			unsigned long ipa1Long = ipa1.to_ulong();
			unsigned long ipa2Long = ipa2.to_ulong();

			for (;ipa1Long <= ipa2Long; ipa1Long++)
			{
				boost::asio::ip::address_v4 a(ipa1Long);
				ips.push_back(a.to_string());
			}
		}
		else
		{
			if (sTeil == "")
			{
			}
			else if (sTeil.size() < 7)
			{
				schreibeLog(WkLog::WkLog_NORMALTYPE, "4201: IP Range Input error: " + sTeil, "4201", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				return 0;
			}
			else
			{
				ips.push_back(sTeil);
			}
		}

		spPos1 = spPos2+1;
		spPos2 = ipRange.find(";", spPos1);
	}

	ports.push_back("22");
	ports.push_back("23");
	ports.push_back("80");
	ports.push_back("443");

	boost::threadpool::pool threads(25);
	for (std::size_t i = 0; i < ips.size(); i++)
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "Scanning " + ips[i] + "\n", "", WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
		threads.schedule(boost::bind(&IPL::startScanner, this, i));
	}

	threads.wait();

	schreibeLog(WkLog::WkLog_ABSATZ, "\n\n<<< RESULT >>>\n", "", WkLog::WkLog_GRUEN, WkLog::WkLog_FETT);

	// Ausgabedatei für die IP Liste:
	std::string ausgabeDatei = ausgabeVz + "ipPortList.txt";
	std::ofstream ausgabe(ausgabeDatei.c_str(), std::ios_base::out | std::ios_base::binary);
	ausgabe << "wktools-generated;" << WKTOOLSVERSION << "\n";

	while (ipQueue.size() > 0)
	{
		ausgabe << ipQueue.front() << ";" << portQueue.front() << "\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, ipQueue.front() + ": " + portQueue.front() + "\n", "", WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
		ipQueue.pop();
		portQueue.pop();
	}

	ausgabe.close();
	
	schreibeLog(WkLog::WkLog_ABSATZ, "\n\n4612: --- FINSHED ---\n\n", "4612", WkLog::WkLog_GRUEN, WkLog::WkLog_FETT);
	
	return 1;
}