#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER
#include "class_wkmdb.h"
#include "class_wkLog.h"

#include <string>
#include <iostream>
#include <fstream>
#include <queue>

typedef unsigned int uint;

class WkmParserDB
{
private:
	WkmDB *dieDB;						// Datenbank für die Einträge
	WkLog *logAusgabe;					// Log Ausgabefenster
	std::string fehlerString;			// String mit allen Parser Fehlern
	bool rControlEmpty;					// rControl Table leer -> Ja/Nein

public:
	std::string dateiname;				// gerade bearbeitete Datei
	bool debugAusgabe;					// Debug Ausgabe für SQL Ausgabe
	bool debugAusgabe1;					// Detaillierte Debug Ausgabe für SQL Abfragen
	WkmParserDB(						// Konstruktor
		WkLog *logA,					// Log Ausgabe
		WkmDB *dieDB					// DB Objekt
		);
	~WkmParserDB();						// Destruktor

	void markNeighbor(					// Zum Markieren der neighbor Einträge mit eigenen MAC und IP Adressen
		std::string dev_id					// dev_id
		);				

	void ipNetzeEintragen(					// Netzwerkadresse vom Interface rausfinden und in die richtige Tabelle eintragen
		std::string intfip,						// IP Adresse
		std::string intfmask,					// Maske
		std::string intf_id						// intf_id vom Interface an dem das Subnet angeschlossen ist
		);
	std::string nextHopIntfCheck(			// Next Hop Interface-ID herausfinden, wenn es in der Routingtabelle nicht angegeben wird.
		std::string ipa,						// IP Adresse
		std::string dev_id						// dev_id
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
	std::string macAddChange(				// MAC Adressen Umwandler für IP Phones von ABCDEFGHIJKL in das Format xxxx.yyyy.zzzz
		std::string macAdd						// zu konvertierende MAC Adresse
		);

	std::string insertDevice(				// Neues Gerät in die DB schreiben
		std::string hName,						// Hostname
		std::string hType,						// Type
		std::string hHwtype,					// HW Type
		std::string hConfReg					// Config Register
		);				
	std::string insertHardware(					// Hardware Infos für Gerät oder Modul schreiben (hwInfo und hwLink Tabellen)
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
	void updateHardware(					// Hardware Infos für Gerät oder Modul updaten (hwInfoTabelle)
		std::string hModell,					// Modell
		std::string hDescription,				// Beschreibung
		std::string hChassisSN,					// S/N
		std::string hRevision,					// Revision Version
		std::string hMem,						// Memory
		std::string hBootfile,					// Bootfile
		std::string hVersion,					// SW Version
		std::string hwInfo_id					// hwInfo_id
		);					
	void insertRControl();					// rControl Tabelle befüllen
	bool insertLic(							// Lic Tabelle befüllen
		std::string lName,						// Lizenz Name
		std::string lStatus,					// Status
		std::string lType,						// Type
		std::string lNextBoot,					// Verwendete Lic beim nächsten Boot
		std::string lCluster,					// Cluster Lics
		std::string dev_id						// dev_id
		);						
	void insertSTPStatus(					// STP Status Tabelle befüllen
		std::string dev_id,						// dev_id
		std::string intfName,					// Interface Name
		std::string stpIntfStatus,				// STP Port-Status
		std::string designatedRootID,			// Root ID vom Designated Port
		std::string designatedBridgeID,			// Bridge ID vom Designated Port
		std::string stpTransitionCount,			// STP Transition Count
		std::string vlan,						// VLAN ID
		bool intVlan							// intVlan Tabelle auch gleich befüllen?
		);
	void insertVlan(						// Vlan Tabelle befüllen
		std::string vlanNr,						// Vlan Nummer
		std::string device_id,					// Device ID
		std::string stpInstance				// STP Instanz
		);	
	std::string insertSTPInstance(			// STP Instanz Tabelle befüllen; Return Value ist stpInstanzID
		std::string stpPrio,					// STP Priority
		std::string stpRootPort
		);
	void insertNeighbor(					// neighbor Tabelle befüllen
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
	std::string insertVrf(					// vrf Tabelle befüllen - Rückgabe vrf_id
		std::string vrfName,					// VRF Name
		std::string rd,							// RD
		std::string	rtI,						// Import RT
		std::string rtE							// Export RT
		);
	void insertVrfLink(						// vrfLink Tabelle befüllen
		std::string intfName,					// InterfaceName
		std::string dev_id,						// dev_id
		std::string vrf_id						// vrf_id
		);
	void insertFlash(						// flash Tabelle befüllen
		std::string fname,						// Name der Partition
		std::string fsize,						// Größe
		std::string ffree,						// Freier Speicher
		std::string dev_id						// dev_id
		);
	void insertRoute(						// l3routes Tabelle befüllen
		std::string prot,						// Routing Protokoll
		std::string ip,							// Netzwerkadresse
		std::string prefix,						// Subnet Mask
		std::string nh,							// Next Hop
		std::string intf,						// Interface
		std::string type,						// Type
		std::string dev_id						// Device ID
		);
	void insertCDP(							// cdp Tabelle befüllen
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
	std::string getIntfNameFromNameif(		// Interface Name vom Nameif zurückliefern
		std::string nameif,						// nameif
		std::string dev_id						// dev_id
		);
	std::string getIntfIDFromNameif(		// Auslesen der Interface ID für einen nameif
		std::string dev_id,						// Device ID
		std::string interfaceName,				// Nameif Name
		bool error = true						// Fehlerausgabe?
		);
	void insertPhone(						// IpPhoneDet Tabelle befüllen
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
	void updateChannelMember(				// Intf-ID vom Channel Interface bei allen Member Interfaces einfügen
		std::string ch_intf_id,					// intf_id vom Channel Interface
		std::string chMem,						// String mit den Interface Namen von den physikalischen Interfaces
		std::string dev_id						// Device ID dev_id
		);
	void updateSubInterfaces(				// Intf-ID vom Hauptinterface am SubInterface einfügen (channel_intf_id)
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
	void insertPoeDev(						// Geräte-bezogene POE Infos einfügen
		std::string poeStatus,					// POE Status
		std::string poeCurrent,					// aktueller POE Verbrauch
		std::string poeMax,						// Maximaler POE Output
		std::string poeRemaining,				// Verfügbare POE Leistung
		std::string dev_id						// dev_id
		);
	void insertPoeIntf(						// Interface-bezogene POE Infos einfügen
		std::string poeStatus,					// POE Status
		std::string poeWatt,					// aktueller POE Verbrauch
		std::string poeWattmax,					// Maximaler POE Output
		std::string poeDevice,					// POE Endgerät
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	std::string insertInterface(			// Interface einfügen - Globale Informationen; intf_id wird zurückgegeben
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
	void insertIntfStats(					// Interface Statistik und Error Counters einfügen
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
	void intertIntfN1k(						// Nexus1k Interface Infos einfügen (Veth Interfaces)
		std::string actModule,					// Active on Module -> Virtual Ethernet Module # auf der das Veth aktiv ist
		std::string owner,						// Owner (VM, der das Interface zugeordnet ist)
		std::string portProfile,				// Port Profile -> VMWare Port Profile
		std::string intf_id						// intf_id
		);
	std::string zeitStempel(						// Zeitstempel eintragen
		std::string info								// zusätzliche Information
		);
	void schreibeLog(						// Funktion für die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void iosCDPSchreiber(				// CDP Infos am Schluss noch in die Device und Interface Tabellen schreiben
		);
	void rControlFlag();				// rControlEmpty Flag rausfinden
};		

#endif // #ifdef WKTOOLS_MAPPER
