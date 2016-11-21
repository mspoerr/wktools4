#include "class_wkm.h"
#include "wktools4.h"
#ifdef WKTOOLS_MAPPER

DEFINE_LOCAL_EVENT_TYPE(wkEVT_MAPPER_FERTIG)

BEGIN_EVENT_TABLE(Wkm, wxEvtHandler)
EVT_COMMAND(wxID_ANY, wkEVT_MAPPER_FERTIG, Wkm::OnWkmFertig)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Wkm::Wkm() : wxEvtHandler()
{
	vars[CWkmPropGrid::WKM_PROPID_STR_SDIR] = "";
	vars[CWkmPropGrid::WKM_PROPID_STR_OFILE] = "";
	vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE] = "log.txt";
	vars[CWkmPropGrid::WKM_PROPID_STR_HOSTFILE] = "";
	vars[CWkmPropGrid::WKM_PROPID_STR_PATTERN] = "";

	vari[CWkmPropGrid::WKM_PROPID_INT_PARSE] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_COMBINE] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB_ALL] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDB] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDBNEW] = 1;

	vari[CWkmPropGrid::WKM_PROPID_INT_L2] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_L3] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_L3_ROUTING] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_ORGANIC] = 1;
	vari[CWkmPropGrid::WKM_PROPID_INT_HIERARCHIC] = 0;
	vari[CWkmPropGrid::WKM_PROPID_INT_ORTHO] = 0;

	vari[CWkmPropGrid::WKM_PROPID_INT_DNS_RESOLUTION] = 0;
	vari[CWkmPropGrid::WKM_PROPID_INT_ENDSYSTEMS] = 0;
	vari[CWkmPropGrid::WKM_PROPID_INT_ROUTENEIGHBORS] = 0;
	vari[CWkmPropGrid::WKM_PROPID_INT_IMP_ES] = 0;

	vari[CWkmPropGrid::WKM_PROPID_INT_INTFSTYLE] = 0;

	vars[CWkmPropGrid::WKM_PROPID_STR_UNITEDGELENGTH] = "100.0";
	vars[CWkmPropGrid::WKM_PROPID_STR_LAYERDISTANCE] = "50.0";
	vars[CWkmPropGrid::WKM_PROPID_STR_NODEDISTANCE] = "50.0";
	vars[CWkmPropGrid::WKM_PROPID_STR_WEIHTBALANCING] = "0.1";


	profilIndex = 0;
	logAusgabe = NULL;
	wkMain = NULL;

	stopThread = false;
	ende = false;
	mitThread = true;

	wkmdbg = false;

	derCombiner = NULL;
	derParser = NULL;
	dieDB = NULL;

	a1 = 9;
	a2 = 20;

}


// Destruktor:
Wkm::~Wkm()
{
	delete derParser;
	derParser = NULL;
	delete derCombiner;
	derCombiner = NULL;
}


// void DoIt:
// Tool Start; Es werden die Einstellungen überprüft und an den Parser übergeben
void Wkm::doIt(bool wt)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "6601: Mapper START\r\n", "6601", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	bool fehler = doSave("");
	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6602: Starting Mapper\r\n", "6602", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

		if (wt)
		{
			thrd = boost::thread(boost::bind(&Wkm::startParser,this));
		}
		else
		{
			mitThread = false;
			startParser();
		}
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6402: Cannot Start because of wrong or missing entries!\r\n", "6402", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);

		wxCommandEvent event(wkEVT_MAPPER_FERTIG);
		event.SetInt(2);
		wxPostEvent(this, event);
	}
}


void Wkm::startParser()
{
	bool starteTool = true;		// Zeigt an, ob das Tool gestartet werden soll
	schreibeLog(WkLog::WkLog_ZEIT, "6615: Database Init (wkm.db)\r\n", "6615", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	size_t pos = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].find_last_of("\\/");
	std::string oDir = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].substr(0, pos+1);

	std::string fehlerString = "";

	std::string dbName = oDir + "wkm.db";
	dieDB = new WkmDB(dbName, vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDB]);

	// Check, ob die DB neu ist
	std::string sString = "SELECT name FROM sqlite_master WHERE type='table' AND name='device';";
	std::vector<std::vector<std::string> > dbCheck = dieDB->query(sString.c_str());
	if (!dbCheck.empty())
	{
		// DB Version Check; Die DB Version wird ab 4.6.8.0 in rControl geschrieben, damit bei einem Upgrade nicht immer wkm.db gelöscht werden muss
		// Die DB muss nur bei groben Änderungen gelöscht werden. Wenn nur Spalten hinzugefügt werden müssen, genügt ein ALTER TABLE
		// Die DB Version wird im class_wkm.h Header gesetzt: static const int wkmDbVersion = x;
		sString = "SELECT dbVer FROM rControl WHERE rc_id IN (SELECT MAX(rc_id) FROM rControl)";
		std::vector<std::vector<std::string> > verRet = dieDB->query(sString.c_str());
		int aktdbVer = 0;
		if (!verRet.empty())
		{
			try
			{
				aktdbVer = boost::lexical_cast<int>(verRet[0][0]);
			}
			catch (boost::bad_lexical_cast &)
			{
				aktdbVer = 0;			
			}
		}

		// Check ob ALTER TABLE genügt, oder ob die komplette DB neu geschrieben werden muss
		if (aktdbVer < 1)
		{
			dieDB->query("DROP TABLE device");
			dieDB->query("DROP TABLE crypto");
			dieDB->query("DROP TABLE interfaces");
			dieDB->query("DROP TABLE devInterface");
			dieDB->query("DROP TABLE stpInstanz");
			dieDB->query("DROP TABLE vlan");
			dieDB->query("DROP TABLE vlan_has_device");
			dieDB->query("DROP TABLE stp_status");
			dieDB->query("DROP TABLE int_vlan");
			dieDB->query("DROP TABLE vlan_stpInstanz");
			dieDB->query("DROP TABLE neighbor");
			dieDB->query("DROP TABLE nlink");
			dieDB->query("DROP TABLE cdp");
			dieDB->query("DROP TABLE clink");
			dieDB->query("DROP TABLE ipSubnet");
			dieDB->query("DROP TABLE intfSubnet");
			dieDB->query("DROP TABLE neighborship");
			dieDB->query("DROP TABLE rControl");
			dieDB->query("DROP TABLE l3routes");
			dieDB->query("DROP TABLE rlink");
			dieDB->query("DROP TABLE ipPhoneDet");
			dieDB->query("DROP TABLE ipphonelink");
			dieDB->query("DROP TABLE hwInfo");
			dieDB->query("DROP TABLE hwlink");
			dieDB->query("DROP TABLE license");
			dieDB->query("DROP TABLE liclink");
			dieDB->query("DROP TABLE flash");
			dieDB->query("DROP TABLE flashlink");
			dieDB->query("DROP TABLE vrflink");
			dieDB->query("DROP TABLE cryptolink");
			dieDB->query("DROP TABLE rpNeighborLink");
			dieDB->query("DROP TABLE rpNeighbor");
			dieDB->query("DROP TABLE ospf");
			dieDB->query("DROP TABLE ospflink");
			dieDB->query("DROP TABLE ospfIntflink");
			dieDB->query("DROP TABLE hsrplink");
			dieDB->query("DROP TABLE hsrp");
			dieDB->query("DROP TABLE auth");
			dieDB->query("DROP TABLE intfAuth");
			dieDB->query("DROP TABLE dhcpSnooping");
			dieDB->query("DROP TABLE dhcpSnoopLink");
			dieDB->query("DROP TABLE devTracking");
			dieDB->query("DROP TABLE devTrackingLink");
		}
		else
		{
			if (aktdbVer < 2)
			{
				// DB Version 1 Upgrade auf 2
				std::string iString = "ALTER TABLE device ADD COLUMN vpcDomId TEXT";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 3)
			{
				// DB Version 2 Upgrade auf 3
				std::string iString = "ALTER TABLE interfaces ADD COLUMN cryptoMapTag TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN cryptoLocalIP TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE rControl ADD COLUMN crypto_id INTEGER";
				dieDB->query(iString.c_str());
			}			
			if (aktdbVer < 4)
			{
				// DB Version 3 Upgrade auf 4
				std::string iString = "ALTER TABLE neighborship ADD COLUMN priority INTEGER";
				dieDB->query(iString.c_str());
			}			
			if (aktdbVer < 5)
			{
				// DB Version 4 Upgrade auf 5
				std::string iString = "ALTER TABLE stp_status ADD COLUMN statsOnly INTEGER";
				dieDB->query(iString.c_str());
			}			
			if (aktdbVer < 6)
			{
				// DB Version 5 Upgrade auf 6
				std::string iString = "ALTER TABLE device ADD COLUMN foStatus TEXT";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 7)
			{
				// DB Version 6 Upgrade auf 7
				std::string iString = "ALTER TABLE device ADD COLUMN bgpAS TEXT";
				dieDB->query(iString.c_str());
//				iString = "ALTER TABLE device ADD COLUMN routerID TEXT";
//				dieDB->query(iString.c_str());
				iString = "ALTER TABLE rControl ADD COLUMN rpNeighbor_id INTEGER";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE rControl ADD COLUMN ospf_id INTEGER";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE rControl ADD COLUMN rpn_id INTEGER";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN cryptoRemoteIP TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN area TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN nwType TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN cost TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE l3routes ADD COLUMN ulSubnet INTEGER";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE l3routes ADD COLUMN ulMask INTEGER";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE l3routes ADD COLUMN ulBroadcast INTEGER";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 8)
			{
				// DB Version 7 Upgrade auf 8
				std::string iString = "ALTER TABLE rControl ADD COLUMN hsrp_id INTEGER";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 9)
			{
				// DB Version 8 Upgrade auf 9
				std::string iString = "ALTER TABLE crypto ADD COLUMN tag TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE crypto ADD COLUMN seqnr TEXT";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 10)
			{
				// DB Version 9 Upgrade auf 10
				std::string iString = "ALTER TABLE interfaces ADD COLUMN dot1x TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN mab TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN dot1xSysAuthCtrl TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN dot1xVer TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE rControl ADD COLUMN auth_id INTEGER";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 11)
			{
				// DB Version 10 Upgrade auf 11
				std::string iString = "ALTER TABLE interfaces ADD COLUMN l2x REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN l2y REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN l3x REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN l3y REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN l3rx REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE interfaces ADD COLUMN l3ry REAL";
				dieDB->query(iString.c_str());

				iString = "ALTER TABLE device ADD COLUMN l2x REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN l2y REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN l3x REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN l3y REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN l3rx REAL";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN l3ry REAL";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 12)
			{
				// DB Version 11 Upgrade auf 12
				std::string iString = "ALTER TABLE interfaces ADD COLUMN dhcpSnoopTrust TEXT";
				dieDB->query(iString.c_str());

				iString = "ALTER TABLE device ADD COLUMN devTrackingStatus TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN devTrackingProbeCount TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN devTrackingProbeInterval TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN devTrackingProbeDelay TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN dhcpSnoopVlans TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN dhcpSnoop82 TEXT";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE device ADD COLUMN dhcpSnoopState TEXT";
				dieDB->query(iString.c_str());

				iString = "ALTER TABLE rControl ADD COLUMN devTracking_id INTEGER";
				dieDB->query(iString.c_str());
				iString = "ALTER TABLE rControl ADD COLUMN dhcpSnoop_id INTEGER";
				dieDB->query(iString.c_str());
			}
			if (aktdbVer < 13)
			{
				// DB Version 12 Upgrade auf 13
				std::string iString = "ALTER TABLE interfaces ADD COLUMN intfColor TEXT";
				dieDB->query(iString.c_str());
			}
		}
	}

	if (starteTool)
	{
		// DB Init
		dieDB->query("CREATE TABLE IF NOT EXISTS device(dev_id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER, hwtype INTEGER, dataSource INTEGER, hostname TEXT, stpBridgeID TEXT, stpProtocol TEXT, snmpLoc TEXT, confReg TEXT, poeStatus TEXT, poeCurrent TEXT, poeMax TEXT, poeRemaining TEXT, vpcDomId TEXT, foStatus TEXT, bgpAS TEXT, dot1xSysAuthCtrl TEXT, dot1xVer TEXT, l2x REAL, l2y REAL, l3x REAL, l3y REAL, l3rx REAL, l3ry REAL, devTrackingStatus TEXT, devTrackingProbeCount TEXT, devTrackingProbeInterval TEXT, devTrackingProbeDelay TEXT, dhcpSnoopState TEXT, dhcpSnoopVlans TEXT, dhcpSnoop82 TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS interfaces(intf_id INTEGER PRIMARY KEY AUTOINCREMENT, intfName TEXT, nameif TEXT, intfType TEXT, phl INTEGER, macAddress TEXT, ipAddress TEXT, subnetMask TEXT, duplex TEXT, speed TEXT, status TEXT, errDisabledCause TEXT, description TEXT, l2l3 TEXT, errLvl INTEGER, lastClear TEXT, lastInput TEXT, loadLvl INTEGER, channel_intf_id INTEGER, vpc_id INTEGER, boundTo_id INTEGER, actModule TEXT, owner TEXT, portProfile TEXT, l2AdminMode TEXT, l2Mode TEXT, l2encap TEXT, dtp TEXT, nativeAccess TEXT, voiceVlan TEXT, opPrivVlan TEXT, allowedVlan TEXT, iCrc TEXT, iFrame TEXT, iOverrun TEXT, iIgnored TEXT, iWatchdog TEXT, iPause TEXT, iDribbleCondition TEXT, ibuffer TEXT, l2decodeDrops TEXT, runts TEXT, giants TEXT, throttles TEXT, ierrors TEXT, underrun TEXT, oerrors TEXT, oCollisions TEXT, oBabbles TEXT, oLateColl TEXT, oDeferred TEXT, oLostCarrier TEXT, oNoCarrier TEXT, oPauseOutput TEXT, oBufferSwapped TEXT, resets TEXT, obuffer TEXT, poeStatus TEXT, poeWatt TEXT, poeWattmax TEXT, poeDevice TEXT, cryptoMapTag TEXT, cryptoLocalIP TEXT, cryptoRemoteIP TEXT, area TEXT, nwType TEXT, cost TEXT, dot1x TEXT, mab TEXT, l2x REAL, l2y REAL, l3x REAL, l3y REAL, l3rx REAL, l3ry REAL, dhcpSnoopTrust TEXT, intfColor TEXT, CONSTRAINT fk_interfaces_interfaces1 FOREIGN KEY (channel_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS devInterface (interfaces_int_id INTEGER, device_dev_id INTEGER, PRIMARY KEY (interfaces_int_id, device_dev_id), CONSTRAINT fk_dev_interface_interfaces1 FOREIGN KEY (interfaces_int_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_dev_interface_device1 FOREIGN KEY (device_dev_id) REFERENCES device (dev_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_devInterface_cdp1 );");
		dieDB->query("CREATE TABLE IF NOT EXISTS neighborship (n_id INTEGER PRIMARY KEY AUTOINCREMENT, dI_intf_id INTEGER, dI_dev_id INTEGER, dI_intf_id1 INTEGER, dI_dev_id1 INTEGER, flag INTEGER NULL, priority INTEGER NULL, CONSTRAINT fk_interfaces_has_interfaces_interfaces1 FOREIGN KEY (dI_intf_id , dI_dev_id) REFERENCES devInterface (interfaces_int_id, device_dev_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_interfaces_has_interfaces_interfaces2 FOREIGN KEY (dI_intf_id1 , dI_dev_id1) REFERENCES devInterface (interfaces_int_id , device_dev_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS rpneighborship(rpn_id INTEGER PRIMARY KEY AUTOINCREMENT, dI_intf_id INTEGER, dI_dev_id INTEGER, dI_intf_id1 INTEGER, dI_dev_id1 INTEGER, flag INTEGER, rp TEXT, CONSTRAINT fk_rpneighborship_devInterface1 FOREIGN KEY(dI_intf_id, dI_dev_id) REFERENCES devInterface(interfaces_int_id, device_dev_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_rpneighborship_devInterface2 FOREIGN KEY(dI_intf_id1, dI_dev_id1) REFERENCES devInterface(interfaces_int_id, device_dev_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS stpInstanz (stp_id INTEGER PRIMARY KEY AUTOINCREMENT, stpPriority TEXT NULL, rootPort TEXT NULL);");
		dieDB->query("CREATE TABLE IF NOT EXISTS vlan (vlan_id INTEGER PRIMARY KEY AUTOINCREMENT, vlan INTEGER NULL);");
		dieDB->query("CREATE TABLE IF NOT EXISTS vlan_has_device(vlan_vlan_id INTEGER, device_dev_id INTEGER, PRIMARY KEY (vlan_vlan_id, device_dev_id), CONSTRAINT fk_vlan_has_device_vlan1  FOREIGN KEY (vlan_vlan_id) REFERENCES vlan (vlan_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_vlan_has_device_device1 FOREIGN KEY (device_dev_id) REFERENCES device (dev_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS stp_status(stp_status_id INTEGER PRIMARY KEY AUTOINCREMENT, stpIntfStatus TEXT NULL, designatedRootID TEXT, designatedBridgeID TEXT, stpTransitionCount INTEGER, statsOnly INTEGER);");
		dieDB->query("CREATE TABLE IF NOT EXISTS int_vlan(interfaces_intf_id INTEGER, vlan_vlan_id INTEGER, stp_status_stp_status_id INTEGER, PRIMARY KEY (interfaces_intf_id, vlan_vlan_id, stp_status_stp_status_id), CONSTRAINT fk_interfaces_has_vlan_interfaces1  FOREIGN KEY (interfaces_intf_id ) REFERENCES interfaces (intf_id ) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_interfaces_has_vlan_vlan1 FOREIGN KEY (vlan_vlan_id ) REFERENCES vlan (vlan_id ) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_int_vlan_stp_status1 FOREIGN KEY (stp_status_stp_status_id ) REFERENCES stp_status (stp_status_id ) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS vlan_stpInstanz (vlan_vlan_id INTEGER, stpInstanz_stp_id INTEGER, PRIMARY KEY (vlan_vlan_id, stpInstanz_stp_id), CONSTRAINT fk_vlan_stpInstanz_vlan1 FOREIGN KEY (vlan_vlan_id ) REFERENCES vlan (vlan_id ) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_vlan_stpInstanz_stpInstanz1 FOREIGN KEY (stpInstanz_stp_id )  REFERENCES stpInstanz (stp_id ) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS neighbor (neighbor_id INTEGER PRIMARY KEY AUTOINCREMENT, l2_addr TEXT NULL , l3_addr TEXT NULL, self INTEGER);");
		dieDB->query("CREATE TABLE IF NOT EXISTS nlink (neighbor_neighbor_id INTEGER, interfaces_intf_id INTEGER, PRIMARY KEY (neighbor_neighbor_id, interfaces_intf_id), CONSTRAINT fk_table1_neighbor1  FOREIGN KEY (neighbor_neighbor_id ) REFERENCES neighbor (neighbor_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_table1_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS cdp (cdp_id INTEGER PRIMARY KEY AUTOINCREMENT, hostname TEXT, nName TEXT, alternatenName TEXT, intf TEXT, nIntfIP TEXT, nIntf TEXT, type INTEGER, platform TEXT, sw_version TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS crypto (crypto_id INTEGER PRIMARY KEY AUTOINCREMENT, peer TEXT, local_net TEXT, remote_net TEXT, vrf TEXT, pfs_grp TEXT, pkt_encaps INTEGER, pkt_decaps INTEGER, pkt_encrypt INTEGER, pkt_decrypt INTEGER, send_err INTEGER, rcv_err INTEGER, path_mtu INTEGER, status TEXT, esp_i TEXT, ah_i TEXT, pcp_i TEXT, esp_o TEXT, ah_o TEXT, pcp_o TEXT, settings TEXT, tag TEXT, seqnr TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS cryptolink (crypto_crypto_id INTEGER, interfaces_intf_id INTEGER, PRIMARY KEY (crypto_crypto_id, interfaces_intf_id), CONSTRAINT fk_cryptolink_crypto1  FOREIGN KEY (crypto_crypto_id) REFERENCES cdp (cdp_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_cryptolink_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS clink (cdp_cdp_id INTEGER, interfaces_intf_id INTEGER, PRIMARY KEY (cdp_cdp_id, interfaces_intf_id), CONSTRAINT fk_clink_cdp1  FOREIGN KEY (cdp_cdp_id) REFERENCES cdp (cdp_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_clink_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS ipSubnet (ipSubnet_id INTEGER PRIMARY KEY AUTOINCREMENT, subnet TEXT NULL, mask TEXT NULL, ulSubnet INTEGER, ulMask INTEGER, ulBroadcast INTEGER);");
		dieDB->query("CREATE TABLE IF NOT EXISTS intfSubnet (ipSubnet_ipSubnet_id INTEGER, interfaces_intf_id INTEGER, PRIMARY KEY (ipSubnet_ipSubnet_id, interfaces_intf_id), CONSTRAINT fk_intfSubnet_ipSubnet1 FOREIGN KEY (ipSubnet_ipSubnet_id) REFERENCES ipSubnet (ipSubnet_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_intfSubnet_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id)  ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS rControl (rc_id INTEGER PRIMARY KEY AUTOINCREMENT, n_id INTEGER, cdp_id INTEGER, dev_id INTEGER, intf_id INTEGER, neighbor_id INTEGER, vlan_id INTEGER, ipSubnet_id INTEGER, stp_status_id INTEGER, stp_id INTEGER, l3r_id INTEGER, ipphone_id INTEGER, hwinf_id INTEGER, license_id INTEGER, flash_id INTEGER, vrf_id INTEGER, crypto_id INTEGER, rpNeighbor_id INTEGER, ospf_id INTEGER, rpn_id INTEGER, hsrp_id INTEGER, auth_id INTEGER, timestamp TEXT, dbVer INTEGER);");
		dieDB->query("CREATE TABLE IF NOT EXISTS l3routes (l3r_id INTEGER PRIMARY KEY AUTOINCREMENT, protocol TEXT NULL, network TEXT NULL, subnet TEXT NULL, nextHop TEXT NULL, interface TEXT NULL, type TEXT NULL, ulSubnet INTEGER, ulMask INTEGER, ulBroadcast INTEGER);");
		dieDB->query("CREATE TABLE IF NOT EXISTS rlink (interfaces_intf_id INTEGER NOT NULL, l3routes_l3r_id INTEGER NOT NULL, vrf_vrf_id INTEGER, PRIMARY KEY (interfaces_intf_id, l3routes_l3r_id, vrf_vrf_id), CONSTRAINT fk_rlink_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_rlink_l3routes1 FOREIGN KEY (l3routes_l3r_id) REFERENCES l3routes (l3r_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_rlink_vrf1 FOREIGN KEY (vrf_vrf_id) REFERENCES vrf (vrf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS ipPhoneDet (ipphone_id INTEGER PRIMARY KEY AUTOINCREMENT, voiceVlan TEXT, cm1 TEXT, cm2 TEXT, dscpSig TEXT, dscpConf TEXT, dscpCall TEXT, defGW TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS ipphonelink (ipPhoneDet_ipphone_id INTEGER, device_dev_id INTEGER NOT NULL ,PRIMARY KEY (ipPhoneDet_ipphone_id, device_dev_id), CONSTRAINT fk_ipphonelink_ipPhoneDet1  FOREIGN KEY (ipPhoneDet_ipphone_id )  REFERENCES ipPhoneDet (ipphone_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_ipphonelink_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS hwInfo (hwinf_id INTEGER PRIMARY KEY AUTOINCREMENT, module TEXT ,type TEXT ,description TEXT ,sn TEXT ,hwRevision TEXT ,memory TEXT ,bootfile TEXT ,sw_version TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS hwlink (device_dev_id INTEGER, hwInfo_hwinf_id INTEGER NOT NULL ,PRIMARY KEY (device_dev_id, hwInfo_hwinf_id), CONSTRAINT fk_hwlink_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_hwlink_hwInfo1  FOREIGN KEY (hwInfo_hwinf_id )  REFERENCES hwInfo (hwinf_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS license (license_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, status TEXT, type TEXT, nextBoot TEXT, cluster INTEGER);");
		dieDB->query("CREATE TABLE IF NOT EXISTS liclink (license_license_id INTEGER, device_dev_id INTEGER NOT NULL ,PRIMARY KEY (license_license_id, device_dev_id), CONSTRAINT fk_table1_license1  FOREIGN KEY (license_license_id )  REFERENCES license (license_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_table1_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS flash (flash_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT ,size TEXT ,free TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS flashlink (flash_flash_id INTEGER, device_dev_id INTEGER NOT NULL ,PRIMARY KEY (flash_flash_id, device_dev_id), CONSTRAINT fk_flashlink_flash1  FOREIGN KEY (flash_flash_id )  REFERENCES flash (flash_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_flashlink_device1  FOREIGN KEY (device_dev_id )  REFERENCES device (dev_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS vrf (vrf_id INTEGER PRIMARY KEY AUTOINCREMENT, vrfName TEXT ,rd TEXT ,Import TEXT ,Export TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS macOUI (oui_id INTEGER PRIMARY KEY AUTOINCREMENT, oui TEXT, vendor TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS vrflink (vrf_vrf_id INTEGER, interfaces_intf_id INTEGER NOT NULL ,PRIMARY KEY (vrf_vrf_id, interfaces_intf_id), CONSTRAINT fk_vrflink_vrf1  FOREIGN KEY (vrf_vrf_id )  REFERENCES vrf (vrf_id )  ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_vrflink_intf1  FOREIGN KEY (interfaces_intf_id )  REFERENCES interfaces (intf_id )  ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS rpNeighbor (rpNeighbor_id INTEGER PRIMARY KEY AUTOINCREMENT, rpNeighborId TEXT, rp TEXT, rpNeighborState TEXT, rpNeighborStateChanges TEXT, rpNeighborAddress TEXT, rpNeighborIntf TEXT, rpNeighborAS TEXT, rpNeighborUp TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS rpNeighborlink (interfaces_intf_id INTEGER, rpNeighbor_rpNeighbor_id INTEGER, PRIMARY KEY (interfaces_intf_id, rpNeighbor_rpNeighbor_id), CONSTRAINT fk_rpNeighborlink_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_rpNeighborlink_rpNeighbor1 FOREIGN KEY (rpNeighbor_rpNeighbor_id) REFERENCES rpNeighbor (rpNeighbor_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS ospf (ospf_id INTEGER PRIMARY KEY AUTOINCREMENT, routerID TEXT, processID TEXT, vrf TEXT, spfLast TEXT, spfExecutions TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS ospflink (device_dev_id INTEGER, ospf_ospf_id INTEGER, PRIMARY KEY (device_dev_id, ospf_ospf_id), CONSTRAINT fk_ospflink_device1 FOREIGN KEY (device_dev_id) REFERENCES device (dev_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_ospflink_ospf1 FOREIGN KEY (ospf_ospf_id) REFERENCES ospf (ospf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS ospfIntflink (ospf_ospf_id INTEGER, interfaces_intf_id INTEGER, PRIMARY KEY (ospf_ospf_id, interfaces_intf_id), CONSTRAINT fk_ospfIntflink_ospf1 FOREIGN KEY (ospf_ospf_id) REFERENCES ospf (ospf_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_ospfIntflink_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS hsrp (hsrp_id INTEGER PRIMARY KEY AUTOINCREMENT, grp TEXT, priority TEXT, state TEXT, active TEXT, standby TEXT, virtualIP TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS hsrplink (interfaces_intf_id INTEGER, hsrp_hsrp_id INTEGER, PRIMARY KEY (interfaces_intf_id, hsrp_hsrp_id), CONSTRAINT fk_hsrplink_interfaces2 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT fk_hsrplink_hsrp1 FOREIGN KEY (hsrp_hsrp_id) REFERENCES hsrp (hsrp_id) ON DELETE CASCADE ON UPDATE CASCADE);");
		dieDB->query("CREATE TABLE IF NOT EXISTS auth (auth_id INTEGER PRIMARY KEY AUTOINCREMENT, userMac TEXT, method TEXT, domain TEXT, status TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS intfAuth (interfaces_intf_id INTEGER, auth_auth_id INTEGER, PRIMARY KEY (interfaces_intf_id, auth_auth_id), CONSTRAINT fk_intfAuth_interfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES interfaces (intf_id) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT fk_intfAuth_auth1 FOREIGN KEY (auth_auth_id) REFERENCES auth (auth_id) ON DELETE NO ACTION ON UPDATE NO ACTION);");
		dieDB->query("CREATE TABLE IF NOT EXISTS devTracking (devTracking_id INTEGER PRIMARY KEY AUTOINCREMENT, ipAddress TEXT, macAddress TEXT, state TEXT, vlanID TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS devTrackingLink (devTracking_devTracking_id INTEGER, interfaces_intf_id INTEGER, PRIMARY KEY (devTracking_devTracking_id,  interfaces_intf_id), CONSTRAINT fk_devTrackingLink_devTracking1 FOREIGN KEY (devTracking_devTracking_id) REFERENCES devTracking (devTracking_id) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT fk_devTrackingLink_inerfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES inerfaces (intf_id) ON DELETE NO ACTION ON UPDATE NO ACTION);");
		dieDB->query("CREATE TABLE IF NOT EXISTS dhcpSnooping (dhcpSnoop_id INTEGER PRIMARY KEY AUTOINCREMENT, ipAddress TEXT, macAddress TEXT, leaseTime TEXT, vlanID TEXT);");
		dieDB->query("CREATE TABLE IF NOT EXISTS dhcpSnoopLink (interfaces_intf_id INTEGER, dhcpSnooping_dhcpSnoop_id INTEGER, PRIMARY KEY (interfaces_intf_id, dhcpSnooping_dhcpSnoop_id), CONSTRAINT fk_dhcpSnoopLink_inerfaces1 FOREIGN KEY (interfaces_intf_id) REFERENCES inerfaces (intf_id) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT fk_dhcpSnoopLink_dhcpSnooping1 FOREIGN KEY (dhcpSnooping_dhcpSnoop_id) REFERENCES dhcpSnooping (dhcpSnoop_id) ON DELETE NO ACTION ON UPDATE NO ACTION);");

		if (vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB_ALL])
		{
			dieDB->query("DELETE FROM device");
			dieDB->query("DELETE FROM crypto");
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
			dieDB->query("DELETE FROM clink");
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
			dieDB->query("DELETE FROM vrf");
			dieDB->query("DELETE FROM vrflink");
			dieDB->query("DELETE FROM cryptolink");
			dieDB->query("DELETE FROM rpNeighborlink");
			dieDB->query("DELETE FROM rpNeighbor");
			dieDB->query("DELETE FROM ospf");
			dieDB->query("DELETE FROM ospflink");
			dieDB->query("DELETE FROM hsrp");
			dieDB->query("DELETE FROM hsrplink");
			dieDB->query("DELETE FROM ospfIntflink");
			dieDB->query("DELETE FROM rpneighborship");
			dieDB->query("DELETE FROM auth");
			dieDB->query("DELETE FROM intfAuth");
			dieDB->query("DELETE FROM dhcpSnooping");
			dieDB->query("DELETE FROM dhcpSnoopLink");
			dieDB->query("DELETE FROM devTracking");
			dieDB->query("DELETE FROM devTrackingLink");
		}
		else if (vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB])
		{
			dieDB->query("DELETE FROM device WHERE dev_id <= (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl))");
			dieDB->query("DELETE FROM crypto WHERE crypto_id <= (SELECT MAX(crypto_id) FROM rControl WHERE dev_id < (SELECT MAX(crypto_id) FROM rControl))");
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
			dieDB->query("DELETE FROM clink WHERE cdp_cdp_id <= (SELECT MAX(cdp_id) FROM rControl WHERE cdp_id < (SELECT MAX(cdp_id) FROM rControl))");
			dieDB->query("DELETE FROM ipSubnet WHERE ipSubnet_id <= (SELECT MAX(ipSubnet_id) FROM rControl WHERE ipSubnet_id < (SELECT MAX(ipSubnet_id) FROM rControl))");
			dieDB->query("DELETE FROM intfSubnet WHERE ipSubnet_ipSubnet_id <= (SELECT MAX(ipSubnet_id) FROM rControl WHERE ipSubnet_id < (SELECT MAX(ipSubnet_id) FROM rControl))");
			dieDB->query("DELETE FROM neighborship WHERE n_id <= (SELECT MAX(n_id) FROM rControl WHERE n_id < (SELECT MAX(n_id) FROM rControl))");
			dieDB->query("DELETE FROM rpneighborship WHERE rpn_id <= (SELECT MAX(rpn_id) FROM rControl WHERE rpn_id < (SELECT MAX(rpn_id) FROM rControl))");
			dieDB->query("DELETE FROM rControl WHERE rc_id < (SELECT MAX(rc_id) FROM rControl) AND rc_id > 1");
			dieDB->query("DELETE FROM l3routes WHERE l3r_id < (SELECT MAX(l3r_id) FROM rControl WHERE l3r_id < (SELECT MAX(l3r_id) FROM rControl))");
			dieDB->query("DELETE FROM rlink WHERE l3routes_l3r_id < (SELECT MAX(l3r_id) FROM rControl WHERE l3r_id < (SELECT MAX(l3r_id) FROM rControl))");
			dieDB->query("DELETE FROM ipPhoneDet WHERE ipphone_id <= (SELECT MAX(ipphone_id) FROM rControl WHERE ipphone_id < (SELECT MAX(ipphone_id) FROM rControl))");
			dieDB->query("DELETE FROM hwInfo WHERE hwinf_id <= (SELECT MAX(hwinf_id) FROM rControl WHERE hwinf_id < (SELECT MAX(hwinf_id) FROM rControl))");
			dieDB->query("DELETE FROM license WHERE license_id <= (SELECT MAX(license_id) FROM rControl WHERE license_id < (SELECT MAX(license_id) FROM rControl))");
			dieDB->query("DELETE FROM flash WHERE flash_id <= (SELECT MAX(flash_id) FROM rControl WHERE flash_id < (SELECT MAX(flash_id) FROM rControl))");
			dieDB->query("DELETE FROM vrf WHERE vrf_id <= (SELECT MAX(vrf_id) FROM rControl WHERE vrf_id < (SELECT MAX(vrf_id) FROM rControl))");
			dieDB->query("DELETE FROM ipphonelink WHERE ipPhoneDet_ipphone_id <= (SELECT MAX(ipphone_id) FROM rControl WHERE ipphone_id < (SELECT MAX(ipphone_id) FROM rControl))");
			dieDB->query("DELETE FROM hwlink WHERE hwInfo_hwinf_id <= (SELECT MAX(hwinf_id) FROM rControl WHERE hwinf_id < (SELECT MAX(hwinf_id) FROM rControl))");
			dieDB->query("DELETE FROM liclink WHERE license_license_id <= (SELECT MAX(license_id) FROM rControl WHERE license_id < (SELECT MAX(license_id) FROM rControl))");
			dieDB->query("DELETE FROM flashlink WHERE flash_flash_id <= (SELECT MAX(flash_id) FROM rControl WHERE flash_id < (SELECT MAX(flash_id) FROM rControl))");
			dieDB->query("DELETE FROM cryptolink WHERE crypto_crypto_id <= (SELECT MAX(crypto_id) FROM rControl WHERE crypto_id < (SELECT MAX(crypto_id) FROM rControl))");
			dieDB->query("DELETE FROM rpNeighbor WHERE rpNeighbor_id <= (SELECT MAX(rpNeighbor_id) FROM rControl WHERE rpNeighbor_id < (SELECT MAX(rpNeighbor_id) FROM rControl))");
			dieDB->query("DELETE FROM rpNeighborlink WHERE rpNeighbor_rpNeighbor_id <= (SELECT MAX(rpNeighbor_id) FROM rControl WHERE rpNeighbor_id < (SELECT MAX(rpNeighbor_id) FROM rControl))");
			dieDB->query("DELETE FROM ospf WHERE ospf_id <= (SELECT MAX(ospf_id) FROM rControl WHERE ospf_id < (SELECT MAX(ospf_id) FROM rControl))");
			dieDB->query("DELETE FROM ospflink WHERE ospf_ospf_id <= (SELECT MAX(ospf_id) FROM rControl WHERE ospf_id < (SELECT MAX(ospf_id) FROM rControl))");
			dieDB->query("DELETE FROM hsrp WHERE hsrp_id <= (SELECT MAX(hsrp_id) FROM rControl WHERE hsrp_id < (SELECT MAX(hsrp_id) FROM rControl))");
			dieDB->query("DELETE FROM hsrplink WHERE hsrp_hsrp_id <= (SELECT MAX(hsrp_id) FROM rControl WHERE hsrp_id < (SELECT MAX(hsrp_id) FROM rControl))");
			dieDB->query("DELETE FROM ospfIntflink WHERE ospf_ospf_id <= (SELECT MAX(ospf_id) FROM rControl WHERE ospf_id < (SELECT MAX(ospf_id) FROM rControl))");
			dieDB->query("DELETE FROM auth WHERE auth_id <= (SELECT MAX(auth_id) FROM rControl WHERE auth_id < (SELECT MAX(auth_id) FROM rControl))");
			dieDB->query("DELETE FROM intfAuth WHERE auth_auth_id <= (SELECT MAX(auth_id) FROM rControl WHERE auth_id < (SELECT MAX(auth_id) FROM rControl))");
			dieDB->query("DELETE FROM devTracking WHERE devTracking_id <= (SELECT MAX(devTracking_id) FROM rControl WHERE devTracking_id < (SELECT MAX(devTracking_id) FROM rControl))");
			dieDB->query("DELETE FROM devTrackingLink WHERE devTracking_devTracking_id <= (SELECT MAX(devTracking_id) FROM rControl WHERE devTracking_id < (SELECT MAX(devTracking_id) FROM rControl))");
			dieDB->query("DELETE FROM dhcpSnooping WHERE dhcpSnoop_id <= (SELECT MAX(dhcpSnoop_id) FROM rControl WHERE dhcpSnoop_id < (SELECT MAX(dhcpSnoop_id) FROM rControl))");
			dieDB->query("DELETE FROM dhcpSnoopLink WHERE dhcpSnooping_dhcpSnoop_id <= (SELECT MAX(dhcpSnoop_id) FROM rControl WHERE dhcpSnoop_id < (SELECT MAX(dhcpSnoop_id) FROM rControl))");
		}

		// Wenn Intf-ID gleich 0, dann auf 10.000 festlegen, damit es keine Überschneidungen mit den Device-IDs gibt
		sString = "SELECT seq FROM SQLITE_SEQUENCE WHERE name LIKE 'interfaces'";
		std::vector<std::vector<std::string>> intfid = dieDB->query(sString.c_str());
		if (!intfid.empty())
		{
			if (intfid[0][0] == "0")
			{
				sString = "INSERT INTO interfaces (intf_id) VALUES (10000)";
				dieDB->query(sString.c_str());
				dieDB->query("DELETE FROM interfaces");
			}
		}
		else
		{
			sString = "INSERT INTO interfaces (intf_id) VALUES (10000)";
			dieDB->query(sString.c_str());
			dieDB->query("DELETE FROM interfaces");
		}

		if (vari[CWkmPropGrid::WKM_PROPID_INT_PARSE])
		{
			schreibeLog(WkLog::WkLog_ZEIT, "6616: Start Parser\r\n", "6616", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			parserdb = new WkmParserDB(logAusgabe, dieDB);
			parserdb->debugAusgabe = wkmdbg;
			parserdb->debugAusgabe1 = wkmdbg;
			derParser = new WkmParser(vars[CWkmPropGrid::WKM_PROPID_STR_SDIR], logAusgabe, dieDB, parserdb);
			derParser->debugAusgabe = wkmdbg;
			derParser->debugAusgabe1 = wkmdbg;
			derParser->startParser(vars[CWkmPropGrid::WKM_PROPID_STR_PATTERN]);
			fehlerString = derParser->fehlerRet();

			delete dieDB;
			dieDB = NULL;
			dieDB = new WkmDB(dbName, vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDB]);
		}

		dieDB->query("CREATE INDEX IF NOT EXISTS 'neighborL2addrIndex' ON 'neighbor' ('l2_addr' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'neighborL3addrIndex' ON 'neighbor' ('l3_addr' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'neighborSelfIndex' ON 'neighbor' ('self' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'interfacesPhlIndex' ON 'interfaces' ('phl' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'interfacesIpaddressIndex' ON 'interfaces' ('ipaddress' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'interfacesL2l3Index' ON 'interfaces' ('l2l3' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'interfacesMacaddressIndex' ON 'interfaces' ('macaddress' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'interfacesIntfnameIndex' ON 'interfaces' ('intfName' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'interfacesStatusIndex' ON 'interfaces' ('status' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'interfacesIntftypeIndex' ON 'interfaces' ('intfType' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'deviceHwtypeIndex' ON 'device' ('hwType' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'deviceTypeIndex' ON 'device' ('type' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'deviceHostnameIndex' ON 'device' ('Hostname' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'stpstatusStatsonlyIndex' ON 'stp_status' ('statsOnly' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'stpstatusStpIntfStatusIndex' ON 'stp_status' ('stpIntfStatus' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'stpinstanceRootPortIndex' ON 'stpInstance' ('rootPort' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'vlanVlanIndex' ON 'vlan' ('vlan' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'neighborshipPriorityIndex' ON 'neighborship' ('Priority' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'neighborshipFlagIndex' ON 'neighborship' ('flag' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'l3routesNetworkIndex' ON 'l3routes' ('network' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'l3routesNextHopIndex' ON 'l3routes' ('nextHop' ASC)");
		dieDB->query("CREATE INDEX IF NOT EXISTS 'ipSubnetUlSubnetIndex' ON 'ipSubnet' ('ulSubnet' ASC)");
		
		if (vari[CWkmPropGrid::WKM_PROPID_INT_IMPORT])
		{
			schreibeLog(WkLog::WkLog_ZEIT, "6623: Start Import\r\n", "6616", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
			WkmImport *derImporter = new WkmImport(vars[CWkmPropGrid::WKM_PROPID_STR_HOSTFILE], dieDB, logAusgabe);
			derImporter->startParser();
			delete derImporter;
		}


		// Ausgabe nur machen, wenn DB nicht leer
		sString = "SELECT dev_id FROM device";
		std::vector<std::vector<std::string>> dbCheck = dieDB->query(sString.c_str());

		if (!dbCheck.empty())
		{
			if (vari[CWkmPropGrid::WKM_PROPID_INT_COMBINE])
			{
				schreibeLog(WkLog::WkLog_ZEIT, "6617: Computing adjacencies\r\n", "6617", 
					WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

				// Wenn nur analysiert ohne neu geparst wird, dann muss n_id beim letzten Eintrag zurückgesetzt werden
				// und die letzte Nachbraschaftsberechnung gelöscht werden
				if (!vari[CWkmPropGrid::WKM_PROPID_INT_PARSE])
				{
					sString = "DELETE FROM neighborship WHERE n_id > (SELECT MAX(n_id) FROM rControl WHERE n_id < (SELECT MAX(n_id) FROM rControl))";
					dieDB->query(sString.c_str());
					sString = "UPDATE rControl SET n_id=NULL WHERE n_id=(SELECT MAX(n_id) FROM rControl)";
					dieDB->query(sString.c_str());
				}

				derCombiner = new WkmCombiner2(dieDB, logAusgabe, parserdb, vari[CWkmPropGrid::WKM_PROPID_INT_IMP_ES], vari[CWkmPropGrid::WKM_PROPID_INT_DNS_RESOLUTION]);

				derCombiner->debugAusgabe = wkmdbg;
				derCombiner->debugAusgabe1 = wkmdbg;
				derCombiner->doIt();
			}

			// dbVersion setzen
			sString = "UPDATE rControl SET dbVer=" + boost::lexical_cast<std::string>(wkmDbVersion) + " WHERE rc_id=(SELECT MAX(rc_id) FROM rControl)";
			dieDB->query(sString.c_str());

			schreibeLog(WkLog::WkLog_ZEIT, "6618: Generating output file\r\n", "6618", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
			WkmAusgabe *dieAusgabe = new WkmAusgabe(dieDB, logAusgabe, vars[CWkmPropGrid::WKM_PROPID_STR_OFILE]);
			dieAusgabe->sets(vari[CWkmPropGrid::WKM_PROPID_INT_CDP], vari[CWkmPropGrid::WKM_PROPID_INT_CDPHOSTS], vari[CWkmPropGrid::WKM_PROPID_INT_UNKNOWNSTP], vari[CWkmPropGrid::WKM_PROPID_INT_ROUTENEIGHBORS],
				vari[CWkmPropGrid::WKM_PROPID_INT_L2], vari[CWkmPropGrid::WKM_PROPID_INT_L3], vari[CWkmPropGrid::WKM_PROPID_INT_L3_ROUTING], vari[CWkmPropGrid::WKM_PROPID_INT_ORGANIC],
				vari[CWkmPropGrid::WKM_PROPID_INT_HIERARCHIC], vari[CWkmPropGrid::WKM_PROPID_INT_ORTHO], vars[CWkmPropGrid::WKM_PROPID_STR_UNITEDGELENGTH], vars[CWkmPropGrid::WKM_PROPID_STR_LAYERDISTANCE], 
				vars[CWkmPropGrid::WKM_PROPID_STR_NODEDISTANCE], vars[CWkmPropGrid::WKM_PROPID_STR_WEIHTBALANCING],	vari[CWkmPropGrid::WKM_PROPID_INT_ENDSYSTEMS], vari[CWkmPropGrid::WKM_PROPID_INT_INTFSTYLE]);
			dieAusgabe->doIt();
			delete dieAusgabe;
		}
		else
		{
			std::string dbgA = "6306: Unable to calculate adjacencies because Database is empty. Please check if \"Parse New\" and \"Analyze New\" is checked under Options.";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6306", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		}

		if (fehlerString != "")
		{
			std::string dbgA = "\n6305: Parser Errors:";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerString, "6305", 
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		}

		//if (res)
		//{
		//	schreibeLog(WkLog::WkLog_ZEIT, "6401: Mapper returned with errors\r\n", "6401", 
		//		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		//}
		//else
		//{
		//	schreibeLog(WkLog::WkLog_ZEIT, "6603: Mapper finished\r\n", "6603", 
		//		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		//}
	}
	delete dieDB;
	dieDB = NULL;

	wxCommandEvent event(wkEVT_MAPPER_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);
}


// bool doLoad:
// zum Laden eines Profils. Die Einstellungen werden überprüft, bevor sie geladen werden
// return Werte: true: erfolgreich geladen
//               false: Fehler beim Laden der Werte
bool Wkm::doLoad(std::string profilname)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "6604: Mapper: Loading Profile\r\n", "6604", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (xmlInit(profilname))
	{
		einstellungenInit();
		schreibeLog(WkLog::WkLog_ZEIT, "6605: Mapper: Profile \"" + profilname + "\" successfully loaded\r\n", "6605", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		return true;
	}
	schreibeLog(WkLog::WkLog_ZEIT, "6310: Mapper: Unable to load Profile \"" + profilname + "\"\r\n", "6310", 
		WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	return false;
}


// bool doSave:
// zum Sichern der Einstellungen in ein Profil
// return Werte: false: erfolgreich gesichert
//               true: Fehler beim Sichern
bool Wkm::doSave(std::string profilname)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "6606: Mapper: Saving Profile\r\n", "6606", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	if (guiStatus)
	{
		fehler = doTest(profilname);
	}
	else
	{
		fehler = doTestGui(profilname);
	}
	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6606: Mapper: Profile \"" + profilname + "\" successfully saved\r\n", "6606", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6302: Mapper: Unable to save Profile \"" + profilname + "\"\r\n", "6302", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// bool doRemove:
// zum Löschen eines Profils
// return Werte: false: erfolgreich gelöscht
//               true: Fehler beim Löschen
bool Wkm::doRemove(std::string profilName)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "6622: Mapper: Removing Profile\r\n", "6622", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (profilName == "" || profilName == "DEFAULT")
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6304: Mapper: Unable to remove Profile \"" + profilName + "\" - Default Profile cannot be deleted\r\n", "6304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
		return fehler;
	}

	fehler = xmlInit(profilName, 2);

	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6621: Mapper: Profile \"" + profilName + "\" successfully removed\r\n", "6621", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6304: Mapper: Unable to remove Profile \"" + profilName + "\"\r\n", "6304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Wkm::sets(TiXmlDocument *document, bool guiStats, WkLogFile *lf)
{
	doc = document;

	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;

	logfile = lf;
}


// bool einstellungenInit()
// Zum Laden der Werte, nachdem die richtige Position des Profils in wktools.xml ausgemacht wurde.
bool Wkm::einstellungenInit()
{
	TiXmlElement *stringSets = pElemProf->FirstChildElement("StringSettings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"sD", "oD", "LogFile", "Pattern", "ULL", "LDist", "NDist", "WeightB", "Hostfile"};
	std::string attInt[] = {"ParseNew", "AnalyzeNew", "CDP", "CDP_Hosts", "UnknownStp", "L2", "L3", "ORG", "HIER", "CleanupDB", "ClearDB", "Import", "revDns", "endsystems", "imp_es", "in-mem-db", "ORTHO", "istyle", "L3Routing", "rpneighbors", "inMemDB" };

	if (stringSets)
	{
		for (int i = 0; i < a1; i++)
		{
			if (stringSets->Attribute(attString[i]))
			{
				vars[i] = *(stringSets->Attribute(attString[i]));
			}
			else
			{
				//vars[i] = "";
			}
			if (!guiStatus)
			{
				// GUI Mode
				wkmProps->einstellungenInit(i, vars[i]);
			}
		}
	}

	if (intSets)
	{
		for (int i = 0; i < a2; i++)
		{
			int ret = intSets->QueryIntAttribute(attInt[i], &vari[i]);
			if (ret != TIXML_SUCCESS)
			{
				//vari[i] = 0;
			}
			if (!guiStatus)
			{
				// GUI Mode
				wkmProps->einstellungenInit(i, vari[i]);
			}
		}
	}

	if (!guiStatus)
	{
		// GUI Mode
		wkmProps->enableDisable();
	}

	if (logfile != NULL)
	{
		logfile->logFileSets(vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE]);
	}

	return true;
}


void Wkm::guiSets(CWkmPropGrid *wkmProp, WkLog *ausgabe)
{
	wkmProps = wkmProp;
	logAusgabe = ausgabe;
}


wxArrayString Wkm::ladeProfile()
{
	wxArrayString profile;

	// Grund Elemente für Dip
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWkm;		// das Element, das auf Dip zeigt

	// Dip im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		profile.Empty();
	}
	else
	{
		pElemWkm = pElem->FirstChildElement("wkm");
		if (!pElemWkm)
		{
			profile.Empty();
		}
		else
		{
			pElemProf = pElemWkm->FirstChildElement("Profile");

			for (profilIndex = 0; pElemProf; profilIndex++)
			{
				std::string profname = pElemProf->Attribute("name");
				profile.Add(profname.c_str());

				pElemProf = pElemProf->NextSiblingElement();
			}
		}
	}

	return profile;
}


void Wkm::cancelIt()
{
	if (derParser != NULL)
	{
		derParser->stop = true;
	}
	if (derCombiner != NULL)
	{
		derCombiner->stop = true;
	}

	// DB CleanUP durchführen
	dieDB->query("DELETE FROM device");
	dieDB->query("DELETE FROM crypto");
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
	dieDB->query("DELETE FROM clink");
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
	dieDB->query("DELETE FROM vrf");
	dieDB->query("DELETE FROM vrflink");
	dieDB->query("DELETE FROM cryptolink");
	dieDB->query("DELETE FROM rpNeighborlink");
	dieDB->query("DELETE FROM rpNeighbor");
	dieDB->query("DELETE FROM ospf");
	dieDB->query("DELETE FROM ospflink");
	dieDB->query("DELETE FROM dhcpSnoopLink");
	dieDB->query("DELETE FROM dhcpSnooping");
	dieDB->query("DELETE FROM devTrackingLink");
	dieDB->query("DELETE FROM devTracking");
	dieDB->query("DELETE FROM auth");
	dieDB->query("DELETE FROM hsrplink");
	dieDB->query("DELETE FROM hsrp");
	dieDB->query("DELETE FROM intfAuth");

	wxCommandEvent event(wkEVT_MAPPER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


void Wkm::kilIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "6607: Mapper killed!\r\n", "6607", 
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);

	ende = true;
	if (mitThread)
	{
		thrd.interrupt();
	}

	wxCommandEvent event(wkEVT_MAPPER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


// xmlInit
//********
// Speichern: 0 - laden; 1 - speichern; 2 - löschen
bool Wkm::xmlInit(std::string profilName, int speichern)
{
	if (profilName == "")
	{
		profilName = "DEFAULT";
	}

	// Grund Elemente für Dip
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWkm;		// das Element, das auf Wkm zeigt

	// Dip im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		return false;
	}

	pElemWkm = pElem->FirstChildElement("wkm");
	if (!pElemWkm)
	{
		pElemWkm = new TiXmlElement("wkm");
		pElem->LinkEndChild(pElemWkm);
	}
	pElemProf = pElemWkm->FirstChildElement("Profile");
	if (!pElemProf)
	{
		pElemProf = new TiXmlElement("Profile");
		pElemWkm->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", "DEFAULT");

		TiXmlElement *stringSets = new TiXmlElement("StringSettings");
		TiXmlElement *intSets = new TiXmlElement("IntSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);

		std::string attString[] = {"sD", "oD", "LogFile", "Pattern", "ULL", "LDist", "NDist", "WeightB", "Hostfile"};
		std::string attInt[] ={ "ParseNew", "AnalyzeNew", "CDP", "CDP_Hosts", "UnknownStp", "L2", "L3", "ORG", "HIER", "CleanupDB", "ClearDB", "Import", "revDns", "endsystems", "imp_es", "in-mem-db", "ORTHO", "istyle", "L3Routing", "rpneighbors", "inMemDB" };

		for (int i = 0; i < a1; i++)
		{
			stringSets->SetAttribute(attString[i].c_str(), "");
		}
		for (int i = 0; i < a2; i++)
		{
			intSets->SetAttribute(attInt[i].c_str(), 0);
		}
	}

	for (profilIndex = 0; pElemProf; profilIndex++)
	{
		std::string profname = pElemProf->Attribute("name");
		if (profname == profilName)
		{
			if (speichern == 2)
			{
				bool rmret = pElemWkm->RemoveChild(pElemProf);
				return !rmret;
			}
			return true;
		}

		pElemProf = pElemProf->NextSiblingElement();
	}

	if (speichern)
	{
		pElemProf = new TiXmlElement("Profile");		
		pElemWkm->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", profilName);

		TiXmlElement *stringSets = new TiXmlElement("StringSettings");
		TiXmlElement *intSets = new TiXmlElement("IntSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);

		std::string attString[] = {"sD", "oD", "LogFile", "Pattern", "ULL", "LDist", "NDist", "WeightB", "Hostfile"};
		std::string attInt[] ={ "ParseNew", "AnalyzeNew", "CDP", "CDP_Hosts", "UnknownStp", "L2", "L3", "ORG", "HIER", "CleanupDB", "ClearDB", "Import", "revDns", "endsystems", "imp_es", "in-mem-db", "ORTHO", "istyle", "L3Routing", "rpneighbors", "inMemDB" };

		for (int i = 0; i < a1; i++)
		{
			stringSets->SetAttribute(attString[i].c_str(), "");
		}
		for (int i = 0; i < a2; i++)
		{
			intSets->SetAttribute(attInt[i].c_str(), 0);
		}

		return true;
	}

	// TODO: FEHLER
	profilIndex = 65535;
	return false;
}


bool Wkm::doTest(std::string profilname)
{
	// Test, ob alle Eingaben gültig sind
	bool fehler = false;

	// Test 1: Search Directory:
	std::string fehlerSDir = "6303: Wrong or missing entry for SEARCH DIRECTORY!";

	wxFileName fn;
	if (!fn.DirExists(vars[CWkmPropGrid::WKM_PROPID_STR_SDIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSDir, "6303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 2: Ausgabe Directory:
	std::string fehlerODir = "6303: Wrong or missing entry for OUTPUT DIRECTORY!";

	wxFileName fn2;
	if (!fn2.DirExists(vars[CWkmPropGrid::WKM_PROPID_STR_OFILE]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODir, "6303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 3: Logfile:	
	std::string fehlerLDat = "6303: Wrong or missing entry for LOG FILE!";
	if (vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerLDat, "6303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	return fehler;
}


bool Wkm::doTestGui(std::string profilname)
{
	if  (!(xmlInit(profilname, true)))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n6101: Init Error -> could not read settings\r\n", "6101", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	TiXmlElement *stringSets = pElemProf->FirstChildElement("StringSettings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"sD", "oD", "LogFile", "Pattern", "ULL", "LDist", "NDist", "WeightB", "Hostfile"};
	std::string attInt[] ={ "ParseNew", "AnalyzeNew", "CDP", "CDP_Hosts", "UnknownStp", "L2", "L3", "ORG", "HIER", "CleanupDB", "ClearDB", "Import", "revDns", "endsystems", "imp_es", "in-mem-db", "ORTHO", "istyle", "L3Routing", "rpneighbors", "inMemDB" };

	bool fehler = false;

	// Test 1: Search Directory:
	std::string fehlerSDir = "6303: Wrong or missing entry for SEARCH DIRECTORY!";


	std::string svz = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_SDIR);
	if (svz != "")
	{
#ifdef _WINDOWS_
		if (svz[svz.length()-1] != '\\') 
		{
			svz.append(std::string("\\")); // append '\'
		}
#else
		if (svz[svz.length()-1] != '/') 
		{
			svz.append(std::string("/")); // append '/'
		}
#endif
	}
	vars[CWkmPropGrid::WKM_PROPID_STR_SDIR] = svz;

	std::ifstream ipaFile(vars[CWkmPropGrid::WKM_PROPID_STR_SDIR].c_str());
	wxFileName fn;

	if (!fn.DirExists(vars[CWkmPropGrid::WKM_PROPID_STR_SDIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSDir, "6303",
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_SDIR], vars[CWkmPropGrid::WKM_PROPID_STR_SDIR]);
	}

	// Test 2: Ausgabe Directory: -> hinfällig, da jetzt File
	//std::string fehlerODir = "6303: Wrong or missing entry for OUTPUT DIRECTORY!";

	std::string avz = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_OFILE);
	vars[CWkmPropGrid::WKM_PROPID_STR_OFILE] = avz;
	stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_OFILE], vars[CWkmPropGrid::WKM_PROPID_STR_OFILE]);

	// Test 3.0: Ausgabeverzeichnis
#ifdef _WINDOWS_
	ausgabeVz = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].substr(0, vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].rfind("\\")+1);
#else
	ausgabeVz = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].substr(0, vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].rfind("/")+1);
#endif
	// Test 3.1: Logfile:	
	vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE] = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_LOGFILE);
	if (vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE] == "")
	{
		vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE] = ausgabeVz + "log.txt";
		stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_LOGFILE], vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE]);
		wkmProps->einstellungenInit(CWkmPropGrid::WKM_PROPID_STR_LOGFILE, vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE]);
	}
	else
	{
		stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_LOGFILE], vars[CWkmPropGrid::WKM_PROPID_STR_LOGFILE]);
	}

	vars[CWkmPropGrid::WKM_PROPID_STR_PATTERN] = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_PATTERN);
	stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_PATTERN], vars[CWkmPropGrid::WKM_PROPID_STR_PATTERN]);
	vars[CWkmPropGrid::WKM_PROPID_STR_HOSTFILE] = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_HOSTFILE);
	stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_HOSTFILE], vars[CWkmPropGrid::WKM_PROPID_STR_HOSTFILE]);
	vari[CWkmPropGrid::WKM_PROPID_INT_IMPORT] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_IMPORT);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_IMPORT], vari[CWkmPropGrid::WKM_PROPID_INT_IMPORT]);
	vari[CWkmPropGrid::WKM_PROPID_INT_IMP_ES] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_IMP_ES);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_IMP_ES], vari[CWkmPropGrid::WKM_PROPID_INT_IMP_ES]);

	// Options
	vari[CWkmPropGrid::WKM_PROPID_INT_PARSE] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_PARSE);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_PARSE], vari[CWkmPropGrid::WKM_PROPID_INT_PARSE]);
	vari[CWkmPropGrid::WKM_PROPID_INT_COMBINE] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_COMBINE);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_COMBINE], vari[CWkmPropGrid::WKM_PROPID_INT_COMBINE]);
	vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_CLEARDB);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_CLEARDB], vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB]);
	vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB_ALL] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_CLEARDB_ALL);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_CLEARDB_ALL], vari[CWkmPropGrid::WKM_PROPID_INT_CLEARDB_ALL]);

	vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDB] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_INMEMDB);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_INMEMDB], vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDB]);
	vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDBNEW] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_INMEMDB);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_INMEMDBNEW], vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDBNEW]);

	vari[CWkmPropGrid::WKM_PROPID_INT_CDP] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_CDP);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_CDP], vari[CWkmPropGrid::WKM_PROPID_INT_CDP]);
	vari[CWkmPropGrid::WKM_PROPID_INT_CDPHOSTS] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_CDPHOSTS);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_CDPHOSTS], vari[CWkmPropGrid::WKM_PROPID_INT_CDPHOSTS]);
	vari[CWkmPropGrid::WKM_PROPID_INT_UNKNOWNSTP] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_UNKNOWNSTP);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_UNKNOWNSTP], vari[CWkmPropGrid::WKM_PROPID_INT_UNKNOWNSTP]);
	vari[CWkmPropGrid::WKM_PROPID_INT_ROUTENEIGHBORS] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_ROUTENEIGHBORS);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_ROUTENEIGHBORS], vari[CWkmPropGrid::WKM_PROPID_INT_ROUTENEIGHBORS]);
	vari[CWkmPropGrid::WKM_PROPID_INT_ENDSYSTEMS] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_ENDSYSTEMS);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_ENDSYSTEMS], vari[CWkmPropGrid::WKM_PROPID_INT_ENDSYSTEMS]);
	
	vari[CWkmPropGrid::WKM_PROPID_INT_L2] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_L2);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_L2], vari[CWkmPropGrid::WKM_PROPID_INT_L2]);
	vari[CWkmPropGrid::WKM_PROPID_INT_L3] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_L3);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_L3], vari[CWkmPropGrid::WKM_PROPID_INT_L3]);
	vari[CWkmPropGrid::WKM_PROPID_INT_L3_ROUTING] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_L3_ROUTING);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_L3_ROUTING], vari[CWkmPropGrid::WKM_PROPID_INT_L3_ROUTING]);
	vari[CWkmPropGrid::WKM_PROPID_INT_ORGANIC] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_ORGANIC);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_ORGANIC], vari[CWkmPropGrid::WKM_PROPID_INT_ORGANIC]);
	vari[CWkmPropGrid::WKM_PROPID_INT_HIERARCHIC] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_HIERARCHIC);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_HIERARCHIC], vari[CWkmPropGrid::WKM_PROPID_INT_HIERARCHIC]);
	vari[CWkmPropGrid::WKM_PROPID_INT_ORTHO] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_ORTHO);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_ORTHO], vari[CWkmPropGrid::WKM_PROPID_INT_ORTHO]);

	vari[CWkmPropGrid::WKM_PROPID_INT_DNS_RESOLUTION] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_DNS_RESOLUTION);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_DNS_RESOLUTION], vari[CWkmPropGrid::WKM_PROPID_INT_DNS_RESOLUTION]);

	vari[CWkmPropGrid::WKM_PROPID_INT_INTFSTYLE] = wkmProps->leseIntSetting(CWkmPropGrid::WKM_PROPID_INT_INTFSTYLE);
	intSets->SetAttribute(attInt[CWkmPropGrid::WKM_PROPID_INT_INTFSTYLE], vari[CWkmPropGrid::WKM_PROPID_INT_INTFSTYLE]);

	vars[CWkmPropGrid::WKM_PROPID_STR_UNITEDGELENGTH] = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_UNITEDGELENGTH);
	stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_UNITEDGELENGTH], vars[CWkmPropGrid::WKM_PROPID_STR_UNITEDGELENGTH]);
	vars[CWkmPropGrid::WKM_PROPID_STR_LAYERDISTANCE] = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_LAYERDISTANCE);
	stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_LAYERDISTANCE], vars[CWkmPropGrid::WKM_PROPID_STR_LAYERDISTANCE]);
	vars[CWkmPropGrid::WKM_PROPID_STR_NODEDISTANCE] = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_NODEDISTANCE);
	stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_NODEDISTANCE], vars[CWkmPropGrid::WKM_PROPID_STR_NODEDISTANCE]);
	vars[CWkmPropGrid::WKM_PROPID_STR_WEIHTBALANCING] = wkmProps->leseStringSetting(CWkmPropGrid::WKM_PROPID_STR_WEIHTBALANCING);
	stringSets->SetAttribute(attString[CWkmPropGrid::WKM_PROPID_STR_WEIHTBALANCING], vars[CWkmPropGrid::WKM_PROPID_STR_WEIHTBALANCING]);



	return fehler;
}


void Wkm::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
#ifdef _WINDOWS_
	Sleep(20);
#else
	usleep(30000);
#endif				

	WkLog::evtData evtDat;
	evtDat.farbe = farbe;
	evtDat.format = format;
	evtDat.groesse = groesse;
	evtDat.logEintrag = logEintrag;
	evtDat.logEintrag2 = log2;
	evtDat.type = type;
	evtDat.report = 0;

	LogEvent evt(EVT_LOG_MELDUNG);
	evt.SetData(evtDat);

	if (logAusgabe != NULL)
	{
		wxPostEvent(logAusgabe, evt);
	}

	if (logfile != NULL)
	{
		wxPostEvent(logfile, evt);
	}
}


void Wkm::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


// OnWkmFertig
// Wird durch Nachricht aufgerufen, wenn der Hauptthread (damit das GUI nicht blockiert) fertig ist
void Wkm::OnWkmFertig(wxCommandEvent &event)
{
	schreibeLog(WkLog::WkLog_ZEIT, "6608: Mapper END\r\n", "6608", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	if (wkMain != NULL)
	{
		wxCommandEvent event(wkEVT_WKM_FERTIG);
		event.SetInt(1);
		wxPostEvent(wkMain, event);
	}
	else
	{
		// Programm beenden, wenn kein GUI vorhanden
		wxGetApp().ExitMainLoop();
	}
}


int Wkm::doMacSearch()
{
	size_t pos = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].find_last_of("\\/");
	std::string oDir = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].substr(0, pos+1);

	std::string dbName = oDir + "wkm.db";
	WkmDB *dieDB = new WkmDB(dbName, vari[CWkmPropGrid::WKM_PROPID_INT_INMEMDB]);

	std::string sString = "SELECT dev_id FROM device";
	std::vector<std::vector<std::string>> dbCheck = dieDB->query(sString.c_str());
	delete dieDB;

	if (!dbCheck.empty())
	{
		MacSearchDialog *sucheHost = new MacSearchDialog(wxT("Search Host"));
		sucheHost->Show(true);
		sucheHost->setEinstellungen(dbName, logAusgabe);
	}
	else
	{
		std::string dbgA = "6306: Unable to do the Search, because Database is empty. Please collect the needed data before using the Search feature.";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6305", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}

	return 1;
}


int Wkm::doMacUpdate()
{
	schreibeLog(WkLog::WkLog_ABSATZ, "6624: Updating MAC Vendor Codes - Please Wait.\r\n", "6624", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	size_t pos = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].find_last_of("\\/");
	std::string oDir = vars[CWkmPropGrid::WKM_PROPID_STR_OFILE].substr(0, pos+1);

	std::string dbName = oDir + "wkm.db";
	MacUpdate *mu = new MacUpdate(dbName);
	int ret = mu->doUpdate();

	if (ret != 0)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6307: Update not possible!\r\n", "6307", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ABSATZ, "6625: MAC OUI Update complete\r\n", "6625", 
			WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
	}
	delete mu;
	return 1;
}


#endif