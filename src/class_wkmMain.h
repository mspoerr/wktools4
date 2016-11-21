#pragma once

#include "stdwx.h"

#ifdef WKTOOLS_MAPPER
#include "class_wkmParser.h"
#include "class_wkmdb.h"
#include "class_wkmCombiner.h"
#include "class_wkmAusgabe.h"
#include "class_wkm.h"
#include "class_wkmImport.h"

#include "class_wkLog.h"

class Wkm;

class WkmMain
{
public:
	WkmMain(					// Konstruktor
		std::string sDir,			// Suchverzeichnis
		bool parse,					// Neu Parsen -> true == ja
		bool combine,				// Neues Auswerten -> true == ja
		Wkm *wkm,					// Parent
		WkLog *logAusgabe,			// Zeiger auf das Debug Fenster
		std::string ausgabe,		// Ausgabefile
		std::string pattern,		// Search Pattern
		bool cleanupDB,				// Datenbank aufräumen
		bool clearDB,				// Datenbank löschen
		std::string hostfile,		// File zum Import der nicht-erfassten Endgeräte
		bool import,				// Import YES/NO
		bool lzr					// Langzeitreports
		);
	~WkmMain();					// Destruktor
	void setAusgabeSets(		// Zum Setzen der Einstellungen für die Ausgabe
		bool cdp,					// CDP Infos anzeigen
		bool cdp_hosts,				// Endgeräte, die über CDP gelernt wurden, anzeigen
		bool unknownStp,			// Unbekannte STP Nachbaren anzeigen
		bool l2,					// L2 Ausgabe
		bool l3,					// L3 Ausgabe
		bool org,					// Hierarchisches Layout
		bool hier,					// Organisches Layout
		std::string unitEdgeLength,	// Unit Edge Length bei Organic Layout
		std::string lDistance,		// Layer Distance bei Hierarchisches Layout
		std::string nDistance,		// Node Distance bei Hierarchisches Layout
		std::string weightBalancing	// Weight Balancing bei Hierarchisches Layout
		);

	int doIt();					// Start
	int doMacSearch();			// MAC Search Dialog starten

private:
	WkmDB *dieDB;
	std::string sDir;			// Suchverzeichnis
	std::string oFile;			// Ausgabeverzeichnis
	std::string sPattern;		// Search Pattern
	std::string ld;				// Unit Edge Length bei Organic Layout
	std::string uel;			// Layer Distance bei Hierarchisches Layout
	std::string nd;				// Node Distance bei Hierarchisches Layout
	std::string wb;				// Weight Balancing bei Hierarchisches Layout
	std::string hostfile;		// File mit den Daten für den Import von nicht-erfassten Endgeräten

	bool parse;					// Neu Parsen -> true==ja
	bool combine;				// Neues Auswerten -> true == ja
	bool cleanupDB;				// Datenbank aufräumen
	bool clearDB;				// Datenbank löschen
	bool import;				// Import Daten JA/NEIN

	bool cdp;					// CDP Infos anzeigen
	bool cdp_hosts;				// Endgeräte, die über CDP gelernt wurden, anzeigen
	bool unknownStp;			// Unbekannte STP Nachbaren anzeigen

	bool lzReports;				// Langzeit Reports ausgeben?

	bool l2;					// L2 Ausgabe
	bool l3;					// L3 Ausgabe
	bool hier;					// Hierarchisches Layout
	bool org;					// Organisches Layout

	Wkm *wkMapper;				// Parent, um die Events richtig zu senden
	WkLog *logAusgabe;			// Zeiger auf das Ausgabefenster

	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,					// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
};

#endif // #ifdef WKTOOLS_MAPPER
