// ALT - Combiner2 verwenden !!!!!!!!!
//////////////////////////////////////

#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER

#include "class_wkmdb.h"
#include "class_wkLog.h"

#include <iostream>
#include <string>
#include <vector>
#include <iostream>



class WkmCombiner
{
private:
	WkmDB *dieDB;					// Die Datenbank
	WkLog *logAusgabe;				// Log Ausgabefenster

	void hostSchreiber();			// Zum Eintragen der Endgeräte in die device Tabelle
	void stpKonistenzCheck();		// Einträge in neighborship DB auf Konsistenz und Vereinfachungen prüfen
	void delDups();					// Doppelte Einträge löschen
	void markAnomaly();				// Anomalien finden und markieren
	void anomalyCdp();				// Anomalien mit den verfügbaren CDP Informationen vergleichen und ausbessern
	void cdpChecks();				// CDP Infos auswerten
	void schreibeLog(						// Funktion für die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,						// Text für den Logeintrag
		std::string log2,					// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void setSwitchDevType();		// Setzen vom richtigen Switch Type (L2, L3, Root usw)
	void insertRControl();			// rControl Tabelle befüllen

	void dbgNachbarAusgabe();		// Nachbarschaftsbeziehungen im Debug Mode ausgeben
	void dbgAsgbe1(					// Debug Ausgabe für SQL Aufrufe und Ergebnisse
		std::string dgbAusgabe,			// Debug String
		int type,						// Global oder untergeordnet: 1: Global; 2: Untergeordnet
		int anzahlWerte,				// Anzahl der Rückgabewerte bei Ergebnis
		std::string position			// Beschreibung der Position
		);

	bool nidEmpty;					// rc_id leer?
	bool importHosts;				// Endgeräte in device Table importieren?
	bool reverseDns;				// Reverse DNS Lookup für Endgeräte
	int dbgGlobalCounter;			// Counter für globale Dbg1 Ausgabe
	int dbgUntergeordnetCounter;	// Counter für untergeordnete Dbg1 Ausgabe


public:
	WkmCombiner(					// Konstruktor
		WkmDB *db,						// Datenbank
		WkLog *logA,
		bool impHosts,					// Hosts Importieren?
		bool revDns						// Reverse DNS für Endgeräte
		);
	~WkmCombiner();					// Destrukor
	bool doIt();					// Start
	bool debugAusgabe;
	bool debugAusgabe1;				// Detaillierte Debug Ausgabe für SQL Abfragen
	volatile bool stop;				// TRUE, wenn Cancel

};

#endif //#ifdef WKTOOLS_MAPPER
