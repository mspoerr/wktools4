#include "class_WkmParserDB.h"


#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>  

#include <map>


#ifdef WKTOOLS_MAPPER

WkmParserDB::WkmParserDB(WkLog *logA, WkmDB *db)
{
	logAusgabe = logA;
	dieDB = db;

#ifdef DEBUG
	debugAusgabe = true;
	debugAusgabe1 = true;
#endif

}



WkmParserDB::~WkmParserDB()
{

}


std::string WkmParserDB::macAddChange(std::string macAdd)
{
	// Als erstes die Großbuchstaben in Kleinbuchstaben umwandeln
	boost::algorithm::to_lower(macAdd);

	// Dann die "." einfügen
	macAdd.insert(8, ".");
	macAdd.insert(4, ".");

	return macAdd;
}


std::string WkmParserDB::intfNameChange(std::string intfname)
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
	else if ((ipos1 = intfname.find("ATM")) != intfname.npos)
	{
		intfname.replace(ipos1, 6, "AT");
	}

	// Etwaige Leerzeichen löschen
	boost::algorithm::trim(intfname);

	return intfname;
}


void WkmParserDB::markNeighbor(std::string dev_id)
{
	zeitStempel("\tmarkNeighbor START");

	std::string sString = "UPDATE neighbor SET self=1 WHERE neighbor_id IN (";
	sString += "SELECT DISTINCT neighbor_id FROM neighbor INNER JOIN nlink ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "WHERE l3_addr IN (SELECT ipAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "WHERE devInterface.device_dev_id=" + dev_id + ") AND device_dev_id=" + dev_id + ")";
	dieDB->query(sString.c_str());

	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0023 - Mark neighbor failed. Please contact wktools@spoerr.org\n";
		dbgA += sString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}

}


// Route Tabelle befüllen
// prot: Routing Protocol
// ip: Netword Address
// prefix: Subnet Mask
// nh: Next Hop
// intf: Interface
// type: Type
// dev_id: dev_id
void WkmParserDB::insertRoute(std::string prot, std::string ip, std::string prefix, std::string nh, std::string intf, std::string type, std::string dev_id)
{
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
		std::string intid = nextHopIntfCheck(nh, dev_id);
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


// Auslesen der Interface ID
// dev_id: Device ID
// interfaceName: Interface Name
// err: Soll eine Fehler ausgegeben werden?
std::string WkmParserDB::getIntfID(std::string dev_id, std::string interfaceName, bool err)
{
	std::string intfID = "";
	std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
	iString += interfaceName + "' AND devInterface.device_dev_id=" + dev_id +";";
	std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
	if (!result.empty())
	{
		intfID = result[0][0];
	}
	else if (err)
	{
		std::string dbgA = "\n6406: Parser Error! Code 0035 - If Interface '" + interfaceName + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
		dbgA += iString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}
	return intfID;

}


// Auslesen der Interface ID aufgrund vom Nameif
// dev_id: Device ID
// nameif: nameif
// err: Soll eine Fehler ausgegeben werden?
std::string WkmParserDB::getIntfIDFromNameif(std::string dev_id, std::string nameif, bool err)
{
	std::string intfID = "";
	std::string iString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.nameif LIKE '";
	iString += nameif + "' AND devInterface.device_dev_id=" + dev_id +";";
	std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
	if (!result.empty())
	{
		intfID = result[0][0];
	}
	else if (err)
	{
		std::string dbgA = "\n6406: Parser Error! Code 0035 - If Interface '" + nameif + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
		dbgA += iString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}
	return intfID;

}


// Interface Name vom Nameif zurückliefern
// nameif: nameif für den das Interface zurückgegeben werden soll
// dev_id: dev_id
std::string WkmParserDB::getIntfNameFromNameif(std::string nameif, std::string dev_id)
{
	std::string iString = "SELECT intfName FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.nameif LIKE '";
	iString += nameif + "' AND devInterface.device_dev_id=" + dev_id +";";
	std::vector<std::vector<std::string> > result = dieDB->query(iString.c_str());
	std::string intfName = "";
	if (!result.empty())
	{
		intfName = result[0][0];
	}
	else
	{
		std::string dbgA = "\n6406: Parser Error! Code 0040 - If Interface '" + nameif + "' is up or the interface name is invalid, please contact wktools@spoerr.org\n";
		dbgA += iString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}

	return intfName;
}


// neighbor Tabelle befüllen
// intf_id: Interface ID
// maca: MAC Adresse
// ipa: IP Adresse
// self: Eigene IP?
void WkmParserDB::insertNeighbor(std::string intf_id, std::string maca, std::string ipa, std::string self)
{
	// Dann die Neighbor Tabelle füllen
	std::string iString2 = "INSERT INTO neighbor (neighbor_id, l2_addr, l3_addr, self) VALUES (NULL, '";
	iString2 += maca + "', '" + ipa + "','" + self + "');";
	dieDB->query(iString2.c_str());
	
	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0006 - Insert Neighbor failed. Please contact wktools@spoerr.org\n";
		dbgA += iString2;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}

	// Dann die nlink Tabelle
	std::string iString3 = "INSERT INTO nlink (neighbor_neighbor_id, interfaces_intf_id) VALUES (last_insert_rowid(),";
	iString3 += intf_id + ");";
	dieDB->query(iString3.c_str());
}


// VRF Tabelle befüllen; Rückgabe vrf_id
// vrfName: VRF Name
// rd: RD
// rtI: Import RT
// rtE: Export RT
std::string WkmParserDB::insertVrf(std::string vrfName, std::string rd, std::string rtI, std::string rtE)
{
	std::string vrf_id = "";

	// Check ob es das vrf schon gibt
	std::string sString = "SELECT vrf_id FROM vrf WHERE vrfName LIKE '" + vrfName + "'";
	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());				
	if (result.empty())
	{
		std::string iString = "INSERT INTO vrf (vrf_id,vrfName,rd,Import,Export) ";
		iString += "VALUES(NULL, '" + vrfName + "','" + rd + "','" + rtI + "','" + rtE + "');";
		dieDB->query(iString.c_str());
		sString = "SELECT last_insert_rowid() FROM vrf;";
		result = dieDB->query(sString.c_str());
		if (result.empty())
		{
			std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
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

	return vrf_id;
}


// VRF Link Tabelle befüllen
// intfName: Interface Name
// dev_id: dev_id
// vrf_id: vrf_id
void WkmParserDB::insertVrfLink(std::string intfName, std::string dev_id, std::string vrf_id)
{
	// Intf ID rausfinden
	// und dann vrflink Tabelle befüllen
	if (intfName != "")
	{
		std::string sString = "SELECT intf_id FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE device.dev_id=" + dev_id + " AND interfaces.intfName LIKE '" + intfName + "'";
		std::vector<std::vector<std::string> > r3 = dieDB->query(sString.c_str());
		if (!r3.empty())
		{
			std::string iString = "INSERT INTO vrflink (interfaces_intf_id, vrf_vrf_id) VALUES (" + r3[0][0] + "," + vrf_id + ");";
			dieDB->query(iString.c_str());
		}
		else
		{
			std::string dbgA = "\n6305: Parser Error! Code 0054 - Contact wktools@spoerr.org\n";
			dbgA += sString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
}


// STP Instance Tabelle befüllen
// stpPrio: STP Priority für das Vlan
// stpRootPort: STP Root Port
std::string WkmParserDB::insertSTPInstance(std::string stpPrio, std::string stpRootPort)
{
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
	return stpInstanzID;
}


// Flash Tabelle befüllen
// fname: Name der Partition
// fsize: Größe
// ffree: Freier Speicher
// dev_id: dev_id
void WkmParserDB::insertFlash(std::string fname, std::string fsize, std::string ffree, std::string dev_id)
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


// STP Status Tabelle befüllen
// dev_id: dev_id
// intfName: Interface Name
// stpIntfStatus: STP Port-Status
// designatedRootID: Root ID
// designatedBridgeID: Designated Bridge ID
// stpTransitionCount: STP Transition Count
// vlanid: VLAN ID
// intVlan: intVlan Tabelle befüllen?
void WkmParserDB::insertSTPStatus(std::string dev_id, std::string intfName, std::string stpIntfStatus, std::string designatedRootID, std::string designatedBridgeID, std::string stpTransitionCount, std::string vlanid, bool intVlan)
{
	// stp_status Tabelle befüllen
	std:: string iString = "INSERT INTO stp_status (stp_status_id, stpIntfStatus, designatedRootID, designatedBridgeID, stpTransitionCount) VALUES (NULL, '";
	iString += stpIntfStatus + "','" + designatedRootID  + "','" + designatedBridgeID + "'," + stpTransitionCount + ");";
	dieDB->query(iString.c_str());

	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0016 - Insert stp_status failed. Please contact wktools@spoerr.org\n";
		dbgA += iString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}


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
	}

	if (intVlan != 1)
	{
		// int_vlan Tabelle befüllen
		iString = "INSERT INTO int_vlan (interfaces_intf_id, vlan_vlan_id, stp_status_stp_status_id) VALUES (";
		iString += intfID + "," + vlanid + ",last_insert_rowid());";
		dieDB->query(iString.c_str());

		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0015 - Insert int_vlan failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
}



// Vlan Tabelle befüllen
// vlanNr: Vlan Nummer
// device_id: dev_id
// stpIntance: STP Instance ID; Leave empty if not applicable
void WkmParserDB::insertVlan(std::string vlanNr, std::string dev_id, std::string stpInstance)
{
	// VLAN Tabelle befüllen
	std::string iString = "INSERT INTO vlan (vlan_id, vlan) VALUES (NULL, ";
	iString += vlanNr + ");";
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

	if (stpInstance != "")
	{
		// vlan_stpInstanz Tabelle befüllen
		iString = "INSERT INTO vlan_stpInstanz (vlan_vlan_id, stpInstanz_stp_id) VALUES (";
		iString += vlID + "," + stpInstance + ");";
		dieDB->query(iString.c_str());

		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0007 - Insert vlan_stpInstanz failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}

	// vlan_has_device Tabelle befüllen
	iString = "INSERT INTO vlan_has_device (vlan_vlan_id, device_dev_id) VALUES (";
	iString += vlID + "," + dev_id + ");";
	dieDB->query(iString.c_str());
}

// cdp Tabelle befüllen
// hostname: Hostname
// nName: Neighbor Hostname
// alternatenName: Alternate Neighbor Hostname
// intf: Interface
// nIntfIP: Neighbor IP
// nIntf: Neighbor Interface 
// type: Neighbor Type
// platform: Neighbor HW Platform
// sw_version: Neighbor SW Version
// dev_id: dev_id
void WkmParserDB::insertCDP(std::string hostname, std::string nName, std::string alternatenName, std::string intf, std::string nIntfIP, std::string nIntf, std::string type, std::string platform, std::string sw_version, std::string dev_id)
{
	// Interface ID auslesen
	std::string sString = "SELECT intf_id FROM interfaces ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "WHERE intfName LIKE '" + intf + "' "; 
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
	iString += " VALUES('" + hostname + "','" + nName + "','" + alternatenName + "','" + intf + "','" + nIntfIP + "','" + nIntf + "'," + type + ",'" + platform + "','" + sw_version + "');";
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

// IpPhoneDet Tabelle befüllen
// voiceVlan: Voice VLAN
// cm1: CallManager 1 IP
// cm2: CallManager 2 IP
// dscpSig: DSCP for Signalling
// dscpConf: DSCP for Config
// dscpCall: DSCP for RDP
// defGW: Default Gateway
// dev_id: Device ID
void WkmParserDB::insertPhone(std::string voiceVlan, std::string cm1, std::string cm2, std::string dscpSig, std::string dscpConf, std::string dscpCall, std::string defGW, std::string dev_id)
{
	// IP Phone Detail
	std::string ipphRow = "";
	std::string iString = "INSERT INTO ipPhoneDet (voiceVlan, cm1, cm2, dscpSig, dscpConf, dscpCall, defGW)";
	iString += " VALUES('" + voiceVlan + "','" + cm1 + "','" + cm2 + "','" + dscpSig + "','" + dscpConf + "','" + dscpCall + "','" + defGW + "');";
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


void WkmParserDB::iosCDPSchreiber()
{
	std::string intf_id = "";
	std::string dev_id = "";
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
		std::string chassisSN = "";
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
		std::string hwPos = "box";
		std::string modell = it->at(2);
		std::string version = it->at(3);
		std::string hwBootfile = "";
		std::string hwMem = "";
		std::string hwDescription = "";
		std::string hwRevision = "";
		std::string hwInfo_id = insertHardware(hwPos, modell, hwDescription, chassisSN, hwRevision, hwMem, hwBootfile, version, dev_id, "");

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
			intf_id = insertInterface(it1->at(1), "", "", "", macAddress, it1->at(0), " ", "", "", "UP", "", "L3", dev_id);

			// 3. clink Tabelle updaten
			iString = "INSERT INTO clink (interfaces_intf_id, cdp_cdp_id) VALUES (";
			iString += intf_id + "," + cdp_id + ");";
			dieDB->query(iString.c_str());


			// 4. intfSubnet befüllen
			// Dafür muss die IP Adresse in ulong umgewandelt werden
			boost::system::error_code errorcode = boost::asio::error::host_not_found;
			boost::asio::ip::address_v4 ipa = boost::asio::ip::address_v4::from_string(it1->at(0), errorcode);
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
			intf_id = insertInterface("Radio", "", "", "1", "", "", "", "", "", "UP", "Radio Interface", "L2", dev_id);
		}
		else if (dType == "11")		// IP Phone
		{
			// Endgeräte INTERFACE -> Generisches Interface zum Anzeigen, ob Endgeräte angeschlossen ist
			//////////////////////////////////////////////////////////////////////////
			intf_id = insertInterface("Port2", "", "", "1", "", "", "", "", "", "UP", "PC Port", "L2", dev_id);
		}
	}
}


void WkmParserDB::ipNetzeEintragen(std::string intfip, std::string intfmask, std::string intf_id)
{
	boost::system::error_code errorcode = boost::asio::error::host_not_found;
	boost::asio::ip::address_v4 ipa = boost::asio::ip::address_v4::from_string(intfip, errorcode);
	boost::asio::ip::address_v4 netmask;
	boost::asio::ip::address_v4 bcAdr;		// Broadcast
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
		netmask = boost::asio::ip::address_v4::from_string(intfmask, errorcode);
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
	boost::asio::ip::address_v4 netzAddr(network);

	boost::asio::ip::address_v4 m(mask);

	std::string netzwerk = netzAddr.to_string();
	std::string maske = m.to_string();
	
	bcAdr = boost::asio::ip::address_v4::broadcast(netzAddr,m);
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


std::string WkmParserDB::nextHopIntfCheck(std::string ipaddr, std::string dev_id)
{
	boost::system::error_code errorcode = boost::asio::error::host_not_found;
	boost::asio::ip::address_v4 ipa = boost::asio::ip::address_v4::from_string(ipaddr, errorcode);
	boost::asio::ip::address_v4 netmask;
	boost::asio::ip::address_v4 bcAdr;		// Broadcast
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
		netmask = boost::asio::ip::address_v4::from_string(ipit->at(1), errorcode);
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
		boost::asio::ip::address_v4 netzAddr(network);
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


std::string WkmParserDB::maskToBits(std::string subnetmask)
{
	boost::system::error_code errorcode = boost::asio::error::host_not_found;
	boost::asio::ip::address_v4 netmask = boost::asio::ip::address_v4::from_string(subnetmask, errorcode);
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


// Neues Gerät in die DB schreiben; Rückgabe dev_id
// hName: Hostname
// hType: Type
// hHwtype: HW Type
// hConfReg: Config Register
std::string WkmParserDB::insertDevice(std::string hName, std::string hType, std::string hHwtype, std::string hConfReg)
{
	// Prüfen, ob es den Hostnamen schon gibt. Wenn ja, dann den neuen Hostnamen ändern (um "-1" erweitern)
	std::string sString = "";
	if (!rControlEmpty)
	{
		sString = "SELECT hostname FROM device WHERE hostname LIKE '" + hName + "' AND device.dev_id > (SELECT MAX (dev_id) FROM rControl)"; 
	}
	else
	{
		sString = "SELECT hostname FROM device WHERE hostname LIKE '" + hName + "'"; 
	}

	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
	if (!result.empty())
	{
		hName = hName + "-1";
	}

	std::string iString = "INSERT INTO device (dev_id,type,hwtype,hostname,confReg) VALUES(NULL, " + hType + ", " + hHwtype + ", '" + hName + "', '" + hConfReg + "');";
	dieDB->query(iString.c_str());

	sString = "SELECT last_insert_rowid() FROM device;";
	result = dieDB->query(sString.c_str());
	if (result.empty())
	{
		std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return "";
	}
	else
	{
		std::string d_id = result[0][0];
		return d_id;
	}
}


// Hardware Infos für Gerät oder Modul schreiben (hwInfo und hwLink Tabellen)
// hPos: Position im Chassis
// hModell: Modell
// hDescription: Beschreibung
// hChassisSN: S/N
// hRevision: HW Revision Version
// hMem: Memory
// hBootfile: Bootfile
// hVersion: SW Version
// dev_id: dev_id
// boxHwInfId: hwInfo_id von der Box
std::string WkmParserDB::insertHardware(std::string hPos, std::string hModell, std::string hDescription, std::string hChassisSN, std::string hRevision, std::string hMem, std::string hBootfile, std::string hVersion, std::string dev_id, std::string boxHwInfId)
{
	boost::algorithm::trim(hPos);
	boost::algorithm::trim(hModell);
	boost::algorithm::trim(hDescription);
	boost::algorithm::trim(hChassisSN);
	boost::algorithm::trim(hRevision);
	boost::algorithm::trim(hMem);
	boost::algorithm::trim(hBootfile);
	boost::algorithm::trim(hVersion);
	std::string iString = "INSERT INTO hwInfo (hwinf_id,module,type,description,sn,hwRevision,memory,bootfile,sw_version) ";
	iString += "VALUES(NULL, '" + hPos + "', '" + hModell + "', '" + hDescription + "', '" + hChassisSN + "', '" + hRevision + "', '" + hMem + "', '" + hBootfile + "', '" + hVersion + "');";
	dieDB->query(iString.c_str());

	std::string hw_id = "";
	std::string hwInfoID = "";
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
		if (hPos == "box")
		{
			hwInfoID = hw_id;
		}
		else
		{
			hwInfoID = boxHwInfId;
		}

		// hwLink befüllen
		std::string iString2 = "INSERT INTO hwlink (hwInfo_hwinf_id, device_dev_id) VALUES (" + hw_id + "," + dev_id + ");";
		dieDB->query(iString2.c_str());

	}
	return hwInfoID;
}


// Hardware Infos für Gerät oder Modul updaten (hwInfoTabelle)
// Es werden nur die Positionen aktualisiert, die nicht leer ("") sind
// hModell: Modell
// hDescription: Beschreibung
// hChassisSN: S/N
// hRevision: HW Revision Version
// hMem: Memory
// hBootfile: Bootfile
// hVersion: SW Version
// hwInfo_id: hwInfo_id; Wenn "last", dann die letzte hwinf_id verwenden
void WkmParserDB::updateHardware(std::string hModell, std::string hDescription, std::string hChassisSN, std::string hRevision, std::string hMem, std::string hBootfile, std::string hVersion, std::string hwinfo_id)
{
	boost::algorithm::trim(hModell);
	boost::algorithm::trim(hDescription);
	boost::algorithm::trim(hChassisSN);
	boost::algorithm::trim(hRevision);
	boost::algorithm::trim(hMem);
	boost::algorithm::trim(hBootfile);
	boost::algorithm::trim(hVersion);

	//std::string iString = "INSERT INTO hwInfo (memory,bootfile,sw_version) ";
	//iString += "VALUES(hMem + "', '" + hBootfile + "', '" + hVersion + "');";

	std::string hwInfString = "";
	if (hwinfo_id == "last")
	{
		hwInfString = "' WHERE hwinf_id IN (SELECT last_insert_rowid() FROM hwInfo)";
	}
	else
	{
		hwInfString = "' WHERE hwinf_id= " + hwinfo_id;
	}

	if (hModell !="")
	{
		std::string iString = "UPDATE hwInfo SET type='" + hModell + hwInfString + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0031 - Update Hardware failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
	if (hDescription !="")
	{
		std::string iString = "UPDATE hwInfo SET description='" + hDescription + hwInfString + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0031 - Update Hardware failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
	if (hChassisSN !="")
	{
		std::string iString = "UPDATE hwInfo SET sn='" + hChassisSN + hwInfString + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0031 - Update Hardware failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
	if (hRevision !="")
	{
		std::string iString = "UPDATE hwInfo SET hwRevision='" + hRevision + hwInfString + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0031 - Update Hardware failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
	if (hMem !="")
	{
		std::string iString = "UPDATE hwInfo SET memory='" + hMem + hwInfString + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0031 - Update Hardware failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
	if (hBootfile !="")
	{
		std::string iString = "UPDATE hwInfo SET bootfile='" + hBootfile + hwInfString + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0031 - Update Hardware failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
	if (hVersion !="")
	{
		std::string iString = "UPDATE hwInfo SET sw_version='" + hVersion + hwInfString + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0031 - Update Hardware failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
	}
}

// Lic Tabelle befüllen
// lName: Lizenz Name
// lStatus: Lizenz Status
// lType: Lizenz Type
// lNextBoot: Lizenz beim nächsten Boot
// lCluster: Cluster Lic
// dev_id: dev_id
bool WkmParserDB::insertLic(std::string lName, std::string lStatus, std::string lType, std::string lNextBoot, std::string lCluster, std::string dev_id)
{
	std::string iString = "INSERT INTO license (license_id, name, status, type, nextBoot, cluster) ";
	iString += "VALUES(NULL, '" + lName + "', '" + lStatus + "', '" + lType + "', '" + lNextBoot + "', " + lCluster + ");";
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


std::string WkmParserDB::zeitStempel(std::string info)
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


std::string WkmParserDB::intfTypeCheck(std::string intfName, std::string intfSpeed)
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
	else if ((ipos1 = intfName.find("AT")) != intfName.npos)
	{
		intfType = "ATM";
	}


	return intfType;
}


std::string WkmParserDB::intfPhlCheck(std::string intfName, std::string intfSpeed)
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


void WkmParserDB::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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


// Interface und Device Tabellen mit STP Infos updaten
// stpBridgeID: STP Bridge ID
// stpProtocol: STP Protokoll
// dev_id: Device ID
void WkmParserDB::updateSTP(std::string stpBridgeID, std::string stpProtocol, std::string dev_id)
{
	std::string iString = "UPDATE device SET stpBridgeID='" + stpBridgeID + "', stpProtocol='" + stpProtocol + "' WHERE dev_id=" + dev_id + ";";
	dieDB->query(iString.c_str());

	iString = "INSERT INTO interfaces (intfName, macAddress, l2l3, status, intfType) VALUES ('STP','" + stpBridgeID + "','L2','UP','STP');";
	dieDB->query(iString.c_str());
}


// Intf-ID vom Channel Interface bei allen Member Interfaces einfügen
// ch_intf_id: intf_id vom Channel Interface
// chMem: String mit den Interface Namen von den physikalischen Interfaces
// dev_id: Device ID dev_id
void WkmParserDB::updateChannelMember(std::string ch_intf_id, std::string chMem, std::string dev_id)
{
	if (chMem != "")
	{
		boost::char_separator<char> sep(" ,");
		boost::tokenizer<boost::char_separator<char>> tok(chMem, sep);

		for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
		{
			std::string iString3 = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
			iString3 += *beg + "' AND devInterface.device_dev_id=" + dev_id +";";
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
	
			std::string iString4 = "UPDATE interfaces SET channel_intf_id=" + ch_intf_id + " WHERE intf_id = " + chMemID + ";";
			dieDB->query(iString4.c_str());
	
			if (dieDB->error != "not an error")
			{
				std::string dbgA = "\n6406: Parser Error! Code 0026 - Update Channel Members failed. Please contact wktools@spoerr.org\n";
				dbgA += iString4;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
					WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
				fehlerString += "\n" + dateiname + dbgA;
			}
		}
	}
}


// Intf-ID vom Hauptinterface am SubInterface einfügen (channel_intf_id)
// intf_id: Interface-ID vom Sub-Interface
// mIntfName: Interface Name vom Hauptinterface
// dev_id: Device ID dev_id
void WkmParserDB::updateSubInterfaces(std::string intf_id, std::string mIntfName, std::string dev_id)
{
	// Das Hauptinterface finden und dann die Subinterfaces markieren
	if (mIntfName != "")
	{
		std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
		sString += mIntfName + "' AND devInterface.device_dev_id=" + dev_id +";";
		std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

		if (!result.empty())
		{
			std::string iString4 = "UPDATE interfaces SET channel_intf_id=" + result[0][0] + " WHERE intf_id =" + intf_id + ";";
			dieDB->query(iString4.c_str());

			if (dieDB->error != "not an error")
			{
				std::string dbgA = "\n6406: Parser Error! Code 0028 - Update SubInterface failed. Please contact wktools@spoerr.org\n";
				dbgA += iString4;
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
					WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
				fehlerString += "\n" + dateiname + dbgA;
			}
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
}


// VSL Interfaces markieren
// intf_id: Zu markierendes Interface; Wenn leer, dann den Channel markieren
void WkmParserDB::markVSL(std::string intf_id)
{
	if (intf_id == "")
	{
		// VSL Port Channel markieren
		std::string iString = "UPDATE interfaces SET phl=2 WHERE intf_id IN (SELECT channel_intf_id FROM interfaces WHERE phl=2)";
		dieDB->query(iString.c_str());
	}
	else
	{
		std::string iString = "UPDATE interfaces SET phl=2 WHERE intf_id = " + intf_id + ";";
		dieDB->query(iString.c_str());
	}
}


// vPC Informationen eintragen
// dev_id: dev_id
// vpcDomId: vPC Domain ID
// vpcLink: Interface Name vom vPC Peer Link
// vpcId: vPC ID vom Interface
void WkmParserDB::markVPC(std::string dev_id, std::string vpcDomId, std::string vpcLink, std::string vpcId)
{

	// Die vPCs markieren
	std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
	sString += vpcLink + "' AND devInterface.device_dev_id=" + dev_id +";";
	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

	std::string iString = "";
	if (!result.empty())
	{
		iString = "UPDATE interfaces SET vpc_id=" + vpcId + " WHERE intf_id = " + result[0][0] + ";";
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

	if (vpcDomId != "")
	{
		// Globale vPC Einstellungen
		// Device Table aktualisieren:
		std::string iString = "UPDATE device SET vpcDomId=" + vpcDomId + " WHERE dev_id = " + dev_id + ";";
		dieDB->query(iString.c_str());

		// vPC Interface in DB einfügen und auf den Peer Link verweisen
		iString = "INSERT INTO interfaces (intfName, macAddress, l2l3, status, intfType, vpc_id) VALUES ('vPC Peer-Link','','L2','UP','vPC Peer-Link'," + vpcId + ");";
		dieDB->query(iString.c_str());

		// devInterface Tabelle mit Infos zu vPC Peer-Link befüllen
		std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
		dieDB->query(iString2.c_str());
	}
}


// Die BoundTo Members in der DB updaten -> Am Logischen Vethernet Interface das physikalische Ethernet Interface hinterlegen (ID)
// dev_id: dev_id
// boundToIntf: Interface Name vom HW Interface, an das das Veth gebunden ist
// intf_id: intf_id vom Veth Interface
void WkmParserDB::markVethMember(std::string dev_id, std::string boundToIntf, std::string intf_id)
{
	std::string iString3 = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
	iString3 += boundToIntf + "' AND devInterface.device_dev_id=" + dev_id + ";";
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


// Interface Switchport Informationen updaten
// l2AdminMode: L2 Admin Mode
// l2Mode: L2 Operational Mode
// l2encap: L2 Encapsulation
// dtp: DTP Status
// nativeAccess: Native und Access Vlan Id (Je nach Operational Mode)
// voiceVlan: Voice Vlan ID
// opPrivVlan: Private VLAN Status
// allowedVlan: Allowed VLANs (Trunk)
// intf_id: intf_id
void WkmParserDB::updateIntfL2Info(std::string l2AdminMode, std::string l2Mode, std::string l2encap, std::string dtp, std::string nativeAccess, std::string voiceVlan, std::string opPrivVlan, std::string allowedVlan, std::string intf_id)
{
	std::string sString = "UPDATE interfaces SET l2AdminMode='" + l2AdminMode + "',l2Mode='" + l2Mode + "', l2encap='" + l2encap + "', dtp='" + dtp;
	sString += "', nativeAccess='" + nativeAccess + "', voiceVlan='" + voiceVlan + "', opPrivVlan='" + opPrivVlan + "', allowedVlan='" + allowedVlan;
	sString += "' WHERE intf_id=" + intf_id + ";";

	dieDB->query(sString.c_str());
	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0029 - Update IntfL2Info failed. Please contact wktools@spoerr.org\n";
		dbgA += sString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}
}


// SNMP Location Information nachtragen
// snmpLoc: SNMP Location
// dev_id: dev_id
void WkmParserDB::insertSnmp(std::string snmpLoc, std::string dev_id)
{
	std::string iString = "UPDATE device SET snmpLoc='" + snmpLoc + "' WHERE dev_id= " + dev_id + ";";
	dieDB->query(iString.c_str());
	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0030 - Update SNMP failed. Please contact wktools@spoerr.org\n";
		dbgA += iString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}
}


// Geräte-bezogene POE Infos einfügen
// poeStatus: POE Status
// poeCurrent: aktueller POE Verbrauch
// poeMax: Maximaler POE Output
// poeRemaining: Verfügbare POE Leistung
// dev_id: dev_id
void WkmParserDB::insertPoeDev(std::string poeStatus, std::string poeCurrent, std::string poeMax, std::string poeRemaining, std::string dev_id)
{
	std::string uString = "UPDATE device SET poeStatus='ON', poeCurrent='" + poeCurrent + "', poeMax='" + poeMax + "', poeRemaining='" + poeRemaining + "' WHERE dev_id=" + dev_id;
	dieDB->query(uString.c_str());
	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0024 - Update device failed. Please contact wktools@spoerr.org\n";
		dbgA += uString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}
}


// Interface-bezogene POE Infos einfügen
// poeStatus: POE Status
// poeWatt: aktueller POE 
// poeWattmax: Maximaler POE Output
// poeDevice: POE Endgerät
// intfName: Interface Name
// dev_id: dev_id
void WkmParserDB::insertPoeIntf(std::string poeStatus, std::string poeWatt, std::string poeWattmax, std::string poeDevice, std::string intfName, std::string dev_id)
{
	// Interface updaten
	std::string sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON (devInterface.interfaces_int_id=interfaces.intf_id) WHERE interfaces.intfName LIKE '";
	sString += intfName + "' AND devInterface.device_dev_id=" + dev_id +";";
	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

	std::string intfID = "";
	if (!result.empty())
	{
		intfID = result[0][0];
		std::string iString = "UPDATE interfaces SET poeStatus='" + poeStatus + "',poeWatt='" + poeWatt + "',poeWattmax='" + poeWattmax + "',poeDevice='" + poeDevice + "' WHERE intf_id = " + intfID + ";";
		dieDB->query(iString.c_str());
		if (dieDB->error != "not an error")
		{
			std::string dbgA = "\n6406: Parser Error! Code 0019 - Update Interface failed. Please contact wktools@spoerr.org\n";
			dbgA += iString;
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			fehlerString += "\n" + dateiname + dbgA;
		}
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


// Interface einfügen - Globale Informationen; intf_id wird zurückgegeben
// intfName: Interface Name (normalized)
// nameif: ASA nameif
// intfType: Interface Type (normalized - intfTypeCheck() )
// phl: logisch oder physikalisch (normalized - intfPhlCheck() )
// macAddress: MAC Adresse
// ipAddress: IP Adresse
// subnetMask: IP Subnet Mask
// duplex: Duplex
// speed: Speed
// status: Interface Status (up, down, shut...)
// description: Description
// l2l3: L2 oder L3
// dev_id: dev_id
std::string WkmParserDB::insertInterface(std::string intfName, std::string nameif, std::string intfType, std::string phl, std::string macAddress, std::string ipAddress, std::string subnetMask,					
		std::string duplex, std::string speed, std::string status, std::string description,	std::string l2l3, std::string dev_id)
{
	std::string iString = "INSERT INTO interfaces (intf_id,intfName,nameif,intfType,phl,macAddress,ipAddress,subnetMask,duplex,speed,status,description,l2l3,channel_intf_id)";
	iString += " VALUES(NULL, '" + intfName + "','" + nameif + "','" + intfType + "'," + phl + ",'" + macAddress + "','" + ipAddress + "','"  + subnetMask + "','" + duplex + "','" + speed;
	iString += "','" + status + "','" + description + "','" + l2l3 + "', NULL);";
	dieDB->query(iString.c_str());

	std::string sString = "SELECT last_insert_rowid() FROM interfaces;";
	std::vector<std::vector<std::string> > result2 = dieDB->query(sString.c_str());
	std::string intf_id = "";
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

	// devInterface Tabelle befüllen
	std::string iString2 = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (last_insert_rowid()," + dev_id + ");";
	dieDB->query(iString2.c_str());

	
	return intf_id;
}


// Interface Statistik und Error Counters einfügen
// errLvl: Error Level
// lastClear: "clear interfaces" Timestamp
// loadLvl: Load Level
// iCrc: CRC Fehler
// iFrame: Frame Fehler
// iOverrun: Overruns
// iIgnored: Ignored
// iWatchdog: Watchdog
// iPause: Pause
// iDribbleCondition: Dribble Condition
// ibuffer: Buffer Fehler
// l2decodeDrops: L2 Decode Drops (ASA)
// runts: Runts
// giants: Giants
// throttles: Throttles
// ierrors: Input Errors
// underrun: Underruns
// oerrors: Output Errors
// oCollisions: Output Collissions
// oBabbles: Babbles
// oLateColl: Late Collissions
// oDeferred: Deferred
// oLostCarrier: Lost Carrier
// oNoCarrier: No Carrier
// oPauseOutput: Output Pause Frames
// oBufferSwapped: Buffer Swapped
// resets: Resets
// obuffer: Output Buffer
// lastInput: Last Input Timestamp
// intf_id: intf_id
void WkmParserDB::insertIntfStats(std::string errLvl, std::string lastClear, std::string loadLvl, std::string iCrc, std::string iFrame, std::string iOverrun, std::string iIgnored, std::string iWatchdog,					
		std::string iPause, std::string iDribbleCondition, std::string ibuffer, std::string l2decodeDrops, std::string runts, std::string giants, std::string throttles, std::string ierrors,					
		std::string underrun, std::string oerrors, std::string oCollisions, std::string oBabbles, std::string oLateColl, std::string oDeferred, std::string oLostCarrier, std::string oNoCarrier,					
		std::string oPauseOutput, std::string oBufferSwapped, std::string resets, std::string obuffer, std::string lastInput, std::string intf_id)
{
	std::string sString = "UPDATE interfaces SET errLvl=" + boost::lexical_cast<std::string>(errLvl) + ",lastClear='" + lastClear + "', loadLvl=" + boost::lexical_cast<std::string>(loadLvl);
	sString += ", iCrc='" + iCrc + "', lastInput='" + lastInput + "', obuffer='" + obuffer;
	sString += "', iFrame='" + iFrame + "', iOverrun='" + iOverrun + "', iIgnored='" + iIgnored + "', iWatchdog='" + iWatchdog + "', iPause='" + iPause + "', iDribbleCondition='" + iDribbleCondition;
	sString += "', ibuffer='" + ibuffer + "', l2decodeDrops='" + l2decodeDrops + "', runts='" + runts + "', giants='" + giants + "', throttles='" + throttles + "', ierrors='" + ierrors;
	sString += "', underrun='" + underrun + "', oerrors='" + oerrors + "', oCollisions='" + oCollisions + "', oBabbles='" + oBabbles + "', oLateColl='" + oLateColl + "', oDeferred='" + oDeferred;
	sString += "', oLostCarrier='" + oLostCarrier + "', oNoCarrier='" + oNoCarrier + "', oPauseOutput='" + oPauseOutput + "', oBufferSwapped='" + oBufferSwapped + "', resets='" + resets;
	sString += "' WHERE intf_id=" + intf_id + ";";

	dieDB->query(sString.c_str());
	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0032 - Update IntfStats failed. Please contact wktools@spoerr.org\n";
		dbgA += sString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}
}


// Nexus1k Interface Infos einfügen (Veth Interfaces)
// actModule: Active on Module -> Virtual Ethernet Module # auf der das Veth aktiv ist
// owner: Owner (VM, der das Interface zugeordnet ist)
// portProfile: Port Profile -> VMWare Port Profile
// intf_id: intf_id
void WkmParserDB::intertIntfN1k(std::string actModule, std::string owner, std::string portProfile, std::string intf_id)
{
	std::string sString = "UPDATE interfaces SET actModule='" + actModule + "',owner='" + owner + "', portProfile='" + portProfile;
	sString += "' WHERE intf_id=" + intf_id + ";";

	dieDB->query(sString.c_str());
	if (dieDB->error != "not an error")
	{
		std::string dbgA = "\n6406: Parser Error! Code 0033 - Update IntfN1k failed. Please contact wktools@spoerr.org\n";
		dbgA += sString;
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		fehlerString += "\n" + dateiname + dbgA;
	}

}


void WkmParserDB::insertRControl()
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


void WkmParserDB::rControlFlag()
{
	// Check, ob rControl leer ist
	std::string sString = "SELECT MAX( n_id) FROM rControl";
	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
	if (ret.empty())
	{
		rControlEmpty = true;
	}
	else
	{
		if (ret[0][0] == "")
		{
			rControlEmpty = true;
		}
		else
		{
			rControlEmpty = false;
		}
	}

}


//bool WkmParserDB::sdirLua()
//{
//	schreibeLog(WkLog::WkLog_ZEIT, "6626: Searching for Lua Parser Scripts\r\n", "6626", 
//	WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
//
//	std::string wktoolsPfad = ::wxGetCwd();
//	std::queue<fs::path> files;				// Files, die nach Lua Scripts durchsucht werden sollen
//
//
//	fs::directory_iterator end_itr; // default construction yields past-the-end
//	for (fs::directory_iterator itr( wktoolsPfad ); itr != end_itr; ++itr)
//	{
//		if (fs::is_directory(itr->status()))
//		{
//			// Pfad gefunden
//			// Es soll nur im Hauptverzeichnis gesucht werden
//		}
//		else
//		{
//			// File gefunden -> Check ob *.lua File
//			// Wenn ja, dann gleich einlesen und zuweisen
//			std::string dateiname = itr->path().string();
//
//			if (dateiname.find(".lua") != dateiname.npos)
//			{
//				std::ifstream konfigDatei(dateiname.c_str(), std::ios_base::in);
//				std::string ganzeDatei = "";
//
//				getline(konfigDatei, ganzeDatei, '\0');
//				konfigDatei.close();
//
//				std::string dbgA = "";
//				// Nur zuweisen, wenn nicht leer
//				if (!ganzeDatei.empty())
//				{
//
//					std::size_t pos1 = dateiname.find_last_of("\\/");
//					std::size_t pos2 = dateiname.find_last_of(".");
//					if (pos2>pos1)
//					{
//						pos1++;
//						std::string luaScriptName = dateiname.substr(pos1, pos2-pos1);
//
//						luaScripts.insert(pss(luaScriptName, ganzeDatei));
//					
//						std::string infoText = "6627: Lua Script \"" + luaScriptName + "\" found: " + dateiname + "\r\n";
//						schreibeLog(WkLog::WkLog_ZEIT, infoText, "6627", 
//						WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
//					}
//				}
//			}
//		}
//	}
//
//	return true;
//}



#endif // WKTOOLS_MAPPER
