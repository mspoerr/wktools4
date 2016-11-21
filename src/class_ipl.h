#ifndef __IPL__
#define __IPL__

#include "stdwx.h"
#include "class_wkl.h"

#include <string>
#include <queue>
#include <vector>

#include "boost/filesystem.hpp"
#include <boost/thread/mutex.hpp>
namespace fs = boost::filesystem;

typedef unsigned int uint;

class Wkl;

class IPL
{
private:
	std::ofstream logAusgabeDatei;		// Die log-Datei
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Wkl *wkListe;						// Zeiger auf Wkn Parent, um die Events richtig zu senden
	std::queue<fs::path> files;			// Files, die nach IPs durchsucht werden sollen
	boost::mutex io_mutex;				// Boost Mutex
	std::vector<std::string> ips;		// Vector mit den IPs, die gesucht werden sollen
	std::vector<std::string> ports;		// Vector mit den Ports, die gescannt werden sollen
	std::queue<std::string> ipQueue;	// Alle IPs, bei denen ein offenes Port gefunden wurde
	std::queue<std::string> portQueue;	// Passendes Port zur IP von ipQueue

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
	IPL(								// Konstruktor
		std::string logfile,						// Name des Logfiles
		WkLog *logAusgabe,					// Zeiger auf das Debug Fenster
		Wkl *wkL							// Zeiger auf Wkl für die Events
		);
	~IPL();								// Destruktor
	uint erstelleIPL(					// Start zum Erstellen der IP Liste per Verzeichnis durchsuchen
		std::string pfad,					// Pfad, der durchsucht werden soll
		std::string ausgabeVz,				// Pfad für die Ausgabe der IP Liste
		std::string kriterien,				// Dateierweiterung
		std::string intf					// Interface, von wo die IP Adresse ausgelesen werden soll
		);
	uint erstelleIPPL(					// Start zum Erstellen der IP Liste per Port Scanner
		std::string ipListe,				// String mit den IP Adress Ranges
		std::string ausgabeVz,				// Pfad für die Ausgabe der IP Liste
		std::string dummy,					// Dummy String
		std::string dummy2					// Dummy String 2
		);
	void startScanner(					// Starte Scanner (für den Thread)
		std::size_t i						// Index von der zu suchenden IP
		);
};

#endif