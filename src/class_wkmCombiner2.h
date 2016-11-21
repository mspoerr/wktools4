#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER

#include "class_wkmdb.h"
#include "class_wkLog.h"
#include "class_wkmParserDB.h"

#include <iostream>
#include <string>
#include <vector>
#include <iostream>



class WkmCombiner2
{
private:
	WkmDB *dieDB;					// Die Datenbank
	WkmParserDB *parserdb;			// Parser DB Operation
	WkLog *logAusgabe;				// Log Ausgabefenster

	void rControlUpdate();			// Aktualisieren von rControl wenn im Nachhinein (nach dem Parser) Daten in DB geschrieben wurden
	void hostSchreiber();			// Zum Eintragen der Endger�te in die device Tabelle
	void stpKonistenzCheck();		// Eintr�ge in neighborship DB auf Konsistenz und Vereinfachungen pr�fen
	void delDups();					// Doppelte Eintr�ge l�schen
	void delDupsRP();				// Doppelte Eintr�ge in der rpneighborship Tabelle l�schen; 
	void markAnomaly();				// Anomalien finden und markieren
	void cdpChecks();				// CDP Infos auswerten
	void switchChecks();			// Switch Neighbor Checks
	void vpcCheck();				// Nexus vPC Check
	void unknownSTP();				// Unknown STP Checks
	void unknownRP();				// Unknown RP Neighbor und NextHops
	void dedicatedL2();				// Dedizierte L2 Neighbor Checks (Non-Switch zu Non-Switch und Non-Switch zu Switch)
	void rpNeighborCheck();			// L3 Routing Neighbor Check
	void rpCloudNeighborCheck();	// Verbindungen zu Wolken eintragen -> Internet, MPLS, unbekannte Netzwerkteile...
	void rpIpsecPeerCheck();		// IPSec - Crypto Map Peers eintragen
	void schreibeLog(				// Funktion f�r die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text f�r den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void setSwitchDevType();		// Setzen vom richtigen Switch Type (L2, L3, Root usw)
	void insertRControl();			// rControl Tabelle bef�llen

	void dbgNachbarAusgabe();		// Nachbarschaftsbeziehungen im Debug Mode ausgeben
	void dbgAsgbe1(					// Debug Ausgabe f�r SQL Aufrufe und Ergebnisse
		std::string dgbAusgabe,			// Debug String
		int type,						// Global oder untergeordnet: 1: Global; 2: Untergeordnet
		int anzahlWerte,				// Anzahl der R�ckgabewerte bei Ergebnis
		std::string position			// Beschreibung der Position
		);
	void dbgNachbarAusgabeRP();		// Nachbarschaftsbeziehungen im Debug Mode ausgeben f�r L3 RP Nachbarschaften

	bool nidEmpty;					// rc_id leer?
	bool importHosts;				// Endger�te in device Table importieren?
	bool reverseDns;				// Reverse DNS Lookup f�r Endger�te
	int dbgGlobalCounter;			// Counter f�r globale Dbg1 Ausgabe
	int dbgUntergeordnetCounter;	// Counter f�r untergeordnete Dbg1 Ausgabe


public:
	WkmCombiner2(					// Konstruktor
		WkmDB *db,						// Datenbank
		WkLog *logA,					// Log Ausgabe
		WkmParserDB *pdb,				// Parser DB Operation
		bool impHosts,					// Hosts Importieren?
		bool revDns						// Reverse DNS f�r Endger�te
		);
	~WkmCombiner2();					// Destrukor
	bool doIt();					// Start
	bool debugAusgabe;
	bool debugAusgabe1;				// Detaillierte Debug Ausgabe f�r SQL Abfragen
	volatile bool stop;				// TRUE, wenn Cancel

};

#endif //#ifdef WKTOOLS_MAPPER
