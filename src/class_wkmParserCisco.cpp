
#include "class_wkmParserCisco.h"
#ifdef WKTOOLS_MAPPER


#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>  

#include <map>


WkmParserCisco::WkmParserCisco(std::string filedaten)
{
	ganzeDatei = filedaten;
	stop = false;
	rControlEmpty = true;
}



WkmParserCisco::~WkmParserCisco()
{

}


int WkmParserCisco::startParser()
{
	pos1 = pos2 = pos3 = pos4 = pos5 = pos6 = pos7 = pos8 = pos9 = pos10 = pos11 = pos12 = pos13 = pos14 = pos15 = pos16 = pos17 = pos18 = pos19 = pos20= 0;
	pos21 = pos22 = pos23 = pos24 = pos25 = pos26 = pos27 = 0;
	aktPos1 = aktPos2 = 0;

	// Gerätetypen:
	//////////////////////////////////////////////////////////////////////////
	// 1    Router
	// 2    Switch
	// 3    Firewall
	// 4    Accesspoint
	// 6    VoiceGateway
	// 7    L2 Switch Root
	// 8    L3 Switch
	// 9    L3 Switch Root
	// 10   Router mit aktivem STP Protokoll
	// 11   Phone
	// 12   Endgerät
	// 13   ATA
	// 14   Camera
	// 30   Netz (Ethernet)
	// 31   Netz (Wolke)
	// 99   Unknown

	int returnValue = 0;

	// Variablen zurücksetzen
	hostname = "";					// Hostname
	version = "";					// IOS 
	modell = "";					// Gerätetype
	devicetype = UNKNOWN;
	devType = "0";
	stpBridgeID = "";
	stpProtocol = "";
	hwRevision = "";
	hwPos = "";
	hwDescription = "";
	hwMem = "";
	hwBootfile = "";
	prozessor = "";
	chassisSN = "";
	confReg = "";

	nodiag = false;
	c72er = false;
	gsr = false;


	pos1 = ganzeDatei.find("show interface");
	pos2 = ganzeDatei.find("show arp", pos1);
	pos3 = ganzeDatei.find("show mac-address-table", pos2);
	pos4 = ganzeDatei.find("show spanning-tree detail", pos3);
	pos5 = ganzeDatei.find("show cdp neigh det", pos4);
	pos6 = ganzeDatei.find("show ip arp", pos5);
	pos7 = ganzeDatei.find("show mac address-table", pos6);
	pos8 = ganzeDatei.find("show vpc", pos7);
	pos9 = ganzeDatei.find("show ip route | excl B ", pos8);
	pos10 = ganzeDatei.find("show ip bgp sum", pos9);
	pos11 = ganzeDatei.find("show ip ospf neigh", pos10);
	pos12 = ganzeDatei.find("show ip eigrp neigh", pos11);
	pos13 = ganzeDatei.find("show crypto ipsec sa", pos12);
	pos14 = ganzeDatei.find("show ospf neigh", pos13);
	pos15 = ganzeDatei.find("show eigrp neigh", pos14);
	pos16 = ganzeDatei.find("show route", pos15);
	pos17 = ganzeDatei.find("show switch virtual link port", pos16);
	pos18 = ganzeDatei.find("show snmp", pos17);
	pos19 = ganzeDatei.find("show spanning-tree", pos18);
	pos20 = ganzeDatei.find("show interface switchport", pos19);
	if (pos20 == ganzeDatei.npos)
	{
		pos20 = ganzeDatei.find("show interfaces switchport", pos19);
	}
	pos21 = ganzeDatei.find("show module", pos20);
	pos22 = ganzeDatei.find("show c7200", pos21);
	pos23 = ganzeDatei.find("show diag", pos22);
	pos24 = ganzeDatei.find("show invent", pos23);
	pos25 = ganzeDatei.find("show hardware", pos24);
	pos26 = ganzeDatei.find("show system", pos25);
	pos27 = ganzeDatei.find("show file system", pos26);
	pos28 = ganzeDatei.find("show power inline", pos27);
	pos29 = ganzeDatei.find("show ip route vrf all", pos28);
	pos30 = ganzeDatei.find("show vrf interface", pos29);
	pos31 = ganzeDatei.find("show ip vrf detail", pos30);


	pos1iph = ganzeDatei.find("HTTP/1.1 200 OK");
	pos2iph = ganzeDatei.find("<HTML>");
	pos3iph = ganzeDatei.find("<html>");

	if ((pos1iph != ganzeDatei.npos) || (pos2iph != ganzeDatei.npos) || (pos3iph != ganzeDatei.npos))
	{
		testPhones();
	}
	else if (pos4 == ganzeDatei.npos)
	{
		// TODO: FEHLERAUSGABE
		//dbgA = "6612: Cannot parse file due to lack of information!\n";
		//schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6612", WkLog::WkLog_ROT);

		return 1;
	}
	else
	{
		// SHOW VER
		//////////////////////////////////////////////////////////////////////////
		// Nexus oder nicht?
		aktPos1 = ganzeDatei.find("Nexus Operating System");
		if (aktPos1 < pos1)
		{
			testNexus();
		}
		else
		{
			aktPos1 = ganzeDatei.find(", Version") + 10;
			aktPos2 = ganzeDatei.find_first_of(", ", aktPos1);
			if (aktPos1 != ganzeDatei.npos)
			{
				version = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
			}				
			else
			{
				aktPos1 = 0;
			}

			aktPos1 = ganzeDatei.find(" uptime is");
			aktPos2 = ganzeDatei.rfind("\n", aktPos1);
			if (aktPos2 == ganzeDatei.npos)
			{
				aktPos2 = ganzeDatei.rfind("\r", aktPos1);
			}
			if (aktPos1 != ganzeDatei.npos)
			{
				aktPos2 = ganzeDatei.find_first_not_of(" \r\n", aktPos2);
				hostname = ganzeDatei.substr(aktPos2, aktPos1 - aktPos2);

				if ((aktPos1 = ganzeDatei.find("System image file is \"", aktPos1)) != ganzeDatei.npos)
				{
					aktPos2 = ganzeDatei.find("\"", aktPos1 + 22);
					if (aktPos2 != ganzeDatei.npos)
					{
						aktPos1 += 22;
						hwBootfile = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
					}
					else
					{
						aktPos2 = ganzeDatei.find("image file", aktPos2);
					}
				}
				testIOS();
			}
			else if ((aktPos1 = ganzeDatei.find("Mod Port Model")) < pos1)
			{				
				// CATOS - CATOS - CATOS - CATOS
				//////////////////////////////////////////////////////////////////////////
				//dbgA = "6613: Unsupported Device!\n";
				//schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6613", WkLog::WkLog_ROT);

				devicetype = SWITCH;
				devType = "2";
				testCatOS();
			}
			else if ((aktPos1 = ganzeDatei.find("Hardware: ")) < pos1)
			{
				devicetype = FIREWALL;
				devType = "3";
				testPixOS();
			}
			else
			{
				aktPos1 = 0;
				testIOS();
			}
		}
	}

	return returnValue;
}


void WkmParserCisco::testIOS()
{
	bool processor = true;
	if ((aktPos1 = ganzeDatei.find("processor", aktPos2)) == ganzeDatei.npos)
	{
		processor = false;
		aktPos1 = ganzeDatei.find("(revision", aktPos2);
	}
	if (aktPos1 < pos1)
	{
		if ((aktPos1 = ganzeDatei.rfind("isco ", aktPos1)) == ganzeDatei. npos)
		{

		}
		else	
		{
			aktPos2 = ganzeDatei.find(" ", aktPos1 + 5);
			if (aktPos2 != ganzeDatei.npos)
			{
				aktPos1 += 5;
				modell = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
				if (modell.find("GRP") != modell.npos)
				{
					gsr = true;
				}
			}
			else
			{
				aktPos2 = aktPos1;
			}
		}
	}

	if ((aktPos1 = ganzeDatei.find("(", aktPos2)) == ganzeDatei.npos)
	{

	}
	else if (processor)
	{
		aktPos2 = ganzeDatei.find(")", aktPos1 + 1);
		if (aktPos2 != ganzeDatei.npos)
		{
			aktPos1++;
			prozessor = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
		}
		else
		{
			aktPos2 = aktPos1;
		}
	}
	if ((aktPos1 = ganzeDatei.find("evision ", aktPos2)) != ganzeDatei.npos)
	{
		aktPos1	+= 8;
		aktPos2 = ganzeDatei.find_first_of(")\r\n", aktPos1);
		hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}
	else
	{
		aktPos1 = aktPos2;
	}


	if ((aktPos1 = ganzeDatei.find("with ", aktPos2)) == ganzeDatei.npos)
	{

	}
	else
	{
		aktPos2 = ganzeDatei.find(" ", aktPos1 + 5);
		if (aktPos2 != ganzeDatei.npos)
		{
			aktPos1 += 5;
			hwMem = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
		}
		else
		{
			aktPos2 = aktPos1;
		}
	}


	if ((aktPos1 = ganzeDatei.find("Processor board ID", aktPos2)) == ganzeDatei.npos)
	{

	}
	else
	{
		aktPos2 = ganzeDatei.find_first_of("\r\n,", aktPos1 + 18);
		if (aktPos2 != ganzeDatei.npos)
		{
			aktPos1 += 19;
			chassisSN = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
		}
	}

	// Herausfinden, ob es sich um einen AccessPoint handelt
	if ((aktPos1 = ganzeDatei.find("Top Assembly Serial Number", aktPos2)) < pos1)
	{
		aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);	

		if ((aktPos1 = ganzeDatei.find("Model Number", aktPos2)) < pos1)
		{
			aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			devicetype = ACCESSPOINT;
			devType = "4";
		}
	}

	// Model Number bei Switches auslesen
	else if ((aktPos1 = ganzeDatei.find("Model revision number", aktPos2)) < pos1)
	{
		aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		if ((aktPos1 = ganzeDatei.find("Model number", aktPos2)) < pos1)
		{
			aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find("System serial number", aktPos2)) < pos1)
		{
			aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
	}

	stackpos = aktPos2;
	hwPos = "box";

	// Config Register
	if ((aktPos1 = ganzeDatei.find("Configuration register is ", aktPos2)) < pos1)
	{
		aktPos1 += 26;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		confReg = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}	

	// Herausfinden, ob es sich um einen Switch handelt
	if (((ganzeDatei.find("Bridge Identifier has priority", pos4)) != ganzeDatei.npos) && (modell.find("WS-C") != modell.npos))
	{
		devicetype = SWITCH;
		devType = "2";
		bool ret = insertDevice();
		if (ret)
		{
			testSW();
		}
	}
	else if ((ganzeDatei.find("Bridge Identifier has priority", pos4)) != ganzeDatei.npos)
	{
		devicetype = ROUTER_SW;
		devType = "10";
		bool ret = insertDevice();
		if (ret)
		{
			testRouter();
		}
	}
	else
	{
		if (modell.find("VG2") != modell.npos)
		{
			devType = "6";
			devicetype = VOICEGATEWAY;
		}
		else if (devType != "4")
		{
			devType = "1";
			devicetype = ROUTER;
		}
		bool ret = insertDevice();
		if (ret)
		{
			testRouter();
		}
	}
	iosCDPParser();
	iosRouteParser();
	iosBGPParser();
	iosOSPFParser();
	iosEIGRPParser();
	ipsecParser();
	iosVSLParser();
	iosSNMPParser();
	iosModuleParser();
	iosHWParser();
	iosInventoryParser();
	iosShowHardwareParser();
	iosFileSystemParser();
	iosPoeParser();
	iosVrfParser();

	markNeighbor();
}


void WkmParserCisco::testPixOS()
{
	// PIX, ASA, FWSM
	//////////////////////////////////////////////////////////////////////////
	// show version
	//////////////////////////////////////////////////////////////////////////
	aktPos1 = 0;
	aktPos2 = 0;

	hwPos = "box";
	hwBootfile = "";

	if ((aktPos1 = ganzeDatei.find("Version ")) < pos1)
	{
		aktPos1 += 8;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		version = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}
	if ((aktPos1 = ganzeDatei.find("System image file is \""),aktPos2) < pos1)
	{
		aktPos1 += 22;
		aktPos2 = ganzeDatei.find_first_of("\"\r\n", aktPos1);
		hwBootfile = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}
	if ((aktPos1 = ganzeDatei.find(" up ", aktPos2) + 4) < pos1)
	{
		aktPos1 = ganzeDatei.rfind("\n", aktPos1) + 1;
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		hostname = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}
	aktPos1 = ganzeDatei.find("Hardware:   ", aktPos2) + 12;
	aktPos2 = ganzeDatei.find(",", aktPos1);
	modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	aktPos1 = aktPos2 + 2;
	aktPos2 = ganzeDatei.find("RAM", aktPos1) -1;
	hwMem = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	aktPos1 = ganzeDatei.find("Flash", aktPos2);
	aktPos1 = ganzeDatei.find(",", aktPos1) + 2;
	aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
	hwFlash = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	aktPos1 = ganzeDatei.find("license.", aktPos2);
	std::string lic = "";
	if (aktPos1 != ganzeDatei.npos)
	{
		aktPos1--;
		aktPos2 = ganzeDatei.rfind("has a", aktPos1) + 4;
		aktPos2 = ganzeDatei.find(" ", aktPos2) + 1;
		lic = ganzeDatei.substr(aktPos2, aktPos1-aktPos2);
	}

	aktPos1 = ganzeDatei.find("Serial Number: ", aktPos2) + 15;
	aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
	chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	aktPos1 = ganzeDatei.find("Configuration register is ", aktPos2) + 26;
	aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
	confReg = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	bool ret = insertDevice();
	if (ret)
	{
		insertHardware();
		pixOSHWParser();
		pixOSInterfaceParser();
		pixOSARPParser();
		ipsecParser();
		pixOSrouteParser();
		pixOSOSPFParser();
		pixOSEIGRPParser();
		pixOSHWParser();
		pixOSFilesystemParser();
		pixOSLicParser();
	}

}


void WkmParserCisco::testRouter()
{
	insertHardware();
	routerLicParser();
	iosInterfaceParser();
	iosSTPParser();
	iosARPParser();
	iosCAMParser();
	iosSwitchportParser();
}


void WkmParserCisco::testSW()
{
	insertHardware();
	switchLicParser();
	iosInterfaceParser();
	iosSTPParser();
	iosARPParser();
	iosCAMParser();
	iosSwitchportParser();
}


void WkmParserCisco::iosInterfaceParser()
{
	zeitStempel("\tiosInterfaceParser START");
	// Alle Interfaces sind interessant, auch die inaktiven
	size_t intfpos = ganzeDatei.find(", line protocol is", pos1);
	intfpos = ganzeDatei.rfind("\n", intfpos);

	while (intfpos < pos2)
	{
		// Interface Name
		aktPos1 = intfpos + 1;
		intfpos = ganzeDatei.find(", line protocol is", intfpos + 50);
		if (intfpos == ganzeDatei.npos)
		{
			intfpos = pos2+2;
		}
		else
		{
			intfpos = ganzeDatei.rfind("\n", intfpos);
		}

		std::string intfName = "";
		std::string intfType = "";
		std::string phl = "0";
		std::string intfHauptIntf = "";
		std::string intfStatus = "UP";
		aktPos2 = aktPos1;
		if ((aktPos1 = ganzeDatei.find("is up, line protocol is up", aktPos2)) < intfpos)
		{
			intfStatus = "UP";
		}
		else if ((aktPos1 = ganzeDatei.find("administratively down", aktPos2)) < intfpos)
		{
			intfStatus = "DISABLED";
		}
		else if ((aktPos1 = ganzeDatei.find("(err-disabled)", aktPos2)) < intfpos)
		{
			intfStatus = "ERR-DISABLED";
		}
		else
		{
			aktPos1 = aktPos2 + 10;
			intfStatus = "DOWN";
		}

		aktPos1 = ganzeDatei.rfind("\n", aktPos1) + 1;
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		intfName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		intfName = intfNameChange(intfName);

		// Herausfinden, ob Subinterface
		if (intfName.find(".") != intfName.npos)
		{
			intfHauptIntf = intfName.substr(0, intfName.find("."));
		}

		// Hardware Adresse, falls vorhanden
		std::string intfMac = "";
		if ((aktPos1 = ganzeDatei.find("Hardware is", aktPos2)) < intfpos)
		{
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			aktPos1 = ganzeDatei.find("address is", aktPos1);
			if (aktPos1 < aktPos2)
			{
				intfMac = ganzeDatei.substr(aktPos1+11, 14);
			}
		}

		// Description, falls vorhanden
		std::string intfDescription = "";
		if ((aktPos1 = ganzeDatei.find("Description: ", aktPos2)) < intfpos)
		{
			aktPos1 += 13;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			intfDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			while (intfDescription.find_first_of("'\"&<>") != intfDescription.npos)
			{
				intfDescription.replace(intfDescription.find_first_of("'\"&<>"), 1, "-");
			}
		}
		// IP Adresse falls vorhanden
		std::string intfIP = "";
		std::string intfIPMask = "";
		std::string l2l3 = "L2";
		if ((aktPos1 = ganzeDatei.find("Internet address is ", aktPos2)) < intfpos)
		{
			aktPos1 += 20;
			aktPos2 = ganzeDatei.find("/", aktPos1)+1;
			intfIP = ganzeDatei.substr(aktPos1, aktPos2-aktPos1-1);
			aktPos1 = aktPos2;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			intfIPMask = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			l2l3 = "L3";
		}

		// Load falls vorhanden
		std::string txLoad = "";
		std::string rxLoad = "";
		int txload = 0;
		int rxload = 0;
		if ((aktPos1 = ganzeDatei.find("txload ", aktPos2)) < intfpos)
		{
			aktPos1 += 7;
			aktPos2 = ganzeDatei.find("/", aktPos1);
			txLoad = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				txload = boost::lexical_cast<int>(txLoad);
			}
			catch (boost::bad_lexical_cast &)
			{
				txload = 0;			
			}


			if ((aktPos1 = ganzeDatei.find("rxLoad ", aktPos2)) < intfpos)
			{
				aktPos1 += 7;
				aktPos2 = ganzeDatei.find("/", aktPos1);
				rxLoad = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					rxload = boost::lexical_cast<int>(rxLoad);
				}
				catch (boost::bad_lexical_cast &)
				{
					rxload = 0;			
				}
			}
		}
		int intfLoadLevel = (rxload+txload)/2;

		// Encapsulation
		//aktPos1 = ganzeDatei.find("Encapsulation ", aktPos2) + 14;
		//aktPos2 = ganzeDatei.find_first_of(",\r\n", aktPos1);
		//std::string encaps = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		// VLAN ID falls vorhanden
		std::string vlanid = "";
		int vlanID = 0;
		if ((aktPos1 = ganzeDatei.find("Vlan ID  ", aktPos2)) < intfpos)
		{
			aktPos1 += 9;
			aktPos2 = ganzeDatei.find(".", aktPos1);
			vlanid = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				vlanID = boost::lexical_cast<int>(vlanid);
			}
			catch (boost::bad_lexical_cast &)
			{
				vlanID = 0;			
			}
			if (devType == "1")
			{
				// VLAN Tabelle befüllen
				std::string iString = "INSERT INTO vlan (vlan_id, vlan) VALUES (NULL, ";
				iString += vlanid + ");";
				dieDB->query(iString.c_str());

				iString = "SELECT last_insert_rowid() FROM vlan;";
				std::vector<std::vector<std::string> > result2 = dieDB->query(iString.c_str());
				std::string vlID = "";
				if (!result2.empty())
				{
					vlID = result2[0][0];
				}
				else
				{
					std::string dbgA = "\n6305: Parser Error! Code 0001 - Contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehlerString += "\n" + dateiname + dbgA;
				}

				// vlan_has_device Tabelle befüllen
				iString = "INSERT INTO vlan_has_device (vlan_vlan_id, device_dev_id) VALUES (";
				iString += vlID + "," + dev_id + ");";
				dieDB->query(iString.c_str());
			}

		}


		// Duplex und Speed falls vorhanden
		std::string duplex = "";
		std::string speed = "";
		if ((aktPos1 = ganzeDatei.find("uplex", aktPos2)) < intfpos)
		{
			aktPos1 = ganzeDatei.rfind(" ", aktPos1);
			aktPos2 = ganzeDatei.find(",", aktPos1);
			duplex = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			boost::algorithm::trim(duplex);

			aktPos1 = aktPos2+2;
			aktPos2 = ganzeDatei.find_first_of(",\r\n", aktPos1);
			speed = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			boost::algorithm::trim(speed);
		}

		// Interface Type rausfinden
		intfType = intfTypeCheck(intfName, speed);
		phl = intfPhlCheck(intfName, speed);

		// Channel Members
		std::string chMem = "";
		std::queue<std::string> chMembers;
		if ((aktPos1 = ganzeDatei.find("Members in this channel", aktPos2)) < intfpos)
		{
			aktPos1 += 25;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			chMem = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if (chMem != "")
		{
			boost::char_separator<char> sep(" ");
			boost::tokenizer<boost::char_separator<char>> tok(chMem, sep);

			for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				chMembers.push(*beg);
			}
		}

		// Last Intf Change
		std::string lastChange = "";
		if ((aktPos1 = ganzeDatei.find("Last input ", aktPos2)) < intfpos)
		{
			aktPos1 += 11;
			aktPos2 = ganzeDatei.find_first_of("\r\n,", aktPos1);
			lastChange = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Last clearing of interface counters
		std::string lastClear = "";
		if ((aktPos1 = ganzeDatei.find("Last clearing of \"show interface\" counters ", aktPos2)) < intfpos)
		{
			aktPos1 += 43;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			lastClear = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Input Bits/s
		std::string ibits = "";
		if ((aktPos1 = ganzeDatei.find("input rate ", aktPos2)) < intfpos)
		{
			aktPos1 += 11;
			aktPos2 = ganzeDatei.find(" ", aktPos1);
			ibits = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Output Bits/s
		std::string obits = "";
		if ((aktPos1 = ganzeDatei.find("output rate ", aktPos2)) < intfpos)
		{
			aktPos1 += 12;
			aktPos2 = ganzeDatei.find(" ", aktPos1);
			obits = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Fehler
		//////////////////////////////////////////////////////////////////////////
		int tempErr = 0;
		int err = 0;

		// Input Buffer errors
		std::string ibuffer = "";
		if ((aktPos1 = ganzeDatei.find(" no buffer", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			ibuffer = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(ibuffer);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Input Runts
		std::string runts = "";
		if ((aktPos1 = ganzeDatei.find(" runts", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			runts = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(runts);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;

		}

		// Input Giants
		std::string giants = "";
		if ((aktPos1 = ganzeDatei.find(" giants", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			giants = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(giants);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Input Throttles
		std::string throttles = "";
		if ((aktPos1 = ganzeDatei.find(" throttles", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			throttles = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(throttles);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Input Erros
		std::string ierrors = "";
		if ((aktPos1 = ganzeDatei.find(" input errors", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			ierrors = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(ierrors);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		std::string iCrc = "";
		std::string iFrame = "";
		std::string iOverrun = "";
		std::string iIgnored = "";
		std::string iWatchdog = "";
		std::string iPause = "";
		std::string iDribbleCondition = "";
		std::string l2decodeDrops = "";

		if ((aktPos1 = ganzeDatei.find(" CRC", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iCrc = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" frame", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iFrame = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" overrun", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iOverrun = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" ignored", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iIgnored = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" watchdog", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iWatchdog = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" watchdog", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iWatchdog = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" pause input", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iPause = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" input packets with dribble condition detected", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iDribbleCondition = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}


		// Output underruns
		std::string underrun = "";
		if ((aktPos1 = ganzeDatei.find(" underruns", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
			underrun = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(underrun);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Output errors
		std::string oerrors = "";
		std::string oCollisions = "";
		std::string oBabbles = "";
		std::string oLateColl = "";
		std::string oDeferred = "";
		std::string oLostCarrier = "";
		std::string oNoCarrier = "";
		std::string oPauseOutput = "";
		std::string oBufferSwapped = "";

		if ((aktPos1 = ganzeDatei.find(" output errors", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
			oerrors = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(oerrors);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		if ((aktPos1 = ganzeDatei.find(" collisions", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oCollisions = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Interface Resets
		std::string resets = "";
		if ((aktPos1 = ganzeDatei.find(" interface resets", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
			resets = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(resets);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		if ((aktPos1 = ganzeDatei.find(" babbles", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oBabbles = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" late collision", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oLateColl = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" deferred", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oDeferred = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" lost carrier", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oLostCarrier = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" no carrier", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oLostCarrier = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" PAUSE output", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oPauseOutput = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Output Buffer Errors
		std::string obuffer = "";
		if ((aktPos1 = ganzeDatei.find(" output buffer failures", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
			obuffer = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(obuffer);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}
		if ((aktPos1 = ganzeDatei.find(" output buffers swapped out", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oBufferSwapped = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		std::string iString = "INSERT INTO interfaces (intf_id,intfName,intfType,phl,macAddress,ipAddress,subnetMask,duplex,speed,status,description,l2l3,";
		iString += "errLvl,lastClear,loadLvl,channel_intf_id,iCrc,iFrame,iOverrun,iIgnored,iWatchdog,iPause,iDribbleCondition,ibuffer,l2decodeDrops,runts,giants,";
		iString += "throttles,ierrors,underrun,oerrors,oCollisions,oBabbles,oLateColl,oDeferred,oLostCarrier,oNoCarrier,oPauseOutput,oBufferSwapped,";
		iString += "resets,obuffer,lastInput)";
		iString += " VALUES(NULL, '" + intfName + "','" + intfType + "'," + phl + ",'" + intfMac + "','" + intfIP + "','"  + intfIPMask + "','" + duplex + "','" + speed;
		iString += "','" + intfStatus + "','" + intfDescription + "','" + l2l3 + "'," + boost::lexical_cast<std::string>(err) + ",'" + lastClear + "',"; 
		iString += boost::lexical_cast<std::string>(intfLoadLevel) + ",NULL,'" + iCrc + "','" + iFrame + "','" + iOverrun + "','" + iIgnored;
		iString += "','" + iWatchdog + "','" + iPause + "','" + iDribbleCondition + "','" + ibuffer + "','"  + l2decodeDrops + "','" + runts + "','" + giants + "','" + throttles;
		iString += "','" + ierrors + "','" + underrun + "','" + oerrors + "','" + oCollisions + "','" + oBabbles + "','" + oLateColl + "','" + oDeferred;
		iString += "','" + oLostCarrier + "','" + oNoCarrier + "','" + oPauseOutput + "','" + oBufferSwapped + "','" + resets + "','" + obuffer + "','" + lastChange + "');";
		dieDB->query(iString.c_str());

		std::string sString = "SELECT last_insert_rowid() FROM interfaces;";
		std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
		intf_id = "";
		if (!result2.empty())
		{
			intf_id = result2[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0002 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// Die Channel Members markieren
		while (chMembers.size())
		{
			std::string iString3 = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			iString3 += chMembers.front() + "' AND devInterface.device_dev_id=" + dev_id +";";
			std::vector<std::vector<std::string> > result = dieDB->query(iString3.c_str());

			std::string chMemID = "";
			if (!result.empty())
			{
				chMemID = result[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0003 - Contact wktools@spoerr.org\n";
				dbgA += iString3;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

			std::string iString4 = "UPDATE interfaces SET channel_intf_id=last_insert_rowid() WHERE intf_id = " + chMemID + ";";
			dieDB->query(iString4.c_str());

			chMembers.pop();
		}

		// Das Hauptinterface finden und dann die Subinterfaces markieren
		if (intfHauptIntf != "")
		{
			std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			sString += intfHauptIntf + "' AND devInterface.device_dev_id=" + dev_id +";";
			std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

			if (!result.empty())
			{
				std::string iString4 = "UPDATE interfaces SET channel_intf_id=" + result[0][0] + " WHERE intf_id = last_insert_rowid();";
				dieDB->query(iString4.c_str());
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0004 - Contact wktools@spoerr.org\n";
				dbgA += sString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}


		}

		// devInterface Tabelle befüllen
		std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
		dieDB->query(iString2.c_str());

		// ipSubnet befüllen
		if (intfIP != "")
		{
			ipNetzeEintragen(intfIP, intfIPMask);
		}
	}
}


void WkmParserCisco::iosSTPParser()
{
	zeitStempel("\tiosSTPParser START");
	size_t stpStartpos = pos4;
	if (ganzeDatei.find("% Invalid input detected at '^' marker.", pos4) < pos5)
	{
		stpStartpos = pos19;
	}

	// Check, ob STP ALT oder NEU oder MST; 0...NEU; 1...MST; 2...ALT
	// Bsp.: für ALT: XL Switches; 12.0 SW
	int stpneu = 0;
	if (ganzeDatei.find("(port ", stpStartpos) < ganzeDatei.npos)
	{
		stpneu = 2;
	}
	if (ganzeDatei.find("( MST", stpStartpos) < pos5)
	{
		stpneu = 1;
	}

	std::string gl1 = "";
	std::string gl2 = "";
	size_t gl2offset = 0;
	switch (stpneu)
	{
	case 0:
		gl1 = " Port ";
		gl2 = " VLAN";
		gl2offset = 5;
		break;
	case 1:
		gl1 = " Port ";
		gl2 = " MST";
		gl2offset = 4;
		break;
	case 2:
		gl1 = "(port ";
		gl2 = "Spanning tree ";
		gl2offset = 14;
		break;
	}

	// Teil 1: Globale und VLAN bezogene STP Infos
	size_t stppos = ganzeDatei.find("Spanning Tree protocol", stpStartpos);
	while (stppos < ganzeDatei.npos)
	{
		bool vlanBridgeGroup = true;	// TRUE wenn VLAN, FALSE wenn BridgeGroup
		size_t portPos = ganzeDatei.find(gl1, stppos);
		aktPos1 = ganzeDatei.rfind(gl2, stppos)+gl2offset;
		if ((aktPos1 < stpStartpos) || (aktPos1 == ganzeDatei.npos))
		{
			aktPos1 = ganzeDatei.rfind(" Bridge group ", stppos)+14;
			vlanBridgeGroup = false;
		}
		stppos = ganzeDatei.find("Spanning Tree protocol", stppos + 20);

		// VLAN ID
		std::string vlanID = "";
		//if (vlanBridgeGroup)
		//{
		//	vlanID = ganzeDatei.substr(aktPos1, 4);
		//}
		//else
		//{
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		vlanID = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		//}

		// STP Protocol
		aktPos1 = ganzeDatei.find("the", aktPos1) + 4;
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		stpProtocol = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		// STP Protokoll Normalizing
		boost::algorithm::to_lower(stpProtocol);


		// Bridge Prio und ID
		std::string stpPrio = "";
		stpBridgeID = "";
		if ((aktPos1 = ganzeDatei.find("Bridge Identifier has priority ", aktPos1)) < stppos)
		{
			aktPos1 += 31;
			aktPos2 = ganzeDatei.find(",", aktPos1);
			stpPrio = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos2);
			if ((aktPos1 = ganzeDatei.find("address", aktPos1)) < aktPos2)
			{
				stpBridgeID = ganzeDatei.substr(aktPos1+8, 14);
			}
		}

		// ROOT ID und ROOT Port
		std::string stpRootID = "";
		std::string stpRootPort = "";
		if ((aktPos1 = ganzeDatei.find("Current root has priority ", aktPos1)) < stppos)
		{
			aktPos1 += 26;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			if ((aktPos1 = ganzeDatei.find("address", aktPos1)) < aktPos2)
			{
				stpRootID = ganzeDatei.substr(aktPos1+8, 14);
			}				

			if ((aktPos1 = ganzeDatei.find("Root port is", aktPos1)) < stppos)
			{
				aktPos1 = ganzeDatei.find("(", aktPos1)+1;
				aktPos2 = ganzeDatei.find(")", aktPos1);
				stpRootPort = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				stpRootPort = intfNameChange(stpRootPort);
			}
		}
		else if ((aktPos1 = ganzeDatei.find("We are the root of the spanning tree", aktPos1)) < stppos)
		{
			stpRootID = stpBridgeID;
			stpRootPort = "";
		}
		// stpInstanz Tabelle befüllen
		std::string iString = "INSERT INTO stpInstanz (stp_id, stpPriority, rootPort) VALUES (NULL, '";
		iString += stpPrio + "', '" + stpRootPort + "');";
		dieDB->query(iString.c_str());

		iString = "SELECT last_insert_rowid() FROM stpInstanz;";
		std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
		std::string stpInstanzID = "";
		if (!result.empty())
		{
			stpInstanzID = result[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0005 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}


		std::string vlID = "";
		if (stpneu != 1)
		{
			// VLAN Tabelle befüllen
			iString = "INSERT INTO vlan (vlan_id, vlan) VALUES (NULL, " + vlanID + ");";
			dieDB->query(iString.c_str());

			iString = "SELECT last_insert_rowid() FROM vlan;";
			std::vector<std::vector<std::string> > result2 = dieDB->query(iString.c_str());
			if (!result.empty())
			{
				vlID = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0006 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}



			// vlan_stpInstanz Tabelle befüllen
			iString = "INSERT INTO vlan_stpInstanz (vlan_vlan_id, stpInstanz_stp_id) VALUES (";
			iString += vlID + "," + stpInstanzID + ");";
			dieDB->query(iString.c_str());

			// vlan_has_device Tabelle befüllen
			iString = "INSERT INTO vlan_has_device (vlan_vlan_id, device_dev_id) VALUES (";
			iString += vlID + "," + dev_id + ");";
			dieDB->query(iString.c_str());
		}


		std::string istr1 = "";

		if (stpneu < 2)
		{
			istr1 = " Port ";
		}
		else
		{
			istr1 = "Interface ";
		}

		// Teil 2: Interfacebezogene Infos
		while (portPos < stppos)
		{
			aktPos1 = portPos;
			portPos = ganzeDatei.find(istr1, portPos + 300);

			// Wird nur berücksichtigt, wenn BPDU received != 0
			size_t bpduCntPos = ganzeDatei.find(", received 0", aktPos1);
			if (bpduCntPos < portPos)
			{
				continue;
			}

			bpduCntPos = ganzeDatei.find(", received ", aktPos1);
			if (bpduCntPos < portPos)
			{
				// Interfacename und STP Status
				std::string intfName = "";
				std::string stpStatus = "";
				if (stpneu < 2)
				{
					if ((aktPos1 = ganzeDatei.find("(", aktPos1)) < portPos)
					{
						aktPos1 += 1;
						aktPos2 = ganzeDatei.find(")", aktPos1);
						intfName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						intfName = intfNameChange(intfName);

						aktPos1 = ganzeDatei.find(" is ", aktPos2) + 4;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						stpStatus = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					}
				}
				else
				{
					aktPos1 = ganzeDatei.rfind("nterface", aktPos1) + 9;
					aktPos2 = ganzeDatei.find(" ", aktPos1);
					intfName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find(" is ", aktPos2) + 4;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					stpStatus = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				}

				// ROOT ID und Designated Bridge ID
				std::string stpRootIDPort = "";
				std::string stpDesignatedBridgeID = "";
				if ((aktPos1 = ganzeDatei.find("address", aktPos1)) < portPos)
				{
					aktPos1 += 8;
					stpRootIDPort = ganzeDatei.substr(aktPos1, 14);

					aktPos1 = ganzeDatei.find("address", aktPos1) + 8;
					stpDesignatedBridgeID = ganzeDatei.substr(aktPos1, 14);
				}

				// Transition to Forwarding State (grün: 0 oder 1; orange: 1-10; rot: > 10)
				std::string stpTransitionNumber = "0";
				if ((aktPos1 = ganzeDatei.find("Number of transitions to forwarding state: ", aktPos1)) < portPos)
				{
					aktPos1 += 43;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					stpTransitionNumber = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				}

				// stp_status Tabelle befüllen
				iString = "INSERT INTO stp_status (stp_status_id, stpIntfStatus, designatedRootID, designatedBridgeID, stpTransitionCount) VALUES (NULL, '";
				iString += stpStatus + "','" + stpRootIDPort  + "','" + stpDesignatedBridgeID + "'," + stpTransitionNumber + ");";
				dieDB->query(iString.c_str());

				// Interface ID auslesen
				iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
				iString += intfName + "' AND devInterface.device_dev_id=" + dev_id +";";
				std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
				std::string intfID = "";
				if (!result.empty())
				{
					intfID = result[0][0];
				}
				else
				{
					std::string dbgA = "\n6406: Parser Error! Code 0034 - If Interface '" + intfName + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
						WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
					fehlerString += "\n" + dateiname + dbgA;
					continue;
				}

				if (stpneu != 1)
				{
					// int_vlan Tabelle befüllen
					iString = "INSERT INTO int_vlan (interfaces_intf_id, vlan_vlan_id, stp_status_stp_status_id) VALUES (";
					iString += intfID + "," + vlID + ",last_insert_rowid());";
					dieDB->query(iString.c_str());
				}
			}
		}
	}
	std::string iString = "UPDATE device SET stpBridgeID='" + stpBridgeID + "', stpProtocol='" + stpProtocol + "' WHERE dev_id=" + dev_id + ";";
	dieDB->query(iString.c_str());

	iString = "INSERT INTO interfaces (intfName, macAddress, l2l3, status, intfType) VALUES ('STP','" + stpBridgeID + "','L2','UP','STP');";
	dieDB->query(iString.c_str());
}


void WkmParserCisco::iosARPParser()
{
	zeitStempel("\tiosARPParser START");

	aktPos1 = ganzeDatei.find("Interface", pos2);
	aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1)+1;

	std::string arpString = ganzeDatei.substr(aktPos1, pos3-aktPos1);

	std::string arpZeile = "";

	std::string ipa = "";
	std::string maca = "";
	std::string intf = "";

	std::string iname = "";			// Interface Name vom letzten Durchlauf
	std::string iid = "";			// Interface ID vom letzten Interface Name

	std::string ageTime = "";
	std::string self = "0";

	aktPos1 = 0;
	aktPos2 = 0;

	int offsets[] = {10, 15, 13, 14, 9, 40};
	boost::offset_separator f3(offsets, offsets + 6, false, true);
	for (; aktPos2 < arpString.npos;)
	{
		aktPos2 = arpString.find_first_of("\r\n", aktPos1);
		arpZeile = arpString.substr(aktPos1, aktPos2 - aktPos1);
		aktPos1 = arpString.find_first_not_of("\r\n", aktPos2);
		if (arpZeile.size() > 60)
		{
			int i = 0;
			boost::tokenizer<boost::offset_separator> tok(arpZeile, f3);
			for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				switch (i)
				{
				case 1:
					ipa = *beg;
					boost::algorithm::trim(ipa);
					break;
				case 2:
					ageTime = *beg;
					boost::algorithm::trim(ageTime);
					break;
				case 3:
					maca = *beg;
					boost::algorithm::trim(maca);
					break;
				case 5:
					intf = *beg;
					boost::algorithm::trim(intf);
					intf = intfNameChange(intf);
					break;
				default:
					break;
				}
				i++;
			}

			if (ageTime.find("-") != ageTime.npos)
			{
				self = "1";
			}
			else
			{
				self = "0";
			}

			// Check, ob Interfacename == "Virtual..." (Nexus)
			if (intf.find("Virtual") != intf.npos)
			{
				continue;
			}

			// Check, ob Interfacename == "Interface" (cat4k Zwischen-Überschrift)
			if (intf.find("Interface") != intf.npos)
			{
				continue;
			}

			// Zuerst die Interface ID in der interface Tabelle suchen
			std::string intfID = "";
			if (iname != intf)
			{
				std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
				iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
				std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
				if (!result.empty())
				{
					intfID = result[0][0];
				}
				else
				{
					std::string dbgA = "\n6406: Parser Error! Code 0035 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
						WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
					fehlerString += "\n" + dateiname + dbgA;
					continue;
				}
			}
			else 
			{
				intfID = iid;
			}

			iname = intf;
			iid = intfID;

			// Dann die Neighbor Tabelle füllen
			std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr, l3_addr, self) VALUES (NULL, '";
			iString2 += maca + "', '" + ipa + "'," + self + ");";
			dieDB->query(iString2.c_str());

			// Dann die nlink Tabelle
			std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
			iString3 += intfID + ");";
			dieDB->query(iString3.c_str());
		}
	}
}


void WkmParserCisco::iosCAMParser()
{
	zeitStempel("\tiosCAMParser START");

	// Herausfinden, ob Cat6k oder anderer Switch
	size_t campos1 = 0;
	size_t campos2 = 0;
	if (pos8 == ganzeDatei.npos)
	{
		if (pos7 != ganzeDatei.npos)
		{
			pos8 = ganzeDatei.size();
		}
	}
	if (pos4-pos3 > 250)
	{
		campos1 = pos3;
		campos2 = pos4;
	}
	else
	{
		campos1 = pos7;
		campos2 = pos8;
	}

	if (campos2-campos1 > 250)
	{
		std::string iname = "";			// Interface Name vom letzten Durchlauf
		std::string iid = "";			// Interface ID vom letzten Interface Name

		size_t typePos = ganzeDatei.find("Legend: * - primary entry", campos1);
		size_t typePos4k = ganzeDatei.find("Unicast Entries", campos1);
		size_t typeXLPos = ganzeDatei.find("Destination Address  Address Type  VLAN  Destination Port", campos1);
		if (typePos < ganzeDatei.npos)
		{
			// Cat6k
			aktPos1 = ganzeDatei.find("*", typePos+50);
			std::string camString = ganzeDatei.substr(aktPos1-1, campos2-aktPos1);
			std::string camZeile = "";

			std::string vlan = "";
			std::string maca = "";
			std::string intf = "";

			aktPos1 = 0;
			aktPos2 = 0;

			int offsets[] = {2, 4, 2, 14, 29, 25};
			boost::offset_separator f3(offsets, offsets + 6, false, true);
			for (; aktPos2 < camString.npos;)
			{
				aktPos2 = camString.find_first_of("\r\n", aktPos1 + 1);
				camZeile = camString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
				aktPos1 = aktPos2;
				if (camZeile.size() > 50)
				{
					int i = 0;
					boost::tokenizer<boost::offset_separator> tok(camZeile, f3);
					for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 1:
							vlan = *beg;
							break;
						case 3:
							maca = *beg;
							break;
						case 5:
							intf = *beg;
							break;
						default:
							break;
						}
						i++;
					}
					if (intf.find(",") == intf.npos)
					{
						// Zuerst die Interface ID in der interface Tabelle suchen
						std::string intfID = "";
						if (iname != intf)
						{
							std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
							iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
							std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
							if (!result.empty())
							{
								intfID = result[0][0];
							}
							else
							{
								// Im Cat6k Fall keine Fehlerausgabe, da sonst zu viele Fehler angezeigt werden.  
								// Grund: Die Tabellenansicht am Cat6k beinhaltet viele nicht-Interface bezogene MAC Adressen. Für die gibt es dann unzählige Fehler
								//
								//std::string dbgA = "\n\n6305: Parser Error! Code 0036 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org";
								//schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
								//	WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
								//fehlerString += "\n" + dateiname + dbgA;

								continue;
							}
						}
						else 
						{
							intfID = iid;
						}

						iname = intf;
						iid = intfID;

						// Dann die Neighbor Tabelle füllen
						std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr) VALUES (NULL, '";
						iString2 += maca + "');";
						dieDB->query(iString2.c_str());

						// Dann die nlink Tabelle
						std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
						iString3 += intfID + ");";
						dieDB->query(iString3.c_str());
					}
				}
			}
		}
		else if (typePos4k < ganzeDatei.npos)
		{
			// Cat4k
			aktPos1 = ganzeDatei.find("-------+-", campos1);
			aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1)+1;
			aktPos2 = ganzeDatei.find("Multicast Entries", aktPos1);
			std::string camString = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			std::string camZeile = "";

			std::string vlan = "";
			std::string maca = "";
			std::string intf = "";

			aktPos1 = 0;
			aktPos2 = 0;


			for (; aktPos2 < camString.npos;)
			{
				aktPos2 = camString.find_first_of("\r\n", aktPos1 + 1);
				camZeile = camString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
				aktPos1 = aktPos2;
				if (camZeile.size() > 35)
				{
					int i = 0;
					boost::char_separator<char> sep(" \t");
					boost::tokenizer<boost::char_separator<char>> tok(camZeile, sep);
					for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 0:
							vlan = *beg;
							break;
						case 1:
							maca = *beg;
							break;
						case 4:
							intf = intfNameChange(*beg);
							break;
						default:
							break;
						}
						i++;
					}
					if (intf.find("Switch") == intf.npos)
					{
						// Zuerst die Interface ID in der interface Tabelle suchen
						std::string intfID = "";
						if (iname != intf)
						{
							std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
							iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
							std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
							if (!result.empty())
							{
								intfID = result[0][0];
							}
							else
							{
								std::string dbgA = "\n6406: Parser Error! Code 0037 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
								dbgA += iString;
								schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
									WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
								fehlerString += "\n" + dateiname + dbgA;
								continue;
							}
						}
						else 
						{
							intfID = iid;
						}

						iname = intf;
						iid = intfID;


						// Dann die Neighbor Tabelle füllen
						std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr) VALUES (NULL, '";
						iString2 += maca + "');";
						dieDB->query(iString2.c_str());

						// Dann die nlink Tabelle
						std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
						iString3 += intfID + ");";
						dieDB->query(iString3.c_str());
					}
				}
			}
		}
		else if (typeXLPos < ganzeDatei.npos)
		{
			aktPos1 = ganzeDatei.find("------", campos1);
			aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1) + 1;
			aktPos2 = ganzeDatei.find("#", aktPos1);
			std::string camString = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			std::string camZeile = "";

			std::string vlan = "";
			std::string maca = "";
			std::string intf = "";

			aktPos1 = 0;
			aktPos2 = 0;

			for (; aktPos2 < camString.npos;)
			{
				aktPos2 = camString.find_first_of("\r\n", aktPos1 + 1);
				camZeile = camString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
				aktPos1 = aktPos2;
				if (camZeile.size() > 45)
				{
					int i = 0;

					boost::char_separator<char> sep(" \t");
					boost::tokenizer<boost::char_separator<char>> tok(camZeile, sep);

					for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 0:
							maca = *beg;
							break;
						case 2:
							vlan = *beg;
							break;
						case 3:
							intf = intfNameChange(*beg);
							break;
						default:
							break;
						}
						i++;
					}
					// Zuerst die Interface ID in der interface Tabelle suchen
					std::string intfID = "";
					if (iname != intf)
					{
						std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
						iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
						std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
						if (!result.empty())
						{
							intfID = result[0][0];
						}
						else
						{
							std::string dbgA = "\n6406: Parser Error! Code 0038 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
							dbgA += iString;
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
								WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
							fehlerString += "\n" + dateiname + dbgA;
							continue;
						}
					}
					else 
					{
						intfID = iid;
					}

					iname = intf;
					iid = intfID;


					// Dann die Neighbor Tabelle füllen
					std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr) VALUES (NULL, '";
					iString2 += maca + "');";
					dieDB->query(iString2.c_str());

					// Dann die nlink Tabelle
					std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
					iString3 += intfID + ");";
					dieDB->query(iString3.c_str());
				}
			}
		}
		else
		{
			// anderer Switch
			aktPos1 = ganzeDatei.find("----    ", campos1);
			aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1)+1;
			aktPos2 = ganzeDatei.find("Total Mac Addresses", aktPos1);
			std::string camString = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			std::string camZeile = "";

			std::string vlan = "";
			std::string maca = "";
			std::string intf = "";

			aktPos1 = 0;
			aktPos2 = 0;

			for (; aktPos2 < camString.npos;)
			{
				aktPos2 = camString.find_first_of("\r\n", aktPos1 + 1);
				camZeile = camString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
				aktPos1 = aktPos2;
				if (camZeile.size() > 35)
				{
					int i = 0;
					boost::char_separator<char> sep(" \t");
					boost::tokenizer<boost::char_separator<char>> tok(camZeile, sep);

					for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 0:
							vlan = *beg;
							break;
						case 1:
							maca = *beg;
							break;
						case 3:
							intf = *beg;
							break;
						default:
							break;
						}
						i++;
					}
					if (intf.find("CPU") == intf.npos)
					{
						// Zuerst die Interface ID in der interface Tabelle suchen
						std::string intfID = "";
						if (iname != intf)
						{
							std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
							iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
							std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
							if (!result.empty())
							{
								intfID = result[0][0];
							}
							else
							{
								std::string dbgA = "\n6406: Parser Error! Code 0039 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
								dbgA += iString;
								schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
									WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
								fehlerString += "\n" + dateiname + dbgA;
								continue;
							}
						}
						else 
						{
							intfID = iid;
						}

						iname = intf;
						iid = intfID;


						// Dann die Neighbor Tabelle füllen
						std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr) VALUES (NULL, '";
						iString2 += maca + "');";
						dieDB->query(iString2.c_str());

						// Dann die nlink Tabelle
						std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
						iString3 += intfID + ");";
						dieDB->query(iString3.c_str());
					}
				}
			}
		}
	}


}


std::string WkmParserCisco::intfNameChange(std::string intfname)
{
	size_t ipos1 = 0;
	if ((ipos1 = intfname.find("TenGigabitEthernet")) != intfname.npos)
	{
		intfname.replace(ipos1, 18, "Te");
	}
	else if ((ipos1 = intfname.find("GigabitEthernet")) != intfname.npos)
	{
		intfname.replace(ipos1, 15, "Gi");
	}
	else if ((ipos1 = intfname.find("FastEthernet")) != intfname.npos)
	{
		intfname.replace(ipos1, 12, "Fa");
	}
	else if ((ipos1 = intfname.find("Ethernet")) != intfname.npos)
	{
		intfname.replace(ipos1, 8, "Eth");
	}
	else if ((ipos1 = intfname.find("Vethernet")) != intfname.npos)
	{
		intfname.replace(ipos1, 9, "Veth");
	}
	else if ((ipos1 = intfname.find("ethernet")) != intfname.npos)
	{
		intfname.replace(ipos1, 8, "Eth");
	}
	else if ((ipos1 = intfname.find("Port-channel")) != intfname.npos)
	{
		intfname.replace(ipos1, 12, "Po");
	}
	else if ((ipos1 = intfname.find("port-channel")) != intfname.npos)
	{
		intfname.replace(ipos1, 12, "Po");
	}
	else if ((ipos1 = intfname.find("Management")) != intfname.npos)
	{
		intfname.replace(ipos1, 10, "Ma");
	}
	else if ((ipos1 = intfname.find("oopback")) != intfname.npos)
	{
		intfname.replace(ipos1-1, 8, "Lo");
	}
	else if ((ipos1 = intfname.find("Tunnel")) != intfname.npos)
	{
		intfname.replace(ipos1, 6, "Tu");
	}

	// Etwaige Leerzeichen löschen
	boost::algorithm::trim(intfname);

	return intfname;
}


void WkmParserCisco::pixOSARPParser()
{
	zeitStempel("\tpixOSARPParser START");

	std::string arpString = ganzeDatei.substr(pos2, pos3-pos2);

	std::string arpZeile = "";

	std::string ipa = "";
	std::string maca = "";
	std::string intf = "";

	aktPos1 = 0;
	aktPos2 = 0;

	for (; aktPos2 < arpString.npos;)
	{
		aktPos2 = arpString.find_first_of("\r\n", aktPos1 + 1);
		arpZeile = arpString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
		aktPos1 = aktPos2;
		if (arpZeile.size() > 35)
		{
			int i = 0;

			boost::char_separator<char> sep(" \t");
			boost::tokenizer<boost::char_separator<char>> tok(arpZeile, sep);

			for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				switch (i)
				{
				case 0:
					intf = *beg;
					break;
				case 1:
					ipa = *beg;
					break;
				case 2:
					maca = *beg;
					break;
				default:
					break;
				}
				i++;
			}
			// Zuerst die Interface ID in der interface Tabelle suchen
			std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.nameif LIKE '";
			iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
			std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
			std::string intfID = "";
			if (!result.empty())
			{
				intfID = result[0][0];
			}
			else
			{
				std::string dbgA = "\n6406: Parser Error! Code 0040 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
					WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
				fehlerString += "\n" + dateiname + dbgA;
				continue;
			}

			// Dann die Neighbor Tabelle füllen
			std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr, l3_addr) VALUES (NULL, '";
			iString2 += maca + "', '" + ipa + "');";
			dieDB->query(iString2.c_str());

			// Dann die nlink Tabelle
			std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
			iString3 += intfID + ");";
			dieDB->query(iString3.c_str());
		}
	}
}


void WkmParserCisco::pixOSInterfaceParser()
{
	zeitStempel("\tpixOSInterfaceParser START");

	// Alle Interfaces sind interessant
	size_t intfpos = ganzeDatei.find("Interface ", pos1);
	std::string intString = "Interface ";
	if (intfpos > pos2)
	{
		// PIX 6.x
		intfpos = ganzeDatei.find("interface ", pos1);
		intString = "interface ";
	}

	while (intfpos < pos2)
	{
		// Interface Name
		aktPos1 = intfpos;
		intfpos = ganzeDatei.find(intString, intfpos + 10);
		aktPos1 += 10;

		std::string intfStatus = "UP";
		if ((aktPos2 = ganzeDatei.find("is up, line protocol is up", aktPos1)) < intfpos)
		{
			intfStatus = "UP";
		}
		else if ((aktPos2 = ganzeDatei.find("is administratively down, line", aktPos1)) < intfpos)
		{
			intfStatus = "DISABLED";
		}
		else if ((aktPos2 = ganzeDatei.find("is down, line protocol", aktPos1)) < intfpos)
		{
			intfStatus = "DOWN";
		}
		aktPos2 -= 4;
		aktPos2 = ganzeDatei.rfind("\"", aktPos2);

		std::string intfName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1-1);
		intfName = intfNameChange(intfName);

		aktPos1 = aktPos2;
		aktPos2 = ganzeDatei.find("\"", aktPos1+1);
		aktPos1++;
		std::string nameif = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		// Herausfinden, ob Subinterface
		std::string intfHauptIntf = "";
		if (intfName.find(".") != intfName.npos)
		{
			intfHauptIntf = intfName.substr(0, intfName.find("."));
		}

		std::string l2Addr = "";
		std::string ipAddress = "";
		std::string ipAddressMask = "";

		// Check ob Version <7.0
		bool pixVer6 = false;
		if ((aktPos1 = ganzeDatei.find(", address is", aktPos2)) < intfpos)
		{
			pixVer6 = true;
			// Hardware Adresse
			aktPos1 += 13;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			l2Addr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			// IP Adresse falls vorhanden
			if ((aktPos1 = ganzeDatei.find("IP address ", aktPos2)) < intfpos)
			{
				aktPos1 += 11;
				aktPos2 = ganzeDatei.find(", ", aktPos1);
				ipAddress = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				aktPos1 = ganzeDatei.find(", subnet mask ", aktPos2) + 14;				

				if (aktPos1 < intfpos)
				{
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					ipAddressMask = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				}
				else
				{
					ipAddress = "";
					ipAddressMask = "";
				}

				if (ipAddress.find("unassigned") != ipAddress.npos)
				{
					ipAddress = "";
					ipAddressMask = "";
				}
			}
		}


		// Duplex und Speed falls vorhanden
		std::string duplex = "";
		std::string speed = "";
		if ((aktPos1 = ganzeDatei.find("Duplex", aktPos2)) < intfpos)
		{
			std::size_t tempPos = ganzeDatei.find_first_of("\r\n", aktPos1);
			aktPos1 = ganzeDatei.find("(", aktPos1);
			aktPos2 = ganzeDatei.find(")", aktPos1);
			if (aktPos1 < tempPos)
			{
				aktPos1 += 1;
				duplex = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			aktPos1 = ganzeDatei.find("(", aktPos2+3);
			aktPos2 = ganzeDatei.find(")", aktPos1);
			if (aktPos1 < tempPos)
			{
				aktPos1 += 1;
				speed = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
		}
		else if ((aktPos1 = ganzeDatei.find("duplex", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			size_t tpos1 = ganzeDatei.rfind("(", aktPos1);
			size_t tpos2 = ganzeDatei.rfind(" ", aktPos1)-2;
			if (tpos2<tpos1)
			{
				// ASA 8.4 Te und Po Interfaces
				aktPos1 = tpos1+1;
				aktPos2 = ganzeDatei.find(")", aktPos1);
				duplex = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = ganzeDatei.find("(", aktPos2)+1;
				aktPos2 = ganzeDatei.find(")", aktPos1);
				speed = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			else
			{
				aktPos1 = tpos2;
				aktPos1 = ganzeDatei.rfind(" ", aktPos1);
				duplex = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = ganzeDatei.rfind("BW", aktPos1)+3;
				aktPos2 = ganzeDatei.find(" ", aktPos1);
				speed = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}



		}
		// Interface Type rausfinden
		std::string intfType = intfTypeCheck(intfName, speed);
		std::string phl = intfPhlCheck(intfName, speed);


		// VLAN ID
		std::string vlanid = "";
		int vlanID = 0;
		if ((aktPos1 = ganzeDatei.find("VLAN identifier ", aktPos2)) < intfpos)
		{
			aktPos1 += 16;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			vlanid = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				vlanID = boost::lexical_cast<int>(vlanid);
			}
			catch (boost::bad_lexical_cast &)
			{
				vlanID = 0;			
			}

			// VLAN Tabelle befüllen
			std::string iString = "INSERT INTO vlan (vlan_id, vlan) VALUES (NULL, ";
			iString += vlanid + ");";
			dieDB->query(iString.c_str());

			iString = "SELECT last_insert_rowid() FROM vlan;";
			std::vector<std::vector<std::string> > result2 = dieDB->query(iString.c_str());
			std::string vlID = "";
			if (!result2.empty())
			{
				vlID = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0007 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

			// vlan_has_device Tabelle befüllen
			iString = "INSERT INTO vlan_has_device (vlan_vlan_id, device_dev_id) VALUES (";
			iString += vlID + "," + dev_id + ");";
			dieDB->query(iString.c_str());
		}

		// Description, falls vorhanden
		std::string descr = "";
		if ((aktPos1 = ganzeDatei.find("Description: ", aktPos2)) < intfpos)
		{
			aktPos1 += 13;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			descr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			while (descr.find_first_of("'\"&") != descr.npos)
			{
				descr.replace(descr.find_first_of("'\"&"), 1, "+");
			}
		}


		std::queue<std::string> chMembers;
		if (!pixVer6)
		{
			// Hardware Adresse, falls vorhanden
			if ((aktPos1 = ganzeDatei.find("MAC address ", aktPos2)) < intfpos)
			{
				l2Addr = ganzeDatei.substr(aktPos1+12, 14);
			}


			// IP Adresse falls vorhanden
			if ((aktPos1 = ganzeDatei.find("IP address ", aktPos2)) < intfpos)
			{
				aktPos1 += 11;
				std::size_t tempPos = aktPos2;
				aktPos2 = ganzeDatei.find(", ", aktPos1);
				ipAddress = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				aktPos1 = ganzeDatei.find(", subnet mask ", aktPos2) + 14;				

				if (aktPos1 < intfpos)
				{
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					ipAddressMask = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				}
				else
				{
					ipAddress = "";
					ipAddressMask = "";
					aktPos2 = tempPos;
				}

				if (ipAddress.find("unassigned") != ipAddress.npos)
				{
					ipAddress = "";
					ipAddressMask = "";
					aktPos2 = tempPos;
				}

			}

			// Channel Member
			std::string chMem = "";
			if ((aktPos1 = ganzeDatei.find("Members in this channel:", aktPos2)) < intfpos)
			{
				aktPos1 = ganzeDatei.find("Active: ", aktPos1);
				aktPos1 = ganzeDatei.find_first_not_of(" ", aktPos1+8);
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				chMem = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if (chMem != "")
			{
				boost::char_separator<char> sep(" ");
				boost::tokenizer<boost::char_separator<char>> tok(chMem, sep);

				for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					chMembers.push(*beg);
				}
			}

		}

		// Fehler
		int err = 0;
		int tempErr = 0;

		std::string iCrc = "";
		std::string iFrame = "";
		std::string iOverrun = "";
		std::string iIgnored = "";
		std::string iWatchdog = "";
		std::string iPause = "";
		std::string iDribbleCondition = "";
		std::string l2decodeDrops = "";

		// Encapsulation
		std::string encaps = "";
		// Input Throttles
		std::string throttles = "";
		// Output Buffer Errors
		std::string obuffer = "";

		// Input Buffer errors
		std::string ibuffer = "";
		if ((aktPos1 = ganzeDatei.find(" no buffer", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			ibuffer = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(ibuffer);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Input Runts
		std::string runts = "";
		if ((aktPos1 = ganzeDatei.find(" runts", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			runts = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(runts);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Input Giants
		std::string giants = "";
		if ((aktPos1 = ganzeDatei.find(" giants", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			giants = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(giants);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Input Erros
		std::string ierrors = "";
		if ((aktPos1 = ganzeDatei.find(" input errors", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind("\t", aktPos2 - 1) + 1;
			ierrors = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(ierrors);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}
		if ((aktPos1 = ganzeDatei.find(" CRC", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iCrc = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" frame", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iFrame = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" overrun", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iOverrun = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" ignored", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			iIgnored = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" pause input", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind("\t", aktPos2 - 1) + 1;
			iPause = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" L2 decode drops", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind("\t", aktPos2 - 1) + 1;
			l2decodeDrops = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		std::string oerrors = "";
		std::string oCollisions = "";
		std::string oBabbles = "";
		std::string oLateColl = "";
		std::string oDeferred = "";
		std::string oLostCarrier = "";
		std::string oNoCarrier = "";
		std::string oPauseOutput = "";
		std::string oBufferSwapped = "";
		// Output underruns
		std::string underrun = "";
		if ((aktPos1 = ganzeDatei.find(" underruns", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
			underrun = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(underrun);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		if ((aktPos1 = ganzeDatei.find(" pause output", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind("\t", aktPos2 - 1) + 1;
			oPauseOutput = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Output errors
		if ((aktPos1 = ganzeDatei.find(" output errors", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind("\t", aktPos1 - 1) + 1;
			oerrors = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(oerrors);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}
		if ((aktPos1 = ganzeDatei.find(" collisions", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oCollisions = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Interface Resets
		std::string resets = "";
		if ((aktPos1 = ganzeDatei.find(" interface resets", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
			resets = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			try
			{
				tempErr = boost::lexical_cast<int>(resets);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}
		if ((aktPos1 = ganzeDatei.find(" late collisions", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind("\t", aktPos2 - 1) + 1;
			oLateColl = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" deferred", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oDeferred = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find(" deferred", aktPos2)) < intfpos)
		{
			aktPos2 = aktPos1;
			aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
			oDeferred = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Input Bits/s
		std::string ibits = "";
		if ((aktPos1 = ganzeDatei.find("1 minute input rate ", aktPos2)) < intfpos)
		{
			aktPos1 += 20;
			aktPos2 = ganzeDatei.find(" ", aktPos1);
			ibits = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Output Bits/s
		std::string obits = "";
		if ((aktPos1 = ganzeDatei.find("1 minute output rate ", aktPos2)) < intfpos)
		{
			aktPos1 += 21;
			aktPos2 = ganzeDatei.find(" ", aktPos1);
			obits = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		std::string iString = "INSERT INTO interfaces (intf_id,intfName,nameif,intfType,phl,macAddress,ipAddress,subnetMask,duplex,speed,status,description,l2l3,";
		iString += "errLvl,loadLvl,channel_intf_id,iCrc,iFrame,iOverrun,iIgnored,iWatchdog,iPause,iDribbleCondition,ibuffer,l2decodeDrops,runts,giants,";
		iString += "throttles,ierrors,underrun,oerrors,oCollisions,oBabbles,oLateColl,oDeferred,oLostCarrier,oNoCarrier,oPauseOutput,oBufferSwapped,";
		iString += "resets,obuffer)";
		iString += " VALUES(NULL, '" + intfName + "','" + nameif + "','" + intfType + "'," + phl + ",'" + l2Addr + "','" + ipAddress + "','" + ipAddressMask + "','" + duplex + "','" + speed;
		iString += "','" + intfStatus + "','" + descr + "','L3'," + boost::lexical_cast<std::string>(err) + ",NULL,NULL,'" + iCrc + "','" + iFrame + "','" + iOverrun + "','" + iIgnored;
		iString +=  + "','" + iWatchdog + "','" + iPause + "','" + iDribbleCondition + "','" + ibuffer + "','"  + l2decodeDrops + "','" + runts + "','" + giants + "','" + throttles;
		iString +=  + "','" + ierrors + "','" + underrun + "','" + oerrors + "','" + oCollisions + "','" + oBabbles + "','" + oLateColl + "','" + oDeferred;
		iString +=  + "','" + oLostCarrier + "','" + oNoCarrier + "','" + oPauseOutput + "','" + oBufferSwapped + "','" + resets + "','" + obuffer + "');";
		dieDB->query(iString.c_str());

		std::string sString = "SELECT last_insert_rowid() FROM interfaces;";
		std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
		if (!result2.empty())
		{
			intf_id = result2[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0008 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// Die Channel Members markieren
		while (chMembers.size())
		{
			std::string iString3 = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			iString3 += chMembers.front() + "' AND devInterface.device_dev_id=" + dev_id +";";
			std::vector<std::vector<std::string> > result = dieDB->query(iString3.c_str());

			std::string chMemID = "";
			if (!result.empty())
			{
				chMemID = result[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0003 - Contact wktools@spoerr.org\n";
				dbgA += iString3;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

			std::string iString4 = "UPDATE interfaces SET channel_intf_id=last_insert_rowid() WHERE intf_id = " + chMemID + ";";
			dieDB->query(iString4.c_str());

			chMembers.pop();
		}

		// Das Hauptinterface finden und dann die Subinterfaces markieren
		if (intfHauptIntf != "")
		{
			std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			sString += intfHauptIntf + "' AND devInterface.device_dev_id=" + dev_id +";";
			std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

			if (!result.empty())
			{
				std::string iString4 = "UPDATE interfaces SET channel_intf_id=" + result[0][0] + " WHERE intf_id = last_insert_rowid();";
				dieDB->query(iString4.c_str());
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0009 - Contact wktools@spoerr.org\n";
				dbgA += sString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

		}

		// DevInterface Tabelle befüllen
		std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
		dieDB->query(iString2.c_str());

		// ipSubnet befüllen
		if (ipAddress != "")
		{
			ipNetzeEintragen(ipAddress, ipAddressMask);
		}
	}
}


bool WkmParserCisco::insertDevice()
{
	// Prüfen, ob es den Hostnamen schon gibt. Wenn ja, dann den neuen Hostnamen ändern (um "-1" erweitern)
	std::string sString = "";
	if (!rControlEmpty)
	{
		sString = "SELECT hostname FROM device WHERE hostname LIKE '" + hostname + "' AND device.dev_id > (SELECT MAX (dev_id) FROM rControl)"; 
	}
	else
	{
		sString = "SELECT hostname FROM device WHERE hostname LIKE '" + hostname + "'"; 
	}

	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
	if (!result.empty())
	{
		hostname = hostname + "-1";
	}

	std::string iString = "INSERT INTO device (dev_id,type,hwtype,hostname,confReg) VALUES(NULL, " + devType + ", " + devType + ", '" + hostname + "', '" + confReg + "');";
	dieDB->query(iString.c_str());

	sString = "SELECT last_insert_rowid() FROM device;";
	result = dieDB->query(sString.c_str());
	if (result.empty())
	{
		std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return false;
	}
	else
	{
		dev_id = result[0][0];
		return true;
	}
}


bool WkmParserCisco::insertHardware()
{
	boost::algorithm::trim(hwPos);
	boost::algorithm::trim(modell);
	boost::algorithm::trim(hwDescription);
	boost::algorithm::trim(chassisSN);
	boost::algorithm::trim(hwRevision);
	boost::algorithm::trim(hwMem);
	boost::algorithm::trim(hwBootfile);
	boost::algorithm::trim(version);
	std::string iString = "INSERT INTO hwInfo (hwinf_id,module,type,description,sn,hwRevision,memory,bootfile,sw_version) ";
	iString += "VALUES(NULL, '" + hwPos + "', '" + modell + "', '" + hwDescription + "', '" + chassisSN + "', '" + hwRevision + "', '" + hwMem + "', '" + hwBootfile + "', '" + version + "');";
	dieDB->query(iString.c_str());

	std::string hw_id = "";
	std::string sString = "SELECT last_insert_rowid() FROM hwInfo;";
	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
	if (result.empty())
	{
		std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return false;
	}
	else
	{
		hw_id = result[0][0];
		if (hwPos == "box")
		{
			hwInfo_id = hw_id;
		}

		// hwLink befüllen
		std::string iString2 = "INSERT INTO hwlink (hwInfo_hwinf_id, device_dev_id) VALUES (" + hw_id + "," + dev_id + ");";
		dieDB->query(iString2.c_str());

	}
	return true;
}


bool WkmParserCisco::insertLic()
{
	std::string iString = "INSERT INTO license (license_id, name, status, type, nextBoot, cluster) ";
	iString += "VALUES(NULL, '" + licName + "', '" + licStatus + "', '" + licType + "', '" + licNextBoot + "', " + licCluster + ");";
	dieDB->query(iString.c_str());

	std::string lic_id = "";
	std::string sString = "SELECT last_insert_rowid() FROM license;";
	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
	if (result.empty())
	{
		std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return false;
	}
	else
	{
		lic_id = result[0][0];

		// liclink befüllen
		iString = "INSERT INTO liclink (license_license_id, device_dev_id) VALUES (" + lic_id + "," + dev_id + ");";
		dieDB->query(iString.c_str());

	}
	return true;
}


std::string WkmParserCisco::zeitStempel(std::string info)
{
	time_t zeit = time(NULL);
	char *fehlerZeit = asctime(localtime(&zeit));
	fehlerZeit[24] = 0x00;

	if (debugAusgabe)
	{
		std::string dbgA = "\n6614: ";
		dbgA += fehlerZeit;
		dbgA += " - " + info ;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6614", WkLog::WkLog_BLAU);
	}

	return fehlerZeit;
}


std::string WkmParserCisco::intfTypeCheck(std::string intfName, std::string intfSpeed)
{
	std::string intfType = "";
	size_t ipos1 = 0;
	if ((ipos1 = intfName.find("Te")) != intfName.npos)
	{
		intfType = "Te";
	}
	else if ((ipos1 = intfName.find("Vlan")) != intfName.npos)
	{
		intfType = "Vlan";
	}
	else if ((ipos1 = intfName.find("Po")) != intfName.npos)
	{
		intfType = "Channel";
	}
	else if ((ipos1 = intfName.find("Veth")) != intfName.npos)
	{
		intfType = "Veth";
	}
	else if ((ipos1 = intfSpeed.find("100000")) != intfSpeed.npos)
	{
		intfType = "Fa";
	}
	else if ((ipos1 = intfSpeed.find("10000")) != intfSpeed.npos)
	{
		intfType = "Eth";
	}
	else if ((ipos1 = intfSpeed.find("10 Gb/s")) != intfSpeed.npos)
	{
		intfType = "Te";
	}
	else if ((ipos1 = intfSpeed.find("1Gbps")) != intfSpeed.npos)
	{
		intfType = "Ge";
	}
	else if ((ipos1 = intfSpeed.find("10Gbps")) != intfSpeed.npos)
	{
		intfType = "Te";
	}
	else if ((ipos1 = intfSpeed.find("1 Gb/s")) != intfSpeed.npos)
	{
		intfType = "Ge";
	}
	else if ((ipos1 = intfSpeed.find("1000")) != intfSpeed.npos)
	{
		intfType = "Ge";
	}
	else if ((ipos1 = intfSpeed.find("100")) != intfSpeed.npos)
	{
		intfType = "Fa";
	}
	else if ((ipos1 = intfSpeed.find("10")) != intfSpeed.npos)
	{
		intfType = "Eth";
	}
	else if ((ipos1 = intfName.find(".")) != intfName.npos)		// Subinterface
	{
		intfType = "SubIntf";
	}
	else if ((ipos1 = intfName.find("Tu")) != intfName.npos)
	{
		intfType = "Tunnel";
	}
	else if ((ipos1 = intfName.find("BVI")) != intfName.npos)
	{
		intfType = "BVI";
	}
	else if ((ipos1 = intfName.find("NVI")) != intfName.npos)
	{
		intfType = "NVI";
	}
	else if ((ipos1 = intfName.find("Fa")) != intfName.npos)
	{
		intfType = "Fa";
	}
	else if ((ipos1 = intfName.find("Eth")) != intfName.npos)
	{
		intfType = "Eth";
	}
	else if ((ipos1 = intfName.find("Ge")) != intfName.npos)
	{
		intfType = "Ge";
	}


	return intfType;
}


std::string WkmParserCisco::intfPhlCheck(std::string intfName, std::string intfSpeed)
{
	// 0...Logisch
	// 1...Physikalisch, einschließlich Po
	// 2...VSL
	// 3...Veth

	std::string intfType = "1";
	size_t ipos1 = 0;
	if ((ipos1 = intfName.find(".")) != intfName.npos)		// Subinterface
	{
		intfType = "0";
	}
	else if ((ipos1 = intfName.find("Te")) != intfName.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfName.find("Veth")) != intfName.npos)
	{
		intfType = "3";
	}
	else if ((ipos1 = intfName.find("Vlan")) != intfName.npos)
	{
		intfType = "0";
	}
	else if ((ipos1 = intfName.find("VLAN")) != intfName.npos)
	{
		intfType = "0";
	}
	else if ((ipos1 = intfName.find("Po")) != intfName.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("10 Gb/s")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("10000")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("1 Gb/s")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("1000")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("100")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("10")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("Gbps")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfSpeed.find("Mbps")) != intfSpeed.npos)
	{
		intfType = "1";
	}
	else if ((ipos1 = intfName.find("Tu")) != intfName.npos)
	{
		intfType = "0";
	}
	else if ((ipos1 = intfName.find("BVI")) != intfName.npos)
	{
		intfType = "0";
	}
	else if ((ipos1 = intfName.find("NVI")) != intfName.npos)
	{
		intfType = "0";
	}

	return intfType;
}


void WkmParserCisco::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
	if (logAusgabe != NULL)
	{
		WkLog::evtData evtDat;
		evtDat.farbe = farbe;
		evtDat.format = format;
		evtDat.groesse = groesse;
		evtDat.logEintrag = logEintrag;
		evtDat.logEintrag2 = log2;
		evtDat.type = type;

		LogEvent evt(EVT_LOG_MELDUNG);
		evt.SetData(evtDat);
		wxPostEvent(logAusgabe, evt);
	}
}


void WkmParserCisco::iosCDPParser()
{
	zeitStempel("\tiosCDPParser START");

	// Nur Interfaces, die UP sind, sind interessant
	size_t cdppos = ganzeDatei.find("-------------------------", pos5);

	while (cdppos < pos6)
	{
		// Der nächste Nachbar
		aktPos1 = cdppos;
		cdppos = ganzeDatei.find("-------------------------", cdppos + 10);

		std::string nName = "";
		std::string alternatenName = "";
		if ((aktPos1 = ganzeDatei.find("Device ID: ", aktPos1)) < cdppos)
		{
			aktPos1 += 11;
			aktPos2 = ganzeDatei.find_first_of("\r\n.( ", aktPos1);
			nName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			alternatenName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		std::string nT = "";
		std::string nType = "99";
		std::string platform = "";
		std::string ipa = "";

		// TempPos einführen, da die IP Adresse manchmal ganz am Ende steht.
		size_t tempPos = aktPos1;

		if ((aktPos1 = ganzeDatei.find("IP address:", aktPos1)) < cdppos)
		{
			aktPos1 += 12;
			aktPos2 = ganzeDatei.find_first_of(",;\r\n", aktPos1);
			ipa = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = ganzeDatei.find("IPv4 Address:", aktPos2)) < cdppos)
		{
			aktPos1 += 14;
			aktPos2 = ganzeDatei.find_first_of(",;\r\n", aktPos1);
			ipa = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}


		if ((aktPos1 = ganzeDatei.find("Platform: ", tempPos)) < cdppos)
		{
			aktPos1 += 10;
			aktPos2 = ganzeDatei.find_first_of(",;\r\n", aktPos1);
			platform = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			if (platform.find("AIR") != platform.npos)
			{
				nType = "4";
			}
		}

		if ((aktPos1 = ganzeDatei.find("Capabilities: ", tempPos)) < cdppos)
		{
			aktPos1 += 14;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			nT = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			if (nT.find("Host Phone") != nT.npos)
			{
				nType = "11";
				boost::algorithm::to_upper(nName);
				boost::algorithm::to_upper(alternatenName);
			}
			else if (nT.find("Host") != nT.npos)
			{
				if (platform.find("ATA") != platform.npos)
				{
					nType = "13";
				}
				else if (platform.find("CIVS") != platform.npos)
				{
					nType = "14";
				}
				else if (platform.find("CP-") != platform.npos)
				{
					nType = "11";
				}
				else
				{
					nType = "12";
				}
			}
			else if (nT.find("Router Switch") != nT.npos)
			{
				nType = "5";
			}
			else if (nT.find("Switch") != nT.npos)
			{
				nType = "2";
			}
			else if (nT.find("Router") != nT.npos)
			{
				nType = "1";
			}
			else if (nT.find("Trans-Bridge") != nT.npos)
			{
				nType = "4";
			}
		}

		std::string nIntf = "";
		if ((aktPos1 = ganzeDatei.find("Interface: ", aktPos1)) < cdppos)
		{
			aktPos1 += 11;
			aktPos2 = ganzeDatei.find_first_of(",", aktPos1);
			nIntf = intfNameChange(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
		}

		std::string nNintf = "";
		if ((aktPos1 = ganzeDatei.find("Port ID (outgoing port): ", aktPos1)) < cdppos)
		{
			aktPos1 += 25;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			nNintf = intfNameChange(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
		}

		std::string nSWver = "";
		if ((aktPos1 = ganzeDatei.find("Version :", aktPos1)) < cdppos)
		{
			aktPos2 = aktPos1 + 9;
			if ((aktPos1 = ganzeDatei.find("Version:", aktPos2)) < cdppos)
			{
				aktPos2 = aktPos1 + 9;
			}
			else if ((aktPos1 = ganzeDatei.find("Cisco Internetwork Operating System Software", aktPos2)) < cdppos)
			{
				aktPos2 = aktPos1 + 45;
			}

			aktPos1 = ganzeDatei.find_first_not_of("\r\n ", aktPos2);
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			nSWver = intfNameChange(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
			if (nSWver.find("Version") != nSWver.npos)
			{
				std::size_t zwischenPos1 = nSWver.find("Version")+8;
				std::size_t zwischenPos2 = nSWver.find_first_of("\r\n ,", zwischenPos1);
				nSWver = nSWver.substr(zwischenPos1, zwischenPos2-zwischenPos1);
			}
			else if (nSWver.find("Release") != nSWver.npos)
			{
				std::size_t zwischenPos1 = nSWver.find("Release")+8;
				std::size_t zwischenPos2 = nSWver.find_first_of("\r\n ,", zwischenPos1);
				nSWver = nSWver.substr(zwischenPos1, zwischenPos2-zwischenPos1);
			}
		}


		// Interface ID auslesen
		std::string sString = "SELECT intf_id FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE intfName LIKE '" + nIntf + "' "; 
		sString += "AND devInterface.device_dev_id=" + dev_id + ";";
		std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
		std::string intRow = "";
		if (!result.empty())
		{
			intRow = result[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0010 - Contact wktools@spoerr.org\n";
			dbgA += sString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// Zuerst die CDP Tabelle befüllen
		std::string iString = "INSERT INTO cdp (hostname, nName, alternatenName, intf, nIntfIP, nIntf, type, platform, sw_version)";
		iString += " VALUES('" + hostname + "','" + nName + "','" + alternatenName + "','" + nIntf + "','" + ipa + "','" + nNintf + "'," + nType + ",'" + platform + "','" + nSWver + "');";
		dieDB->query(iString.c_str());

		sString = "SELECT last_insert_rowid() FROM cdp;";
		result = dieDB->query(sString.c_str());
		std::string cdpRow = "";
		if (!result.empty())
		{
			cdpRow = result[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0011 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// Und zum Schluss die cLink Tabelle befüllen
		iString = "INSERT INTO clink (interfaces_intf_id, cdp_cdp_id) VALUES (";
		iString += intRow + "," + cdpRow + ");";
		dieDB->query(iString.c_str());
	}
}


void WkmParserCisco::iosCDPSchreiber()
{
	// Nexus 7k Nachbarschaften:
	// Wenn ein Nexus 7k als Nachbar ist, dann werden die Connectivity Management Processor (CMP) als eigene Geräte angezeigt
	// -> Den cmp Zusatz vom Hostnamen zum Interface hängen
	std::string sString = "SELECT nName,alternatenName,nIntf FROM cdp WHERE nName LIKE '%-cmp%'";
	std::vector<std::vector<std::string>> cmpret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = cmpret.begin(); it < cmpret.end(); ++it)
	{
		std::string nName = it->at(0);
		std::size_t cmpPos = nName.find("-cmp");
		std::string cmpString = nName.substr(cmpPos);
		nName.erase(cmpPos);
		std::string anName = it->at(1);
		std::size_t cmpEndPos = anName.find_first_of(".(");
		if (cmpEndPos == anName.npos)
		{
			anName.erase(cmpPos);
		}
		else
		{
			anName.erase(cmpPos, cmpEndPos-cmpPos);
		}
		std::string nIntf = it->at(2) + cmpString;

		sString = "UPDATE cdp SET nIntf='" + nIntf + "'WHERE alternatenName LIKE '" + it->at(1) + "'";
		dieDB->query(sString.c_str());
		sString = "UPDATE cdp SET nName='" + nName + "'WHERE nName LIKE '" + it->at(0) + "'";
		dieDB->query(sString.c_str());
		sString = "UPDATE cdp SET alternatenName='" + anName + "'WHERE alternatenName LIKE '" + it->at(1) + "'";
		dieDB->query(sString.c_str());
	}

	// Nexus1010 Type gleichziehen -> Soll alles Host sein, nicht gemischt Host und Switch
	sString = "UPDATE cdp SET type=12 WHERE platform LIKE 'Nexus1010%' AND type=2";
	dieDB->query(sString.c_str());

	// AP Hostnamenproblem lösen:
	// Bei Verwendung von WLC's heißen alle APs ähnlich -> AP01.abc; AP01.abe; AP01.abf
	// Daher werden in der cdp Tabelle beide Namen verwendet -> nName (Name bis Punkt) und alternatenName (kompletter Name)
	// Jetzt muss überprüft werden, ob es mehrere Einträge für nName gibt, aber alternatenName unterschiedlich ist
	// In dem Fall wird alternatenName in die Device Tabelle geschrieben
	sString = "UPDATE OR REPLACE cdp SET nName=alternatenName WHERE nName IN ( ";
	sString += "SELECT nName FROM cdp WHERE nName IN (SELECT nName FROM cdp	";
	if (!rControlEmpty)
	{
		sString += "WHERE cdp_id > (SELECT MAX(cdp_id) FROM rControl) ";
	}
	sString += "GROUP BY nName HAVING (COUNT(nName) > 1)) ";
	if (!rControlEmpty)
	{
		sString += "AND cdp_id > (SELECT MAX(cdp_id) FROM rControl) ";
	}
	sString += "GROUP BY alternatenName HAVING (COUNT(alternatenName)>1) ";
	sString += "AND nName NOT IN (SELECT hostname FROM device) ";
	if (!rControlEmpty)
	{
		sString += "AND cdp_id > (SELECT MAX(cdp_id) FROM rControl) ";
	}
	sString += ") ";
	if (!rControlEmpty)
	{
		sString += "AND cdp_id > (SELECT MAX(cdp_id) FROM rControl) ";
	}
	dieDB->query(sString.c_str());

	// Alle per CDP gelernten Hosts, die nicht ausgelesen wurden, werden nun in die Devicetabelle und die anderen relevanten Tabellen eintragen
	// 1. Fehlende Hosts in die device Table schreiben
	std::string dType = "";
	sString = "SELECT DISTINCT nName,type,platform,sw_version,nIntfIP FROM cdp ";
	sString += "WHERE nName NOT IN ( ";
	sString += "SELECT hostname FROM device";
	if (!rControlEmpty)
	{
		sString += " WHERE dev_id > (SELECT MAX(dev_id) FROM rControl)";
	}
	sString += ") ";
	if (!rControlEmpty)
	{
		sString += " AND cdp_id > (SELECT MAX(cdp_id) FROM rControl)";
	}
	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Check, ob S/N im AlternatenName vorkommt (steht zwischen Klammern)
		chassisSN = "";
		std::string sString1 = "SELECT DISTINCT alternatenName FROM cdp WHERE nName LIKE '" + it->at(0) + "' ";
		if (!rControlEmpty)
		{
			sString1 += " AND cdp_id > (SELECT MAX(cdp_id) FROM rControl)";
		}
		std::vector<std::vector<std::string>> retAN = dieDB->query(sString1.c_str());
		if (retAN.empty())
		{
			continue;
		}
		std::string alternatenName = "";
		for(std::vector<std::vector<std::string>>::iterator itan = retAN.begin(); itan < retAN.end(); ++itan)
		{
			// Es kann sein, dass ein Gerät mit mehreren potentiellen S/Ns vorkommt
			alternatenName = itan->at(0);

			if (alternatenName.find("(") != alternatenName.npos)
			{
				size_t anPos1 = alternatenName.find("(") + 1;
				size_t anPos2 = alternatenName.find(")", anPos1);
				std::string tempsn = alternatenName.substr(anPos1, anPos2-anPos1);
				if (chassisSN.find(tempsn) == chassisSN.npos)
				{
					chassisSN += tempsn + ", ";
				}
				alternatenName.erase(anPos1-1);
				sString1 = "UPDATE cdp SET alternatenName='" + alternatenName + "' WHERE alternatenName LIKE '" + itan->at(0) + "'";
				dieDB->query(sString1.c_str());
			}
		}

		sString1 = "SELECT intf_id FROM interfaces WHERE ipAddress LIKE '" + it->at(4) + "'" ;
		if (!rControlEmpty)
		{
			sString1 += " AND intf_id > (SELECT MAX(intf_id) FROM rControl)";
		}
		std::vector<std::vector<std::string>> retIP = dieDB->query(sString1.c_str());
		if (!retIP.empty())
		{
			continue;
		}

		dType = it->at(1);
		std::string iString = "INSERT INTO device (dev_id,type,dataSource,hostname) VALUES(NULL, " + it->at(1) + ",1, '" + it->at(0) + "');";
		dieDB->query(iString.c_str());

		sString1 = "SELECT last_insert_rowid() FROM device;";
		std::vector<std::vector<std::string> > ret1 = dieDB->query(sString1.c_str());
		dev_id = "";
		if (!ret1.empty())
		{
			dev_id = ret1[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0012 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}
		// HW Tabelle befüllen
		hwPos = "box";
		modell = it->at(2);
		version = it->at(3);
		hwBootfile = "";
		hwMem = "";
		hwDescription = "";
		hwRevision = "";
		insertHardware();

		// 2. Fehlende Interfaces in die interfaces Table schreiben
		sString1 = "SELECT DISTINCT nIntfIP,nIntf,cdp_id FROM cdp WHERE nName LIKE '" + it->at(0) + "' ";
		if (!rControlEmpty)
		{
			sString1 += "AND cdp_id > (SELECT MAX(cdp_id) FROM rControl);";
		}
		std::vector<std::vector<std::string> > ret2 = dieDB->query(sString1.c_str());
		for(std::vector<std::vector<std::string>>::iterator it1 = ret2.begin(); it1 < ret2.end(); ++it1)
		{
			// MAC Adresse rausfinden
			std::string macAddress = "";
			sString1 = "SELECT l2_addr FROM neighbor WHERE l3_addr LIKE '" + it1->at(0) + "' ";
			if (!rControlEmpty)
			{
				sString1 += "AND neighbor_id > (SELECT MAX(neighbor_id) FROM rControl);";
			}
			std::vector<std::vector<std::string> > retMac = dieDB->query(sString1.c_str());
			if (!retMac.empty())
			{
				macAddress = retMac[0][0];
			}

			std::string cdp_id = it1->at(2);
			iString = "INSERT INTO interfaces (intfName, macAddress, ipAddress, subnetMask, status, l2l3) VALUES ('" + it1->at(1) + "', '" + macAddress + "', '" + it1->at(0) + "',' ', 'UP', 'L3');";
			dieDB->query(iString.c_str());

			sString1 = "SELECT last_insert_rowid() FROM interfaces;";
			std::vector<std::vector<std::string> > ret3 = dieDB->query(sString1.c_str());
			std::string intf_id = "";
			if (!ret3.empty())
			{
				intf_id = ret3[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0013 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}


			// 3. devInterface und clink Tabelle updaten
			iString = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (";
			iString += intf_id + "," + dev_id + ");";
			dieDB->query(iString.c_str());

			iString = "INSERT INTO clink (interfaces_intf_id, cdp_cdp_id) VALUES (";
			iString += intf_id + "," + cdp_id + ");";
			dieDB->query(iString.c_str());


			// 4. intfSubnet befüllen
			// Dafür muss die IP Adresse in ulong umgewandelt werden
			asio::error_code errorcode = asio::error::host_not_found;
			asio::ip::address_v4 ipa = asio::ip::address_v4::from_string(it1->at(0), errorcode);
			// Bei Fehler wird das ignoriert
			if (errorcode)
			{
				std::string dbgA = "\n6406 Parser Error: CDP Neighbor IP address " + it1->at(0) + " is invalid. \r\nPlease contact wktools@spoerr.org.\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				continue;
			}
			else
			{
				unsigned long ipAddress = ipa.to_ulong();
				sString1 = "SELECT ipSubnet_id FROM ipSubnet WHERE ulSubnet<" + boost::lexical_cast<std::string>(ipAddress); 
				sString1 += " AND ulBroadcast>" + boost::lexical_cast<std::string>(ipAddress);
				std::vector<std::vector<std::string> > ret5 = dieDB->query(sString1.c_str());
				if (!ret5.empty())
				{
					iString = "INSERT INTO intfSubnet (ipSubnet_ipSubnet_id, interfaces_intf_id) VALUES (" + ret5[0][0] + ", ";
					iString += intf_id + ");";
					dieDB->query(iString.c_str());
				}
				else
				{
					std::string dbgA = "\n6406 Parser Error: No Subnet found for CDP Neighbor Address " + it1->at(0) + ".\n";
					dbgA += sString1;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
						WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
				}
			}
		}

		// WLAN INTERFACE -> Generisches Interface zum Anzeigen, ob Endgeräte angeschlossen sind
		// Nur bei WLAN AccessPoints und IP Phones
		//////////////////////////////////////////////////////////////////////////
		if (dType == "4")	// AP
		{
			std::string iString = "INSERT INTO interfaces (intf_id,intfName,intfType,phl,status,description,l2l3)";
			iString += " VALUES(NULL, 'Radio','',1,'UP','Radio Interface','L2');";
			dieDB->query(iString.c_str());

			std::string sString1 = "SELECT last_insert_rowid() FROM interfaces;";
			std::vector<std::vector<std::string> > result2 = dieDB->query(sString1.c_str());
			intf_id = "";
			if (!result2.empty())
			{
				intf_id = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0048 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

			// devInterface Tabelle befüllen
			std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
			dieDB->query(iString2.c_str());
		}
		else if (dType == "11")		// IP Phone
		{
			// Endgeräte INTERFACE -> Generisches Interface zum Anzeigen, ob Endgeräte angeschlossen ist
			//////////////////////////////////////////////////////////////////////////
			std::string iString = "INSERT INTO interfaces (intf_id,intfName,intfType,phl,status,description,l2l3)";
			iString += " VALUES(NULL, 'Port 2','',1,'UP','PC Port','L2');";
			dieDB->query(iString.c_str());

			std::string sString1 = "SELECT last_insert_rowid() FROM interfaces;";
			std::vector<std::vector<std::string> > result2 = dieDB->query(sString1.c_str());
			intf_id = "";
			if (!result2.empty())
			{
				intf_id = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0048 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

			// devInterface Tabelle befüllen
			std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
			dieDB->query(iString2.c_str());
		}
	}
}


void WkmParserCisco::ipNetzeEintragen(std::string intfip, std::string intfmask)
{
	asio::error_code errorcode = asio::error::host_not_found;
	asio::ip::address_v4 ipa = asio::ip::address_v4::from_string(intfip, errorcode);
	asio::ip::address_v4 netmask;
	asio::ip::address_v4 bcAdr;		// Broadcast
	unsigned long mask = 0;


	unsigned long maskbits = 0;

	// Bei Fehler weiter zur nächsten IP Adresse
	if (errorcode)
	{
		std::string dbgA = "\n6406 Parser Error: Interface IP address " + intfip + " is invalid. \r\nIf the device is an ASA or PIX firewall, please make sure you disabled names -> \"no names\" \r\nPlease contact wktools@spoerr.org.";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return;
	}

	// Wenn ein "." in der Subnetmask gefunden wird, dann die Netmask in Bits umwandeln...
	if (intfmask.find(".") != intfmask.npos)
	{
		netmask = asio::ip::address_v4::from_string(intfmask, errorcode);
		if (errorcode)
		{
			std::string dbgA = "\n6406 Parser Error: Interface Subnetmask " + intfmask + " is invalid. \r\nPlease contact wktools@spoerr.org.";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			return;
		}

		mask = netmask.to_ulong();
		for (maskbits=32 ; (mask & (1UL<<(32-maskbits))) == 0 ; maskbits--)
		{

		}
	}
	else // sonst wurde es mit /xx angegeben und muss nur in ulong umgewandelt werden
	{
		try
		{
			maskbits = boost::lexical_cast<unsigned long>(intfmask);
		}
		catch (boost::bad_lexical_cast &)
		{
			std::string dbgA = "\n6406 Parser Error: Interface Subnetmask /" + intfmask + " is invalid. \r\nPlease contact wktools@spoerr.org.";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			return;			
		}

		// Create the netmask from the number of bits
		for (int i=0 ; i<maskbits ; i++)
		{
			mask |= 1<<(31-i);
		}

	}


	unsigned long int network = mask & ipa.to_ulong();
	asio::ip::address_v4 netzAddr(network);

	asio::ip::address_v4 m(mask);

	std::string netzwerk = netzAddr.to_string();
	std::string maske = m.to_string();

	bcAdr = asio::ip::address_v4::broadcast(netzAddr,m);
	std::string broadcastAddr = bcAdr.to_string();

	unsigned long bca = bcAdr.to_ulong();	// "mask" und "network" sind die anderen beiden Werte

	std::string iString = "INSERT INTO ipSubnet (subnet,mask,ulSubnet,ulMask,ulBroadcast) VALUES ('";
	iString += netzwerk + "', '" + maske + "'," + boost::lexical_cast<std::string>(network) + "," + boost::lexical_cast<std::string>(mask) + ",";
	iString += boost::lexical_cast<std::string>(bca) + ");";
	dieDB->query(iString.c_str());

	std::string sString = "SELECT last_insert_rowid() FROM ipSubnet;";
	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
	std::string subnetid = "";
	if (!result.empty())
	{
		subnetid = result[0][0];
	}
	else
	{
		std::string dbgA = "\n6305: Parser Error! Code 0014 - Contact wktools@spoerr.org\n";
		dbgA += iString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehlerString += "\n" + dateiname + dbgA;
	}

	iString = "INSERT INTO intfSubnet (ipSubnet_ipSubnet_id, interfaces_intf_id) VALUES (" + subnetid + ", ";
	iString += intf_id + ");";
	dieDB->query(iString.c_str());

}


std::string WkmParserCisco::nextHopIntfCheck(std::string ipaddr)
{
	asio::error_code errorcode = asio::error::host_not_found;
	asio::ip::address_v4 ipa = asio::ip::address_v4::from_string(ipaddr, errorcode);
	asio::ip::address_v4 netmask;
	asio::ip::address_v4 bcAdr;		// Broadcast
	unsigned long mask = 0;

	unsigned long maskbits = 0;

	// Bei Fehler weiter zur nächsten IP Adresse
	if (errorcode)
	{
		std::string dbgA = "\n6406 Parser Error: Interface IP address " + ipaddr + " is invalid. \r\nPlease contact wktools@spoerr.org.";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return "";
	}

	std::string netzwerk = "";

	// Als erstes alle IP/Subnet Infos für das aktuelle Gerät auslesen
	std::string sString = "SELECT subnet,mask from ipSubnet INNER JOIN intfSubnet ON ipSubnet.ipSubnet_id=intfSubnet.ipSubnet_ipSubnet_id ";
	sString += "INNER JOIN interfaces ON intfSubnet.interfaces_intf_id=interfaces.intf_id ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE devInterface.device_dev_id=" + dev_id;
	std::vector<std::vector<std::string>> ipsubnets = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator ipit = ipsubnets.begin(); ipit < ipsubnets.end(); ++ipit)
	{
		// Die Netmask in Bits umwandeln
		netmask = asio::ip::address_v4::from_string(ipit->at(1), errorcode);
		if (errorcode)
		{
			std::string dbgA = "\n6406 Parser Error: Interface Subnetmask " + ipit->at(1) + " is invalid. \r\nPlease contact wktools@spoerr.org.";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			return "";
		}

		mask = netmask.to_ulong();
		for (maskbits=32 ; (mask & (1UL<<(32-maskbits))) == 0 ; maskbits--)
		{

		}
		// Umwandeln ENDE

		// Dann Netzwerkadresse für die NH Adresse rausfinden
		unsigned long int network = mask & ipa.to_ulong();
		asio::ip::address_v4 netzAddr(network);
		netzwerk = netzAddr.to_string();

		// Check, ob NetzwerkAdresse vom NH mit der ausgelesenen Interface Netzwerkadresse übereinstimmt. Wenn ja, dann ist das das NH Interface
		if (ipit->at(0) == netzwerk)
		{
			break;
		}
	}



	sString = "SELECT intf_id from interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN intfSubnet ON intfSubnet.interfaces_intf_id=interfaces.intf_id ";
	sString += "INNER JOIN ipSubnet ON ipSubnet.ipSubnet_id=intfSubnet.ipSubnet_ipSubnet_id ";
	sString += "WHERE devInterface.device_dev_id=" + dev_id + " AND ipSubnet.subnet LIKE '" + netzwerk + "'";
	std::vector<std::vector<std::string>> result = dieDB->query(sString.c_str());
	if (!result.empty())
	{
		return result[0][0];
	}
	else
	{
		return "";
	}
}


std::string WkmParserCisco::maskToBits(std::string subnetmask)
{
	asio::error_code errorcode = asio::error::host_not_found;
	asio::ip::address_v4 netmask = asio::ip::address_v4::from_string(subnetmask, errorcode);
	unsigned long mask = 0;

	unsigned long maskbits = 0;

	// Bei Fehler weiter zur nächsten IP Adresse
	if (errorcode)
	{
		std::string dbgA = "\n6406 Parser Error: Subnetmask " + subnetmask + " is invalid. \r\nPlease contact wktools@spoerr.org.";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return "";
	}

	mask = netmask.to_ulong();
	if (mask)
	{
		for (maskbits=32 ; (mask & (1UL<<(32-maskbits))) == 0 ; maskbits--)
		{

		}
	}
	else
	{
		maskbits = 0;
	}

	std::string bits =  boost::lexical_cast<std::string>(maskbits);
	bits = "/" + bits;
	return bits;
}


void WkmParserCisco::testNexus()
{
	confReg = "";
	vpcPeerLink = "";

	aktPos1 = ganzeDatei.find("system:");
	if (aktPos1 < pos1)
	{
		aktPos1 += 19;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		version = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}

	aktPos1 = ganzeDatei.find("system image file is:", aktPos2);
	if (aktPos1 < pos1)
	{
		aktPos1 += 22;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		hwBootfile = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		boost::algorithm::trim(hwBootfile);
	}

	aktPos1 = ganzeDatei.find("Hardware", aktPos2);
	if (aktPos1 < pos1)
	{
		aktPos1 = ganzeDatei.find("isco", aktPos1) + 5;
		aktPos2 = ganzeDatei.find("(", aktPos1) - 1;
		modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}
	aktPos1 = ganzeDatei.find("with ", aktPos2);
	if (aktPos1 < pos1)
	{
		aktPos1 += 5;
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		aktPos2 = ganzeDatei.find(" ", aktPos2+1);
		hwMem = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}

	aktPos1 = ganzeDatei.find("Device name: ", aktPos2);
	if (aktPos1 < pos1)
	{
		aktPos1 += 13;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		hostname = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
	}

	hwPos = "box";
	devicetype = SWITCH;
	devType = "2";
	bool ret = insertDevice();
	if (ret)
	{
		insertHardware();
		nxosInterfaceParser();
		nxosVPCParser();
		nxosSTPParser();
		nxosARPParser();
		nxosCAMParser();
		iosSwitchportParser();
		nxosCDPParser();
		nxosSNMPParser();
		nxosInventoryParser();
		nxosVrfParser();
		nxosRouteParser();
		//		iosFileSystemParser();

		markNeighbor();
	}

}


void WkmParserCisco::nxosARPParser()
{
	zeitStempel("\tnxARPParser START");

	aktPos1 = ganzeDatei.find("Interface", pos6);
	aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1)+1;

	std::string arpString = ganzeDatei.substr(aktPos1, pos7-aktPos1);

	std::string arpZeile = "";

	std::string ipa = "";
	std::string maca = "";
	std::string intf = "";

	std::string iname = "";			// Interface Name vom letzten Durchlauf
	std::string iid = "";			// Interface ID vom letzten Interface Name

	std::string ageTime = "";
	std::string self = "0";

	aktPos1 = 0;
	aktPos2 = 0;

	int offsets[] = {16, 10, 14, 2, 16};
	boost::offset_separator f3(offsets, offsets + 5, false, true);
	for (; aktPos2 < arpString.npos;)
	{
		aktPos2 = arpString.find_first_of("\r\n", aktPos1 + 1);
		arpZeile = arpString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
		aktPos1 = aktPos2;
		if (arpZeile.size() > 45)
		{
			int i = 0;
			boost::tokenizer<boost::offset_separator> tok(arpZeile, f3);
			for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				switch (i)
				{
				case 0:
					ipa = *beg;
					break;
				case 1:
					ageTime = *beg;
					break;
				case 2:
					maca = *beg;
					break;
				case 4:
					intf = *beg;
					intf = intfNameChange(intf);
					break;
				default:
					break;
				}
				i++;
			}

			if (ageTime.find("-") != ageTime.npos)
			{
				self = "1";
			}
			else
			{
				self = "0";
			}

			// Zuerst die Interface ID in der interface Tabelle suchen
			std::string intfID = "";
			if (iname != intf)
			{
				std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
				iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
				std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
				if (!result.empty())
				{
					intfID = result[0][0];
				}
				else
				{
					std::string dbgA = "\n6406: Parser Error! Code 0041 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
						WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
					fehlerString += "\n" + dateiname + dbgA;
					continue;
				}
			}
			else 
			{
				intfID = iid;
			}

			iname = intf;
			iid = intfID;

			// Dann die Neighbor Tabelle füllen
			std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr, l3_addr, self) VALUES (NULL, '";
			iString2 += maca + "', '" + ipa + "'," + self + ");";
			dieDB->query(iString2.c_str());

			// Dann die nlink Tabelle
			std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
			iString3 += intfID + ");";
			dieDB->query(iString3.c_str());
		}
	}
}


void WkmParserCisco::nxosCAMParser()
{
	zeitStempel("\tnxosCAMParser START");

	std::string iname = "";			// Interface Name vom letzten Durchlauf
	std::string iid = "";			// Interface ID vom letzten Interface Name

	size_t typePos = ganzeDatei.find("* - primary entry", pos7);
	if (typePos < ganzeDatei.npos)		// Nexus 5k und 7k
	{
		aktPos1 = ganzeDatei.find("*", typePos+50);

		size_t camende = ganzeDatei.size();
		// TODO: Bessere Lösung finden, als ganzeDatei.size()
		if (pos8 < camende)
		{
			camende = pos8;
		}
		std::string camString = ganzeDatei.substr(aktPos1-1, camende-aktPos1);
		std::string camZeile = "";

		std::string vlan = "";
		std::string maca = "";
		std::string intf = "";
		std::string type = "";

		aktPos1 = 0;
		aktPos2 = 0;

		int offsets[] = {2, 4, 5, 14, 33, 25};
		boost::offset_separator f3(offsets, offsets + 6, false, true);
		for (; aktPos2 < camString.npos;)
		{
			aktPos2 = camString.find_first_of("\r\n", aktPos1 + 1);
			camZeile = camString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
			aktPos1 = aktPos2;
			if (camZeile.size() > 50)
			{
				if (camZeile.find("* - primary entry, G - Gateway MAC, (R) - Routed MAC, O - Overlay MAC") != camZeile.npos || 
					camZeile.find("age - seconds since last seen,+ - primary entry using vPC Peer-Link") != camZeile.npos || 
					camZeile.find("VLAN     MAC Address      Type      age     Secure NTFY    Ports") != camZeile.npos || 
					camZeile.find("VLAN     MAC Address      Type      age     Secure NTFY   Ports/SWID.SSID.LID") != camZeile.npos || 
					camZeile.find("---------+-----------------+--------+---------+------+----+------------------") != camZeile.npos)
				{
					continue;
				}

				int i = 0;
				boost::tokenizer<boost::offset_separator> tok(camZeile, f3);
				for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 1:
						vlan = *beg;
						break;
					case 3:
						maca = *beg;
						break;
					case 4:
						type = *beg;
						break;
					case 5:
						intf = *beg;
						break;
					default:
						break;
					}
					i++;
				}
				// Wenn type==igmp, dann weiter, da MC MAC Adressen nicht interessant sind
				// Es kann dann auch vorkommen, dass die Interfaces in mehreren Zeilen stehen; Daher wird geprüft, ob die MAC Adresse aus alles Leerzeichen besteht.
				if (type.find("igmp") != type.npos || maca.find("              ") != maca.npos)
				{
					continue;
				}

				// vPC Peer Link Interface einsetzen/ersetzen
				if (intf.find("vPC Peer-Link") != intf.npos)
				{
					intf = vpcPeerLink;
				}
				// keyword "Router" überspringen
				if (intf.find("Router") != intf.npos)
				{
					continue;
				}

				// Eigene MAC Adressen ignorieren
				if (intf.find("sup-eth") == intf.npos)
				{
					// Zuerst die Interface ID in der interface Tabelle suchen
					std::string intfID = "";
					if (iname != intf)
					{
						std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
						iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
						std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
						if (!result.empty())
						{
							intfID = result[0][0];
						}
						else
						{
							std::string dbgA = "\n6406: Parser Error! Code 0042 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
							dbgA += iString;
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
								WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
							fehlerString += "\n" + dateiname + dbgA;
							continue;
						}
					}
					else 
					{
						intfID = iid;
					}

					iname = intf;
					iid = intfID;

					// Dann die Neighbor Tabelle füllen
					std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr) VALUES (NULL, '";
					iString2 += maca + "');";
					dieDB->query(iString2.c_str());

					// Dann die nlink Tabelle
					std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
					iString3 += intfID + ");";
					dieDB->query(iString3.c_str());
				}
			}
		}
	}
	else if (pos8-pos7 > 200)		// Nexus 1k
	{
		aktPos1 = ganzeDatei.find("-+-", pos7);
		aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);

		std::string camString = ganzeDatei.substr(aktPos1-1, pos8-aktPos1);
		std::string camZeile = "";

		std::string vlan = "";
		std::string maca = "";
		std::string intf = "";
		std::string type = "";
		std::string age = "";

		aktPos1 = 0;
		aktPos2 = 0;

		for (; aktPos2 < camString.npos;)
		{
			aktPos2 = camString.find_first_of("\r\n", aktPos1 + 1);
			camZeile = camString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
			aktPos1 = aktPos2;
			if (camZeile.size() > 50)
			{
				if (camZeile.find("VLAN      MAC Address       Type    Age       Port                           Mod") != camZeile.npos || 
					camZeile.find("---------+-----------------+-------+---------+------------------------------+---") != camZeile.npos)
				{
					continue;
				}

				boost::char_separator<char> sep(" ,");
				boost::tokenizer<boost::char_separator<char>> tok(camZeile, sep);

				int i = 0;
				for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 0:
						vlan = *beg;
						break;
					case 1:
						maca = *beg;
						break;
					case 2:
						type = *beg;
						break;
					case 3:
						age = *beg;
						break;
					case 4:
						intf = *beg;
						break;
					default:
						break;
					}
					i++;
				}
				// Wenn type==igmp, dann weiter, da MC MAC Adressen nicht interessant sind
				// Es kann dann auch vorkommen, dass die Interfaces in mehreren Zeilen stehen; Daher wird geprüft, ob die MAC Adresse aus alles Leerzeichen besteht.
				if (type.find("igmp") != type.npos || maca.find("              ") != maca.npos)
				{
					continue;
				}

				// vPC Peer Link Interface einsetzen/ersetzen
				if (intf.find("vPC Peer-Link") != intf.npos)
				{
					intf = vpcPeerLink;
				}
				else if (intf.find("N1KV") != intf.npos)
				{
					continue;
				}

				// Zuerst die Interface ID in der interface Tabelle suchen
				std::string intfID = "";
				if (iname != intf)
				{
					std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
					iString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
					std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
					if (!result.empty())
					{
						intfID = result[0][0];
					}
					else
					{
						std::string dbgA = "\n6406: Parser Error! Code 0042 - If Interface '" + intf + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
						dbgA += iString;
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
							WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
						fehlerString += "\n" + dateiname + dbgA;
						continue;
					}
				}
				else 
				{
					intfID = iid;
				}

				iname = intf;
				iid = intfID;

				// Dann die Neighbor Tabelle füllen
				std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr) VALUES (NULL, '";
				iString2 += maca + "');";
				dieDB->query(iString2.c_str());

				// Dann die nlink Tabelle
				std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
				iString3 += intfID + ");";
				dieDB->query(iString3.c_str());
			}
		}
	}
}


void WkmParserCisco::nxosCDPParser()
{
	zeitStempel("\tnxosCDPParser START");
	// Nur Interfaces, die UP sind, sind interessant
	size_t cdppos = ganzeDatei.find("-------------------------", pos5);

	while (cdppos < pos6)
	{
		// Der nächste Nachbar
		aktPos1 = cdppos;
		aktPos2 = cdppos;
		cdppos = ganzeDatei.find("-------------------------", cdppos + 30);

		std::string nName = "";
		std::string alternatenName = "";
		if ((aktPos1 = ganzeDatei.find("Device ID:", aktPos1)) < cdppos)
		{
			aktPos1 += 10;
			aktPos1 = ganzeDatei.find_first_not_of(" ", aktPos1);
			aktPos2 = ganzeDatei.find_first_of("\r\n.( ", aktPos1);
			nName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			alternatenName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		std::string nT = "";
		std::string nType = "99";
		std::string platform = "";
		std::string ipa = "";

		// TempPos einführen, da die IP Adresse manchmal ganz am Ende steht.
		size_t tempPos = aktPos1;

		if ((aktPos1 = ganzeDatei.find("IP address:", aktPos1)) < cdppos)
		{
			aktPos1 += 12;
			aktPos2 = ganzeDatei.find_first_of(",;\r\n", aktPos1);
			ipa = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = ganzeDatei.find("IPv4 Address:", aktPos2)) < cdppos)
		{
			aktPos1 += 14;
			aktPos2 = ganzeDatei.find_first_of(",;\r\n", aktPos1);
			ipa = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		if ((aktPos1 = ganzeDatei.find("Platform: ", tempPos)) < cdppos)
		{
			aktPos1 += 10;
			aktPos2 = ganzeDatei.find_first_of(",;\r\n", aktPos1);
			platform = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		if ((aktPos1 = ganzeDatei.find("Capabilities: ", tempPos)) < cdppos)
		{
			aktPos1 += 14;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			nT = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			if (nT.find("Host Phone") != nT.npos)
			{
				nType = "11";
			}
			else if (nT.find("Host") != nT.npos)
			{
				if (platform.find("ATA") != platform.npos)
				{
					nType = "13";
				}
				else if (platform.find("CIVS") != platform.npos)
				{
					nType = "14";
				}
				else if (platform.find("CP-") != platform.npos)
				{
					nType = "11";
				}
				else
				{
					nType = "12";
				}
			}
			else if (nT.find("Router Switch") != nT.npos)
			{
				nType = "5";
			}
			else if (nT.find("Switch") != nT.npos)
			{
				nType = "2";
			}
			else if (nT.find("Router") != nT.npos)
			{
				nType = "1";
			}
			else if (nT.find("Trans-Bridge") != nT.npos)
			{
				nType = "4";
			}
		}

		std::string nIntf = "";
		if ((aktPos1 = ganzeDatei.find("Interface: ", aktPos2)) < cdppos)
		{
			aktPos1 += 11;
			aktPos2 = ganzeDatei.find_first_of(",", aktPos1);
			nIntf = intfNameChange(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
		}

		std::string nNintf = "";
		if ((aktPos1 = ganzeDatei.find("Port ID (outgoing port): ", aktPos2)) < cdppos)
		{
			aktPos1 += 25;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			nNintf = intfNameChange(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
		}

		std::string nSWver = "";
		if ((aktPos1 = ganzeDatei.find("Version:", aktPos1)) < cdppos)
		{
			aktPos2 = aktPos1 + 9;
			if ((aktPos1 = ganzeDatei.find("Version", aktPos2)) < cdppos)
			{
				aktPos2 = ganzeDatei.find_first_not_of(" :", aktPos1);
			}
			else if ((aktPos1 = ganzeDatei.find("Cisco Internetwork Operating System Software", aktPos2)) < cdppos)
			{
				aktPos2 = aktPos1 + 45;
			}

			aktPos1 = ganzeDatei.find_first_not_of("\r\n ", aktPos2);
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			nSWver = intfNameChange(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
			if (nSWver.find("Version") != nSWver.npos)
			{
				std::size_t zwischenPos1 = nSWver.find("Version")+8;
				std::size_t zwischenPos2 = nSWver.find_first_of("\r\n ,", zwischenPos1);
				nSWver = nSWver.substr(zwischenPos1, zwischenPos2-zwischenPos1);
			}
			else if (nSWver.find("Release") != nSWver.npos)
			{
				std::size_t zwischenPos1 = nSWver.find("Release")+8;
				std::size_t zwischenPos2 = nSWver.find_first_of("\r\n ,", zwischenPos1);
				nSWver = nSWver.substr(zwischenPos1, zwischenPos2-zwischenPos1);
			}
		}

		// Interface ID auslesen
		std::string sString = "SELECT intf_id FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE intfName LIKE '" + nIntf + "' "; 
		sString += "AND devInterface.device_dev_id=" + dev_id + ";";
		std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
		std::string intRow = "";
		if (!result.empty())
		{
			intRow = result[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0015 - Contact wktools@spoerr.org\n";
			dbgA += sString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// Zuerst die CDP Tabelle befüllen
		// Zuerst die CDP Tabelle befüllen
		std::string iString = "INSERT INTO cdp (hostname, nName, alternatenName, intf, nIntfIP, nIntf, type, platform, sw_version)";
		iString += " VALUES('" + hostname + "','" + nName + "','" + alternatenName + "','" + nIntf + "','" + ipa + "','" + nNintf + "'," + nType + ",'" + platform + "','" + nSWver + "');";
		dieDB->query(iString.c_str());

		sString = "SELECT last_insert_rowid() FROM cdp;";
		result = dieDB->query(sString.c_str());
		std::string cdpRow = "";
		if (!result.empty())
		{
			cdpRow = result[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0016 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// Und zum Schluss die clink Tabelle updaten
		iString = "INSERT INTO clink (interfaces_intf_id, cdp_cdp_id) VALUES (";
		iString += intRow + "," + cdpRow + ");";
		dieDB->query(iString.c_str());
	}
}


void WkmParserCisco::nxosInterfaceParser()
{
	zeitStempel("\tnxosInterfaceParser START");
	// Channel Interfaces und physikalische Interfaces können durchmischt sein -> ein Channelinterface kann vor den zugehörigen physikalischen Interfaces vorkommen
	// -> Daher werden die Channelzugehörigkeiten in einer Map gesammelt und erst zum Schluss ausgewertet
	std::map <std::string, std::string> chMember;
	std::map <std::string, std::string>::iterator chMemIter, chMemAnfang, chMemEnde;
	typedef std::pair <std::string, std::string> pss;

	// Selbiges für Bound To
	std::map <std::string, std::string> boundToMember;
	std::map <std::string, std::string>::iterator boundToMemIter, boundToMemAnfang, boundToMemEnde;

	// Nur Interfaces, die UP sind, sind interessant
	std::size_t intfpos1 = ganzeDatei.find(" is up", pos1);
	std::size_t intfpos2 = ganzeDatei.find(" is down ", pos1);
	std::size_t intfpos3 = ganzeDatei.find(" is trunking", pos1);
	std::size_t intfpos = 0;

	std::string intfStatus = "UP";

	while ((intfpos1 < pos2) || (intfpos2 < pos2) || (intfpos3 < pos2))
	{
		bool intfup = true;	// Interface up (true) oder down (true)?
		// intfpos bestimmen
		if (intfpos1 < intfpos2 && intfpos1 < intfpos3)
		{
			aktPos1 = intfpos1;
			intfup = true;
			intfStatus = "UP";
		}
		else if (intfpos2 < intfpos1 && intfpos2 < intfpos3)
		{
			aktPos1 = intfpos2;
			intfup = true;
			intfStatus = "DOWN";
		}
		else
		{
			aktPos1 = intfpos3;
			intfup = true;
			intfStatus = "UP";
		}
		intfpos1 = ganzeDatei.find(" is up", aktPos1+50);
		intfpos2 = ganzeDatei.find(" is down", aktPos1+50);
		intfpos3 = ganzeDatei.find(" is trunking", aktPos1+50);
		if (intfpos1 < intfpos2 && intfpos1 < intfpos3)
		{
			intfpos = intfpos1;
		}
		else if (intfpos2 < intfpos1 && intfpos2 < intfpos3)
		{
			intfpos = intfpos2;
		}
		else
		{
			intfpos = intfpos3;
		}

		if (intfpos > pos3)
		{
			intfpos = pos3;
		}


		// Interface Name
		std::string intfName = "";
		std::string intfType = "";
		std::string phl = "0";
		std::string intfHauptIntf = "";
		if (intfup)
		{
			aktPos1 = ganzeDatei.rfind("\n", aktPos1) + 1;
			aktPos2 = ganzeDatei.find(" ", aktPos1);
			intfName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			intfName = intfNameChange(intfName);

			// Herausfinden, ob Subinterface
			if (intfName.find(".") != intfName.npos)
			{
				intfHauptIntf = intfName.substr(0, intfName.find("."));
			}

			// Granulareren Interface Status bestimmen
			if ((aktPos1 = ganzeDatei.find("(dministratively down)", aktPos2)) < intfpos)
			{
				intfStatus = "DISABLED";
			}
			else if ((aktPos1 = ganzeDatei.find("(Bound Physical Interface Down)", aktPos2)) < intfpos)
			{
				intfStatus = "BOUNDIFDOWN";
			}
			else if ((aktPos1 = ganzeDatei.find("(SFP not inserted)", aktPos2)) < intfpos)
			{
				intfStatus = "NOSFP";
			}
			else
			{
				aktPos1 = aktPos2;
			}


			// Description bei Nexus 1k -> Interface Description für alle anderen Switches kommt weiter unten
			std::string intfDescription = "";
			if ((aktPos1 = ganzeDatei.find("Port description is ", aktPos2)) < intfpos)
			{
				aktPos1 += 20;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				intfDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				while (intfDescription.find_first_of("'\"&<>") != intfDescription.npos)
				{
					intfDescription.replace(intfDescription.find_first_of("'\"&<>"), 1, "-");
				}
			}
			else if ((aktPos1 = ganzeDatei.find("Description: ", aktPos2)) < intfpos)
			{
				aktPos1 += 13;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				intfDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				while (intfDescription.find_first_of("'\"&<>") != intfDescription.npos)
				{
					intfDescription.replace(intfDescription.find_first_of("'\"&<>"), 1, "-");
				}
			}

			// Falls Vethernet -> Bound Interface auslesen
			std::string boundTo = "";
			if ((aktPos1 = ganzeDatei.find("Bound Interface is ", aktPos2)) < intfpos)
			{
				aktPos1 += 19;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				boundTo = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				boost::algorithm::trim(boundTo);
				if (boundTo == "--")
				{
					boundTo = "";
				}
				else
				{
					boundTo = intfNameChange(boundTo);
				}
			}



			// Hardware Adresse, falls vorhanden
			std::string intfMac = "";
			if ((aktPos1 = ganzeDatei.find("Hardware:", aktPos2)) < intfpos)
			{
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				aktPos1 = ganzeDatei.find("address:", aktPos1);
				if (aktPos1 < aktPos2)
				{
					intfMac = ganzeDatei.substr(aktPos1+9, 14);
				}
			}
			else if ((aktPos1 = ganzeDatei.find("Hardware is ", aktPos2)) < intfpos)
			{
				if (ganzeDatei.find("Fibre Channel", aktPos1) > intfpos)
				{
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					aktPos1 = ganzeDatei.find("address is ", aktPos1);
					aktPos1 += 11;
					aktPos1 = ganzeDatei.find_first_not_of(" ", aktPos1);
					if (aktPos1 < aktPos2)
					{
						intfMac = ganzeDatei.substr(aktPos1, 14);
					}
				}
			}

			// Owner (Nexus 1k)
			std::string ownerIs = "";
			if ((aktPos1 = ganzeDatei.find("Owner is ", aktPos2)) < intfpos)
			{
				aktPos1 += 9;
				aktPos2 = ganzeDatei.find(",", aktPos1);
				ownerIs = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				while (ownerIs.find_first_of("'\"&<>") != ownerIs.npos)
				{
					ownerIs.replace(ownerIs.find_first_of("'\"&<>"), 1, " ");
				}

			}
			// Active on Module (Nexus 1k)
			std::string actModule = "";
			if ((aktPos1 = ganzeDatei.find("Active on module ", aktPos2)) < intfpos)
			{
				aktPos1 += 17;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				actModule = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			// Port Profile (Nexus 1k)
			std::string portProfile = "";
			if ((aktPos1 = ganzeDatei.find("Port-Profile is ", aktPos2)) < intfpos)
			{
				aktPos1 += 16;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				portProfile = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}


			// Description, falls vorhanden
			if ((aktPos1 = ganzeDatei.find("Description: ", aktPos2)) < intfpos)
			{
				aktPos1 += 13;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				intfDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				while (intfDescription.find_first_of("'\"&<>") != intfDescription.npos)
				{
					intfDescription.replace(intfDescription.find_first_of("'\"&<>"), 1, "-");
				}
			}
			// IP Adresse falls vorhanden
			std::string intfIP = "";
			std::string intfIPMask = "";
			std::string l2l3 = "L2";
			if ((aktPos1 = ganzeDatei.find("Internet Address is ", aktPos2)) < intfpos)
			{
				aktPos1 += 20;
				aktPos2 = ganzeDatei.find("/", aktPos1)+1;
				intfIP = ganzeDatei.substr(aktPos1, aktPos2-aktPos1-1);
				aktPos1 = aktPos2;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				intfIPMask = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				l2l3 = "L3";
			}

			// Load falls vorhanden
			std::string txLoad = "";
			std::string rxLoad = "";
			int txload = 0;
			int rxload = 0;
			if ((aktPos1 = ganzeDatei.find("txload ", aktPos2)) < intfpos)
			{
				aktPos1 += 7;
				aktPos2 = ganzeDatei.find("/", aktPos1);
				txLoad = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					txload = boost::lexical_cast<int>(txLoad);
				}
				catch (boost::bad_lexical_cast &)
				{
					txload = 0;			
				}


				if ((aktPos1 = ganzeDatei.find("rxLoad ", aktPos2)) < intfpos)
				{
					aktPos1 += 7;
					aktPos2 = ganzeDatei.find("/", aktPos1);
					rxLoad = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					try
					{
						rxload = boost::lexical_cast<int>(rxLoad);
					}
					catch (boost::bad_lexical_cast &)
					{
						rxload = 0;			
					}
				}
			}
			int intfLoadLevel = (rxload+txload)/2;

			// Encapsulation
			//aktPos1 = ganzeDatei.find("Encapsulation ", aktPos2) + 14;
			//aktPos2 = ganzeDatei.find_first_of(",\r\n", aktPos1);
			//std::string encaps = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			// VLAN ID falls vorhanden
			std::string vlanid = "";
			int vlanID = 0;
			if ((aktPos1 = ganzeDatei.find("Vlan ID  ", aktPos2)) < intfpos)
			{
				aktPos1 += 9;
				aktPos2 = ganzeDatei.find(".", aktPos1);
				vlanid = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					vlanID = boost::lexical_cast<int>(vlanid);
				}
				catch (boost::bad_lexical_cast &)
				{
					vlanID = 0;			
				}
			}


			// Duplex und Speed falls vorhanden
			std::string duplex = "";
			std::string speed = "";
			if ((aktPos1 = ganzeDatei.find("duplex", aktPos2)) < intfpos)
			{
				aktPos1 = ganzeDatei.rfind(" ", aktPos1);
				aktPos2 = ganzeDatei.find(",", aktPos1);
				duplex = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = aktPos2+2;
				aktPos2 = ganzeDatei.find_first_of(",\r\n", aktPos1);
				speed = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			// Interface Type rausfinden
			intfType = intfTypeCheck(intfName, speed);
			phl = intfPhlCheck(intfName, speed);

			// Channel Members
			std::string chMem = "";
			if ((aktPos1 = ganzeDatei.find("Members in this channel", aktPos2)) < intfpos)
			{
				aktPos1 += 25;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				chMem = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			// Last Intf Change
			std::string lastChange = "";
			if ((aktPos1 = ganzeDatei.find("Last link flapped ", aktPos2)) < intfpos)
			{
				aktPos1 += 18;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				lastChange = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			// Last clearing of interface counters
			std::string lastClear = "";
			if ((aktPos1 = ganzeDatei.find("Last clearing of \"show interface\" counters ", aktPos2)) < intfpos)
			{
				aktPos1 += 43;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				lastClear = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			// Input Bits/s
			std::string ibits = "";
			if ((aktPos1 = ganzeDatei.find("input rate ", aktPos2)) < intfpos)
			{
				aktPos1 += 11;
				aktPos2 = ganzeDatei.find(" ", aktPos1);
				ibits = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			// Output Bits/s
			std::string obits = "";
			if ((aktPos1 = ganzeDatei.find("output rate ", aktPos2)) < intfpos)
			{
				aktPos1 += 12;
				aktPos2 = ganzeDatei.find(" ", aktPos1);
				obits = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			// Fehler
			int tempErr = 0;
			int err = 0;

			std::string iCrc = "";
			std::string iFrame = "";
			std::string iOverrun = "";
			std::string iIgnored = "";
			std::string iWatchdog = "";
			std::string iPause = "";
			std::string iDribbleCondition = "";
			std::string l2decodeDrops = "";

			// Input Runts
			std::string runts = "";
			if ((aktPos1 = ganzeDatei.find(" runts", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				runts = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(runts);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;

			}

			// Input Giants
			std::string giants = "";
			if ((aktPos1 = ganzeDatei.find(" giants   ", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind("\n", aktPos2 - 1) + 1;

				giants = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(giants);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}
			else if ((aktPos1 = ganzeDatei.find(" giants", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;

				giants = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(giants);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}

			// Input Throttles
			std::string crc = "";
			if ((aktPos1 = ganzeDatei.find(" CRC", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				crc = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				iCrc = crc;
				try
				{
					tempErr = boost::lexical_cast<int>(crc);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}

			// Input Buffer errors
			std::string ibuffer = "";
			if ((aktPos1 = ganzeDatei.find(" no buffer", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				ibuffer = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(ibuffer);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}

			// Input Erros
			std::string ierrors = "";
			if ((aktPos1 = ganzeDatei.find(" input error", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				ierrors = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(ierrors);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}
			if ((aktPos1 = ganzeDatei.find(" short frame", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				iFrame = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" overrun", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				iOverrun = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" ignored", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				iIgnored = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" watchdog", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				iWatchdog = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" input with dribble", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				iDribbleCondition = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" Rx pause", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				iPause = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}

			// Output underruns
			std::string underrun = "";
			if ((aktPos1 = ganzeDatei.find(" underruns", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
				underrun = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(underrun);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}

			// Output errors
			std::string oerrors = "";
			std::string oCollisions = "";
			std::string oBabbles = "";
			std::string oLateColl = "";
			std::string oDeferred = "";
			std::string oLostCarrier = "";
			std::string oNoCarrier = "";
			std::string oPauseOutput = "";
			std::string oBufferSwapped = "";
			if ((aktPos1 = ganzeDatei.find(" output error", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
				oerrors = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(oerrors);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}

			if ((aktPos1 = ganzeDatei.find(" collisions", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				oCollisions = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" deferred", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				oDeferred = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" late collision", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				oLateColl = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" lost carrier", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				oLostCarrier = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" no carrier", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				oNoCarrier = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" babble", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				oBabbles = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
			if ((aktPos1 = ganzeDatei.find(" Tx pause", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
				oPauseOutput = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}


			// Interface Resets
			std::string resets = "";
			if ((aktPos1 = ganzeDatei.find(" interface resets", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
				resets = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(resets);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}

			// Output Buffer Errors
			std::string obuffer = "";
			if ((aktPos1 = ganzeDatei.find(" output buffer failures", aktPos2)) < intfpos)
			{
				aktPos2 = aktPos1;
				aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
				obuffer = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				try
				{
					tempErr = boost::lexical_cast<int>(obuffer);
				}
				catch (boost::bad_lexical_cast &)
				{
					tempErr = 0;			
				}
				err += tempErr;
			}
			//std::string iString = "INSERT INTO interfaces (intf_id,intfName,intfType,phl,macAddress,ipAddress,subnetMask,duplex,speed,status,description,l2l3,errLvl,lastClear,loadLvl,channel_intf_id)";
			//iString += " VALUES(NULL, '" + intfName + "','" + intfType + "'," + phl + ",'" + intfMac + "','" + intfIP + "','"  + intfIPMask + "','" + duplex + "','" + speed;
			//iString += "','UP','" + intfDescription + "','" + l2l3 + "'," + boost::lexical_cast<std::string>(err) + ",'" + lastClear + "'," + boost::lexical_cast<std::string>(intfLoadLevel) + ",NULL);";
			//dieDB->query(iString.c_str());
			std::string iString = "INSERT INTO interfaces (intf_id,intfName,intfType,phl,macAddress,ipAddress,subnetMask,duplex,speed,status,description,l2l3,";
			iString += "errLvl,lastClear,loadLvl,channel_intf_id,iCrc,iFrame,iOverrun,iIgnored,iWatchdog,iPause,iDribbleCondition,ibuffer,l2decodeDrops,runts,giants,";
			iString += "throttles,ierrors,underrun,oerrors,oCollisions,oBabbles,oLateColl,oDeferred,oLostCarrier,oNoCarrier,oPauseOutput,oBufferSwapped,";
			iString += "resets,obuffer,lastInput,actModule,owner,portProfile)";
			iString += " VALUES(NULL, '" + intfName + "','" + intfType + "'," + phl + ",'" + intfMac + "','" + intfIP + "','"  + intfIPMask + "','" + duplex + "','" + speed;
			iString += "','" + intfStatus + "','" + intfDescription + "','" + l2l3 + "'," + boost::lexical_cast<std::string>(err) + ",'" + lastClear + "',"; 
			iString += boost::lexical_cast<std::string>(intfLoadLevel) + ",NULL,'" + iCrc + "','" + iFrame + "','" + iOverrun + "','" + iIgnored;
			iString += "','" + iWatchdog + "','" + iPause + "','" + iDribbleCondition + "','" + ibuffer + "','"  + l2decodeDrops + "','" + runts + "','" + giants + "','" + "";
			iString += "','" + ierrors + "','" + underrun + "','" + oerrors + "','" + oCollisions + "','" + oBabbles + "','" + oLateColl + "','" + oDeferred;
			iString += "','" + oLostCarrier + "','" + oNoCarrier + "','" + oPauseOutput + "','" + oBufferSwapped + "','" + resets + "','";
			iString += obuffer + "','" + lastChange + "','" + actModule + "','" + ownerIs + "','" + portProfile + "');";
			dieDB->query(iString.c_str());

			std::string sString = "SELECT last_insert_rowid() FROM interfaces;";
			std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
			intf_id = ""; 
			if (!result2.empty())
			{
				intf_id = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0017 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

			// Die Channel Members temporär in die Map schreiben
			if (chMem != "")
			{
				chMember.insert(pss(intf_id, chMem));
			}

			// Die BoundTo Members temporär in die Map schreiben
			if (boundTo != "")
			{
				boundToMember.insert(pss(intf_id, boundTo));
			}

			// Das Hauptinterface finden und dann die Subinterfaces markieren
			if (intfHauptIntf != "")
			{
				std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
				sString += intfHauptIntf + "' AND devInterface.device_dev_id=" + dev_id +";";
				std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

				if (!result.empty())
				{
					std::string iString4 = "UPDATE interfaces SET channel_intf_id=" + result[0][0] + " WHERE intf_id = last_insert_rowid();";
					dieDB->query(iString4.c_str());
				}
				else
				{
					std::string dbgA = "\n6305: Parser Error! Code 0019 - Contact wktools@spoerr.org\n";
					dbgA += sString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehlerString += "\n" + dateiname + dbgA;
				}
			}

			// devInterface Tabelle befüllen
			std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
			dieDB->query(iString2.c_str());

			// ipSubnet befüllen
			if (intfIP != "")
			{
				ipNetzeEintragen(intfIP, intfIPMask);
			}
		}
	}

	// Die Channel Members in der DB updaten
	for (chMemIter=chMember.begin(); chMemIter != chMember.end(); chMemIter++)
	{
		std::string intf_id = chMemIter->first;
		std::string chMem = chMemIter->second;

		// Die einzelnen Members aus dem String extrahieren
		boost::char_separator<char> sep(" ,");
		boost::tokenizer<boost::char_separator<char>> tok(chMem, sep);

		for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
		{
			std::string iString3 = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			iString3 += *beg + "' AND devInterface.device_dev_id=" + dev_id + ";";
			std::vector<std::vector<std::string> > result = dieDB->query(iString3.c_str());

			std::string chMemID = "";
			if (!result.empty())
			{
				chMemID = result[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0045 - Contact wktools@spoerr.org\n";
				dbgA += iString3;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}
			std::string iString4 = "UPDATE interfaces SET channel_intf_id=" + intf_id + " WHERE intf_id=" + chMemID + ";";
			dieDB->query(iString4.c_str());
		}
	}

	// Die BoundTo Members in der DB updaten -> Am Logischen Vethernet Interface das physikalische Ethernet Interface hinterlegen (ID)
	for (boundToMemIter=boundToMember.begin(); boundToMemIter != boundToMember.end(); boundToMemIter++)
	{
		std::string intf_id = boundToMemIter->first;
		std::string boundTo = boundToMemIter->second;


		std::string iString3 = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
		iString3 += boundTo + "' AND devInterface.device_dev_id=" + dev_id + ";";
		std::vector<std::vector<std::string> > result = dieDB->query(iString3.c_str());

		std::string boundTo_ID = "";
		if (!result.empty())
		{
			boundTo_ID = result[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0018 - Contact wktools@spoerr.org\n";
			dbgA += iString3;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}
		std::string iString4 = "UPDATE interfaces SET boundTo_id=" + boundTo_ID + " WHERE intf_id=" + intf_id + ";";
		dieDB->query(iString4.c_str());
	}

}


void WkmParserCisco::nxosVPCParser()
{
	zeitStempel("\tnxVPCParser START");

	aktPos1 = ganzeDatei.find("Active vlans", pos7);
	aktPos1 = ganzeDatei.find("-----------------------------------", aktPos1);
	aktPos1 = ganzeDatei.find(" ", aktPos1);
	if (aktPos1 != ganzeDatei.npos)
	{
		aktPos2 = ganzeDatei.rfind("\n", aktPos1);
		if (aktPos2+10 < aktPos1)
		{
			aktPos2 = ganzeDatei.rfind("\r", aktPos1);
		}
		std::string vpcID = ganzeDatei.substr(aktPos2, aktPos1-aktPos2);	

		aktPos1 = ganzeDatei.find_first_not_of(" ", aktPos1);
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		std::string vpcLink = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		// Die vPCs markieren
		std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
		sString += vpcLink + "' AND devInterface.device_dev_id=" + dev_id +";";
		std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

		vpcPeerLink = vpcLink;

		std::string iString = "";
		if (!result.empty())
		{
			iString = "UPDATE interfaces SET vpc_id=" + vpcID + " WHERE intf_id = " + result[0][0] + ";";
			dieDB->query(iString.c_str());
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0020 - Contact wktools@spoerr.org\n";
			dbgA += sString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// vPC Interface in DB einfügen und auf den Peer Link verweisen
		iString = "INSERT INTO interfaces (intfName, macAddress, l2l3, status, intfType, vpc_id) VALUES ('vPC Peer-Link','','L2','UP','vPC Peer-Link'," + vpcID + ");";
		dieDB->query(iString.c_str());

		// devInterface Tabelle mit Infos zu vPC Peer-Link befüllen
		std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
		dieDB->query(iString2.c_str());

		aktPos1 = ganzeDatei.find("Active vlans", aktPos2);
		aktPos1 = ganzeDatei.find_first_not_of("- \r\n", aktPos1+15);

		if (aktPos1 != ganzeDatei.npos)
		{
			size_t vpcende = pos9;
			// TODO: Bessere Lösung finden, als ganzeDatei.size()
			//if (pos9 < vpcende)
			//{
			//	vpcende = pos9;
			//}
			std::string vpcString = ganzeDatei.substr(aktPos1-1, vpcende-aktPos1);
			std::string vpcZeile = "";

			std::string id = "";
			std::string port = "";
			std::string status = "";

			aktPos1 = 0;
			aktPos2 = 0;

			typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
			boost::char_separator<char> sep(" \r\n\t");

			for (; aktPos2 < vpcString.npos;)
			{
				aktPos2 = vpcString.find_first_of("\r\n", aktPos1 + 1);
				vpcZeile = vpcString.substr(aktPos1 + 1, aktPos2 - aktPos1-1);
				aktPos1 = aktPos2;
				if (vpcZeile.size() > 50 && vpcZeile[0] != ' ')
				{
					int i = 0;
					tokenizer tokens(vpcZeile, sep);
					for (tokenizer::iterator beg = tokens.begin();	beg != tokens.end(); ++beg)
					{
						switch (i)
						{
						case 0:
							id = *beg;
							break;
						case 1:
							port = *beg;
							break;
						case 2:
							status = *beg;
						default:
							break;
						}
						i++;
					}
					// Nur vPCs, die up sind, markieren
					if (status.find("down") == status.npos)
					{
						// Die vPCs markeren
						sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
						sString += port + "' AND devInterface.device_dev_id=" + dev_id +";";
						result = dieDB->query(sString.c_str());

						if (!result.empty())
						{
							std::string iString = "UPDATE interfaces SET vpc_id=" + id + " WHERE intf_id = " + result[0][0] + ";";
							dieDB->query(iString.c_str());
						}
						else
						{
							std::string dbgA = "\n6305: Parser Error! Code 0021 - Contact wktools@spoerr.org\n";
							dbgA += sString;
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
								WkLog::WkLog_ROT, WkLog::WkLog_FETT);
							fehlerString += "\n" + dateiname + dbgA;
						}
					}
				}
			}	
		}
	}
}


void WkmParserCisco::nxosSTPParser()
{
	zeitStempel("\tnxSTPParser START");

	// Teil 1: Globale und VLAN bezogene STP Infos
	size_t stppos = ganzeDatei.find("Spanning Tree protocol", pos4);
	bool mst = false; // MST oder STP?
	while (stppos < ganzeDatei.npos)
	{
		size_t portPos = ganzeDatei.find(" Port ", stppos);
		aktPos1 = ganzeDatei.rfind(" VLAN", stppos);
		if (aktPos1 > pos5 || aktPos1 < pos4)
		{
			aktPos1 = ganzeDatei.rfind(" MST", stppos) + 4;
			mst = true;
		}
		else
		{
			aktPos1 += 5;
		}
		stppos = ganzeDatei.find("Spanning Tree protocol", stppos + 20);

		// VLAN ID oder MST ID
		std::string vlanID = ganzeDatei.substr(aktPos1, 4);

		// STP Protocol
		aktPos1 += 22;
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		stpProtocol = ganzeDatei.substr(aktPos1, aktPos2-aktPos1); 
		// STP Protokoll Normalizing
		boost::algorithm::to_lower(stpProtocol);


		// Bridge Prio und ID
		std::string stpPrio = "";
		stpBridgeID = "";
		if ((aktPos1 = ganzeDatei.find("Bridge Identifier has priority ", aktPos1)) < stppos)
		{
			aktPos1 += 31;
			aktPos2 = ganzeDatei.find(",", aktPos1);
			stpPrio = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos2);
			if ((aktPos1 = ganzeDatei.find("address", aktPos1)) < aktPos2)
			{
				stpBridgeID = ganzeDatei.substr(aktPos1+8, 14);
			}
		}

		// ROOT ID und ROOT Port
		std::string stpRootID = "";
		std::string stpRootPort = "";
		if ((aktPos1 = ganzeDatei.find("Current root has priority ", aktPos1)) < stppos)
		{
			aktPos1 += 26;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			if ((aktPos1 = ganzeDatei.find("address", aktPos1)) < aktPos2)
			{
				stpRootID = ganzeDatei.substr(aktPos1+8, 14);
			}				

			if ((aktPos1 = ganzeDatei.find("Root port is", aktPos1)) < stppos)
			{
				aktPos1 = ganzeDatei.find("(", aktPos1)+1;
				aktPos2 = ganzeDatei.find(")", aktPos1);
				stpRootPort = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				stpRootPort = intfNameChange(stpRootPort);
			}
		}
		else if ((aktPos1 = ganzeDatei.find("We are the root of the spanning tree", aktPos1)) < stppos)
		{
			stpRootID = stpBridgeID;
			stpRootPort = "";
		}
		// stpInstanz Tabelle befüllen
		std::string iString = "INSERT INTO stpInstanz (stp_id, stpPriority, rootPort) VALUES (NULL, '";
		iString += stpPrio + "', '" + stpRootPort + "');";
		dieDB->query(iString.c_str());

		std::string sString = "SELECT last_insert_rowid() FROM stpInstanz;";
		std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
		std::string stpInstanzID = "";
		if (!result.empty())
		{
			stpInstanzID = result[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0022 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}


		// VLAN Tabelle befüllen
		std::string vlID = "";
		if (!mst)
		{
			iString = "INSERT INTO vlan (vlan_id, vlan) VALUES (NULL, " + vlanID + ");";
			dieDB->query(iString.c_str());

			sString = "SELECT last_insert_rowid() FROM vlan;";
			std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
			if (!result2.empty())
			{
				vlID = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0023 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}


			// vlan_stpInstanz Tabelle befüllen
			iString = "INSERT INTO vlan_stpInstanz (vlan_vlan_id, stpInstanz_stp_id) VALUES (";
			iString += vlID + "," + stpInstanzID + ");";
			dieDB->query(iString.c_str());

			// vlan_has_device Tabelle befüllen
			iString = "INSERT INTO vlan_has_device (vlan_vlan_id, device_dev_id) VALUES (";
			iString += vlID + "," + dev_id + ");";
			dieDB->query(iString.c_str());
		}



		// Teil 2: Interfacebezogene Infos
		while (portPos < stppos)
		{
			aktPos1 = portPos;
			portPos = ganzeDatei.find(" Port ", portPos + 400);

			// Wird nur berücksichtigt, wenn BPDU received != 0
			size_t bpduCntPos = ganzeDatei.find(", received 0", aktPos1);
			if (bpduCntPos < portPos)
			{
				continue;
			}

			bpduCntPos = ganzeDatei.find(", received ", aktPos1);
			if (bpduCntPos < portPos)
			{
				// Interfacename und STP Status
				std::string intfName = "";
				std::string stpStatus = "";
				if ((aktPos1 = ganzeDatei.find("(", aktPos1)) < portPos)
				{
					aktPos1 += 1;
					aktPos2 = ganzeDatei.find_first_of(",)", aktPos1);
					intfName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					intfName = intfNameChange(intfName);

					aktPos1 = ganzeDatei.find(" is ", aktPos2) + 4;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					stpStatus = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				}

				// ROOT ID und Designated Bridge ID
				std::string stpRootIDPort = "";
				std::string stpDesignatedBridgeID = "";
				if ((aktPos1 = ganzeDatei.find("address", aktPos1)) < portPos)
				{
					aktPos1 += 8;
					stpRootIDPort = ganzeDatei.substr(aktPos1, 14);

					aktPos1 = ganzeDatei.find("address", aktPos1) + 8;
					stpDesignatedBridgeID = ganzeDatei.substr(aktPos1, 14);
				}

				// Transition to Forwarding State (grün: 0 oder 1; orange: 1-10; rot: > 10)
				std::string stpTransitionNumber = "";
				if ((aktPos1 = ganzeDatei.find("Number of transitions to forwarding state: ", aktPos1)) < portPos)
				{
					aktPos1 += 43;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					stpTransitionNumber = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				}

				// stp_status Tabelle befüllen
				iString = "INSERT INTO stp_status (stp_status_id, stpIntfStatus, designatedRootID, designatedBridgeID, stpTransitionCount) VALUES (NULL, '";
				iString += stpStatus + "','" + stpRootIDPort  + "','" + stpDesignatedBridgeID + "'," + stpTransitionNumber + ");";
				dieDB->query(iString.c_str());

				// Interface ID auslesen
				iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
				iString += intfName + "' AND devInterface.device_dev_id=" + dev_id +";";
				std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
				std::string intfID = "";
				if (!result.empty())
				{
					intfID = result[0][0];
				}
				else
				{
					std::string dbgA = "\n6406: Parser Error! Code 0043 - If Interface '" + intfName + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
						WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
					fehlerString += "\n" + dateiname + dbgA;
					continue;
				}

				if (!mst)
				{
					// int_vlan Tabelle befüllen
					iString = "INSERT INTO int_vlan (interfaces_intf_id, vlan_vlan_id, stp_status_stp_status_id) VALUES (";
					iString += intfID + "," + vlID + ",last_insert_rowid());";
					dieDB->query(iString.c_str());
				}
			}
		}
	}
	std::string iString = "UPDATE device SET stpBridgeID='" + stpBridgeID + "', stpProtocol='" + stpProtocol + "' WHERE dev_id=" + dev_id + ";";
	dieDB->query(iString.c_str());

	iString = "INSERT INTO interfaces (intfName, macAddress, l2l3, status, intfType) VALUES ('STP','" + stpBridgeID + "','L2','UP','STP');";
	dieDB->query(iString.c_str());

}


void WkmParserCisco::pixOSrouteParser()
{
	zeitStempel("\tpixOSrouteParser START");
	// Zwei Varianten: ALT und NEU; NEU ähnlich IOS;
	// Unterscheidung: Wenn "via" gefunden wird, dann NEU, ansonsten ALT

	size_t routePos = ganzeDatei.find("via", pos16);

	// Wenn true, dann NEU
	if (routePos < ganzeDatei.npos)
	{
		aktPos1 = routePos-45;
		aktPos2 = ganzeDatei.find("#", aktPos1);
		std::string routes = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		//aktPos1 = routes.find("\r\n ");
		//while (aktPos1 < routes.npos)
		//{
		//	routes.erase(aktPos1, 4);
		//	aktPos1 = routes.find("\r\n  ", aktPos1);
		//}

		std::string rZeile = "";

		// Vorige Werte; Werden dann verwendet, wenn mehrere Next Hops vorhanden sind
		std::string vPrefix = "";		// voriger Prefix
		std::string vRP = "";			// voriges Routing Protokoll
		std::string vIP = "";			// vorige IP
		bool vEinfuegen = false;		// Zeigt an, ob die v Werte eingefügt werden sollen
		aktPos2 = routes.find_first_of("\r\n");
		aktPos1 = 0;

		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> sep(" ,\r\n\t");

		while (aktPos2 != routes.npos)
		{
			rZeile = routes.substr(aktPos1, aktPos2 - aktPos1);
			aktPos1 = aktPos2;

			if (rZeile.length() < 17)
			{
				aktPos2 = routes.find("\n", aktPos1+1);
				continue;
			}

			std::string rp = "";

			if (rZeile.find("subnetted") != rZeile.npos)
			{
				std::size_t ppos1 = rZeile.find("/");
				std::size_t ppos2 = rZeile.find(" ", ppos1);
				vPrefix = rZeile.substr(ppos1, ppos2-ppos1);
				aktPos2 = routes.find("\n", aktPos1+1);
				continue;
			}
			if (rZeile[0] != ' ')
			{
				rp = rZeile.substr(0, 5);
				rZeile = rZeile.substr(5, rZeile.npos);
			}

			tokenizer tokens(rZeile, sep);
			int i = 0;
			std::string teil1 = "";
			std::string ip = "";
			std::string prefix = "";
			std::string nh = "";
			std::string intf = "";

			for (tokenizer::iterator tok_iter = tokens.begin();	tok_iter != tokens.end(); ++tok_iter)
			{
				switch (i)
				{
				case 0:		// IP + Prefix
					teil1 = *tok_iter;
					if (teil1[0] == '[')		// Wenn mehere Wege möglich sind, dann werden die zuätzlichen Next Hop Adressen in der nächsten Zeile, ohne die restlichen Infos angezeigt
					{
						vEinfuegen = true;
						i = i+2;
					}
					else
					{
						vRP = rp;
						vEinfuegen = false;
						ip = *tok_iter;
						vIP = ip;
					}
					break;
				case 1:		// Subnetmask
					prefix = *tok_iter;
					vPrefix = prefix;
					break;
				case 4:		// Next Hop
					nh = *tok_iter;
					if (vEinfuegen)
					{
						ip = vIP;
						prefix = vPrefix;
						rp = vRP;
					}
					break;
				case 5:		// Interface bei statischen Routen
					intf = *tok_iter;
					intf = intfNameChange(intf);
					break;
				case 6:		// Interface
					intf = *tok_iter;
					intf = intfNameChange(intf);
					break;
				default:
					break;
				}
				i++;

			}

			// Den Eintrag nur schreiben, wenn es einen Next Hop gibt. Manchmal wird ein Routing Eintrag in zwei Zeilen geschrieben und damit wird ein doppelter Eintrag in die DB verhindert
			if (nh != "")
			{
				// Als erstes die Maske in /Bits umwandeln; Die ASA/PIX schreibt die Routingtabelle mit Subnetmask anstatt /Bits
				prefix = maskToBits(prefix);

				std::string iString = "INSERT INTO l3routes (l3r_id,protocol,network,subnet,nextHop,interface,type)";
				iString += " VALUES(NULL, '" + rp + "','" + ip + "','" + prefix + "','" + nh + "','" + intf + "',NULL);";
				dieDB->query(iString.c_str());

				std::string sString = "SELECT last_insert_rowid() FROM l3routes;";
				std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
				std::string l3r_id = "";
				if (!result2.empty())
				{
					l3r_id = result2[0][0];
				}
				else
				{
					std::string dbgA = "\n6305: Parser Error! Code 0024 - Contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehlerString += "\n" + dateiname + dbgA;
				}


				// rlink Tabelle befüllen
				// Schritt 1: Next Hop Interface auslesen; Wenn es nicht schon feststeht, dann über das Subnet rausfinden
				if (intf != "")
				{
					std::string sString = "SELECT intf_id FROM interfaces ";
					sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
					sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
					sString += "WHERE device.dev_id=" + dev_id + " AND interfaces.nameif LIKE '" + intf + "'";
					std::vector<std::vector<std::string> > r3 = dieDB->query(sString.c_str());
					if (!r3.empty())
					{
						iString = "INSERT INTO rlink (interfaces_intf_id, l3routes_l3r_id) VALUES (" + r3[0][0] + "," + l3r_id + ");";
						dieDB->query(iString.c_str());
					}
					else
					{
						std::string dbgA = "\n6305: Parser Error! Code 0025 - Contact wktools@spoerr.org\n";
						dbgA += sString;
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehlerString += "\n" + dateiname + dbgA;
					}

				}
				else
				{
					std::string intid = nextHopIntfCheck(nh);
					if (intid == "")
					{
						intid = "NULL";
					}
					iString = "INSERT INTO rlink (interfaces_intf_id, l3routes_l3r_id) VALUES (" + intid + "," + l3r_id + ");";
					dieDB->query(iString.c_str());

					if (intid != "NULL")
					{
						std::string sString = "SELECT intfName FROM interfaces WHERE intf_id=" + intid;
						std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

						if (!ret.empty())
						{
							iString = "UPDATE l3routes SET interface='" + ret[0][0] + "' WHERE l3r_id = " + l3r_id + ";";
							dieDB->query(iString.c_str());
						}
						else
						{
							std::string dbgA = "\n6305: Parser Error! Code 0026 - Contact wktools@spoerr.org\n";
							dbgA += sString;
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
								WkLog::WkLog_ROT, WkLog::WkLog_FETT);
							fehlerString += "\n" + dateiname + dbgA;
						}
					}
				}


				//std::string astring = "<" + rp + "><" + ip + "><" + prefix + "><" + nh + "><" + intf + ">\n";
				//schreibeLog(WkLog::WkLog_NORMALTYPE, astring, WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
			}
			aktPos2 = routes.find_first_of("\r\n", aktPos1+1);
		}
	}
	else
	{
		// ALT (<7.0)
		routePos = ganzeDatei.find("OTHER", pos16);
		if (routePos != ganzeDatei.npos)
		{
			aktPos1 = pos16;
			aktPos2 = ganzeDatei.find("#", aktPos1);
			std::string routes = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			std::string rZeile = "";

			// Vorige Werte; Werden dann verwendet, wenn mehrere Next Hops vorhanden sind
			std::string vPrefix = "";		// voriger Prefix
			std::string vRP = "";			// voriges Routing Protokoll
			std::string vIP = "";			// vorige IP
			bool vEinfuegen = false;		// Zeigt an, ob die v Werte eingefügt werden sollen
			aktPos2 = routes.find_first_of("\r\n");
			aktPos1 = 0;
			typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
			boost::char_separator<char> sep(" ,\r\n");

			while (aktPos2 != routes.npos)
			{
				rZeile = routes.substr(aktPos1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;

				if (rZeile.length() < 17)
				{
					aktPos2 = routes.find("\n", aktPos1+1);
					continue;
				}

				std::string rp = "";

				// Connected Routes ignorieren
				if (rZeile.find("CONNECT") != rZeile.npos)
				{
					aktPos2 = routes.find("\n", aktPos1+1);
					continue;
				}
				tokenizer tokens(rZeile, sep);
				int i = 0;
				std::string teil1 = "";
				std::string ip = "";
				std::string prefix = "";
				std::string nh = "";
				std::string intf = "";

				for (tokenizer::iterator tok_iter = tokens.begin();	tok_iter != tokens.end(); ++tok_iter)
				{
					switch (i)
					{
					case 0:		// Interface
						intf = *tok_iter;
						break;
					case 1:		// Netz
						ip = *tok_iter;
						break;
					case 2:		// Mask
						prefix = *tok_iter;
						break;
					case 3:		// Next Hop
						nh = *tok_iter;
						break;
					case 6:		// Protokoll
						rp = *tok_iter;
						break;
					default:
						break;
					}
					i++;
				}

				// Als erstes die Maske in /Bits umwandeln; Die ASA/PIX schreibt die Routingtabelle mit Subnetmask anstatt /Bits
				prefix = maskToBits(prefix);

				// Dann in die DB schreiben
				std::string iString = "INSERT INTO l3routes (l3r_id,protocol,network,subnet,nextHop,interface,type)";
				iString += " VALUES(NULL, '" + rp + "','" + ip + "','" + prefix + "','" + nh + "','" + intf + "',NULL);";
				dieDB->query(iString.c_str());

				std::string sString = "SELECT last_insert_rowid() FROM l3routes;";
				std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
				std::string l3r_id = "";
				if (!result2.empty())
				{
					l3r_id = result2[0][0];
				}
				else
				{
					std::string dbgA = "\n6305: Parser Error! Code 0027 - Contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehlerString += "\n" + dateiname + dbgA;
				}

				// rlink Tabelle befüllen
				sString = "SELECT intf_id FROM interfaces ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
				sString += "WHERE device.dev_id=" + dev_id + " AND interfaces.nameif LIKE '" + intf + "'";
				std::vector<std::vector<std::string> > r3 = dieDB->query(sString.c_str());
				if (!r3.empty())
				{
					iString = "INSERT INTO rlink (interfaces_intf_id, l3routes_l3r_id) VALUES (" + r3[0][0] + "," + l3r_id + ");";
					dieDB->query(iString.c_str());
				}
				else
				{
					std::string dbgA = "\n6305: Parser Error! Code 0028 - Contact wktools@spoerr.org\n";
					dbgA += sString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehlerString += "\n" + dateiname + dbgA;
				}

				aktPos2 = routes.find_first_of("\r\n", aktPos1+1);
			}
		}
	}
}


void WkmParserCisco::pixOSOSPFParser()
{
	zeitStempel("\tpixOSOSPFParser START");
}


void WkmParserCisco::pixOSEIGRPParser()
{
	zeitStempel("\tpixOSEIGRPParser START");
}


void WkmParserCisco::ipsecParser()
{
	zeitStempel("\tipsecParser START");
}


void WkmParserCisco::iosBGPParser()
{
	zeitStempel("\tiosBGPParser START");
}


void WkmParserCisco::iosOSPFParser()
{
	zeitStempel("\tiosOSPFParser START");
}


void WkmParserCisco::iosEIGRPParser()
{
	zeitStempel("\tiosEIGRPParser START");
}


void WkmParserCisco::iosRouteParser()
{
	zeitStempel("\tiosRouteParser START");
	size_t routePos = ganzeDatei.find("Gateway of last resort", pos9);
	aktPos1 = ganzeDatei.find_first_of("\r\n", routePos);

	if (routePos > pos10)
	{
		routePos = ganzeDatei.find("via", pos9);
		aktPos1 = routePos-45;
	}

	if (routePos < pos10)
	{
		std::string routes = ganzeDatei.substr(aktPos1, pos10-aktPos1);
		//aktPos1 = routes.find("\r\n ");
		//while (aktPos1 < routes.npos)
		//{
		//	routes.erase(aktPos1, 4);
		//	aktPos1 = routes.find("\r\n  ", aktPos1);
		//}

		std::string rZeile = "";

		// Vorige Werte; Werden dann verwendet, wenn mehrere Next Hops vorhanden sind
		std::string vPrefix = "";		// voriger Prefix
		std::string vRP = "";			// voriges Routing Protokoll
		std::string vIP = "";			// vorige IP
		bool vEinfuegen = false;		// Zeigt an, ob die v Werte eingefügt werden sollen
		aktPos2 = routes.find_first_of("\r\n");
		aktPos1 = 0;

		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> sep(" ,\r\n");

		while (aktPos2 != routes.npos)
		{
			rZeile = routes.substr(aktPos1, aktPos2 - aktPos1);
			aktPos1 = aktPos2;

			if (rZeile.length() < 17)
			{
				aktPos2 = routes.find("\n", aktPos1+1);
				continue;
			}

			std::string rp = "";

			if (rZeile.find("subnetted") != rZeile.npos)
			{
				std::size_t ppos1 = rZeile.find("/");
				std::size_t ppos2 = rZeile.find(" ", ppos1);
				vPrefix = rZeile.substr(ppos1, ppos2-ppos1);
				aktPos2 = routes.find("\n", aktPos1+1);
				continue;
			}
			if (rZeile[0] != ' ')
			{
				rp = rZeile.substr(0, 5);
				rZeile = rZeile.substr(5, rZeile.npos);
			}

			tokenizer tokens(rZeile, sep);
			int i = 0;
			std::string teil1 = "";
			std::string ip = "";
			std::string prefix = "";
			std::string nh = "";
			std::string intf = "";

			for (tokenizer::iterator tok_iter = tokens.begin();	tok_iter != tokens.end(); ++tok_iter)
			{
				switch (i)
				{
				case 0:		// IP + Prefix
					teil1 = *tok_iter;
					if (teil1[0] == '[')		// Wenn mehere Wege möglich sind, dann werden die zuätzlichen Next Hop Adressen in der nächsten Zeile, ohne die restlichen Infos angezeigt
					{
						vEinfuegen = true;
						i++;
					}
					else
					{
						vRP = rp;
						vEinfuegen = false;
						ip = *tok_iter;
						size_t ppos = ip.find("/");
						if (ppos != ip.npos)
						{
							prefix = ip.substr(ppos);
							ip.erase(ppos);
						}
						else
						{
							prefix = vPrefix;
						}
						vPrefix = prefix;
						vIP = ip;
					}
					break;
				case 3:		// Next Hop
					nh = *tok_iter;
					if (vEinfuegen)
					{
						ip = vIP;
						prefix = vPrefix;
						rp = vRP;
					}
					break;
				case 4:		// Interface bei statischen Routen
					intf = *tok_iter;
					intf = intfNameChange(intf);
					break;
				case 5:		// Interface
					intf = *tok_iter;
					intf = intfNameChange(intf);
					break;
				default:
					break;
				}
				i++;

			}

			// Den Eintrag nur schreiben, wenn es einen Next Hop gibt. Manchmal wird ein Routing Eintrag in zwei Zeilen geschrieben und damit wird ein doppelter Eintrag in die DB verhindert
			if (nh != "")
			{

				std::string iString = "INSERT INTO l3routes (l3r_id,protocol,network,subnet,nextHop,interface,type)";
				iString += " VALUES(NULL, '" + rp + "','" + ip + "','" + prefix + "','" + nh + "','" + intf + "',NULL);";
				dieDB->query(iString.c_str());

				std::string sString = "SELECT last_insert_rowid() FROM l3routes;";
				std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
				std::string l3r_id = "";
				if (!result2.empty())
				{
					l3r_id = result2[0][0];
				}
				else
				{
					std::string dbgA = "\n6305: Parser Error! Code 0029 - Contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehlerString += "\n" + dateiname + dbgA;
				}


				// rlink Tabelle befüllen
				// Schritt 1: Next Hop Interface auslesen; Wenn es nicht schon feststeht, dann über das Subnet rausfinden
				if (intf != "")
				{
					std::string sString = "SELECT intf_id FROM interfaces ";
					sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
					sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
					sString += "WHERE device.dev_id=" + dev_id + " AND interfaces.intfName LIKE '" + intf + "'";
					std::vector<std::vector<std::string> > r3 = dieDB->query(sString.c_str());
					if (!r3.empty())
					{
						iString = "INSERT INTO rlink (interfaces_intf_id, l3routes_l3r_id) VALUES (" + r3[0][0] + "," + l3r_id + ");";
						dieDB->query(iString.c_str());
					}
					else
					{
						std::string dbgA = "\n6305: Parser Error! Code 0030 - Contact wktools@spoerr.org\n";
						dbgA += sString;
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehlerString += "\n" + dateiname + dbgA;
					}

				}
				else
				{
					std::string intid = nextHopIntfCheck(nh);
					if (intid == "")
					{
						intid = "NULL";
					}
					iString = "INSERT INTO rlink (interfaces_intf_id, l3routes_l3r_id) VALUES (" + intid + "," + l3r_id + ");";
					dieDB->query(iString.c_str());

					if (intid != "NULL")
					{
						std::string sString = "SELECT intfName FROM interfaces WHERE intf_id=" + intid;
						std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

						if (!ret.empty())
						{
							iString = "UPDATE l3routes SET interface='" + ret[0][0] + "' WHERE l3r_id = " + l3r_id + ";";
							dieDB->query(iString.c_str());
						}
						else
						{
							std::string dbgA = "\n6305: Parser Error! Code 0031 - Contact wktools@spoerr.org\n";
							dbgA += sString;
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
								WkLog::WkLog_ROT, WkLog::WkLog_FETT);
							fehlerString += "\n" + dateiname + dbgA;
						}
					}
				}


				//std::string astring = "<" + rp + "><" + ip + "><" + prefix + "><" + nh + "><" + intf + ">\n";
				//schreibeLog(WkLog::WkLog_NORMALTYPE, astring, WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
			}
			aktPos2 = routes.find_first_of("\r\n", aktPos1+1);
		}
	}
}


void WkmParserCisco::iosVSLParser()
{
	zeitStempel("\tiosVSLParser START");

	aktPos1 = ganzeDatei.find("vfsp", pos17);
	if (aktPos1 != ganzeDatei.npos)
	{
		aktPos1 = ganzeDatei.rfind("-------", aktPos1);
		aktPos2 = ganzeDatei.find("Flags:", aktPos1);

		std::string vslLink = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		aktPos1 = vslLink.find("1/");
		while (aktPos1 != vslLink.npos)
		{
			aktPos1 = vslLink.rfind("\n", aktPos1)+1;
			aktPos2 = vslLink.find(" ", aktPos1);
			std::string sw1intf = vslLink.substr(aktPos1, aktPos2-aktPos1);

			aktPos1 = vslLink.find("2/", aktPos2);
			aktPos1 = vslLink.rfind(" ", aktPos1) + 1;
			aktPos2 = vslLink.find(" ", aktPos1);
			std::string sw2intf = vslLink.substr(aktPos1, aktPos2-aktPos1);

			// Die VSLs markeren
			// Physikalische Interfaces SW 1
			std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			sString += sw1intf + "' AND devInterface.device_dev_id=" + dev_id +";";
			std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

			if (!result.empty())
			{
				std::string iString = "UPDATE interfaces SET phl=2 WHERE intf_id = " + result[0][0] + ";";
				dieDB->query(iString.c_str());
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0032 - Contact wktools@spoerr.org\n";
				dbgA += sString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}

			// Physikalische Interfaces SW 2
			sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			sString += sw2intf + "' AND devInterface.device_dev_id=" + dev_id +";";
			result = dieDB->query(sString.c_str());

			if (!result.empty())
			{
				std::string iString = "UPDATE interfaces SET phl=2 WHERE intf_id = " + result[0][0] + ";";
				dieDB->query(iString.c_str());
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0033 - Contact wktools@spoerr.org\n";
				dbgA += sString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}



			aktPos1 = vslLink.find("1/", aktPos2);
		}

		// VSL Port Channel markieren
		std::string iString = "UPDATE interfaces SET phl=2 WHERE intf_id IN (SELECT channel_intf_id FROM interfaces WHERE phl=2)";
		dieDB->query(iString.c_str());
	}
}


void WkmParserCisco::iosSwitchportParser()
{
	zeitStempel("\tiosSwitchportParser START");

	size_t intfpos = ganzeDatei.find("Name: ", pos20);

	while (intfpos < ganzeDatei.npos)
	{
		// Interface Name
		aktPos1 = intfpos+6;
		aktPos2 = aktPos1;
		intfpos = ganzeDatei.find("Name: ", aktPos1);

		std::string intfName = "";
		aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
		intfName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		intfName = intfNameChange(intfName);

		// Uninteressante Interfaces überspringen
		std::size_t opModePos = ganzeDatei.find("Operational Mode: ", aktPos2);
		if ((ganzeDatei.find("Operational Mode: down", aktPos2)) < intfpos)
		{
			// Weiter zum nächsten Interface, da dieses Down ist
			continue;
		}
		else if ((opModePos > intfpos) || (opModePos > pos21))
		{
			// Weiter zum nächsten Interface, da dieses ungenügend Informationen bereitstellt
			continue;
		}
		if ((ganzeDatei.find("Switchport: Disabled", aktPos2)) < intfpos)
		{
			// Weiter zum nächsten Interface, da dieses ein routed Port ist
			continue;
		}

		// Infos sammeln
		// Als erstes die Interface-ID auslesen
		std::string sString = "SELECT interfaces_int_id FROM devInterface INNER JOIN interfaces on interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "WHERE interfaces.intfName LIKE '" + intfName + "' AND device_dev_id=" + dev_id +";";
		std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
		if (!result.empty())
		{
			intf_id = result[0][0];
		}
		else
		{
			// Wenn das Interface nicht erfasst ist, dann weiter zum nächsten Interface
			std::string dbgA = "\n6406: Parser Error! Code 0044 - If Interface '" + intfName + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
			dbgA += sString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
			continue;
		}

		// Dann die verfügbaren Daten parsen
		std::string l2AdminMode = "";
		std::string l2Mode = "";
		std::string l2encap = "";
		std::string dtp = "";
		std::string nativeAccess = "";
		std::string voiceVlan = "";
		std::string opPrivVlan = "";
		std::string allowedVlan = ""; 
		if ((aktPos1 = ganzeDatei.find("Administrative Mode: ", aktPos2)) < intfpos)
		{
			aktPos1 += 21;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			l2AdminMode = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find("Operational Mode: ", aktPos2)) < intfpos)
		{
			aktPos1 += 18;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			l2Mode = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find("Operational Trunking Encapsulation: ", aktPos2)) < intfpos)
		{
			aktPos1 += 36;
			aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
			l2encap = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find("Negotiation of Trunking: ", aktPos2)) < intfpos)
		{
			aktPos1 += 25;
			aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
			dtp = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		if (l2Mode.find("access") != l2Mode.npos)
		{
			if ((aktPos1 = ganzeDatei.find("Access Mode VLAN: ", aktPos2)) < intfpos)
			{
				aktPos1 += 18;
				aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
				nativeAccess = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
		}
		else
		{
			if ((aktPos1 = ganzeDatei.find("Trunking Native Mode VLAN: ", aktPos2)) < intfpos)
			{
				aktPos1 += 27;
				aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
				nativeAccess = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			}
		}

		if ((aktPos1 = ganzeDatei.find("Voice VLAN: ", aktPos2)) < intfpos)
		{
			aktPos1 += 12;
			aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
			voiceVlan = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		size_t tempPos = aktPos2;
		if ((aktPos1 = ganzeDatei.find("Operational private-vlan: ", tempPos)) < intfpos)
		{
			aktPos1 += 26;
			aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
			opPrivVlan = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		if ((aktPos1 = ganzeDatei.find("Trunking VLANs Enabled: ", tempPos)) < intfpos)
		{
			aktPos1 += 24;
			aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
			allowedVlan = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		// Interface Infos updaten
		sString = "UPDATE interfaces SET l2AdminMode='" + l2AdminMode + "',l2Mode='" + l2Mode + "', l2encap='" + l2encap + "', dtp='" + dtp;
		sString += "', nativeAccess='" + nativeAccess + "', voiceVlan='" + voiceVlan + "', opPrivVlan='" + opPrivVlan + "', allowedVlan='" + allowedVlan;
		sString += "' WHERE intf_id=" + intf_id + ";";

		dieDB->query(sString.c_str());
	}
}


void WkmParserCisco::insertRControl()
{
	const int index = 14;
	std::string tableNames[] = {"cdp", "device", "interfaces", "vlan", "ipSubnet", "stp_status", "stpInstanz", "neighbor", "l3routes", "ipPhoneDet", "hwInfo", "license", "flash", "vrf"};
	std::string indexNames[] = {"cdp_id", "dev_id", "intf_id", "vlan_id", "ipSubnet_id", "stp_status_id", "stp_id", "neighbor_id", "l3r_id", "ipphone_id", "hwinf_id", "license_id", "flash_id", "vrf_id"};
	std::string rIndex[index];

	for (int i=0; i<index; i++)
	{
		std::string sString = "SELECT MAX(" + indexNames[i] + ") FROM " + tableNames[i] + ";";
		std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
		rIndex[i] = "";
		if (!result.empty())
		{
			rIndex[i] = result[0][0];
			if (rIndex[i] == "")
			{
				rIndex[i] = "0";
			}
		}
		else
		{
			rIndex[i] = "0";
		}
	}

	if (rControlEmpty)
	{
		std::string iString = "INSERT INTO rControl (rc_id,n_id,cdp_id,dev_id,intf_id,vlan_id,ipSubnet_id,stp_status_id,stp_id,neighbor_id,l3r_id,ipphone_id,hwinf_id,license_id,flash_id,vrf_id) VALUES (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);";
		dieDB->query(iString.c_str());
	}

	std::string iString = "INSERT INTO rControl (cdp_id,dev_id,intf_id,vlan_id,ipSubnet_id,stp_status_id,stp_id,neighbor_id,l3r_id,ipphone_id,hwinf_id,license_id,flash_id,vrf_id) VALUES (";
	iString += rIndex[0] + "," + rIndex[1] + "," + rIndex[2] + "," + rIndex[3] + "," + rIndex[4] + "," + rIndex[5] + "," + rIndex[6] + "," + rIndex[7] + "," + rIndex[8] + "," + rIndex[9] + "," + rIndex[10] + "," + rIndex[11] + "," + rIndex[12] + "," + rIndex[13] + ");";
	dieDB->query(iString.c_str());
}


void WkmParserCisco::iosSNMPParser()
{
	zeitStempel("\tiosSNMPParser START");

	aktPos1 = ganzeDatei.find("Location: ", pos17);
	if (aktPos1 != ganzeDatei.npos)
	{
		aktPos1 += 10;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);

		std::string snmpLoc = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		while (snmpLoc.find_first_of("'\"&<>") != snmpLoc.npos)
		{
			snmpLoc.replace(snmpLoc.find_first_of("'\"&<>"), 1, "-");
		}


		// SNMP Location in DB eintragen
		std::string iString = "UPDATE device SET snmpLoc='" + snmpLoc + "' WHERE dev_id= " + dev_id + ";";
		dieDB->query(iString.c_str());
	}
}


void WkmParserCisco::nxosSNMPParser()
{
	zeitStempel("\tnxosSNMPParser START");

	aktPos1 = ganzeDatei.find("sys location: ", pos17);
	if (aktPos1 != ganzeDatei.npos)
	{
		aktPos1 += 14;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		std::string snmpLoc = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		while (snmpLoc.find_first_of("'\"&<>") != snmpLoc.npos)
		{
			snmpLoc.replace(snmpLoc.find_first_of("'\"&<>"), 1, "-");
		}


		// SNMP Location in DB eintragen
		std::string iString = "UPDATE device SET snmpLoc='" + snmpLoc + "' WHERE dev_id= " + dev_id + ";";
		dieDB->query(iString.c_str());
	}
}


void WkmParserCisco::testPhones()
{
	confReg = "";
	hwPos = "box"; 
	modell = hwDescription = chassisSN = hwRevision = hwMem = hwBootfile = version = "";
	devicetype = PHONE;
	devType = "11";

	size_t htmlPos = ganzeDatei.find("MAC");
	size_t titlePos = ganzeDatei.find("</TITLE>");
	if (htmlPos != ganzeDatei.npos)
	{
		std::string htmlParse = ganzeDatei.substr(htmlPos);
		std::string standort = "";

		aktPos1 = 0;
		aktPos2 = 0;

		// Beschreibung
		if ((aktPos1 = ganzeDatei.find("Cisco IP Phone")) != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("<(", aktPos1);
			hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = ganzeDatei.find("Cisco Unified")) != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("<(", aktPos1);
			hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = ganzeDatei.find("Cisco Systems, Inc.", titlePos)) != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("<(", aktPos1);
			hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = ganzeDatei.find("Cisco ATA")) != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("<(", aktPos1);
			hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
			devicetype = PHONE;
			devType = "13";
		}
		else
		{
			hwDescription = "";
		}

		// Alle <IconIndex>-1</IconIndex> entfernen -> 7937G
		aktPos1 = aktPos2 = 0;
		while (1)
		{
			aktPos1 = htmlParse.find("<IconIndex>-1</IconIndex>");
			if (aktPos1 == htmlParse.npos)
			{
				break;
			}
			htmlParse.erase(aktPos1, 25);
		}
		// Alle ": " auf "<>" ändern und die Leerzeichen davor und danach löschen -> 7937G
		aktPos1 = aktPos2 = 0;
		while (1)
		{
			aktPos1 = htmlParse.find(": ");
			if (aktPos1 == htmlParse.npos)
			{
				break;
			}
			htmlParse.replace(aktPos1, 2, "<>");
			aktPos1 = aktPos1+2;
			aktPos2 = htmlParse.find_first_not_of(" ", aktPos1);
			if (aktPos2 == htmlParse.npos)
			{
				break;
			}
			htmlParse.erase(aktPos1, aktPos2-aktPos1);
		}

		// alle html Tags löschen, so dass nur noch <>;& und die Texte da sind
		aktPos1 = aktPos2 = 0;
		while (1)
		{
			aktPos1 = htmlParse.find_first_of("<&;", aktPos1);
			if (aktPos1 == htmlParse.npos)
			{
				break;
			}
			aktPos1++;
			aktPos2 = htmlParse.find_first_of(">;", aktPos1);
			htmlParse = htmlParse.erase(aktPos1, aktPos2-aktPos1);
		}

		// alle Probleme löschen. Definition: bis zu drei Zeichen zwischen \r\n
		aktPos1 = aktPos2 = 0;
		while (1)
		{
			aktPos1 = htmlParse.find("\r\n", aktPos1);
			if (aktPos1 == htmlParse.npos)
			{
				break;
			}
			aktPos2 = htmlParse.find("\r\n", aktPos1+1);
			if (aktPos2-aktPos1 < 6)
			{
				aktPos2 += 2;
				// Manchmal kommen zwei Probleme gleich hintereinander; Das muss rausgefunden werden
				std::size_t endePos = htmlParse.find("\r\n", aktPos2+4);
				if (endePos-aktPos2 < 6)
				{
					endePos += 2;
					htmlParse = htmlParse.erase(aktPos1, endePos-aktPos1);
				}
				else
				{
					htmlParse = htmlParse.erase(aktPos1, aktPos2-aktPos1);
				}
			}
			aktPos1++;
		}

		// MAC Adresse
		aktPos1 = htmlParse.find_first_of("<>;&");
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		std::string macAdresse = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		macAdresse = macAddChange(macAdresse);

		// Wenn vorhanden, WLAN MAC Adresse erfassen
		std::string wlanMacAdresse = "";
		std::size_t tempPos = aktPos2;
		aktPos1 = htmlParse.find("WLAN", aktPos2);
		if (aktPos1 != ganzeDatei.npos)
		{
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			wlanMacAdresse = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (wlanMacAdresse.length() == 12)
			{
				wlanMacAdresse = macAddChange(wlanMacAdresse);
			}
			else
			{
				wlanMacAdresse = "";
				aktPos2 = tempPos;
			}
		}

		// Hostname
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos2);
		aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		hostname = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		if ((hostname.find("ata") != hostname.npos) || (hostname.find("ata") != hostname.npos))
		{
			std::size_t temppos1 = htmlParse.find("SEP");
			if (temppos1 != htmlParse.npos)
			{
				size_t temppos2 = htmlParse.find_first_of("<>;&\r\n\t ", temppos1);
				hostname = htmlParse.substr(temppos1, temppos2-temppos1);
				boost::algorithm::to_upper(hostname);
			}
		}

		// Telefonnummer -> Wird als snmpLoc abgespeichert
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos2);
		aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		standort = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		try
		{
			int temp = boost::lexical_cast<int>(standort);
		}
		catch (boost::bad_lexical_cast &)
		{
			standort = "UNKNOWN";			
		}


		// Version
		aktPos1 = htmlParse.find("ersion", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 7;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			version = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}

		// HW Revision
		aktPos1 = htmlParse.find("evision", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 8;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			hwRevision = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}

		// Seriennummer
		aktPos1 = htmlParse.find("eri");
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 4;
			aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			chassisSN = htmlParse.substr(aktPos1, aktPos2-aktPos1);

			// Type
			aktPos1 = htmlParse.find("CP-", aktPos2);
			if (aktPos1 != htmlParse.npos)
			{
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				modell = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}
			else
			{
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos2);
				aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				modell = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}
		}

		// IP Adresse
		std::string ipa = "";
		std::string subnetMask = "";
		std::string defaultGateway = "";
		if ((aktPos1 = htmlParse.find("IP-", aktPos2)) != htmlParse.npos)
		{
			aktPos1 += 10;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			ipa = htmlParse.substr(aktPos1, aktPos2-aktPos1);

			// Subnetmask
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos2);
			aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			subnetMask = htmlParse.substr(aktPos1, aktPos2-aktPos1);

			// Default Gateway
			aktPos1 = htmlParse.find("router", aktPos2);
			if (aktPos1 != htmlParse.npos)
			{
				aktPos1 += 9;
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				defaultGateway = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}
		}
		else if ((aktPos1 = htmlParse.find("IP A", aktPos2)) != htmlParse.npos)
		{
			aktPos1 += 10;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			ipa = htmlParse.substr(aktPos1, aktPos2-aktPos1);

			tempPos = aktPos2;
			// Default Gateway
			aktPos1 = htmlParse.find("outer", aktPos2);
			if (aktPos1 != htmlParse.npos)
			{
				aktPos1 += 8;
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				defaultGateway = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}

			// Subnetmask
			aktPos1 = htmlParse.find("ask", tempPos);
			if (aktPos1 != htmlParse.npos)
			{
				aktPos1 += 5;
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				subnetMask = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}
		}

		// Voice VLAN
		std::string voiceVlan = "";
		if ((aktPos1 = htmlParse.find("VLAN-ID", aktPos2)) != htmlParse.npos)
		{
			aktPos1 += 9;
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			voiceVlan = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = htmlParse.find("VLAN Id", aktPos2)) != htmlParse.npos)
		{
			aktPos1 += 9;
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			voiceVlan = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}

		// CallManager
		std::string cm2 = "";
		std::string cm1 = "";
		aktPos1 = htmlParse.find("Unified CM 1", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			// CM1
			aktPos1 += 13;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cm1 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			// CM2
			aktPos1 = htmlParse.find("Unified CM 2", aktPos2) + 13;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cm2 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = htmlParse.find("CallManager")) != htmlParse.npos)
		{
			// CM1
			aktPos1 += 12;
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cm1 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			// CM2
			aktPos1 = htmlParse.find("CallManager", aktPos2) + 12;
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cm2 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = htmlParse.find("Call Manager")) != htmlParse.npos)
		{
			// CM1
			aktPos1 += 13;
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cm1 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			// CM2
			aktPos1 = htmlParse.find("Call Manager", aktPos2) + 13;
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cm2 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}
		else
		{
			aktPos1 = htmlParse.find("CUCM-Server1", aktPos2);
			if (aktPos1 != htmlParse.npos)
			{
				// CM1
				aktPos1 += 13;
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				cm1 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
				// CM2
				aktPos1 = htmlParse.find("CUCM-Server2", aktPos2) + 13;
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				cm2 = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}
		}


		// DSCP Sig
		std::string dscpSig = "";
		aktPos1 = htmlParse.find("DSCP", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			dscpSig = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}
		// DSCP conf
		std::string dscpConf = "";
		aktPos1 = htmlParse.find("DSCP", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			dscpConf = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}
		// DSCP Call
		std::string dscpCall = "";
		aktPos1 = htmlParse.find("DSCP", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			dscpCall = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}

		// FEHLER (Netzwerk Statistiken)
		//////////////////////////////////////////////////////////////////////////
		int tempErr = 0;
		int err = 0;
		// CRC Fehler
		std::string crcErr = "";
		aktPos1 = htmlParse.find("Rx crcErr", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 10;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			crcErr = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (crcErr == "00000000")
			{
				crcErr = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(crcErr);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Align Fehler
		std::string alignErr = "";
		aktPos1 = htmlParse.find("Rx alignErr", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 12;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			alignErr = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (alignErr == "00000000")
			{
				alignErr = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(alignErr);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// shortErr Fehler
		std::string shortErr = "";
		aktPos1 = htmlParse.find("Rx shortErr", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 12;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			shortErr = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (shortErr == "00000000")
			{
				shortErr = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(shortErr);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// longErr Fehler
		std::string longErr = "";
		aktPos1 = htmlParse.find("Rx longErr", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 12;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			longErr = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (longErr == "00000000")
			{
				longErr = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(longErr);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// tokenDrop Fehler
		std::string tokenDrop = "";
		aktPos1 = htmlParse.find("Rx tokenDrop", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 13;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			tokenDrop = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (tokenDrop == "00000000")
			{
				tokenDrop = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(tokenDrop);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// excessDefer Fehler
		std::string excessDefer = "";
		aktPos1 = htmlParse.find("Tx excessDefer", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 14;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			excessDefer = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (excessDefer == "00000000")
			{
				excessDefer = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(excessDefer);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// lateCollision Fehler
		std::string lateCollision = "";
		aktPos1 = htmlParse.find("Tx lateCollision", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 17;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			lateCollision = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (lateCollision == "00000000")
			{
				lateCollision = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(lateCollision);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// Collisions Fehler
		std::string Collisions = "";
		aktPos1 = htmlParse.find("Tx Collisions", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 14;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			Collisions = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (Collisions == "00000000")
			{
				Collisions = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(Collisions);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// excessLength Fehler
		std::string excessLength = "";
		aktPos1 = htmlParse.find("Tx excessLength", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 += 17;
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			excessLength = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			if (excessLength == "00000000")
			{
				excessLength = "0";
			}
			try
			{
				tempErr = boost::lexical_cast<int>(excessLength);
			}
			catch (boost::bad_lexical_cast &)
			{
				tempErr = 0;			
			}
			err += tempErr;
		}

		// CDP Neighbor
		std::string cdpNeigh = "";
		std::string cdpNeighIP = "";
		std::string cdpNeighPort = "";
		if ((aktPos1 = htmlParse.find("CDP", aktPos2)) != htmlParse.npos)
		{
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cdpNeigh = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = htmlParse.find("-ID", aktPos2)) != htmlParse.npos)
		{
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			cdpNeigh = htmlParse.substr(aktPos1, aktPos2-aktPos1);
		}

		if (cdpNeigh.find("-IP-") != cdpNeigh.npos)
		{
			cdpNeigh = "";
		}
		else
		{
			// CDP Neighbor IP Adresse
			if ((aktPos1 = htmlParse.find("CDP", aktPos2)) != htmlParse.npos)
			{
				aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				cdpNeighIP = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}
			else if ((aktPos1 = htmlParse.find("-IP-", aktPos2)) != htmlParse.npos)
			{
				aktPos1 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				cdpNeighIP = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			}

			// CDP Neighbor interface
			aktPos1 = htmlParse.find("Port", aktPos2);
			if (aktPos1 != htmlParse.npos)
			{
				aktPos1 += 5;
				aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
				aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
				cdpNeighPort = htmlParse.substr(aktPos1, aktPos2-aktPos1);
				cdpNeighPort = intfNameChange(cdpNeighPort);
			}
		}


		// Duplex/Speed 
		std::string speedDuplex = "";
		std::string intfType = "";
		std::string duplex = "";
		std::string speed = "";
		aktPos1 = htmlParse.find("Port-", aktPos2);
		if (aktPos1 != htmlParse.npos)
		{
			aktPos1 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
			aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
			aktPos2 = htmlParse.find_first_of("<>;&\r\n\t", aktPos1);
			speedDuplex = htmlParse.substr(aktPos1, aktPos2-aktPos1);
			intfType = intfTypeCheck("", speedDuplex);
			duplex = speedDuplex.substr(0, speedDuplex.find(","));
			speed = speedDuplex.substr(speedDuplex.find(",")+1);
		}

		// DEVICE
		//////////////////////////////////////////////////////////////////////////
		bool ret = insertDevice();
		if (ret)
		{
			insertHardware();
			// SNMP Location in DB eintragen
			std::string iString = "UPDATE device SET snmpLoc='" + standort + "' WHERE dev_id= " + dev_id + ";";
			dieDB->query(iString.c_str());

			// IP Phone Detail
			std::string ipphRow = "";
			iString = "INSERT INTO ipPhoneDet (voiceVlan, cm1, cm2, dscpSig, dscpConf, dscpCall, defGW)";
			iString += " VALUES('" + voiceVlan + "','" + cm1 + "','" + cm2 + "','" + dscpSig + "','" + dscpConf + "','" + dscpCall + "','" + defaultGateway + "');";
			dieDB->query(iString.c_str());

			std::string sString = "SELECT last_insert_rowid() FROM ipPhoneDet;";
			std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
			if (!result2.empty())
			{
				ipphRow = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0047 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}
			// ipphonelink Tabelle befüllen
			std::string iString2 = "INSERT INTO ipphonelink (ipPhoneDet_ipphone_id, device_dev_id) VALUES (" + ipphRow + "," + dev_id + ");";
			dieDB->query(iString2.c_str());

		}

		// INTERFACE
		//////////////////////////////////////////////////////////////////////////
		std::string iString = "INSERT INTO interfaces (intf_id,intfName,intfType,phl,macAddress,ipAddress,subnetMask,duplex,speed,status,description,l2l3,";
		iString += "errLvl,lastClear,loadLvl,channel_intf_id,iCrc,iFrame,iOverrun,iIgnored,iWatchdog,iPause,iDribbleCondition,ibuffer,l2decodeDrops,runts,giants,";
		iString += "throttles,ierrors,underrun,oerrors,oCollisions,oBabbles,oLateColl,oDeferred,oLostCarrier,oNoCarrier,oPauseOutput,oBufferSwapped,";
		iString += "resets,obuffer)";
		iString += " VALUES(NULL, 'Port 1','" + intfType + "',1,'" + macAdresse + "','" + ipa + "','"  + subnetMask + "','" + duplex + "','" + speed;
		iString += "','UP','','L2'," + boost::lexical_cast<std::string>(err) + ",'',0,NULL,'" + crcErr + "','','" + tokenDrop + "','','','','','','','";
		iString += shortErr + "','" + longErr + "','','','','','" + Collisions + "','','" + lateCollision + "','','','','','','','');";
		dieDB->query(iString.c_str());

		std::string sString = "SELECT last_insert_rowid() FROM interfaces;";
		std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
		intf_id = "";
		if (!result2.empty())
		{
			intf_id = result2[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0048 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// devInterface Tabelle befüllen
		std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
		dieDB->query(iString2.c_str());

		// ipSubnet befüllen
		if (ipa != "")
		{
			ipNetzeEintragen(ipa, subnetMask);
		}

		// CDP
		//////////////////////////////////////////////////////////////////////////
		if (cdpNeigh != "")
		{
			std::string nName = cdpNeigh.substr(0, cdpNeigh.find("."));
			std::string alternatenName = cdpNeigh;

			// CDP Tabelle befüllen
			iString = "INSERT INTO cdp (hostname, nName, alternatenName, intf, nIntfIP, nIntf, type, platform)";
			iString += " VALUES('" + hostname + "','" + nName + "','" + alternatenName + "','Port1','" + cdpNeighIP + "','" + cdpNeighPort + "',2,'');";
			dieDB->query(iString.c_str());

			sString = "SELECT last_insert_rowid() FROM cdp;";
			result2 = dieDB->query(sString.c_str());
			std::string cdpRow = "";
			if (!result2.empty())
			{
				cdpRow = result2[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0049 - Contact wktools@spoerr.org\n";
				dbgA += iString;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				fehlerString += "\n" + dateiname + dbgA;
			}
			// Und zum Schluss die cLink Tabelle befüllen
			iString = "INSERT INTO clink (interfaces_intf_id, cdp_cdp_id) VALUES (";
			iString += intf_id + "," + cdpRow + ");";
			dieDB->query(iString.c_str());

		}

		// Endgeräte INTERFACE -> Generisches Interface zum Anzeigen, ob Endgeräte angeschlossen ist
		//////////////////////////////////////////////////////////////////////////
		iString = "INSERT INTO interfaces (intf_id,intfName,intfType,phl,status,description,l2l3)";
		iString += " VALUES(NULL, 'Port 2','',1,'UP','PC Port','L2');";
		dieDB->query(iString.c_str());

		sString = "SELECT last_insert_rowid() FROM interfaces;";
		std::vector<std::vector<std::string> > result5 = dieDB->query(sString.c_str());
		intf_id = "";
		if (!result5.empty())
		{
			intf_id = result2[0][0];
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0048 - Contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}

		// devInterface Tabelle befüllen
		iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
		dieDB->query(iString2.c_str());

	}
	else
	{
		std::string errstring = "6501: Device not yet supported. Please send the file to wktools@spoerr.org " + dateiname;
		errstring += "\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, errstring, "6501", WkLog::WkLog_ROT);
	}
}


std::string WkmParserCisco::macAddChange(std::string macAdd)
{
	// Als erstes die Großbuchstaben in Kleinbuchstaben umwandeln
	boost::algorithm::to_lower(macAdd);

	// Dann die "." einfügen
	macAdd.insert(8, ".");
	macAdd.insert(4, ".");

	return macAdd;
}


void WkmParserCisco::testCatOS()
{
	zeitStempel("\ttestCatOS START");
	catOSHWParser();
}

void WkmParserCisco::catOSHWParser()
{
	zeitStempel("\tcatOSHWParser START");
	// CATOS - CATOS - CATOS - CATOS
	//////////////////////////////////////////////////////////////////////////

	// Vorgezogenes show system
	// Hostname
	aktPos1 = ganzeDatei.find("System Name", pos26);
	aktPos1 = ganzeDatei.find("--", aktPos1);
	aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);
	aktPos1 = ganzeDatei.find_first_not_of("\r\n", aktPos1);
	aktPos2 = ganzeDatei.find(" ", aktPos1);
	hostname = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	// SHOW VER
	//////////////////////////////////////////////////////////////////////////
	aktPos1 = ganzeDatei.find("NmpSW: ") + 7;
	aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
	version = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	aktPos1 = ganzeDatei.find("Hardware Version: ", aktPos2) + 18;
	aktPos2 = ganzeDatei.find(" ", aktPos1);
	hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	aktPos1 = ganzeDatei.find("Model: ") + 7;
	aktPos2 = ganzeDatei.find(" ", aktPos1);
	modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	aktPos1 = ganzeDatei.find("Serial #: ", aktPos2) + 10;
	aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
	chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

	hwPos = "box";
	hwDescription = "";

	// PS Erkennung nach hinten reihen; Daher die Pos2 merken;
	size_t tempPos = aktPos2;

	aktPos1 = ganzeDatei.find("DRAM", aktPos2);
	aktPos1 = ganzeDatei.find("---", aktPos1);
	aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);
	aktPos2 = ganzeDatei.find("Uptime", aktPos1);

	std::string speicherAlle = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
	std::string speicherZeile = "";
	std::string tempSpeicher = "";
	std::string temptemp = "";

	aktPos1 = -1;
	aktPos2 = 0;
	int off[] = {7, 7, 17, 7, 9, 7};
	boost::offset_separator f1(off, off + 6, false);
	for (; aktPos2 < speicherAlle.npos;)
	{
		aktPos2 = speicherAlle.find("\n", aktPos1 + 1);
		speicherZeile = speicherAlle.substr(aktPos1 + 1, aktPos2 - aktPos1);
		aktPos1 = aktPos2;
		if (speicherZeile.size() > 50)
		{
			int i = 0;
			boost::tokenizer<boost::offset_separator> tok(speicherZeile,f1);
			for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				switch (i)
				{
				case 1:
					temptemp = *beg;
					while (temptemp[0] == ' ')
					{
						temptemp.erase(0, 1);
					}

					hwMem = temptemp;
					break;
				case 3:
					temptemp = *beg;
					while (temptemp[0] == ' ')
					{
						temptemp.erase(0, 1);
					}

					tempSpeicher += temptemp;
					tempSpeicher += ";";
					break;
				case 4:
					temptemp = *beg;
					while (temptemp[0] == ' ')
					{
						temptemp.erase(0, 1);
					}

					tempSpeicher += temptemp;
					tempSpeicher += ";";
					hwFlash = "Flash:;" + tempSpeicher;
					break;
				default:
					break;
				}
				i++;
			}
		}
	}

	// Gerät einfügen
	hwBootfile = "";
	insertDevice();
	insertHardware();

	// SHOW MODULE
	//////////////////////////////////////////////////////////////////////////

	if (pos22 - pos21 > 300)
	{
		// Alle Informationen werden in einer Queue gesammlt und zum Schluss in die DB geschrieben; 
		// Vector wegen dem, da die Infos nicht in einem Schritt erfasst werden können
		std::queue<std::string> hRev;
		std::queue<std::string> hMod;
		std::queue<std::string> hDes;
		std::queue<std::string> hSer;
		std::queue<std::string> hPos;

		size_t modPos1 = ganzeDatei.find("Mod ", pos21);
		size_t modPos2 = ganzeDatei.find_first_of("\r\n", modPos1);
		std::string cat6k;
		std::string cat6kZeile;

		if (modPos2-modPos1 > 73)
		{
			// Steinalt CatOS
			modPos2 = ganzeDatei.find("Mod ", modPos2);
			modPos1 = ganzeDatei.rfind("---", modPos2)+4;

			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 4);
			cat6kZeile = "";
			int offsets[] = {2, 28, 22, 10, 9};
			boost::offset_separator f(offsets, offsets + 5, false);
			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find("\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;
				if (cat6kZeile.size() > 50)
				{
					int i = 0;
					boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f);
					for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 0:
							hPos.push(*beg);
							break;
						case 2:
							hDes.push(*beg);
							break;
						case 3:
							hMod.push(*beg);
							break;
						case 4:
							hSer.push(*beg);
						default:
							break;
						}
						i++;
					}
				}
			}

			modPos2 = ganzeDatei.find("Mod ", modPos2 + 4) - 2;
			modPos1 = ganzeDatei.rfind("---", modPos2)+4;
			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1);
			cat6kZeile = "";
			int offsets1[] = {43, 4};
			boost::offset_separator f1(offsets1, offsets1 + 2, false);
			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find("\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;
				if (cat6kZeile.size() > 50)
				{
					int i = 0;
					boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f1);
					for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 1:
							hRev.push(*beg);
							break;
						default:
							break;
						}
						i++;
					}
				}
			}
		}
		else
		{
			modPos2 = ganzeDatei.find("Mod ", modPos2);
			modPos1 = ganzeDatei.rfind("---", modPos2)+4;

			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 4);
			cat6kZeile = "";
			int offsets[] = {2, 13, 26, 19};
			boost::offset_separator f(offsets, offsets + 4, false);
			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find("\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;
				if (cat6kZeile.size() > 50)
				{
					int i = 0;
					boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f);
					for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 0:
							hPos.push(*beg);
							break;
						case 2:
							hDes.push(*beg);
							break;
						case 3:
							hMod.push(*beg);
							break;
						default:
							break;
						}
						i++;
					}
				}
			}
			modPos2 = ganzeDatei.find("Mod ", modPos2 + 4) - 2;
			modPos1 = ganzeDatei.rfind("---", modPos2)+4;
			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1);
			cat6kZeile = "";
			int offsets1[] = {25, 11};
			boost::offset_separator f1(offsets1, offsets1 + 2, false);
			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find("\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;
				if (cat6kZeile.size() > 35)
				{
					int i = 0;
					boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f1);
					for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 1:
							hSer.push(*beg);
							break;
						default:
							break;
						}
						i++;
					}
				}
			}

			modPos2 = ganzeDatei.find("Mod ", modPos2 + 4);
			if (modPos2 == ganzeDatei.npos)
			{
				modPos2 = pos3;			// show c7200
			}
			modPos2 -= 2;
			modPos1 = ganzeDatei.rfind("---", modPos2)+4;
			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1);
			cat6kZeile = "";
			int offsets3[] = {43, 4};
			boost::offset_separator f3(offsets3, offsets3 + 2, false);
			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find("\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;
				if (cat6kZeile.size() > 45)
				{
					int i = 0;
					boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f3);
					for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 1:
							hRev.push(*beg);
							break;
						default:
							break;
						}
						i++;
					}
				}
			}
		}
		if ((modPos2 = ganzeDatei.find("Mod Sub-Type", pos2)) < ganzeDatei.npos)
		{
			modPos1 = ganzeDatei.find("---", modPos2);
			modPos1 = ganzeDatei.find_first_of("\r\n", modPos1);
			modPos1 = ganzeDatei.find_first_not_of("\r\n", modPos1);
			modPos2 = ganzeDatei.rfind("\n", pos3);
			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2-modPos1);

			int offsets2[5];
			// Bei den Submodulen gibt es Unterschiede zwischen alten CatOS Versionen und neuen
			size_t lpos = cat6k.find_first_of("\r\n", aktPos1 + 1);
			if (lpos < 41)
			{
				offsets2[0] = 4;
				offsets2[1] = 9;
				offsets2[2] = 10;
				offsets2[3] = 11;
				offsets2[4] = 3;
			}
			else
			{
				offsets2[0] = 4;
				offsets2[1] = 24;
				offsets2[2] = 20;
				offsets2[3] = 12;
				offsets2[4] = 3;
			}
			cat6kZeile = "";
			boost::offset_separator f2(offsets2, offsets2 + 5, false);
			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find_first_of("\r\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1+1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;
				if (cat6kZeile.size() > 35)
				{
					int i = 0;
					boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f2);
					for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
					{
						switch (i)
						{
						case 0:
							{
								std::string mod = *beg + " (Submodule)";
								hPos.push(mod);
								break;
							}
						case 1:
							hDes.push(*beg);
							break;
						case 2:
							hMod.push(*beg);
							break;
						case 3:
							hSer.push(*beg);
							break;
						case 4:
							hRev.push(*beg);
						default:
							break;
						}
						i++;
					}
				}
			}
		}
		// Datenbank mit Modulinfos befüllen
		while (!hPos.empty())
		{
			hwMem = "";
			hwBootfile = "";
			version = "";

			hwDescription = hDes.front();
			hDes.pop();
			modell = hMod.front();
			hMod.pop();
			hwPos = hPos.front();
			hPos.pop();
			chassisSN = hSer.front();
			hSer.pop();
			hwRevision = hRev.front();
			hRev.pop();
			insertHardware();
		}
	}

	// SHOW SYSTEM
	//////////////////////////////////////////////////////////////////////////

	// Davor noch die PS Infos von show version
	// Power Supply; Variante 1
	hwRevision = "";
	std::string psSN[] = {"",""};
	aktPos1 = ganzeDatei.find("PS1  Module: ", tempPos);
	if (aktPos1 != ganzeDatei.npos)
	{
		aktPos1 = ganzeDatei.find("Serial #: ", aktPos1) + 10;
		aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
		psSN[0] = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		aktPos1 = ganzeDatei.find("Serial #: ", aktPos2) + 10;
		if (aktPos1-aktPos2 < 100)
		{
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			psSN[1] = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
	}


	if (pos26 != ganzeDatei.npos)
	{
		// Power Supply
		aktPos1 = ganzeDatei.find("PS1-Type", pos26);
		aktPos1 = ganzeDatei.find("---", aktPos1);
		aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);
		aktPos1 = ganzeDatei.find_first_not_of("\r\n", aktPos1);

		aktPos2 = ganzeDatei.find("  ", aktPos1);
		modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		if (modell.find("none") == modell.npos)
		{
			hwMem = "";
			hwBootfile = "";
			version = "";
			hwPos = "PS1";
			hwDescription = "";
			chassisSN = psSN[0];
			hwRevision = "";
			insertHardware();

		}

		aktPos1 = ganzeDatei.find_first_not_of(" ", aktPos2);
		aktPos2 = ganzeDatei.find("  ", aktPos1);
		modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		if (modell.find("none") == modell.npos)
		{
			hwMem = "";
			hwBootfile = "";
			version = "";
			hwPos = "PS2";
			hwDescription = "";
			chassisSN = psSN[1];
			hwRevision = "";
			insertHardware();
		}
	}
}


void WkmParserCisco::pixOSHWParser()
{
	zeitStempel("\tpixOSHWParser START");
	// show module
	//////////////////////////////////////////////////////////////////////////

	int revZaehler = 0;		// zum Zählen, ob genug HW Revisions ausgelesen wurden

	// Alle Informationen werden in einer Queue gesammlt und zum Schluss in die DB geschrieben; 
	// Queue wegen dem, da die Infos nicht in einem Schritt erfasst werden können
	std::queue<std::string> hRev;
	std::queue<std::string> hMod;
	std::queue<std::string> hDes;
	std::queue<std::string> hSer;
	std::queue<std::string> hPos;

	std::string asaMod = "";
	std::string asaModZeile = "";
	if (pos3-pos2 > 300)
	{
		size_t modpos1 = 0;
		size_t modpos2 = 0;

		modpos1 = ganzeDatei.find("Mod MAC Address", pos2);
		modpos2 = ganzeDatei.find("Mod", modpos1+10);

		modpos1 = ganzeDatei.rfind("---", modpos2) + 4;
		asaMod = ganzeDatei.substr(modpos1, modpos2-modpos1);
		aktPos1 = -1;
		aktPos2 = 0;
		int offsets3[] = {38, 4};
		boost::offset_separator f3(offsets3, offsets3 + 2, false);
		for (; aktPos2 < asaMod.npos;)
		{
			aktPos2 = asaMod.find("\n", aktPos1 + 1);
			asaModZeile = asaMod.substr(aktPos1 + 1, aktPos2 - aktPos1);
			aktPos1 = aktPos2;
			if (asaModZeile.size() > 45)
			{
				int i = 0;
				boost::tokenizer<boost::offset_separator> tok(asaModZeile,f3);
				for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 1:
						hRev.push(*beg);
						revZaehler++;
						break;
					default:
						break;
					}
					i++;
				}
			}
		}
	}
	else
	{
		hRev.push("");
	}


	// show invent
	//////////////////////////////////////////////////////////////////////////
	if(pos6 - pos5 > 300)
	{
		bool ed = true;		// erster Durchlauf wird ausgelassen, da im ersten Eitrag das Chassis steht
		aktPos2 = pos5;
		while (1)
		{
			if ((aktPos1 = ganzeDatei.find("Name", aktPos2)) != ganzeDatei.npos)
			{
				if (ed)
				{
					ed = false;
					aktPos2 = aktPos1 + 10;
					revZaehler--;
					continue;
				}
				aktPos1 += 7;
				aktPos2 = ganzeDatei.find("\"", aktPos1);
				std::string modNr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				hPos.push(modNr);

				aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
				aktPos2 = ganzeDatei.find("\"", aktPos1);
				hDes.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

				aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
				aktPos2 = ganzeDatei.find(" ", aktPos1);
				hMod.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

				aktPos1 = ganzeDatei.find("SN", aktPos2) + 4;
				aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
				hSer.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

				if (revZaehler < 1)
				{
					hRev.push("");
				}
				revZaehler--;
			}
			else
			{
				break;
			}
		}
	}

	// Datenbank mit Modulinfos befüllen
	while (!hPos.empty())
	{
		hwMem = "";
		hwBootfile = "";
		version = "";

		hwDescription = hDes.front();
		hDes.pop();
		modell = hMod.front();
		hMod.pop();
		hwPos = hPos.front();
		hPos.pop();
		chassisSN = hSer.front();
		hSer.pop();
		hwRevision = hRev.front();
		hRev.pop();
		insertHardware();
	}

	// show file system
	//////////////////////////////////////////////////////////////////////////
	//if (pos27 != ganzeDatei.npos)
	//{
	//	size_t flashPos1 = ganzeDatei.find("Prefixes", pos27);
	//	if (flashPos1 != ganzeDatei.npos)
	//	{
	//		size_t flashPos2 = ganzeDatei.rfind(":", ganzeDatei.npos) + 1;
	//		if ((flashPos2 - flashPos1) < 3000)
	//		{
	//			filesystem = true;
	//		}
	//	}
	//	else
	//	{
	//		flash = tempflash;
	//	}
	//}
	//else
	//{
	//	flash = tempflash;
	//}

}


void WkmParserCisco::nxosInventoryParser()
{
	zeitStempel("\tnxosInventoryParser START");

	// show invent
	//////////////////////////////////////////////////////////////////////////
	if(pos25 - pos24 > 300)
	{
		aktPos1 = aktPos2 = pos24;
		if (chassisSN == "")
		{
			if ((aktPos1 = ganzeDatei.find("hassis", pos24)) != ganzeDatei.npos)
			{
				aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
				aktPos2 = ganzeDatei.find("\"", aktPos1);
				hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
				aktPos2 = ganzeDatei.find(" ", aktPos1);
				modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = ganzeDatei.find("SN: ", aktPos1) + 4;
				aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
				chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				// UPDATE Device
				std::string iString = "UPDATE hwInfo SET sn='" + chassisSN + "' WHERE hwinf_id= " + hwInfo_id + ";";
				dieDB->query(iString.c_str());
				iString = "UPDATE hwInfo SET type='" + modell + "' WHERE hwinf_id= " + hwInfo_id + ";";
				dieDB->query(iString.c_str());
				iString = "UPDATE hwInfo SET description='" + hwDescription + "' WHERE hwinf_id=" + hwInfo_id + ";";
				dieDB->query(iString.c_str());

			}
		}

		bool ed = true;		// erster Durchlauf wird ausgelassen, da im ersten Eitrag das Chassis steht
		aktPos2 = pos5;
		while (1)
		{
			if ((aktPos1 = ganzeDatei.find("NAME", aktPos2)) != ganzeDatei.npos)
			{
				if (ed)
				{
					ed = false;
					aktPos2 = aktPos1 + 10;
					continue;
				}
				aktPos1 += 7;

				aktPos2 = ganzeDatei.find("\"", aktPos1);
				std::string modNr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
				hwPos = modNr;
				hwMem = "";

				aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
				aktPos2 = ganzeDatei.find("\"", aktPos1);
				size_t aktPos3 = ganzeDatei.rfind("Rev. ", aktPos2);
				if ((aktPos3 < aktPos1) || (aktPos3 == ganzeDatei.npos))
				{
					hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					hwRevision = "";
				}
				else
				{
					aktPos3 += 5;
					hwDescription = ganzeDatei.substr(aktPos1, aktPos3-aktPos1);
					hwRevision = ganzeDatei.substr(aktPos3, aktPos2-aktPos3);
				}



				aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
				aktPos2 = ganzeDatei.find(" ", aktPos1);
				modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = ganzeDatei.find("SN", aktPos2) + 4;
				aktPos2 = ganzeDatei.find_first_of(" \r\n", aktPos1);
				chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				hwBootfile = "";
				version = "";
				insertHardware();
			}
			else
			{
				break;
			}
		}
	}
}


void WkmParserCisco::iosModuleParser()
{
	zeitStempel("\tiosModuleParser START");

	// Herausfinden, ob Stack
	if ((aktPos1 = ganzeDatei.find("Model revision number", stackpos)) < pos1)
	{
		// Wenn 3750 Stack, dann Infos zu den anderen Switches auslesen und als Modulinfo abspeichern
		while ( (aktPos1 = ganzeDatei.find("Model revision number", aktPos2)) < pos1)
		{
			aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos1 = ganzeDatei.find("Model number", aktPos2);
			aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos1 = ganzeDatei.find("System serial number", aktPos2);
			aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
			aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
			chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			hwDescription = "";
			hwPos = "StackMember";
			hwBootfile = "";
			hwMem = "";
			version = "";
			insertHardware();
		}
	}

	if ((pos22-pos21 > 300) && (pos25 - pos24 < 300))   // keine "show invent" Infos
	{
		// Alle Informationen werden in einer Queue gesammlt und zum Schluss in die DB geschrieben; 
		// Queue wegen dem, da die Infos nicht in einem Schritt erfasst werden können
		std::queue<std::string> hRev;
		std::queue<std::string> hMod;
		std::queue<std::string> hDes;
		std::queue<std::string> hSer;
		std::queue<std::string> hPos;

		// Modulinfos excl HW Revision
		size_t modPos1 = ganzeDatei.find("Mod ", pos2);
		size_t modPos2 = ganzeDatei.find("MAC addresses", modPos1 + 4);
		modPos1 = ganzeDatei.rfind("---", modPos2)+4;

		aktPos1 = -1;
		aktPos2 = 0;
		std::string cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 5);
		std::string cat6kZeile = "";
		int offsets[] = {3, 7, 39, 19, 11};
		boost::offset_separator f(offsets, offsets + 5);
		for (; aktPos2 < cat6k.npos;)
		{
			aktPos2 = cat6k.find("\n", aktPos1 + 1);
			cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
			aktPos1 = aktPos2;

			if (cat6kZeile.length() < 60)
			{
				continue;
			}

			int i = 0;
			boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f);
			for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				switch (i)
				{
				case 0:
					hPos.push(*beg);
					break;
				case 2:
					hDes.push(*beg);
					break;
				case 3:
					hMod.push(*beg);
					break;
				case 4:
					hSer.push(*beg);
					break;
				default:
					break;
				}
				i++;
			}
		}

		// HW Revision der Hauptmodule
		bool cat4k = false;
		if ((ganzeDatei.find("-+---+------------+-", pos2) != ganzeDatei.npos))
		{
			cat4k = true;
		}
		modPos1 = ganzeDatei.find("MAC addresses", pos2);
		modPos2 = ganzeDatei.find("Sub-Module", modPos1 + 4);
		if (modPos2 == ganzeDatei.npos)
		{
			modPos2 = pos3;			// show c7200
		}
		if (modPos2 != ganzeDatei.npos)
		{
			modPos1 = ganzeDatei.rfind("---", modPos2)+4;

			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 5);
			cat6kZeile = "";
			int offsets1[2];
			if (cat4k)
			{
				offsets1[0] = 36;
				offsets1[1] = 3;
			}
			else
			{
				offsets1[0] = 40;
				offsets1[1] = 4;									
			}
			boost::offset_separator f1(offsets1, offsets1 + 2);

			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find("\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;

				if (cat6kZeile.length() < 40)
				{
					continue;
				}

				int i = 0;
				boost::tokenizer<boost::offset_separator> tok1(cat6kZeile,f1);
				for(boost::tokenizer<boost::offset_separator>::iterator beg1=tok1.begin(); beg1!=tok1.end();++beg1)
				{
					switch (i)
					{
					case 0:
						break;
					case 1:
						hRev.push(*beg1);
						break;
					default:
						break;
					}
					i++;
				}
			}

		}
		// Submodul Infos
		modPos1 = ganzeDatei.find("Sub-Module", pos2);
		if (modPos1 < ganzeDatei.npos)
		{
			modPos2 = ganzeDatei.find("Online Diag", modPos1 + 4);
			modPos1 = ganzeDatei.rfind("---", modPos2)+4;

			aktPos1 = -1;
			aktPos2 = 0;
			cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 5);
			cat6kZeile = "";
			int offsets2[] = {4, 28, 19, 14, 5};
			boost::offset_separator f2(offsets2, offsets2 + 5);
			for (; aktPos2 < cat6k.npos;)
			{
				aktPos2 = cat6k.find("\n", aktPos1 + 1);
				cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
				aktPos1 = aktPos2;

				if (cat6kZeile.length() < 50)
				{
					continue;
				}

				int i = 0;
				std::string temp = "";
				boost::tokenizer<boost::offset_separator> tok2(cat6kZeile,f2);
				for(boost::tokenizer<boost::offset_separator>::iterator beg2=tok2.begin(); beg2!=tok2.end();++beg2)
				{
					switch (i)
					{
					case 0:
						temp = "Submod. SL " + *beg2;
						hPos.push(temp);
						break;
					case 1:
						hDes.push(*beg2);
						break;
					case 2:
						hMod.push(*beg2);
						break;
					case 3:
						hSer.push(*beg2);
						break;
					case 4:
						hRev.push(*beg2);
						break;
					default:
						break;
					}
					i++;
				}
			}
		}
		// Datenbank mit Modulinfos befüllen
		while (!hPos.empty())
		{
			hwMem = "";
			hwBootfile = "";
			version = "";

			hwDescription = hDes.front();
			hDes.pop();
			modell = hMod.front();
			hMod.pop();
			hwPos = hPos.front();
			hPos.pop();
			chassisSN = hSer.front();
			hSer.pop();
			hwRevision = hRev.front();
			hRev.pop();
			insertHardware();
		}
	}
}


void WkmParserCisco::iosHWParser()
{
	zeitStempel("\tiosHWParser START");

	// SHOW C7200
	//////////////////////////////////////////////////////////////////////////
	if (pos23 - pos22 > 300)
	{
		c72er = true;

		aktPos1 = ganzeDatei.find("EEPROM", pos22);

		aktPos1 = ganzeDatei.find("Hardware revision", aktPos1) + 18;
		aktPos2 = ganzeDatei.find(" ", aktPos1);
		hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		aktPos2 = ganzeDatei.find("EEPROM", aktPos1+1);
		aktPos1 = ganzeDatei.find("Hardware revision", aktPos2);
		if (aktPos1 > pos4)
		{
			aktPos1 = ganzeDatei.find("Hardware Revision", aktPos2);
		}
		aktPos1 += 18;
		std::string iString = "UPDATE hwInfo SET hwRevision='" + hwRevision + "' WHERE hwinf_id IN (SELECT last_insert_rowid() FROM hwInfo);";
		dieDB->query(iString.c_str());


		aktPos2 = ganzeDatei.find(" ", aktPos1);
		hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		aktPos1 = ganzeDatei.find("Serial ", aktPos1);
		aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
		aktPos1 = ganzeDatei.rfind(" ", aktPos2-4);
		std::string npeSN = ganzeDatei.substr(aktPos1+1, aktPos2-aktPos1-1);
		chassisSN = npeSN;
		modell = prozessor;
		hwDescription = "";
		hwMem = "";
		hwBootfile = "";
		version = "";
		hwPos = "Processor";
	}

	// SHOW DIAG
	//////////////////////////////////////////////////////////////////////////
	// Überspringen, wenn show inventory verfügbar ist, 72er Router oder GSR

	if((pos25 - pos24 < 300) || c72er || gsr)
	{
		if ((pos24-pos23 > 300))
		{
			size_t slotpos = pos23;

			if (pos22-pos21 > 300)
			{
				// OSM Module bei 76er
				if ((slotpos = ganzeDatei.find("\nSlot", pos23)) < pos24)
				{
					while(slotpos < pos24)
					{
						aktPos1 = slotpos + 6;
						slotpos = ganzeDatei.find("\nSlot", aktPos1);
						osmNr.push(ganzeDatei.substr(aktPos1, 1));

						aktPos2 = ganzeDatei.find("MBytes Total", aktPos1) - 1;
						aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
						osmMem.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1) + "MB");
					}
				}
			}
			else if ((slotpos = ganzeDatei.find("Physical slot", pos4)) < pos5)
			{
				// 75er    -    75er    -    75er
				//////////////////////////////////////////////////////////////////////////
				size_t chassisPos = ganzeDatei.find("(virtual)", pos23);
				size_t processorSlotpos = ganzeDatei.find("Processor", pos23);
				while (slotpos < chassisPos)
				{
					aktPos1 = slotpos;

					if (processorSlotpos < slotpos)
					{
						aktPos1 = processorSlotpos;
						aktPos1 = ganzeDatei.rfind("Slot", aktPos1) + 5;
						aktPos2 = ganzeDatei.find(":", aktPos1);
						hwPos = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						aktPos2 = ganzeDatei.find(",", aktPos2);
						aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
						hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						modell = "";
						processorSlotpos = ganzeDatei.find("Processor", aktPos1);

						aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
						aktPos2 = ganzeDatei.find(",", aktPos1);
						hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = ganzeDatei.find("Serial", aktPos2) + 15;
						aktPos2 = ganzeDatei.find(" ", aktPos1);
						chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						hwMem = "";
						version = "";
						hwBootfile = "";
						insertHardware();

						aktPos1 = slotpos;
					}

					slotpos += 10;
					slotpos = ganzeDatei.find("Physical slot", slotpos);

					aktPos1 = ganzeDatei.rfind("Slot", aktPos1) + 5;
					aktPos2 = ganzeDatei.find(":", aktPos1);
					hwPos = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos2 = ganzeDatei.find("controller,", aktPos2) + 10;
					aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
					hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					modell = "";							

					aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
					aktPos2 = ganzeDatei.find(",", aktPos1);
					hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("Serial", aktPos2) + 15;
					aktPos2 = ganzeDatei.find(" ", aktPos1);
					chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("Memory Size:", aktPos2) + 13;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					hwMem = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);


					size_t bevorSN = aktPos2;
					aktPos1 = ganzeDatei.find("VIP Software", aktPos2);							
					aktPos1 = ganzeDatei.find("Version ", aktPos1) + 8;
					aktPos2 = ganzeDatei.find(",", aktPos1);
					version = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					hwBootfile = "";
					insertHardware();

					// PAs
					while((aktPos1 = ganzeDatei.find("PA Bay", bevorSN)) < slotpos)
					{
						hwPos = ganzeDatei.substr(aktPos1, 8); 

						hwMem = "";
						version = "";

						aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1) + 2;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
						hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						modell = "";

						aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
						aktPos2 = ganzeDatei.find(",", aktPos1);
						hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = ganzeDatei.find("Serial number: ", aktPos1) + 15;
						aktPos2 = ganzeDatei.find(" ", aktPos1);
						chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						bevorSN = aktPos2;
						hwBootfile = "";
						insertHardware();
					}

				}
				aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
				aktPos2 = ganzeDatei.find(",", aktPos1);
				hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = ganzeDatei.find("Serial number", chassisPos) + 15;
				aktPos2 = ganzeDatei.find(" ", aktPos1);
				chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

				// Update von der Chassis S/N
				std::string iString = "UPDATE hwInfo SET hwRevision='" + hwRevision + "' WHERE hwinf_id=" + hwInfo_id + ";";
				dieDB->query(iString.c_str());
				iString = "UPDATE hwInfo SET sn='" + chassisSN + "' WHERE hwinf_id=" + hwInfo_id + ";";
				dieDB->query(iString.c_str());

			}
			else if ((slotpos = ganzeDatei.find("FRU NUMBER", pos23)) < pos24)
			{
				// AS5300
				slotpos = ganzeDatei.find("Slot", pos23);
				while (slotpos < pos5)
				{
					aktPos1 = slotpos + 5;
					slotpos = ganzeDatei.find("Slot", aktPos1);
					hwPos = ganzeDatei.substr(aktPos1, 1);

					aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1) + 2;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("Hardware Version ", aktPos2) + 17;
					aktPos2 = ganzeDatei.find(",", aktPos1);
					hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("Serial ", aktPos2);
					aktPos2 = ganzeDatei.find(",", aktPos1);
					aktPos1 = ganzeDatei.rfind(" ", aktPos2) + 1;
					chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					if ((aktPos1 = ganzeDatei.find("FRU", aktPos2)) < slotpos)
					{
						aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
						aktPos2--;
						for (; ganzeDatei[aktPos2] == ' '; aktPos2--)
						{
						}
						aktPos1 = ganzeDatei.rfind(":", aktPos2-4) + 2;
						std::string bez = ganzeDatei.substr(aktPos1, aktPos2-aktPos1+1);
						if (bez == "UNKNOWN_BOARD_ID")
						{
							modell = "";
						}
						else
						{
							modell = "";
						}
					}
					else
					{
						modell = "";
						aktPos1 = aktPos2;
					}
					hwBootfile = "";
					version = "";
					hwMem = "";
					insertHardware();
				}
			}
			else if ((slotpos = ganzeDatei.find("Slot", pos23)) < pos24)
			{
				// Cisco Allgemein
				while (slotpos < pos24)
				{
					bool wic = false;
					size_t wicPos = ganzeDatei.find("WIC Slot", aktPos1);
					if (slotpos-wicPos < 10)
					{
						wic = true;
					}
					aktPos1 = slotpos + 5;
					slotpos = ganzeDatei.find("Slot", aktPos1);
					if (wic)
					{
						hwPos = "WIC " + ganzeDatei.substr(aktPos1, 1);
					}
					else
					{
						hwPos = ganzeDatei.substr(aktPos1, 1);
					}

					aktPos1 = ganzeDatei.find_first_of("\t ", aktPos1);
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					while (ganzeDatei[aktPos1+1] == ' ')
					{
						aktPos1++;
					}
					hwDescription = ganzeDatei.substr(aktPos1+1, aktPos2-aktPos1-1);

					if ((aktPos1 = ganzeDatei.find("Hardware Revision", aktPos2)) < slotpos)
					{
						aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
						aktPos2 = ganzeDatei.find_first_of("\r\n\t", aktPos1);
					}
					else if ((aktPos1 = ganzeDatei.find("Hardware revision", aktPos2)) < slotpos)
					{
						aktPos1 += 18;
						aktPos2 = ganzeDatei.find_first_of(" \t", aktPos1);
					}
					else
					{
						aktPos1 = aktPos2;
					}
					hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("Serial ", aktPos2);
					aktPos2 = ganzeDatei.find_first_of("(\t\r\n", aktPos1);
					size_t snPos = ganzeDatei.find("Part number", aktPos1);
					if(snPos < aktPos2)
					{
						aktPos2 = ganzeDatei.rfind(" ", snPos);
						while (ganzeDatei[aktPos2] == ' ')
						{
							aktPos2--;
						}
						aktPos2++;
					}

					aktPos1 = ganzeDatei.rfind(" ", aktPos2-4) + 1;
					chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					if ((aktPos1 = ganzeDatei.find("FRU", aktPos2)) < slotpos)
					{
						aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
						aktPos2--;
						for (; ganzeDatei[aktPos2] == ' '; aktPos2--)
						{
						}
						aktPos1 = ganzeDatei.rfind(" ", aktPos2-4) + 1;
						modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1+1);
					}
					else if ((aktPos1 = ganzeDatei.find("Product Number", aktPos2)) < slotpos)
					{
						aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
						aktPos2--;
						for (; ganzeDatei[aktPos2] == ' '; aktPos2--)
						{
						}
						aktPos1 = ganzeDatei.rfind(" ", aktPos2-4) + 1;
						modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1+1);
					}
					else
					{
						modell = "";
						aktPos1 = aktPos2;
					}
					hwBootfile = "";
					version = "";
					hwMem = "";
					insertHardware();
				}
			}
			else if ((slotpos = ganzeDatei.find("SLOT", pos4)) < pos5)
			{
				// GSR - GSR - GSR - GSR - GSR
				while (slotpos < pos5)
				{
					aktPos1 = slotpos + 5;
					slotpos = ganzeDatei.find("SLOT", aktPos1);
					hwPos = ganzeDatei.substr(aktPos1, 2);

					aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("rev", aktPos2) + 4;
					aktPos2 = ganzeDatei.find_first_of("\r\n\t", aktPos1);
					hwRevision = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("S/N", aktPos1) + 3;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					if ((aktPos1 = ganzeDatei.find("Linecard/Module: ", aktPos2)) < slotpos)
					{
						aktPos1 += 17;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					}
					else
					{
						modell = "";
						aktPos1 = aktPos2;
					}

					if ((aktPos1 = ganzeDatei.find("Route Memory: ", aktPos2)) < slotpos)
					{
						aktPos1 += 14;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						hwMem = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					}
					else
					{
						aktPos1 = aktPos2;
						hwMem = "";
					}

					aktPos1 = ganzeDatei.find("Packet Memory: ", aktPos2);
					if (aktPos1 < slotpos)
					{
						aktPos1 += 15;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						hwMem += " / " + ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					}
					else
					{
						aktPos1 = aktPos2;
						hwMem += " / ";
					}
				}
			}
		}
		else
		{
			nodiag = true;
		}
	}
	else
	{
		nodiag = true;
	}
}


void WkmParserCisco::iosInventoryParser()
{
	zeitStempel("\tiosInventoryParser START");

	// SHOW INVENT
	//////////////////////////////////////////////////////////////////////////
	if(pos25 - pos24 > 300)
	{
		aktPos1 = aktPos2 = pos24;
		if ((aktPos1 = ganzeDatei.find("hassis", pos24)) != ganzeDatei.npos)
		{
			aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
			aktPos2 = ganzeDatei.find("\"", aktPos1);
			hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
			aktPos2 = ganzeDatei.find(" ", aktPos1);
			modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			aktPos1 = ganzeDatei.find("SN: ", aktPos1) + 4;
			aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
			chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

			// UPDATE
			std::string iString = "UPDATE hwInfo SET description='" + hwDescription + "' WHERE hwinf_id=" + hwInfo_id + ";";
			dieDB->query(iString.c_str());
			iString = "UPDATE hwInfo SET type='" + modell + "' WHERE hwinf_id=" + hwInfo_id + ";";
			dieDB->query(iString.c_str());
			iString = "UPDATE hwInfo SET sn='" + chassisSN + "' WHERE hwinf_id=" + hwInfo_id + ";";
			dieDB->query(iString.c_str());

		}
		if(nodiag && !c72er)
		{
			bool ed = true;		// erster Durchlauf wird ausgelassen, da im ersten Eitrag das Chassis steht
			aktPos2 = pos5;
			while (1)
			{
				if ((aktPos1 = ganzeDatei.find("NAME", aktPos2)) != ganzeDatei.npos)
				{
					if (ed)
					{
						ed = false;
						aktPos2 = aktPos1 + 10;
						continue;
					}
					aktPos1 += 7;

					aktPos2 = ganzeDatei.find("\"", aktPos1);
					std::string modNr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					hwPos = modNr;
					if (!osmNr.empty())
					{
						if (modNr == osmNr.front())
						{
							hwMem = osmMem.front();
							osmMem.pop();
							osmNr.pop();
						}
						else
						{
							hwMem = "";
						}
					}
					else
					{
						hwMem = "";
					}

					aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
					aktPos2 = ganzeDatei.find("\"", aktPos1);
					size_t aktPos3 = ganzeDatei.rfind("Rev. ", aktPos2);
					if ((aktPos3 < aktPos1) || (aktPos3 == ganzeDatei.npos))
					{
						hwDescription = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						hwRevision = "";
					}
					else
					{
						aktPos3 += 5;
						hwDescription = ganzeDatei.substr(aktPos1, aktPos3-aktPos1);
						hwRevision = ganzeDatei.substr(aktPos3, aktPos2-aktPos3);
					}

					aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
					aktPos2 = ganzeDatei.find(" ", aktPos1);
					modell = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

					aktPos1 = ganzeDatei.find("SN", aktPos2) + 4;
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
					chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					hwBootfile = "";
					version = "";

					insertHardware();
				}
				else
				{
					break;
				}
			}
		}
	}
}


void WkmParserCisco::iosShowHardwareParser()
{
	zeitStempel("\tiosShowHardwareParser START");

	// SHOW HARDWARE
	//////////////////////////////////////////////////////////////////////////
	// LS1010
	//////////////////////////////////////////////////////////////////////////
	if(((ganzeDatei.find("EEPROM", pos25) != ganzeDatei.npos)) && (chassisSN == ""))
	{
		size_t modPos1 = ganzeDatei.find("Slot", pos25);
		size_t modPos2 = ganzeDatei.find("EEPROM", modPos1);
		modPos2 = ganzeDatei.rfind("\n", modPos2);
		modPos1 = ganzeDatei.rfind("---", modPos2) + 4;

		aktPos1 = 0;
		aktPos2 = 0;

		std::string modulTabelle = ganzeDatei.substr(modPos1, modPos2-modPos1-4);
		std::string modulZeile = "";
		int offsets[] = {5, 14, 16, 8, 22, 4};
		boost::offset_separator f(offsets, offsets+6, false);
		for (; aktPos2 < modulTabelle.npos;)
		{
			aktPos2 = modulTabelle.find("\n", aktPos1 + 1);
			modulZeile = modulTabelle.substr(aktPos1 + 1, aktPos2 - aktPos1);

			if (modulZeile.size() < 10)
			{
				aktPos1++;
				continue;
			}

			aktPos1 = aktPos2;

			int i = 0;
			boost::tokenizer<boost::offset_separator> tok(modulZeile, f);

			for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				switch (i)
				{
				case 0:
					hwPos = *beg;
					break;
				case 1:
					hwDescription = *beg;
					break;
				case 3:
					chassisSN = *beg;
					break;
				case 5:
					hwRevision = *beg;
					break;
				default:
					break;
				}
				i++;
			}
			modell = "";
			hwMem = "";
			hwBootfile = "";
			version = "";
		}

		aktPos1 = ganzeDatei.find("--\n", modPos2) + 16;
		chassisSN = ganzeDatei.substr(aktPos1, 8);
	}
}

void WkmParserCisco::iosFileSystemParser()
{
	zeitStempel("\tiosFileSystemParser START");
	size_t flashPos1 = ganzeDatei.find("Prefixes", pos27);
	if (flashPos1 != ganzeDatei.npos)
	{
		flashPos1 = ganzeDatei.find(" ", flashPos1);

		size_t flashPos2 = ganzeDatei.rfind(":", ganzeDatei.npos) + 1;

		size_t prefPos1 = 0;
		size_t prefPos2 = 0;
		std::string fsize = "";
		std::string ffree = "";
		std::string fname = "";
		bool naechsteZeile = false;

		aktPos1 = -1;
		aktPos2 = 0;
		std::string speicher = ganzeDatei.substr(flashPos1, flashPos2 - flashPos1);
		std::string speicherZeile = "";
		for (; aktPos2 < speicher.npos;)
		{
			aktPos2 = speicher.find_first_of("\r\n", aktPos1 + 1);
			speicherZeile = speicher.substr(aktPos1 + 1, aktPos2 - aktPos1);
			aktPos1 = aktPos2;
			bool schreibeInfo = false;
			if (speicherZeile.size() > 45)
			{
				int i = 0;
				boost::char_separator<char> sep(" \t");
				boost::tokenizer<boost::char_separator<char>> tok(speicherZeile, sep);
				for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 0:			// Size
						fsize = *beg;
						if ((fsize[0] == '*'))
						{
							continue;
						}
						break;
					case 1:
						ffree = *beg;
						break;
					case 2:
						if (*beg == "flash")
						{
							schreibeInfo = true;
						}
						else if (*beg == "disk")
						{
							schreibeInfo = true;
						}
						else
						{
							schreibeInfo = false;
						}
						break;
					case 4:
						fname = *beg;
						break;
					default:
						break;
					}
					i++;
				}

				if (schreibeInfo)
				{
					// Flash Tabelle befüllen
					std::string iString = "INSERT INTO flash (flash_id,name,size,free) ";
					iString += "VALUES(NULL, '" + fname + "', '" + fsize + "', '" + ffree + "');";
					dieDB->query(iString.c_str());

					std::string flash_id = "";
					std::string sString = "SELECT last_insert_rowid() FROM flash;";
					std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
					if (result.empty())
					{
						std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					}
					else
					{
						flash_id = result[0][0];
						// hwLink befüllen
						std::string iString2 = "INSERT INTO flashlink (flash_flash_id, device_dev_id) VALUES (" + flash_id + "," + dev_id + ");";
						dieDB->query(iString2.c_str());
					}
				}
			}
		}
	}
}


void WkmParserCisco::pixOSFilesystemParser()
{
	zeitStempel("\tpixOSFilesystemParser START");
	iosFileSystemParser();
}


void WkmParserCisco::switchLicParser()
{
	zeitStempel("\tswitchLicParser START");
	licStatus = "";
	licName = "";
	licNextBoot = "";
	licCluster = "0";
	licType = "";
	if ((aktPos1 = ganzeDatei.find("License Level: ")) < pos2)
	{
		aktPos1 = aktPos1 + 15;
		aktPos2 = ganzeDatei.find_first_of("\r\n \t", aktPos1);
		licName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		aktPos1 = ganzeDatei.find("Type: ", aktPos2);
		if (aktPos1 != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("\r\n \t", aktPos1);
			licType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		aktPos1 = ganzeDatei.find("Next re", aktPos2);
		if (aktPos1 != ganzeDatei.npos)
		{
			aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
			aktPos2 = ganzeDatei.find_first_of("\r\n \t", aktPos1);
			licNextBoot = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		insertLic();
	}
}


void WkmParserCisco::routerLicParser()
{
	zeitStempel("\trouterLicParser START");

	licStatus = "";
	licName = "";
	licNextBoot = "";
	licCluster = "0";
	licType = "";
	// Variante 1
	if ((aktPos1 = ganzeDatei.find("License Level: ")) < pos2)
	{
		aktPos1 = aktPos1 + 15;
		aktPos2 = ganzeDatei.find_first_of("\r\n \t", aktPos1);
		licName = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		aktPos1 = ganzeDatei.find("Type: ", aktPos2);
		if (aktPos1 != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("\r\n \t", aktPos1);
			licType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		aktPos1 = ganzeDatei.find("Next re", aktPos2);
		if (aktPos1 != ganzeDatei.npos)
		{
			aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
			aktPos2 = ganzeDatei.find_first_of("\r\n \t", aktPos1);
			licNextBoot = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}

		insertLic();
	}
	else if ((aktPos1 = ganzeDatei.find("Technology Package License Information")) < pos2)
	{
		aktPos1 = ganzeDatei.find("Next re", aktPos1) + 25;
		aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);
		aktPos2 = ganzeDatei.find("Configuration register", aktPos1);

		std::string licString = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
		std::string licZeile = "";

		aktPos1 = 0;
		aktPos2 = 0;
		int off[] = {14,14,14,14};
		boost::offset_separator f1(off, off + 4, false, true);
		for (; aktPos2 < licString.npos;)
		{
			aktPos2 = licString.find("\n", aktPos1 + 1);
			licZeile = licString.substr(aktPos1 + 1, aktPos2 - aktPos1);
			aktPos1 = aktPos2;
			if (licZeile.size() > 35)
			{
				int i = 0;
				boost::tokenizer<boost::offset_separator> tok(licZeile,f1);
				for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 0:		// Name
						licName = *beg;
						licName = licName.substr(0, licName.find(" "));
						break;
					case 1:		// Status
						licStatus = *beg;
						licStatus = licStatus.substr(0, licStatus.find(" "));
						break;
					case 2:		// Type
						licType = *beg;
						licType = licType.substr(0, licType.find(" "));
						break;
					case 3:		// Next Reboot
						licNextBoot = *beg;
						licNextBoot = licNextBoot.substr(0, licNextBoot.find(" "));
						break;
					default:
						break;
					}
					i++;
				}
				insertLic();
			}
		}
	}
}


void WkmParserCisco::pixOSLicParser()
{
	zeitStempel("\tpixOSLicParser START");
	aktPos1 = ganzeDatei.find("Licensed features for this platform:");
	if (aktPos1 < pos2)
	{
		aktPos1 += 37;
		aktPos2 = ganzeDatei.find("This platform has an", aktPos1);
		std::string licString = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		std::string licZeile = "";
		int offsets[] = {34, 2, 15, 30};
		boost::offset_separator f(offsets, offsets+4, false);
		aktPos1 = aktPos2 = 0;
		for (; aktPos2 < licString.npos;)
		{
			aktPos2 = licString.find_first_of("\r\n", aktPos1);
			licZeile = licString.substr(aktPos1, aktPos2 - aktPos1);
			aktPos1 = aktPos2+1;
			bool schreibeInfo = false;
			licName = "";
			licStatus = "";
			licType = "";
			licCluster = "0";
			licNextBoot = "";
			if (licZeile.size() > 45)
			{
				int i = 0;
				boost::tokenizer<boost::offset_separator> tok(licZeile, f);
				for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 0:			// Name
						licName = *beg;
						licName = licName.substr(0, licName.find(" "));
						break;
					case 1:			// Nix
						break;
					case 2:			// Anzahl/Art
						licStatus = *beg;
						licStatus = licStatus.substr(0, licStatus.find(" "));
						break;
					case 3:			// Info
						licType = *beg;
						licType = licType.substr(0, licType.find(" "));
						break;
					default:
						break;
					}
					i++;
				}
				insertLic();
			}
		}
	}

	// Cluster Lizenzen
	aktPos1 = ganzeDatei.find("Failover cluster licensed features for this platform:");
	if (aktPos1 != ganzeDatei.npos)
	{
		aktPos1 += 54;
		aktPos2 = ganzeDatei.find("This platform has a", aktPos1);
		std::string licString = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

		std::string licZeile = "";
		int offsets[] = {34, 2, 15, 30};
		boost::offset_separator f(offsets, offsets+4, false);
		aktPos1 = aktPos2 = 0;
		for (; aktPos2 < licString.npos;)
		{
			aktPos2 = licString.find_first_of("\r\n", aktPos1);
			licZeile = licString.substr(aktPos1, aktPos2 - aktPos1);
			aktPos1 = aktPos2+1;
			bool schreibeInfo = false;
			licName = "";
			licStatus = "";
			licType = "";
			licCluster = "1";
			licNextBoot = "";
			if (licZeile.size() > 45)
			{
				int i = 0;
				boost::tokenizer<boost::offset_separator> tok(licZeile, f);
				for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 0:			// Name
						licName = *beg;
						licName = licName.substr(0, licName.find(" "));
						break;
					case 1:			// Nix
						break;
					case 2:			// Anzahl/Art
						licStatus = *beg;
						licStatus = licStatus.substr(0, licStatus.find(" "));
						break;
					case 3:			// Info
						licType = *beg;
						licType = licType.substr(0, licType.find(" "));
						break;
					default:
						break;
					}
					i++;
				}
				insertLic();
			}
		}
	}
}


void WkmParserCisco::markNeighbor()
{
	zeitStempel("\tmarkNeighbor START");

	std::string sString = "UPDATE neighbor SET self=1 WHERE neighbor_id IN (";
	sString += "SELECT DISTINCT neighbor_id FROM neighbor INNER JOIN nlink ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "WHERE l3_addr IN (SELECT ipAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "WHERE devInterface.device_dev_id=" + dev_id + ") AND device_dev_id=" + dev_id + ")";
	dieDB->query(sString.c_str());
}


void WkmParserCisco::iosPoeParser()
{
	zeitStempel("\tiosPoeParser START");
	size_t poePos1 = ganzeDatei.find("Power", pos28);
	if (poePos1 < pos29)
	{
		poePos1 = ganzeDatei.find("Watts", poePos1);
		poePos1 = ganzeDatei.find("----", poePos1);
		if (poePos1 != ganzeDatei.npos)
		{
			bool cat6k = false;
			poePos1 = ganzeDatei.find_first_of("\r\n", poePos1);

			size_t poePos2 = ganzeDatei.find("Totals: ", poePos1);
			if (poePos2 != ganzeDatei.npos)
			{
				cat6k = true;
			}
			else
			{
				poePos2 = ganzeDatei.rfind("\n", pos29);
			}

			size_t prefPos1 = 0;
			size_t prefPos2 = 0;
			std::string intf = "";
			std::string status = "";
			std::string watt = "";
			std::string wattMax = "";
			std::string device = "";
			bool naechsteZeile = false;

			if (cat6k)
			{
				// Gerätebezogene Infos:
				std::size_t poePos3 = ganzeDatei.find_first_of("\r\n", poePos2);
				std::string totals = ganzeDatei.substr(poePos2, poePos3-poePos2);
				aktPos1 = totals.find_first_not_of(" ", 25);
				aktPos2 = totals.find(" ", aktPos1);
				std::string maxPoe = totals.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = totals.find_first_not_of(" ", aktPos2);
				aktPos2 = totals.find_first_of(" \r\n", aktPos1);
				std::string currentPoe = totals.substr(aktPos1, aktPos2-aktPos1);

				std::string uString = "UPDATE device SET poeStatus='ON', poeCurrent='" + currentPoe + "', poeMax='" + maxPoe + "', poeRemaining='" + "' WHERE dev_id=" + dev_id;
				dieDB->query(uString.c_str());

				aktPos1 = -1;
				aktPos2 = 0;
				int offsets[] = {10, 7, 11, 11, 11, 20, 10, 30};
				boost::offset_separator f(offsets, offsets+8, false);
				std::string poeString = ganzeDatei.substr(poePos1, poePos2 - poePos1);
				std::string poeZeile = "";
				for (; aktPos2 < poeString.npos;)
				{
					aktPos2 = poeString.find_first_of("\r\n", aktPos1 + 1);
					poeZeile = poeString.substr(aktPos1 + 1, aktPos2 - aktPos1);
					aktPos1 = aktPos2;
					bool schreibeInfo = false;
					if (poeZeile.size() > 45)
					{
						int i = 0;
						boost::tokenizer<boost::offset_separator> tok(poeZeile, f);
						for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
						{
							switch (i)
							{
							case 0:			// Size
								intf = *beg;
								boost::algorithm::trim(intf);
								intfNameChange(intf);
								break;
							case 2:
								status = *beg;
								boost::algorithm::trim(status);
								break;
							case 3:
								wattMax = *beg;
								boost::algorithm::trim(wattMax);
								break;
							case 4:
								watt = *beg;
								boost::algorithm::trim(watt);
								break;
							case 5:
								device = *beg;
								boost::algorithm::trim(device);
								break;
							default:
								break;
							}
							i++;
						}
						if (intf[0] != '-' && intf[0] != ' ')
						{
							// Interface updaten
							std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
							sString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
							std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

							std::string intfID = "";
							if (!result.empty())
							{
								intfID = result[0][0];
								std::string iString = "UPDATE interfaces SET poeStatus='" + status + "',poeWatt='" + watt + "',poeWattmax='" + wattMax + "',poeDevice='" + device + "' WHERE intf_id = " + intfID + ";";
								dieDB->query(iString.c_str());
							}
							else
							{
								std::string dbgA = "\n6305: Parser Error! Code 0046 - Contact wktools@spoerr.org\n";
								dbgA += sString;
								schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
									WkLog::WkLog_ROT, WkLog::WkLog_FETT);
								fehlerString += "\n" + dateiname + dbgA;
							}
						}
					}
				}
			}
			else
			{
				// Gerätebezogene Infos:
				bool poeAlt = true;
				std::size_t poePos3 = ganzeDatei.find("Available", pos28);
				if (poePos3 < pos29)
				{
					poeAlt = false;
				}

				if (!poeAlt)
				{
					std::string remainPoe = "";
					std::string currentPoe = "";
					std::string totals = "";
					std::string maxPoe = "";

					if (ganzeDatei.find("Used:", poePos3) < pos29)
					{
						// Cat4k; Cat29,37,35 teilweise
						std::size_t poePos4 = ganzeDatei.find_first_of("\r\n", poePos3);
						totals = ganzeDatei.substr(poePos3, poePos4-poePos3);

						aktPos1 = totals.find("Available:") + 10;
						aktPos2 = totals.find("(", aktPos1);
						maxPoe = totals.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = totals.find("Used:") + 5;
						aktPos2 = totals.find("(", aktPos1);
						currentPoe = totals.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = totals.find("Remaining:", aktPos2) + 10;
						aktPos2 = totals.find("(", aktPos1);
						remainPoe = totals.substr(aktPos1, aktPos2-aktPos1);
					}
					else
					{
						// Cat29,35,37
						float mPoe = 0;
						float cPoe = 0;
						float rPoe = 0;

						// Ende der Box-spezifischen Infos und Beginn der Interface PoE Infos
						size_t tpos1 = ganzeDatei.find("Interface", poePos3);

						poePos3 = ganzeDatei.find("---", poePos3);
						poePos3 = ganzeDatei.find_first_of("\r\n", poePos3) + 1;

						// Check ob PoE Daten vorhanden
						size_t tpos2 = ganzeDatei.find(".", poePos3);
						if (tpos2 < pos29)
						{
							std::string boxPoe = ganzeDatei.substr(poePos3, tpos1-poePos3);

							aktPos2 = aktPos1 = 0;
							std::string poeZeile = "";

							while (aktPos2 != boxPoe.npos)
							{
								aktPos2 = boxPoe.find_first_of("\r\n", aktPos1 + 1);
								poeZeile = boxPoe.substr(aktPos1 + 1, aktPos2 - aktPos1);
								aktPos1 = aktPos2;
								bool schreibeInfo = false;
								if (poeZeile.size() > 25)
								{
									int i = 0;
									boost::char_separator<char> sep(" \t\r\n");
									boost::tokenizer<boost::char_separator<char>> tok(poeZeile, sep);
									for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
									{
										switch (i)
										{
										case 1:
											maxPoe = *beg;
											try
											{
												mPoe += boost::lexical_cast<float>(maxPoe);
											}
											catch (boost::bad_lexical_cast &)
											{
												mPoe += 0;		
											}
											break;
										case 2:
											currentPoe = *beg;
											try
											{
												cPoe += boost::lexical_cast<float>(currentPoe);
											}
											catch (boost::bad_lexical_cast &)
											{
												cPoe += 0;		
											}
											break;
										case 3:
											remainPoe = *beg;
											currentPoe = *beg;
											try
											{
												rPoe += boost::lexical_cast<float>(remainPoe);
											}
											catch (boost::bad_lexical_cast &)
											{
												rPoe += 0;		
											}
											break;
										default:
											break;
										}
										i++;
									}
								}
							}
						}
						maxPoe = boost::lexical_cast<std::string>(mPoe);
						currentPoe = boost::lexical_cast<std::string>(cPoe);
						remainPoe = boost::lexical_cast<std::string>(rPoe);
						std::string uString = "UPDATE device SET poeStatus='ON', poeCurrent='" + currentPoe + "', poeRemaining='" + remainPoe + "', poeMax='"  + maxPoe + "' WHERE dev_id=" + dev_id;
						dieDB->query(uString.c_str());
					}
				}	

				aktPos1 = -1;
				aktPos2 = 0;
				int offsets[] = {10, 7, 11, 8, 20, 6, 5};
				if (poeAlt)
				{
					offsets[0] = 11;
					offsets[1] = 6;
				}
				boost::offset_separator f(offsets, offsets+7, false);
				std::string poeString = ganzeDatei.substr(poePos1, poePos2 - poePos1);
				std::string poeZeile = "";
				for (; aktPos2 < poeString.npos;)
				{
					aktPos2 = poeString.find_first_of("\r\n", aktPos1 + 1);
					poeZeile = poeString.substr(aktPos1 + 1, aktPos2 - aktPos1);
					aktPos1 = aktPos2;
					bool schreibeInfo = false;
					if (poeZeile.size() > 45)
					{
						int i = 0;
						boost::tokenizer<boost::offset_separator> tok(poeZeile, f);
						for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
						{
							wattMax = "";
							switch (i)
							{
							case 0:			// Size
								intf = *beg;
								boost::algorithm::trim(intf);
								intfNameChange(intf);
								break;
							case 2:
								status = *beg;
								boost::algorithm::trim(status);
								break;
							case 3:
								watt = *beg;
								boost::algorithm::trim(watt);
								break;
							case 4:
								device = *beg;
								boost::algorithm::trim(device);
								break;
							case 6:
								wattMax = *beg;
								boost::algorithm::trim(wattMax);
								break;
							default:
								break;
							}
							i++;
						}
						// Interface updaten
						std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
						sString += intf + "' AND devInterface.device_dev_id=" + dev_id +";";
						std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

						std::string intfID = "";
						if (!result.empty())
						{
							intfID = result[0][0];
							std::string iString = "UPDATE interfaces SET poeStatus='" + status + "',poeWatt='" + watt + "',poeWattmax='" + wattMax + "',poeDevice='" + device + "' WHERE intf_id = " + intfID + ";";
							dieDB->query(iString.c_str());
						}
						else
						{
							std::string dbgA = "\n6305: Parser Error! Code 0046 - Contact wktools@spoerr.org\n";
							dbgA += sString;
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
								WkLog::WkLog_ROT, WkLog::WkLog_FETT);
							fehlerString += "\n" + dateiname + dbgA;
						}
					}
				}
			}
		}
	}
}


std::string WkmParserCisco::fehlerRet()
{
	return fehlerString;
}


void WkmParserCisco::iosVrfParser()
{
	zeitStempel("\tiosVrfParser START");

	//aktPos1 = aktPos2 = 0;
	//size_t vrfPos = ganzeDatei.find("VRF ", pos31);
	//while (vrfPos != ganzeDatei.npos)
	//{
	//	aktPos1 += vrfPos + 4;
	//	vrfPos = ganzeDatei.find("VRF ", vrfPos);

	//	std::string intName = "";
	//	std::string vrfName = "";










	//	// Wenn nein, dann das neue vrf eintragen
	//	// vrf Tabelle befüllen
	//	std::string iString = "INSERT INTO vrf (vrf_id,vrfName) ";
	//	iString += "VALUES(NULL, '" + vrfName + "');";
	//	dieDB->query(iString.c_str());
	//	sString = "SELECT last_insert_rowid() FROM vrf;";
	//	result = dieDB->query(sString.c_str());
	//	if (result.empty())
	//	{
	//		std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
	//		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
	//			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	//		continue;
	//	}
	//	else
	//	{
	//		vrf_id = result[0][0];
	//	}

	//	// Intf ID rausfinden
	//	// und dann vrflink Tabelle befüllen
	//	if (intName != "")
	//	{
	//		std::string sString = "SELECT intf_id FROM interfaces ";
	//		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	//		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	//		sString += "WHERE device.dev_id=" + dev_id + " AND interfaces.intfName LIKE '" + intName + "'";
	//		std::vector<std::vector<std::string> > r3 = dieDB->query(sString.c_str());
	//		if (!r3.empty())
	//		{
	//			std::string iString = "INSERT INTO vrflink (interfaces_intf_id, vrf_vrf_id) VALUES (" + r3[0][0] + "," + vrf_id + ");";
	//			dieDB->query(iString.c_str());
	//		}
	//		else
	//		{
	//			std::string dbgA = "\n6305: Parser Error! Code 0054 - Contact wktools@spoerr.org\n";
	//			dbgA += sString;
	//			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
	//				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	//			fehlerString += "\n" + dateiname + dbgA;
	//		}
	//	}
	//}
}


void WkmParserCisco::nxosVrfParser()
{
	zeitStempel("\tnxosVrfParser START");

	size_t vrfPos = ganzeDatei.find("VRF-Name", pos30);
	if (vrfPos < pos31)
	{
		vrfPos = ganzeDatei.find_first_of("\r\n", vrfPos);

		std::string intName = "";
		std::string vrfName = "";

		aktPos1 = -1;
		aktPos2 = 0;
		std::string vrfs = ganzeDatei.substr(vrfPos, pos31 - vrfPos);
		std::string vrfZeile = "";
		for (; aktPos2 < vrfs.npos;)
		{
			aktPos2 = vrfs.find_first_of("\r\n", aktPos1 + 1);
			vrfZeile = vrfs.substr(aktPos1 + 1, aktPos2 - aktPos1);
			aktPos1 = aktPos2;
			if (vrfZeile.size() > 50)
			{
				int i = 0;
				boost::char_separator<char> sep(" \t");
				boost::tokenizer<boost::char_separator<char>> tok(vrfZeile, sep);
				for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
				{
					switch (i)
					{
					case 0:			// Interface Name
						intName = *beg;
						intName = intfNameChange(intName);
						break;
					case 1:
						vrfName = *beg;
						break;
					default:
						break;
					}
					i++;
				}

				std::string vrf_id = "";
				// Check ob es das vrf schon gibt
				std::string sString = "SELECT vrf_id FROM vrf WHERE vrfName LIKE '" + vrfName + "'";
				std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());				
				if (result.empty())
				{
					// Wenn nein, dann das neue vrf eintragen
					// vrf Tabelle befüllen
					std::string iString = "INSERT INTO vrf (vrf_id,vrfName) ";
					iString += "VALUES(NULL, '" + vrfName + "');";
					dieDB->query(iString.c_str());
					sString = "SELECT last_insert_rowid() FROM vrf;";
					result = dieDB->query(sString.c_str());
					if (result.empty())
					{
						std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						continue;
					}
					else
					{
						vrf_id = result[0][0];
					}
				}
				else
				{
					vrf_id = result[0][0];
				}
				// Intf ID rausfinden
				// und dann vrflink Tabelle befüllen
				if (intName != "")
				{
					std::string sString = "SELECT intf_id FROM interfaces ";
					sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
					sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
					sString += "WHERE device.dev_id=" + dev_id + " AND interfaces.intfName LIKE '" + intName + "'";
					std::vector<std::vector<std::string> > r3 = dieDB->query(sString.c_str());
					if (!r3.empty())
					{
						std::string iString = "INSERT INTO vrflink (interfaces_intf_id, vrf_vrf_id) VALUES (" + r3[0][0] + "," + vrf_id + ");";
						dieDB->query(iString.c_str());
					}
					else
					{
						std::string dbgA = "\n6305: Parser Error! Code 0050 - Contact wktools@spoerr.org\n";
						dbgA += sString;
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehlerString += "\n" + dateiname + dbgA;
					}

				}
				else
				{
					continue;
				}
			}
		}
	}
}


void WkmParserCisco::nxosRouteParser()
{
	zeitStempel("\tnxosRouteParser START");

	size_t routePos = ganzeDatei.find("IP Route Table for VRF \"", pos29);
	bool rEnde = false;			// zeigt an wenn alle Routing Tabellen erfasst wurden

	while (!rEnde)
	{
		std::size_t vrfPos1 = routePos + 24;
		std::size_t vrfPos2 = ganzeDatei.find("\"", vrfPos1+1);
		std::string vrfName = ganzeDatei.substr(vrfPos1, vrfPos2-vrfPos1);


		std::size_t routePos2 = ganzeDatei.find("]", vrfPos2);
		routePos2 = ganzeDatei.find_first_of("\r\n", routePos2);
		routePos2 = ganzeDatei.find_first_not_of("\r\n\t ", routePos2);
		routePos = ganzeDatei.find("IP Route Table for VRF \"", vrfPos2);
		if (routePos == ganzeDatei.npos)
		{
			rEnde = true;
			routePos = ganzeDatei.rfind("\n", pos30);
		}

		// Falls Routing Tabelle für das entsprechende vrf leer, dann weiterspringen
		if (routePos2 >= routePos)
		{
			continue;
		}

		std::string routes = ganzeDatei.substr(routePos2, routePos-routePos2);
		std::string rZeile = "";

		// Beispiel für einen NX OS Routing Eintrag; in dem Fall mit zwei gleichwertigen Routen
		// 0.0.0.0/0, ubest/mbest: 1/0
		//     *via 10.143.255.1, Vlan3, [1/0], 1w4d, static
		//     *via 10.143.254.32, Null0, [1/0], 00:00:05, static
		std::string ip = "";
		std::string prefix = "";
		std::string nh = "";
		std::string intf = "";
		std::string prot = "";

		std::size_t viaPos = 0;
		std::size_t rPos = routes.find("ubest/mbest", viaPos);
		viaPos = routes.find("*via", rPos);
		while (viaPos != routes.npos)
		{
			if (viaPos < rPos)
			{
				// Next Hop parsen und DB Update
				aktPos1 = routes.find(" ", viaPos) + 1;
				aktPos2 = routes.find(",", aktPos1);
				nh = routes.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = aktPos2+2;
				aktPos2 = routes.find(",", aktPos1);
				intf = routes.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = routes.find(",", aktPos2+1);
				aktPos1 = routes.find(",", aktPos1+1) + 2;
				aktPos2 = routes.find_first_of("\r\n", aktPos1);
				prot = routes.substr(aktPos1, aktPos2-aktPos1);

				viaPos = routes.find("*via", viaPos+4);

				std::string iString = "INSERT INTO l3routes (l3r_id,protocol,network,subnet,nextHop,interface,type)";
				iString += " VALUES(NULL, '" + prot + "','" + ip + "','" + prefix + "','" + nh + "','" + intf + "',NULL);";
				dieDB->query(iString.c_str());

				std::string sString = "SELECT last_insert_rowid() FROM l3routes;";
				std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
				std::string l3r_id = "";
				if (!result2.empty())
				{
					l3r_id = result2[0][0];
				}
				else
				{
					std::string dbgA = "\n6305: Parser Error! Code 0051 - Contact wktools@spoerr.org\n";
					dbgA += iString;
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehlerString += "\n" + dateiname + dbgA;
				}

				// rlink Tabelle befüllen
				// Schritt 1: Next Hop Interface auslesen; Wenn es nicht schon feststeht, dann über das Subnet rausfinden
				if (intf != "")
				{
					std::string sString = "SELECT intf_id FROM interfaces ";
					sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
					sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
					sString += "WHERE device.dev_id=" + dev_id + " AND interfaces.intfName LIKE '" + intf + "'";
					std::vector<std::vector<std::string> > r3 = dieDB->query(sString.c_str());
					if (!r3.empty())
					{
						iString = "INSERT INTO rlink (interfaces_intf_id, l3routes_l3r_id) VALUES (" + r3[0][0] + "," + l3r_id + ");";
						dieDB->query(iString.c_str());
					}
					else
					{
						std::string dbgA = "\n6305: Parser Error! Code 0052 - Contact wktools@spoerr.org\n";
						dbgA += sString;
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehlerString += "\n" + dateiname + dbgA;
					}

				}
				else
				{
					std::string intid = nextHopIntfCheck(nh);
					if (intid == "")
					{
						intid = "NULL";
					}
					iString = "INSERT INTO rlink (interfaces_intf_id, l3routes_l3r_id) VALUES (" + intid + "," + l3r_id + ");";
					dieDB->query(iString.c_str());

					if (intid != "NULL")
					{
						std::string sString = "SELECT intfName FROM interfaces WHERE intf_id=" + intid;
						std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

						if (!ret.empty())
						{
							iString = "UPDATE l3routes SET interface='" + ret[0][0] + "' WHERE l3r_id = " + l3r_id + ";";
							dieDB->query(iString.c_str());
						}
						else
						{
							std::string dbgA = "\n6305: Parser Error! Code 0053 - Contact wktools@spoerr.org\n";
							dbgA += sString;
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
								WkLog::WkLog_ROT, WkLog::WkLog_FETT);
							fehlerString += "\n" + dateiname + dbgA;
						}
					}
				}
			}
			else
			{
				// IP/Prefix parsen
				aktPos1 = routes.rfind("\n", rPos);
				if (aktPos1 == routes.npos)
				{
					// Für den ersten Eintrag...
					aktPos1 = 0;
				}
				aktPos2 = routes.find("/", aktPos1);
				ip = routes.substr(aktPos1, aktPos2-aktPos1);

				aktPos1 = aktPos2;
				aktPos2 = routes.find(",", aktPos1);
				prefix = routes.substr(aktPos1, aktPos2-aktPos1);

				rPos = routes.find("ubest/mbest", viaPos);
			}
		}
	}
}

#endif // WKTOOLS_MAPPER