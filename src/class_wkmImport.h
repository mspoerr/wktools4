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
	WkmDB *dieDB;						// Datenbank für die Einträge
	WkLog *logAusgabe;					// Log Ausgabefenster

	bool insertDevice();				// Neues Gerät in die DB schreiben
	bool rControlEmpty;					// Ist rControl leer?

	std::string zeitStempel(			// Zeitstempel eintragen
		std::string info					// zusätzliche Information
		);
	void schreibeLog(					// Funktion für die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);

	std::string dateiname;					// gerade bearbeitete Datei

	// Device Table
	int devicetype;							// Gerätetype bzw Kategorie -> Switch, Router, Firewall usw.
	std::string hostname;					// Hostname
	std::string ipAddress;					// SW Version 
	std::string macAddress;					// Gerätetype
	std::string intfName;					// STP Bridge ID
	std::string location;					// STP Protokoll
	std::string devType;					// Gerätekategorie (gleich wie "int devicetype", nur in std::string Format)
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
	bool testImportFile(					// Import File prüfen
		);
	void newlineEater(					// Zum Löschen der \n in der Linux Version
		);
};


#endif // #ifdef WKTOOLS_MAPPER
