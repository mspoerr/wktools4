#ifndef __UTA__
#define __UTA__
#include "stdwx.h"
#include "class_dip.h"

#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;

typedef unsigned int uint;

class Dip;

class UTAParse
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
	void schreibeJa(					// wird verwendet, wenn Ausgabe geschrieben werden soll
		std::string text							// String der ausgegeben werden soll
		);
	void schreibeNein(					// wird verwendet, wenn nichts ausgegeben werden soll
		std::string nix							// Dummy String
		);
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
	void varsInit();					// zum Initialisieren der Variablen
	void testPhones(					// ausgelagerte Funktion zum Testen der IP Phone Daten
		std::string daten						// zu testende Daten
		);
	void testType(						// ausgelagerte Funbktion zum Testen, ob CatOS, IOS, PIXOS...
		std::string daten						// zu testende Daten 
		);
	std::string aDatName;					// Ausgabedatei
	std::string dateiname;					// gerade bearbeitete Datei

	bool catos;							// CatOS Daten?
	bool dslam;							// Handelt es sich bei den eingelesenen Date um einen DSLAM?
	bool pixos;							// PIX, ASA, FWSM Daten?
	bool iosbox;						// Gerät mit IOS?
	bool nxosbox;						// Nexus
	bool ipphone;						// IP Phone?
	bool filesystem;					// Filesystem?
	bool nodiag;						// diag Infos?
	bool c72er;							// 7200 Router?
	bool gsr;							// GSR?
	std::string hostname;				// Hostname
	std::string version;				// IOS 
	std::string deviceType;				// Gerätetype
	std::string prozessor;				// Prozessor Type
	std::string RAM;					// RAM 
	std::string chassisSN;				// S/N vom Chassis
	std::string npeSN;					// S/N von der NPE
	std::string npeBez;					// NPE Type
	std::string standort;				// SNMP Location
	std::string iosversion;				// IOS Versionsnummer
	std::string flash;					// Flashgröße
	std::string beschreibung;			// Beschreibung
	std::string lic;							// Lizenzinfo
	std::queue<std::string> modulRouteMem;		// Route Memory
	std::queue<std::string> modulPacketMem;		// Packet Memory
	std::queue<std::string> tempModNr;			// temporäre ModulNummer bei OSM Modulen
	std::queue<std::string> tempMem;			// Temporärer Memory bei OSM Modulen
	std::queue<std::string> modulNr;			// Modulnummer
	std::queue<std::string> modulBez;			// Modulbezeichnung (Produktnummer)
	std::queue<std::string> modulSN;			// S/N vom Modul
	std::queue<std::string> hwRevision;			// Hardware Revision
	std::queue<std::string> modulBesch;			// Modulbeschreibung
	std::queue<std::string> modulVer;			// IOS Version vom Modul
	// folgende size_t's sind zum Auffinden der wichtigsten show Befehle
	size_t pos1;						// HauptPos1
	size_t pos2;						// HauptPos2
	size_t pos3;						// ...
	size_t pos4;
	size_t pos5;
	size_t pos6;
	size_t pos7;
	size_t pos8;
	size_t pos1dsl;						// DSLAM POS 1
	size_t pos2dsl;						// DSLAM POS 3
	size_t pos3dsl;						// ....
	size_t pos1iph;						// IP Phone Pos
	size_t pos2iph;						// IP Phone Pos
	size_t pos3iph;						// IP Phone Pos
	size_t aktPos1;
	size_t aktPos2;

public:
	volatile bool stop;					// TRUE, wenn Cancel
	UTAParse(							// Konstruktor
		std::string suchPfad,					// Pfad, der durchsucht werden soll
		std::string ausgabePfad,					// Ausgabeverzeichnis
		std::string ausgabeDatei,				// Ausgabedatei
		int slc,							// Anzahl der snmp location Spalten
		std::string logfile,						// Name des Logfiles
		WkLog *logAusgabe,					// Zeiger auf das Debug Fenster
		Dip *dip							// Zeiger auf Dip für die Events
		);
	~UTAParse();						// Destruktor
	struct oo {							// Output Options
		bool spalte1;
		bool spalte2;
		bool spalte3;
		bool spalte4;
		bool spalte5;
		bool spalte6;
		bool spalte7;
		bool spalte8;
		bool spalte9;
		bool spalte10;
		bool spalte11;
	};
	int startParser(					// Parser starten
		std::string pattern,						// Search Pattern
		bool append,						// Ausgabe zur alten Datei hinzufügen?
		oo outOpt,							// Output Options (Welche Spalten werden angezeigt)
		bool nurChassis						// Nur die Chassis Infos ausgeben?
		);					
};



#endif