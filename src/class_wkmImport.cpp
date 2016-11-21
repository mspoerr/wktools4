#include "class_wkmImport.h"
#ifdef WKTOOLS_MAPPER


#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>


WkmImport::WkmImport(std::string hostfile, WkmDB *db, WkLog *logA)
{
	dateiname = hostfile;
	dieDB = db;
	logAusgabe = logA;

	stop = false;
	rControlEmpty = true;
}



WkmImport::~WkmImport()
{

}


int WkmImport::startParser()
{
	// Check, ob rControl leer ist
	std::string sString = "SELECT MAX( n_id) FROM rControl";
	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
	if (ret[0][0] == "")
	{
		rControlEmpty = true;
	}
	else
	{
		rControlEmpty = false;
	}

	// Zum Importieren von Geräten, die nicht erfasst wurden
	// Fileformat: Eine Zeile pro Gerät
	// hostname;ip-adresse;mac-adresse;interface-name;location;type;modell;s/n;
	int returnValue = 0;
	// Variablen zurücksetzen
	if (testImportFile())
	{
		// Fehler -> Ende
		return 1;
	}

	// Wenn rControl komplett leer ist, dann abbrechen
	sString = "SELECT MAX(dev_id) FROM rControl";
	ret = dieDB->query(sString.c_str());
	if (ret[0][0] == "")
	{
		std::string fehler = "6207: Importing devices without running the Parser at least once is not possible!";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehler, "6207", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 1;
	}

	
	hostname = ipAddress = macAddress = location = devType = intfName = chassisSN = modell = "";

	std::ifstream importFile(dateiname.c_str(), std::ios_base::in);
	ganzeDatei = "";

	std::string dev_id = "";

	getline(importFile, ganzeDatei, '\0');
	importFile.close();

	std::string dbgA = "";
	// Wenn die Datei leer ist, wird der Import beendet.
	if (ganzeDatei.empty())
	{
		dbgA = "\n6611: " + dateiname + " is empty\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6611", WkLog::WkLog_ROT);
	}
	else
	{
		// ganzeDatei zeilenweise verarbeiten
		std::string importZeile = "";
		aktPos1=aktPos2=0;
		boost::char_separator<char> sep(";", "", boost::keep_empty_tokens);

		for (; aktPos2 < ganzeDatei.npos;)
		{
			aktPos2 = ganzeDatei.find("\n", aktPos1 + 1);
			importZeile = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
			aktPos1 = aktPos2+1;
			
			boost::tokenizer<boost::char_separator<char>> tok(importZeile, sep);

			bool impt = true;			// Geräte importieren?

			int i = 0;
			for(boost::tokenizer<boost::char_separator<char>>::iterator beg=tok.begin(); beg!=tok.end();++beg)
			{
				switch (i)
				{
				case 0:					// hostname
					hostname = *beg;
					if (hostname == "")
					{
						dbgA = "\n6404: Element not imported: Hostname must not be empty: " + importZeile + "\n";
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6404", WkLog::WkLog_ROT);
						impt = false;
					}
					break;
				case 1:					// ip-adresse
					ipAddress = *beg;
					break;
				case 2:					// mac-adresse
					macAddress = *beg;
					break;
				case 3:					// interface-name
					intfName = *beg;
					if (intfName == "")
					{
						intfName = "Intf1";
					}
					break;
				case 4:					// location
					location = *beg;
					break;
				case 5:					// type
					devType = *beg;
					if (devType == "")
					{
						devType = 12;
					}
					break;
				case 6:					// Modell
					modell = *beg;
					break;
				case 7:					// s/n
					chassisSN = *beg;
					break;
				default:
					break;
				}
				i++;
			}

			// Entweder MAC oder IP muss angegeben werden
			if (macAddress == "" && ipAddress == "")
			{
				dbgA = "\n6404: Element not imported: Either MAC or IP address must be specified: " + importZeile + "\n";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6404", WkLog::WkLog_ROT);
				impt = false;
			}

			// Wenn die MAC Adresse leer ist, dann anhand der IP Adresse in der Neighbor Tabelle (CAM und ARP) suchen
			if (macAddress == "")
			{
				sString = "SELECT l2_addr FROM neighbor WHERE l3_addr LIKE '" + ipAddress + "'";
				std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
				if (!result.empty())
				{
					macAddress = result[0][0];
				}
			}

			// Prüfen, ob es den Hostnamen schon gibt. Wenn ja, dann prüfen, ob es sich um ein neues Interface handelt
			std::string sString = "";
			if (!rControlEmpty)
			{
				sString = "SELECT hostname,dev_id FROM device WHERE hostname LIKE '" + hostname + "' AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl))"; 
			}
			else
			{
				sString = "SELECT hostname,dev_id FROM device WHERE hostname LIKE '" + hostname + "'"; 
			}

			std::vector<std::vector<std::string> > result = dieDB->query(sString.c_str());
			if (!result.empty())
			{
				sString = "SELECT hostname FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
				sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id WHERE hostname LIKE '";
				sString += hostname + "' AND interfaces.macAddress LIKE '" + macAddress + "'";
				
				std::vector<std::vector<std::string> > result1 = dieDB->query(sString.c_str());
				
				// Wenn es den Hostnamen mit dem Interface (MAC Adresse) schon gibt, dann nicht einfügen
				if (!result1.empty())
				{
					dbgA = "\n6405: Element not imported: Device already exists in database: " + hostname + "\n";
					schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6405", WkLog::WkLog_ROT);
					impt = false;
				}
				else
				{
					dev_id = result[0][1];
				}
			}


			if (impt)
			{
				// Import der Daten in die Datenbank
				// 1. Gerät anlegen
				if (dev_id == "")
				{
					std::string iString = "INSERT INTO device (dev_id,type,hwtype,hostname,snmpLoc,devType,sn) VALUES(NULL, " + devType + ", " + devType + ", '" + hostname + "', '" + location + "', '" + modell + "', '" + chassisSN + "');";
					dieDB->query(iString.c_str());
					std::string sString = "SELECT last_insert_rowid() FROM device;";
					std::vector<std::vector<std::string>> result = dieDB->query(sString.c_str());
					if (result.empty())
					{
						std::string dbgA = "6205: Database error. Please delete wkm.db and restart! ";
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6205", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						returnValue = 2;
						return returnValue;
					}
					else
					{
						dev_id = result[0][0];
					}
				}


				// 2. Interface anlegen... 
				std::string iString = "INSERT INTO interfaces (intfName, phl, macAddress, ipAddress, subnetMask, status, l2l3) VALUES ('" + intfName + "',1, '" + macAddress + "', '" + ipAddress + "',' ', 'UP', 'L3');";
				dieDB->query(iString.c_str());

				sString = "SELECT last_insert_rowid() FROM interfaces;";
				std::vector<std::vector<std::string> > ret3 = dieDB->query(sString.c_str());
				std::string intf_id = ret3[0][0];

				// ... und mit Gerät verlinken
				iString = "INSERT INTO devInterface (interfaces_int_id, device_dev_id) VALUES (";
				iString += intf_id + "," + dev_id + ");";
				dieDB->query(iString.c_str());
				
				// Am Ende rControl anpassen
				sString = "UPDATE rControl SET dev_id=(SELECT MAX(dev_id) FROM device),intf_id=(SELECT MAX(intf_id) FROM interfaces) WHERE rc_id IN (SELECT MAX(rc_id) FROM rControl)";
				dieDB->query(sString.c_str());
			}
		}
	}

	return returnValue;
}


// testImportFile
// Testen, ob das IP Adressfile konsistent ist
bool WkmImport::testImportFile()
{
#ifndef _WINDOWS_
	newlineEater();
#endif

	std::string ipf;
	std::ifstream eingabe(dateiname.c_str(), std::ios_base::in);
	getline(eingabe, ipf, '\0');
	eingabe.close();

	std::string fehlerHDat = "";


	// Test 1: Ist überhaupt ein ";" vorhanden
	size_t letzterSP = ipf.rfind(";");
	if (letzterSP == ipf.npos)
	{
		fehlerHDat = "6205: Import File inconsistent -> no \";\" found";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "6205", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	// Test 2: Anzahl der ";" modulo Zeilen muss 0 sein

	// Zählen der ";"
	long spAnzahl = 0;
	for (size_t i = 0; i < ipf.size(); i++)
	{
		if (ipf[i] == ';')
		{
			spAnzahl++;
		}
	}

	// Löschen aller Zeichen in einer Zeile nach dem ";"
	size_t pos1 = 0;
	size_t pos2 = 0;
	while (1)
	{
		pos1 = ipf.find(";", pos1);
		if (pos1 == ipf.npos)
		{
			break;
		}
		pos2 = ipf.find("\n", pos1);
		pos1++;
		ipf.erase(pos1, pos2-pos1);
	}

	// Zählen der Zeilen. Etwaige Leerzeilen am Schluss werden gelöscht.
	letzterSP = ipf.find("\n", letzterSP);

	// Wenn kein ENTER am Schluss, dann wird eines hinzugefügt.
	if (letzterSP == ipf.npos)
	{
		ipf += "\n";
		letzterSP = ipf.rfind(";") + 1;
	}

	ipf.erase(letzterSP+1, ipf.npos);

	long zeilenAnzahl = 0;
	for (size_t i = 0; i < ipf.size(); i++)
	{
		if (ipf[i] == '\n')
		{
			zeilenAnzahl++;
		}
	}

	if (spAnzahl%zeilenAnzahl)
	{
		fehlerHDat = "6206: Import File inconsistent -> not all rows have the same number of colums or \";\" is missing at the end of some lines";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "6206", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}
	// Test 3: Anzahl der ";" modulo 10 muss 0 sein
	if (spAnzahl%8)
	{
		fehlerHDat = "6206: Import File inconsistent -> not all rows have the same number of colums or \";\" is missing at the end of some lines";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "6206", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	return false;
}


void WkmImport::newlineEater()
{
	std::string fileInhalt;
	std::ifstream eingabe(dateiname.c_str(), std::ios_base::in);
	getline(eingabe, fileInhalt, '\0');
	eingabe.close();

	if (fileInhalt.find("\n") != fileInhalt.npos)
	{
		size_t pos = 0;
		while (pos != fileInhalt.npos)
		{
			pos = fileInhalt.find("\r");
			if (pos != fileInhalt.npos)
			{
				fileInhalt.erase(pos, 1);
			}
		}
		std::ofstream ausgabe;
		ausgabe.rdbuf()->open(dateiname.c_str(), std::ios_base::out | std::ios_base::binary);
		ausgabe << fileInhalt;
	}

}


void WkmImport::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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



#endif // WKTOOLS_MAPPER