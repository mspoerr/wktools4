// Neue Combiner Klasse mit neuer Reihenfolge und mehr Vertrauen auf CDP

#include "class_wkmCombiner2.h"
#ifdef WKTOOLS_MAPPER

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>    


WkmCombiner2::WkmCombiner2(WkmDB *db, WkLog *logA, WkmParserDB *pdb, bool ih, bool rd)
{
	dieDB = db;
	logAusgabe = logA;
	parserdb = pdb;
	reverseDns = rd;
	importHosts = ih;

	nidEmpty = true;

	debugAusgabe = false;
	debugAusgabe1 = false; // Detaillierte Debug Ausgabe für SQL Abfragen

#ifdef DEBUG
	debugAusgabe = true;
	debugAusgabe1 = true;
#endif
	stop = false;
	dbgGlobalCounter = 0;
	dbgUntergeordnetCounter = 0;
}


WkmCombiner2::~WkmCombiner2()
{

}


bool WkmCombiner2::doIt()
{
	// Check, ob n_id leer ist
	std::string sString = "SELECT MAX(n_id) FROM rControl";
	std::vector<std::vector<std::string>> rcidret = dieDB->query(sString.c_str());
	if (rcidret[0][0] == "" || rcidret[0][0] == "0")
	{
		nidEmpty = true;
	}
	else
	{
		nidEmpty = false;
	}

	// CDP CHECK
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	cdpChecks();

	// Switch CHECK
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	switchChecks();

	// vPC CHECK
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	vpcCheck();

	// Non-Switch CHECK
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	dedicatedL2();

	// Unknown STP CHECK
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	unknownSTP();
	
	// Unknown RP Neighbor CHECK
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	unknownRP();

	// Duplikate entfernen
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	delDups();
	
	// Abschließender Anomalie Check und Engeräte Import
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	markAnomaly();

	// L3 Checks
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	rpNeighborCheck();
	rpCloudNeighborCheck();
	rpIpsecPeerCheck();

	// Switch Dev Typen setzen
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}
	setSwitchDevType();

	// rControl befüllen
	//////////////////////////////////////////////////////////////////////////
	insertRControl();
}


void WkmCombiner2::unknownRP()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Unknown RP Neighbor Check\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	std::string sString = "SELECT dev_id,hostname,intfName,intf_id,macAddress,ipAddress FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
	sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id WHERE dataSource=3 AND macAddress NOT LIKE '' ";
	if (!nidEmpty)
	{
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	}
	dbgAsgbe1(sString, 1, 6, "RP Neighbor: Find RP neighbors");

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

	// Unknown RP Nachbarschaften auslesen
	//////////////////////////////////////////////////////////////////////////
	for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Alle Interfaces herausfinden, auf dem MAC Adressen von einem Nicht-Switch auf L2 Interfaces gelernt werden, die nicht schon eine Nachbarschaftsbeziehung haben
		sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface ";
		sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
		sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
		sString += "WHERE neighbor.l2_addr IN ( ";
		sString += "SELECT macAddress FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE device.dev_id=" + it->at(0) + ") ";
		sString += "AND interfaces.l2l3 LIKE 'L2' ";
		sString += "AND interfaces.intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship) ";
		sString += "AND interfaces.intf_id NOT IN (SELECT dI_intf_id FROM neighborship) ";
		if (!nidEmpty)
		{
			sString += "AND interfaces_int_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 2, "Unknown RP");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

		for (std::vector<std::vector<std::string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			// Änderung 06.03.2013: "AND device.hwtype <> 2" hinzugefügt
			sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
			sString += "WHERE interfaces.macAddress IN ( ";
			sString += "SELECT l2_addr FROM neighbor ";
			sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
			sString += "WHERE interfaces.intf_id=" + it2->at(0) + ") ";
			sString += "AND interfaces.phl=1 AND device.hwtype <> 2 ";
			if (!nidEmpty)
			{
				sString += "AND interfaces_int_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
			}

			dbgAsgbe1(sString, 2, 2, "Unknown RP");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			for (std::vector<std::vector<std::string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			{
				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + it3->at(0);
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + it3->at(0) + ");";

				dbgAsgbe1(sString, 2, 1, "Unknown RP");

				std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

				if (nid.empty())
				{
					std::string intf1 = it2->at(0);
					std::string intf2 = it3->at(0);

					// Check, ob Subinterface, oder nicht
					sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf1 + ";";
					std::vector<std::vector<std::string>> sid1 = dieDB->query(sString.c_str());

					dbgAsgbe1(sString, 2, 1, "Unknown RP");

					if (sid1[0][0] != "")
					{
						intf1 = sid1[0][0];
					}

					sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf2 + ";";
					std::vector<std::vector<std::string>> sid2 = dieDB->query(sString.c_str());

					dbgAsgbe1(sString, 2, 1, "Unknown RP");

					if (sid2[0][0] != "")
					{
						intf2 = sid2[0][0];
					}

					std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
					iString += "VALUES (" + intf1 + "," + it2->at(1) + "," + intf2 + "," + it3->at(1) + ", 7);";
					dieDB->query(iString.c_str());

					dbgAsgbe1(iString, 2, 0, "Unknown RP");

				}
			}
		}
	}
	dbgNachbarAusgabe();

	// Unknown RP Nachbarschaften auslesen #2
	//////////////////////////////////////////////////////////////////////////
	for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Alle Interfaces herausfinden, auf dem MAC Adressen von einem Nicht-Switch auf L2 Interfaces gelernt werden, die nicht schon eine Nachbarschaftsbeziehung haben
		sString =  "SELECT hostname,intfName,macAddress,ipAddress,dev_id,intf_id,l2_addr,l3_addr FROM neighbor ";
		sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE l2_addr LIKE '" + it->at(4) + "' AND l3_addr LIKE '" + it->at(5) + "' ";
		sString += "AND phl=1 AND interfaces.intf_id NOT IN(SELECT dI_intf_id1 FROM neighborship) AND interfaces.intf_id NOT IN(SELECT dI_intf_id FROM neighborship) ";
		if (!nidEmpty)
		{
			sString += "AND interfaces_int_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 8, "Unknown RP #2");

		std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

		if (!ret2.empty())
		{
			std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
			iString += "VALUES (" + it->at(3) + "," + it->at(0) + "," + ret2[0][5] + "," + ret2[0][4] + ", 8);";
			dieDB->query(iString.c_str());

			dbgAsgbe1(iString, 2, 0, "Unknown RP #2");
		}
	}
	dbgNachbarAusgabe();
}


void WkmCombiner2::cdpChecks()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - CDP Check\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Alle CDP Nachbarschaftsbeziehungen finden und dann in die Nachbarschaftstabelle einbauen
	std::string sString = "SELECT devInterface.device_dev_id,devInterface.interfaces_int_id,cdp.nName,cdp.nIntf,cdp.nIntfIP FROM devInterface ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id INNER JOIN clink ON clink.interfaces_intf_id=interfaces.intf_id ";
	sString += "INNER JOIN cdp ON cdp.cdp_id=clink.cdp_cdp_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.dataSource IS NULL ";
	if (!nidEmpty)
	{
		sString += "AND cdp_cdp_id > (SELECT MAX(cdp_id) FROM rControl WHERE cdp_id < (SELECT MAX(cdp_id) FROM rControl));";
	}
	dbgAsgbe1(sString, 1, 5, "CDP C: Find cdp neighbors");

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Die device_id zur IP Adresse finden
		sString = "SELECT dev_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.hostname LIKE '" + it->at(2);
		sString += "' AND interfaces.ipAddress LIKE '" + it->at(4) + "' ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}
		dbgAsgbe1(sString, 2, 1, "CDP C: Find cdp neighbors");
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		if (ret1.empty())
		{
			dbgAsgbe1(sString, 2, 0, "CDP C: Check CDP Schreiber!");
			continue;
		}
		sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.dev_id=" + ret1[0][0];
		sString += " AND interfaces.intfName LIKE '" + it->at(3) + "' ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}
		dbgAsgbe1(sString, 2, 1, "CDP C: Find cdp neighbors");
		std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

		// Mit Flag=3 und Prio=1 in Tabelle eintragen
		if (!ret2.empty())
		{
			std::string iString = "INSERT INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1,flag,priority) VALUES (";
			iString += it->at(1) + "," + it->at(0) + "," + ret2[0][0] + "," + ret1[0][0] + ",3,1);";
			dbgAsgbe1(iString, 2, 0, "CDP C: Find cdp neighbors");
			dieDB->query(iString.c_str());
		}
	}
	dbgNachbarAusgabe();
}


void WkmCombiner2::switchChecks()
{
	// Device IDs der Switches auslesen
	std::string sString = "SELECT dev_id FROM device WHERE hwtype=2 ";
	if (!nidEmpty)
	{
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 1, "Switch Dev IDs");

	std::vector<std::vector<std::string>> switches = dieDB->query(sString.c_str());

	// Dedizierte L2 Nachbarschaften auslesen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 1: L2\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	for(std::vector<std::vector<std::string>>::iterator it = switches.begin(); it < switches.end(); ++it)
	{
		// Alle Interfaces herausfinden, auf dem L2 Interface MAC Adressen von einem bestimmten Gerät gelernt werden
		sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
		sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
		sString += "WHERE neighbor.l2_addr IN ( ";
		sString += "SELECT macAddress FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND interfaces.phl=1) ";
		sString += "AND interfaces.phl=1 AND interfaces.status LIKE 'UP' AND device.hwtype=2 ";
		if (!nidEmpty)
		{
			sString += "AND device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 2, "Dedicated L2 neighborships");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			sString =  "SELECT DISTINCT intf_id,intfType FROM interfaces ";
// Änderung am 06.02.2013: Die folgenden beiden INNER JOIN Statements hinzugefügt
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "INNER JOIN device ON devInterface.device_dev_id=device.dev_id ";
//			sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
//			sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
			sString += "WHERE interfaces.macAddress IN ( ";
			sString += "SELECT l2_addr FROM neighbor ";
			sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
//			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces_intf_id ";
//			sString += "INNER JOIN device ON devInterface.device_dev_id=device.dev_id ";
			sString += "WHERE interfaces.phl=1 AND interfaces.intf_id=" + it2->at(0) + ") ";
// Änderung am 06.02.2013: +  "AND device.hwType=2" 
			sString += "AND interfaces.phl=1 AND interfaces.status LIKE 'UP' AND device.hwType=2 ";
			sString += "AND interfaces.intfType IN (SELECT intfType FROM interfaces WHERE intf_id="	+ it2->at(0) + ") ";
			if (!nidEmpty)
			{
				sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
			}

			dbgAsgbe1(sString, 2, 2, "Dedicated L2 neighborships");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			for(std::vector<std::vector<std::string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			{
				sString = "SELECT device_dev_id FROM devInterface WHERE interfaces_int_id=" + it3->at(0) + ";";

				dbgAsgbe1(sString, 2, 1, "Dedicated L2 neighborships");

				std::vector<std::vector<std::string>> ret3 = dieDB->query(sString.c_str());
				

				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + it3->at(0);
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + it3->at(0) + ");";

				dbgAsgbe1(sString, 2, 1, "Dedicated L2 neighborships");

				std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

				// Check, ob die Interfaces zusammenpassen (intfType)
				if (nid.empty())
				{
					std::string iString = "";
					if (ret2.size() > 1)
					{
						iString += "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, priority) ";
						iString += "VALUES (" + it2->at(0) + "," + it2->at(1) + "," + it3->at(0) + "," + ret3[0][0] + ", 2, 3);";
					}
					else
					{
						iString += "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
						iString += "VALUES (" + it2->at(0) + "," + it2->at(1) + "," + it3->at(0) + "," + ret3[0][0] + ", 3);";
					}

					dbgAsgbe1(iString, 2, 0, "Dedicated L2 neighborships");

					dieDB->query(iString.c_str());
				}
			}
		}
	}

	dbgNachbarAusgabe();	
	
	// STP Topologie #1
	//////////////////////////////////////////////////////////////////////////
	dbgAsgbe1("",3,0,"");
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 2: STP#1\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	for(std::vector<std::vector<std::string>>::iterator it = switches.begin(); it < switches.end(); ++it)
	{
		// 1) ALLE interface IDs von einer Box abfragen, 
		// bei denen die Designated Bridge ID nicht gleich einer eigenen MAC Adresse ist
		sString = "SELECT DISTINCT intf_id,intfType FROM interfaces ";
		sString += "INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "INNER JOIN int_vlan ON int_vlan.interfaces_intf_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND stp_status_stp_status_id IN (";
		sString += "SELECT DISTINCT stp_status_stp_status_id FROM int_vlan ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces_intf_id ";
		sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
		sString += "WHERE stp_status.statsOnly=0 AND devInterface.device_dev_id=" + it->at(0);
		sString += " AND stp_status.designatedBridgeID NOT IN (";
		sString += "SELECT macAddress FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + ")) ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 2, "STP Topology #1");

		std::vector<std::vector<std::string>> macIntf = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it2 = macIntf.begin(); it2 < macIntf.end(); ++it2)
		{
			// 2) Device ID der Designated Bridge IDs von 1) rausfinden
			sString = "SELECT DISTINCT device_dev_id FROM devInterface ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id ";
			sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
			sString += "WHERE interfaces.macAddress IN (";
			sString += "SELECT DISTINCT designatedBridgeID FROM stp_status ";
			sString += "INNER JOIN int_vlan ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
			sString += "WHERE int_vlan.interfaces_intf_id=" + it2->at(0) + " AND stp_status.designatedBridgeID NOT IN (";
			sString += "SELECT macAddress FROM interfaces ";
			sString += "INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
			sString += "WHERE devInterface.device_dev_id =" + it->at(0) + ")) ";
			//sString += "OR device.stpBridgeID IN (SELECT DISTINCT l2_addr FROM neighbor ";
			//sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			//sString += "INNER JOIN interfaces ON nlink.interfaces_intf_id=interfaces.intf_id ";
			//sString += "WHERE interfaces.intf_id=" + it2->at(0) + ");";
			if (!nidEmpty)
			{
				sString += "AND device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
			}

			dbgAsgbe1(sString, 2, 1, "STP Topology #1");

			std::vector<std::vector<std::string>> intfID = dieDB->query(sString.c_str());

			for(std::vector<std::vector<std::string>>::iterator it3 = intfID.begin(); it3 < intfID.end(); ++it3)
			{
				// 3) Schauen, ob MAC Adressen von der Ausgangsbox irgendwo auf einem L2 Interface 
				// auf der Box von 2) gelernt werden
				sString = "SELECT DISTINCT intf_id,intfType FROM interfaces ";
				sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				sString += "INNER JOIN neighbor ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
				sString += "WHERE devInterface.device_dev_id=" + it3->at(0) + " AND neighbor.l2_addr IN (";
				sString += "SELECT DISTINCT macAddress FROM interfaces ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND interfaces.l2l3 LIKE 'L2') AND neighbor.l3_addr LIKE '';";

				dbgAsgbe1(sString, 2, 2, "STP Topology #1");

				std::vector<std::vector<std::string>> intfID2 = dieDB->query(sString.c_str());

				for(std::vector<std::vector<std::string>>::iterator it4 = intfID2.begin(); it4 < intfID2.end(); ++it4)
				{
					std::string nIntf = "0";
					if (it2->at(1) == it4->at(1))
					{
						nIntf = it4->at(0);
					}


					// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
					sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + nIntf + " OR dI_intf_id=" + it2->at(0);
					sString += ") AND (dI_intf_id1=" + nIntf + " OR dI_intf_id1=" + it2->at(0) + ");";

					dbgAsgbe1(sString, 2, 1, "STP Topology #1");

					std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

					if (nid.empty())
					{
						if (nIntf == "0")
						{
							// Check, ob es einen ähnlichen Eintrag schon gibt, bei dem kein NULL vorkommt
							sString = "SELECT n_id FROM neighborship WHERE dI_dev_id=" + it3->at(0) + " AND (dI_dev_id1=" + it->at(0) + " AND dI_intf_id1=" + it2->at(0) + ");";
							sString += "OR (dI_dev_id=" + it->at(0) + " AND dI_dev_id1=" + it3->at(0) + " AND dI_intf_id=" + it2->at(0) + ");";

							dbgAsgbe1(sString, 2, 1, "STP Topology #1");

							nid = dieDB->query(sString.c_str());

							if (nid.empty())
							{
								std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
								iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nIntf + "," + it3->at(0) + ", 2);";
								dieDB->query(iString.c_str());
								dbgAsgbe1(iString, 2, 0, "STP Topology #1");
							}
						}
						else
						{
							std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nIntf + "," + it3->at(0) + ", 2);";
							dieDB->query(iString.c_str());

							dbgAsgbe1(iString, 2, 0, "STP Topology #1");

						}
					}
				}
			}
		}
	}
	dbgNachbarAusgabe();

	// STP Topologie #2 (Sandoz, RedBull)
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 3: STP#2\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	dbgAsgbe1("",3,0,"");
	for(std::vector<std::vector<std::string>>::iterator it = switches.begin(); it < switches.end(); ++it)
	{
		// Änderung 15.03.2013: Zusätzliche Positionen abgefragt
		// 1) ALLE Interfaces rausfinden, auf denen die Designated Bridge ID nicht gleich einer der eigenen MAC Adressen ist
		sString = "SELECT DISTINCT intf_id,designatedBridgeID,hostname,intfName,macAddress,dev_id FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "INNER JOIN stp_status ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
		sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
		sString += "WHERE statsOnly=0 AND devInterface.device_dev_id=" + it->at(0) + " ";
		sString += "AND l2l3 LIKE 'L2' ";
//		sString += "AND interfaces.channel_intf_id IS NULL ";
		sString += "AND intfType NOT LIKE 'Vlan' ";
		sString += "AND stp_status.designatedBridgeID NOT IN (";
		sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ") ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 6, "STP Topology #2");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			// Änderung 15.03.2013: Abfrage verschlankt
			// 2) Device ID für oben gefundene STP BridgeID rausfinden
			sString = "SELECT DISTINCT dev_id FROM device WHERE stpBridgeID LIKE '" + it2->at(1) + "' ";
			if (!nidEmpty)
			{
				sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
			}

			dbgAsgbe1(sString, 2, 1, "STP Topology #2");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			for(std::vector<std::vector<std::string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			{
				// 3) Gegenseiteeite finden
				sString = "SELECT DISTINCT intf_id,neighbor_id,l2_addr FROM interfaces ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
				sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
				sString += "WHERE devInterface.device_dev_id=" + it3->at(0) + " ";
				sString += "AND l2l3 LIKE 'L2' ";
//				sString += "AND interfaces.channel_intf_id IS NULL ";
				sString += "AND intfType NOT LIKE 'Vlan' ";
				sString += "AND neighbor.l2_addr IN (";
				sString += "SELECT macAddress FROM interfaces ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				// Änderung 17.03.2013: " + " AND intfName NOT LIKE 'mgmt%'" hinzugefügt, da Probleme bei N5k Nachbarschaften wenn Mgmt MAC gelernt wird
				sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND intfName NOT LIKE 'mgmt%') ";
				sString += "AND interfaces.intfType IN ( ";
				sString += "SELECT intfType FROM interfaces WHERE intf_id=" + it2->at(0) + ") ";
				if (!nidEmpty)
				{
					sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
				}

				//sString += "OR neighbor.l2_addr IN (";
				//sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ");";

				dbgAsgbe1(sString, 2, 3, "STP Topology #2");

				std::vector<std::vector<std::string>> ret3 = dieDB->query(sString.c_str());

				if (ret3.empty())
				{
					// 4) Wenn 3) nicht erfolgreich, dann STP Infos prüfen
					sString = "SELECT DISTINCT intf_id FROM interfaces "; 
					sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
					sString += "INNER JOIN stp_status ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
					sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
					sString += "WHERE statsOnly=0 AND devInterface.device_dev_id=" + it3->at(0) + " ";
					sString += "AND l2l3 LIKE 'L2' ";
//					sString += "AND interfaces.channel_intf_id IS NULL ";
					sString += "AND intfType NOT LIKE 'Vlan' ";
					sString += "AND stp_status.designatedBridgeID IN (";
					sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ") ";
					sString += "AND interfaces.intfType IN ( ";
					sString += "SELECT intfType FROM interfaces WHERE intf_id=" + it2->at(0) + ") ";
					if (!nidEmpty)
					{
						sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
					}

					dbgAsgbe1(sString, 2, 1, "STP Topology #2");

					ret3 = dieDB->query(sString.c_str());

				}

				std::string nintf = "0";
				if (!ret3.empty())
				{
					nintf = ret3[0][0];
				}
					
				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + nintf;
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + nintf + ");";

				dbgAsgbe1(sString, 2, 1, "STP Topology #2");

				std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

				if (nid.empty())
				{
					if (nintf == "0")
					{
						// Check, ob es einen ähnlichen Eintrag schon gibt, bei dem kein NULL vorkommt
						sString = "SELECT n_id FROM neighborship WHERE dI_dev_id=" + it3->at(0) + " AND (dI_dev_id1=" + it->at(0) + " AND dI_intf_id1=" + it2->at(0) + ") ";
						sString += "OR (dI_dev_id=" + it->at(0) + " AND dI_dev_id1=" + it3->at(0) + " AND dI_intf_id=" + it2->at(0) + ");";

						dbgAsgbe1(sString, 2, 1, "STP Topology #2");

						nid = dieDB->query(sString.c_str());

						if (nid.empty())
						{
							std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nintf + "," + it3->at(0) + ");";
							sString += "OR (dI_dev_id=" + it->at(0) + " AND dI_dev_id1=" + it3->at(0) + " AND dI_intf_id=" + it2->at(0) + ", 2);";

							dbgAsgbe1(iString, 2, 0, "STP Topology #2");

							dieDB->query(iString.c_str());

						}
					}
					else
					{
						// Check, ob es beide Intf/Dev Pärchen schon gibt
						sString = "SELECT n_id FROM neighborship ";
						sString += "WHERE (dI_intf_id=" + it2->at(0) + " AND dI_dev_id=" + it->at(0) + ") ";
						sString += "OR (dI_intf_id1=" + it2->at(0) + " AND dI_dev_id1=" + it->at(0) + ");";

						dbgAsgbe1(sString, 2, 1, "STP Topology #2");

						std::vector<std::vector<std::string>> nid2 = dieDB->query(sString.c_str());

						sString = "SELECT n_id FROM neighborship ";
						sString += "WHERE (dI_intf_id=" + nintf + " AND dI_dev_id=" + it3->at(0) + ") ";
						sString += "OR (dI_intf_id1=" + nintf + " AND dI_dev_id1=" + it3->at(0) + ");";
						
						dbgAsgbe1(sString, 2, 1, "STP Topology #2");

						std::vector<std::vector<std::string>> nid3 = dieDB->query(sString.c_str());

						if (nid2.empty() || nid3.empty())
						{
							std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nintf + "," + it3->at(0) + ", 2);";
							dieDB->query(iString.c_str());

							dbgAsgbe1(iString, 2, 0, "STP Topology #2");
						}
					}
				}
			}
		}
	}

	dbgNachbarAusgabe();
}


void WkmCombiner2::vpcCheck()
{
	// vPC Peer Link Nachbarschaften
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 4: L2 - vPC Peer Link\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	std::string sString = "SELECT intf_id,devInterface.device_dev_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "WHERE vpc_id IN (SELECT vpc_id FROM interfaces WHERE intfName LIKE 'vPC Peer-Link') AND intfName NOT LIKE 'vPC Peer-Link' ";
	if (!nidEmpty)
	{
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 2, "vPC Peer Link");

	std::vector<std::vector<std::string>> vpcInfo = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = vpcInfo.begin(); it < vpcInfo.end(); ++it)
	{
		sString = "SELECT DISTINCT intf_id,devInterface.device_dev_id FROM neighbor INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE l2_addr IN (SELECT DISTINCT macAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(1) + ") AND intf_id IN ";
		sString += "(SELECT intf_id FROM interfaces WHERE vpc_id IN (SELECT vpc_id FROM interfaces WHERE intfName LIKE 'vPC Peer-Link') AND intfName NOT LIKE 'vPC Peer-Link') ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 2, "vPC Peer Link");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
		
		for(std::vector<std::vector<std::string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			// Check, ob die vpc Domain ID zusammenpasst; Nur dann wird der Eintrag erfasst
			sString = "SELECT vpcDomId FROM device WHERE dev_id=" + it->at(1);
			dbgAsgbe1(sString, 2, 1, "vPC Doamin ID");
			std::vector<std::vector<std::string>> did1 = dieDB->query(sString.c_str());
			std::string s1 = did1.begin()->at(0);

			sString = "SELECT vpcDomId FROM device WHERE dev_id=" + it2->at(1);
			dbgAsgbe1(sString, 2, 1, "vPC Domain ID");
			std::vector<std::vector<std::string>> did2 = dieDB->query(sString.c_str());
			std::string s2 = did2.begin()->at(0);

			if (s1 == s2)
			{
				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it->at(0) + " OR dI_intf_id=" + it2->at(0);
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + it->at(0) + ");";
				std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

				dbgAsgbe1(sString, 2, 1, "vPC Peer Link");

				if (nid.empty())
				{
					std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
					iString += "VALUES (" + it->at(0) + "," + it->at(1) + "," + it2->at(0) + "," + it2->at(1) + ", 2);";
					dieDB->query(iString.c_str());

					dbgAsgbe1(iString, 2, 0, "vPC Peer Link");
				}
			}
		}
	}

	dbgNachbarAusgabe();
}


void WkmCombiner2::dedicatedL2()
{
	// NonSwitch Nachbarschaften auslesen (Bsp.: Router<->Router)
	//////////////////////////////////////////////////////////////////////////
	// Intf_id's auslesen
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 5: L2 - Non Switch\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	std::string sString = "SELECT intf_id,intfName,hostname,dev_id FROM interfaces ";
	sString += "INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	sString += "WHERE device.hwtype <>2 AND interfaces.phl=1 ";
	if (!nidEmpty)
	{
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 4, "Non-Switch L2");

	std::vector<std::vector<std::string>> nonSwitches = dieDB->query(sString.c_str());

	// Dedizierte L2 Nachbarschaften auslesen
	//////////////////////////////////////////////////////////////////////////
	for(std::vector<std::vector<std::string>>::iterator it = nonSwitches.begin(); it < nonSwitches.end(); ++it)
	{
		//sString = "SELECT interface_RH.intf_id FROM interfaces AS interface_LH INNER JOIN nlink ON nlink.interfaces_intf_id = interface_LH.intf_id ";
		//sString += "INNER JOIN neighbor ON nlink.neighbor_neighbor_id = neighbor.neighbor_id INNER JOIN interfaces AS interface_RH ON interface_RH.macAddress = neighbor.l2_addr ";
		//sString += "WHERE interface_LH.intf_id = " + it->at(0) + " AND interface_RH.status LIKE 'UP' AND interface_RH.phl = 1 AND interface_RH.intf_id <> " + it->at(0);
		//sString += " AND interface_RH.intfType = interface_LH.intfType ";
		
		sString = "SELECT intf_id FROM interfaces ";
		sString += "WHERE intfType IN (SELECT intfType FROM interfaces WHERE intf_id=" + it->at(0);
		sString += ") AND macAddress IN (SELECT l2_addr FROM neighbor ";
		sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
		sString += "WHERE interfaces.intf_id=" + it->at(0) + ") ";
		sString += "AND status LIKE 'UP' AND phl=1 AND intf_id <> " + it->at(0) + " ";
		if (!nidEmpty)
		{
			sString += "AND interface_LH.intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 1, "Non-Switch L2");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
		
		for(std::vector<std::vector<std::string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			sString = "SELECT DISTINCT device_dev_id FROM devInterface WHERE interfaces_int_id=" + it->at(0);
			std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

			dbgAsgbe1(sString, 2, 1, "Non-Switch L2");

			sString = "SELECT DISTINCT device_dev_id FROM devInterface WHERE interfaces_int_id=" + it2->at(0);
			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			dbgAsgbe1(sString, 2, 1, "Non-Switch L2");

			// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
			sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + ret2[0][0];
			sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + ret2[0][0] + ");";
			std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

			dbgAsgbe1(sString, 2, 1, "Non-Switch L2");

			if (nid.empty())
			{
				std::string intf1 = it->at(0);
				std::string intf2 = it2->at(0);

				// Check, ob Subinterface, oder nicht
				sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf1 + ";";
				std::vector<std::vector<std::string>> sid1 = dieDB->query(sString.c_str());

				dbgAsgbe1(sString, 2, 1, "Non-Switch L2");

				if (sid1[0][0] != "")
				{
					intf1 = sid1[0][0];
				}

				sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf2 + ";";
				std::vector<std::vector<std::string>> sid2 = dieDB->query(sString.c_str());

				dbgAsgbe1(sString, 2, 1, "Non-Switch L2");

				if (sid2[0][0] != "")
				{
					intf2 = sid2[0][0];
				}

				std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
				iString += "VALUES (" + intf1 + "," + ret1[0][0] + "," + intf2 + "," + ret2[0][0] + ", 3);";
				dieDB->query(iString.c_str());

				dbgAsgbe1(iString, 2, 0, "Non-Switch L2");

			}
		}
	}
	
	dbgNachbarAusgabe();
	// Dedizierte L2 Nachbarschaften auslesen #2
	//////////////////////////////////////////////////////////////////////////
	sString = "SELECT dev_id FROM device WHERE hwtype <> 2 AND dataSource IS NULL ";
	if (!nidEmpty)
	{
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	}
	
	nonSwitches = dieDB->query(sString.c_str());

	dbgAsgbe1(sString, 1, 1, "Non-Switch L2 #2");


	for(std::vector<std::vector<std::string>>::iterator it = nonSwitches.begin(); it < nonSwitches.end(); ++it)
	{
		// Alle Interfaces herausfinden, auf dem MAC Adressen von einem Nicht-Switch auf L2 Interfaces gelernt werden, die nicht schon eine Nachbarschaftsbeziehung haben
		sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface ";
		sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
		sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
		sString += "WHERE neighbor.l2_addr IN ( ";
		sString += "SELECT macAddress FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE device.dev_id=" + it->at(0) + ") ";
		sString += "AND interfaces.l2l3 LIKE 'L2' ";
		sString += "AND interfaces.intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship) ";
		sString += "AND interfaces.intf_id NOT IN (SELECT dI_intf_id FROM neighborship) ";
		if (!nidEmpty)
		{
			sString += "AND interfaces_int_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 2, "Non-Switch L2 #2");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
		
		for(std::vector<std::vector<std::string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			// Änderung 06.03.2013: "AND device.hwtype <> 2" hinzugefügt
			sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
			sString += "WHERE interfaces.macAddress IN ( ";
			sString += "SELECT l2_addr FROM neighbor ";
			sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
			sString += "WHERE interfaces.intf_id=" + it2->at(0) + ") ";
			sString += "AND interfaces.phl=1 AND device.hwtype <> 2 ";
			if (!nidEmpty)
			{
				sString += "AND interfaces_int_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
			}

			dbgAsgbe1(sString, 2, 2, "Non-Switch L2 #2");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			for(std::vector<std::vector<std::string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			{
				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + it3->at(0);
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + it3->at(0) + ");";

				dbgAsgbe1(sString, 2, 1, "Non-Switch L2 #2");

				std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

				if (nid.empty())
				{
					std::string intf1 = it2->at(0);
					std::string intf2 = it3->at(0);

					// Check, ob Subinterface, oder nicht
					sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf1 + ";";
					std::vector<std::vector<std::string>> sid1 = dieDB->query(sString.c_str());

					dbgAsgbe1(sString, 2, 1, "Non-Switch L2 #2");

					if (sid1[0][0] != "")
					{
						intf1 = sid1[0][0];
					}

					sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf2 + ";";
					std::vector<std::vector<std::string>> sid2 = dieDB->query(sString.c_str());

					dbgAsgbe1(sString, 2, 1, "Non-Switch L2 #2");

					if (sid2[0][0] != "")
					{
						intf2 = sid2[0][0];
					}

					std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
					iString += "VALUES (" + intf1 + "," + it2->at(1) + "," + intf2 + "," + it3->at(1) + ", 2);";
					dieDB->query(iString.c_str());

					dbgAsgbe1(iString, 2, 0, "Non-Switch L2 #2");

				}
			}
		}
	}			
	dbgNachbarAusgabe();
}


void WkmCombiner2::unknownSTP()
{
	// Alle Interfaces, auf denen unbekannte Switches hängen eintragen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 7: L2 - Unknown STP Neighbors\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	// 1) ALLE Interfaces rausfinden, auf denen die Designated Bridge ID nicht gleich einer der eigenen MAC Adressen ist
	std::string sString = "SELECT DISTINCT interfaces.intf_id,interfaces.intfName FROM interfaces ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id WHERE statsOnly=0 ";
	sString += "AND interfaces_intf_id NOT IN (SELECT dI_intf_id FROM neighborship) ";
	sString += "AND interfaces_intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship) ";
	if (!nidEmpty)
	{
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 2, "Unknown Switches");

	std::vector<std::vector<std::string>> unknownIntfs = dieDB->query(sString.c_str());
		
	for(std::vector<std::vector<std::string>>::iterator it = unknownIntfs.begin(); it < unknownIntfs.end(); ++it)
	{
		sString = "SELECT dev_id FROM device ";
		sString += "INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
		sString += "WHERE devInterface.interfaces_int_id=" + it->at(0) + ";";
		
		dbgAsgbe1(sString, 2, 1, "Unknown Switches");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
		
		std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, priority) ";
		iString += "VALUES (" + it->at(0) + "," + ret[0][0] + ", 0, 0, 9);";

		dbgAsgbe1(iString, 2, 0, "Unknown Switches");

		dieDB->query(iString.c_str());
	}

	dbgNachbarAusgabe();
}


void WkmCombiner2::setSwitchDevType()
{
	// L3 Switches suchen
	std::string sString = "SELECT dev_id FROM device ";
	sString += "INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
	sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "WHERE l2l3 LIKE 'L3' AND device.hwtype=2  ";
	sString += "GROUP BY dev_id ";
	sString += "HAVING (COUNT(dev_id)>1) ";
	if (!nidEmpty)
	{
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 1, "Set Switch Type");

	std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
	
	for(std::vector<std::vector<std::string>>::iterator it = ret1.begin(); it < ret1.end(); ++it)
	{
		std::string iString = "UPDATE OR REPLACE device SET type=8 WHERE dev_id=" + it->at(0) + ";";
		dieDB->query(iString.c_str());
		dbgAsgbe1(iString, 2, 0, "Set Switch Type");
	}


	//Root Bridges auslesen
	sString = "SELECT DISTINCT dev_id,type FROM device ";
	sString += "INNER JOIN vlan_has_device ON device.dev_id=vlan_has_device.device_dev_id ";
	sString += "INNER JOIN vlan ON vlan.vlan_id=vlan_has_device.vlan_vlan_id ";
	sString += "INNER JOIN vlan_stpInstanz ON vlan_stpInstanz.vlan_vlan_id=vlan.vlan_id ";
	sString += "INNER JOIN stpInstanz ON stpInstanz.stp_id=vlan_stpInstanz.stpInstanz_stp_id ";
	sString += "WHERE rootPort LIKE '' AND vlan.vlan>1 ";

	if (!nidEmpty)
	{
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 2, "Set Switch Type: RB");
	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
	
	if (ret.empty())
	{
		// Falls es nur Vlan 1 gibt, dann noch einmal abfragen
		sString = "SELECT DISTINCT dev_id,type FROM device ";
		sString += "INNER JOIN vlan_has_device ON device.dev_id=vlan_has_device.device_dev_id ";
		sString += "INNER JOIN vlan ON vlan.vlan_id=vlan_has_device.vlan_vlan_id ";
		sString += "INNER JOIN vlan_stpInstanz ON vlan_stpInstanz.vlan_vlan_id=vlan.vlan_id ";
		sString += "INNER JOIN stpInstanz ON stpInstanz.stp_id=vlan_stpInstanz.stpInstanz_stp_id ";
		sString += "WHERE rootPort LIKE ''";

		if (!nidEmpty)
		{
			sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		}
		ret = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 2, "Set Switch Type: RB");

	}
	
	dbgAsgbe1(sString, 1, 2, "Set Switch Type");

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		if (it->at(1) == "2")
		{
			std::string iString = "UPDATE OR REPLACE device SET type=7 WHERE dev_id=" + it->at(0) + ";";
			dbgAsgbe1(iString, 2, 0, "Set Switch Type");
			dieDB->query(iString.c_str());
		}
		else if (it->at(1) == "8")
		{
			std::string iString = "UPDATE OR REPLACE device SET type=9 WHERE dev_id=" + it->at(0) + ";";
			dbgAsgbe1(iString, 2, 0, "Set Switch Type");
			dieDB->query(iString.c_str());
		}
		else if (it->at(1) == "20")
		{
			std::string iString = "UPDATE OR REPLACE device SET type=22 WHERE dev_id=" + it->at(0) + ";";
			dbgAsgbe1(iString, 2, 0, "Set Switch Type");
			dieDB->query(iString.c_str());
		}
		else if (it->at(1) == "21")
		{
			std::string iString = "UPDATE OR REPLACE device SET type=23 WHERE dev_id=" + it->at(0) + ";";
			dbgAsgbe1(iString, 2, 0, "Set Switch Type");
			dieDB->query(iString.c_str());
		}
	}
}


void WkmCombiner2::insertRControl()
{
	// rControl aktualisieren
	std::string sString = "SELECT MAX(dev_id) FROM device";
	std::vector<std::vector<std::string> > rRes1 = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 1, "rControlUpdate");

	sString = "SELECT MAX(intf_id) FROM interfaces";
	std::vector<std::vector<std::string> > rRes2 = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 1, "rControlUpdate");

	sString = "UPDATE rControl SET dev_id=" + rRes1[0][0] + ",intf_id=" + rRes2[0][0] + " WHERE rc_id IN (SELECT MAX(rc_id) FROM rControl)";
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 0, "rControlUpdate");



	sString = "SELECT MAX(n_id) FROM neighborship;";
	
	dbgAsgbe1(sString, 1, 1, "insertRControl");

	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

	std::string nid = "";
	if (!result.empty())
	{
		nid = result[0][0];
	}

	sString = "SELECT MAX(rpn_id) FROM rpneighborship;";

	dbgAsgbe1(sString, 1, 1, "insertRControl");

	result = dieDB->query(sString.c_str());

	std::string rpnid = "";
	if (!result.empty())
	{
		rpnid = result[0][0];
		if (rpnid== "")
		{
			rpnid = "0";
		}
	}

	sString = "SELECT MAX(rc_id) FROM rControl;";
	dbgAsgbe1(sString, 1, 1, "insertRControl");

	result = dieDB->query(sString.c_str());

	std::string iString = "UPDATE rControl SET n_id=" + nid + " WHERE rc_id=" + result[0][0] + ";";
	dbgAsgbe1(iString, 2, 0, "insertRControl");
	dieDB->query(iString.c_str());
	iString = "UPDATE rControl SET rpn_id=" + rpnid + " WHERE rc_id=" + result[0][0] + ";";
	dbgAsgbe1(iString, 2, 0, "insertRControl");
	dieDB->query(iString.c_str());

	time_t zeit = time(NULL);
	char *zeitStempel = asctime(localtime(&zeit));
	zeitStempel[24] = 0x00;
	iString = "UPDATE rControl SET timestamp='";
	iString += zeitStempel;
	iString += "' WHERE rc_id=" + result[0][0] + ";";
	dieDB->query(iString.c_str());
	dbgAsgbe1(iString, 1, 0, "insertRControl");
}


void WkmCombiner2::dbgNachbarAusgabe()
{
	if (debugAusgabe)
	{
		std::string sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1,flag,priority FROM neighborship ";
		if (!nidEmpty)
		{
			sString += "WHERE dI_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		}

		std::vector<std::vector<std::string>> dbg = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator dbgIt = dbg.begin(); dbgIt < dbg.end(); ++dbgIt)
		{
			sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(0) + ";";
			std::vector<std::vector<std::string>> dbg1 = dieDB->query(sString.c_str());
			std::string h1 = dbg1[0][0];
			std::string h2;
			if (dbgIt->at(2) != "0")
			{
				sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(2) + ";";
				dbg1 = dieDB->query(sString.c_str());
				h2 = dbg1[0][0];
			}
			else
			{
				h2 = "UNKNOWN";
			}
			sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(1) + ";";
			dbg1 = dieDB->query(sString.c_str());
			std::string i1 = "UNKNOWN";
			if (!dbg1.empty())
			{
				i1 = dbg1[0][0];
			}
			std::string i2;
			if (dbgIt->at(3) != "0")
			{
				sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(3) + ";";
				dbg1 = dieDB->query(sString.c_str());
				i2 = dbg1[0][0];
			}
			else
			{
				i2 = "UNKNOWN";
			}
			std::string nachbaren = h1 + "-" + i1 + " <--> " + h2 + "-" + i2 + "   (Flag: " + dbgIt->at(4) + " // Priority: " + dbgIt->at(5) + ")\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);
		}
	}
}


void WkmCombiner2::dbgNachbarAusgabeRP()
{
	if (debugAusgabe)
	{
		std::string sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1,flag,rp FROM rpneighborship ";
		if (!nidEmpty)
		{
			sString += "WHERE dI_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		}

		std::vector<std::vector<std::string>> dbg = dieDB->query(sString.c_str());
		for (std::vector<std::vector<std::string>>::iterator dbgIt = dbg.begin(); dbgIt < dbg.end(); ++dbgIt)
		{
			sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(0) + ";";
			std::vector<std::vector<std::string>> dbg1 = dieDB->query(sString.c_str());
			std::string h1 = dbg1[0][0];
			std::string h2;
			if (dbgIt->at(2) != "0")
			{
				sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(2) + ";";
				dbg1 = dieDB->query(sString.c_str());
				h2 = dbg1[0][0];
			}
			else
			{
				h2 = "UNKNOWN";
			}
			sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(1) + ";";
			dbg1 = dieDB->query(sString.c_str());
			std::string i1 = "UNKNOWN";
			if (!dbg1.empty())
			{
				i1 = dbg1[0][0];
			}
			std::string i2;
			if (dbgIt->at(3) != "0")
			{
				sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(3) + ";";
				dbg1 = dieDB->query(sString.c_str());
				i2 = dbg1[0][0];
			}
			else
			{
				i2 = "UNKNOWN";
			}
			std::string nachbaren = h1 + "-" + i1 + " <--> " + h2 + "-" + i2 + "   (Flag: " + dbgIt->at(4) + " // RP: " + dbgIt->at(5) + ")\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);
		}
	}
}


void WkmCombiner2::dbgAsgbe1(std::string dbga, int type, int anzahlWerte, std::string position)
{
	// Type:
	// 1... Global
	// 2... Untergeordnet
	// 3... Untergeordnet, aber zurücksetzen -> Sonst nichts tun

	
	if (debugAusgabe1)
	{
		std::string qr1 = "";
		
		if (type == 3)
		{
			dbgUntergeordnetCounter = 0;
			dbgGlobalCounter++;
		}
		else
		{
			if (type == 1)
			{
				dbgUntergeordnetCounter = 0;
				dbgGlobalCounter++;
				qr1 = boost::lexical_cast<std::string>(dbgGlobalCounter);
			}
			else if (type == 2)
			{
				dbgUntergeordnetCounter++;
				qr1 = boost::lexical_cast<std::string>(dbgGlobalCounter) + "." + boost::lexical_cast<std::string>(dbgUntergeordnetCounter);
			}

			schreibeLog(WkLog::WkLog_ZEIT, "6701: " + position + ": " + qr1 + " Query: " + dbga + "\n", "6701", WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);

			if (anzahlWerte != 0)
			{
				std::vector<std::vector<std::string>> ergebnis = dieDB->query(dbga.c_str());

				for(std::vector<std::vector<std::string>>::iterator it = ergebnis.begin(); it < ergebnis.end(); ++it)
				{
					std::string werte = "";
					for (int i = 0; i < anzahlWerte; i++)
					{
						werte += it->at(i) + " | ";
					}

					schreibeLog(WkLog::WkLog_ZEIT, "6702: " + position + ": " + qr1 + " Result: " + werte, "6702", WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
				}
			}
		}
	}
}


void WkmCombiner2::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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


void WkmCombiner2::delDups()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Remove Duplicate Entries\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Doppelte Einträge löschen
	std::string sString = "DELETE FROM neighborship WHERE EXISTS ( ";
	sString += "SELECT 1 FROM neighborship N2 ";
	sString += "WHERE neighborship.dI_intf_id=N2.dI_intf_id  ";
	sString += "AND neighborship.dI_dev_id=N2.dI_dev_id  ";
	sString += "AND neighborship.dI_intf_id1=N2.dI_intf_id1 ";
	sString += "AND neighborship.dI_dev_id1=N2.dI_dev_id1  ";
	sString += "AND neighborship.n_id>N2.n_id ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl));";
	}
	else
	{
		sString += ");";
	}

	dieDB->query(sString.c_str());

	dbgAsgbe1(sString, 1, 0, "delDups");

	sString = "DELETE FROM neighborship WHERE EXISTS ( ";
	sString += "SELECT 1 FROM neighborship N2 ";
	sString += "WHERE neighborship.dI_intf_id=N2.dI_intf_id1  ";
	sString += "AND neighborship.dI_dev_id=N2.dI_dev_id1  ";
	sString += "AND neighborship.dI_intf_id1=N2.dI_intf_id ";
	sString += "AND neighborship.dI_dev_id1=N2.dI_dev_id  ";
	sString += "AND neighborship.n_id>N2.n_id ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl));";
	}
	else
	{
		sString += ");";
	}

	dieDB->query(sString.c_str());

	dbgAsgbe1(sString, 2, 0, "delDups");

	// Alle Nachbarschaften mit VSL Links löschen
	// Zuerst die Flag=2 Markierungen aufheben, für Nachbarschaften bei denen das Flag gesetzt ist, aber die doppelten Einträge VSL Links betreffen
	sString = "UPDATE neighborship SET flag=0 WHERE dI_intf_id IN (SELECT dI_intf_id FROM neighborship WHERE flag=2 AND dI_intf_id1 IN (SELECT intf_id FROM interfaces WHERE phl=2))";

	dieDB->query(sString.c_str());

	dbgAsgbe1(sString, 2, 0, "delDups VSL");
	
	sString = "DELETE FROM neighborship WHERE dI_intf_id IN (SELECT intf_id FROM interfaces WHERE phl=2) OR dI_intf_id1 IN (SELECT intf_id FROM interfaces WHERE phl=2)";

	dbgAsgbe1(sString, 2, 0, "delDups VSL");

	dieDB->query(sString.c_str());

	dbgNachbarAusgabe();
}


void WkmCombiner2::delDupsRP()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Remove Duplicate Entries - RP Neighborship Table\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Doppelte Einträge löschen
	std::string sString = "DELETE FROM rpneighborship WHERE EXISTS ( ";
	sString += "SELECT 1 FROM rpneighborship N2 ";
	sString += "WHERE rpneighborship.dI_intf_id=N2.dI_intf_id  ";
	sString += "AND rpneighborship.dI_dev_id=N2.dI_dev_id  ";
	sString += "AND rpneighborship.dI_intf_id1=N2.dI_intf_id1 ";
	sString += "AND rpneighborship.dI_dev_id1=N2.dI_dev_id1  ";
	sString += "AND rpneighborship.rpn_id>N2.rpn_id ";
	if (!nidEmpty)
	{
		sString += "AND rpn_id > (SELECT MAX(rpn_id) FROM rControl));";
	}
	else
	{
		sString += ");";
	}

	dieDB->query(sString.c_str());

	dbgAsgbe1(sString, 1, 0, "delDupsRP");

	sString = "DELETE FROM rpneighborship WHERE EXISTS ( ";
	sString += "SELECT 1 FROM rpneighborship N2 ";
	sString += "WHERE rpneighborship.dI_intf_id=N2.dI_intf_id1  ";
	sString += "AND rpneighborship.dI_dev_id=N2.dI_dev_id1  ";
	sString += "AND rpneighborship.dI_intf_id1=N2.dI_intf_id ";
	sString += "AND rpneighborship.dI_dev_id1=N2.dI_dev_id  ";
	sString += "AND rpneighborship.rpn_id>N2.rpn_id ";
	if (!nidEmpty)
	{
		sString += "AND rpn_id > (SELECT MAX(rpn_id) FROM rControl));";
	}
	else
	{
		sString += ");";
	}

	dieDB->query(sString.c_str());

	dbgAsgbe1(sString, 2, 0, "delDupsRP");

	dbgNachbarAusgabeRP();
}


void WkmCombiner2::markAnomaly()
{
	// Flags:
	// 0: Alles OK
	// 1: Eintrag muss kontrolliert werden, da ein Interface öfters verwendet wird
	// 2: Temporär; Wurde an anderer Stelle (doIt) gesetzt, wenn für ein Interface mehrere Nachbaren gefunden wurden; Wird hier am Schluss durch 1 ersetzt
	// 3: Soll vom 3way Check nicht erfasst werden
	// 99: soll nicht angezeigt werden

	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Anomaly Check\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Bei Loops auf sich selber auf das selbe Interface die Priorität erhöhen
	std::string sString = "UPDATE neighborship SET priority=99 WHERE n_id IN (SELECT n_id FROM neighborship WHERE dI_intf_id=dI_intf_id1)";

	dbgAsgbe1(sString, 1, 0, "Anomaly: Delete Lopps to same intf");

	dieDB->query(sString.c_str());

	// Nachbarschaften von Nexus1k auf Non-UCS Switches löschen und Nachbarschaften von UCS<->N1k mit flag=3 markieren, damit sie vom 2-Way Check nicht erfasst werden
	sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE intf_id IN ( ";
	sString += "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship UNION ALL SELECT dI_intf_id1 FROM neighborship)) AND device.type =21 OR device.type =23 ";
	if (!nidEmpty)
	{
		sString += "AND  intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}
	dbgAsgbe1(sString, 1, 1, "Anomaly: N1k Check");

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Als erstes muss für jedes Interface geschaut werden, dass es unter dI_intf_id eingeordnet ist. 
		// Falls dies nicht der Fall ist, muss die Nachbarschaft umgeschrieben werden. Das wird immer schrittweise gemacht, also immer dann, wenn ein Interface
		// an der Reihe ist, damit es zu keinen Überschneidungen kommt.
		sString = "SELECT n_id,dI_intf_id,dI_dev_id,dI_intf_id1,dI_dev_id1 FROM neighborship WHERE dI_intf_id1=" + it->at(0);
		dbgAsgbe1(sString, 2, 5, "Anomaly: N1k Check");

		std::vector<std::vector<std::string>> ret11 = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it11 = ret11.begin(); it11 < ret11.end(); ++it11)
		{
			sString = "UPDATE neighborship SET dI_intf_id=" + it11->at(3) + ",dI_intf_id1=" + it11->at(1) + ",dI_dev_id="; 
			sString += it11->at(4) + ",dI_dev_id1=" + it11->at(2) + " WHERE n_id=" + it11->at(0);
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "Anomaly: N1k Check");
		}

		// Dann die Einträge löschen, bei denen eine N1k<->HW Switch Nachbarschaft besteht
		sString = "DELETE FROM neighborship WHERE n_id IN (SELECT n_id FROM neighborship INNER JOIN devInterface ON dI_intf_id1=devInterface.interfaces_int_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE dI_intf_id=" + it->at(0) + " AND device.type <20)";
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: N1k Check");

		// Zum Schluss die Einträge für UCSC<->N1k markieren
		sString = "UPDATE neighborship SET flag=3 WHERE n_id IN (SELECT n_id FROM neighborship INNER JOIN devInterface ON dI_intf_id1=devInterface.interfaces_int_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE dI_intf_id=" + it->at(0) + " AND device.type>19)";
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: N1k Check");


	}
	dbgNachbarAusgabe();

	// Die Veth Interfaces den HW Interfaces zuordnen, falls relevant
	sString = "SELECT intf_id,boundTo_id FROM interfaces WHERE intf_id IN (SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1>0) AND boundTo_id NOT NULL ";
	if (!nidEmpty)
	{
		sString += "AND  intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	dbgAsgbe1(sString, 1, 2, "Anomaly: Veth to HW Intf");

	std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it2 = ret2.begin(); it2 < ret2.end(); ++it2)
	{
		sString = "UPDATE neighborship SET dI_intf_id=" + it2->at(1) + " WHERE dI_intf_id=" + it2->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Veth to HW Intf");

		sString = "UPDATE neighborship SET dI_intf_id1=" + it2->at(1) + " WHERE dI_intf_id1=" + it2->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Veth to HW Intf");
	}
	dbgNachbarAusgabe();
	
	// Die HW Interfaces, falls relevant, den Channels zuordnen
	sString = "SELECT intf_id,channel_intf_id FROM interfaces WHERE intf_id IN (SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1>0) AND channel_intf_id NOT NULL ";
	sString += "AND channel_intf_id NOT IN (SELECT intf_id FROM interfaces WHERE status NOT LIKE 'UP') ";
	if (!nidEmpty)
	{
		sString += "AND  intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	dbgAsgbe1(sString, 1, 2, "Anomaly: HW Intf to Channel");

	std::vector<std::vector<std::string>> ret3 = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it3 = ret3.begin(); it3 < ret3.end(); ++it3)
	{
		sString = "UPDATE neighborship SET dI_intf_id=" + it3->at(1) + " WHERE dI_intf_id=" + it3->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: HW Intf to Channel");

		sString = "UPDATE neighborship SET dI_intf_id1=" + it3->at(1) + " WHERE dI_intf_id1=" + it3->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: HW Intf to Channel");
	}
	dbgNachbarAusgabe();

	// Alle Nachbarschaften mit inaktiven Interfaces löschen
	sString = "DELETE FROM neighborship WHERE dI_intf_id1 IN (SELECT intf_id FROM interfaces WHERE status NOT LIKE 'UP' ";
	if (!nidEmpty)
	{
		sString += "AND  intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}
	sString += ")";
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Remove Intf down");

	sString = "DELETE FROM neighborship WHERE dI_intf_id IN (SELECT intf_id FROM interfaces WHERE status NOT LIKE 'UP' ";
	if (!nidEmpty)
	{
		sString += "AND  intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}
	sString += ")";
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Remove Intf down");
	dbgNachbarAusgabe();
	
	// Wenn es einen vollständigen Eintrag gibt und einen ähnlichen, bei dem auf einer Seite 0 steht, dann den Eintrag mit 0 löschen
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1 ";

	dbgAsgbe1(sString, 1, 1, "Anomaly: Remove half neighborship");

	std::vector<std::vector<std::string>> ret4 = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it4 = ret4.begin(); it4 < ret4.end(); ++it4)
	{
		sString = "DELETE FROM neighborship WHERE dI_intf_id=" + it4->at(0) + " AND dI_intf_id1=0;";
		std::vector<std::vector<std::string>> ret5 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Remove half neighborship");
	}
	dbgNachbarAusgabe();

	// Duplikate entfernen
	//////////////////////////////////////////////////////////////////////////
	delDups();

	//// Bei allen Einträgen Flag gleich 0 setzen, wenn NULL
	sString = "UPDATE neighborship SET flag=0 WHERE flag IS NULL";
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Set flag=0 where flag=NULL");

	// Bei allen Anomalien schauen, dass nur der Eintrag mit der höchsten Priorität behalten wird
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship WHERE dI_intf_id1 > 0 UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1 ";
	if (!nidEmpty)
	{
		sString += "AND  col > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	std::vector<std::vector<std::string>> ret6 = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 1, "Anomaly: Priority Check");


	for(std::vector<std::vector<std::string>>::iterator it6 = ret6.begin(); it6 < ret6.end(); ++it6)
	{
		// Als erstes muss für jedes Interface geschaut werden, dass es unter dI_intf_id eingeordnet ist. 
		// Falls dies nicht der Fall ist, muss die Nachbarschaft umgeschrieben werden. Das wird immer schrittweise gemacht, also immer dann, wenn ein Interface
		// an der Reihe ist, damit es zu keinen Überschneidungen kommt.
		sString = "SELECT n_id,dI_intf_id,dI_dev_id,dI_intf_id1,dI_dev_id1 FROM neighborship WHERE dI_intf_id1=" + it6->at(0);
		dbgAsgbe1(sString, 2, 5, "Anomaly: Priority Check");

		std::vector<std::vector<std::string>> ret11 = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it11 = ret11.begin(); it11 < ret11.end(); ++it11)
		{
			sString = "UPDATE neighborship SET dI_intf_id=" + it11->at(3) + ",dI_intf_id1=" + it11->at(1) + ",dI_dev_id="; 
			sString += it11->at(4) + ",dI_dev_id1=" + it11->at(2) + " WHERE n_id=" + it11->at(0);
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "Anomaly: Priority Check");
		}

		// Dann die eigentliche Überprüfung durchführen...		
		sString = "DELETE FROM neighborship WHERE n_id IN (SELECT n_id FROM neighborship WHERE n_id IN (SELECT n_id FROM neighborship WHERE dI_intf_id=" + it6->at(0);
		sString += " AND priority > (SELECT MIN(priority) FROM neighborship WHERE priority IN (SELECT priority FROM neighborship WHERE dI_intf_id=" + it6->at(0) + "))));";
		std::vector<std::vector<std::string>> ret7 = dieDB->query(sString.c_str());
		
		dbgAsgbe1(sString, 2, 1, "Anomaly: Priority Check");
		
	}
	dbgNachbarAusgabe();

	// STP Konstistenz Check
	stpKonistenzCheck();
	dbgNachbarAusgabe();

	// 3way Check
	dbgAsgbe1(sString, 1, 1, "Anomaly: 3-Way Check");

	// Szenario: Eine Anomalie, bei der beide Interfaces mindestens noch eine weitere Nachbarschaftsbeziehung haben.
	// In dem Fall wird die Anomalie gelöscht
	// Bsp:
	//  id  , i1    , h1 , i2    , h2 , f
	// "287","13594","91","12947","86","2"
	// "289","13594","91","13721","95","2"
	// "276","12947","86","12245","80",
	// -> 287 wird gelöscht
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship WHERE dI_intf_id1 > 0 AND flag <>3 UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 AND flag <>3) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1 ";
	if (!nidEmpty)
	{
		sString += "AND  col > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	std::vector<std::vector<std::string>> ret8 = dieDB->query(sString.c_str());
	std::vector<std::string> nRows;

	for(std::vector<std::vector<std::string>>::iterator it8 = ret8.begin(); it8 < ret8.end(); ++it8)
	{
		// Als erstes muss für jedes Interface geschaut werden, dass es unter dI_intf_id eingeordnet ist. 
		// Falls dies nicht der Fall ist, muss die Nachbarschaft umgeschrieben werden. Das wird immer schrittweise gemacht, also immer dann, wenn ein Interface
		// an der Reihe ist, damit es zu keinen Überschneidungen kommt.
		sString = "SELECT n_id,dI_intf_id,dI_dev_id,dI_intf_id1,dI_dev_id1 FROM neighborship WHERE dI_intf_id1=" + it8->at(0);
		dbgAsgbe1(sString, 2, 5, "Anomaly: 3-Way Check");

		std::vector<std::vector<std::string>> ret11 = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it11 = ret11.begin(); it11 < ret11.end(); ++it11)
		{
			sString = "UPDATE neighborship SET dI_intf_id=" + it11->at(3) + ",dI_intf_id1=" + it11->at(1) + ",dI_dev_id="; 
			sString += it11->at(4) + ",dI_dev_id1=" + it11->at(2) + " WHERE n_id=" + it11->at(0);
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "Anomaly: 3-Way Check");
		}

		// Dann die eigentliche Überprüfung durchführen...		
		//sString = "DELETE FROM neighborship WHERE n_id IN ( ";
		sString = "SELECT n_id FROM neighborship WHERE dI_intf_id=" + it8->at(0) + " AND dI_intf_id1 IN ( ";
		sString += "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
		sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 ";
		sString += ") T1 GROUP BY col HAVING COUNT(*) > 1);";
		std::vector<std::vector<std::string>> ret9 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 1, "Anomaly: 3-Way Check");
		
		// 28.03.2013: Check hinzugefügt
		// Noch einen Check machen -> Schauen, ob dI_dev_id==dI_dev_id1...		
		sString = "SELECT n_id FROM neighborship WHERE dI_intf_id=" + it8->at(0) + " AND dI_dev_id=dI_dev_id1 AND dI_intf_id1 IN ( ";
		sString += "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
		sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 ";
		sString += ") T1 GROUP BY col HAVING COUNT(*) > 1);";
		std::vector<std::vector<std::string>> ret111 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 1, "Anomaly: 3-Way Check");
		
		if (ret9.size() > ret11.size() > 0)
		{
			// Falls dI_dev_id==dI_dev_id1 und andere Einträge, dann die dI_dev_id==dI_dev_id1 Einträge löschen
			for(std::vector<std::vector<std::string>>::iterator it111 = ret111.begin(); it111 < ret111.end(); ++it111)
			{
				sString = "DELETE FROM neighborship WHERE n_id = " + it111->at(0);
				dieDB->query(sString.c_str());
				dbgAsgbe1(sString, 2, 0, "Anomaly: 3-Way Check");
			}
			// Und dann noch einmal Check
			sString = "SELECT n_id FROM neighborship WHERE dI_intf_id=" + it8->at(0) + " AND dI_intf_id1 IN ( ";
			sString += "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
			sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 ";
			sString += ") T1 GROUP BY col HAVING COUNT(*) > 1);";
			ret9 = dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 1, "Anomaly: 3-Way Check");
		}

		for(std::vector<std::vector<std::string>>::iterator it9 = ret9.begin(); it9 < ret9.end(); ++it9)
		{
			nRows.push_back(it9->at(0));
		}
	}
	// Änderung 04.03.: Delete ausgelagert:
	for (std::size_t i = 0; i < nRows.size(); i++)
	{
		sString = "DELETE FROM neighborship WHERE n_id = " + nRows[i];
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: 3-Way Check");
	}
	
	dbgNachbarAusgabe();

	// Endgeräte importieren
	//////////////////////////////////////////////////////////////////////////
	if (importHosts)
	{
		hostSchreiber();
		dbgNachbarAusgabe();
	}

	// Flag 3 löschen
	sString = "UPDATE neighborship SET flag=0 WHERE flag=3";
	ret = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Clear Flag=3");

	// Alle übriggebliebenen Anomalien markieren
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship WHERE flag < 2 ";
	sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 AND flag < 2) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1 ";
	if (!nidEmpty)
	{
		sString += "AND  col > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}
	
	dbgAsgbe1(sString, 1, 1, "Anomaly: Mark left");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "UPDATE neighborship SET flag=1 WHERE dI_intf_id1=" + it->at(0) + " OR dI_intf_id=" + it->at(0) + ";";
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Mark left");
	}

	// Alle Einträge mit Flag 2 auf Flag 1 ummarkieren
	sString = "UPDATE neighborship SET flag=1 WHERE flag=2";
	ret = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Set flag=1 where flag=2");

	// Markierung für vPC's aufheben
	sString = "SELECT n_id FROM neighborship ";
	sString += "WHERE (flag=1 AND dI_intf_id IN (SELECT intf_id FROM interfaces WHERE vpc_id NOT NULL)) ";
	sString += "OR (flag=1 AND dI_intf_id1 IN (SELECT intf_id FROM interfaces WHERE vpc_id NOT NULL)) ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}
	dbgAsgbe1(sString, 1, 1, "Anomaly: Remove vPC flags");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "UPDATE neighborship SET flag=0 WHERE n_id=" + it->at(0) + ";";
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Remove vPC flags");
	}


	// Alle markierten Einträge ein letztes Mal prüfen und im Fall updaten
	sString = "UPDATE neighborship SET flag=0 WHERE flag=1 AND dI_intf_id NOT IN (SELECT col FROM( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship WHERE flag=1 UNION ALL SELECT dI_intf_id1  AS col FROM neighborship WHERE flag=1) ";
	sString += "GROUP BY col HAVING COUNT(*)>1) AND dI_intf_id1 NOT IN (SELECT col FROM(SELECT dI_intf_id AS col FROM neighborship WHERE flag=1 ";
	sString += "UNION ALL SELECT dI_intf_id1  AS col FROM neighborship WHERE flag=1) GROUP BY col HAVING COUNT(*)>1) ";
	ret = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Last Check");

	dbgNachbarAusgabe();
}


void WkmCombiner2::stpKonistenzCheck()
{
	// Prüfen, ob es Einträge gibt, die kombiniert werden können, da gegengleiche "0" Einträge beim Interface bestehen
	std::string sString = "SELECT n_id,dI_intf_id FROM neighborship WHERE dI_intf_id1=0 ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}

	dbgAsgbe1(sString, 1, 2, "STP: Cross-0 Check");

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());


	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT n_id FROM neighborship WHERE (dI_dev_id=(SELECT dI_dev_id1 FROM neighborship WHERE n_id=" + it->at(0);
		sString += ") AND dI_dev_id1=(SELECT dI_dev_id FROM neighborship WHERE n_id=" + it->at(0) + ") AND dI_intf_id1=0);";

		dbgAsgbe1(sString, 2, 1, "Cross-0 Check");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		if (!ret1.empty())
		{
			sString = "SELECT n_id,dI_intf_id FROM neighborship ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE interfaces.intfType IN (SELECT intfType FROM interfaces WHERE intf_id=" + it->at(1) + ") ";
			sString += "AND neighborship.n_id IN (SELECT n_id FROM neighborship WHERE dI_intf_id1=0) ";
			if (!nidEmpty)
			{
				sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
			}

			dbgAsgbe1(sString, 2, 2, "STP: Cross-0 Check");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			if (ret2.size() == 2)
			{
				sString = "UPDATE neighborship SET dI_intf_id1=" + ret2[1][1] + " WHERE n_id=" + ret2[0][0] + ";";
				dieDB->query(sString.c_str());

				dbgAsgbe1(sString, 2, 0, "STP: Cross-0 Check");

				sString = "DELETE FROM neighborship WHERE n_id=" + ret2[1][0] + ";";
				dieDB->query(sString.c_str());
				dbgAsgbe1(sString, 2, 0, "STP: Cross-0 Check");

			}
		}
	}

	dbgNachbarAusgabe();

	// dI_intf_id1 auf 0 setzen, wenn es bereits einen Eintrag auf einen anderen Host gibt und wenn der STP Status von dem Interface auf Blocking ist:
	sString = "UPDATE OR IGNORE neighborship SET dI_intf_id1=0 WHERE n_id IN (";
	sString += "SELECT DISTINCT n_id FROM neighborship ";
	sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
	sString += "WHERE (dI_intf_id IN (SELECT dI_intf_id1 FROM neighborship) OR dI_intf_id1 IN (SELECT dI_intf_id FROM neighborship)) ";
	sString += "AND stp_status.stpIntfStatus LIKE 'alternate blocking%') AND priority>1";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}

	dbgAsgbe1(sString, 1, 0, "STP: STP Block -> Set dI_intf_id1=0");

	dieDB->query(sString.c_str());

	sString = "UPDATE OR IGNORE neighborship SET dI_intf_id=0 WHERE n_id IN (";
	sString += "SELECT DISTINCT n_id FROM neighborship ";
	sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id1 ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
	sString += "WHERE (dI_intf_id IN (SELECT dI_intf_id1 FROM neighborship) OR dI_intf_id1 IN (SELECT dI_intf_id FROM neighborship)) ";
	sString += "AND stp_status.stpIntfStatus LIKE 'alternate blocking%') AND priority>1";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "STP: STP Block -> Set dI_intf_id=0");

	dbgNachbarAusgabe();

	// Prüfen, ob es komplette Einträge und passende halbe Einträge gibt; Wenn ja, dann den halben Eintrag löschen
	sString = "SELECT n_id,dI_intf_id FROM neighborship WHERE dI_intf_id1=0 ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}

	dbgAsgbe1(sString, 1, 2, "STP: Cross-0 Check");

	ret = dieDB->query(sString.c_str());
	dbgAsgbe1("",3,0,"");
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT n_id FROM neighborship WHERE (dI_dev_id=(SELECT dI_dev_id1 FROM neighborship WHERE n_id=" + it->at(0);
		sString += ") AND dI_dev_id1=(SELECT dI_dev_id FROM neighborship WHERE n_id=" + it->at(0) + ") AND dI_intf_id1=" + it->at(1) + ");";

		dbgAsgbe1(sString, 2, 1, "STP: Delete half entries");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		if (!ret1.empty())
		{
			sString = "DELETE FROM neighborship WHERE n_id=" + it->at(0) + ";";
			dieDB->query(sString.c_str());

			dbgAsgbe1(sString, 2, 0, "STP: Delete half entries");
		}
	}
	
	dbgNachbarAusgabe();
}


void WkmCombiner2::hostSchreiber()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Import EndSystems\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);


	// Änderung 05.03.2013: "Zeile gelöscht: AND self=0 AND l2_addr NOT IN (SELECT DISTINCT macAddress FROM interfaces WHERE macAddress NOT LIKE '' AND phl<>3
	// Änderung 05.03.2013: "Zeile hinzugefügt: AND self=0 AND l3_addr NOT IN (SELECT DISTINCT ipAddress FROM interfaces WHERE ipAddress NOT LIKE ''
	std::string sString = "SELECT DISTINCT l2_addr,l3_addr FROM neighbor WHERE l3_addr NOT LIKE '' AND l2_addr NOT LIKE 'Incomplete%' ";
	sString += "AND self=0 AND l3_addr NOT IN (SELECT DISTINCT ipAddress FROM interfaces WHERE ipAddress NOT LIKE '' ";
	if (!nidEmpty)
	{
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}
	sString += ")";
	std::vector<std::vector<std::string>> esRep = dieDB->query(sString.c_str());
	
	dbgAsgbe1(sString, 1, 2, "hostSchreiber");
	
	for(std::vector<std::vector<std::string>>::iterator it = esRep.begin(); it < esRep.end(); ++it)
	{
		std::string sString1 = "SELECT intf_id,device.dev_id FROM interfaces INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
		sString1 += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
		sString1 += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString1 += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString1 += "WHERE neighbor.l2_addr LIKE '" + it->at(0) + "' "; // 18.11.2014: Entfernt: --->AND intf_id NOT IN (SELECT DISTINCT int_vlan.interfaces_intf_id FROM int_vlan) ";<---
		sString1 += "AND interfaces.l2l3 LIKE 'L2' ";

		sString1 += "AND intf_id NOT IN (SELECT dI_intf_id FROM neighborship WHERE dI_intf_id1 IN (SELECT dI_intf_id1 FROM neighborship ";
		sString1 += "INNER JOIN devInterface ON devInterface.interfaces_int_id=neighborship.dI_intf_id1 INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString1 += "WHERE device.hwtype=2)) AND intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id IN (SELECT dI_intf_id FROM neighborship ";
		sString1 += "INNER JOIN devInterface ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
		sString1 += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.hwtype=2)) ";

		if (!nidEmpty)
		{
			sString1 += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
		}

		std::vector<std::vector<std::string>> esRep1 = dieDB->query(sString1.c_str());
		dbgAsgbe1(sString1, 2, 2, "hostSchreiber");
		for(std::vector<std::vector<std::string>>::iterator it0 = esRep1.begin(); it0 < esRep1.end(); ++it0)
		{
			// Check ob Interface BoundTo_id eingetragen ist und ob das entsprechende Interface zu einer Nachbarschaft gehört
			sString = "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship UNION ALL SELECT dI_intf_id1 FROM neighborship) ";
			sString += "WHERE col IN (SELECT boundTo_id FROM interfaces WHERE intf_id=" + it0->at(0) + ")";
			std::vector<std::vector<std::string>> check = dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 1, "hostSchreiber");

			if (!check.empty())
			{
				continue;
			}

			// Check ob Interface channel_intf_id eingetragen ist und ob das entsprechende Interface zu einer Nachbarschaft gehört
			sString = "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship UNION ALL SELECT dI_intf_id1 FROM neighborship) ";
			sString += "WHERE col IN (SELECT channel_intf_id FROM interfaces WHERE intf_id=" + it0->at(0) + ")";
			check = dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 1, "hostSchreiber");
			if (!check.empty())
			{
				continue;
			}
			
			// DNS Reverse Lookup für den Endpoint
			// Wird dann als Hostname eingetragen
			std::string hostname = "";
			boost::system::error_code errorcode = boost::asio::error::host_not_found;
			boost::asio::ip::address_v4 ipa = boost::asio::ip::address_v4::from_string(it->at(1), errorcode);
			// Bei Fehler wird das ignoriert
			if (errorcode)
			{
				std::string dbgA = "\n6406 Parser Error: Host IP address " + it->at(1) + " is invalid. \r\nPlease contact wktools@spoerr.org.";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				continue;
			}
			else
			{
				if (reverseDns)
				{
					boost::asio::ip::tcp::endpoint ep;
					ep.address(ipa);
					boost::asio::io_service io_service;
					boost::asio::ip::tcp::resolver resolver(io_service);
					boost::asio::ip::tcp::resolver::iterator destination = resolver.resolve(ep);
					hostname =destination->host_name();
				}
				else
				{
					hostname = it->at(1);
				}
			}

			// MAC Vendor aus DB auslesen
			std::string vendorName = "";
			std::string oui = it->at(0).substr(0, 7);
			sString1 = "SELECT vendor FROM macOUI WHERE oui LIKE '" + oui + "'";
			dbgAsgbe1(sString1, 2, 1, "hostSchreiber");

			std::vector<std::vector<std::string>> vendor = dieDB->query(sString1.c_str());
			if (!vendor.empty())
			{
				vendorName = vendor[0][0];
			}
			else
			{
				vendorName = "UNKNOWN";
			}

			// Informationen in DB schreiben
			std::string dev_id = parserdb->insertDevice(hostname, "12", "12", "");
			parserdb->updateDeviceDataSource(dev_id, "2");
			parserdb->insertSnmp(vendorName, dev_id);

			std::string intf_id = parserdb->insertInterface("Port1", "", "", "1", it->at(0), it->at(1), "", "", "", "UP", "wktools generated interface", "L3", dev_id);

			parserdb->ipNetzeEintragen(it->at(1), "", intf_id);
			
			// Neighborship Tabelle befüllen
			std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, priority) ";
			iString += "VALUES (" + it0->at(0) + "," + it0->at(1) + "," + intf_id + "," + dev_id + ",0,99)";
			dieDB->query(iString.c_str());
			dbgAsgbe1(iString, 2, 0, "hostSchreiber");
		}
	}
	dbgNachbarAusgabe();
}


void WkmCombiner2::rControlUpdate()
{
	// rControl aktualisieren
	std::string sString = "SELECT MAX(dev_id) FROM device";
	std::vector<std::vector<std::string> > rRes1 = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 1, "rControlUpdate");

	sString = "SELECT MAX(intf_id) FROM interfaces";
	std::vector<std::vector<std::string> > rRes2 = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 1, "rControlUpdate");

	sString = "UPDATE rControl SET dev_id=" + rRes1[0][0] + ",intf_id=" + rRes2[0][0] + " WHERE rc_id IN (SELECT MAX(rc_id) FROM rControl)";
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 0, "rControlUpdate");
}


void WkmCombiner2::rpIpsecPeerCheck()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: L3 - rpIpsecPeerCheck\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Alle IPSec Crypto Map Nachbarschaftsbeziehungen finden und dann in die L3 Nachbarschaftstabelle einbauen
	//////////////////////////////////////////////////////////////////////////////////////////////
	// Als erstes alle lokalen Infos auslesen
	std::string sString = "SELECT DISTINCT intfName,intf_id,peer,channel_intf_id,hostname,dev_id FROM crypto INNER JOIN cryptolink ON cryptolink.crypto_crypto_id = crypto.crypto_id ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id = cryptolink.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id = devInterface.device_dev_id WHERE interfaces.intfName LIKE 'C-MAP%' ";
	if (!nidEmpty)
	{
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	}
	dbgAsgbe1(sString, 1, 6, "IPSec Neighbors");

	std::vector<std::vector<std::string>> result = dieDB->query(sString.c_str());

	for (std::vector<std::vector<std::string>>::iterator it = result.begin(); it < result.end(); ++it)
	{
		// Dann für jeden Eintrag den Peer finden
		sString = "SELECT hostname,dev_id,intf_id,intfName FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id = devInterface.device_dev_id WHERE ipAddress LIKE '" + it->at(2) + "' ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}
		dbgAsgbe1(sString, 2, 4, "IPSec Neighbors");
		std::vector<std::vector<std::string>> result1 = dieDB->query(sString.c_str());
		if (!result1.empty())
		{
			std::vector<std::vector<std::string>>::iterator it1 = result1.begin();
			// Die Nachbarschaft eintragen.
			std::string iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
			iString += "VALUES (" + it->at(1) + "," + it->at(5) + "," + it1->at(2) + "," + it1->at(1) + ", 0, 'CRYPTO');";
			dbgAsgbe1(iString, 2, 0, "IPSec Neighbors - Insert");
			dieDB->query(iString.c_str());
		}
	}
	delDupsRP();
}


void WkmCombiner2::rpNeighborCheck()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: L3 - rpNeighborCheck\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Alle RP Nachbarschaftsbeziehungen finden und dann in die L3 Nachbarschaftstabelle einbauen
	//////////////////////////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: RP Adjacencies\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	std::string sString = "SELECT interfaces.intf_id, devInterface.device_dev_id, rpNeighborAddress, rp FROM rpNeighbor ";
	sString += "INNER JOIN rpNeighborlink ON rpNeighborlink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id INNER JOIN interfaces ON interfaces.intf_id=rpNeighborlink.interfaces_intf_id ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	if (!nidEmpty)
	{
		sString += "WHERE rpNeighbor_id > (SELECT MAX(rpNeighbor_id) FROM rControl WHERE rpNeighbor_id < (SELECT MAX(rpNeighbor_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 4, "RP Neighbors");

	std::vector<std::vector<std::string>> rpn = dieDB->query(sString.c_str());

	for (std::vector<std::vector<std::string>>::iterator it = rpn.begin(); it < rpn.end(); ++it)
	{
		// Pro gefundenem Eintrag eine Abfrage auf die Nachbar Intf-ID und Device-ID anhand der Nachbar-IP
		sString =  "SELECT intf_id, devInterface.device_dev_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE ipAddress LIKE '" + it->at(2) + "' ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 2, "RP Neighbors");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

		if (!ret.empty())
		{
			std::string iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
			iString += "VALUES (" + it->at(0) + "," + it->at(1) + "," + ret[0][0] + "," + ret[0][1] + ", 0, '" + it->at(3) + "');";
			dbgAsgbe1(iString, 2, 0, "RP Neighbors");
			dieDB->query(iString.c_str());
		}
		else
		{
			// Unbekannte RP Nachbaren
			sString = "SELECT dev_id,devInterface.interfaces_int_id FROM device INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id WHERE hostname LIKE 'H-" + it->at(2) + "' ";
			if (!nidEmpty)
			{
				sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
			}
			dbgAsgbe1(sString, 2, 2, "RP Neighbors - dev_id/Unknown Neighbor");
			std::vector<std::vector<std::string>> rpuknown = dieDB->query(sString.c_str());

			if (!rpuknown.empty())
			{
				for (std::vector<std::vector<std::string>>::iterator it2 = rpuknown.begin(); it2 < rpuknown.end(); ++it2)
				{
					std::string iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
					iString += "VALUES (" + it->at(0) + "," + it->at(1) + "," + it2->at(1) + "," + it2->at(0) + ", 0, '" + it->at(3) + "');";
					dbgAsgbe1(iString, 2, 0, "RP Neighbors - dev_id/Unknown Neighbor - Insert");
					dieDB->query(iString.c_str());
				}
			}
			else
			{
				std::string iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
				iString += "VALUES (" + it->at(0) + "," + it->at(1) + ",0, 0, 0, '" + it->at(3) + "');";
				dbgAsgbe1(iString, 2, 0, "RP Neighbors - Unknown Neighbor");
				dieDB->query(iString.c_str());
			}
		}
	}

	dbgNachbarAusgabeRP();

	delDupsRP();

	// Unnumbered Interface auf einer Seite -> Umbauen auf richtiges Interfaces
	dbgAsgbe1(sString, 1, 1, "RP Neighbor: Unnumbered Intf: Normalizing");
	// Alle Interfaces, die mehr als eine Nachbarschaftsbeziehung haben auslesen
	sString = "SELECT col FROM(SELECT dI_intf_id AS col FROM rpneighborship WHERE dI_intf_id1 > 0 ";
	sString += "UNION ALL SELECT dI_intf_id1 FROM rpneighborship WHERE dI_intf_id1 > 0) T1 GROUP BY col HAVING COUNT(*) > 1 ";
	if (!nidEmpty)
	{
		sString += "AND  col > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
	for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Als erstes muss für jedes Interface geschaut werden, dass es unter dI_intf_id eingeordnet ist. 
		// Falls dies nicht der Fall ist, muss die Nachbarschaft umgeschrieben werden. Das wird immer schrittweise gemacht, also immer dann, wenn ein Interface
		// an der Reihe ist, damit es zu keinen Überschneidungen kommt.
		sString = "SELECT rpn_id,dI_intf_id,dI_dev_id,dI_intf_id1,dI_dev_id1 FROM rpneighborship WHERE dI_intf_id1=" + it->at(0);
		dbgAsgbe1(sString, 2, 5, "RP Neighbor: Unnumbered Intf: Normalizing Step2");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		for (std::vector<std::vector<std::string>>::iterator it1 = ret1.begin(); it1 < ret1.end(); ++it1)
		{
			sString = "UPDATE rpneighborship SET dI_intf_id=" + it1->at(3) + ",dI_intf_id1=" + it1->at(1) + ",dI_dev_id=";
			sString += it1->at(4) + ",dI_dev_id1=" + it1->at(2) + " WHERE rpn_id=" + it1->at(0);
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "RP Neighbor: Unnumbered Intf: Normalizing Update");
		}
		// Jetzt schauen, ob in der Nachbarschaft 2x der selbe Host vorkommt
		sString = "SELECT dI_dev_id1 FROM rpneighborship WHERE dI_intf_id=" + it->at(0) + " GROUP BY dI_dev_id1 HAVING COUNT(*) >1";
		dbgAsgbe1(sString, 2, 1, "RP Neighbor: Unnumbered Intf: Check 1");
		std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());
		
		for (std::vector<std::vector<std::string>>::iterator it2 = ret2.begin(); it2 < ret2.end(); ++it2)
		{
			sString = "SELECT intf_id, intfName, rpn_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "INNER JOIN rpneighborship ON rpneighborship.dI_intf_id1=devInterface.interfaces_int_id ";
			sString += "WHERE intf_id IN (SELECT dI_intf_id1 FROM rpneighborship WHERE dI_intf_id=";
			sString += it->at(0) + " AND dI_dev_id1=" + it2->at(0) + ") AND ipAddress NOT LIKE '%numbered%' AND dI_intf_id=" + it->at(0);
			dbgAsgbe1(sString, 2, 3, "RP Neighbor: Unnumbered Intf: SELECT for Delete Operation");
			std::vector<std::vector<std::string>> ret3 = dieDB->query(sString.c_str());
			for (std::vector<std::vector<std::string>>::iterator it3 = ret3.begin(); it3 < ret3.end(); ++it3)
			{
				sString = "DELETE FROM rpneighborship WHERE rpn_id=" + it3->at(2);
				dbgAsgbe1(sString, 2, 0, "RP Neighbor: Unnumbered Intf: Delete Operation");
				dieDB->query(sString.c_str());
			}
		}
	}
	dbgNachbarAusgabeRP();
}


void WkmCombiner2::rpCloudNeighborCheck()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: L3 - rpCloudNeighborCheck\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Alle Cloud Nachbarschaftsbeziehungen finden und dann in die L3 Nachbarschaftstabelle einbauen
	//////////////////////////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: RP Cloud Adjacencies\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	// Default Network als Host in die Device Tabelle eintragen
	std::string ndev_id = parserdb->unknownNetworkSchreiber("DefaultNetwork", "40");
	std::string nintf_id = parserdb->getIntfID(ndev_id, "DefaultNetwork", true);

	// Schritt 1: Default Routen
	// Alle L3 Geräte finden(Jedes Gerät mit mehr als einer IP Adresse) :
	std::string sString = "SELECT dev_id,hostname FROM device INNER JOIN devInterface ON device.dev_id = devInterface.device_dev_id ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id = devInterface.interfaces_int_id WHERE ipAddress NOT NULL AND ipAddress NOT LIKE '' AND status LIKE 'UP' ";
	if (!nidEmpty)
	{
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
	}
	sString += "GROUP BY dev_id	HAVING COUNT(dev_id)>1";

	dbgAsgbe1(sString, 1, 2, "RP Default Route Neighbors");

	std::vector<std::vector<std::string>> rpn = dieDB->query(sString.c_str());

	for (std::vector<std::vector<std::string>>::iterator it = rpn.begin(); it < rpn.end(); ++it)
	{
		// Für jedes Gerät unbekannte Next Hop IPs für die Default Route anzeigen
		// Für diese Hops dann Link in die Internet - Wolke -> Bekannter Host - Intf - Unbekanntes Intf - Unbekannter NH - Unbekanntes Intf - Wolke
		sString = "SELECT DISTINCT nextHop,intf_id,intfName,hostname FROM l3routes INNER JOIN rlink ON rlink.l3routes_l3r_id = l3routes.l3r_id INNER JOIN interfaces ON interfaces.intf_id = rlink.interfaces_intf_id ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE network LIKE '0.0.0.0' AND devInterface.device_dev_id = " + it->at(0);
		sString += " AND nextHop NOT IN(SELECT ipAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE ipAddress NOT NULL AND ipAddress NOT LIKE '' AND status LIKE 'UP' AND intfName NOT LIKE 'Unkwn-Intf' AND dataSource IS NULL) AND nextHop NOT IN(SELECT virtualIP FROM hsrp) ";

		dbgAsgbe1(sString, 2, 4, "RP Default Route Neighbors - Default Next Hops");
		std::vector<std::vector<std::string>> rpdg = dieDB->query(sString.c_str());

		for (std::vector<std::vector<std::string>>::iterator it1 = rpdg.begin(); it1 < rpdg.end(); ++it1)
		{
			sString = "SELECT dev_id,devInterface.interfaces_int_id FROM device INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id WHERE hostname LIKE 'H-" + it1->at(0) + "' ";
			if (!nidEmpty)
			{
				sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
			}
			dbgAsgbe1(sString, 2, 2, "RP Default Route Neighbors - dev_id/Unknown NH");
			std::vector<std::vector<std::string>> rpuknown = dieDB->query(sString.c_str());
			if (rpuknown.empty())
			{
				sString = "SELECT dev_id,intf_id FROM device INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id INNER JOIN interfaces ON interfaces.intf_id = devInterface.interfaces_int_id ";
				sString += "WHERE ipAddress LIKE '" + it1->at(0) + "' AND dataSource=1 ";
				if (!nidEmpty)
				{
					sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
				}
				dbgAsgbe1(sString, 2, 2, "RP Default Route Neighbors - RP NH-HOP==CDP?");
				rpuknown = dieDB->query(sString.c_str());
			}

			std::vector<std::vector<std::string>>::iterator it2 = rpuknown.begin();
			if (!rpuknown.empty())
			{
				// Default Gateway RP Nachbarschaftstabelle schreiben
				std::string iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
				iString += "VALUES (" + it1->at(1) + "," + it->at(0) + "," + it2->at(1) + "," + it2->at(0) + ", 0, '');";
				dbgAsgbe1(iString, 2, 0, "RP Default Route Neighbors - Insert Default Gateway");
				dieDB->query(iString.c_str());

				// Wolke in RP Nachbarschaftstabelle schreiben
				iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
				iString += "VALUES (0," + it2->at(0) + ",0," + ndev_id + ", 0, '');";
				dbgAsgbe1(iString, 2, 0, "RP Default Route Neighbors - Insert Default Network");
				dieDB->query(iString.c_str());
			}
		}
	}
	dbgNachbarAusgabeRP();

	// Schritt 2: Routen zu Netzen, die auf keinem anderen bekannten Router anliegen
	// Für jedes Gerät unbekannte Next - Hops rausfinden, die auf Netze anderer bekannter Router zeigen
	// Pro L3 Box schauen
	for (std::vector<std::vector<std::string>>::iterator it = rpn.begin(); it < rpn.end(); ++it)
	{
		// Schritt 2.1: Unbekannte Next Hops für diese Box rausfinden

		sString = "SELECT DISTINCT nextHop,intf_id,intfName,hostname FROM l3routes INNER JOIN rlink ON rlink.l3routes_l3r_id = l3routes.l3r_id INNER JOIN interfaces ON interfaces.intf_id = rlink.interfaces_intf_id ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE devInterface.device_dev_id = " + it->at(0) + " AND nextHop NOT IN(";
		sString += "SELECT ipAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE ipAddress NOT NULL AND ipAddress NOT LIKE '' AND status LIKE 'UP' AND intfName NOT LIKE 'Unkwn-Intf' AND dataSource IS NULL) AND nextHop NOT IN(SELECT virtualIP FROM hsrp) ";
		dbgAsgbe1(sString, 2, 4, "RP Unknown Network Neighbors - Next Hops");

		std::vector<std::vector<std::string>> rpun = dieDB->query(sString.c_str());
		for (std::vector<std::vector<std::string>>::iterator it1 = rpun.begin(); it1 < rpun.end(); ++it1)
		{
			// Check, ob unbekannter NH bereits mit Wolke verbunden ist
			// Zuerst alle bekannten Wolken (Typ 40 oder 41) durchsuchen und dann prüfen, ob es mit der Wolke und dem Unknown Gateway bereits eine Nachbarschaftsbeziehung gibt
			// Wenn ja, dann continue; Wenn nein, dann mit Wolke verbinden und weiter mit den Tests

			sString = "SELECT dev_id from device WHERE type=40 OR type=41 ";
			if (!nidEmpty)
			{
				sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
			}
			dbgAsgbe1(sString, 2, 1, "RP Unknown Route Neighbors - Available Clouds");
			std::vector<std::vector<std::string>> rpcloud = dieDB->query(sString.c_str());

			sString = "SELECT dev_id,devInterface.interfaces_int_id FROM device INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id WHERE hostname LIKE 'H-" + it1->at(0) + "' ";
			if (!nidEmpty)
			{
				sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
			}
			dbgAsgbe1(sString, 2, 2, "RP Unknown Route Neighbors - dev_id/Unknown NH");

			std::vector<std::vector<std::string>> rpuknown = dieDB->query(sString.c_str());
			if (rpuknown.empty())
			{
				// Check ob bekannter CDP Nachbar
				sString = "SELECT dev_id,intf_id,hostname,intfName FROM device INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id INNER JOIN interfaces ON interfaces.intf_id = devInterface.interfaces_int_id ";
				sString += "WHERE ipAddress LIKE '" + it1->at(0) + "' AND dataSource=1 ";
				if (!nidEmpty)
				{
					sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
				}
				dbgAsgbe1(sString, 2, 4, "RP Default Route Neighbors - RP NH-HOP==CDP? #2");
				rpuknown = dieDB->query(sString.c_str());
			}

			bool cont = false;	// for-Durchlauf überspringen bei true
			for (std::vector<std::vector<std::string>>::iterator it3 = rpcloud.begin(); it3 < rpcloud.end(); ++it3)
			{
				if (!rpuknown.empty())
				{
					sString = "SELECT rpn_id FROM rpneighborship WHERE dI_dev_id=" + rpuknown[0][0] + " AND dI_dev_id1=" + it3->at(0);
					dbgAsgbe1(sString, 2, 1, "RP Unknown Route Neighbors - dev_id/Unknown NH");
					std::vector<std::vector<std::string>> rpcneighbor = dieDB->query(sString.c_str());
					if (!rpcneighbor.empty())
					{
						cont = true;
					}
				}
				else
				{
					cont = true;
				}
			}

			if (cont == true)
			{
				continue;
			}

			// Wenn der Nachbar noch an keiner Wolke hängt, dann ist auch die Wolke noch nicht vorhanden Daher:
			// Neue Wolke einfügen
			std::string cdev_id = parserdb->unknownNetworkSchreiber("NetworkCloud", "41");
			std::string cintf_id = parserdb->getIntfID(cdev_id, "NetworkCloud", true);

			// Lokale Box mit Unbekannten NH verbinden
			if (!rpuknown.empty())
			{
				std::vector<std::vector<std::string>>::iterator it2 = rpuknown.begin();
				// Unknown NH in RP Nachbarschaftstabelle schreiben
				std::string iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
				iString += "VALUES (" + it1->at(1) + "," + it->at(0) + "," + it2->at(1) + "," + it2->at(0) + ", 0, '');";
				dbgAsgbe1(iString, 2, 0, "RP Unknown Route Neighbors - Insert Unknown Gateway");
				dieDB->query(iString.c_str());

				// Wolke in RP Nachbarschaftstabelle schreiben
				iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
				iString += "VALUES (0," + it2->at(0) + ",0," + cdev_id + ", 0, '');";
				dbgAsgbe1(iString, 2, 0, "RP Unknown Route Neighbors - Insert Unknown Network");
				dieDB->query(iString.c_str());
			}

			// Alle Router finden, die Netze direkt kennen, die auf dem evaluierten Router nur per unbekannten NH erreichbar sind
			sString = "SELECT dev_id, hostname FROM device INNER JOIN devInterface ON device.dev_id = devInterface.device_dev_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id = devInterface.interfaces_int_id INNER JOIN intfSubnet ON intfSubnet.interfaces_intf_id = interfaces.intf_id ";
			sString += "INNER JOIN ipSubnet ON ipSubnet.ipSubnet_id = intfSubnet.ipSubnet_ipSubnet_id WHERE ipAddress NOT NULL AND ipAddress NOT LIKE '' AND status LIKE 'UP' ";
			sString += "AND ipSubnet.ulSubnet IN(SELECT ulSubnet FROM l3routes WHERE nextHop LIKE '" + it1->at(0) + "') AND device.hwType NOT NULL ";
			sString += "GROUP BY dev_id HAVING COUNT(dev_id)>1 ";
			dbgAsgbe1(sString, 2, 2, "RP Unknown Network Neighbors - Remote Router");
			std::vector<std::vector<std::string>> rpRemoteRouter = dieDB->query(sString.c_str());

			// Alle lokalen Netze auslesen
			// Wird für die Funktion nicht benötigt, lassen wir aber so
			sString = "SELECT ulSubnet, ulMask, ulBroadcast, subnet, mask FROM ipSubnet	INNER JOIN intfSubnet ON intfSubnet.ipSubnet_ipSubnet_id = ipSubnet.ipSubnet_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id = intfSubnet.interfaces_intf_id	INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
			sString += "WHERE devInterface.device_dev_id = " + it->at(0) + "GROUP BY ulSubnet, ulBroadcast";
			dbgAsgbe1(sString, 2, 5, "RP Unknown Network Neighbors - Local Subnets");
			std::vector<std::vector<std::string>> rploc = dieDB->query(sString.c_str());

			for (std::vector<std::vector<std::string>>::iterator it4 = rpRemoteRouter.begin(); it4 < rpRemoteRouter.end(); ++it4)
			{
				// Für jeden Router prüfen, ob er Netze per unbekannten NH kennt, die am evaluierten Router anliegen
				// Wenn es solche Vorkommnisse gibt, dann den Router in dieselbe Wolke verbinden
				sString = "SELECT nextHop,intf_id FROM l3routes INNER JOIN rlink ON rlink.l3routes_l3r_id = l3routes.l3r_id INNER JOIN interfaces ON interfaces.intf_id = rlink.interfaces_intf_id ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id WHERE devInterface.device_dev_id=" + it4->at(0) + " AND ulSubnet IN(";
				sString += "SELECT ulSubnet FROM ipSubnet INNER JOIN intfSubnet ON intfSubnet.ipSubnet_ipSubnet_id = ipSubnet.ipSubnet_id ";
				sString += "INNER JOIN interfaces ON interfaces.intf_id = intfSubnet.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
				sString += "WHERE devInterface.device_dev_id=" + it->at(0) + ") AND nextHop NOT IN ";
				sString += "(SELECT ipAddress FROM interfaces WHERE ipAddress NOT NULL AND ipAddress NOT LIKE '' AND status LIKE 'UP' AND intfName NOT LIKE 'Unkwn-Intf')";
				dbgAsgbe1(sString, 2, 1, "RP Unknown Network Neighbors - Remote NH");
				std::vector<std::vector<std::string>> rpremnh = dieDB->query(sString.c_str());

				for (std::vector<std::vector<std::string>>::iterator it5 = rpremnh.begin(); it5 < rpremnh.end(); ++it5)
				{
					sString = "SELECT dev_id,devInterface.interfaces_int_id FROM device INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id WHERE hostname LIKE 'H-" + it5->at(0) + "' ";
					if (!nidEmpty)
					{
						sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
					}
					dbgAsgbe1(sString, 2, 2, "RP Unknown Route Neighbors - dev_id/Unknown NH #2");

					std::vector<std::vector<std::string>> rpuknown2 = dieDB->query(sString.c_str());
					if (rpuknown2.empty())
					{
						// Check ob bekannter CDP Nachbar
						sString = "SELECT dev_id,intf_id FROM device INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id INNER JOIN interfaces ON interfaces.intf_id = devInterface.interfaces_int_id ";
						sString += "WHERE ipAddress LIKE '" + it5->at(0) + "' AND dataSource=1 ";
						if (!nidEmpty)
						{
							sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) ";
						}
						dbgAsgbe1(sString, 2, 2, "RP Default Route Neighbors - RP NH-HOP==CDP? #2");
						rpuknown2 = dieDB->query(sString.c_str());
					}


					if (!rpuknown2.empty())
					{
						dbgAsgbe1("NOT EMPTY", 2, 0, "RP Unknown Route Neighbors - Insert Unknown Gateway NOT Empty");
						std::vector<std::vector<std::string>>::iterator it6 = rpuknown2.begin();
						// Unknown NH in RP Nachbarschaftstabelle schreiben
						std::string iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
						iString += "VALUES (" + it5->at(1) + "," + it4->at(0) + "," + it6->at(1) + "," + it6->at(0) + ", 0, '');";
						dbgAsgbe1(iString, 2, 0, "RP Unknown Route Neighbors - Insert Unknown Gateway #2");
						dieDB->query(iString.c_str());

						// Wolke in RP Nachbarschaftstabelle schreiben
						iString = "INSERT OR IGNORE INTO rpneighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag, rp) ";
						iString += "VALUES (0," + it6->at(0) + ",0," + cdev_id + ", 0, '');";
						dbgAsgbe1(iString, 2, 0, "RP Unknown Route Neighbors - Insert Unknown Network #2");
						dieDB->query(iString.c_str());
					}
				}
			}
		}
	}
	delDupsRP();
}



#endif //#ifdef WKTOOLS_MAPPER
