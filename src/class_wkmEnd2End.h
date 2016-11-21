#pragma once
#include "stdwx.h"
#include "boost/lexical_cast.hpp"

#ifdef WKTOOLS_MAPPER
#include "class_wkmdb.h"
#include "class_wkLog.h"


class WkmEnd2End
{
public:
	WkmEnd2End(								// Konstruktor
		WkLog *logA,							// Log Ausgabe
		WkmDB *db								// Datenbank
		);
	~WkmEnd2End();

	 std::string doEnd2End(					// Analysiere E2E von h1 und h2
		std::string h1,							// IP Adresse 1
		std::string h2							// IP Adresse 2
		);


	struct hostDetails						// Host Details
	{
		std::vector<std::string> hIp;			// Host - IP Adresse
		std::vector<std::string> hMac;			// Host - MAC Adresse
		std::vector<std::string> hintf_id;		// Host - Interface ID auf dem das Gerät hängt
		std::vector<std::string> hIntfName;		// Host - Interface Name
		std::vector<std::string> hHostname;		// Host - NW Box Hostname
		std::vector<std::string> hIntfDescr;	// Host - Interface Description
		std::vector<std::string> hIntfType;		// Host - Interface Type
		std::vector<std::string> hdev_id;		// Host - Device ID der NW Box, auf dem das Gerät hängt
		std::vector<std::string> hvlan;			// Host - VLAN
		std::vector<std::string> hsubnet;		// Host - Subnet
		std::vector<std::string> hdg;			// Host - Default Gateway
		std::vector<std::string> hOuiVendor;	// Host - OUI Vendor ID
		std::vector<std::string> timestamp;		// Zeitstempel von der DB
		std::vector<bool> status;				// Host Details gefunden (true) oder fehlgeschlagen (false)
		int anzahl;								// Anzahl der verfügbaren Werte

		std::string errorCode;
	};
	
	hostDetails ipSearch(					// IP Adresse suchen
		std::string ipAddress					// zu suchende IP Adresse
		);

private:
	WkmDB *dieDB;							// Datenbank für die Einträge
	WkLog *logAusgabe;						// Logsuagabe Fenster

	bool debugAusgabe;						// Debug Ausgabe?


	void schreibeLog(						// Funktion für die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);

	std::vector<std::string> ExecuteRequest(unsigned long src, unsigned long dest, bool *cloudPresent);
	WkmEnd2End::hostDetails FilterResponse(std::vector<std::vector<std::string>> sqlBaseResponse);
	bool CheckIfTargetNetworkReached(unsigned long src, unsigned long dest);

};
#endif