#include "class_wkmMain.h"
#include "class_macSearchDialog.h"
#ifdef WKTOOLS_MAPPER

WkmMain::WkmMain(std::string sd, bool p, bool c, Wkm *wkm, WkLog *logA, std::string asgbe, std::string pattern, bool cluDB, bool clDB, std::string hfile, bool imp, bool lzr)
{
	sDir = sd;
	parse = p;
	combine = c;
	wkMapper = wkm;
	logAusgabe = logA;
	oFile = asgbe;
	cdp = true;
	cdp_hosts = true;
	unknownStp = true;
	sPattern = pattern;
	cleanupDB = cluDB;
	clearDB = clDB;
	hostfile = hfile;
	import = imp;
	lzReports = lzr;
	
	size_t pos = oFile.find_last_of("\\/");
	std::string oDir = oFile.substr(0, pos+1);

	std::string dbName = oDir + "wkm.db";
	dieDB = new WkmDB(dbName);
}


WkmMain::~WkmMain()
{
	delete dieDB;
}

int WkmMain::doIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6615: Database Init (wkm.db)\r\n", "6615", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);


	dieDB->query("CREATE TABLE IF NOT EXISTS device(dev_id INTEGER PRIMARY KEY AUTOINCREMENT, type INT, hwtype INT, dataSource INT, hostname TEXT, stpBridgeID TEXT, stpProtocol TEXT, snmpLoc TEXT);");
	dieDB->query("CREATE TABLE IF NOT EXISTS interfaces(intf_id INTEGER PRIMARY KEY AUTOINCREMENT, intfName TEXT, nameif TEXT, intfType TEXT, phl INT, macAddress TEXT, ipAddress TEXT, subnetMask TEXT, duplex TEXT, speed TEXT, status TEXT, description TEXT, l2l3 TEXT, errLvl INT, lastClear TEXT, loadLvl INT, channel_intf_id INT, vpc_id INT, l2AdminMode TEXT, l2Mode TEXT, l2encap TEXT, dtp TEXT, nativeAccess TEXT, voiceVlan TEXT, opPrivVlan TEXT, allowedVlan TEXT, iCrc TEXT, iFrame TEXT, iOverrun TEXT, iIgnored TEXT, iWatchdog TEXT, iPause TEXT, iDribbleCondition TEXT, ibuffer TEXT, l2decodeDrops TEXT, runts TEXT, giants TEXT, throttles TEXT, ierrors TEXT, underrun TEXT, oerrors TEXT, oCollisions TEXT, oBabbles TEXT, oLateColl TEXT, oDeferred TEXT, oLostCarrier TEXT, oNoCarrier TEXT, oPauseOutput TEXT, oBufferSwapped TEXT, resets TEXT, obuffer TEXT, CONSTRAINT fk_interfaces_interfaces1 FOREIGN KEY (channel_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS devInterface (interfaces_int_id INT, device_dev_id INT, cdp_cdp_id INT, PRIMARY KEY (interfaces_int_id, device_dev_id, cdp_cdp_id), CONSTRAINT fk_dev_interface_interfaces1 FOREIGN KEY (interfaces_int_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_dev_interface_device1 FOREIGN KEY (device_dev_id) REFERENCES device (dev_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_devInterface_cdp1 FOREIGN KEY (cdp_cdp_id) REFERENCES cdp (cdp_id) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS neighborship (n_id INTEGER PRIMARY KEY AUTOINCREMENT, dI_intf_id INT, dI_dev_id INT, dI_intf_id1 INT, dI_dev_id1 INT, flag INT NULL, CONSTRAINT fk_interfaces_has_interfaces_interfaces1 FOREIGN KEY (dI_intf_id , dI_dev_id) REFERENCES devInterface (interfaces_int_id, device_dev_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_interfaces_has_interfaces_interfaces2 FOREIGN KEY (dI_intf_id1 , dI_dev_id1) REFERENCES devInterface (interfaces_int_id , device_dev_id) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS stpInstanz (stp_id INTEGER PRIMARY KEY AUTOINCREMENT, stpPriority TEXT NULL, rootPort TEXT NULL);");
	dieDB->query("CREATE TABLE IF NOT EXISTS vlan (vlan_id INTEGER PRIMARY KEY AUTOINCREMENT, vlan INT NULL);");
	dieDB->query("CREATE TABLE IF NOT EXISTS vlan_has_device(vlan_vlan_id INT, device_dev_id INT, PRIMARY KEY (vlan_vlan_id, device_dev_id), CONSTRAINT fk_vlan_has_device_vlan1  FOREIGN KEY (vlan_vlan_id) REFERENCES vlan (vlan_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_vlan_has_device_device1 FOREIGN KEY (device_dev_id) REFERENCES device (dev_id) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS stp_status(stp_status_id INTEGER PRIMARY KEY AUTOINCREMENT, stpIntfStatus TEXT NULL, designatedRootID TEXT, designatedBridgeID TEXT, stpTransitionCount INTEGER);");
	dieDB->query("CREATE TABLE IF NOT EXISTS int_vlan(interfaces_intf_id INT, vlan_vlan_id INT, stp_status_stp_status_id INT, PRIMARY KEY (interfaces_intf_id, vlan_vlan_id, stp_status_stp_status_id), CONSTRAINT fk_interfaces_has_vlan_interfaces1  FOREIGN KEY (interfaces_intf_id ) REFERENCES interfaces (intf_id ) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_interfaces_has_vlan_vlan1 FOREIGN KEY (vlan_vlan_id ) REFERENCES vlan (vlan_id ) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_int_vlan_stp_status1 FOREIGN KEY (stp_status_stp_status_id ) REFERENCES stp_status (stp_status_id ) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS vlan_stpInstanz (vlan_vlan_id INTEGER, stpInstanz_stp_id INTEGER, PRIMARY KEY (vlan_vlan_id, stpInstanz_stp_id), CONSTRAINT fk_vlan_stpInstanz_vlan1 FOREIGN KEY (vlan_vlan_id ) REFERENCES vlan (vlan_id ) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_vlan_stpInstanz_stpInstanz1 FOREIGN KEY (stpInstanz_stp_id )  REFERENCES stpInstanz (stp_id ) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS neighbor (neighbor_id INTEGER PRIMARY KEY AUTOINCREMENT, l2_addr TEXT NULL , l3_addr TEXT NULL);");
	dieDB->query("CREATE TABLE IF NOT EXISTS nlink (neighbor_neighbor_id INT, interfaces_intf_id INT, PRIMARY KEY (neighbor_neighbor_id, interfaces_intf_id), CONSTRAINT fk_table1_neighbor1  FOREIGN KEY (neighbor_neighbor_id ) REFERENCES neighbor (neighbor_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_table1_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS cdp (cdp_id INTEGER PRIMARY KEY AUTOINCREMENT, hostname TEXT, nName TEXT, alternatenName TEXT, intf TEXT, nIntfIP TEXT, nIntf TEXT, type INT, platform TEXT, sw_version TEXT);");
	dieDB->query("CREATE TABLE IF NOT EXISTS ipSubnet (ipSubnet_id INTEGER PRIMARY KEY AUTOINCREMENT, subnet TEXT NULL, mask TEXT NULL);");
	dieDB->query("CREATE TABLE IF NOT EXISTS intfSubnet (ipSubnet_ipSubnet_id INTEGER, interfaces_intf_id INTEGER, PRIMARY KEY (ipSubnet_ipSubnet_id, interfaces_intf_id), CONSTRAINT fk_intfSubnet_ipSubnet1 FOREIGN KEY (ipSubnet_ipSubnet_id) REFERENCES ipSubnet (ipSubnet_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_intfSubnet_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id)  ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS rControl (rc_id INTEGER PRIMARY KEY AUTOINCREMENT, n_id INTEGER, cdp_id INTEGER, dev_id INTEGER, intf_id INTEGER, neighbor_id INTEGER, vlan_id INTEGER, ipSubnet_id INTEGER, stp_status_id INTEGER, stp_id INTEGER, l3r_id INTEGER, ipphone_id INTEGER, hwinf_id INTEGER, license_id INTEGER, flash_id INTEGER, timestamp TEXT);");
	dieDB->query("CREATE TABLE IF NOT EXISTS l3routes (l3r_id INTEGER PRIMARY KEY AUTOINCREMENT, protocol TEXT NULL, network TEXT NULL, subnet TEXT NULL, nextHop TEXT NULL, interface TEXT NULL, type TEXT NULL);");
	dieDB->query("CREATE TABLE IF NOT EXISTS rlink (interfaces_intf_id INT NOT NULL, l3routes_l3r_id INT NOT NULL, PRIMARY KEY (interfaces_intf_id, l3routes_l3r_id), CONSTRAINT fk_rlink_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_rlink_l3routes1 FOREIGN KEY (l3routes_l3r_id) REFERENCES l3routes (l3r_id) ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS ipPhoneDet (ipphone_id INTEGER PRIMARY KEY AUTOINCREMENT, voiceVlan TEXT, cm1 TEXT, cm2 TEXT, dscpSig TEXT, dscpConf TEXT, dscpCall TEXT, defGW TEXT);");
	dieDB->query("CREATE TABLE IF NOT EXISTS ipphonelink (ipPhoneDet_ipphone_id INT, device_dev_id INT NOT NULL ,PRIMARY KEY (ipPhoneDet_ipphone_id, device_dev_id), CONSTRAINT fk_ipphonelink_ipPhoneDet1  FOREIGN KEY (ipPhoneDet_ipphone_id )  REFERENCES ipPhoneDet (ipphone_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_ipphonelink_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS hwInfo (hwinf_id INTEGER PRIMARY KEY AUTOINCREMENT, module TEXT ,type TEXT ,description TEXT ,sn TEXT ,hwRevision TEXT ,memory TEXT ,bootfile TEXT ,sw_version TEXT);");
	dieDB->query("CREATE TABLE IF NOT EXISTS hwlink (device_dev_id INT, hwInfo_hwinf_id INT NOT NULL ,PRIMARY KEY (device_dev_id, hwInfo_hwinf_id), CONSTRAINT fk_hwlink_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_hwlink_hwInfo1  FOREIGN KEY (hwInfo_hwinf_id )  REFERENCES hwInfo (hwinf_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS license (license_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, status TEXT, type TEXT, nextBoot TEXT, cluster INT);");
	dieDB->query("CREATE TABLE IF NOT EXISTS liclink (license_license_id INT, device_dev_id INT NOT NULL ,PRIMARY KEY (license_license_id, device_dev_id), CONSTRAINT fk_table1_license1  FOREIGN KEY (license_license_id )  REFERENCES license (license_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_table1_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
	dieDB->query("CREATE TABLE IF NOT EXISTS flash (flash_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT ,size TEXT ,free TEXT);");
	dieDB->query("CREATE TABLE IF NOT EXISTS flashlink (flash_flash_id INT, device_dev_id INT NOT NULL ,PRIMARY KEY (flash_flash_id, device_dev_id), CONSTRAINT fk_flashlink_flash1  FOREIGN KEY (flash_flash_id )  REFERENCES flash (flash_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_flashlink_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE);");

	if (clearDB)
	{
		dieDB->query("DELETE FROM device");
		dieDB->query("DELETE FROM interfaces");
		dieDB->query("DELETE FROM devInterface");
		dieDB->query("DELETE FROM stpInstanz");
		dieDB->query("DELETE FROM vlan");
		dieDB->query("DELETE FROM vlan_has_device");
		dieDB->query("DELETE FROM stp_status");
		dieDB->query("DELETE FROM int_vlan");
		dieDB->query("DELETE FROM vlan_stpInstanz");
		dieDB->query("DELETE FROM neighbor");
		dieDB->query("DELETE FROM nlink");
		dieDB->query("DELETE FROM cdp");
		dieDB->query("DELETE FROM ipSubnet");
		dieDB->query("DELETE FROM intfSubnet");
		dieDB->query("DELETE FROM neighborship");
		dieDB->query("DELETE FROM rControl");
		dieDB->query("DELETE FROM l3routes");
		dieDB->query("DELETE FROM rlink");
		dieDB->query("DELETE FROM ipPhoneDet");
		dieDB->query("DELETE FROM ipphonelink");
		dieDB->query("DELETE FROM hwInfo");
		dieDB->query("DELETE FROM hwlink");
		dieDB->query("DELETE FROM license");
		dieDB->query("DELETE FROM liclink");
		dieDB->query("DELETE FROM flash");
		dieDB->query("DELETE FROM flashlink");
	}
	else if (cleanupDB)
	{
		dieDB->query("DELETE FROM device WHERE dev_id <= (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl))");
		dieDB->query("DELETE FROM interfaces WHERE intf_id <= (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl))");
		dieDB->query("DELETE FROM devInterface WHERE device_dev_id <= (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl))");
		dieDB->query("DELETE FROM stpInstanz WHERE stp_id <= (SELECT MAX(stp_id) FROM rControl WHERE stp_id < (SELECT MAX(stp_id) FROM rControl))");
		dieDB->query("DELETE FROM vlan WHERE vlan_id <= (SELECT MAX(vlan_id) FROM rControl WHERE vlan_id < (SELECT MAX(vlan_id) FROM rControl))");
		dieDB->query("DELETE FROM vlan_has_device WHERE vlan_vlan_id <= (SELECT MAX(vlan_id) FROM rControl WHERE vlan_id < (SELECT MAX(vlan_id) FROM rControl))");
		dieDB->query("DELETE FROM stp_status WHERE stp_status_id <= (SELECT MAX(stp_status_id) FROM rControl WHERE stp_status_id < (SELECT MAX(stp_status_id) FROM rControl))");
		dieDB->query("DELETE FROM int_vlan WHERE vlan_vlan_id <= (SELECT MAX(vlan_id) FROM rControl WHERE vlan_id < (SELECT MAX(vlan_id) FROM rControl))");
		dieDB->query("DELETE FROM vlan_stpInstanz WHERE vlan_vlan_id <= (SELECT MAX(vlan_id) FROM rControl WHERE vlan_id < (SELECT MAX(vlan_id) FROM rControl))");
		dieDB->query("DELETE FROM neighbor WHERE neighbor_id <= (SELECT MAX(neighbor_id) FROM rControl WHERE neighbor_id < (SELECT MAX(neighbor_id) FROM rControl))");
		dieDB->query("DELETE FROM nlink WHERE neighbor_neighbor_id <= (SELECT MAX(neighbor_id) FROM rControl WHERE neighbor_id < (SELECT MAX(neighbor_id) FROM rControl))");
		dieDB->query("DELETE FROM cdp WHERE cdp_id <= (SELECT MAX(cdp_id) FROM rControl WHERE cdp_id < (SELECT MAX(cdp_id) FROM rControl))");
		dieDB->query("DELETE FROM ipSubnet WHERE ipSubnet_id <= (SELECT MAX(ipSubnet_id) FROM rControl WHERE ipSubnet_id < (SELECT MAX(ipSubnet_id) FROM rControl))");
		dieDB->query("DELETE FROM intfSubnet WHERE ipSubnet_ipSubnet_id <= (SELECT MAX(ipSubnet_id) FROM rControl WHERE ipSubnet_id < (SELECT MAX(ipSubnet_id) FROM rControl))");
		dieDB->query("DELETE FROM neighborship WHERE n_id <= (SELECT MAX(n_id) FROM rControl WHERE n_id < (SELECT MAX(n_id) FROM rControl))");
		dieDB->query("DELETE FROM rControl WHERE rc_id < (SELECT MAX(rc_id) FROM rControl) AND rc_id > 1");
		dieDB->query("DELETE FROM l3routes WHERE l3r_id < (SELECT MAX(l3r_id) FROM rControl WHERE l3r_id < (SELECT MAX(l3r_id) FROM rControl))");
		dieDB->query("DELETE FROM rlink WHERE l3routes_l3r_id < (SELECT MAX(l3r_id) FROM rControl WHERE l3r_id < (SELECT MAX(l3r_id) FROM rControl))");
		dieDB->query("DELETE FROM ipPhoneDet WHERE ipphone_id <= (SELECT MAX(ipphone_id) FROM rControl WHERE ipphone_id < (SELECT MAX(ipphone_id) FROM rControl))");
		dieDB->query("DELETE FROM hwInfo WHERE hwinf_id <= (SELECT MAX(hwinf_id) FROM rControl WHERE hwinf_id < (SELECT MAX(hwinf_id) FROM rControl))");
		dieDB->query("DELETE FROM license WHERE license_id <= (SELECT MAX(license_id) FROM rControl WHERE license_id < (SELECT MAX(license_id) FROM rControl))");
		dieDB->query("DELETE FROM flash WHERE flash_id <= (SELECT MAX(flash_id) FROM rControl WHERE flash_id < (SELECT MAX(flash_id) FROM rControl))");
		dieDB->query("DELETE FROM ipphonelink WHERE ipPhoneDet_ipphone_id <= (SELECT MAX(ipphone_id) FROM rControl WHERE ipphone_id < (SELECT MAX(ipphone_id) FROM rControl))");
		dieDB->query("DELETE FROM hwlink WHERE hwInfo_hwinf_id <= (SELECT MAX(hwinf_id) FROM rControl WHERE hwinf_id < (SELECT MAX(hwinf_id) FROM rControl))");
		dieDB->query("DELETE FROM liclink WHERE license_license_id <= (SELECT MAX(license_id) FROM rControl WHERE license_id < (SELECT MAX(license_id) FROM rControl))");
		dieDB->query("DELETE FROM flashlink WHERE flash_flash_id <= (SELECT MAX(flash_id) FROM rControl WHERE flash_id < (SELECT MAX(flash_id) FROM rControl))");
	}

	if (parse)
	{

		schreibeLog(WkLog::WkLog_ZEIT, "6616: Start Parser\r\n", "6616", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

		WkmParser *derParser = new WkmParser(sDir, dieDB, logAusgabe);
		derParser->startParser(sPattern);
		delete derParser;
	}

	if (import)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6623: Start Import\r\n", "6616", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		WkmImport *derImporter = new WkmImport(hostfile, dieDB, logAusgabe);
		derImporter->startParser();
		delete derImporter;
	}


	// Ausgabe nur machen, wenn DB nicht leer
	std::string sString = "SELECT dev_id FROM device";
	std::vector<std::vector<std::string>> dbCheck = dieDB->query(sString.c_str());

	if (!dbCheck.empty())
	{
		if (combine)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "6617: Computing adjacencies\r\n", "6617", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
			
			// Wenn nur analysiert ohne neu geparst wird, dann muss n_id beim letzten Eintrag zurückgesetzt werden
			// und die letzte Nachbraschaftsberechnung gelöscht werden
			if (!parse)
			{
				sString = "DELETE FROM neighborship WHERE n_id > (SELECT MAX(n_id) FROM rControl WHERE n_id < (SELECT MAX(n_id) FROM rControl))";
				dieDB->query(sString.c_str());
				sString = "UPDATE rControl SET n_id=NULL WHERE n_id=(SELECT MAX(n_id) FROM rControl)";
				dieDB->query(sString.c_str());
			}
			
			WkmCombiner *derCombiner = new WkmCombiner(dieDB, logAusgabe);

			derCombiner->doIt();
			delete derCombiner;
		}


		schreibeLog(WkLog::WkLog_ZEIT, "6618: Generating output file\r\n", "6618", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		WkmAusgabe *dieAusgabe = new WkmAusgabe(dieDB, logAusgabe, oFile);
		dieAusgabe->sets(cdp, cdp_hosts, unknownStp, l2, l3, org, hier, uel, ld, nd, wb, lzReports);
		dieAusgabe->doIt();
		delete dieAusgabe;
	}
	else
	{
		std::string dbgA = "6306: Unable to calculate adjacencies because Database is empty. Please check if \"Parse New\" and \"Analyze New\" is checked under Options.";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}


	return 0;

}

void WkmMain::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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


void WkmMain::setAusgabeSets(bool c, bool ch, bool u, bool layer2, bool layer3, bool o, bool h, std::string unitEdgeLength,	
							 std::string lDistance, std::string nDistance, std::string weightBalancing)
{
	cdp = c;
	cdp_hosts = ch;
	unknownStp = u;
	l2 = layer2;
	l3 = layer3;
	hier = h;
	org = o;
	nd = nDistance;
	ld = lDistance;
	wb = weightBalancing;
	uel = unitEdgeLength;
}


int WkmMain::doMacSearch()
{
	MacSearchDialog *sucheHost = new MacSearchDialog(wxT("Search Host"));
	sucheHost->Show(true);
	sucheHost->setEinstellungen(dieDB, logAusgabe);
	return 1;
}

#endif // #ifdef WKTOOLS_MAPPER
