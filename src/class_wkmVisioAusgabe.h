#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER
#include <string>
#include <iostream>
#include <map>
#include <fstream>

#include "class_wkmdb.h"


typedef unsigned int uint;

class WkmVisioAusgabe
{
private:
	WkmDB *dieDB;					// Datenbank für die Einträge

	std::string gmlInput;			// GML Source Organic
	std::string aDatName;			// Ausgabefile Namen
	std::string pageID;				// Page ID
	std::string pageName;			// Page Name

	std::map<std::string, std::string> interfaces;			// Sammlung aller Routing-Protokoll-Enabled Interface IDs, auf denen die Nachbarschaft nicht FULL/UP/... ist
	std::map<std::string, std::string> cryptoInterfaces;	// Sammlung aller Crypto Map Interfaces
	typedef std::pair <std::string, std::string> pss;		// Pair für das Einfügen in die Map

	bool intfErr;					// Interface Errors
	bool intfDuplex;				// Interface Duplex
	bool intfStp;					// Interface STP Probleme
	bool intfLoad;					// Interface Load 

	int pId;						// Page ID
	int nodeID;						// Node ID für die Sub Nodes

	int intfStyle;					// Interface Style
	int type;						// Ausgabe Type (Zusatzinfos für L3 Routing ja/nein)

	std::ofstream vdxAusgabe;		// vdx Ausgabe

	std::string farbeNodeUmsetzer(	// Farbe auf Node umsetzen
		std::string farbe,				// Farbe
		std::string id,					// ID
		std::string x,					// X Koordinate
		std::string y,					// Y Koordinate
		std::string label
		);

	void insertL2Coord(				// Interface Koordinaten in DB eintragen
		std::string x,					// X Koordinate
		std::string y,					// Y Koordinate
		std::string id,					// Intf-ID
		bool device						// Wenn true -> device; Bei false Interface
		);
	void insertL3Coord(				// Interface Koordinaten in DB eintragen
		std::string x,					// X Koordinate
		std::string y,					// Y Koordinate
		std::string id,					// Intf-ID
		bool device						// Wenn true -> device; Bei false Interface
		);
	void insertL3RPCoord(				// Interface Koordinaten in DB eintragen
		std::string x,					// X Koordinate
		std::string y,					// Y Koordinate
		std::string id,					// Intf-ID
		bool device						// Wenn true -> device; Bei false Interface
		);
	void insertIntfColor(				// Interface Farbe, die von wkmAusgabe mitgegeben wird, in DB eintragen
		std::string farbe,					// Farbe
		std::string id						// Intf-ID
		);

	std::string shapeData(			// Shape Data in die Visio Ausgabe übertragen
		std::string nodeName,			// Name für den Node (Interface oder Gerät) für den die Shape Data eingetragen werden soll
		std::string type				// L2 oder L3
		);
	std::string shapeDataCloud(		// Shape Data in die Visio Ausgabe übertragen
		std::string nodeName,			// Name für den Node (Cloud) für den die Shape Data eingetragen werden soll
		std::string type				// for future use
		);
	std::string shapeDataIntf(		// Shape Data in die Visio Ausgabe übertragen
		std::string nodeName,			// Name für den Node (Interface oder Gerät) für den die Shape Data eingetragen werden soll
		std::string type,				// L2 oder L3
		std::string intfID,				// InterfaceID
		std::string id					// Visio Node ID
		);

public:
	WkmVisioAusgabe(				// Konstruktor
		std::string aDat,				// Ausgabedatei
		WkmDB *db						// Datenbank

		);
	~WkmVisioAusgabe();				// Destruktor

	void doIt(						// Starte umwandeln
		std::string gmlFile,			// Organic GML File zum Einlesen (als String, nicht Filename)
		std::string page,				// Seitenname
		int intfStyle,					// Interface Style
		int type						// Ausgabe Type (L2, L3, L3RP)
		);

	enum INTERFACE_STYLE {
		WKM_AUSGABE_INTF_STYLE_NORMAL = 0,
		WKM_AUSGABE_INTF_STYLE_REDUCED,
		WKM_AUSGABE_INTF_STYLE_REDUCED_NOLINE
	};

};

#endif // #ifdef WKTOOLS_MAPPER
