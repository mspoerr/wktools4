#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER
#include "class_wkmdb.h"
#include "class_wkLog.h"
#include "class_wkmVisioAusgabe.h"

#include <string>
#include <iostream>
#include <fstream>

typedef unsigned int uint;

class WkmAusgabe
{
private:
	WkmDB *dieDB;				// Datenbank für die Einträge
	WkLog *logAusgabe;			// Logsuagabe Fenster
	WkmVisioAusgabe *vdxA;		// Visio Ausgabe
	std::string ausgabe;		// Ausgabe
	std::string nodes;			// Nodes in GML Format
	std::string edges;			// Edges in GML Format
	std::string clusters;		// Cluster im GML Format
	std::string devType;		// Device Type

	bool cdp;					// CDP Infos anzeigen
	bool cdp_hosts;				// Endgeräte, die über CDP gelernt wurden, anzeigen
	bool unknownSTP;			// Unbekannte STP Nachbaren anzeigen
	bool unknownRP;				// Unbekannte RP Nachbaren anzeigen
	bool l2;					// L2 Ausgabe
	bool l3;					// L3 Ausgabe
	bool l3routing;				// L3 Routing Ausgabe
	bool hier;					// Hierarchisches Layout
	bool org;					// Organisches Layout
	bool ortho;					// Orthogonal Layout
	bool endsystems;			// EndSystems
	int intfStyle;				// Interface Style
	std::string uel;			// Unit Edge Length bei Organic Layout
	std::string ld;				// Layer Distance bei Hierarchisches Layout
	std::string nd;				// Node Distance bei Hierarchisches Layout
	std::string wb;				// Weight Balancing bei Hierarchisches Layout

	std::string ausgabeVz;		// Ausgabeverzeichnis

	bool neighborEmpty;			// Nachbarschaftstabelle leer?

	std::string setDevType(			// GML Type für das richtige Device zurückliefern
		std::string devT					// Device Type (Return von der DB)
		);
	int doItL2(					// L2 Parser starten
		bool perVLAN				// Global oder per VLAN Ausgabe
		);	
	int doItL3(					// L3 Parser starten
		);	
	int doItL3Routing(			// L3 Routing Parser starten
		);
	void report(				// Diff über die letzten beiden Auswertungen
		int auswertung = 0			// For future use: Mit welcher Auswertung soll das Diff gemacht werden?
		);
	void reportDiff(			// Topologie Diff Report
		std::string dbtable			// Für welche Tabelle soll der Diff gemacht werden? rpneighborship, neighborship
		);			
	void reportIntfErr();		// Interface Fehler und Detaillierter Interface Fehler Report
	void reportSTP();			// Alle L2/STP bezogenen Reports
	void reportInventory();		// HW Inventory Report
	void reportEndSystems();	// Endgeräte Report
	void reportIPT();			// Telefon Report
	void reportL2Devs();		// Report für STP sprechende Geräte
	void reportL2();			// Report für Status der Switchports
	void reportSTPStat();		// Alter Report für die STP Protokoll Übersicht
	void reportL3ospf();		// OSPF Detail Report
	void reportL3RP();			// Routing Protokoll Übersicht
	void reportL3RPneighbors();	// Routing Protokoll Neighbors Übersicht
	void reportDot1xIntfStat();	// 802.1X Interface Status -> Aktiviert Ja/Nein?
	void reportDot1xFailed();	// 802.1X Failed Authentications Report
	void schreibeLog(						// Funktion für die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	int zeichneGraph(			// Zeichne den Graph und gib ein GML File aus
		std::string filename				// Namenszusatz für das Ausgabefile
		);				

	void markiereEintraege();		// Markiere die Einträge, die nicht ausgegeben werden sollen

public:
	WkmAusgabe(							// Konstruktor
		WkmDB *db,							// Datenbank
		WkLog *logA,						// Log Fenster
		std::string asgbe					// Ausgabe 
		);
	~WkmAusgabe();						// Destruktor

	int doIt(							// Parser starten
		);	
	void sets(					// Zum Setzen der Einstellungen für die Ausgabe
		bool cdp,					// CDP Infos anzeigen
		bool cdp_hosts,				// Endgeräte, die über CDP gelernt wurden, anzeigen
		bool unknownSTP,			// Unbekannte STP Nachbaren
		bool unknownRP,				// Unbekannte RP Nachbaren
		bool l2,					// L2 Ausgabe
		bool l3,					// L3 Ausgabe
		bool l3routing,				// L3 Routing Ausgabe
		bool org,					// Hierarchisches Layout
		bool hier,					// Organisches Layout
		bool ortho,					// Orthogonal Layout
		std::string unitEdgeLength,	// Unit Edge Length bei Organic Layout
		std::string lDistance,		// Layer Distance bei Hierarchisches Layout
		std::string nDistance,		// Node Distance bei Hierarchisches Layout
		std::string weightBalancing,// Weight Balancing bei Hierarchisches Layout
		bool endsystems,			// Endgeräte
		int intfStyle				// Interface Style
		);

};

#endif // #ifdef WKTOOLS_MAPPER
