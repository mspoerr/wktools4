#ifndef __INTFPARSE__
#define __INTFPARSE__
#include "stdwx.h"
#include "class_dip.h"

#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;

typedef unsigned int uint;

class Dip;

class IntfParse
{
private:
	std::string suchVz;						// Verzeichnis, das durchsucht werden soll
	std::string ausgabeVz;					// Ausgabeverzeichnis	
	std::string ausgabeDat;					// Ausgabedatei
	int snmpLocCol;						// Anzahl der SNMP Location Spalten
	std::ofstream ausgabe;					// Ausgabe
	std::ofstream logAusgabeDatei;			// Die log-Datei
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Dip *devInfo;						// Zeiger auf Dip Parent, um die Events richtig zu senden
	std::queue<fs::path> files;				// Files, die nach IPs durchsucht werden sollen

	std::string aDatName;					// Ausgabedatei
	std::string dateiname;					// gerade bearbeitete Datei

	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		std::string logEintrag,				// Text für den Logeintrag
		std::string log2,					// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void schreibeLogfile(				// zum Schreiben von Einträgen in die Log Datei
		std::string logInput						// String, der in die Log Datei geschrieben werden soll
		);
	bool sdir(							// Verzeichnis samt Unterverzeichnisse durchsuchen
		fs::path pfad						// Pfad, der durchsucht werden soll
		);
public:
	volatile bool stop;					// TRUE, wenn Cancel
	IntfParse(							// Konstruktor
		std::string suchPfad,					// Pfad, der durchsucht werden soll
		std::string ausgabePfad,					// Ausgabeverzeichnis
		std::string ausgabeDatei,				// Ausgabedatei
		int slc,							// Anzahl der snmp location Spalten
		std::string logfile,						// Name des Logfiles
		WkLog *logAusgabe,					// Zeiger auf das Debug Fenster
		Dip *dip							// Zeiger auf Dip für die Events
		);
	~IntfParse();						// Destruktor
	int startParser(					// Parser starten
		std::string pattern,						// Search Pattern
		bool append,						// Ausgabe zur alten Datei hinzufügen?
		bool noDescr						// Interface Description mitausgeben?
		);					
};



#endif