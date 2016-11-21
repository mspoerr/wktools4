#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER
#include "class_wkLog.h"
#include "class_wkmParserDB.h"

#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;

typedef unsigned int uint;

class WkmParser
{
private:
	WkLog *logAusgabe;					// Log Ausgabefenster
	WkmParserDB *parserdb;				// Parser DB Operation
	WkmDB *dieDB;						// Die Datenbank
	std::string suchVz;					// Verzeichnis, das durchsucht werden soll

	std::queue<fs::path> files;				// Files, die nach IPs durchsucht werden sollen
	bool sdir(							// Verzeichnis samt Unterverzeichnisse durchsuchen
		fs::path pfad						// Pfad, der durchsucht werden soll
		);
	//void hostSchreiber();				// Zum Eintragen der Endger�te in die device Tabelle
	void varsInit();					// zum Initialisieren der Variablen
	void testIOS(						// ausgelagerte Funbktion zum Testen, ob Router oder Switch und zum Auslesen der show version Daten
		);
	void testPixOS(						// ausgelagerte Funktion zum Testen von PIXOS
		);
	void testSW(						// ausgelagerte Funktion zum Testen von IOS Switches
		);
	void testRouter(					// ausgelagerte Funktion zum Testen von IOS Routern
		);
	void testPhones(					// ausgelagerte Funktion zum Testen der IP Phone Daten
		);
	void iosInterfaceParser(			// zum Parsen der Interface Infos von IOS Boxen
		);
	void iosSTPParser(					// zum Parsen der STP Infos von IOS Boxen
		);
	void iosARPParser(					// zum Parsen der ARP Infos von IOS Boxen
		);
	void iosCAMParser(					// zum Parsen der CAM Table Infos von IOS Boxen
		);
	void iosSNMPParser(					// zum Parsen der SNMP Location Einstellungen
		);
	void iosSwitchportParser(			// zum Parsen der "show interface switchport" Infos von IOS Boxen
		);
	void iosModuleParser(				// zum Parsen von show module und den STakcinfos bei show version
		);
	void iosHWParser(					// zum Parsen von "show c7200" und "show diag"
		);
	void iosInventoryParser(			// zum Parsen von "show inventory"
		);	
	void iosShowHardwareParser(			// zum Parsen von "show hardware"
		);
	void iosFileSystemParser(			// Zum Parsen von "show file system"
		);
	void iosPoeParser(					// Zum Parsen von "show power inline"
		);
	void iosVrfParser(					// IOS VRF Parser
		);
	void testNexus(						// ausgelagerte Funktion zum Testen von NXOS Boxen
		);
	void nxosInterfaceParser(			// zum Parsen der Interface Infos von NXOS Boxen
		);
	void nxosSTPParser(					// zum Parsen der STP Infos von NXOS Boxen
		);
	void nxosARPParser(					// zum Parsen der ARP Infos von NXOS Boxen
		);
	void nxosCAMParser(					// zum Parsen der CAM Table Infos von NXOS Boxen
		);
	void nxosCDPParser(					// zum Parsen der CDP Infos von NXOS Boxen
		);
	void nxosVPCParser(					// zum Parsen der vPC Infos von NXOS Boxen
		);
	void nxosSNMPParser(				// zum Parsen der SNMP Infos von NXOS Boxen
		);
	void nxosInventoryParser(			// NXOS Hardware Parser (show Inventory)
		);
	void nxosVrfParser(					// NXOS VRF Parser
		);
	void nxosRouteParser(				// NXOS "show ip route" Parser
		);
	void testCatOS(						// CatOS Tester
		);
	void catOSHWParser(					// CatOS Hardware Parser
		);
	void pixOSInterfaceParser(			// zum Parsen der Interface Infos von PIX und ASA Boxen
		);
	void pixOSARPParser(				// zum Parsen der ARP Infos von PIX und ASA Boxen
		);
	void pixOSrouteParser(				// PIX und ASA show route Parser
		);
	void pixOSOSPFParser(				// PIX und ASA show ospf neigh Parser
		);
	void pixOSEIGRPParser(				// PIX und ASA show eigrp neigh Parser
		);
	void pixOSHWParser(					// PIX und ASA Hardware Parser
		);
	void pixOSFilesystemParser(			// show file system Parser
		);
	void pixOSLicParser(				// PIX License Info Parser
		);
	void ipsecParser(					// show crypto ipsec sa Parser
		);
	void iosCDPParser(					// zum Parsen der CDP Infos von IOS Boxen
		);
	void iosRouteParser(				// IOS show ip route Parser
		);
	void iosOSPFParser(					// IOS show ip ospf neigh
		);
	void iosEIGRPParser(				// IOS show ip eigrp neigh
		);
	void iosBGPParser(					// IOS show ip bgp summary Parser
		);
	void iosVSLParser(					// IOS VSS VSL Parser
		);
	void switchLicParser(				// IOS Swtich Lizenz Parser
		);
	void routerLicParser(				// IOS Router Lizenz Parser
		);


	std::string zeitStempel(						// Zeitstempel eintragen
		std::string info								// zus�tzliche Information
		);
	void schreibeLog(						// Funktion f�r die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text f�r den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);

	//bool sdirLua();						// Einlesen aller .lua Files imwktools Verzeichnis


	std::string dateiname;					// gerade bearbeitete Datei

	// Device Table
	int devicetype;							// Ger�tetype bzw Kategorie -> Switch, Router, Firewall usw.
	std::string hostname;					// Hostname
	std::string version;					// SW Version 
	std::string modell;						// Ger�tetype
	std::string prozessor;					// Prozessortype
	std::string stpBridgeID;				// STP Bridge ID
	std::string stpProtocol;				// STP Protokoll
	std::string devType;					// Ger�tekategorie (gleich wie "int devicetype", nur in std::string Format)
	std::string intf_id;					// Interface ID in der DB f�r das aktuelle Interface
	std::string chassisSN;					// Chassis S/N
	std::string confReg;					// Config Register
	std::string hwRevision;					// Hardware Revision
	std::string hwPos;						// Position (Modul#, PS, Box usw)
	std::string hwDescription;				// Modell Beschreibung
	std::string hwMem;						// Speicherinfo (RAM)
	std::string hwFlash;					// Speicherinfo (Flash)
	std::string hwBootfile;					// Bootfile

	std::string licName;					// Lic Name
	std::string licStatus;					// Lic Status (Anzahl, Enabled/Disabled/Unlimited usw)
	std::string licType;					// Perpetual, Permanent usw...
	std::string licCluster;					// Zeigt an, ob Cluster oder Box Lizenz
	std::string licNextBoot;				// Lizenz beim n�chsten Boot

	std::string vpcPeerLink;				// Name des VPC Peer Link Interfaces

	std::queue<std::string> osmNr;			// OSM Modulnummer
	std::queue<std::string> osmMem;			// OSM Memory


	bool nodiag;							// diag Infos?
	bool c72er;								// 7200 Router?
	bool gsr;								// GSR?

	//bool importHosts;						// Endger�te in device Table importieren?
	//bool reverseDns;						// Reverse DNS Lookup f�r Endger�te

	std::string dev_id;						// Device ID in der Datenbank f�r das aktuelle Ger�t
	std::string hwInfo_id;					// hwInfo_id vom Chassis

	std::string fehlerString;				// String mit allen Parser Fehlern

	std::string ganzeDatei;					// eingelesene Daten

	// folgende size_t's sind zum Auffinden der wichtigsten show Befehle
	std::size_t pos1;						// HauptPos1
	std::size_t pos2;						// HauptPos2
	std::size_t pos3;						// ...
	std::size_t pos4;
	std::size_t pos5;
	std::size_t pos6;
	std::size_t pos7;
	std::size_t pos8;
	std::size_t pos9;
	std::size_t pos10;
	std::size_t pos11;
	std::size_t pos12;
	std::size_t pos13;
	std::size_t pos14;
	std::size_t pos15;
	std::size_t pos16;
	std::size_t pos17;
	std::size_t pos18;
	std::size_t pos19;
	std::size_t pos20;
	std::size_t pos21;
	std::size_t pos22;
	std::size_t pos23;
	std::size_t pos24;
	std::size_t pos25;
	std::size_t pos26;
	std::size_t pos27;
	std::size_t pos28;
	std::size_t pos29;
	std::size_t pos30;
	std::size_t pos31;
	std::size_t pos32;

	std::size_t aktPos1;
	std::size_t aktPos2;

	std::size_t pos1iph;					// IP Phone Pos
	std::size_t pos2iph;					// IP Phone Pos
	std::size_t pos3iph;					// IP Phone Pos

	std::size_t stackpos;					// Position, wo etwaige Stack Infos anfangen w�rden

	std::map <std::string, std::string> luaScripts;								// Map mit den eingelesenen Lua Scripts
	std::map <std::string, std::string>::iterator lsIter, lsAnfang, lsEnde;		// Map Iteratoren
	typedef std::pair <std::string, std::string> pss;							// Pair zum Einf�gen in die Map

public:
	bool debugAusgabe;					// Debug Ausgabe f�r SQL Ausgabe
	bool debugAusgabe1;					// Detaillierte Debug Ausgabe f�r SQL Abfragen
	volatile bool stop;					// TRUE, wenn Cancel
	WkmParser(							// Konstruktor
		std::string suchPfad,			// Pfad, der durchsucht werden soll
		WkLog *logA,					// Log Ausgabe
		WkmDB *dieDB,					// DB Objekt
		WkmParserDB *parserdb			// Paser DB Operation
		);
	~WkmParser();						// Destruktor

	int startParser(					// Parser starten
		std::string pattern				// Search Pattern
		//bool impHosts,						// Hosts Importieren?
		//bool revDns							// Reverse DNS f�r Endger�te
		);	
	enum DEVTYPE {
		UNKNOWN,
		ROUTER,
		SWITCH,
		FIREWALL,
		ACCESSPOINT,
		VOICEGATEWAY,
		ROUTER_SW,
		PHONE,
		SERVER,
		CLIENT,
		GENERIC_ENDPOINT,
		CLOUD
	};

	std::string fehlerRet(					// r�ckgabe vom Fehlerstring
		);
};		

#endif // #ifdef WKTOOLS_MAPPER
