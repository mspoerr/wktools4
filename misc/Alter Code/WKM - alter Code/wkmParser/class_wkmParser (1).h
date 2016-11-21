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

class WkmParser
{
private:
	WkmDB *dieDB;						// Datenbank f�r die Eintr�ge
	WkLog *logAusgabe;					// Log Ausgabefenster
	std::string suchVz;					// Verzeichnis, das durchsucht werden soll

	std::queue<fs::path> files;				// Files, die nach IPs durchsucht werden sollen
	bool sdir(							// Verzeichnis samt Unterverzeichnisse durchsuchen
		fs::path pfad						// Pfad, der durchsucht werden soll
		);
	//void hostSchreiber();				// Zum Eintragen der Endger�te in die device Tabelle
	void markNeighbor();				// Zum Markieren der neighbor Eintr�ge mit eigenen MAC und IP Adressen
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
	void iosCDPSchreiber(				// CDP Infos am Schluss noch in die Device und Interface Tabellen schreiben
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

	bool rControlEmpty;					// rControl Table leer -> Ja/Nein

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
		WkmDB *db						// Datenbank
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

	void ipNetzeEintragen(					// Netzwerkadresse vom Interface rausfinden und in die richtige Tabelle eintragen
		std::string intfip,						// IP Adresse
		std::string intfmask,					// Maske
		std::string intf_id						// intf_id vom Interface an dem das Subnet angeschlossen ist
		);
	std::string nextHopIntfCheck(			// Next Hop Interface-ID herausfinden, wenn es in der Routingtabelle nicht angegeben wird.
		std::string ipa							// IP Adresse
		);
	std::string maskToBits(					// Subnetmask in Bits umwandeln (/xx)
		std::string subnetmask					// Subnetmask
		);

	
	std::string intfNameChange(				// Zum Angleichen der Interfaces, da bei show CAM und show interfaces unterschiedliche Namen verwendet werden
		std::string intfName					// Interface zum angleichen
		);
	std::string intfTypeCheck(				// Zum Rausfinden des Interface Typs
		std::string intfName,					// Interfacename
		std::string intfSpeed					// Link Speed
		);
	std::string intfPhlCheck(				// Zum Rausfinden ob physisch (1), oder logisch (NULL, 0)
		std::string intfName,					// Interfacename
		std::string intfSpeed					// Link Speed
		);
	std::string macAddChange(				// MAC Adressen Umwandler f�r IP Phones von ABCDEFGHIJKL in das Format xxxx.yyyy.zzzz
		std::string macAdd						// zu konvertierende MAC Adresse
		);

	std::string insertDevice(				// Neues Ger�t in die DB schreiben
		std::string hName,						// Hostname
		std::string hType,						// Type
		std::string hHwtype,					// HW Type
		std::string hConfReg					// Config Register
		);				
	std::string insertHardware(					// Hardware Infos f�r Ger�t oder Modul schreiben (hwInfo und hwLink Tabellen)
		std::string hPos,						// Position im Chassis
		std::string hModell,					// Modell
		std::string hDescription,				// Beschreibung
		std::string hChassisSN,					// S/N
		std::string hRevision,					// Revision Version
		std::string hMem,						// Memory
		std::string hBootfile,					// Bootfile
		std::string hVersion,					// SW Version
		std::string dev_id,						// dev_id
		std::string boxHwInfId					// hwInfo_id von der Box
		);					
	void updateHardware(					// Hardware Infos f�r Ger�t oder Modul updaten (hwInfoTabelle)
		std::string hModell,					// Modell
		std::string hDescription,				// Beschreibung
		std::string hChassisSN,					// S/N
		std::string hRevision,					// Revision Version
		std::string hMem,						// Memory
		std::string hBootfile,					// Bootfile
		std::string hVersion,					// SW Version
		std::string hwInfo_id					// hwInfo_id
		);					
	void insertRControl();					// rControl Tabelle bef�llen
	bool insertLic(							// Lic Tabelle bef�llen
		std::string lName,						// Lizenz Name
		std::string lStatus,					// Status
		std::string lType,						// Type
		std::string lNextBoot,					// Verwendete Lic beim n�chsten Boot
		std::string lCluster,					// Cluster Lics
		std::string dev_id						// dev_id
		);						
	void insertSTPStatus(					// STP Status Tabelle bef�llen
		std::string dev_id,						// dev_id
		std::string intfName,					// Interface Name
		std::string stpIntfStatus,				// STP Port-Status
		std::string designatedRootID,			// Root ID vom Designated Port
		std::string designatedBridgeID,			// Bridge ID vom Designated Port
		std::string stpTransitionCount,			// STP Transition Count
		std::string vlan,						// VLAN ID
		bool intVlan							// intVlan Tabelle auch gleich bef�llen?
		);
	void insertVlan(						// Vlan Tabelle bef�llen
		std::string vlanNr,						// Vlan Nummer
		std::string device_id,					// Device ID
		std::string stpInstance					// STP Instanz
		);	
	std::string insertSTPInstance(			// STP Instanz Tabelle bef�llen; Return Value ist stpInstanzID
		std::string stpPrio,					// STP Priority
		std::string stpRootPort
		);
	void insertNeighbor(					// neighbor Tabelle bef�llen
		std::string intf_id,					// Interface ID
		std::string macAdd,						// MAC Adresse
		std::string ipAddr,						// IP Adresse
		std::string self						// Marker, ob eigene IP
		);
	std::string getIntfID(					// Auslesen der Interface ID
		std::string dev_id,						// Device ID
		std::string interfaceName,				// Interface Name
		bool error = true						// Fehlerausgabe?
		);
	std::string insertVrf(					// vrf Tabelle bef�llen - R�ckgabe vrf_id
		std::string vrfName,					// VRF Name
		std::string rd,							// RD
		std::string	rtI,						// Import RT
		std::string rtE							// Export RT
		);
	void insertVrfLink(						// vrfLink Tabelle bef�llen
		std::string intfName,					// InterfaceName
		std::string dev_id,						// dev_id
		std::string vrf_id						// vrf_id
		);
	void insertFlash(						// flash Tabelle bef�llen
		std::string fname,						// Name der Partition
		std::string fsize,						// Gr��e
		std::string ffree,						// Freier Speicher
		std::string dev_id						// dev_id
		);
	void insertRoute(						// l3routes Tabelle bef�llen
		std::string prot,						// Routing Protokoll
		std::string ip,							// Netzwerkadresse
		std::string prefix,						// Subnet Mask
		std::string nh,							// Next Hop
		std::string intf,						// Interface
		std::string type,						// Type
		std::string dev_id						// Device ID
		);
	void insertCDP(							// cdp Tabelle bef�llen
		std::string hostname,					// Hostname
		std::string nName,						// Neighbor Hostname
		std::string alternatenName,				// Alternate Neighbor Hostname
		std::string intf,						// Interface
		std::string nIntfIP,					// Neighbor IP
		std::string nIntf,						// Neighbor Interface 
		std::string type,						// Neighbor Type
		std::string platform,					// Neighbor HW Platform
		std::string sw_version,					// Neighbor SW Version
		std::string dev_id						// dev_id
		);
	std::string getIntfNameFromNameif(		// Interface Name vom Nameif zur�ckliefern
		std::string nameif,						// nameif
		std::string dev_id						// dev_id
		);
	std::string getIntfIDFromNameif(		// Auslesen der Interface ID f�r einen nameif
		std::string dev_id,						// Device ID
		std::string interfaceName,				// Nameif Name
		bool error = true						// Fehlerausgabe?
		);
	void insertPhone(						// IpPhoneDet Tabelle bef�llen
		std::string voiceVlan,					// Voice VLAN
		std::string cm1,						// CallManager 1 IP
		std::string cm2,						// CallManager 2 IP
		std::string dscpSig,					// DSCP for Signalling
		std::string dscpConf,					// DSCP for Config
		std::string dscpCall,					// DSCP for RDP
		std::string defGW,						// Default Gateway
		std::string dev_id						// Device ID
		);
	void updateSTP(							// Interface und Device Tabellen mit STP Infos updaten
		std::string stpBridgeID,				// STP Bridge ID
		std::string stpProtocol,				// STP Protokoll
		std::string dev_id						// Device ID
		);
	void updateChannelMember(				// Intf-ID vom Channel Interface bei allen Member Interfaces einf�gen
		std::string ch_intf_id,					// intf_id vom Channel Interface
		std::string chMem,						// String mit den Interface Namen von den physikalischen Interfaces
		std::string dev_id						// Device ID dev_id
		);
	void updateSubInterfaces(				// Intf-ID vom Hauptinterface am SubInterface einf�gen (channel_intf_id)
		std::string intf_id,					// Interface-ID vom Sub-Interface
		std::string mIntfName,					// Interface Name vom Hauptinterface
		std::string dev_id						// Device ID dev_id
		);
	void markVSL(							// VSL Interfaces markieren
		std::string intf_id						// Zu markierendes Interface; Wenn leer, dann den Channel markieren
		);
	void markVPC(							// vPC Informationen eintragen
		std::string dev_id,						// dev_id
		std::string vpcDomId,					// vPC Domain ID
		std::string vpcLink,					// Interface Name vom vPC Peer Link
		std::string vpcId						// vPC ID vom Interface
		);
	void markVethMember(					// Die BoundTo Members in der DB updaten -> Am Logischen Vethernet Interface das physikalische Ethernet Interface hinterlegen (ID)
		std::string dev_id,						// dev_id
		std::string boundToIntf,				// Interface Name vom HW Interface, an das das Veth gebunden ist
		std::string intf_id						// intf_id vom Veth Interface
		);
	void updateIntfL2Info(					// Interface Switchport Informationen updaten
		std::string l2AdminMode,				// L2 Admin Mode
		std::string l2Mode,						// L2 Operational Mode
		std::string l2encap,					// L2 Encapsulation
		std::string dtp,						// DTP Status
		std::string nativeAccess,				// Native und Access Vlan Id (Je nach Operational Mode)
		std::string voiceVlan,					// Voice Vlan ID
		std::string opPrivVlan,					// Private VLAN Status
		std::string allowedVlan,				// Allowed VLANs (Trunk)
		std::string intf_id						// intf_id
		);
	void insertSnmp(						// SNMP Location Information nachtragen
		std::string snmpLoc,					// SNMP Location
		std::string dev_id						// dev_id
		);
	void insertPoeDev(						// Ger�te-bezogene POE Infos einf�gen
		std::string poeStatus,					// POE Status
		std::string poeCurrent,					// aktueller POE Verbrauch
		std::string poeMax,						// Maximaler POE Output
		std::string poeRemaining,				// Verf�gbare POE Leistung
		std::string dev_id						// dev_id
		);
	void insertPoeIntf(						// Interface-bezogene POE Infos einf�gen
		std::string poeStatus,					// POE Status
		std::string poeWatt,					// aktueller POE Verbrauch
		std::string poeWattmax,					// Maximaler POE Output
		std::string poeDevice,					// POE Endger�t
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	std::string insertInterface(			// Interface einf�gen - Globale Informationen; intf_id wird zur�ckgegeben
		std::string intfName,					// Interface Name (normalized)
		std::string nameif,						// ASA nameif
		std::string intfType,					// Interface Type (normalized - intfTypeCheck() )
		std::string phl,						// logisch oder physikalisch (normalized - intfPhlCheck() )
		std::string macAddress,					// MAC Adresse
		std::string ipAddress,					// IP Adresse
		std::string subnetMask,					// IP Subnet Mask
		std::string duplex,						// Duplex
		std::string speed,						// Speed
		std::string status,						// Interface Status (up, down, shut...)
		std::string description,				// Description
		std::string l2l3,						// L2 oder L3
		std::string dev_id						// dev_id
		);
	void insertIntfStats(					// Interface Statistik und Error Counters einf�gen
		std::string errLvl,						// Error Level
		std::string lastClear,					// "clear interfaces" Timestamp
		std::string loadLvl,					// Load Level
		std::string iCrc,						// CRC Fehler
		std::string iFrame,						// Frame Fehler
		std::string iOverrun,					// Overruns
		std::string iIgnored,					// Ignored
		std::string iWatchdog,					// Watchdog
		std::string iPause,						// Pause
		std::string iDribbleCondition,			// Dribble Condition
		std::string ibuffer,					// Buffer Fehler
		std::string l2decodeDrops,				// L2 Decode Drops (ASA)
		std::string runts,						// Runts
		std::string giants,						// Giants
		std::string throttles,					// Throttles
		std::string ierrors,					// Input Errors
		std::string underrun,					// Underruns
		std::string oerrors,					// Output Errors
		std::string oCollisions,				// Output Collissions
		std::string oBabbles,					// Babbles
		std::string oLateColl,					// Late Collissions
		std::string oDeferred,					// Deferred
		std::string oLostCarrier,				// Lost Carrier
		std::string oNoCarrier,					// No Carrier
		std::string oPauseOutput,				// Output Pause Frames
		std::string oBufferSwapped,				// Buffer Swapped
		std::string resets,						// Resets
		std::string obuffer,					// Output Buffer
		std::string lastInput,					// Last Input Timestamp
		std::string intf_id						// intf_id
		);
	void intertIntfN1k(						// Nexus1k Interface Infos einf�gen (Veth Interfaces)
		std::string actModule,					// Active on Module -> Virtual Ethernet Module # auf der das Veth aktiv ist
		std::string owner,						// Owner (VM, der das Interface zugeordnet ist)
		std::string portProfile,				// Port Profile -> VMWare Port Profile
		std::string intf_id						// intf_id
		);

};		

#endif // #ifdef WKTOOLS_MAPPER
