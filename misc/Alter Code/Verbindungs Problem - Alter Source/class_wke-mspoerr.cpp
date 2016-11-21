/***************************************************************
* Name:      class_Wke.cpp
* Purpose:   Code for Configure Devices I/O Class
* Created:   2008-08-22
**************************************************************/
// Änderungen
///////////////////////////////////////////////////////////////
//


#include "class_wke.h"

#include <fstream>
#include <vector>
#include <string>
using namespace std;

#include "cryptlib.h"

DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_FEHLER)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_DEBUGFEHLER)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_INFO)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_SYSTEMFEHLER)

BEGIN_EVENT_TABLE(Wke, wxEvtHandler)
EVT_COMMAND(wxID_ANY, wkEVT_EINSPIELEN_FERTIG, Wke::OnWkeFertig)
EVT_COMMAND(wxID_ANY, wkEVT_EINSPIELEN_INFO, Wke::OnWkeInfo)
EVT_COMMAND(wxID_ANY, wkEVT_EINSPIELEN_FEHLER, Wke::OnWkeFehler)
EVT_COMMAND(wxID_ANY, wkEVT_EINSPIELEN_SYSTEMFEHLER, Wke::OnWkeSystemFehler)
EVT_COMMAND(wxID_ANY, wkEVT_EINSPIELEN_DEBUGFEHLER, Wke::OnWkeDebugFehler)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Wke::Wke() : wxEvtHandler()
{
	vari[CWkePropGrid::WKE_PROPID_INT_MODUS] = 2;
	vari[CWkePropGrid::WKE_PROPID_INT_TYPE] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_SHOW] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_TERMSERVER] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION] = 0;
	
	vari[CWkePropGrid::WKE_PROPID_INT_DYNPW] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW] = 1;
	vari[CWkePropGrid::WKE_PROPID_INT_RAUTE] = 1;
	vari[CWkePropGrid::WKE_PROPID_INT_LOGSY] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE] = 0;

	vars[CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG] = ".txt";
	vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = "tempDevGroup.txt";

	vars[CWkePropGrid::WKE_PROPID_STR_ODIR] = "c:\\";
	vars[CWkePropGrid::WKE_PROPID_STR_EXEC] = "enable";
	vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = "log.txt";
	vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND] = "telnet $ip";
	vars[CWkePropGrid::WKE_PROPID_STR_PORT] = "";


	profilIndex = 0;
	wkeProps = NULL;
	logAusgabe = NULL;
	wkMain = NULL;
	serVerbinder = NULL;

	devGroupInit = false;
	devGroupSPanzahl = 10;
	devGroupIK = false;
	devGroupV2 = false;
	stopThread = false;

	ende = false;
	ueFehler = false;

	configDatei = "";
}


// Destruktor:
Wke::~Wke()
{
}


// Funktionszeiger
bool(Wke::*cfgTest)(string);							// Funktionszeiger für ConfigTest Funktionen
void(Verbinder::*initMethode)();						// Funktionszeiger für die Initialisierung der Verbindungsart
dqstring(Verbinder::*configMethode)(string, bool);		// Funktionszeiger für die Initialisierung der ConfigDatei


// void DoIt:
// Tool Start; Es werden die Einstellungen überprüft und an den Ersetzer übergeben
void Wke::doIt()
{
	if (guiStatus)
	{
		// Wenn kein GUI verwendet wird, dann braucht das DeviceGroup Fenster auch nicht aktualisiert werden.
		devGroupInit = true;
	}
	if (!devGroupInit)
	{
		// Fehler, wenn bei Verwendung des GUIs der "DeviceGroup Window Refresh" Button nicht gedrückt wurde
		schreibeLog(WkLog::WkLog_ZEIT, "Click \"DeviceGroup Window Refresh\" before Starting!\r\n", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		
		wxCommandEvent event(wkEVT_WKE_FERTIG);
		OnWkeFertig(event);
	}
	else
	{
		derzeitigeIP = 0;

		schreibeLog(WkLog::WkLog_ABSATZ, "Configure Devices START\r\n", 
			WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

		badIP = "\n\nIP Addresses with WARNINGS:\n";
		worstIP = "\n\nIP Addresses with MAJOR ERRORS:\n";
		goodIP = "\n\nIP Addresses without any errors:\n";

		bool fehler = doSave("");
		if (!fehler)
		{
			if (!guiStatus)
			{
				// GUI Mode
				schreibeDevGroup(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);
				aktualisiereDevGroup();
			}


			schreibeLog(WkLog::WkLog_ZEIT, "Starting Configure Devices\r\n", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			thrd = boost::thread(boost::bind(&Wke::startVerbinder,this));
		}
		else
		{
			schreibeLog(WkLog::WkLog_ZEIT, "Cannot Start because of wrong or missing entries!\r\n", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);

			wxCommandEvent event(wkEVT_EINSPIELEN_FERTIG);
			event.SetInt(2);
			wxPostEvent(this, event);
		}
	}


}


// void startVerbinder
// Einstiegsfunktion in den Thread
void Wke::startVerbinder()
{
	// Verbindungsart auswerten
	bool ret = 0;

	if (vari[CWkePropGrid::WKE_PROPID_INT_TYPE] == 2)		// Parallel -> for future Use
	{
		plll = true;
//		ret = startParallelDevGroup();
	}
	else
	{
		ret = startSeriellDevGroup();
	}
}


bool Wke::startSeriellDevGroup()
{
	// Verbinder starten und globale Einstellungen setzen
	asio::io_service ioservice;
	Verbinder *derVerbinder = new Verbinder(logAusgabe, this, ioservice);
	serVerbinder = derVerbinder;

	derVerbinder->setDebug(vari[CWkePropGrid::WKE_PROPID_INT_DBG_SEND], vari[CWkePropGrid::WKE_PROPID_INT_DBG_RECEIVE], 
		vari[CWkePropGrid::WKE_PROPID_INT_DBG_FUNCTION], vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTEST], 
		vari[CWkePropGrid::WKE_PROPID_INT_DBG_HOSTNAME], vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTESTDET], 
		vari[CWkePropGrid::WKE_PROPID_INT_DBG_SHOW]);

	// Funktionszeiger für die Verbinder Funktionen
	uint (Verbinder::*verbinden)(string, uint, uint, dqstring);


	// Was wird bei falschem Enable Passwort gesendet; Umsetzen auf Verbinder Werte
	if (vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE] == 1)
	{
		vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE] = Verbinder::COMMAND;
	}
	else
	{
		vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE] = Verbinder::EXIT;
	}

	// Setzen diverser Einstellungen
	// EXEC: Was tun bei "router>"
	// SHOW: Wie soll das show Output File gespeichert werden
	// ODIR: Output Directory
	// MHOPCOMMAND: Multihop Command -> wie wird weiterverbunden (mit welchem Kommando)
	// LOGINMODE: Was wird bei falschem Enable Passwort gesendet
	// RAUTE: Auf # warten
	derVerbinder->setEinstellungen(vars[CWkePropGrid::WKE_PROPID_STR_EXEC], vari[CWkePropGrid::WKE_PROPID_INT_SHOW], 
		vars[CWkePropGrid::WKE_PROPID_STR_ODIR], vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND], 
		vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE], vari[CWkePropGrid::WKE_PROPID_INT_RAUTE]);

	uint status = 0;					// Neue Verbindung oder Weiter?
	bool initialMuss = true;			// "true" für Verbindungsarten, die eine Startverbindung brauchen
	uint mods = 0;						// Welcher Modus wird verwendet? Zwei Arten: NEU und WEITER
	bool ersterDurchlauf = true;		// Um feststellen zu können, ob die nachfolgende for Schleife
										// zum ersten Mal ganz durchlaufen wurde

	// DeviceGroupFile laden
	if (!ladeDevGrp())
	{
		return false;
	}
	
	// Für jede IP Adresse wird nun eine Session initiiert. 
	// Bei Multihop wird die erste Session beibehalten und von dort weiterverbunden
	for(derzeitigeIP = 0; !dgSpalten[1].empty(); derzeitigeIP++)
	{
		if (stopThread)
		{
			stopThread = false;
			break;
		}

		if (dgSpalten[WKE_DGS_CHECK].front() == "x")
		{
			bool abbrechen = false;
			bool keinFehler = true;
			ueFehlerJetzt = false;
			string ipA = dgSpalten[WKE_DGS_HOSTNAME].front();

			string infoVorgang = "Configuring " + ipA + " ...";
			
			schreibeLog(WkLog::WkLog_ZEIT, infoVorgang, 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			// Einlesen der Konfiguration
			// Falls ein Fehler entdeckt wurde, wird zum nächsten Host weitergegangen
			string confName = dgSpalten[WKE_DGS_CONFIG].front();
			if (confName == "ERROR")
			{
				schreibeLog(WkLog::WkLog_NORMALTYPE, "No Config File found!", 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				continue;
			}

			// Einlesen der Config
			if (confName == "INVENTORY")
			{
				configMethode = &Verbinder::inventoryConf;
			}
			else if (confName == "INTERFACE")
			{
				configMethode = &Verbinder::intfConf;
			}
			else
			{
				configMethode = &Verbinder::statConfig;
			}
			dqstring conf = (derVerbinder->*configMethode)(confName, false);
			//dqstring conf = derVerbinder->statConfig(confName, false);

			// Setzen von Username/Passwort
			derVerbinder->setUserPass(dgSpalten[WKE_DGS_USERNAME].front(), dgSpalten[WKE_DGS_LOPW].front(), dgSpalten[WKE_DGS_ENPW].front());

			// Setzen vom Multihop Command
			derVerbinder->setMultiHop(dgSpalten[WKE_DGS_MHCOMMAND].front());

			// Setzen vom VerbindungsPort und Terminal Server Settings
			derVerbinder->setPort(dgSpalten[WKE_DGS_PORT].front(), dgSpalten[WKE_DGS_TSC].front());

			// Setzen der showAusgabe Parameter
			derVerbinder->setShowAppend(vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND]);

			// Protokollauswertung
			// Modus auswerten
			string protokoll = dgSpalten[WKE_DGS_PROTOKOLL].front();
			if (protokoll == "Telnet")
			{
				mods = Verbinder::NEU;
				initMethode = &Verbinder::telnetInit;
				verbinden = &Verbinder::telnetVerbindung;
				initialMuss = false;
			}
			else if (protokoll == "COM1")
			{
				mods = Verbinder::WEITER;
				derVerbinder->setModus(Verbinder::COM1);
				initMethode = &Verbinder::consoleInit;
				verbinden = &Verbinder::consoleVerbindung;
				initialMuss = true;
			}
			else if (protokoll == "COMx")
			{
				mods = Verbinder::WEITER;
				derVerbinder->setModus(Verbinder::COMx);
				initMethode = &Verbinder::consoleInit;
				verbinden = &Verbinder::consoleVerbindung;
				initialMuss = true;
			}
			else if (protokoll == "SSHv1")
			{
				mods = Verbinder::NEU;
				derVerbinder->setModus(Verbinder::SSH1);
				initMethode = &Verbinder::sshInit;
				verbinden = &Verbinder::sshVerbindung;
				initialMuss = false;
			}
			else if (protokoll == "SSHv2")
			{
				mods = Verbinder::NEU;
				derVerbinder->setModus(Verbinder::SSH2);
				initMethode = &Verbinder::sshInit;
				verbinden = &Verbinder::sshVerbindung;
				initialMuss = false;
			}
			else if (protokoll == "HTTP")
			{
				mods = Verbinder::NEU;
				initMethode = &Verbinder::httpInit;
				verbinden = &Verbinder::httpVerbindung;
				initialMuss = false;
			}
			else
			{
				string error = "\n" + ipA + ": Unknown Protocol!";
				schreibeLog(WkLog::WkLog_NORMALTYPE, error, 
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);

				continue;
			}


			if (mods == Verbinder::NEU)
			{
				if (vari[CWkePropGrid::WKE_PROPID_INT_TYPE] == 1)			// Multihop
				{
					mods = Verbinder::WEITER;
					if (ersterDurchlauf)
					{
						tempProt = protokoll;
						(derVerbinder->*initMethode)();
					}
					else if (tempProt != protokoll)
					{
						string error = "\n" + ipA + ": Use the same protocol for all Multihop devices!";
						schreibeLog(WkLog::WkLog_NORMALTYPE, error, 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						continue;
					}
				}
				else
				{
					(derVerbinder->*initMethode)();
				}
			}

			// Setzen von logsy

			uint loggsy = 0;
			if (vari[CWkePropGrid::WKE_PROPID_INT_LOGSY])
			{
				loggsy = Verbinder::LOGGSYNC;
			}
			else
			{
				loggsy = Verbinder::NICHTS;
			}
			derVerbinder->setLogsy(loggsy);


			// Beim ersten for- Durchlauf wird immer eine neue Verbindung aufgebaut, bei allen anderen Durchläufen wird unterschieden,
			// ob eine bestehende Verbindung genutzt wird, oder eine neue aufgebaut wird.
			if (ersterDurchlauf)
			{
				status = Verbinder::NEU;
			}
			else
			{
				if (mods == Verbinder::WEITER)
				{
					status = Verbinder::WEITER;
				}
				else
				{
					status = Verbinder::NEU;
				}
			}
			ersterDurchlauf = false;

			// Nachdem alle Verbindungsdetails feststehen, wird nun ein Verbindungsversuch durchgeführt
			switch ((derVerbinder->*verbinden)(ipA, status, mods, conf))
			{
			case Verbinder::SOCKETERROR:
				if (mods == Verbinder::WEITER)
				{
					abbrechen = true;
					string error = "\nSocket Error with " + ipA;
					schreibeLog(WkLog::WkLog_NORMALTYPE, error, 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					break;
				}
			case Verbinder::SCHLECHT:
				{
					string error = "\nConnection Error with " + ipA;
					schreibeLog(WkLog::WkLog_NORMALTYPE, error, 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					break;
				}
			case Verbinder::NOCONN:
				{
					string error = "\nNo connection with " + ipA;
					schreibeLog(WkLog::WkLog_NORMALTYPE, error, 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					break;
				}
			default:
				break;
			}
			if (!ueFehlerJetzt)
			{
				string suchString = ipA + "\n";
				if (goodIP.find(suchString) == goodIP.npos)
				{
					wxColour hintergrund("#00FF00");
					for (int i=0; i < 11; i++)
					{
						devGrp->devGrpAnsicht->SetCellBackgroundColour(hintergrund, derzeitigeIP, i);
					}
					devGrp->devGrpAnsicht->ForceRefresh();
					goodIP += ipA;
					goodIP += "\n";
				}
			}
			// Falls ein gravierender Fehler während der for- Schleife erkannt wurde, wird das Programm beendet
			if (abbrechen)
				break;
		}
		// Löschen der verwendeten Werte in den DevGrp-Spalten-Queues
		for (int i = 0; i < 11; i++)
		{
			dgSpalten[i].pop();
		}
		// Beim letzten Durchlauf muss die Verbindung auf jeden Fall geschlossen werden.		
		if (dgSpalten[1].empty())
		{
			mods = Verbinder::ENDE;
		}

		//else
		//{
		//	devGroup->m_ListCtrl.zeilenFarben.push_back(WKE_ZF_WEISS);
		//}
	}

	// notwendige Aufräumarbeiten...
	delete derVerbinder;

	ende = true;

	wxCommandEvent event(wkEVT_WKE_FERTIG);
	OnWkeFertig(event);

	return TRUE;
}



// bool doLoad:
// zum Laden eines Profils. Die Einstellungen werden überprüft, bevor sie geladen werden
// return Werte: true: erfolgreich geladen
//               false: Fehler beim Laden der Werte
bool Wke::doLoad(string profilname)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "Configure Devices: Loading Profile\r\n", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (xmlInit(profilname))
	{
		einstellungenInit();
		schreibeLog(WkLog::WkLog_ZEIT, "Configure Devices: Profile \"" + profilname + "\" successfully loaded\r\n", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		return true;
	}
	schreibeLog(WkLog::WkLog_ZEIT, "Configure Devices: Unable to load Profile \"" + profilname + "\"\r\n", 
		WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	return false;
}


// bool doSave:
// zum Sichern der Einstellungen in ein Profil
// return Werte: false: erfolgreich gesichert
//               true: Fehler beim Sichern
bool Wke::doSave(string profilname)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "Configure Devices: Saving Profile\r\n", 
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
		schreibeLog(WkLog::WkLog_ZEIT, "Configure Devices: Profile \"" + profilname + "\" successfully saved\r\n", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "Configure Devices: Unable to save Profile \"" + profilname + "\"\r\n", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Wke::sets(TiXmlDocument *document, bool guiStats)
{
	doc = document;

	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;
}


// bool einstellungenInit()
// Zum Laden der Werte, nachdem die richtige Position des Profils in wktools.xml ausgemacht wurde.
bool Wke::einstellungenInit()
{
	TiXmlElement *stringSets = pElemProf->FirstChildElement("Settings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	string attString[] = {"Hosts", "Config", "ConfDir", "defConFile", "ext", "devGF",
		"User", "lopw", "enpw", "avz", "ena", "logfile", "mHop", "port"};
	string attInt[] = {"Modus", "ModArt", "showA", "TSC", "cOption",
		"dynPW", "clearPW", "Raute", "logSy", "devGroup", "append", "login",
		"tx", "rx", "function", "bufTest", "hostname", "bufTestDet", "showO"
	};
	int a1 = 14;
	int a2 = 19;

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
				vars[i] = "";
			}
			if (!guiStatus)
			{
				// GUI Mode
				wkeProps->einstellungenInit(i, vars[i]);
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
				vari[i] = 0;
			}
			if (!guiStatus)
			{
				// GUI Mode
				wkeProps->einstellungenInit(i, vari[i]);
			}
		}
	}

	wkeProps->enableDisable();


	return true;
}


void Wke::guiSets(CWkePropGrid *wkeProp, WkLog *ausgabe, DevGrpPanel *dg)
{
	wkeProps = wkeProp;
	logAusgabe = ausgabe;
	devGrp = dg;
}


wxArrayString Wke::ladeProfile()
{
	wxArrayString profile;

	// Grund Elemente für Wke
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWke;		// das Element, das auf Wke zeigt

	// Wke im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		profile.Empty();
	}
	else
	{
		pElemWke = pElem->FirstChildElement("wke");
		pElemProf = pElemWke->FirstChildElement("Profile");

		for (profilIndex = 0; pElemProf; profilIndex++)
		{
			string profname = pElemProf->Attribute("name");
			profile.Add(profname.c_str());

			pElemProf = pElemProf->NextSiblingElement();
		}
	}

	return profile;
}


void Wke::cancelIt()
{
	stopThread = true;
	if (serVerbinder != NULL)
	{
		serVerbinder->cancelVerbindung = true;
	}
}


void Wke::kilIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "Configure Devices killed!\r\n", 
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);

	ende = true;
	thrd.interrupt();

	wxCommandEvent event(wkEVT_EINSPIELEN_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


bool Wke::xmlInit(string profilName, bool speichern)
{
	if (profilName == "")
	{
		profilName = "DEFAULT";
	}

	// Grund Elemente für Wke
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWke;		// das Element, das auf Wke zeigt

	// Wke im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		return false;
	}

	pElemWke = pElem->FirstChildElement("wke");
	pElemProf = pElemWke->FirstChildElement("Profile");

	for (profilIndex = 0; pElemProf; profilIndex++)
	{
		string profname = pElemProf->Attribute("name");
		if (profname == profilName)
		{
			return true;
		}

		pElemProf = pElemProf->NextSiblingElement();
	}

	if (speichern)
	{
		pElemProf = new TiXmlElement("Profile");		
		pElemWke->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", profilName);

		TiXmlElement *stringSets = new TiXmlElement("Settings");
		TiXmlElement *intSets = new TiXmlElement("IntSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);

		string attString[] = {"Hosts", "Config", "ConfDir", "defConFile", "ext", "devGF",
			"User", "lopw", "enpw", "avz", "ena", "logfile", "mHop", "port"};
		string attInt[] = {"Modus", "ModArt", "showA", "TSC", "cOption",
			"dynPW", "clearPW", "Raute", "logSy", "devGroup", "append", "login",
			"tx", "rx", "function", "bufTest", "hostname", "bufTestDet", "showO"
			};
		int a1 = 14;
		int a2 = 19;

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


bool Wke::doTest(string profilname)
{
	// Test, ob alle Eingaben gültig sind
	bool fehler = false;
	devGroupIK = false;

	// Test pre 1: Device Group
	if (vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP])
	{
		string fehlerDGDat = "Wrong or missing entry for DEVICE GROUP FILE!";
		ifstream devGrpFile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str());
		if (!devGrpFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			devGrpFile.close();
		
			bool retWert = testDGfile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);
			
			if (retWert)
			{
				fehler = true;
				devGroupIK = true;
			}
			else
			{
				devGroupIK = false;
			}
		}
	}
	else
	{
		vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = "tempDevGroup.txt";
		// TODO: espFrm->dgf = devGroupFile;
		
		
		// Test 1: Hostliste:
		string fehlerHDat = "Wrong or missing entry for HOSTLIST!";

		ifstream ipaFile(vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE].c_str());
		if (!ipaFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			ipaFile.close();
			bool retWert = testIPAfile(vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE]);
			if (retWert)
			{
				devGroupIK = true;
				fehler = true;
			}
		}
		
		// Test 2: Konfiguration:
		switch (vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION])
		{
			case 4:		// Dyn Config + Default File
				{
					string fehlerDefCon = "Wrong or missing entry for DEFAULT CONFIGURATION!";
					ifstream defconFile(vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE].c_str());
					if (!defconFile.is_open())
					{
						schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDefCon, 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehler = true;
					}
					else
					{
						defconFile.close();
					}

				}
				// kein break, da der nächste Test auch notwendig ist.
			case 3:		// Dyn Config
				{
					string fehlerVz = "Wrong or missing entry for DYN CONFIG FOLDER!";

					wxFileName fn;

					if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER]))
					{
						schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerVz, 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehler = true;
					}
				}
				cfgTest = &Wke::cfgTestDummy;
				break;
			case 0:		// Statisches Config File
				cfgTest = &Wke::cfgTestFile;
				configDatei = vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE];
				// TODO: configMethode = &Verbinder::statConfig;
				break;
			case 1:		// Inventory
				cfgTest = &Wke::cfgTestDummy;
				configDatei = "Inventory";
				// TODO: configMethode = &Verbinder::inventoryConf;
				break;
			case 2:		// Interface
				cfgTest = &Wke::cfgTestDummy;
				configDatei = "Interface";
				// TODO: configMethode = &Verbinder::intfConf;
				break;
			default:
				break;
		}
		string fehlerConfigDatei = "Wrong or missing entry for CONFIG FILE";
		if (!(*this.*cfgTest)(vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE]))
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerConfigDatei, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		
		// Test 3: User/Passwort: entfällt, da nicht zwingend notwendig. 
		// Test 4: Modus: entfällt, da Default auf SingleHop Telnet.
		// Test 5: Zusatzoptionen: Entfällt ebenfalls, da es bei allen Optionen eine Default Einstellung gibt.

		if (!vari[CWkePropGrid::WKE_PROPID_INT_RAUTE])
		{
			vari[CWkePropGrid::WKE_PROPID_INT_LOGSY] = 0;
		}


		string fehlerLDat = "Missing entry for LOG FILE!";
		if (vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] == "")
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerLDat, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}

		// Test 7: Ausgabeverzeichnis:
		string fehleravz = "\r\nWrong or missing entry for OUTPUT DIRECTORY\r\n";
		wxFileName fn;

		if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_ODIR]))
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehleravz, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
	}

	return fehler;
}


bool Wke::doTestGui(string profilname)
{
	if  (!(xmlInit(profilname, true)))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\nInit Error -> could not read settings\r\n", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	TiXmlElement *stringSets = pElemProf->FirstChildElement("Settings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	string attString[] = {"Hosts", "Config", "ConfDir", "defConFile", "ext", "devGF",
		"User", "lopw", "enpw", "avz", "ena", "logfile", "mHop", "port"};
	string attInt[] = {"Modus", "ModArt", "showA", "TSC", "cOption",
		"dynPW", "clearPW", "Raute", "logSy", "devGroup", "append", "login",
		"tx", "rx", "function", "bufTest", "hostname", "bufTestDet", "showO"
	};

	bool fehler = false;
	devGroupIK = false;

	// Test pre 1: Device Group

	vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DEVGRP);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DEVGRP], vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP]);
	if (vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP])
	{
		string fehlerDGDat = "Wrong or missing entry for DEVICE GROUP FILE!";

		vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE);
		ifstream devGrpFile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str());
		if (!devGrpFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			devGrpFile.close();

			bool retWert = testDGfile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);

			if (retWert)
			{
				fehler = true;
				devGroupIK = true;
			}
			else
			{
				stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE], vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);
				devGroupIK = false;
			}
		}
	}
	else
	{
		vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = "tempDevGroup.txt";
		// TODO: espFrm->dgf = devGroupFile;


		// Test 1: Hostliste:
		string fehlerHDat = "Wrong or missing entry for HOSTLIST!";

		vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_HOSTLFILE);
		ifstream ipaFile(vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE].c_str());
		if (!ipaFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			ipaFile.close();
			bool retWert = testIPAfile(vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE]);
			if (retWert)
			{
				devGroupIK = true;
				fehler = true;
			}
			else
			{
				stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE], vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE]);
			}
		}

		// Test 2: Konfiguration:
		vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION);
		intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION], vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION]);
		switch (vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION])
		{
		case 4:		// Dyn Config + Default File
			{
				string fehlerDefCon = "Wrong or missing entry for DEFAULT CONFIGURATION!";

				vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE);
				ifstream defconFile(vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE].c_str());
				if (!defconFile.is_open())
				{
					schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDefCon, 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehler = true;
				}
				else
				{
					stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE], vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE]);
					defconFile.close();
				}

			}
			// kein break, da der nächste Test auch notwendig ist.
		case 3:		// Dyn Config
			{
				string fehlerVz = "Wrong or missing entry for DYN CONFIG FOLDER!";

				wxFileName fn;

				vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER);
				if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER]))
				{
					schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerVz, 
						WkLog::WkLog_ROT, WkLog::WkLog_FETT);
					fehler = true;
				}
				else
				{
					stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER], vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER]);
				}
			}
			
			vars[CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG);
			stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG], vars[CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG]);
			vari[CWkePropGrid::WKE_PROPID_INT_DYNPW] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DYNPW);
			intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DYNPW], vari[CWkePropGrid::WKE_PROPID_INT_DYNPW]);

			cfgTest = &Wke::cfgTestDummy;
			break;
		case 0:		// Statisches Config File
			cfgTest = &Wke::cfgTestFile;
			configDatei = vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE];
			// TODO: configMethode = &Verbinder::statConfig;
			break;
		case 1:		// Inventory
			cfgTest = &Wke::cfgTestDummy;
			configDatei = "Inventory";
			// TODO: configMethode = &Verbinder::inventoryConf;
			break;
		case 2:		// Interface
			cfgTest = &Wke::cfgTestDummy;
			configDatei = "Interface";
			// TODO: configMethode = &Verbinder::intfConf;
			break;
		default:
			break;
		}
		string fehlerConfigDatei = "Wrong or missing entry for CONFIG FILE";
		
		vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_CONFIGFILE);
		if ((*this.*cfgTest)(vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE]))
		{
			stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE], vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE]);
		}
		else
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerConfigDatei, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}

		// Test 3: User/Passwort: entfällt, da nicht zwingend notwendig. 
		vars[CWkePropGrid::WKE_PROPID_STR_USER] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_USER);
		vars[CWkePropGrid::WKE_PROPID_STR_LOPW] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_LOPW);
		vars[CWkePropGrid::WKE_PROPID_STR_ENPW] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_ENPW);
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_USER], vars[CWkePropGrid::WKE_PROPID_STR_USER]);
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_LOPW], vars[CWkePropGrid::WKE_PROPID_STR_LOPW]);
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_ENPW], vars[CWkePropGrid::WKE_PROPID_STR_ENPW]);

		// Test 4: Modus: entfällt, da Default auf SingleHop Telnet.
		vari[CWkePropGrid::WKE_PROPID_INT_TERMSERVER] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_TERMSERVER);
		intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_TERMSERVER], vari[CWkePropGrid::WKE_PROPID_INT_TERMSERVER]);
		vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND);
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND], vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND]);
		vars[CWkePropGrid::WKE_PROPID_STR_PORT] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_PORT);
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_PORT], vars[CWkePropGrid::WKE_PROPID_STR_PORT]);
	}
	
	// Test 5: Zusatzoptionen: Entfällt ebenfalls, da es bei allen Optionen eine Default Einstellung gibt.
	vari[CWkePropGrid::WKE_PROPID_INT_SHOW] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_SHOW);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_SHOW], vari[CWkePropGrid::WKE_PROPID_INT_SHOW]);
	vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND], vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND]);
	vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_LOGINMODE);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_LOGINMODE], vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE]);
	vars[CWkePropGrid::WKE_PROPID_STR_EXEC] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_EXEC);
	stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_EXEC], vars[CWkePropGrid::WKE_PROPID_STR_EXEC]);

	vari[CWkePropGrid::WKE_PROPID_INT_RAUTE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_RAUTE);
	vari[CWkePropGrid::WKE_PROPID_INT_LOGSY] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_LOGSY);
	if (!vari[CWkePropGrid::WKE_PROPID_INT_RAUTE])
	{
		vari[CWkePropGrid::WKE_PROPID_INT_LOGSY] = 0;
	}
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_RAUTE], vari[CWkePropGrid::WKE_PROPID_INT_RAUTE]);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_LOGSY], vari[CWkePropGrid::WKE_PROPID_INT_LOGSY]);


	string fehlerLDat = "Missing entry for LOG FILE!";
	vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_LOGFILE);
	if (vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerLDat, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_LOGFILE], vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE]);
	}

	// Test 7: Ausgabeverzeichnis:
	string fehleravz = "\r\nWrong or missing entry for OUTPUT DIRECTORY\r\n";

	wxFileName fn;
	string avz = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_ODIR);
#ifdef _WINDOWS_
	if (avz[avz.length()-1] != '\\') 
	{
		avz.append(std::string("\\")); // append '\'
	}
#else
	if (avz[avz.length()-1] != '/') 
	{
		avz.append(std::string("/")); // append '/'
	}
#endif
	vars[CWkePropGrid::WKE_PROPID_STR_ODIR] = avz;
	if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_ODIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehleravz, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_ODIR], vars[CWkePropGrid::WKE_PROPID_STR_ODIR]);
	}
	// Diverse Optionen speichern
	vari[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_VISIBLEPW);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW], vari[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW]);
	vari[CWkePropGrid::WKE_PROPID_INT_MODUS] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_MODUS);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_MODUS], vari[CWkePropGrid::WKE_PROPID_INT_MODUS]);
	vari[CWkePropGrid::WKE_PROPID_INT_TYPE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_TYPE);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_TYPE], vari[CWkePropGrid::WKE_PROPID_INT_TYPE]);
	
	
	// Debug Optionen speichern
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_SEND] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_SEND);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_SEND], vari[CWkePropGrid::WKE_PROPID_INT_DBG_SEND]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_RECEIVE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_RECEIVE);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_RECEIVE], vari[CWkePropGrid::WKE_PROPID_INT_DBG_RECEIVE]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_FUNCTION] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_FUNCTION);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_FUNCTION], vari[CWkePropGrid::WKE_PROPID_INT_DBG_FUNCTION]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTEST] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_BUFTEST);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTEST], vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTEST]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_HOSTNAME] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_HOSTNAME);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_HOSTNAME], vari[CWkePropGrid::WKE_PROPID_INT_DBG_HOSTNAME]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTESTDET] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_BUFTESTDET);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTESTDET], vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTESTDET]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_SHOW] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_SHOW);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_SHOW], vari[CWkePropGrid::WKE_PROPID_INT_DBG_SHOW]);


	return fehler;
}


void Wke::schreibeLog(int type, string logEintrag, int farbe, int format, int groesse)
{
	if (logAusgabe != NULL)
	{
		logAusgabe->schreibeLog(type, logEintrag, farbe, format, groesse);
	}
}


void Wke::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


// OnWkeFertig
// Wird durch Nachricht aufgerufen, wenn der Hauptthread (damit das GUI nicht blockiert) fertig ist
void Wke::OnWkeFertig(wxCommandEvent &event)
{
	if (ende)
	{
		if (ueFehler)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "Configuring devices ended with errors!", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		}
		else
		{
			schreibeLog(WkLog::WkLog_ZEIT, "Configuring devices ended without any errors\r\n", 
				WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
		}
		schreibeLog(WkLog::WkLog_NORMALTYPE, worstIP, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		schreibeLog(WkLog::WkLog_NORMALTYPE, badIP, 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		schreibeLog(WkLog::WkLog_NORMALTYPE, goodIP, 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

		string ausg = "Configure Devices END\n\n";
		schreibeLog(WkLog::WkLog_ZEIT, ausg, 
			WkLog::WkLog_GRUEN, WkLog::WkLog_FETT);

	}
	

	if (wkMain != NULL)
	{
		wxCommandEvent event(wkEVT_WKE_FERTIG);
		event.SetInt(1);
		wxPostEvent(wkMain, event);
	}
}


// testDGfile:
//************
// zum Testen, ob das DeviceGroup File konsistent ist.
bool Wke::testDGfile(string dgfile)
{
	string dgf;
	ifstream eingabe(dgfile.c_str(), ios_base::in);
	getline(eingabe, dgf, '\0');
	eingabe.close();

	string fehlerDGDat = "";

	// Vor den Tests: Zum Schluss muss ein Enter vorkommen
	dgf += "\n";


	// Test 0: Versionsabfrage
	size_t zeilenende = dgf.find("\n");
	size_t vPos = dgf.find("wktoolsDgVer=2;");
	if (vPos < zeilenende)
	{
		devGroupV2 = true;
		dgf.erase(0, zeilenende+1);
	}

	// Test 1.1: Ist überhaupt ein ";" vorhanden
	size_t letzterSP = dgf.rfind(";");
	if (letzterSP == dgf.npos)
	{
		fehlerDGDat = "DEVICE GROUP FILE inconsistent! -> no \";\" found";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	// Test 1.2:
	// Falls das DeviceGroup File im Excel editiert wurde, sind am Zeilenende keine ";"
	// -> Den Test 3 ändern -> "modulo 9" statt "modulo 10"
	zeilenende = dgf.find("\n");
	size_t strichpunkt = dgf.rfind(";", zeilenende);
	if (strichpunkt +2 < zeilenende)
	{
		devGroupSPanzahl = 9;
	}
	else
	{
		devGroupSPanzahl = 10;
	}

	if (devGroupV2)
	{
		devGroupSPanzahl++;
	}

	// Test 1.3
	// Ebenso kann es vorkommen, dass die erste Spalte fehlt
	// -> hinzufügen
	size_t erstesZeichen = dgf.find_first_of("x;");
	if (erstesZeichen > 0)
	{
		fehlerDGDat = "DEVICE GROUP FILE inconsistent! -> First Column (Checkboxes) missing!";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}


	// Test 2: Anzahl der ";" modulo Zeilen muss 0 sein

	// Zählen der ";"
	long spAnzahl = 0;
	for (size_t i = 0; i < dgf.size(); i++)
	{
		if (dgf[i] == ';')
		{
			spAnzahl++;
		}
	}

	// Zählen der Zeilen. Etwaige Leerzeilen am Schluss werden gelöscht.
	letzterSP = dgf.find("\n", letzterSP);
	dgf.erase(letzterSP+1, dgf.npos);

	long zeilenAnzahl = 0;
	for (size_t i = 0; i < dgf.size(); i++)
	{
		if (dgf[i] == '\n')
		{
			zeilenAnzahl++;
		}
	}

	if (spAnzahl%zeilenAnzahl)
	{
		fehlerDGDat = "DEVICE GROUP FILE inconsistent! -> Not all rows have the same number of columns or \";\" is missing at the end of some rows";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	// Test 3: Anzahl der ";" modulo 10 muss 0 sein
	if (spAnzahl%devGroupSPanzahl)
	{
		fehlerDGDat = "DEVICE GROUP FILE inconsistent! -> Not all rows have 10 columns";
		if (devGroupV2)
		{
			fehlerDGDat = "DEVICE GROUP FILE inconsistent! -> Not all rows have 11 columns";
		}
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	return false;
}


// testIPAfile
// Testen, ob das IP Adressfile konsistent ist
bool Wke::testIPAfile(string ipfile)
{
	string ipf;
	ifstream eingabe(ipfile.c_str(), ios_base::in);
	getline(eingabe, ipf, '\0');
	eingabe.close();

	string fehlerHDat = "";


	// Test 1: Ist überhaupt ein ";" vorhanden
	size_t letzterSP = ipf.rfind(";");
	if (letzterSP == ipf.npos)
	{
		fehlerHDat = "HOSTLIST inconsistent -> no \";\" found";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, 
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

	if ((spAnzahl/zeilenAnzahl) != 1)
	{
	fehlerHDat = "HOSTLIST inconsistent -> not all rows have the same number of colums or \";\" is missing at the end of some lines";
	schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	// Test 3: Es dürfen keine Spaces im File vorkommen
	if (ipf.find(" ") != ipf.npos)
	{
		fehlerHDat = "HOSTLIST inconsistent -> SPACES found in hostnames or IP addresses";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	return false;
}


// cfgTestFile
// Testen, ob das angegebene Config File vorhanden ist
bool Wke::cfgTestFile(string fname)
{
	ifstream file(fname.c_str());
	if (!file.is_open())
	{
		return false;
	}
	else
	{
		file.close();
		return true;
	}
}


// cfgTestDummy
// Config File testen. Diese Variante wird ausgewählt, wenn ein internes File verwendet wird (Bsp.: INVENTORY)
bool Wke::cfgTestDummy(string dummyString)
{
	return true;
}

// schreibeDevGroup:
// die Daten vom Devgroup Fenster in ein File schreiben
void Wke::schreibeDevGroup(string filename)
{
#ifdef _WINDOWS_
	string pfadSeperator = "\\";
#else
	string pfadSeperator = "/";
#endif
	// DeviceGroup File schreiben
	if (filename == "")
	{
		filename = vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE];
	}
	
	if (filename == "tempDevGroup.txt")
	{
		filename = vars[CWkePropGrid::WKE_PROPID_STR_ODIR] + pfadSeperator + filename;
	}

	ofstream devGroupAusgabe;
	devGroupAusgabe.rdbuf()->open(filename.c_str(), ios_base::out);
	devGroupAusgabe << "wktoolsDgVer=2;\n";
	for (int i = 0; i < devGrp->devGrpAnsicht->GetNumberRows(); i++)
	{
		for (int j = 0; j < 11; j++)
		{
			if (j == WKE_DGS_CHECK)
			{
				string chkbxStat = devGrp->devGrpAnsicht->GetCellValue(i, j);
				if (chkbxStat != "")
				{
					devGroupAusgabe << "x";
				}
				devGroupAusgabe << ";";
			}
			else if (j == WKE_DGS_CONFIG)
			{
				string siTxt = devGrp->devGrpAnsicht->GetCellValue(i, j);

				if (siTxt == "INVENTORY")
				{
					cfgTest = &Wke::cfgTestDummy;
				}
				else if (siTxt == "INTERFACE")
				{
					cfgTest = &Wke::cfgTestDummy;
				}
				else
				{
					cfgTest = &Wke::cfgTestFile;
				}

				if ((*this.*cfgTest)(siTxt))
				{
					devGroupAusgabe << siTxt << ";";
				}
				else
				{
					devGroupAusgabe << "ERROR;";
				}
			}
			else
			{
				string siTxt = devGrp->devGrpAnsicht->GetCellValue(i, j);
				devGroupAusgabe << siTxt << ";";
			}
		}
		devGroupAusgabe << "\n";
	}
	devGroupAusgabe.close();


	// Use DeviceGroup auf TRUE setzen
	vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP] = 1;
	wkeProps->einstellungenInit(9, vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP]);

	// DeviceGroup File auf das temporär generierte setzen
	vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = filename;
	wkeProps->einstellungenInit(5, vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);
	wkeProps->enableDisable();
}

// aktualisiereDevGroup
// DeviceGroup Ansicht wird aktualisiert
void Wke::aktualisiereDevGroup()
{
	devGroupInit = true;
	doSave("");
	
	devGrp->devGrpAnsicht->ClearGrid();
	devGrp->devGrpAnsicht->DeleteRows(0, devGrp->devGrpAnsicht->GetNumberRows()-2);
	devGrp->devGrpAnsicht->unCheck(true);

	if (devGroupIK)
	{

	}
	// DevGroup Ansicht aus Optionen neu erstellen
	else if (!vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP])
	{
		// IP Adress File einlesen und die IP Adressen ausgeben.
		// Es werden auch sonst alle Elemente ausgegeben
		string ipdt = vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE];
		string ips;
		ifstream eingabe(ipdt.c_str(), ios_base::in);
		getline(eingabe, ips, '\0');
		eingabe.close();

		char seps[] = ";,";				// gültige Seperatoren für die IP Adressen

		size_t pos1 = 0;				// Position 1 bei der Suche nach den IP Adressen
		size_t pos2 = 0;				// Position 2 bei der Suche nach den IP Adressen
		size_t pos3 = 0;				// Position 3 bei der Suche nach den IP Adressen; Wichtig für die Remarks

		int zaehler = 0;
		while (pos2 != ips.npos)
		{
			pos2 = ips.find_first_of(seps, pos1);
			size_t groesse = pos2 - pos1;// - 1;
			if ((groesse > 6) && (pos2 != ips.npos))
			{
				string ipaddress = ips.substr(pos1, groesse);
				pos3 = pos2+1; // ; weglassen
				pos2 = ips.find_first_of("\r\n", pos2);
				pos1 = pos2 + 1;
				// Remark suchen
				string remark = "";
				if(pos2-pos3 > 2)
				{
					remark = ips.substr(pos3, pos2-pos3);
				}
				
				// Neue Zeile anhängen, falls keine leere mehr zur Verfügung steht
				if (zaehler == devGrp->devGrpAnsicht->GetNumberRows())
				{
					devGrp->zeileHinzu();
				}

				// IP Adresse und Remark einfügen
				devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_HOSTNAME, ipaddress.c_str());
				devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_REMARK, remark.c_str());

				// Protokoll, Port und MH Command hinzufügen
				devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_MHCOMMAND, vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND].c_str());
				devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_PORT, vars[CWkePropGrid::WKE_PROPID_STR_PORT].c_str());

				string protokoll = "";
				switch (vari[CWkePropGrid::WKE_PROPID_INT_MODUS])
				{
				case 0:
					protokoll = "COM1";
					break;
				case 1:
					protokoll = "COM2";
					break;
				case 2:
					protokoll = "Telnet";
					break;
				case 3:
					protokoll = "SSHv1";
					break;
				case 4:
					protokoll = "SSHv2";
					break;
				case 5:
					protokoll = "HTTP";
					break;
				default:
					break;
				}
				devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_PROTOKOLL, protokoll.c_str());

				string tsc = "";
				switch (vari[CWkePropGrid::WKE_PROPID_INT_TERMSERVER])
				{
				case 0:
					tsc = "None";
					break;
				case 1:
					tsc = "Authentication";
					break;
				case 2:
					tsc = "No Authentication";
					break;
				default:
					break;
				}
				devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_TSC, tsc.c_str());

				// Username/Passwort und Config File wenn alles statisch
				if (!vari[CWkePropGrid::WKE_PROPID_INT_DYNPW])
				{
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_USERNAME, vars[CWkePropGrid::WKE_PROPID_STR_USER].c_str());
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_ENPW, vars[CWkePropGrid::WKE_PROPID_STR_ENPW].c_str());
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_LOPW, vars[CWkePropGrid::WKE_PROPID_STR_LOPW].c_str());
				}
				if (vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION] < 3)		// Statische Config
				{
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_CONFIG, configDatei);
				}
				else
				{
					string dcf = vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER];
#ifdef _WINDOWS_
					if (dcf[dcf.length()-1] != '\\') 
					{
						dcf.append(std::string("\\")); // append '\'
					}
#else
					if (dcf[dcf.length()-1] != '/') 
					{
						dcf.append(std::string("/")); // append '/'
					}
#endif
					string filename = dcf + ipaddress;
					filename += vars[CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG];

					string aFile = "";
					if (!cfgTestFile(filename))
					{
						if (vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION] == 4)	// Dynamic + Default
						{
							aFile = vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE];
						}
						else
						{
							aFile = "No Config File found!";
						}
					}
					else
					{
						aFile = filename;
					}
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_CONFIG, aFile);

					if (vari[CWkePropGrid::WKE_PROPID_INT_DYNPW])
					{
						int spalte = 0;
						ifstream ganzeConfig(aFile.c_str(), ios_base::in);
						string zeile;
						uint upwZaehler = 0;					// Zaehler für dynamische User/PW, wieviele USer/PW schon gefunden wurden	
						while (!ganzeConfig.eof())
						{
							getline(ganzeConfig, zeile, '\n');
							if (!zeile.empty())
							{
								string upw[] = {"!user", "!lopw", "!enpw"};
								string userpass;		// Hier wird der User/PW zwischengespeichert
								// Drei Durchläufe, da es drei Sachen zu finden gilt.
								string zw;	// temp. Buffer
								for (uint i = upwZaehler; i < 3; i++)
								{
									size_t gefunden = zeile.find(upw[i]);
									if (gefunden != zeile.npos)
									{
										// Falls etwas gefunden wurde, wird nun geschaut, was es ist und der upwZaehler um eins erhöht
										switch (i)
										{
										case 0:
											spalte = WKE_DGS_USERNAME;
											i = 3;
											break;
										case 1:
											spalte = WKE_DGS_LOPW;
											i = 3;
											break;
										case 2:
											spalte = WKE_DGS_ENPW;
											break;
										}
										upwZaehler++;
										devGrp->devGrpAnsicht->SetCellValue(zaehler, spalte, userpass);
									}
								}
							}
						}
						ganzeConfig.close();
					}
				}
				zaehler++;
			}
		}
		// Elemente ausgeben - ENDE -

	}
	// DevGroup Ansicht aus DevGroupFile befüllen
	else
	{
		// Test ob devGroupFile ok ist.
		if (testDGfile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]))
		{	
		}
		else
		{
			ifstream ganzeDatei(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str(), ios_base::in);

			// Wenn das DevGroup File ein Versions 2 File ist, dann muss die Versionsanzeige ignoriert werden
			if (devGroupV2)
			{
				ganzeDatei.ignore(16);
			}
			string zeile;
			size_t pos1, pos2;
			int zaehler = 0;
			while (!ganzeDatei.eof())
			{
				getline(ganzeDatei, zeile, '\n');

				if (zeile.size() > 5)
				{
					if (zaehler == devGrp->devGrpAnsicht->GetNumberRows())
					{
						devGrp->zeileHinzu();
					}
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_CHECK, "");
					
					pos1 = 0;
					pos2 = 0;
					string temp = "";
					for (int i = 0; i < devGroupSPanzahl; i++)
					{
						pos2 = zeile.find(";", pos1);
						if (pos2 == zeile.npos)
						{
							break;
						}

						temp = zeile.substr(pos1, pos2-pos1);
						pos1 = pos2+1;
						if (temp == "x")
						{
							devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_CHECK, "1");
						}
						else
						{
							devGrp->devGrpAnsicht->SetCellValue(zaehler, i, temp.c_str());
						}
					}
				}
				zaehler++;
			}
		}
	}

	devGrp->devGrpAnsicht->ForceRefresh();
}


// ladeDevGrp
// Laden der DeviceGroup in die qstrings
// Return Werte:
//	true: ok; false: nok
bool Wke::ladeDevGrp()
{
	// TODO: DevGrp File Spalten in die qstrings zuordnen...
	// Test ob devGroupFile ok ist.
	if (testDGfile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]))
	{
		// Nicht OK
		return false;
	}
	else
	{
		// OK
		ifstream ganzeDatei(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str(), ios_base::in);

		// Wenn das DevGroup File ein Versions 2 File ist, dann muss die Versionsanzeige ignoriert werden
		if (devGroupV2)
		{
			ganzeDatei.ignore(16);
		}
		string zeile;
		size_t pos1, pos2;
		while (!ganzeDatei.eof())
		{
			getline(ganzeDatei, zeile, '\n');

			if (zeile.size() > 5)
			{
				pos1 = 0;
				pos2 = 0;
				string temp = "";
				for (int i = 0; i < devGroupSPanzahl; i++)
				{
					pos2 = zeile.find(";", pos1);
					if (pos2 == zeile.npos)
					{
						break;
					}

					temp = zeile.substr(pos1, pos2-pos1);
					pos1 = pos2+1;
					dgSpalten[i].push(temp.c_str());
				}
			}
		}
	}

	return true;
}


void Wke::OnWkeDebugFehler(wxCommandEvent &event)
{
	event.SetEventType(wkEVT_SCHREIBELOG);
	wxPostEvent(logAusgabe, event);
}


void Wke::OnWkeInfo(wxCommandEvent &event)
{
	event.SetEventType(wkEVT_SCHREIBELOG);
	wxPostEvent(logAusgabe, event);
}


void Wke::OnWkeSystemFehler(wxCommandEvent &event)
{
	event.SetEventType(wkEVT_SCHREIBELOG);
	wxPostEvent(logAusgabe, event);

	evtData *evtDat = (evtData*)event.GetClientData();
	string ipaddr = evtDat->ipa;

	string suchString = ipaddr + "\n";
	if (worstIP.find(suchString) == worstIP.npos)
	{
		wxColour hintergrund("#ff0000");
		for (int i=0; i < 11; i++)
		{
			devGrp->devGrpAnsicht->SetCellBackgroundColour(hintergrund, derzeitigeIP, i);
		}
		devGrp->devGrpAnsicht->ForceRefresh();

		worstIP += ipaddr;
		worstIP += "\n";
	}
	ueFehler = true;
	ueFehlerJetzt = true;
}


void Wke::OnWkeFehler(wxCommandEvent &event)
{
	event.SetEventType(wkEVT_SCHREIBELOG);
	wxPostEvent(logAusgabe, event);

	evtData *evtDat = (evtData*)event.GetClientData();
	string ipaddr = evtDat->ipa;
	string suchString = ipaddr + "\n";

	if (badIP.find(suchString) == badIP.npos)
	{
		wxColour hintergrund("#FF8040");
		for (int i=0; i < 11; i++)
		{
			devGrp->devGrpAnsicht->SetCellBackgroundColour(hintergrund, derzeitigeIP, i);
		}
		devGrp->devGrpAnsicht->ForceRefresh();
		badIP += ipaddr;
		badIP += "\n";
	}
	ueFehler = true;
	ueFehlerJetzt = true;
}

