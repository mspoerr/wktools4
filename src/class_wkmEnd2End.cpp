#include "class_wkmEnd2End.h"
#ifdef WKTOOLS_MAPPER

WkmEnd2End::WkmEnd2End(WkLog *logA, WkmDB *db)
{
	logAusgabe = logA;
	dieDB = db;

#ifdef DEBUG
	debugAusgabe = true;
#endif

}



WkmEnd2End::~WkmEnd2End()
{

}


std::string WkmEnd2End::doEnd2End(std::string h1, std::string h2)
{
	std::string ergebnis = h1;
	std::string sqlQuery;
	std::string deviceId;
	
	std::vector<std::string> path; 

	boost::asio::ip::address_v4 src = boost::asio::ip::address_v4::from_string(h1);
	boost::asio::ip::address_v4 dest = boost::asio::ip::address_v4::from_string(h2);

	unsigned long ulSrc = src.to_ulong();
	unsigned long ulDest = dest.to_ulong();
	unsigned long ulHop = ulSrc;

	bool done = false; 
	bool cloudPresent = false; 

	path = ExecuteRequest(ulHop, ulDest, &cloudPresent);

	for (std::vector<std::string>::iterator pathIt = path.begin(); pathIt != path.end(); pathIt++)
	{
		ergebnis += " -> ";
		ergebnis += *pathIt;
	}

	if (cloudPresent)
	{
		std::swap(ulSrc, ulDest);
		path = ExecuteRequest(ulSrc, ulDest, &cloudPresent);

		ergebnis += " -> Cloud";

		for (std::vector<std::string>::reverse_iterator pathIt = path.rbegin(); pathIt != path.rend(); pathIt++)
		{
			ergebnis += " -> ";
			ergebnis += *pathIt;
		}
	}

	ergebnis += " -> " + h2;
	return ergebnis;
}

std::vector<std::string> WkmEnd2End::ExecuteRequest(unsigned long src, unsigned long dest, bool *cloudPresent)
{
	std::string sqlQuery; 
	std::string deviceId;

	std::vector<std::string> path;

	bool done = false; 

	done = CheckIfTargetNetworkReached(src, dest);

	while (!done)
	{

		sqlQuery = "SELECT dev_id, hostname, intfName, ipAddress FROM device ";
		sqlQuery += "INNER JOIN devInterface ON device.dev_id = devInterface.device_dev_id ";
		sqlQuery += "INNER JOIN interfaces ON interfaces.intf_id = devInterface.interfaces_int_id ";
		sqlQuery += "INNER JOIN intfSubnet ON intfSubnet.interfaces_intf_id = interfaces.intf_id ";
		sqlQuery += "INNER JOIN ipSubnet ON ipSubnet.ipSubnet_id = intfSubnet.ipSubnet_ipSubnet_id ";
		sqlQuery += "WHERE ipAddress NOT NULL AND ipAddress NOT LIKE '' AND status LIKE 'UP' ";
		sqlQuery += "AND ipSubnet.ulSubnet<" + boost::lexical_cast<std::string>(src) + " AND ipSubnet.ulBroadcast>" + boost::lexical_cast<std::string>(src);
		sqlQuery += " AND device.hwType NOT NULL; ";

		std::vector<std::vector<std::string>> response = dieDB->query(sqlQuery.c_str());

		if (response.size() == 0)
		{
			std::string logMsg = "No device in specified net.";
			schreibeLog(WkLog::WkLog_NORMALTYPE, logMsg, "6310", WkLog::WkLog_SCHWARZ);

			break;
		}
		else if (response.size() == 1)
		{
			path.push_back(response[0][1] + " " + response[0][2] + " " + response[0][3]);

			deviceId = response[0][0];
			src = boost::asio::ip::address_v4::from_string(response[0][3]).to_ulong();
		}
		else if (response.size() > 1)
		{
			deviceId = FilterResponse(response).hdev_id[0];
			if (deviceId == "ERROR" || deviceId == "Dead End")
			{
				path.push_back(deviceId);
				break;
			}
			/*
			sqlQuery = "SELECT hostname, intfName, ipAddress FROM device ";
			sqlQuery += "INNER JOIN devInterface on device.dev_id = devInterface.device_dev_id ";
			sqlQuery += "INNER JOIN interfaces on devInterface.interfaces_int_id=interfaces.intf_id ";
			sqlQuery += "WHERE dev_id = " + deviceId;

			path.push_back(hostname[0][0] + " " + hostname[0][1] + " " + hostname[0][2]);
			*/

			sqlQuery = "SELECT hostname FROM device ";
			sqlQuery += "INNER JOIN devInterface on device.dev_id = devInterface.device_dev_id ";
			sqlQuery += "INNER JOIN interfaces on devInterface.interfaces_int_id=interfaces.intf_id ";
			sqlQuery += "WHERE dev_id = " + deviceId;

			std::vector<std::vector<std::string>> hostname = dieDB->query(sqlQuery.c_str());

			if(hostname.size() > 0)
				path.push_back(hostname[0][0]);
			else
			{
				//Error msg
			}

			//ulSrc = boost::asio::ip::address_v4::from_string(hostname[0][2]).to_ulong();
		}
		else
		{
			std::string logMsg = "Internal Error. This case is never ment to happen.";
			schreibeLog(WkLog::WkLog_NORMALTYPE, logMsg, "0206", WkLog::WkLog_ROT);
		}

		if (path.back().compare(0, 2, "H-") == 0)
		{
			*cloudPresent = true;

			done = true;
		}

		if (!done)
		{
			done = CheckIfTargetNetworkReached(src, dest);

			sqlQuery = "SELECT nextHop FROM l3routes ";
			sqlQuery += "INNER JOIN rlink ON rlink.l3routes_l3r_id = l3routes.l3r_id ";
			sqlQuery += "INNER JOIN interfaces ON rlink.interfaces_intf_id = interfaces.intf_id ";
			sqlQuery += "INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
			sqlQuery += "INNER JOIN device ON devInterface.device_dev_id = device.dev_id ";
			sqlQuery += "WHERE dev_id = " + deviceId + " AND ulSubnet<" + boost::lexical_cast<std::string>(dest) + " AND ulBroadcast>" + boost::lexical_cast<std::string>(dest);
			sqlQuery += " AND ulBroadcast - ulSubnet = (SELECT MIN(ulBroadcast - ulSubnet) ";
			sqlQuery += "FROM(";
			sqlQuery += "SELECT ulSubnet, ulBroadcast FROM l3routes ";
			sqlQuery += "INNER JOIN rlink ON rlink.l3routes_l3r_id = l3routes.l3r_id ";
			sqlQuery += "INNER JOIN interfaces ON rlink.interfaces_intf_id = interfaces.intf_id ";
			sqlQuery += "INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
			sqlQuery += "INNER JOIN device ON devInterface.device_dev_id = device.dev_id ";
			sqlQuery += "WHERE dev_id = " + deviceId + " AND ulSubnet<" + boost::lexical_cast<std::string>(dest) + " AND ulBroadcast>" + boost::lexical_cast<std::string>(dest) + "))";

			src = boost::asio::ip::address_v4::from_string((dieDB->query(sqlQuery.c_str()))[0][0]).to_ulong();					//ugliest line of code ever written
		}
	}

	return path;
}

WkmEnd2End::hostDetails WkmEnd2End::FilterResponse(std::vector<std::vector<std::string>> sqlBaseResponse)
{
	std::string sqlQuery;
	WkmEnd2End::hostDetails result;
	//std::string result; 
	std::string deviceId;

	std::vector<std::vector<std::string>> response; 

	std::vector<std::vector<std::string>>::iterator it = sqlBaseResponse.begin();


	while (it != sqlBaseResponse.end())
	{
		deviceId = (*it)[0];

		sqlQuery = "SELECT DISTINCT nextHop FROM l3routes ";
		sqlQuery += "INNER JOIN rlink ON rlink.l3routes_l3r_id = l3routes.l3r_id ";
		sqlQuery += "INNER JOIN interfaces ON interfaces.intf_id = rlink.interfaces_intf_id ";
		sqlQuery += "INNER JOIN devInterface ON interfaces.intf_id = devInterface.interfaces_int_id ";
		sqlQuery += "WHERE device_dev_id = " + deviceId + ";";

		response = dieDB->query(sqlQuery.c_str());

		if (response.size() > 0)
		{
			for (std::vector<std::vector<std::string>>::iterator nextHopsIt = response.begin(); nextHopsIt != response.end(); nextHopsIt++)
			{
				std::string nextHop = (*nextHopsIt)[0];

				sqlQuery = "SELECT dev_id, hostname, intfName FROM device ";
				sqlQuery += "INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id ";
				sqlQuery += "INNER JOIN interfaces ON devInterface.interfaces_int_id = interfaces.intf_id ";
				sqlQuery += "WHERE ipAddress LIKE '" + nextHop + "'";

				std::vector<std::vector<std::string>> targetDevIdResponse = dieDB->query(sqlQuery.c_str());

				for (std::vector<std::vector<std::string>>::iterator it2 = sqlBaseResponse.begin(); it2 != sqlBaseResponse.end(); it2++)
				{
					if (targetDevIdResponse[0][0] == (*it2)[0])
					{
						result.hdev_id.push_back((*it2)[0]);
						result.hHostname.push_back((*it2)[1]);
						result.hIntfName.push_back((*it2)[2]);
						result.hIp.push_back(nextHop);
						return result;
					}
				}
			}
		}
		it++;
	}

	it = sqlBaseResponse.begin();

	while (it != sqlBaseResponse.end())
	{
		deviceId = (*it)[0];

		sqlQuery = "SELECT type FROM device WHERE dev_id=" + deviceId; 

		response = dieDB->query(sqlQuery.c_str());

		if (response[0][0] == "3")
		{
			sqlQuery = "SELECT dev_id FROM device";
			sqlQuery += "INNER JOIN devInterface ON devInterface.device_dev ";
			sqlQuery +=  " WHERE foStatus LIKE \"%Active\" AND dev_id=" + deviceId;

			response = dieDB->query(sqlQuery.c_str());

			if(response.size()==0)
			{	}
			else if (response.size() == 1)
			{	
				result.hdev_id = *it;
				return result;
			}
			else if (response.size() > 1)
			{
				std::string logMsg = "Multiple DB entries for device ID " + deviceId + "! Also, more than one Active Firewall in a Failover Stack!";			//Message gehört mit passender Terminologie überarbeitet
				schreibeLog(WkLog::WkLog_NORMALTYPE, logMsg, "6209", WkLog::WkLog_ROT);
				result.errorCode = "ERROR";
				return result;
			}
			else
			{	
				std::string logMsg = "Internal Error. This case is never ment to happen.";
				schreibeLog(WkLog::WkLog_NORMALTYPE, logMsg, "0206", WkLog::WkLog_ROT);
			}
		}
		/*
		else if (response[0][0] == "1" || response[0][0] == "2"													NYI
			|| response[0][0] == "6" || response[0][0] == "8"
			|| response[0][0] == "9" || response[0][0] == "10")
		{
			sqlQuery = "SELECT dev_id FROM device";
			sqlQuery += "INNER JOIN devInterface ON devInterface.device_dev_id = device.dev_id";
			sqlQuery += "INNER JOIN interfaces ON interfaces.intf_id = devInterface.interfaces_int_id";
			sqlQuery += "INNER JOIN hsrplink ON hsrplink.interfaces_intf_id = interfaces.intf_id";
			sqlQuery += "INNER JOIN hsrp ON hsrp.hsrp_id = hsrplink.hsrp_hsrp_id";
			sqlQuery += "WHERE hsrp.state= XXXXXX AND device.dev_id=" + deviceId; //missing state

			response = dieDB->query(sqlQuery.c_str());


			//todo CHCK

		}
		*/
		it++;
	}
}

bool WkmEnd2End::CheckIfTargetNetworkReached(unsigned long src, unsigned long dest)				//not enough testet yet
{
	std::string sqlQuery = "SELECT ipSubnet_id FROM ipSubnet";
	sqlQuery += " WHERE ipSubnet.ulSubnet<" + boost::lexical_cast<std::string>(src);
	sqlQuery += " AND ipSubnet.ulBroadcast>" + boost::lexical_cast<std::string>(src);

	std::vector<std::vector<std::string>> srcNet = dieDB->query(sqlQuery.c_str());

	sqlQuery = "SELECT ipSubnet_id FROM ipSubnet";
	sqlQuery += " WHERE ipSubnet.ulSubnet<" + boost::lexical_cast<std::string>(dest);
	sqlQuery += " AND ipSubnet.ulBroadcast>" + boost::lexical_cast<std::string>(dest);

	std::vector<std::vector<std::string>> destNet = dieDB->query(sqlQuery.c_str());

	if (srcNet[0][0] == destNet[0][0])
		return true;
	else
		return false; 
}

WkmEnd2End::hostDetails WkmEnd2End::ipSearch(std::string ipAddr)
{
	hostDetails ergebnis;
	ergebnis.anzahl = 0;

	std::string sString = "SELECT DISTINCT l2_addr FROM neighbor WHERE l3_addr LIKE '" + ipAddr + "'";
	std::vector<std::vector<std::string>> l2A = dieDB->query(sString.c_str());
	if (!l2A.empty())
	{
		for (std::vector<std::vector<std::string>>::iterator it0 = l2A.begin(); it0 < l2A.end(); ++it0)
		{
			sString = "SELECT DISTINCT intf_id, intfname,intfType,description FROM interfaces INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
			sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE neighbor.l2_addr LIKE '";
			sString += it0->at(0) + "' AND l2l3 LIKE 'L2' AND device.hwtype=2 "; // 18.11.2014: Entfernt: --->AND intf_id NOT IN (SELECT int_vlan.interfaces_intf_id FROM int_vlan)  ";<---
			sString += "AND intf_id NOT IN (SELECT dI_intf_id FROM neighborship WHERE dI_intf_id1 IN (SELECT dI_intf_id1 FROM neighborship ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=neighborship.dI_intf_id1 INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
			sString += "WHERE device.hwtype=2))	AND intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id IN (SELECT dI_intf_id FROM neighborship ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
			sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.hwtype=2)) ORDER BY intf_id DESC";

			bool ersterDurchlauf = true;
			std::vector<std::vector<std::string>> intf = dieDB->query(sString.c_str());
			for (std::vector<std::vector<std::string>>::iterator it = intf.begin(); it < intf.end(); ++it)
			{
				sString = "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship UNION ALL SELECT dI_intf_id1 FROM neighborship) ";
				sString += "WHERE col IN (SELECT boundTo_id FROM interfaces WHERE intf_id=" + it->at(0) + ")";
				std::vector<std::vector<std::string>> check = dieDB->query(sString.c_str());
				if (!check.empty())
				{
					continue;
				}

				sString = "SELECT hostname,dev_id FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id WHERE devInterface.interfaces_int_id=" + it->at(0);
				std::vector<std::vector<std::string>> host = dieDB->query(sString.c_str());
				std::vector<std::vector<std::string>>::iterator it1 = host.begin();
				if (!host.empty())
				{
					ergebnis.status.push_back(true);
					ergebnis.hIp.push_back(ipAddr);
					ergebnis.hMac.push_back(it0->at(0));
					ergebnis.hdev_id.push_back(it1->at(1));
					ergebnis.hintf_id.push_back(it->at(0));
					ergebnis.hIntfDescr.push_back(it->at(3));
					ergebnis.hIntfType.push_back(it->at(2));
					ergebnis.hHostname.push_back(it1->at(0));
					ergebnis.hIntfName.push_back(it->at(1));
					ergebnis.anzahl++;

					ergebnis.hvlan.push_back("");
					ergebnis.hsubnet.push_back("");
					ergebnis.hdg.push_back("");

					// Zeitstempel:
					sString = "SELECT timestamp FROM rControl WHERE intf_id>=" + it->at(0) + " AND timestamp NOT NULL";
					std::vector<std::vector<std::string>> zeitstempel = dieDB->query(sString.c_str());
					std::vector<std::vector<std::string>>::iterator it2 = zeitstempel.begin();
					ergebnis.timestamp.push_back(it2->at(0));

				}
				else
				{
					std::string dbgA = "\n6207: DB error (hostSearch 0002). Please contact wktools@spoerr.org";
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6207",
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					break;
				}

				std::string oui = it0->at(0).substr(0, 7);
				sString = "SELECT vendor FROM macOUI WHERE oui LIKE '" + oui + "'";
				std::vector<std::vector<std::string>> vendor = dieDB->query(sString.c_str());
				if (!vendor.empty())
				{
					ergebnis.hOuiVendor.push_back(vendor[0][0]);
				}
				else
				{
					ergebnis.hOuiVendor.push_back("");
				}
				if (ersterDurchlauf)
				{
					ersterDurchlauf = false;
					// ergebnis += "\r\nHistory:\r\n";
				}

			}
		}		
	}

	return ergebnis;
}


void WkmEnd2End::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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



#endif

