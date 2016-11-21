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
	std::string ganzeDatei;				// Dateiinhalt der gerade bearbeiteten Datei
	bool rControlEmpty;					// rControl Table leer -> Ja/Nein

public:
	std::string dateiname;				// gerade bearbeitete Datei
	bool debugAusgabe;					// Debug Ausgabe für SQL Ausgabe
	bool debugAusgabe1;					// Detaillierte Debug Ausgabe für SQL Abfragen

	struct cryptoSets
	{
		std::string intfName;			// Interface Name (normalized)
		std::string dev_id;				// Device ID
		std::string peer;				// Peer IP Address
		std::string local_net;			// Local Crypto Network
		std::string remote_net;			// Remote Crypto Network
		std::string vrf;				// VRF
		std::string pfs_grp;			// PFS DH Group
		std::string pkt_encaps;			// Number of encapsulated packets
		std::string pkt_decaps;			// Number of decapsulated packets
		std::string pkt_encrypt;		// Number of encrypted packets
		std::string pkt_decrypt;		// Number of decrypted packets
		std::string send_err;			// Number of send errors 
		std::string rcv_err;			// Number of receive errors
		std::string path_mtu;			// Path MTU
		std::string status;				// Status of SA
		std::string esp_i;				// Inbound ESP SA
		std::string ah_i;				// Inbound AH SA
		std::string pcp_i;				// Inbound PCP SA
		std::string esp_o;				// Outbound ESP SA
		std::string ah_o;				// Outbound AH SA
		std::string pcp_o;				// Outbound PCP SA
		std::string settings;			// Settings
		std::string tag;				// Crypto Map Tag
		std::string seqnr;				// Crypto Map Sequence Number
	};
	struct intfSets
	{
		std::string intfName;					// Interface Name (normalized)
		std::string nameif;						// ASA nameif
		std::string intfType;					// Interface Type (normalized - intfTypeCheck() )
		std::string phl;						// logisch oder physikalisch (normalized - intfPhlCheck() )
		std::string macAddress;					// MAC Adresse
		std::string ipAddress;					// IP Adresse
		std::string subnetMask;					// IP Subnet Mask
		std::string duplex;						// Duplex
		std::string speed;						// Speed
		std::string status;						// Interface Status (up, down, shut...)
		std::string description;				// Description
		std::string l2l3;						// L2 oder L3
		std::string dev_id;						// dev_id
	};
	struct intfStats
	{
		std::string errLvl;						// Error Level
		std::string lastClear;					// "clear interfaces" Timestamp
		std::string loadLvl;					// Load Level
		std::string iCrc;						// CRC Fehler
		std::string iFrame;						// Frame Fehler
		std::string iOverrun;					// Overruns
		std::string iIgnored;					// Ignored
		std::string iWatchdog;					// Watchdog
		std::string iPause;						// Pause
		std::string iDribbleCondition;			// Dribble Condition
		std::string ibuffer;					// Buffer Fehler
		std::string l2decodeDrops;				// L2 Decode Drops (ASA)
		std::string runts;						// Runts
		std::string giants;						// Giants
		std::string throttles;					// Throttles
		std::string ierrors;					// Input Errors
		std::string underrun;					// Underruns
		std::string oerrors;					// Output Errors
		std::string oCollisions;				// Output Collissions
		std::string oBabbles;					// Babbles
		std::string oLateColl;					// Late Collissions
		std::string oDeferred;					// Deferred
		std::string oLostCarrier;				// Lost Carrier
		std::string oNoCarrier;					// No Carrier
		std::string oPauseOutput;				// Output Pause Frames
		std::string oBufferSwapped;				// Buffer Swapped
		std::string resets;						// Resets
		std::string obuffer;					// Output Buffer
		std::string lastInput;					// Last Input Timestamp
		std::string intf_id;					// intf_id
	};

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
	std::string getIntfIDfromL3Route(		// Outbound Interface ID von einer L3 Route für Eingabe-IP zurückliefern
		std::string ipAddress,					// IP Adresse für die das Outbound Interface in der Routing Tabelle gesucht werden soll
		std::string dev_id						// device ID
		);
	std::string getIntfNamefromIP(			// Interface Namen aufgrund der IP Adresse zurückliefern
		std::string ipAddress,					// Interface IP Adresse
		std::string dev_id						// device ID
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
	bool intfCheck(							// Zum Checken, ob der Interface Name grundsätzlich gültig sein könnte im Falle eines Fehlers; Kommt immer wieder vor bei Interfaces, die nur interne Verwendung haben
		std::string intfName					// Interface zum angleichen
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
	std::string insertOspfProcess(			// Neuen OSPF Process in die DB schreiben
		std::string routerID,					// Router ID
		std::string processID,					// Process ID
		std::string vrf,						// vrf Name
		std::string spfExecutions,				// # SPF Executions
		std::string spfLastRun,					// SPF Last Run
		std::string dev_id						// dev_id
		);
	void insertOspfIntf(					// OSPF Interface bezogene Infos in Interfaces Tabelle schreiben
		std::string area,						// Area
		std::string nwType,						// Network Type
		std::string cost,						// ospf cost
		std::string processID,					// Process ID
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertDot1xIntf(					// 802.1X-bezogene Interface-Infos in DB schreiben
		std::string dot1x,						// dot1x Rolle
		std::string mab,						// MAB Status
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertDot1xDev(					// 802.1X-bezogene Geräte-Infos in DB schreiben
		std::string status,						// 802.1X System Auth Control Status
		std::string version,					// 802.1X Protokoll Version
		std::string dev_id						// dev_id
		);
	void insertDot1xUser(					// 802.1X-bezogene User-Infos in die DB schreiben
		std::string userMac,					// User MAC Adresse
		std::string method,						// Authentication Methode
		std::string domain,						// Authentication Domain
		std::string status,						// Authentication Status
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertDevTracking(					// IP Device Tracking Infos in die DB schreiben
		std::string macAddress,					// MAC Adresse
		std::string ipAddress,					// IP Adresse
		std::string state,						// State
		std::string vlanID,						// Vlan-ID
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertDHCPSnooping(				// IP DHCP Snooping Infos in DB schreiben
		std::string macAddress,					// MAC Adresse
		std::string ipAddress,					// IP Adresse
		std::string leaseTime,					// DHCP Lease Time
		std::string vlanID,						// Vlan-ID
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertSnoopingIntf(				// Interface-bezogene DHCP Snooping Infos einfügen
		std::string dhcpSnoopingTrust,			// DHCP Snooping Trust
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertSnoopingDev(					// Geräte-bezogene DHCP Snooping Infos einfügen
		std::string dhcpSnoopState,				// DHCP Snooping Status
		std::string dhcpSnoopVlans,				// DHCP Snooping VLANs
		std::string dhcpSnoop82,				// DHCP Option 82 Status
		std::string dev_id						// dev_id
		);
	void insertDevTrackingDev(				// Geräte-bezogene IP Device Tracking Infos einfügen
		std::string devTrackingStatus,			// Device Tracking Status
		std::string devTrackingProbeCount,		// Device Tracking Probe Count
		std::string devTrackingProbeInterval,	// Device Tracking Probe Interval
		std::string devTrackingProbeDelay,		// Device Tracking Probe Delay Interval
		std::string dev_id						// dev_id
		);
	void iosDot1xUpdater(					// Setzt alle Ports, auf denen 802.1X nicht rennt, in der DB auf "OFF"
		);
	void iosMabUpdater(						// Setzt alle Ports, auf denen MAB nicht rennt, in der DB auf "OFF"
		);
	void insertRpNeighbor(					// Routing Protocol Neighbor in DB schreiben
		std::string rp,							// Routing Protocol Name
		std::string neighborID,					// Neighbor ID
		std::string neighborIntfIP,				// Neighbor IP Address
		std::string uptime,						// Neighbor Uptime
		std::string neighborState,				// Neighbor State
		std::string neighborStateChanges,		// # Neighbor State Changes
		std::string neighborAS,					// Neighbor AS #
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertRpGlobal(					// Routing Protocol global Infos in DB schreiben
		std::string bgpAS,						// BGP AS#
		std::string dev_id						// dev_id
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
		bool intVlan,							// intVlan Tabelle auch gleich befüllen?
		std::string statsOnly="0"				// Nur für Statistikzwecke?
		);
	std::string insertVlan(					// Vlan Tabelle befüllen
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
	void insertHSRP(						// hsrp Tabelle befüllen
		std::string intfName,					// Interface Name
		std::string grp,						// Group#
		std::string priority,					// Prio
		std::string state,						// Status
		std::string active,						// Active Device
		std::string standby,					// Standby Device
		std::string virtualIP,					// HSRP Adresse
		std::string dev_id						// Device ID
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
	void insertCryptoMapIntf(				// Virtuelles Crypto Map Interface pro Peer einfügen
		std::string intfName,					// Interface Name
		std::string dev_id						// dev_id
		);
	void insertCryptoIntf(					// Interface-bezogene Crypto Infos einfügen
		std::string cryptoMapTag,				// cryptoMapTag: Crypto Map Name
		std::string cryptoLocalIP,				// cryptoLocalIP: Crypto Local IP Address
		std::string cryptoRemoteIP,				// cryptoRemoteIP: Crypto Remote IP Address
		std::string intfName,					// intfName: Interface Name
		std::string dev_id						// dev_id: Device ID
		);
	bool insertCDP(							// cdp Tabelle befüllen
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
	std::string getIntfNameFromID(			// Auslesen vom Interface Namen anhand der Intf ID
		std::string intf_id,					// intf_id: Interface ID
		bool err								// err: Soll eine Fehler ausgegeben werden?
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
		std::string mintf_id,					// Interface ID vom Hauptinterface
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
	void updateDeviceDataSource(			// Update Device Data Source ID
		std::string dev_id,						// Device ID
		std::string dataSource					// DataSource ID
		);
	void insertPoeDev(						// Geräte-bezogene POE Infos einfügen
		std::string poeStatus,					// POE Status
		std::string poeCurrent,					// aktueller POE Verbrauch
		std::string poeMax,						// Maximaler POE Output
		std::string poeRemaining,				// Verfügbare POE Leistung
		std::string dev_id						// dev_id
		);
	void insertFOStatus(						// Failover Status
		std::string foStatus,					// Failover Status
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
	std::string insertInterfaceStruct(		// Interface einfügen mittles struct intfSets
		intfSets *iSets
		);	
	void insertIntfStatsStruct(				// Interface updaten mittels struct intsStats
		intfStats *iStats
		);			
	void insertCrypto(						// Crypto Infos einfügen mittles struct cryptoSets
		cryptoSets *cSets
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
	void insertIntfN1k(						// Nexus1k Interface Infos einfügen (Veth Interfaces)
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
	void schreibeLogMin(					// Funktion für minimale Log Ausgabe
		std::string logEintrag					// Text für den Logeintrag
		);
	void iosCDPSchreiber(					// CDP Infos am Schluss noch in die Device und Interface Tabellen schreiben
		);
	void unknownNextHopsSchreiber(			// Unbekannte Next Hops in die Device und Interface Tabellen schreiben
		);
	void unknownrpNeighborSchreiber(		// Unbekannte RP Nachbaren in die Device und Interface Tabellen schreiben
		);
	void unknownIpsecPeersSchreiber(		// Unbekannte IPSec Nachbaren in die Device und Interface Tabellen schreiben
		);
	std::string unknownNetworkSchreiber(	// Unbekannte Netze (Wolken) in die Device und Interface Tabellen schreiben
		std::string networkName,				// Netzwerkname
		std::string type						// Wolkentype (40-42)
		);
	void rControlFlag();					// rControlEmpty Flag rausfinden
	std::string getSTP();					// Liefert den zu parsenden String zurück
	void setSTP(							// Setzt ganzeDatei
		std::string stp							// Fileinhalt
		);
};		

#endif // #ifdef WKTOOLS_MAPPER
