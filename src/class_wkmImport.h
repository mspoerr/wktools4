#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER
#include "class_wkmdb.h"
#include "class_wkLog.h"

#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;

typedef unsigned int uint;

class WkmImport
{
private:
	WkmDB *dieDB;						// Datenbank f�r die Eintr�ge
	WkLog *logAusgabe;					// Log Ausgabefenster

	bool insertDevice();				// Neues Ger�t in die DB schreiben
	bool rControlEmpty;					// Ist rControl leer?

	std::string zeitStempel(			// Zeitstempel eintragen
		std::string info					// zus�tzliche Information
		);
	void schreibeLog(					// Funktion f�r die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text f�r den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);

	std::string dateiname;					// gerade bearbeitete Datei

	// Device Table
	int devicetype;							// Ger�tetype bzw Kategorie -> Switch, Router, Firewall usw.
	std::string hostname;					// Hostname
	std::string ipAddress;					// SW Version 
	std::string macAddress;					// Ger�tetype
	std::string intfName;					// STP Bridge ID
	std::string location;					// STP Protokoll
	std::string devType;					// Ger�tekategorie (gleich wie "int devicetype", nur in std::string Format)
	std::string chassisSN;					// Chassis S/N
	std::string modell;						// Modell Type

	std::string ganzeDatei;					// eingelesene Daten

	size_t aktPos1;
	size_t aktPos2;

public:
	volatile bool stop;					// TRUE, wenn Cancel
	WkmImport(							// Konstruktor
		std::string hostfile,			// Pfad, der durchsucht werden soll
		WkmDB *db,						// Datenbank
		WkLog *logA						// Log Ausgabe
		);
	~WkmImport();						// Destruktor

	int startParser(					// Parser starten
		);	
	bool testImportFile(					// Import File pr�fen
		);
	void newlineEater(					// Zum L�schen der \n in der Linux Version
		);
};


#endif // #ifdef WKTOOLS_MAPPER
