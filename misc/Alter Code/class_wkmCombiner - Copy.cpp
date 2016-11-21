
#include "class_wkmCombiner.h"
#ifdef WKTOOLS_MAPPER

WkmCombiner::WkmCombiner(WkmDB *db, WkLog *logA)
{
	dieDB = db;
	logAusgabe = logA;
}


WkmCombiner::~WkmCombiner()
{

}


void WkmCombiner::doIt()
{
	// Device IDs der Switches auslesen
	string sString = "SELECT dev_id FROM device WHERE type=2;";
	vector<vector<string>> switches = dieDB->query(sString.c_str());

	// Dedizierte L2 Nachbarschaften auslesen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 1: L2", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	for(vector<vector<string>>::iterator it = switches.begin(); it < switches.end(); ++it)
	{
		// Alle Interfaces herausfinden, auf dem L2 Interface MAC Adressen von einem bestimmten Gerät gelernt werden
		sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface ";
		sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
		sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
		sString += "WHERE neighbor.l2_addr IN ( ";
		sString += "SELECT macAddress FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND interfaces.l2l3 LIKE 'L2' AND device.type=2) ";
		sString += "AND interfaces.l2l3 LIKE 'L2';";

		vector<vector<string>> ret = dieDB->query(sString.c_str());
		for(vector<vector<string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			sString =  "SELECT DISTINCT interfaces_int_id,intfType FROM interfaces ";
			sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
			sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
			sString += "WHERE interfaces.macAddress IN ( ";
			sString += "SELECT l2_addr FROM neighbor ";
			sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces_intf_id ";
			sString += "INNER JOIN device ON devInterface.device_dev_id=device.dev_id ";
			sString += "WHERE interfaces.l2l3 LIKE 'L2' AND interfaces.intf_id=" + it2->at(0) + ") ";
			sString += "AND interfaces.l2l3 LIKE 'L2' ";
			sString += "AND interfaces.intfType IN (SELECT intfType FROM interfaces WHERE intf_id="	+ it2->at(0) + ";";
			vector<vector<string>> ret2 = dieDB->query(sString.c_str());
			if (!ret2.empty())
			{
				sString = "SELECT device_dev_id FROM devInterface WHERE interfaces_int_id=" + ret2[0][0] + ";";
				vector<vector<string>> ret3 = dieDB->query(sString.c_str());
				
				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + ret2[0][0];
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + ret2[0][0] + ");";
				vector<vector<string>> nid = dieDB->query(sString.c_str());

				// Check, ob die Interfaces zusammenpassen (intfType)


				if (nid.empty())
				{
					string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
					iString += "VALUES (" + it2->at(0) + "," + it2->at(1) + "," + ret2[0][0] + "," + ret3[0][0] + ");";
					dieDB->query(iString.c_str());
				}
			}
		}
	}

	// STP Topologie #1
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 2: STP#1", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	for(vector<vector<string>>::iterator it = switches.begin(); it < switches.end(); ++it)
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
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + "));";

		vector<vector<string>> macIntf = dieDB->query(sString.c_str());
		for(vector<vector<string>>::iterator it2 = macIntf.begin(); it2 < macIntf.end(); ++it2)
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
			sString += "WHERE devInterface.device_dev_id =" + it->at(0) + "));";
			//sString += "OR device.stpBridgeID IN (SELECT DISTINCT l2_addr FROM neighbor ";
			//sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			//sString += "INNER JOIN interfaces ON nlink.interfaces_intf_id=interfaces.intf_id ";
			//sString += "WHERE interfaces.intf_id=" + it2->at(0) + ");";

			vector<vector<string>> intfID = dieDB->query(sString.c_str());
			for(vector<vector<string>>::iterator it3 = intfID.begin(); it3 < intfID.end(); ++it3)
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
				sString += "WHERE devInterface.device_dev_id=" + it->at(0) + ") AND neighbor.l3_addr IS NULL;";

				vector<vector<string>> intfID2 = dieDB->query(sString.c_str());
				for(vector<vector<string>>::iterator it4 = intfID2.begin(); it4 < intfID2.end(); ++it4)
				{
					string nIntf = "0";
					if (it2->at(1) == it4->at(1))
					{
						nIntf = it4->at(0);
					}


					// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
					sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + nIntf + " OR dI_intf_id=" + it2->at(0);
					sString += ") AND (dI_intf_id1=" + nIntf + " OR dI_intf_id1=" + it2->at(0) + ");";
					vector<vector<string>> nid = dieDB->query(sString.c_str());

					if (nid.empty())
					{
						if (nIntf == "0")
						{
							// Check, ob es einen ähnlichen Eintrag schon gibt, bei dem kein NULL vorkommt
							sString = "SELECT n_id FROM neighborship WHERE dI_dev_id=" + it3->at(0) + " AND (dI_dev_id1=" + it->at(0) + " AND dI_intf_id1=" + it2->at(0) + ");";
							sString += "OR (dI_dev_id=" + it->at(0) + " AND dI_dev_id1=" + it3->at(0) + " AND dI_intf_id=" + it2->at(0) + ");";
							nid = dieDB->query(sString.c_str());
							if (nid.empty())
							{
								string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
								iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nIntf + "," + it3->at(0) + ");";
								dieDB->query(iString.c_str());
							}
						}
						else
						{
							string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nIntf + "," + it3->at(0) + ");";
							dieDB->query(iString.c_str());
						}
					}
				}
			}
		}
	}

	// STP Topologie #2
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 3: STP#2", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	for(vector<vector<string>>::iterator it = switches.begin(); it < switches.end(); ++it)
	{
		// 1) ALLE Interfaces rausfinden, auf denen die Designated Bridge ID nicht gleich einer der eigenen MAC Adressen ist
		sString = "SELECT DISTINCT intf_id FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN stp_status ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
		sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " ";
		sString += "AND l2l3 LIKE 'L2' ";
		sString += "AND interfaces.channel_intf_id IS NULL ";
		sString += "AND intfType NOT LIKE 'Vlan' ";
		sString += "AND stp_status.designatedBridgeID NOT IN (";
		sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ");";

		vector<vector<string>> ret = dieDB->query(sString.c_str());
		for(vector<vector<string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			// 2) Device ID zum Interface von 1) rausfinden
			sString = "SELECT DISTINCT dev_id FROM device WHERE stpBridgeID IN (";
			sString += "SELECT DISTINCT designatedBridgeID FROM stp_status ";
			sString += "INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
			sString += "WHERE int_vlan.interfaces_intf_id=" + it2->at(0) + " ";
			sString += "AND designatedBridgeID NOT IN ( ";
			sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + "));";


			vector<vector<string>> ret2 = dieDB->query(sString.c_str());
			for(vector<vector<string>>::iterator it3 = ret2.begin(); it3 < ret2.end(); ++it3)
			{
				// 3) Gegenseiteeite finden
				sString = "SELECT DISTINCT intf_id FROM interfaces ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				sString += "INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
				sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id ";
				sString += "WHERE devInterface.device_dev_id=" + it3->at(0) + " ";
				sString += "AND l2l3 LIKE 'L2' ";
				sString += "AND interfaces.channel_intf_id IS NULL ";
				sString += "AND intfType NOT LIKE 'Vlan' ";
				sString += "AND neighbor.l2_addr IN (";
				sString += "SELECT macAddress FROM interfaces ";
				sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
				sString += "WHERE devInterface.device_dev_id=" + it->at(0) + ") ";
				sString += "AND interfaces.intfType IN ( ";
				sString += "SELECT intfType FROM interfaces WHERE intf_id=" + it2->at(0) + ");";
				//sString += "OR neighbor.l2_addr IN (";
				//sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ");";
				
				vector<vector<string>> ret3 = dieDB->query(sString.c_str());
				if (ret3.empty())
				{
					// 4) Wenn 3) nicht erfolgreich, dann STP Infos prüfen
					sString = "SELECT DISTINCT intf_id FROM interfaces "; 
					sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
					sString += "INNER JOIN stp_status ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
					sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
					sString += "WHERE devInterface.device_dev_id=" + it3->at(0) + " ";
					sString += "AND l2l3 LIKE 'L2' ";
					sString += "AND interfaces.channel_intf_id IS NULL ";
					sString += "AND intfType NOT LIKE 'Vlan' ";
					sString += "AND stp_status.designatedBridgeID IN (";
					sString += "SELECT stpBridgeID FROM device WHERE dev_id=" + it->at(0) + ") ";
					sString += "AND interfaces.intfType IN ( ";
					sString += "SELECT intfType FROM interfaces WHERE intf_id=" + it2->at(0) + ");";
				}

				string nintf = "0";
				if (!ret3.empty())
				{
					nintf = ret3[0][0];
				}
					
				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + nintf;
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + nintf + ");";
				vector<vector<string>> nid = dieDB->query(sString.c_str());

				if (nid.empty())
				{
					if (nintf == "0")
					{
						// Check, ob es einen ähnlichen Eintrag schon gibt, bei dem kein NULL vorkommt
						sString = "SELECT n_id FROM neighborship WHERE dI_dev_id=" + it3->at(0) + " AND (dI_dev_id1=" + it->at(0) + " AND dI_intf_id1=" + it2->at(0) + ") ";
						sString += "OR (dI_dev_id=" + it->at(0) + " AND dI_dev_id1=" + it3->at(0) + " AND dI_intf_id=" + it2->at(0) + ");";
						nid = dieDB->query(sString.c_str());
						if (nid.empty())
						{
							string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nintf + "," + it3->at(0) + ");";
							dieDB->query(iString.c_str());
						}
					}
					else
					{
						// Check, ob es beide Intf/Dev Pärchen schon gibt
						sString = "SELECT n_id FROM neighborship ";
						sString += "WHERE (dI_intf_id=" + it2->at(0) + " AND dI_dev_id=" + it->at(0) + ") ";
						sString += "OR (dI_intf_id1=" + it2->at(0) + " AND dI_dev_id1=" + it->at(0) + ");";
						vector<vector<string>> nid2 = dieDB->query(sString.c_str());

						sString = "SELECT n_id FROM neighborship ";
						sString += "WHERE (dI_intf_id=" + nintf + " AND dI_dev_id=" + it3->at(0) + ") ";
						sString += "OR (dI_intf_id1=" + nintf + " AND dI_dev_id1=" + it3->at(0) + ");";
						vector<vector<string>> nid3 = dieDB->query(sString.c_str());

						if (nid2.empty() || nid3.empty())
						{
							string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
							iString += "VALUES (" + it2->at(0) + "," + it->at(0) + "," + nintf + "," + it3->at(0) + ");";
							dieDB->query(iString.c_str());
						}
					}
				}
			}
		}
	}

	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 4: STP Check", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	stpKonistenzCheck();


	// NonSwitch Nachbarschaften auslesen
	//////////////////////////////////////////////////////////////////////////
	// Intf_id's auslesen
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 5: L2 - Non Switch", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	sString = "SELECT intf_id FROM interfaces ";
	sString += "INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	sString += "WHERE device.type <>2 AND interfaces.phl=1;";
	
	vector<vector<string>> nonSwitches = dieDB->query(sString.c_str());
	// Dedizierte L2 Nachbarschaften auslesen
	//////////////////////////////////////////////////////////////////////////
	for(vector<vector<string>>::iterator it = nonSwitches.begin(); it < nonSwitches.end(); ++it)
	{
		sString = "SELECT intf_id FROM interfaces ";
		sString += "WHERE macAddress IN ( ";
		sString += "SELECT l2_addr FROM neighbor ";
		sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
		sString += "WHERE interfaces.intf_id=" + it->at(0) + ") ";
		sString += "AND intf_id <> " + it->at(0) + ";";
		vector<vector<string>> ret = dieDB->query(sString.c_str());
		for(vector<vector<string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			sString = "SELECT DISTINCT device_dev_id FROM devInterface WHERE interfaces_int_id=" + it->at(0);
			vector<vector<string>> ret1 = dieDB->query(sString.c_str());
			sString = "SELECT DISTINCT device_dev_id FROM devInterface WHERE interfaces_int_id=" + it2->at(0);
			vector<vector<string>> ret2 = dieDB->query(sString.c_str());

			// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
			sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + ret2[0][0];
			sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + ret2[0][0] + ");";
			vector<vector<string>> nid = dieDB->query(sString.c_str());
			if (nid.empty())
			{
				string intf1 = it->at(0);
				string intf2 = it2->at(0);

				// Check, ob Subinterface, oder nicht
				sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf1 + ";";
				vector<vector<string>> sid1 = dieDB->query(sString.c_str());
				if (sid1[0][0] != "")
				{
					intf1 = sid1[0][0];
				}

				sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf2 + ";";
				vector<vector<string>> sid2 = dieDB->query(sString.c_str());
				if (sid2[0][0] != "")
				{
					intf2 = sid2[0][0];
				}

				string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
				iString += "VALUES (" + intf1 + "," + ret1[0][0] + "," + intf2 + "," + ret2[0][0] + ");";
				dieDB->query(iString.c_str());
			}
		}
	}
	
	// Dedizierte L2 Nachbarschaften auslesen #2
	//////////////////////////////////////////////////////////////////////////
	sString = "SELECT dev_id FROM device WHERE type <> 2 AND dataSource IS NULL;";
	nonSwitches = dieDB->query(sString.c_str());

	for(vector<vector<string>>::iterator it = nonSwitches.begin(); it < nonSwitches.end(); ++it)
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
		sString += "AND interfaces.intf_id NOT IN (SELECT dI_intf_id FROM neighborship); ";

		vector<vector<string>> ret = dieDB->query(sString.c_str());
		for(vector<vector<string>>::iterator it2 = ret.begin(); it2 < ret.end(); ++it2)
		{
			sString =  "SELECT DISTINCT interfaces_int_id,device_dev_id FROM devInterface ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE interfaces.macAddress IN ( ";
			sString += "SELECT l2_addr FROM neighbor ";
			sString += "INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=nlink.interfaces_intf_id ";
			sString += "WHERE interfaces.intf_id=" + it2->at(0) + ") ";
			sString += "AND interfaces.phl=1;";
			vector<vector<string>> ret2 = dieDB->query(sString.c_str());
			if (!ret2.empty())
			{
				// Check, ob es den selben Eintrag in dieser oder in anderer Reihenfolge schon gibt
				sString = "SELECT n_id FROM neighborship WHERE (dI_intf_id=" + it2->at(0) + " OR dI_intf_id=" + ret2[0][0];
				sString += ") AND (dI_intf_id1=" + it2->at(0) + " OR dI_intf_id1=" + ret2[0][0] + ");";
				vector<vector<string>> nid = dieDB->query(sString.c_str());

				if (nid.empty())
				{
					string intf1 = it2->at(0);
					string intf2 = ret2[0][0];

					// Check, ob Subinterface, oder nicht
					sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf1 + ";";
					vector<vector<string>> sid1 = dieDB->query(sString.c_str());
					if (sid1[0][0] != "")
					{
						intf1 = sid1[0][0];
					}

					sString = "SELECT channel_intf_id FROM interfaces WHERE intf_id = " + intf2 + ";";
					vector<vector<string>> sid2 = dieDB->query(sString.c_str());
					if (sid2[0][0] != "")
					{
						intf2 = sid2[0][0];
					}

					string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
					iString += "VALUES (" + intf1 + "," + it2->at(1) + "," + intf2 + "," + ret2[0][1] + ");";
					dieDB->query(iString.c_str());
				}
			}
		}
	}			

	
	// CDP Checks
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 6: CDP Check", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	cdpChecks();
	


	// Alle Interfaces, auf denen unbekannte Switches hängen eintragen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 7: L2 - Unknown STP Neighbors", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	// 1) ALLE Interfaces rausfinden, auf denen die Designated Bridge ID nicht gleich einer der eigenen MAC Adressen ist
	sString = "SELECT DISTINCT interfaces.intf_id,interfaces.intfName FROM interfaces ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "WHERE interfaces_intf_id NOT IN ( ";
	sString += "SELECT dI_intf_id FROM neighborship) ";
	sString += "AND interfaces_intf_id NOT IN ( ";
	sString += "SELECT dI_intf_id1 FROM neighborship) ";
	sString += "AND interfaces.channel_intf_id IS NULL ";
	vector<vector<string>> unknownIntfs = dieDB->query(sString.c_str());
		
	for(vector<vector<string>>::iterator it = unknownIntfs.begin(); it < unknownIntfs.end(); ++it)
	{
		sString = "SELECT dev_id FROM device ";
		sString += "INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
		sString += "WHERE devInterface.interfaces_int_id=" + it->at(0) + ";";
		vector<vector<string>> ret = dieDB->query(sString.c_str());
		
		string iString = "INSERT OR IGNORE INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) ";
		iString += "VALUES (" + it->at(0) + "," + ret[0][0] + ", 0, 0);";
		dieDB->query(iString.c_str());
	}
	// Duplikate entfernen
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 8: Remove Duplicate Entries", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	delDups();

	// Abschließender Anomalie Check
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_ZEIT, "6620: Adjacencies - Step 9: Anomaly Check", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	markAnomaly();

	// L3 Checks
	//////////////////////////////////////////////////////////////////////////

}


void WkmCombiner::stpKonistenzCheck()
{
	// Prüfen, ob es Einträge gibt, die kombiniert werden können, da gegengleiche "0" Einträge beim Interface bestehen
	string sString = "SELECT n_id,dI_intf_id FROM neighborship WHERE dI_intf_id1=0;";
	vector<vector<string>> ret = dieDB->query(sString.c_str());

	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT n_id FROM neighborship WHERE (dI_dev_id=(SELECT dI_dev_id1 FROM neighborship WHERE n_id=" + it->at(0);
		sString += ") AND dI_dev_id1=(SELECT dI_dev_id FROM neighborship WHERE n_id=" + it->at(0) + ") AND dI_intf_id1=0);";
		vector<vector<string>> ret1 = dieDB->query(sString.c_str());
		if (!ret1.empty())
		{
			sString = "SELECT n_id,dI_intf_id FROM neighborship ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE interfaces.intfType IN (SELECT intfType FROM interfaces WHERE intf_id=" + it->at(1) + ") ";
			sString += "AND neighborship.n_id IN (SELECT n_id FROM neighborship WHERE dI_intf_id1=0);"; 
			vector<vector<string>> ret2 = dieDB->query(sString.c_str());
			if (ret2.size() == 2)
			{
				sString = "UPDATE neighborship SET dI_intf_id1=" + ret2[1][1] + " WHERE n_id=" + ret2[0][0] + ";";
				dieDB->query(sString.c_str());

				sString = "DELETE FROM neighborship WHERE n_id=" + ret2[1][0] + ";";
				dieDB->query(sString.c_str());
			}
		}
	}

	// dI_intf_id1 auf 0 setzen, wenn es bereits einen Eintrag auf einen anderen Host gibt und wenn der STP Status von dem Interface auf Blocking ist:
	sString = "UPDATE OR IGNORE neighborship SET dI_intf_id1=0 WHERE n_id IN (";
	sString += "SELECT DISTINCT n_id FROM neighborship ";
	sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
	sString += "WHERE (dI_intf_id IN (SELECT dI_intf_id1 FROM neighborship) OR dI_intf_id1 IN (SELECT dI_intf_id FROM neighborship)) ";
	sString += "AND stp_status.stpIntfStatus LIKE 'alternate blocking ');";
	dieDB->query(sString.c_str());

	sString = "UPDATE OR IGNORE neighborship SET dI_intf_id=0 WHERE n_id IN (";
	sString += "SELECT DISTINCT n_id FROM neighborship ";
	sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id1 ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN int_vlan ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
	sString += "WHERE (dI_intf_id IN (SELECT dI_intf_id1 FROM neighborship) OR dI_intf_id1 IN (SELECT dI_intf_id FROM neighborship)) ";
	sString += "AND stp_status.stpIntfStatus LIKE 'alternate blocking ');";
	dieDB->query(sString.c_str());

	// Schauen, ob beim Nachbaren nur ein Interface vom selben Typ vorkommt und im Fall ein Update
	sString = "SELECT n_id, dI_intf_id,dI_dev_id1 FROM neighborship WHERE dI_intf_id1=0;";
	ret = dieDB->query(sString.c_str());

	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT intf_id FROM devInterface "; 
		sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(2) + " ";
		sString += "AND intfType IN ( ";
		sString += "SELECT intfType FROM interfaces WHERE intf_id=" + it->at(1) + ") ";
		sString += "AND intf_id NOT IN (SELECT dI_intf_id FROM neighborship) ";
		sString += "AND intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship);";
		vector<vector<string>> ret1 = dieDB->query(sString.c_str());
		if (ret1.size() == 1)
		{
			sString = "UPDATE neighborship SET dI_intf_id1=" + ret1[0][0] + " WHERE n_id=" + it->at(0) + ";";
			dieDB->query(sString.c_str());
		}
	}

	// Prüfen #2, ob es Einträge gibt, die kombiniert werden können, da gegengleiche "0" Einträge beim Interface bestehen
	sString = "SELECT n_id,dI_intf_id FROM neighborship WHERE dI_intf_id1=0;";
	ret = dieDB->query(sString.c_str());

	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT n_id FROM neighborship WHERE (dI_dev_id=(SELECT dI_dev_id1 FROM neighborship WHERE n_id=" + it->at(0);
		sString += ") AND dI_dev_id1=(SELECT dI_dev_id FROM neighborship WHERE n_id=" + it->at(0) + ") AND dI_intf_id1=0);";
		vector<vector<string>> ret1 = dieDB->query(sString.c_str());
		if (!ret1.empty())
		{
			sString = "SELECT n_id,dI_intf_id FROM neighborship ";
			sString += "INNER JOIN interfaces ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE interfaces.intfType IN (SELECT intfType FROM interfaces WHERE intf_id=" + it->at(1) + ") ";
			sString += "AND neighborship.n_id IN (SELECT n_id FROM neighborship WHERE dI_intf_id1=0);"; 
			vector<vector<string>> ret2 = dieDB->query(sString.c_str());
			if (ret2.size() == 2)
			{
				sString = "UPDATE neighborship SET dI_intf_id1=" + ret2[1][1] + " WHERE n_id=" + ret2[0][0] + ";";
				dieDB->query(sString.c_str());

				sString = "DELETE FROM neighborship WHERE n_id=" + ret2[1][0] + ";";
				dieDB->query(sString.c_str());
			}
		}
	}

	// Prüfen, ob es komplette Einträge und passende halbe Einträge gibt; Wenn ja, dann den halben Eintrag löschen
	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT n_id FROM neighborship WHERE (dI_dev_id=(SELECT dI_dev_id1 FROM neighborship WHERE n_id=" + it->at(0);
		sString += ") AND dI_dev_id1=(SELECT dI_dev_id FROM neighborship WHERE n_id=" + it->at(0) + ") AND dI_intf_id1=" + it->at(1) + ");";
		vector<vector<string>> ret1 = dieDB->query(sString.c_str());
		if (!ret1.empty())
		{
			sString = "DELETE FROM neighborship WHERE n_id=" + it->at(0) + ";";
			dieDB->query(sString.c_str());
		}
	}

}


void WkmCombiner::delDups()
{
	// Doppelte Einträge löschen
	string sString = "DELETE FROM neighborship WHERE EXISTS ( ";
	sString += "SELECT 1 FROM neighborship N2 ";
	sString += "WHERE neighborship.dI_intf_id=N2.dI_intf_id  ";
	sString += "AND neighborship.dI_dev_id=N2.dI_dev_id  ";
	sString += "AND neighborship.dI_intf_id1=N2.dI_intf_id1 ";
	sString += "AND neighborship.dI_dev_id1=N2.dI_dev_id1  ";
	sString += "AND neighborship.n_id>N2.n_id);";
	dieDB->query(sString.c_str());

	sString = "DELETE FROM neighborship WHERE EXISTS ( ";
	sString += "SELECT 1 FROM neighborship N2 ";
	sString += "WHERE neighborship.dI_intf_id=N2.dI_intf_id1  ";
	sString += "AND neighborship.dI_dev_id=N2.dI_dev_id1  ";
	sString += "AND neighborship.dI_intf_id1=N2.dI_intf_id ";
	sString += "AND neighborship.dI_dev_id1=N2.dI_dev_id  ";
	sString += "AND neighborship.n_id>N2.n_id);";
	dieDB->query(sString.c_str());

}


void WkmCombiner::schreibeLog(int type, string logEintrag, int farbe, int format, int groesse)
{
	if (logAusgabe != NULL)
	{
		WkLog::evtData evtDat;
		evtDat.farbe = farbe;
		evtDat.format = format;
		evtDat.groesse = groesse;
		evtDat.logEintrag = logEintrag;
		evtDat.type = type;

		LogEvent evt(EVT_LOG_MELDUNG);
		evt.SetData(evtDat);
		wxPostEvent(logAusgabe, evt);
	}
}


void WkmCombiner::markAnomaly()
{
	// Schritt 1: Wenn es einen vollständigen Eintrag gibt und einen ähnlichen, bei dem auf einer Seite 0 steht, dann den Eintrag mit 0 löschen
	string sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1;";
	vector<vector<string>> ret = dieDB->query(sString.c_str());
	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "DELETE FROM neighborship WHERE dI_intf_id=" + it->at(0) + " AND dI_intf_id1=0;";
		dieDB->query(sString.c_str());
	}

	// Schritt 2: Wenn es 3 Einträge für zwei Nachbarschaftsbeziehungen gibt und zwei Einträge doppelt vorkommen 
	// und diese Einträge zusätzlich in einer Zeile zu finden sind, dann  diese Zeile löschen
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship UNION ALL ";
	sString += "SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1;";
	ret = dieDB->query(sString.c_str());
	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "DELETE FROM neighborship WHERE n_id IN ( ";
		sString += "SELECT n_id FROM neighborship WHERE dI_intf_id=" + it->at(0) + " AND dI_intf_id1 IN ( ";
		sString += "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship ";
		sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 ";
		sString += ") T1 GROUP BY col HAVING COUNT(*) > 1));";
		dieDB->query(sString.c_str());
	}
	
	// Alle übriggebliebenen Anomalien markieren
	sString = "SELECT col FROM ( ";
	sString += "SELECT dI_intf_id AS col FROM neighborship ";
	sString += "UNION ALL SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id1 > 0 ) T1 ";
	sString += "GROUP BY col HAVING COUNT(*) > 1;";
	
	ret = dieDB->query(sString.c_str());
	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "UPDATE neighborship SET flag=1 WHERE dI_intf_id1=" + it->at(0) + " OR dI_intf_id=" + it->at(0) + ";";
		dieDB->query(sString.c_str());
	}
}


void WkmCombiner::cdpChecks()
{
	// Alle Interfaces anzeigen, bei denen cdp_cdp_id != NULL und
	// die nicht in neighborship vorkommen außer Flag ist gesetzt oder das Nachbarinterface ist 0
	string sString = "SELECT interfaces_int_id, cdp_cdp_id, device_dev_id FROM devInterface ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id ";
	sString += "WHERE  cdp_cdp_id NOT NULL ";
	sString += "AND interfaces_int_id NOT IN( ";
	sString += "SELECT dI_intf_id FROM neighborship "; 
	sString += "WHERE dI_intf_id1 <>0 "; 
	sString += "AND flag IS NULL) ";
	sString += "AND interfaces_int_id NOT IN( ";
	sString += "SELECT dI_intf_id1 FROM neighborship ";
	sString += "WHERE flag IS NULL) ";
	sString += "AND interfaces.channel_intf_id IS NULL; ";

	vector<vector<string>> ret = dieDB->query(sString.c_str());
	for(vector<vector<string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		sString = "SELECT interfaces_int_id, device_dev_id FROM devInterface WHERE  cdp_cdp_id =" + it->at(1) + ";";
		vector<vector<string>> ret1 = dieDB->query(sString.c_str());
		if (ret1.size() > 1)
		{
			string iString = "INSERT INTO neighborship (dI_intf_id, dI_dev_id, dI_intf_id1, dI_dev_id1) VALUES (";
			iString += ret1[0][0] + "," + ret1[0][1] + "," + ret1[1][0] + "," + ret1[1][1] + ");";
			dieDB->query(iString.c_str());
		}
	}
}




#endif //#ifdef WKTOOLS_MAPPER
