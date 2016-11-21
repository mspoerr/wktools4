
#include "class_wkmCombiner.h"
#ifdef WKTOOLS_MAPPER

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>    


WkmCombiner::WkmCombiner(WkmDB *db, WkLog *logA, bool ih, bool rd)
{
	dieDB = db;
	logAusgabe = logA;
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


WkmCombiner::~WkmCombiner()
{

}


bool WkmCombiner::doIt()
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

	// Device IDs der Switches auslesen
	sString = "SELECT dev_id FROM device WHERE hwtype=2 ";
	if (!nidEmpty)
	{
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 1, "Switch Dev IDs");

	std::vector<std::vector<std::string>> switches = dieDB->query(sString.c_str());

	if (stop)
	{
		return false;
	}
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
//			sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
//			sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
			sString += "WHERE interfaces.macAddress IN ( ";
			sString += "SELECT l2_addr FROM neighbor ";
			sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
//			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces_intf_id ";
//			sString += "INNER JOIN device ON devInterface.device_dev_id=device.dev_id ";
			sString += "WHERE interfaces.phl=1 AND interfaces.intf_id=" + it2->at(0) + ") ";
			sString += "AND interfaces.phl=1 AND interfaces.status LIKE 'UP' ";
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
						iString += "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag) ";
						iString += "VALUES (" + it2->at(0) + "," + it2->at(1) + "," + it3->at(0) + "," + ret3[0][0] + ", 2);";
					}
					else
					{
						iString += "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
						iString += "VALUES (" + it2->at(0) + "," + it2->at(1) + "," + it3->at(0) + "," + ret3[0][0] + ");";
					}

					dbgAsgbe1(iString, 2, 0, "Dedicated L2 neighborships");

					dieDB->query(iString.c_str());
				}
			}
		}
	}

	dbgNachbarAusgabe();	
	
	if (stop)
	{
		return false;
	}

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
		sString += "WHERE devInterface.device_dev_id=" + it->at(0);
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
				sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND interfaces.l2l3 LIKE 'L2') AND neighbor.l3_addr IS NULL;";

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
								std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
								iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nIntf + "," + it3->at(0) + ");";
								dieDB->query(iString.c_str());
								dbgAsgbe1(iString, 2, 0, "STP Topology #1");
							}
						}
						else
						{
							std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nIntf + "," + it3->at(0) + ");";
							dieDB->query(iString.c_str());

							dbgAsgbe1(iString, 2, 0, "STP Topology #1");

						}
					}
				}
			}
		}
	}
	dbgNachbarAusgabe();

	if (stop)
	{
		return false;
	}
	// STP Topologie #2 (Sandoz, RedBull)
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 3: STP#2\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	dbgAsgbe1("",3,0,"");
	for(std::vector<std::vector<std::string>>::iterator it = switches.begin(); it < switches.end(); ++it)
	{
		// 1) ALLE Interfaces rausfinden, auf denen die Designated Bridge ID nicht gleich einer der eigenen MAC Adressen ist
		sString = "SELECT DISTINCT intf_id FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN stp_status ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
		sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " ";
		sString += "AND l2l3 LIKE 'L2' ";
//		sString += "AND interfaces.channel_intf_id IS NULL ";
		sString += "AND intfType NOT LIKE 'Vlan' ";
		sString += "AND stp_status.designatedBridgeID NOT IN (";
		sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ") ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 1, "STP Topology #2");

		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			// 2) Device ID zum Interface von 1) rausfinden
			sString = "SELECT DISTINCT dev_id FROM device WHERE stpBridgeID IN (";
			sString += "SELECT DISTINCT designatedBridgeID FROM stp_status ";
			sString += "INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
			sString += "WHERE int_vlan.interfaces_intf_id=" + it2->at(0) + " ";
			sString += "AND designatedBridgeID NOT IN ( ";
			sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ")) ";
			if (!nidEmpty)
			{
				sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
			}

			dbgAsgbe1(sString, 2, 1, "STP Topology #2");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			for(std::vector<std::vector<std::string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			{
				// 3) Gegenseiteeite finden
				sString = "SELECT DISTINCT intf_id FROM interfaces ";
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
				sString += "WHERE devInterface.device_dev_id=" + it->at(0) + ") ";
				sString += "AND interfaces.intfType IN ( ";
				sString += "SELECT intfType FROM interfaces WHERE intf_id=" + it2->at(0) + ") ";
				if (!nidEmpty)
				{
					sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
				}

				//sString += "OR neighbor.l2_addr IN (";
				//sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ");";

				dbgAsgbe1(sString, 2, 1, "STP Topology #2");

				std::vector<std::vector<std::string>> ret3 = dieDB->query(sString.c_str());

				if (ret3.empty())
				{
					// 4) Wenn 3) nicht erfolgreich, dann STP Infos prüfen
					sString = "SELECT DISTINCT intf_id FROM interfaces "; 
					sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
					sString += "INNER JOIN stp_status ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
					sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
					sString += "WHERE devInterface.device_dev_id=" + it3->at(0) + " ";
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
							std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nintf + "," + it3->at(0) + ");";
							sString += "OR (dI_dev_id=" + it->at(0) + " AND dI_dev_id1=" + it3->at(0) + " AND dI_intf_id=" + it2->at(0) + ");";

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
							std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nintf + "," + it3->at(0) + ");";
							dieDB->query(iString.c_str());

							dbgAsgbe1(iString, 2, 0, "STP Topology #2");
						}
					}
				}
			}
		}
	}

	dbgNachbarAusgabe();

	if (stop)
	{
		return false;
	}
	// vPC Peer Link Nachbarschaften
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 4: L2 - vPC Peer Link\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	sString = "SELECT intf_id,devInterface.device_dev_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
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
					std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
					iString += "VALUES (" + it->at(0) + "," + it->at(1) + "," + it2->at(0) + "," + it2->at(1) + ");";
					dieDB->query(iString.c_str());

					dbgAsgbe1(iString, 2, 0, "vPC Peer Link");
				}
			}
		}
	}

	dbgNachbarAusgabe();

	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - STP Check\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	stpKonistenzCheck();

	dbgNachbarAusgabe();


	if (stop)
	{
		return false;
	}
	// NonSwitch Nachbarschaften auslesen (Bsp.: Router<->Router)
	//////////////////////////////////////////////////////////////////////////
	// Intf_id's auslesen
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 5: L2 - Non Switch\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	sString = "SELECT intf_id FROM interfaces ";
	sString += "INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	sString += "WHERE device.hwtype <>2 AND interfaces.phl=1 ";
	if (!nidEmpty)
	{
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	}

	dbgAsgbe1(sString, 1, 1, "Non-Switch L2");

	std::vector<std::vector<std::string>> nonSwitches = dieDB->query(sString.c_str());

	if (stop)
	{
		return false;
	}
	// Dedizierte L2 Nachbarschaften auslesen
	//////////////////////////////////////////////////////////////////////////
	for(std::vector<std::vector<std::string>>::iterator it = nonSwitches.begin(); it < nonSwitches.end(); ++it)
	{
		sString = "SELECT intf_id FROM interfaces ";
		sString += "WHERE macAddress IN ( ";
		sString += "SELECT l2_addr FROM neighbor ";
		sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
		sString += "WHERE interfaces.intf_id=" + it->at(0) + ") ";
		sString += "AND status LIKE 'UP' AND phl=1 AND intf_id <> " + it->at(0) + " ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
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

				std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
				iString += "VALUES (" + intf1 + "," + ret1[0][0] + "," + intf2 + "," + ret2[0][0] + ");";
				dieDB->query(iString.c_str());

				dbgAsgbe1(iString, 2, 0, "Non-Switch L2");

			}
		}
	}
	
	dbgNachbarAusgabe();

	if (stop)
	{
		return false;
	}
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
			sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE interfaces.macAddress IN ( ";
			sString += "SELECT l2_addr FROM neighbor ";
			sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
			sString += "WHERE interfaces.intf_id=" + it2->at(0) + ") ";
			sString += "AND interfaces.phl=1 ";
			if (!nidEmpty)
			{
				sString += "AND interfaces_int_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
			}

			dbgAsgbe1(sString, 2, 2, "Non-Switch L2 #2");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			// Falls für ein Interface mehrere Nachbaren gefunden wurde, wird geprüft, 
			// ob die MAC Adresse der gefundenen Interfaces irgendwo in der neighbor Tabelle für das Nachbar Gerät gedunden wird. 
			// Falls nicht, wird das Interface aus dem Ergebnis gelöscht
			// Falls alle Interfaces gelöscht werden sollten, dann nichts löschen
			//if (ret2.size() > 1)
			//{
			//	std::vector<std::string> intfIDs;
			//	for(std::vector<std::vector<std::string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			//	{
			//		sString = "SELECT DISTINCT intf_id,intfName FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			//		sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
			//		sString += "WHERE l2_addr IN (SELECT macAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			//		sString += "WHERE devInterface.device_dev_id=" + it2->at(1) + ") AND interfaces.intf_id=" + it3->at(0);

			//		if (debugAusgabe1)
			//		{
			//			schreibeLog(WkLog::WkLog_ZEIT, "6701: 5.2 Query 3.1: " + sString + "\n", "6701", WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
			//		}

			//		std::vector<std::vector<std::string>> nid = dieDB->query(sString.c_str());

			//		if (debugAusgabe1)
			//		{
			//			for(std::vector<std::vector<std::string>>::iterator dbgit = nid.begin(); dbgit < nid.end(); ++dbgit)
			//			{
			//				schreibeLog(WkLog::WkLog_ZEIT, "6702: 5.2 Result 3.1: " + dbgit->at(0), "6702", WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
			//			}
			//		}
			//		
			//		if (!nid.empty())
			//		{
			//			for(std::vector<std::vector<std::string>>::iterator it4 = nid.begin(); it4 < nid.end(); ++it4)
			//			{
			//				intfIDs.push_back(it4->at(0));
			//			}
			//		}
			//	}

			//	// Interfaces aus Result 2 löschen, die nicht in intfIDs vorkommen:
			//	if (!intfIDs.empty())
			//	{
			//		for(std::vector<std::vector<std::string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			//		{
			//			bool loeschen = true;
			//			for (int i=0; i < intfIDs.size(); i++)
			//			{
			//				if (intfIDs[i]==it3->at(0))
			//				{
			//					loeschen = false;
			//				}
			//			}
			//			
			//			if (loeschen)
			//			{
			//				ret2.erase(it3);
			//				it3 = ret2.begin();
			//			}
			//		}
			//	}
			//}


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

					std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
					iString += "VALUES (" + intf1 + "," + it2->at(1) + "," + intf2 + "," + it3->at(1) + ");";
					dieDB->query(iString.c_str());

					dbgAsgbe1(iString, 2, 0, "Non-Switch L2 #2");

				}
			}
		}
	}			
	dbgNachbarAusgabe();

	if (stop)
	{
		return false;
	}
	// CDP Checks
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 6: CDP Check\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	cdpChecks();
	dbgNachbarAusgabe();

	if (stop)
	{
		return false;
	}
	// Alle Interfaces, auf denen unbekannte Switches hängen eintragen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 7: L2 - Unknown STP Neighbors\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	// 1) ALLE Interfaces rausfinden, auf denen die Designated Bridge ID nicht gleich einer der eigenen MAC Adressen ist
	sString = "SELECT DISTINCT interfaces.intf_id,interfaces.intfName FROM interfaces ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "WHERE interfaces_intf_id NOT IN ( ";
	sString += "SELECT dI_intf_id FROM neighborship) ";
	sString += "AND interfaces_intf_id NOT IN ( ";
	sString += "SELECT dI_intf_id1 FROM neighborship) ";
//	sString += "AND interfaces.channel_intf_id IS NULL ";
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
		
		std::string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
		iString += "VALUES (" + it->at(0) + "," + ret[0][0] + ", 0, 0);";

		dbgAsgbe1(iString, 2, 0, "Unknown Switches");

		dieDB->query(iString.c_str());
	}

	dbgNachbarAusgabe();
	
	if (stop)
	{
		return false;
	}
	// Duplikate entfernen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 8: Remove Duplicate Entries\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	delDups();

	dbgNachbarAusgabe();

	if (stop)
	{
		return false;
	}
	// Abschließender Anomalie Check und Engeräte Import
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 9: Anomaly Check\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	markAnomaly();


	
	dbgNachbarAusgabe();

	// L3 Checks
	//////////////////////////////////////////////////////////////////////////
	if (stop)
	{
		return false;
	}

	// Switch Dev Typen setzen
	//////////////////////////////////////////////////////////////////////////
	setSwitchDevType();
	if (stop)
	{
		return false;
	}

	// rControl befüllen
	//////////////////////////////////////////////////////////////////////////
	insertRControl();

}


void WkmCombiner::stpKonistenzCheck()
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

	if (debugAusgabe)
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "stpKonistenzCheck - 1\r\n", "", WkLog::WkLog_ROT);
		dbgNachbarAusgabe();
	}

	// dI_intf_id1 auf 0 setzen, wenn es bereits einen Eintrag auf einen anderen Host gibt und wenn der STP Status von dem Interface auf Blocking ist:
	sString = "UPDATE OR IGNORE neighborship SET dI_intf_id1=0 WHERE n_id IN (";
	sString += "SELECT DISTINCT n_id FROM neighborship ";
	sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
	sString += "WHERE (dI_intf_id IN (SELECT dI_intf_id1 FROM neighborship) OR dI_intf_id1 IN (SELECT dI_intf_id FROM neighborship)) ";
	sString += "AND stp_status.stpIntfStatus LIKE 'alternate blocking ') ";
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
	sString += "AND stp_status.stpIntfStatus LIKE 'alternate blocking ') ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "STP: STP Block -> Set dI_intf_id=0");

	if (debugAusgabe)
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "stpKonistenzCheck - 2\r\n", "", WkLog::WkLog_ROT);
		dbgNachbarAusgabe();
	}


	// Schauen, ob beim Nachbaren nur ein Interface vom selben Typ vorkommt und im Fall ein Update
	sString = "SELECT n_id, dI_intf_id,dI_dev_id1 FROM neighborship WHERE dI_intf_id1=0 ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}
	ret = dieDB->query(sString.c_str());

	dbgAsgbe1(sString, 1, 3, "STP: Same Intf Type Check");

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT intf_id FROM devInterface "; 
		sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(2) + " ";
		sString += "AND intfType IN ( ";
		sString += "SELECT intfType FROM interfaces WHERE intf_id=" + it->at(1) + ") ";
		sString += "AND intf_id NOT IN (SELECT dI_intf_id FROM neighborship) ";
		sString += "AND intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship) ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 1, "STP: Same Intf Type Check");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		if (ret1.size() == 1)
		{
			sString = "UPDATE neighborship SET dI_intf_id1=" + ret1[0][0] + " WHERE n_id=" + it->at(0) + ";";
			dieDB->query(sString.c_str());
			
			dbgAsgbe1(sString, 2, 0, "STP: Same Intf Type Check");
		}
	}

	if (debugAusgabe)
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "stpKonistenzCheck - 3\r\n", "", WkLog::WkLog_ROT);
		dbgNachbarAusgabe();
	}


	// Prüfen #2, ob es Einträge gibt, die kombiniert werden können, da gegengleiche "0" Einträge beim Interface bestehen
	sString = "SELECT n_id,dI_intf_id FROM neighborship WHERE dI_intf_id1=0 ";
	if (!nidEmpty)
	{
		sString += "AND n_id > (SELECT MAX(n_id) FROM rControl);";
	}
	dbgAsgbe1(sString, 1, 2, "STP: Cross-0 Check #2");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT n_id FROM neighborship WHERE (dI_dev_id=(SELECT dI_dev_id1 FROM neighborship WHERE n_id=" + it->at(0);
		sString += ") AND dI_dev_id1=(SELECT dI_dev_id FROM neighborship WHERE n_id=" + it->at(0) + ") AND dI_intf_id1=0);";

		dbgAsgbe1(sString, 2, 1, "STP: Cross-0 Check #2");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		if (!ret1.empty())
		{
			sString = "SELECT n_id,dI_intf_id FROM neighborship ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE interfaces.intfType IN (SELECT intfType FROM interfaces WHERE intf_id=" + it->at(1) + ") ";
			sString += "AND neighborship.n_id IN (SELECT n_id FROM neighborship WHERE dI_intf_id1=0);"; 

			dbgAsgbe1(sString, 2, 2, "STP: Cross-0 Check #2");

			std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());

			if (ret2.size() == 2)
			{
				sString = "UPDATE neighborship SET dI_intf_id1=" + ret2[1][1] + " WHERE n_id=" + ret2[0][0] + ";";
				dieDB->query(sString.c_str());

				dbgAsgbe1(sString, 2, 0, "STP: Cross-0 Check #2");

				sString = "DELETE FROM neighborship WHERE n_id=" + ret2[1][0] + ";";
				dieDB->query(sString.c_str());

				dbgAsgbe1(sString, 2, 0, "STP: Cross-0 Check #2");
			}
		}
	}

	if (debugAusgabe)
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "stpKonistenzCheck - 4\r\n", "", WkLog::WkLog_ROT);
		dbgNachbarAusgabe();
	}


	// Prüfen, ob es komplette Einträge und passende halbe Einträge gibt; Wenn ja, dann den halben Eintrag löschen
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

}


void WkmCombiner::delDups()
{
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
}


void WkmCombiner::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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

void WkmCombiner::anomalyCdp()
{
	// Schritt 1: Alle Anomalien finden
	//////////////////////////////////////////////////////////////////////////
	std::string sString = "SELECT col FROM ( SELECT dI_intf_id AS col FROM neighborship WHERE flag < 2 ";
	sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 AND flag < 2) T1 GROUP BY col HAVING COUNT(*) > 1 ";
	if (!nidEmpty)
	{
		sString += "AND  col > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}


	dbgAsgbe1(sString, 1, 1, "CDP A:Find Anomalies");

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

	// Für alle Anomalien prüfen, ob es passende CDP Neighbor Einträge gibt
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// CDP Eintrag finden
		sString = "SELECT interfaces_int_id, device.dev_id, device.type FROM devInterface ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE interfaces.intfName IN ( ";
		sString += "SELECT nIntf FROM cdp WHERE cdp_id IN ( ";
		sString += "SELECT cdp_cdp_id FROM clink WHERE interfaces_intf_id=" + it->at(0) + ")) ";
		sString += "AND device.hostname IN ( ";
		sString += "SELECT nName FROM cdp WHERE cdp_id IN ( ";
		sString += "SELECT cdp_cdp_id FROM clink WHERE interfaces_intf_id=" + it->at(0) + "))";
		if (!nidEmpty)
		{
			sString += " AND device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl))";
		}

		dbgAsgbe1(sString, 2, 3, "CDP A:Find Anomalies, Find CDP");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it1 = ret1.begin();

		if (!ret1.empty())
		{
			// Wenn CDP Neighbor Eintrag gefunden, dann schauen, ob die anderen Anomalien Endgeräte sind. Wenn ja, dann auf den cdp Neighbor umhängen
			bool endSysCheck = false;
			std::string intfDesc = "";
			if (it1->at(2) == "4")				// AP
			{
				endSysCheck = true;
				intfDesc = "%adio%";

			}
			else if (it1->at(2) == "11")		// IP Phone
			{
				endSysCheck = true;
				intfDesc = "PC Port";
			}
			else if (it1->at(2) == "2")			// Switch
			{
				endSysCheck = true;
				intfDesc = "DummyPort";
			}

			if (endSysCheck)
			{
				sString = "SELECT intf_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				sString += "WHERE description LIKE '" + intfDesc + "' AND devInterface.device_dev_id=" + it1->at(1);

				dbgAsgbe1(sString, 2, 1, "CDP A:Find Anomalies, EndSystem Check");

				std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());
				std::vector<std::vector<std::string>>::iterator it2 = ret2.begin();
				
				if (!ret2.empty())
				{
					// Alle Nachbarschaftsbeziehungen von Endgeräten auf das neue Interface umhängen
					sString = "UPDATE OR REPLACE neighborship SET dI_intf_id=" + it2->at(0) + ",dI_dev_id=" + it1->at(1) + " WHERE dI_dev_id1 IN (";
					sString +="SELECT DISTINCT dI_dev_id1 FROM neighborship INNER JOIN devInterface ON devInterface.device_dev_id=neighborship.dI_dev_id1 ";
					sString +="INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE dataSource=2 AND dI_intf_id=" + it->at(0) + ")";
					dieDB->query(sString.c_str());
					dbgAsgbe1(sString, 2, 0, "CDP A:Find Anomalies, EndSystem neighborship change");
				}
			}
			
			
			// Zum Schluss die falschen Einträge löschen und den cdp Eintrag belassen
			sString = "DELETE FROM neighborship WHERE dI_intf_id=" + it->at(0) + " AND dI_intf_id1 != " + it1->at(0) + "";
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "CDP A:Find Anomalies, Delete");

			sString = "DELETE FROM neighborship WHERE dI_intf_id1=" + it->at(0) + " AND dI_intf_id != " + it1->at(0) + "";
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "CDP A:Find Anomalies, Delete");
		}
	}

	// Schritt 2: Alle Nachbarschaften mit dI_intf_id1 gleich 0 untersuchen
	// Wenn es solche Einträge gibt, dann schauen, ob es für das dI_intf_id einen cdp Nachbaren gibt. Wenn ja, dann den Eintrag entsprechend updaten
	//////////////////////////////////////////////////////////////////////////
	sString = "SELECT dI_intf_id FROM neighborship WHERE dI_intf_id1=0";

	if (!nidEmpty)
	{
		sString += " AND dI_intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	dbgAsgbe1(sString, 1, 1, "CDP A:Check dI_intf_id1=0");

	ret = dieDB->query(sString.c_str());

	// Für alle Anomalien prüfen, ob es passende CDP Neighbor Einträge gibt
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT interfaces_int_id, device.dev_id FROM devInterface ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE interfaces.intfName IN ( ";
		sString += "SELECT nIntf FROM cdp WHERE cdp_id IN ( ";
		sString += "SELECT cdp_cdp_id FROM clink WHERE interfaces_intf_id=" + it->at(0) + ")) ";
		sString += "AND device.hostname IN ( ";
		sString += "SELECT nName FROM cdp WHERE cdp_id IN ( ";
		sString += "SELECT cdp_cdp_id FROM clink WHERE interfaces_intf_id=" + it->at(0) + ")) ";
		if (!nidEmpty)
		{
			sString += "AND device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl))";
		}

		dbgAsgbe1(sString, 2, 2, "CDP A:Check dI_intf_id1=0");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		if (!ret1.empty())
		{
			// Wenn CDP Neighbor Einträge gefunden wurden, den Eintrag komplettieren
			sString = "UPDATE neighborship SET dI_intf_id1=" + ret1[0][0] + ",dI_dev_id1=" + ret1[0][1] + " WHERE dI_intf_id=" + it->at(0) + ";";
			
			dbgAsgbe1(sString, 2, 0, "CDP A:Check dI_intf_id1=0");
			dieDB->query(sString.c_str());
		}

	}

	// Schritt 3: Neue Duplikate löschen
	//////////////////////////////////////////////////////////////////////////
	delDups();

	// Schritt 4: Flag 3 löschen
	sString = "UPDATE neighborship SET flag=0 WHERE flag=3";
	ret = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "CDP A:Clear Flag=3");
}


void WkmCombiner::markAnomaly()
{
	// Flags:
	// 0: Alles OK
	// 1: Eintrag muss kontrolliert werden, da ein Interface öfters verwendet wird
	// 2: Temporär; Wurde an anderer Stelle (doIt) gesetzt, wenn für ein Interface mehrere Nachbaren gefunden wurden; Wird hier am Schluss durch 1 ersetzt
	// 3: Temporär; Wird bei cdpChecks gesetzt, um die cdp Nachbaren anzuzeigen, damit sie bei anomalyCDP ausgelassen werden können; Wird nach anomalyCDP zurückgesetzt
	// 99: soll nicht angezeigt werden
	

	// Schritt 0: Loops auf sich selber auf das selbe Interface löschen
	std::string sString = "DELETE FROM neighborship WHERE n_id IN (SELECT n_id FROM neighborship WHERE dI_intf_id=dI_intf_id1)";

	dbgAsgbe1(sString, 1, 0, "Anomaly: Delete Lopps to same intf");

	dieDB->query(sString.c_str());

	// Schritt 0.2: Die Veth Interfaces den HW Interfaces zuordnen, falls relevant
	sString = "SELECT intf_id,boundTo_id FROM interfaces WHERE intf_id IN (SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1>0) AND boundTo_id NOT NULL ";
	if (!nidEmpty)
	{
		sString += "AND  intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	dbgAsgbe1(sString, 1, 2, "Anomaly: Veth to HW Intf");

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "UPDATE neighborship SET dI_intf_id=" + it->at(1) + " WHERE dI_intf_id=" + it->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Veth to HW Intf");

		sString = "UPDATE neighborship SET dI_intf_id1=" + it->at(1) + " WHERE dI_intf_id1=" + it->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Veth to HW Intf");
	}
	
	// Schritt 0.3: Die HW Interfaces, falls relevant, den Channels zuordnen
	sString = "SELECT intf_id,channel_intf_id FROM interfaces WHERE intf_id IN (SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1>0) AND channel_intf_id NOT NULL ";
	if (!nidEmpty)
	{
		sString += "AND  intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	dbgAsgbe1(sString, 1, 2, "Anomaly: HW Intf to Channel");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "UPDATE neighborship SET dI_intf_id=" + it->at(1) + " WHERE dI_intf_id=" + it->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: HW Intf to Channel");

		sString = "UPDATE neighborship SET dI_intf_id1=" + it->at(1) + " WHERE dI_intf_id1=" + it->at(0);
		dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: HW Intf to Channel");
	}


	// Schritt 0.5: Alle Nachbarschaften mit inaktiven Interfaces löschen
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
	
	// Schritt 1: Wenn es einen vollständigen Eintrag gibt und einen ähnlichen, bei dem auf einer Seite 0 steht, dann den Eintrag mit 0 löschen
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1 ";

	dbgAsgbe1(sString, 1, 1, "Anomaly: Remove half neighborship");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "DELETE FROM neighborship WHERE dI_intf_id=" + it->at(0) + " AND dI_intf_id1=0;";
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: Remove half neighborship");
	}
	dbgNachbarAusgabe();

	// Schritt 2: Einträge mit Flag 2 bearbeiten; Wird bei Schritt 1 von doIt gesetzt, wenn es auf einem Interface mehrere Nachbar Interfaces gibt
	sString = "SELECT dI_intf_id1 FROM neighborship WHERE flag=2 GROUP BY  dI_intf_id1 HAVING COUNT(*) > 1;";
	dbgAsgbe1(sString, 1, 1, "Anomlay: Check Flag=2");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Markierung löschen, wenn Flag gesetzt, aber Interface von vorher nicht vorkommt
		sString = "UPDATE neighborship SET flag=NULL WHERE flag=2 AND dI_intf_id1 NOT LIKE " + it->at(0) + ";";
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomlay: Check Flag=2");

		// Einträge, bei denen das gefundene Interface vorkommt, löschen
		sString  = "DELETE FROM neighborship WHERE flag=2 AND dI_intf_id1= " + it->at(0) + ";";
		ret1 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomlay: Check Flag=2");
	}
	dbgNachbarAusgabe();

	// Duplikate entfernen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 8 Again: Remove Duplicate Entries\n", "6620",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	delDups();

	dbgNachbarAusgabe();

	// Bei allen Einträge Flag gleich 0 setzen, wenn NULL
	sString = "UPDATE neighborship SET flag=0 WHERE flag IS NULL";
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Set flag=0 where flag=NULL");

	// Schritt 3: 
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

	dbgAsgbe1(sString, 1, 0, "Anomaly: 3-Way Check");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		// Als erstes muss für jedes Interface geschaut werden, dass es unter dI_intf_id eingeordnet ist. 
		// Falls dies nicht der Fall ist, muss die Nachbarschaft umgeschrieben werden. Das wird immer schrittweise gemacht, also immer dann, wenn ein Interface
		// an der Reihe ist, damit es zu keinen Überschneidungen kommt.
		sString = "SELECT n_id,dI_intf_id,dI_dev_id,dI_intf_id1,dI_dev_id1 FROM neighborship WHERE dI_intf_id1=" + it->at(0);
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
		sString = "DELETE FROM neighborship WHERE n_id IN ( ";
		sString += "SELECT n_id FROM neighborship WHERE dI_intf_id=" + it->at(0) + " AND dI_intf_id1 IN ( ";
		sString += "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
		sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 ";
		sString += ") T1 GROUP BY col HAVING COUNT(*) > 1));";
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 0, "Anomaly: 3-Way Check");
	}
	
	dbgNachbarAusgabe();


	// Endgeräte importieren
	//////////////////////////////////////////////////////////////////////////
	if (importHosts)
	{
		hostSchreiber();
	}
	dbgNachbarAusgabe();

	// Schritt 4: Die verbliebenen Anomalien mit den CDP Nachbarschaften vergleichen und verbessern
	anomalyCdp();
	dbgNachbarAusgabe();
	
	// Schritt 5: Intf l2l3 Check - wenn ein Interface öfters vorkommt, dann schauen, ob auf beiden Seiten l2l3 gleich ist. 
	// Wenn nicht für einen Eintrag, dann diesen Eintrag löschen
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship WHERE flag != 2 ";
	sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1 ";
	if (!nidEmpty)
	{
		sString += "AND  col > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))";
	}

	dbgAsgbe1(sString, 1, 1, "Anomaly: Intf l2l3 Check");

	ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT l2l3 FROM interfaces where intf_id=" + it->at(0);
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		dbgAsgbe1(sString, 2, 1, "Anomaly: Intf l2l3 Check");

		sString = "SELECT intf_id FROM interfaces WHERE intf_id IN (SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
		sString += "WHERE dI_intf_id1=" + it->at(0) + " UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id=" + it->at(0) + ")) ";
		sString += "AND l2l3 IN (SELECT l2l3 FROM interfaces WHERE intf_id=" + it->at(0) + ")";
		std::vector<std::vector<std::string>> ret2 = dieDB->query(sString.c_str());
		
		if (!ret2.empty())
		{
			// Wenn bei der vorigen Abfrage mindestens ein Wert zurückgegeben wurde, dann wird jetzt der Rest gelöscht
			sString = "DELETE FROM neighborship WHERE dI_intf_id1=" + it->at(0) + " AND dI_intf_id IN (";
			sString += "SELECT intf_id FROM interfaces WHERE intf_id IN (SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
			sString += "WHERE dI_intf_id1=" + it->at(0) + " UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id=" + it->at(0) + ")) ";
			sString += "AND l2l3 NOT IN (SELECT l2l3 FROM interfaces WHERE intf_id=" + it->at(0) + "))";
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "Anomaly: Intf l2l3 Check");

			sString = "DELETE FROM neighborship WHERE dI_intf_id=" + it->at(0) + " AND dI_intf_id1 IN (";
			sString += "SELECT intf_id FROM interfaces WHERE intf_id IN (SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
			sString += "WHERE dI_intf_id1=" + it->at(0) + " UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id=" + it->at(0) + ")) ";
			sString += "AND l2l3 NOT IN (SELECT l2l3 FROM interfaces WHERE intf_id=" + it->at(0) + "))";
			dieDB->query(sString.c_str());
			dbgAsgbe1(sString, 2, 0, "Anomaly: Intf l2l3 Check");
		}
	}
	
	// Schritt 6: Alle übriggebliebenen Anomalien markieren
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship WHERE flag != 2 ";
	sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0) T1 ";
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

	// Schritt 7: Alle Einträge mit Flag 2 auf Flag 1 ummarkieren
	sString = "UPDATE neighborship SET flag=1 WHERE flag=2";
	ret = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Set flag=1 where flag=2");

	// Schritt 8: Markierung für vPC's aufheben
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


	// Schritt 9: Alle markierten Einträge ein letztes Mal prüfen und im Fall updaten
	sString = "UPDATE neighborship SET flag=0 WHERE flag=1 AND dI_intf_id NOT IN (SELECT col FROM( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship WHERE flag=1 UNION ALL SELECT dI_intf_id1  AS col FROM neighborship WHERE flag=1) ";
	sString += "GROUP BY col HAVING COUNT(*)>1) AND dI_intf_id1 NOT IN (SELECT col FROM(SELECT dI_intf_id AS col FROM neighborship WHERE flag=1 ";
	sString += "UNION ALL SELECT dI_intf_id1  AS col FROM neighborship WHERE flag=1) GROUP BY col HAVING COUNT(*)>1) ";
	ret = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 1, 0, "Anomaly: Last Check");
}


void WkmCombiner::cdpChecks()
{
	// Alle Nachbarschaftsbeziehungen finden, für die es einen cdp Eintrag gibt, aber keine neighborship
	std::string sString = "SELECT devInterface.device_dev_id,devInterface.interfaces_int_id,cdp.nName,cdp.nIntf FROM devInterface ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id INNER JOIN clink ON clink.interfaces_intf_id=interfaces.intf_id ";
	sString += "INNER JOIN cdp ON cdp.cdp_id=clink.cdp_cdp_id WHERE devInterface.interfaces_int_id NOT IN (SELECT dI_intf_id AS col FROM neighborship ";
	sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship) ";
	if (!nidEmpty)
	{
		sString += "AND cdp_cdp_id > (SELECT MAX(cdp_id) FROM rControl WHERE cdp_id < (SELECT MAX(cdp_id) FROM rControl));";
	}
	dbgAsgbe1(sString, 1, 4, "CDP C: Find cdp neighbors");

	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());

	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT intf_id,dev_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.hostname LIKE '" + it->at(2);
		sString += "' AND interfaces.intfName LIKE '" + it->at(3) + "' ";
		if (!nidEmpty)
		{
			sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		}

		dbgAsgbe1(sString, 2, 2, "CDP C: Find cdp neighbors");

		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());

		if (!ret1.empty())
		{
			std::string iString = "INSERT INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1,flag) VALUES (";
			iString += it->at(1) + "," + it->at(0) + "," + ret1[0][0] + "," + ret1[0][1] + ",3);";
			dbgAsgbe1(iString, 2, 0, "CDP C: Find cdp neighbors");
			dieDB->query(iString.c_str());
		}
	}
}


void WkmCombiner::setSwitchDevType()
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
	}
}

void WkmCombiner::insertRControl()
{
	std::string sString = "SELECT MAX(n_id) FROM neighborship;";
	
	dbgAsgbe1(sString, 1, 1, "insertRControl");

	std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());

	std::string nid = "";
	if (!result.empty())
	{
		nid = result[0][0];
	}

	sString = "SELECT MAX(rc_id) FROM rControl;";
	dbgAsgbe1(sString, 1, 1, "insertRControl");

	result = dieDB->query(sString.c_str());

	std::string iString = "UPDATE rControl SET n_id=" + nid + " WHERE rc_id=" + result[0][0] + ";";
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


void WkmCombiner::hostSchreiber()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Import EndSystems\n", "6620", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);


	std::string sString = "SELECT DISTINCT l2_addr,l3_addr FROM neighbor WHERE l3_addr NOT NULL AND l2_addr NOT LIKE 'Incomplete%' ";
	sString += "AND self=0 AND l2_addr NOT IN (SELECT DISTINCT macAddress FROM interfaces WHERE macAddress NOT LIKE '' AND phl<>3 ";
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
		sString1 += "WHERE neighbor.l2_addr LIKE '" + it->at(0) + "' AND intf_id NOT IN (SELECT DISTINCT int_vlan.interfaces_intf_id FROM int_vlan) ";
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

			std::string iString = "INSERT INTO device (dev_id,type,hwtype,dataSource,hostname,snmpLoc) VALUES(NULL,12,12,2, '" + hostname + "','" + vendorName + "');";
			dieDB->query(iString.c_str());
			dbgAsgbe1(iString, 2, 0, "hostSchreiber");


			sString1 = "SELECT last_insert_rowid() FROM device;";
			dbgAsgbe1(sString1, 2, 1, "hostSchreiber");

			std::vector<std::vector<std::string> > ret1 = dieDB->query(sString1.c_str());
			std::string dev_id = "";
			if (!ret1.empty())
			{
				dev_id = ret1[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0050 - Contact wktools@spoerr.org";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			}

			iString = "INSERT INTO interfaces (intfName, ipAddress, macAddress, subnetMask, status, l2l3) VALUES ('Port1', '" + it->at(1) + "','" + it->at(0) + "',' ', 'UP', 'L3');";
			dbgAsgbe1(iString, 2, 0, "hostSchreiber");
			dieDB->query(iString.c_str());

			sString1 = "SELECT last_insert_rowid() FROM interfaces;";
			dbgAsgbe1(sString1, 2, 1, "hostSchreiber");

			std::vector<std::vector<std::string> > ret3 = dieDB->query(sString1.c_str());
			std::string intf_id = "";
			if (!ret3.empty())
			{
				intf_id = ret3[0][0];
			}
			else
			{
				std::string dbgA = "\n6305: Parser Error! Code 0051 - Contact wktools@spoerr.org";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			}


			// 3. devInterface Tabelle updaten
			iString = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (";
			iString += intf_id + "," + dev_id + ");";
			dieDB->query(iString.c_str());
			dbgAsgbe1(iString, 2, 0, "hostSchreiber");

			// 4. intfSubnet befüllen
			// Dafür muss die IP Adresse in ulong umgewandelt werden
			// Error Handling und Initialisierung der IP Adresse schon beim Reverse DNS gemacht
			unsigned long ipAddress = ipa.to_ulong();
			sString1 = "SELECT ipSubnet_id FROM ipSubnet WHERE ulSubnet<" + boost::lexical_cast<std::string>(ipAddress); 
			sString1 += " AND ulBroadcast>" + boost::lexical_cast<std::string>(ipAddress);
			dbgAsgbe1(sString1, 2, 1, "hostSchreiber");

			std::vector<std::vector<std::string> > ret5 = dieDB->query(sString1.c_str());
			if (!ret5.empty())
			{
				iString = "INSERT INTO intfSubnet (ipSubnet_ipSubnet_id, interfaces_intf_id) VALUES (" + ret5[0][0] + ", ";
				iString += intf_id + ");";
				dieDB->query(iString.c_str());
				dbgAsgbe1(iString, 2, 0, "hostSchreiber");
			}
			else
			{
				std::string dbgA = "\n6406 Parser Error: No Subnet found for Host Address " + it->at(1) + ".";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6406", 
					WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			}

			// 5. neighborship Tabelle befüllen
			iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1, flag) ";
			iString += "VALUES (" + it0->at(0) + "," + it0->at(1) + "," + intf_id + "," + dev_id + ",0)";
			dieDB->query(iString.c_str());
			dbgAsgbe1(iString, 2, 0, "hostSchreiber");
		}
	}

	// rControl aktualisieren
	sString = "SELECT MAX(dev_id) FROM device";
	std::vector<std::vector<std::string> > rRes1 = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 1, "hostSchreiber");

	sString = "SELECT MAX(intf_id) FROM interfaces";
	std::vector<std::vector<std::string> > rRes2 = dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 1, "hostSchreiber");

	sString = "UPDATE rControl SET dev_id=" + rRes1[0][0] + ",intf_id=" + rRes2[0][0] + " WHERE rc_id IN (SELECT MAX(rc_id) FROM rControl)";
	dieDB->query(sString.c_str());
	dbgAsgbe1(sString, 2, 0, "hostSchreiber");
}

void WkmCombiner::dbgNachbarAusgabe()
{
	if (debugAusgabe)
	{
		std::string sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1 FROM neighborship ";
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
			std::string nachbaren = h1 + "-" + i1 + " <--> " + h2 + "-" + i2 + "\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);
		}
	}
}


void WkmCombiner::dbgAsgbe1(std::string dbga, int type, int anzahlWerte, std::string position)
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
#endif //#ifdef WKTOOLS_MAPPER
