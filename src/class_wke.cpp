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

//#include <boost/asio/ssl.hpp>

#include "cryptlib.h"
#include "wktools4.h"

//#define WKT_DBG

DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_FEHLER)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_DEBUGFEHLER)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_INFO)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_SYSTEMFEHLER)

BEGIN_EVENT_TABLE(Wke, wxEvtHandler)
	EVT_COMMAND(wxID_ANY, wkEVT_EINSPIELEN_FERTIG, Wke::OnWkeFertig)
	EVT_LOG(wxID_ANY, Wke::OnWkeLog)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Wke::Wke() : wxEvtHandler()
{
	vari[CWkePropGrid::WKE_PROPID_INT_MODUS] = 3;
	vari[CWkePropGrid::WKE_PROPID_INT_TYPE] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_SHOW] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_TERMSERVER] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION] = 0;
	
	vari[CWkePropGrid::WKE_PROPID_INT_DYNPW] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW] = 1;
	vari[CWkePropGrid::WKE_PROPID_INT_RAUTE] = 1;
	vari[CWkePropGrid::WKE_PROPID_INT_LOGSY] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_F2301] = 1;
	vari[CWkePropGrid::WKE_PROPID_INT_SHOWDATE] = 1;
	vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND] = 0;
	vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE] = 0;

	vars[CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG] = ".txt";
	vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = "tempDevGroup.txt";

#ifdef _WINDOWS_
	vars[CWkePropGrid::WKE_PROPID_STR_ODIR] = "c:\\";
#else
	vars[CWkePropGrid::WKE_PROPID_STR_ODIR] = "~";
#endif
	
	vars[CWkePropGrid::WKE_PROPID_STR_EXEC] = "enable";
	vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = "log.txt";
	vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND] = "telnet $ip";
	vars[CWkePropGrid::WKE_PROPID_STR_PORT] = "";
	vars[CWkePropGrid::WKE_PROPID_STR_SHP] = "";


	profilIndex = 0;
	wkeProps = NULL;
	logAusgabe = NULL;
	wkMain = NULL;
	serVerbinder = NULL;
	wkFehler = NULL;

	devGroupInit = false;
	devGroupSPanzahl = 10;
	devGroupIK = false;
	devGroupV2 = false;
	devGroupV3 = false;
	stopThread = false;
	ipPortFile = false;

	ende = false;
	ueFehler = false;

	configDatei = "";
	ipA = "";

	thrd = NULL;
	mitThread = true;

	sqlConn = NULL;
}


// Destruktor:
Wke::~Wke()
{
	delete thrd;
}


// Funktionszeiger
bool(Wke::*cfgTest)(std::string);							// Funktionszeiger für ConfigTest Funktionen
bool(Verbinder::*initMethode)();						// Funktionszeiger für die Initialisierung der Verbindungsart
dqstring(Verbinder::*configMethode)(std::string, bool);		// Funktionszeiger für die Initialisierung der ConfigDatei


// void DoIt:
// Tool Start; Es werden die Einstellungen überprüft und an den Ersetzer übergeben
void Wke::doIt(bool wt)
{
#ifdef WKT_DBG
		schreibeLog(WkLog::WkLog_ZEIT, "2701: DOIT\r\n", "2701", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif

	if (guiStatus)
	{
		// Wenn kein GUI verwendet wird, dann braucht das DeviceGroup Fenster auch nicht aktualisiert werden.
		devGroupInit = true;
	}
	if (!devGroupInit)
	{
		// Fehler, wenn bei Verwendung des GUIs der "DeviceGroup Window Refresh" Button nicht gedrückt wurde
		schreibeLog(WkLog::WkLog_ZEIT, "2401: Click \"DeviceGroup Window Refresh\" before Starting!\r\n", "2401", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		
		wxCommandEvent event(wkEVT_WKE_FERTIG);
		OnWkeFertig(event);
	}
	else
	{
		derzeitigeIP = 0;

		schreibeLog(WkLog::WkLog_ABSATZ, "2601: Configure Devices START\r\n", "2601", 0,
			WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

		badIP="";
		worstIP="";
		goodIP="";
		//badIP = "\n\nIP Addresses with WARNINGS:\n";
		//worstIP = "\n\nIP Addresses with MAJOR ERRORS:\n";
		//goodIP = "\n\nIP Addresses without any errors:\n";

		bool fehler = doSave("");
		if (!fehler)
		{
			if (!guiStatus)
			{
				// GUI Mode
				schreibeDevGroup(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);
				aktualisiereDevGroup();
			}


			schreibeLog(WkLog::WkLog_ZEIT, "2602: Starting Configure Devices\r\n", "2602", 0,
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			if (wt)
			{
				thrd = new boost::thread(boost::bind(&Wke::startVerbinder,this));
			}
			else
			{
				mitThread = false;
				startVerbinder();
			}
		}
		else
		{
			schreibeLog(WkLog::WkLog_ZEIT, "2402: Cannot Start because of wrong or missing entries!\r\n", "2402", 0,
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);

			wxCommandEvent event(wkEVT_EINSPIELEN_FERTIG);
			event.SetInt(2);
			wxPostEvent(this, event);
		}
	}
}


// void startVerbinder
// Einstiegsfunktion in den Thread
bool Wke::startVerbinder()
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: startVerbinder\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
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
	return ret;
}


bool Wke::startSeriellDevGroup()
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: startSeriellDevGroup\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	// Verbinder starten und globale Einstellungen setzen
	boost::asio::io_service ioservice;
	//boost::asio::ssl::context ctx(ioservice, boost::asio::ssl::context::sslv23);
	//ctx.set_verify_mode(boost::asio::ssl::context::verify_none);

	Verbinder *derVerbinder = new Verbinder(logAusgabe, this, ioservice/*, ctx*/);
	serVerbinder = derVerbinder;

	derVerbinder->setDebug(vari[CWkePropGrid::WKE_PROPID_INT_DBG_SEND], vari[CWkePropGrid::WKE_PROPID_INT_DBG_RECEIVE], 
		vari[CWkePropGrid::WKE_PROPID_INT_DBG_FUNCTION], vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTEST], 
		vari[CWkePropGrid::WKE_PROPID_INT_DBG_HOSTNAME], vari[CWkePropGrid::WKE_PROPID_INT_DBG_BUFTESTDET], 
		vari[CWkePropGrid::WKE_PROPID_INT_DBG_SHOW], vari[CWkePropGrid::WKE_PROPID_INT_DBG_RGX], vari[CWkePropGrid::WKE_PROPID_INT_DBG_HEX],
		vari[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL1], vari[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL2]);

	// Funktionszeiger für die Verbinder Funktionen
	uint (Verbinder::*verbinden)(std::string, uint, uint, dqstring, int);


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
	// Show Pattern: Zusatz im Filenamen bei Show Ausgabe
	// MHOPCOMMAND: Multihop Command -> wie wird weiterverbunden (mit welchem Kommando)
	// LOGINMODE: Was wird bei falschem Enable Passwort gesendet
	// RAUTE: Auf # warten
	derVerbinder->setEinstellungen(vars[CWkePropGrid::WKE_PROPID_STR_EXEC], vari[CWkePropGrid::WKE_PROPID_INT_SHOW], 
		vars[CWkePropGrid::WKE_PROPID_STR_ODIR], vars[CWkePropGrid::WKE_PROPID_STR_SHP], vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND], 
		vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE], vari[CWkePropGrid::WKE_PROPID_INT_RAUTE], vari[CWkePropGrid::WKE_PROPID_INT_F2301]);

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
	
	derzeitigeIP = 0;
	// Für jede IP Adresse wird nun eine Session initiiert. 
	// Bei Multihop wird die erste Session beibehalten und von dort weiterverbunden
	for(derzeitigeIP = 0; !dgSpalten[1].empty(); derzeitigeIP++)
	{
		// Beschreibung auslesen
		desc = dgSpalten[WKE_DGS_REMARK].front();
		if (stopThread)
		{
			stopThread = false;
			break;
		}

		if (dgSpalten[WKE_DGS_CHECK].front() == "x")
		{
			bool abbrechen = false;
			bool keinFehler = true;
			ipA = dgSpalten[WKE_DGS_HOSTNAME].front();

			std::string infoVorgang = "2603: Configuring " + ipA + " ...";
			
			schreibeLog(WkLog::WkLog_ZEIT, infoVorgang, "2603", 0,
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			// Einlesen der Konfiguration
			// Falls ein Fehler entdeckt wurde, wird zum nächsten Host weitergegangen
			std::string confName = dgSpalten[WKE_DGS_CONFIG].front();
			if (confName == "ERROR")
			{
				schreibeLog(WkLog::WkLog_NORMALTYPE, "2307: No Config File found!", "2307",  0,
					WkLog::WkLog_ROT, WkLog::WkLog_FETT, WkLog::WkLog_SYSTEMFEHLER);
				
				// Löschen der verwendeten Werte in den DevGrp-Spalten-Queues
				for (int i = 0; i < 11; i++)
				{
					dgSpalten[i].pop();
				}

				continue;
			}
			
			// Im CLI Mode muss überprüft werden, ob der Pfad vom Config File passt. Im Fall muss zwischen Linux und Windows Pfaden geswitcht werden
			// Im GUI Modus wird das bei aktualisiereDevGroup gemacht.
			if (guiStatus)
			{
				confName = dirUmbauen(confName);
			}

			// Einlesen der Config
			if (confName == "Inventory")
			{
				// Wenn Inventory und HTTP/S, dann soll die Config leer sein
				if (dgSpalten[WKE_DGS_PROTOKOLL].front().find("HTTP") != dgSpalten[WKE_DGS_PROTOKOLL].front().npos)
				{
					configMethode = &Verbinder::leereConf;
				}
				else
				{
					configMethode = &Verbinder::inventoryConf;
				}
			}
			else if (confName == "Interface")
			{
				configMethode = &Verbinder::intfConf;
			}
			else if (confName == "Mapper")
			{
				// Wenn Mapper und HTTP/S, dann soll das IPT Template verwendet werden
				if (dgSpalten[WKE_DGS_PROTOKOLL].front().find("HTTP") != dgSpalten[WKE_DGS_PROTOKOLL].front().npos)
				{
					configMethode = &Verbinder::mapperConfIPT;
				}
				else
				{
					configMethode = &Verbinder::mapperConf;
				}
			}
			else
			{
				configMethode = &Verbinder::statConfig;
			}
			dqstring conf = (derVerbinder->*configMethode)(confName, false);
			if (!conf.empty())
			{
				if (conf.front() == "ERROR!")
				{
					schreibeLog(WkLog::WkLog_NORMALTYPE, "2307: No Config File found!", "2307",  0,
						WkLog::WkLog_ROT, WkLog::WkLog_FETT, WkLog::WkLog_SYSTEMFEHLER);

					// Löschen der verwendeten Werte in den DevGrp-Spalten-Queues
					for (int i = 0; i < 11; i++)
					{
						dgSpalten[i].pop();
					}

					continue;
				}
			}
			//dqstring conf = derVerbinder->statConfig(confName, false);

			// Setzen von Username/Passwort
			derVerbinder->setUserPass(dgSpalten[WKE_DGS_USERNAME].front(), dgSpalten[WKE_DGS_LOPW].front(), dgSpalten[WKE_DGS_ENPW].front());

			// Setzen vom Multihop Command
			derVerbinder->setMultiHop(dgSpalten[WKE_DGS_MHCOMMAND].front());

			// Setzen vom VerbindungsPort und Terminal Server Settings
			derVerbinder->setPort(dgSpalten[WKE_DGS_PORT].front(), dgSpalten[WKE_DGS_TSC].front());

			// Setzen der showAusgabe Parameter
			derVerbinder->setShowAppend(vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND], vari[CWkePropGrid::WKE_PROPID_INT_SHOWDATE]);

			derVerbinder->setDesc(desc);

			// Protokollauswertung
			// Modus auswerten
			std::string protokoll = dgSpalten[WKE_DGS_PROTOKOLL].front();

			bool dummyCommand = false;		// Dummy Command setzen (für SingleHop SSH und Telnet)

			// Löschen der verwendeten Werte in den DevGrp-Spalten-Queues
			for (int i = 0; i < 11; i++)
			{
				dgSpalten[i].pop();
			}

			if (protokoll == "Telnet")
			{
				mods = Verbinder::NEU;
				initMethode = &Verbinder::telnetInit;
				verbinden = &Verbinder::telnetVerbindung;
				initialMuss = false;
				dummyCommand = true;
			}
			else if (protokoll == "COM1")
			{
				mods = Verbinder::NEU;
				derVerbinder->setModus(Verbinder::COM1);
				initMethode = &Verbinder::consoleInit;
				verbinden = &Verbinder::consoleVerbindung;
				initialMuss = true;
				dummyCommand = false;
			}
			else if (protokoll == "COMx")
			{
				mods = Verbinder::NEU;
				derVerbinder->setModus(Verbinder::COMx);
				initMethode = &Verbinder::consoleInit;
				verbinden = &Verbinder::consoleVerbindung;
				initialMuss = true;
				dummyCommand = false;
			}
			else if (protokoll == "SSH")
			{
				mods = Verbinder::NEU;
				derVerbinder->setModus(Verbinder::SSH2);
				initMethode = &Verbinder::sshInit;
				verbinden = &Verbinder::sshVerbindung;
				initialMuss = false;
				dummyCommand = true;
			}
			else if (protokoll == "SSHv1")
			{
				mods = Verbinder::NEU;
				derVerbinder->setModus(Verbinder::SSH1);
				initMethode = &Verbinder::sshInit;
				verbinden = &Verbinder::sshVerbindung;
				initialMuss = false;
				dummyCommand = true;
			}
			else if (protokoll == "SSHv2")
			{
				mods = Verbinder::NEU;
				derVerbinder->setModus(Verbinder::SSH2);
				initMethode = &Verbinder::sshInit;
				verbinden = &Verbinder::sshVerbindung;
				initialMuss = false;
				dummyCommand = true;
			}
			else if (protokoll == "HTTP")
			{
				mods = Verbinder::NEU;
				initMethode = &Verbinder::httpInit;
				verbinden = &Verbinder::httpVerbindung;
				initialMuss = false;
				dummyCommand = false;
			}
			else if (protokoll == "HTTPS")
			{
				mods = Verbinder::NEU;
				initMethode = &Verbinder::httpsInit;
				verbinden = &Verbinder::httpsVerbindung;

				//initMethode = &Verbinder::httpsInitAsio;
				//verbinden = &Verbinder::httpsVerbindungAsio;

				initialMuss = false;
				dummyCommand = false;
			}
			else if (protokoll == "CLI")
			{
				mods = Verbinder::NEU;
				initMethode = &Verbinder::cliInit;
				verbinden = &Verbinder::cliVerbindung;
				initialMuss = false;
				dummyCommand = false;
			}
			else
			{
				std::string error = "\n2308: " + ipA + ": Unknown Protocol!";
				schreibeLog(WkLog::WkLog_NORMALTYPE, error, "2308", 0,
					WkLog::WkLog_ROT, WkLog::WkLog_FETT, WkLog::WkLog_SYSTEMFEHLER);

				continue;
			}


			if (mods == Verbinder::NEU)
			{
				if (vari[CWkePropGrid::WKE_PROPID_INT_TYPE] == 1)			// Multihop
				{
					derVerbinder->setDummyCommand(false);					// Bei MultiHop immer deaktivieren
					mods = Verbinder::WEITER;
					if (ersterDurchlauf)
					{
						tempProt = protokoll;
						abbrechen = (derVerbinder->*initMethode)();
					}
					else if (tempProt != protokoll)
					{
						std::string error = "\n2309: " + ipA + ": Use the same protocol for all Multihop devices!";
						schreibeLog(WkLog::WkLog_NORMALTYPE, error, "2309", 0,
							WkLog::WkLog_ROT, WkLog::WkLog_FETT, WkLog::WkLog_SYSTEMFEHLER);

						continue;
					}
				}
				else
				{
					abbrechen = (derVerbinder->*initMethode)();
					derVerbinder->setDummyCommand(dummyCommand);
				}
			}
			// Falls ein gravierender Fehler während der for- Schleife erkannt wurde, wird das Programm beendet
			if (abbrechen)
				break;


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
			switch ((derVerbinder->*verbinden)(ipA, status, mods, conf, derzeitigeIP))
			{
			case Verbinder::SOCKETERROR:
				if (mods == Verbinder::WEITER)
				{
					abbrechen = true;
					std::string error = "\n2403: Socket Error with " + ipA;
					schreibeLog(WkLog::WkLog_NORMALTYPE, error, "2403", 0,
						WkLog::WkLog_ROT, WkLog::WkLog_FETT, WkLog::WkLog_SYSTEMFEHLER);
					break;
				}
			case Verbinder::SCHLECHT:
				{
					std::string error = "\n2404: Connection Error with " + ipA;
					schreibeLog(WkLog::WkLog_NORMALTYPE, error, "2404", 0,
						WkLog::WkLog_ROT, WkLog::WkLog_FETT, WkLog::WkLog_SYSTEMFEHLER);
					break;
				}
			case Verbinder::NOCONN:
				{
					std::string error = "\n2405: No connection with " + ipA;
					schreibeLog(WkLog::WkLog_NORMALTYPE, error, "2405", 0,
						WkLog::WkLog_ROT, WkLog::WkLog_FETT, WkLog::WkLog_SYSTEMFEHLER);
					break;
				}
			default:
				break;
			}
			// Falls ein gravierender Fehler während der for- Schleife erkannt wurde, wird das Programm beendet
			if (abbrechen)
				break;
		}
		else
		{
			// Löschen der verwendeten Werte in den DevGrp-Spalten-Queues
			for (int i = 0; i < 11; i++)
			{
				dgSpalten[i].pop();
			}
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

	desc = "";
	ende = true;

	wxCommandEvent event(wkEVT_WKE_FERTIG);
	OnWkeFertig(event);

	return TRUE;
}



// bool doLoad:
// zum Laden eines Profils. Die Einstellungen werden überprüft, bevor sie geladen werden
// return Werte: true: erfolgreich geladen
//               false: Fehler beim Laden der Werte
bool Wke::doLoad(std::string profilname)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: doLoad\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	schreibeLog(WkLog::WkLog_ABSATZ, "2604: Configure Devices: Loading Profile\r\n", "2604", 0,
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (xmlInit(profilname))
	{
		einstellungenInit();
		schreibeLog(WkLog::WkLog_ZEIT, "2605: Configure Devices: Profile \"" + profilname + "\" successfully loaded\r\n", "2605", 0,
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		return true;
	}
	schreibeLog(WkLog::WkLog_ZEIT, "2310: Configure Devices: Unable to load Profile \"" + profilname + "\"\r\n", "2310", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	return false;
}


// bool doSave:
// zum Sichern der Einstellungen in ein Profil
// return Werte: false: erfolgreich gesichert
//               true: Fehler beim Sichern
bool Wke::doSave(std::string profilname)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: doSave\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "2606: Configure Devices: Saving Profile\r\n", "2606", 0,
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
		schreibeLog(WkLog::WkLog_ZEIT, "2607: Configure Devices: Profile \"" + profilname + "\" successfully saved\r\n", "2607", 0,
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "2311: Configure Devices: Unable to save Profile \"" + profilname + "\"\r\n", "2311", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// bool doRemove:
// zum Löschen eines Profils
// return Werte: false: erfolgreich gelöscht
//               true: Fehler beim Löschen
bool Wke::doRemove(std::string profilName)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: doRemove\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif

	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "2615: Configure Devices: Removing Profile\r\n", "2615", 0, 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (profilName == "" || profilName == "DEFAULT")
	{
		schreibeLog(WkLog::WkLog_ZEIT, "2318: Configure Devices: Unable to remove Profile \"" + profilName + "\" - Default Profile cannot be deleted\r\n", "2318", 0,  
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
		return fehler;
	}

	fehler = xmlInit(profilName, 2);

	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "2614: Configure Devices: Profile \"" + profilName + "\" successfully removed\r\n", "2614", 0,  
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "2318: Configure Devices: Unable to remove Profile \"" + profilName + "\"\r\n", "2318", 0,  
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Wke::sets(TiXmlDocument *document, bool guiStats, WkLogFile *lf, CSqlConn *sqlC, CDirMappingData *dirMap, WKErr *fp)
{
	doc = document;

	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;

	logfile = lf;

	sqlConn = sqlC;

	dMaps = dirMap;

	wkFehler = fp;
}


// bool einstellungenInit()
// Zum Laden der Werte, nachdem die richtige Position des Profils in wktools.xml ausgemacht wurde.
bool Wke::einstellungenInit()
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: einstellungenInit\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	TiXmlElement *stringSets = pElemProf->FirstChildElement("Settings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"Hosts", "Config", "ConfDir", "defConFile", "ext", "devGF",
		"User", "lopw", "enpw", "avz", "ena", "logfile", "mHop", "port", "showPat", "hostid", "hostid"};
	std::string attInt[] = {"Modus", "ModArt", "showA", "TSC", "cOption",
		"dynPW", "clearPW", "Raute", "logSy", "devGroup", "append", "login", 
		"tx", "rx", "function", "bufTest", "hostname", "bufTestDet", "showO", "rgx", "F2301", "hex", "sp1", "sp2", "showD"
	};
	int a1 = 16;
	int a2 = 25;

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

			// Verzeichnisse umbauen wenn notwendig
			vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = dirUmbauen(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);
			vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE] = dirUmbauen(vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE]);
			vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE] = dirUmbauen(vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE]);
			vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER] = dirUmbauen(vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER]);
			vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE] = dirUmbauen(vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE]);
			vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = dirUmbauen(vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE]);
			vars[CWkePropGrid::WKE_PROPID_STR_ODIR] = dirUmbauen(vars[CWkePropGrid::WKE_PROPID_STR_ODIR]);

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
				if (attInt[i] == attInt[CWkePropGrid::WKE_PROPID_INT_MODUS])
				{
					vari[i] = 3;
				}
				else
				{
					vari[i] = 0;
				}
			}
			if (!guiStatus)
			{
				// GUI Mode
				wkeProps->einstellungenInit(i, vari[i]);
			}
		}
	}

	if (!guiStatus)
	{
		// GUI Mode
		wkeProps->enableDisable();
	}

	if (logfile != NULL)
	{
		logfile->logFileSets(vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE]);
	}

	if (sqlConn != NULL)
	{
		sqlConn->setDesc(vars[CWkePropGrid::WKE_PROPID_STR_HOSTID]);
	}

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
			std::string profname = pElemProf->Attribute("name");
			profile.Add(profname.c_str());

			pElemProf = pElemProf->NextSiblingElement();
		}
	}

	return profile;
}


void Wke::cancelIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "2608: Configure Devices canceled!\r\n", "2608", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	stopThread = true;
	if (serVerbinder != NULL)
	{
		serVerbinder->cancelVerbindung = true;
	}
	if (mitThread)
	{
		delete thrd;
		thrd = NULL;
	}
}


void Wke::kilIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "2608: Configure Devices killed!\r\n", "2608", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);

	ende = true;
	if (mitThread)
	{
		thrd->interrupt();
	}

	wxCommandEvent event(wkEVT_EINSPIELEN_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


// xmlInit
//********
// Speichern: 0 - laden; 1 - speichern; 2 - löschen
bool Wke::xmlInit(std::string profilName, int speichern)
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
		std::string profname = pElemProf->Attribute("name");
		if (profname == profilName)
		{
			if (speichern == 2)
			{
				bool rmret = pElemWke->RemoveChild(pElemProf);
				return !rmret;
			}
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

		std::string attString[] = {"Hosts", "Config", "ConfDir", "defConFile", "ext", "devGF",
			"User", "lopw", "enpw", "avz", "ena", "logfile", "mHop", "port", "showPat", "hostid"};
		std::string attInt[] = {"Modus", "ModArt", "showA", "TSC", "cOption",
			"dynPW", "clearPW", "Raute", "logSy", "devGroup", "append", "login",
			"tx", "rx", "function", "bufTest", "hostname", "bufTestDet", "showO", "rgx", "F2301", "hex", "sp1", "sp2", "showD"
			};
		int a1 = 16;
		int a2 = 25;

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


bool Wke::doTest(std::string profilname)
{
	// Test, ob alle Eingaben gültig sind
	bool fehler = false;
	devGroupIK = false;

	// Test pre 1: Device Group
	if (vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP])
	{
		std::string fehlerDGDat = "2312: Wrong or missing entry for DEVICE GROUP FILE!";
		std::ifstream devGrpFile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str());
		if (!devGrpFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, "2312", 0,
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
		std::string fehlerHDat = "2312: Wrong or missing entry for HOSTLIST!";

		std::ifstream ipaFile(vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE].c_str());
		if (!ipaFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "2312", 0,
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
			case 5:		// Dyn Config + Default File
				{
					std::string fehlerDefCon = "2312: Wrong or missing entry for DEFAULT CONFIGURATION!";
					std::ifstream defconFile(vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE].c_str());
					if (!defconFile.is_open())
					{
						schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDefCon, "2312", 0,
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehler = true;
					}
					else
					{
						defconFile.close();
					}

				}
				// kein break, da der nächste Test auch notwendig ist.
			case 4:		// Dyn Config
				{
					std::string fehlerVz = "2312: Wrong or missing entry for DYN CONFIG FOLDER!";

					wxFileName fn;

					if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER]))
					{
						schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerVz, "2312", 0,
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);
						fehler = true;
					}
				}
				cfgTest = &Wke::cfgTestDummy;
				break;
			case 0:		// Statisches Config File
				cfgTest = &Wke::cfgTestFile;
				configDatei = vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE];
				break;
			case 1:		// Inventory
				cfgTest = &Wke::cfgTestDummy;
				configDatei = "Inventory";
				break;
			case 2:		// Interface
				cfgTest = &Wke::cfgTestDummy;
				configDatei = "Interface";
				break;
			case 3:		// Mapper
				cfgTest = &Wke::cfgTestDummy;
				configDatei = "Mapper";
				break;
			default:
				break;
		}
		std::string fehlerConfigDatei = "2312: Wrong or missing entry for CONFIG FILE";
		if (!(*this.*cfgTest)(vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE]))
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerConfigDatei, "2312", 0,
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

		// Test 6: Ausgabeverzeichnis:
		std::string fehleravz = "2312: Wrong or missing entry for OUTPUT DIRECTORY";
		wxFileName fn;

		if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_ODIR]))
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehleravz, "2312", 0,
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
	
		// Test 7: Log File
		vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_LOGFILE);
		if (vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] == "")
		{
			vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = vars[CWkePropGrid::WKE_PROPID_STR_ODIR] + "log.txt";
		}
	}

	return fehler;
}


bool Wke::doTestGui(std::string profilname)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: doTestGui\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	if  (!(xmlInit(profilname, true)))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n2103: Init Error -> could not read settings\r\n", "2103", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	TiXmlElement *stringSets = pElemProf->FirstChildElement("Settings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"Hosts", "Config", "ConfDir", "defConFile", "ext", "devGF",
		"User", "lopw", "enpw", "avz", "ena", "logfile", "mHop", "port", "showPat", "hostid"};
	std::string attInt[] = {"Modus", "ModArt", "showA", "TSC", "cOption",
		"dynPW", "clearPW", "Raute", "logSy", "devGroup", "append", "login",
		"tx", "rx", "function", "bufTest", "hostname", "bufTestDet", "showO", "rgx", "F2301", "hex", "sp1", "sp2", "showD"
	};

	bool fehler = false;
	devGroupIK = false;

	// Test pre 1: Device Group

	vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DEVGRP);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DEVGRP], vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP]);
	if (vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP])
	{
		std::string fehlerDGDat = "2312: Wrong or missing entry for DEVICE GROUP FILE!";

		vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE);
		std::ifstream devGrpFile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str());
		if (!devGrpFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, "2312", 0,
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
		std::string fehlerHDat = "2312: Wrong or missing entry for HOSTLIST!";

		vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_HOSTLFILE);
		std::ifstream ipaFile(vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE].c_str());
		if (!ipaFile.is_open())
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "2312", 0,
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
		case 5:		// Dyn Config + Default File
			{
				std::string fehlerDefCon = "2312: Wrong or missing entry for DEFAULT CONFIGURATION!";

				vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE);
				std::ifstream defconFile(vars[CWkePropGrid::WKE_PROPID_STR_DEFCONFIGFILE].c_str());
				if (!defconFile.is_open())
				{
					schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDefCon, "2312", 0,
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
		case 4:		// Dyn Config
			{
				std::string fehlerVz = "2312: Wrong or missing entry for DYN CONFIG FOLDER!";

				wxFileName fn;

				vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER);
				if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER]))
				{
					schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerVz, "2312", 0,
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
			break;
		case 1:		// Inventory
			cfgTest = &Wke::cfgTestDummy;
			configDatei = "Inventory";
			break;
		case 2:		// Interface
			cfgTest = &Wke::cfgTestDummy;
			configDatei = "Interface";
			break;
		case 3:		// Interface
			cfgTest = &Wke::cfgTestDummy;
			configDatei = "Mapper";
			break;
		default:
			break;
		}
		std::string fehlerConfigDatei = "2312: Wrong or missing entry for CONFIG FILE";
		
		vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_CONFIGFILE);
		if ((*this.*cfgTest)(vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE]))
		{
			stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE], vars[CWkePropGrid::WKE_PROPID_STR_CONFIGFILE]);
		}
		else
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerConfigDatei, "2312", 0,
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
	vars[CWkePropGrid::WKE_PROPID_STR_SHP] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_SHP);
	stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_SHP], vars[CWkePropGrid::WKE_PROPID_STR_SHP]);

	vari[CWkePropGrid::WKE_PROPID_INT_SHOW] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_SHOW);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_SHOW], vari[CWkePropGrid::WKE_PROPID_INT_SHOW]);
	
	vari[CWkePropGrid::WKE_PROPID_INT_SHOWDATE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_SHOWDATE);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_SHOWDATE], vari[CWkePropGrid::WKE_PROPID_INT_SHOWDATE]);
	vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND], vari[CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND]);
	vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_LOGINMODE);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_LOGINMODE], vari[CWkePropGrid::WKE_PROPID_INT_LOGINMODE]);
	vars[CWkePropGrid::WKE_PROPID_STR_EXEC] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_EXEC);
	stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_EXEC], vars[CWkePropGrid::WKE_PROPID_STR_EXEC]);
	vars[CWkePropGrid::WKE_PROPID_STR_HOSTID] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_HOSTID);
	stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_HOSTID], vars[CWkePropGrid::WKE_PROPID_STR_HOSTID]);

	vari[CWkePropGrid::WKE_PROPID_INT_RAUTE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_RAUTE);
	vari[CWkePropGrid::WKE_PROPID_INT_LOGSY] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_LOGSY);
	if (!vari[CWkePropGrid::WKE_PROPID_INT_RAUTE])
	{
		vari[CWkePropGrid::WKE_PROPID_INT_LOGSY] = 0;
	}
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_RAUTE], vari[CWkePropGrid::WKE_PROPID_INT_RAUTE]);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_LOGSY], vari[CWkePropGrid::WKE_PROPID_INT_LOGSY]);


	// Test 6: Ausgabeverzeichnis:
	std::string fehleravz = "2312: Wrong or missing entry for OUTPUT DIRECTORY";

	wxFileName fn;
	std::string avz = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_ODIR);
	if (avz != "")
	{
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
	}

	vars[CWkePropGrid::WKE_PROPID_STR_ODIR] = avz;
	if (!fn.DirExists(vars[CWkePropGrid::WKE_PROPID_STR_ODIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehleravz, "2312", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_ODIR], vars[CWkePropGrid::WKE_PROPID_STR_ODIR]);
	}
	
	// Test 7: Log File
	vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_LOGFILE);
	if (vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] == "")
	{
		vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE] = vars[CWkePropGrid::WKE_PROPID_STR_ODIR] + "log.txt";
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_LOGFILE], vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE]);
		wkeProps->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_LOGFILE, vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE]);
	}
	else
	{
		stringSets->SetAttribute(attString[CWkePropGrid::WKE_PROPID_STR_LOGFILE], vars[CWkePropGrid::WKE_PROPID_STR_LOGFILE]);
	}
	
	// Diverse Optionen speichern
	vari[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_VISIBLEPW);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW], vari[CWkePropGrid::WKE_PROPID_INT_VISIBLEPW]);
	vari[CWkePropGrid::WKE_PROPID_INT_MODUS] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_MODUS);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_MODUS], vari[CWkePropGrid::WKE_PROPID_INT_MODUS]);
	vari[CWkePropGrid::WKE_PROPID_INT_TYPE] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_TYPE);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_TYPE], vari[CWkePropGrid::WKE_PROPID_INT_TYPE]);
	vari[CWkePropGrid::WKE_PROPID_INT_F2301] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_F2301);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_F2301], vari[CWkePropGrid::WKE_PROPID_INT_F2301]);
	
	
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
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_RGX] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_RGX);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_RGX], vari[CWkePropGrid::WKE_PROPID_INT_DBG_RGX]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_HEX] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_HEX);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_HEX], vari[CWkePropGrid::WKE_PROPID_INT_DBG_HEX]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL1] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL1);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL1], vari[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL1]);
	vari[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL2] = wkeProps->leseIntSetting(CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL2);
	intSets->SetAttribute(attInt[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL2], vari[CWkePropGrid::WKE_PROPID_INT_DBG_SPECIAL2]);


	return fehler;
}


void Wke::schreibeLog(int type, std::string logEintrag, std::string log2, int report, int farbe, int format, int dringlichkeit, int groesse)
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
	evtDat.type = type;
	evtDat.logEintrag2 = log2;
	evtDat.logEintrag3 = desc;
	evtDat.toolSpecific2 = 0;
	evtDat.toolSpecific = derzeitigeIP;
	evtDat.severity = dringlichkeit;
	evtDat.ipa = ipA;
	evtDat.report = report;

	LogEvent evt(EVT_LOG_MELDUNG);
	evt.SetData(evtDat);
	
	wxPostEvent(this, evt);
	
	//if (logAusgabe != NULL)
	//{
	//	wxPostEvent(logAusgabe, evt);
	//}

	//if (logfile != NULL)
	//{
	//	wxPostEvent(logfile, evt);
	//}
}


void Wke::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


// OnWkeFertig
// Wird durch Nachricht aufgerufen, wenn der Hauptthread (damit das GUI nicht blockiert) fertig ist
void Wke::OnWkeFertig(wxCommandEvent &event)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: OnWkeFertig\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	if (ende)
	{
		if ((worstIP == "") && (badIP == ""))
		{
			schreibeLog(WkLog::WkLog_ZEIT, "2610: Configuring devices ended without any errors\r\n", "2610", 1,
				WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
		}
		else
		{
			schreibeLog(WkLog::WkLog_ZEIT, "2609: Configuring devices ended with errors!", "2609", 1,
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		}
		if (worstIP != "")
		{
			worstIP = "\n\nIP Addresses with MAJOR ERRORS:\n" + worstIP;
			schreibeLog(WkLog::WkLog_NORMALTYPE, worstIP, "", 1,
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		}
		if (badIP != "")
		{
			badIP ="\n\nIP Addresses with WARNINGS:\n" + badIP;
			schreibeLog(WkLog::WkLog_NORMALTYPE, badIP, "", 1,
				WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
		}
		if (goodIP != "")
		{
			goodIP = "\n\nIP Addresses without any errors:\n" + goodIP;
			schreibeLog(WkLog::WkLog_NORMALTYPE, goodIP, "", 1,
				WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		}

		std::multimap <std::string, std::string>::iterator fehlerCodesIter;
		std::string logA = "";
		std::string letzterHost = "";
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n\r\nDetailed Warning/Error List: ", "", 1, WkLog::WkLog_SCHWARZ, WkLog::WkLog_FETT);

		std::string fehlerErklaerung = "";		// String mit allen Erklärungen zu den Fehlern, die in dieser Session aufgetreten sind
		std::string fcs;						// Liste mit den bereits verwerteten Fehlercodes; Für Fehlererklärung interessant, damit für einen Fehlercode nur einmal eine Erklärung ausgegeben wird.

		for (fehlerCodesIter = fehlerCodes.begin(); fehlerCodesIter != fehlerCodes.end();	++fehlerCodesIter)
		{
			if (letzterHost != fehlerCodesIter->first)
			{
				letzterHost = fehlerCodesIter->first;
				logA = "\r\n" + fehlerCodesIter->first + ": ";
				schreibeLog(WkLog::WkLog_NORMALTYPE, logA, "", 1, WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);
			}
			logA = fehlerCodesIter->second + ", ";
			schreibeLog(WkLog::WkLog_NORMALTYPE, logA, "", 1, WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);

			if (fcs.find(fehlerCodesIter->second + ";") == fcs.npos)
			{
				fehlerErklaerung += "\n" + fehlerCodesIter->second + ": " + wkFehler->getFehlerDetail(fehlerCodesIter->second) + "\n";
				fcs += fehlerCodesIter->second + ";";
			}
		}

		fehlerCodes.clear();

		schreibeLog(WkLog::WkLog_NORMALTYPE, "\n" + fehlerErklaerung, "", 1, WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);

		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n\r\n", "", 0,
			WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
		std::string ausg = "2611: Configure Devices END\n\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, ausg, "2611", 1,
			WkLog::WkLog_GRUEN, WkLog::WkLog_FETT);

	}
	

	if (wkMain != NULL)
	{
		wxCommandEvent event(wkEVT_WKE_FERTIG);
		event.SetInt(1);
		wxPostEvent(wkMain, event);
	}
	else
	{
		// Programm beenden, wenn kein GUI vorhanden
		wxGetApp().ExitMainLoop();
	}
}


// testDGfile:
//************
// zum Testen, ob das DeviceGroup File konsistent ist.
bool Wke::testDGfile(std::string dgfile)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: testDGfile\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
#ifndef _WINDOWS_
	newlineEater(dgfile);
#endif
	std::string dgf;
	std::ifstream eingabe(dgfile.c_str(), std::ios_base::in);
	getline(eingabe, dgf, '\0');
	eingabe.close();
	devGroupV2 = devGroupV3 = false;

	std::string fehlerDGDat = "";

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
	else
	{
		vPos = dgf.find("wktoolsDgVer=3;");
		if (vPos < zeilenende)
		{
			devGroupV3 = true;
			dgf.erase(0, zeilenende+1);
		}
	}

	// Test 1.1: Ist überhaupt ein ";" vorhanden
	size_t letzterSP = dgf.rfind(";");
	if (letzterSP == dgf.npos)
	{
		fehlerDGDat = "2209: DEVICE GROUP FILE inconsistent! -> no \";\" found";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, "2209", 0,
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
	else if (devGroupV3)
	{
		devGroupSPanzahl += 2;
	}

	// Test 1.3
	// Ebenso kann es vorkommen, dass die erste Spalte fehlt
	// -> hinzufügen
	size_t erstesZeichen = dgf.find_first_of("x;");
	if (erstesZeichen > 0)
	{
		fehlerDGDat = "2209: DEVICE GROUP FILE inconsistent! -> First Column (Checkboxes) missing!";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, "2209", 0,
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
		fehlerDGDat = "2209: DEVICE GROUP FILE inconsistent! -> Not all rows have the same number of columns or \";\" is missing at the end of some rows";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, "2209", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	// Test 3: Anzahl der ";" modulo 10 muss 0 sein
	if (spAnzahl%devGroupSPanzahl)
	{
		fehlerDGDat = "2209: DEVICE GROUP FILE inconsistent! -> Not all rows have 10 columns";
		if (devGroupV2)
		{
			fehlerDGDat = "2209: DEVICE GROUP FILE inconsistent! -> Not all rows have 11 columns";
		}
		else if (devGroupV3)
		{
			fehlerDGDat = "2209: DEVICE GROUP FILE inconsistent! -> Not all rows have 12 columns";
		}
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDGDat, "2209", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	return false;
}


// testIPAfile
// Testen, ob das IP Adressfile konsistent ist
bool Wke::testIPAfile(std::string ipfile)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: testIPAfile\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
#ifndef _WINDOWS_
	newlineEater(ipfile);
#endif

	std::string ipf;
	std::ifstream eingabe(ipfile.c_str(), std::ios_base::in);
	getline(eingabe, ipf, '\0');
	eingabe.close();

	std::string fehlerHDat = "";


	// Test 1: Ist überhaupt ein ";" vorhanden
	size_t letzterSP = ipf.rfind(";");
	if (letzterSP == ipf.npos)
	{
		fehlerHDat = "2210: HOSTLIST inconsistent -> no \";\" found";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "2210", 0,
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
		fehlerHDat = "2210: HOSTLIST inconsistent -> not all rows have the same number of colums or \";\" is missing at the end of some lines";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "2210", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	// Test 3: Es dürfen keine Spaces im File vorkommen
	if (ipf.find(" ") != ipf.npos)
	{
		fehlerHDat = "2210: HOSTLIST inconsistent -> SPACES found in hostnames or IP addresses";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerHDat, "2210", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	return false;
}


// cfgTestFile
// Testen, ob das angegebene Config File vorhanden ist
bool Wke::cfgTestFile(std::string fname)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: cfgTestFile\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	std::ifstream file(fname.c_str());
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
bool Wke::cfgTestDummy(std::string dummyString)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: cfgTestDummy\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	return true;
}

// schreibeDevGroup:
// die Daten vom Devgroup Fenster in ein File schreiben
void Wke::schreibeDevGroup(std::string filename)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: schreibeDevGroup\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
#ifdef _WINDOWS_
	std::string pfadSeperator = "\\";
#else
	std::string pfadSeperator = "/";
#endif
	if (devGroupIK)
	{

	}
	else
	{
		// DeviceGroup File schreiben
		vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = wkeProps->leseStringSetting(CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE);
		if (filename == "")
		{
			if (vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] == "")
			{
				filename = vars[CWkePropGrid::WKE_PROPID_STR_ODIR] + pfadSeperator + "tempDevGroup.txt";
			}
			else
			{
				filename = vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE];
			}
		}
		
		if (filename == "tempDevGroup.txt")
		{
			filename = vars[CWkePropGrid::WKE_PROPID_STR_ODIR] + pfadSeperator + filename;
		}

		// Check, ob es den Filenamen schon gibt; Soll nur gemacht werden, wenn UseDeviceGroupFile nicht angehakt
		bool nichtschreiben = false;
		if (vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP] == 0)
		{
			std::ifstream devGrpFileTest(filename.c_str());
			if (devGrpFileTest.is_open())
			{
				// DevGrp File gibt es bereits
				int ret = wxMessageBox("Do you want to overwrite existing DeviceGroup File?", "DevGrp Warning", wxYES_NO | wxCANCEL);
				if (ret == wxNO)
				{
					wxRenameFile(filename, filename + ".old", true);
				}
				else if (ret == wxCANCEL)
				{
					nichtschreiben = true;
				}
			}
		}

		if (!nichtschreiben)
		{
			std::ofstream devGroupAusgabe;
			devGroupAusgabe.rdbuf()->open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
			devGroupAusgabe << "wktoolsDgVer=3;\n";
			for (int i = 0; i < devGrp->devGrpAnsicht->GetNumberRows(); i++)
			{
				for (int j = 0; j < 12; j++)
				{
					if (j == WKE_DGS_CHECK)
					{
						std::string chkbxStat = devGrp->devGrpAnsicht->GetCellValue(i, j).c_str();
						if (chkbxStat != "")
						{
							devGroupAusgabe << "x";
						}
						devGroupAusgabe << ";";
					}
					else if (j == WKE_DGS_CONFIG)
					{
						std::string siTxt = devGrp->devGrpAnsicht->GetCellValue(i, j).c_str();

						if (siTxt == "Inventory")
						{
							cfgTest = &Wke::cfgTestDummy;
						}
						else if (siTxt == "Interface")
						{
							cfgTest = &Wke::cfgTestDummy;
						}
						else if (siTxt == "Mapper")
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
						std::string siTxt = devGrp->devGrpAnsicht->GetCellValue(i, j).c_str();
						devGroupAusgabe << siTxt << ";";
					}
				}
				devGroupAusgabe << "\n";
			}
			devGroupAusgabe.close();
			schreibeLog(WkLog::WkLog_ZEIT, "2613: DeviceGroup File written\r\n", "2613", 0,
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);



			// Use DeviceGroup auf TRUE setzen
			vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP] = 1;
			wkeProps->einstellungenInit(9, vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP]);

			// DeviceGroup File auf das temporär generierte setzen
			vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE] = filename;
			wkeProps->einstellungenInit(5, vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]);
			wkeProps->enableDisable();
		}
	}

}

// aktualisiereDevGroup
// DeviceGroup Ansicht wird aktualisiert
void Wke::aktualisiereDevGroup()
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: aktualisiereDevGroup\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	devGroupInit = true;
	doSave("");
	
	devGrp->devGrpAnsicht->ClearGrid();
	if (devGrp->devGrpAnsicht->GetNumberRows())
	{
		devGrp->devGrpAnsicht->DeleteRows(0, devGrp->devGrpAnsicht->GetNumberRows());
	}
	devGrp->devGrpAnsicht->unCheck(true);

	if (devGroupIK)
	{

	}
	// DevGroup Ansicht aus Optionen neu erstellen
	else if (!vari[CWkePropGrid::WKE_PROPID_INT_DEVGRP])
	{
		// IP Adress File einlesen und die IP Adressen ausgeben.
		// Es werden auch sonst alle Elemente ausgegeben
		std::string ipdt = vars[CWkePropGrid::WKE_PROPID_STR_HOSTLFILE];
		std::string ips;
		std::ifstream eingabe(ipdt.c_str(), std::ios_base::in);
		getline(eingabe, ips, '\0');
		eingabe.close();

		char seps[] = ";,";				// gültige Seperatoren für die IP Adressen

		size_t pos1 = 0;				// Position 1 bei der Suche nach den IP Adressen
		size_t pos2 = 0;				// Position 2 bei der Suche nach den IP Adressen
		size_t pos3 = 0;				// Position 3 bei der Suche nach den IP Adressen; Wichtig für die Remarks

		int zaehler = 0;
		ipPortFile = false;
		while (pos2 != ips.npos)
		{
			pos2 = ips.find_first_of(seps, pos1);
			size_t groesse = pos2 - pos1;// - 1;
			if ((groesse > 6) && (pos2 != ips.npos))
			{
				std::string ipaddress = ips.substr(pos1, groesse);
				
				pos3 = pos2+1; // ; weglassen
				pos2 = ips.find_first_of("\r\n", pos2);
				pos1 = pos2 + 1;
				
				// Check, ob das File per wktools Portscanner generiert wurde. In dem Fall dann die erste Zeile auslassen
				if (ipaddress == "wktools-generated")
				{
					ipPortFile = true;
					continue;
				}
				
				// Remark suchen
				std::string remark = "";
				if(pos2-pos3 >= 1)
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
				if (!ipPortFile)
				{
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_REMARK, remark.c_str());
				}

				// Protokoll, Port und MH Command hinzufügen
				devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_MHCOMMAND, vars[CWkePropGrid::WKE_PROPID_STR_MHOPCOMMAND].c_str());
				if (!ipPortFile)
				{
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_PORT, vars[CWkePropGrid::WKE_PROPID_STR_PORT].c_str());

					std::string protokoll = "";
					switch (vari[CWkePropGrid::WKE_PROPID_INT_MODUS])
					{
					case 0:
						protokoll = "COM1";
						break;
					case 1:
						protokoll = "COMx";
						break;
					case 2:
						protokoll = "Telnet";
						break;
					case 3:
						protokoll = "SSH";
						break;
					case 4:
						protokoll = "HTTP";
						break;
					case 5:
						protokoll = "HTTPS";
						break;
					case 6:
						protokoll = "CLI";
						break;
					default:
						break;
					}
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_PROTOKOLL, protokoll.c_str());
				}
				else
				{
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_PORT, remark);
					std::string protokoll = "";
					if (remark == "22")
					{
						protokoll = "SSH";
					}
					else if (remark == "23")
					{
						protokoll = "Telnet";
					}
					else if (remark == "80")
					{
						protokoll = "HTTP";
					}
					else if (remark == "443")
					{
						protokoll = "HTTPS";
					}
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_PROTOKOLL, protokoll.c_str());
			}

				std::string tsc = "";
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
				if (vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION] < 4)		// Statische Config
				{
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_CONFIG, configDatei);
				}
				else
				{
					std::string dcf = vars[CWkePropGrid::WKE_PROPID_STR_DYNCONFIGFOLDER];
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
					std::string filename = dcf + ipaddress;
					filename += vars[CWkePropGrid::WKE_PROPID_STR_ERWEITERUNG];

					std::string aFile = "";
					if (!cfgTestFile(filename))
					{
						if (vari[CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION] == 5)	// Dynamic + Default
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
						std::ifstream ganzeConfig(aFile.c_str(), std::ios_base::in);
						std::string zeile;
						uint upwZaehler = 0;					// Zaehler für dynamische User/PW, wieviele USer/PW schon gefunden wurden	
						while (!ganzeConfig.eof())
						{
							getline(ganzeConfig, zeile, '\n');
							if (!zeile.empty())
							{
								std::string upw[] = {"!user", "!lopw", "!enpw"};
								std::string userpass;		// Hier wird der User/PW zwischengespeichert
								// Drei Durchläufe, da es drei Sachen zu finden gilt.
								std::string zw;	// temp. Buffer
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
			std::ifstream ganzeDatei(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str(), std::ios_base::in);

			// Wenn das DevGroup File ein Versions 2 File ist, dann muss die Versionsanzeige ignoriert werden
			if (devGroupV2 ||devGroupV3)
			{
				ganzeDatei.ignore(16);
			}
			std::string zeile;
			size_t pos1, pos2;
			int zaehler = 0;

			while (!ganzeDatei.eof())
			{
				getline(ganzeDatei, zeile, '\n');

				if (zeile.size() > 5)
				{
					if (zaehler >= devGrp->devGrpAnsicht->GetNumberRows())
					{
						devGrp->zeileHinzu();
					}
					devGrp->devGrpAnsicht->SetCellValue(zaehler, WKE_DGS_CHECK, "");
					
					pos1 = 0;
					pos2 = 0;
					std::string temp = "";
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
							if (i == 5) // Config File
							{
								temp = dirUmbauen(temp);
							}
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
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: ladeDevGrp\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif
	// TODO: DevGrp File Spalten in die qstrings zuordnen...
	// Test ob devGroupFile ok ist.
	for (int i=0; i < devGroupSPanzahl; i++)
	{
		while (!dgSpalten[i].empty())
		{
			dgSpalten[i].pop();
		}
	}
	if (testDGfile(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE]))
	{
		// Nicht OK
		return false;
	}
	else
	{
		// OK
		std::ifstream ganzeDatei(vars[CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE].c_str(), std::ios_base::in);

		// Wenn das DevGroup File ein Versions 2 File ist, dann muss die Versionsanzeige ignoriert werden
		if (devGroupV2 ||devGroupV3)
		{
			ganzeDatei.ignore(16);
		}
		std::string zeile;
		size_t pos1, pos2;
		while (!ganzeDatei.eof())
		{
			getline(ganzeDatei, zeile, '\n');

			if (zeile.size() > 5)
			{
				pos1 = 0;
				pos2 = 0;
				std::string temp = "";
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


void Wke::OnWkeLog(LogEvent &event)
{
	boost::any data = event.GetData();
	WkLog::evtData evtDat = event.GetData<WkLog::evtData>();
	if (evtDat.report != 1)
	{
		evtDat.report = 0;
	}
	
	if (data.type() == typeid(WkLog::evtData))
	{
		if (evtDat.toolSpecific2 == Verbinder::WZNIP)
		{
			std::string ipaddr = evtDat.ipa;
			std::string suchString = ipaddr + "\n";
			if ((worstIP.find(suchString) == worstIP.npos) && (badIP.find(suchString) == badIP.npos))
			{
				std::string suchString = evtDat.ipa + "\n";
				if (goodIP.find(suchString) == goodIP.npos)
				{
					if (!guiStatus)
					{
						// Nur im GUI Mode
						wxColour hintergrund("#00FF00");
						for (int i=0; i < 11; i++)
						{
							devGrp->devGrpAnsicht->SetCellBackgroundColour(hintergrund, evtDat.toolSpecific, i);
						}
						devGrp->devGrpAnsicht->ForceRefresh();
					}
					goodIP += evtDat.ipa;
					goodIP += "\n";
				}
			}
		}
		switch(evtDat.severity)
		{
		case WkLog::WkLog_INFO:
			break;
		case WkLog::WkLog_FEHLER:
			{
				int doNotLog = wkFehler->getFehlerStatus(evtDat.logEintrag2);
				if (doNotLog == 0)
				{
					if (evtDat.logEintrag2 != "")
					{
						fehlerCodes.insert(pss(evtDat.ipa, evtDat.logEintrag2));
					}
					std::string ipaddr = evtDat.ipa;
					std::string suchString = ipaddr + "\n";

					if (badIP.find(suchString) == badIP.npos)
					{
						if (!guiStatus)
						{
							// Nur im GUI Modus
							wxColour hintergrund("#FBB117");
							for (int i=0; i < 11; i++)
							{
								devGrp->devGrpAnsicht->SetCellBackgroundColour(hintergrund, evtDat.toolSpecific, i);
							}
							devGrp->devGrpAnsicht->ForceRefresh();
						}
						badIP += ipaddr;
						badIP += "\n";
						ueFehler = true;
					}
				}
			}
			break;
		case WkLog::WkLog_SYSTEMFEHLER:
			{
				int doNotLog = wkFehler->getFehlerStatus(evtDat.logEintrag2);
				if (doNotLog == 0)
				{
					if (evtDat.logEintrag2 != "")
					{
						fehlerCodes.insert(pss(evtDat.ipa, evtDat.logEintrag2));
					}
					std::string ipaddr = evtDat.ipa;

					std::string suchString = ipaddr + "\n";
					if (worstIP.find(suchString) == worstIP.npos)
					{
						if (!guiStatus)
						{
							// Nur im GUI Modus
							wxColour hintergrund("#ff0000");
							for (int i=0; i < 11; i++)
							{
								devGrp->devGrpAnsicht->SetCellBackgroundColour(hintergrund, evtDat.toolSpecific, i);
							}
							devGrp->devGrpAnsicht->ForceRefresh();
						}

						worstIP += ipaddr;
						worstIP += "\n";
						ueFehler = true;
					}
				}
			}
			break;
		case WkLog::WkLog_DEBUGFEHLER:
			if (evtDat.logEintrag2 != "")
			{
				int doNotLog = wkFehler->getFehlerStatus(evtDat.logEintrag2);
				if (doNotLog == 0)
				{
					fehlerCodes.insert(pss(evtDat.ipa, evtDat.logEintrag2));
				}
			}
			break;
		default:
			break;
		}

		event.SetData(evtDat);

		if (logAusgabe != NULL)
		{
			wxPostEvent(logAusgabe, event);
		}
		else
		{
			std::cout << evtDat.logEintrag;
		}
		
		if (logfile != NULL)
		{
			wxPostEvent(logfile, event);
		}

		if (sqlConn != NULL)
		{
			evtDat.tool = WkLog::WkLog_WKE;
			event.SetData(evtDat);
			wxPostEvent(sqlConn, event);
		}
	}
}


std::string  Wke::dirUmbauen(std::string dirAlt)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: dirUmbauen\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif

#ifdef _WINDOWS_
	// DeviceGroup File Pfad ändern
	size_t dirEndPos = dirAlt.find_last_of("/");
	if (dirEndPos != dirAlt.npos)
	{
		std::string dir = dirAlt.substr(0, dirEndPos+1);
		dMaps->werteIter = dMaps->dirMapsLin.find(dir);

		if (dMaps->werteIter != dMaps->dirMapsLin.end())
		{
			dirAlt.replace(0, dirEndPos+1, dMaps->werteIter->second);
		}
	}
	return dirAlt;
#else
	// DeviceGroup File Pfad ändern
	size_t dirEndPos = dirAlt.find_last_of("\\");
	if (dirEndPos != dirAlt.npos)
	{
		std::string dir = dirAlt.substr(0, dirEndPos+1);
		dMaps->werteIter = dMaps->dirMapsWin.find(dir);

		if (dMaps->werteIter != dMaps->dirMapsWin.end())
		{
			dirAlt.replace(0, dirEndPos+1, dMaps->werteIter->second);
		}
	}
	return dirAlt;
#endif
}


void Wke::newlineEater(std::string filename)
{
#ifdef WKT_DBG
	schreibeLog(WkLog::WkLog_ZEIT, "2701: newlineEater\r\n", "2701", 0,
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
#endif

	std::string fileInhalt;
	std::ifstream eingabe(filename.c_str(), std::ios_base::in);
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
		ausgabe.rdbuf()->open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
		ausgabe << fileInhalt;
	}
	
}


void Wke::dgInit()
{
	aktualisiereDevGroup();

	for( int i = 0; i < wkMain->dieDevGrp->devGrpAnsicht->GetNumberRows(); i++ )
	{
		wkMain->dieDevGrp->devGrpAnsicht->SetCellValue(i, 0, "1");
	} 

	schreibeDevGroup();
}