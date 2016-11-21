/***************************************************************
* Name:      class_wkl.cpp
* Purpose:   Code for IP List I/O Class
* Created:   2008-08-22
**************************************************************/

#include "class_wkl.h"
#include "wktools4.h"

DEFINE_LOCAL_EVENT_TYPE(wkEVT_IPL_FERTIG)

BEGIN_EVENT_TABLE(Wkl, wxEvtHandler)
EVT_COMMAND(wxID_ANY, wkEVT_IPL_FERTIG, Wkl::OnWklFertig)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Wkl::Wkl() : wxEvtHandler()
{
	vars[CWklPropGrid::WKL_PROPID_STR_SDIR] = "";
	vars[CWklPropGrid::WKL_PROPID_STR_ODIR] = "";
	vars[CWklPropGrid::WKL_PROPID_STR_INTERFACE] = "";
	vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE] = "log.txt";
	vars[CWklPropGrid::WKL_PROPID_STR_PATTERN] = "";
	vars[CWklPropGrid::WKL_PROPID_STR_INTFNR] = "0";
	vars[CWklPropGrid::WKL_PROPID_STR_IPRANGE] = "";


	profilIndex = 0;
	logAusgabe = NULL;
	wkMain = NULL;

	stopThread = false;
	ende = false;
	mitThread = true;

	a1 = 7;
	a2 = 1;
}


// Destruktor:
Wkl::~Wkl()
{
}


// void DoIt:
// Tool Start; Es werden die Einstellungen überprüft und an den Parser übergeben
void Wkl::doIt(bool wt)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "4601: IP List START\r\n", "4601", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	bool fehler = doSave("");
	if (!fehler)
	{
		if (vari[CWklPropGrid::WKL_PROPID_INT_TOOL] == 0)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "4602: Starting IP List\r\n", "4602", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			dieListe = new IPL(vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE], logAusgabe, this);

			if (wt)
			{
				thrd = boost::thread(boost::bind(&Wkl::startParser,this));
			}
			else
			{
				mitThread = false;
				startParser();
			}
		}
		else if (vari[CWklPropGrid::WKL_PROPID_INT_TOOL] == 1)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "4602: Starting Port Scanner\r\n", "4602", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
			dieListe = new IPL(vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE], logAusgabe, this);

			if (wt)
			{
				thrd = boost::thread(boost::bind(&Wkl::startScanner,this));
			}
			else
			{
				mitThread = false;
				startScanner();
			}
		}
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4402: Cannot Start because of wrong or missing entries!\r\n", "4402", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);

		wxCommandEvent event(wkEVT_IPL_FERTIG);
		event.SetInt(2);
		wxPostEvent(this, event);
	}
}


void Wkl::startParser()
{
	std::string intf = vars[CWklPropGrid::WKL_PROPID_STR_INTERFACE] + vars[CWklPropGrid::WKL_PROPID_STR_INTFNR];
	if (dieListe->erstelleIPL(vars[CWklPropGrid::WKL_PROPID_STR_SDIR], vars[CWklPropGrid::WKL_PROPID_STR_ODIR],
		vars[CWklPropGrid::WKL_PROPID_STR_PATTERN], intf))
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4603: Parser finished\r\n", "4603", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4401: Parser stopped with errors\r\n", "4401", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}

	wxCommandEvent event(wkEVT_IPL_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);

}


void Wkl::startScanner()
{
	if (dieListe->erstelleIPPL(vars[CWklPropGrid::WKL_PROPID_STR_IPRANGE], vars[CWklPropGrid::WKL_PROPID_STR_ODIR], "", ""))
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4603: Scanner finished\r\n", "4603", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4401: Scanner stopped with errors\r\n", "4401", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}

	wxCommandEvent event(wkEVT_IPL_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);
}



// bool doLoad:
// zum Laden eines Profils. Die Einstellungen werden überprüft, bevor sie geladen werden
// return Werte: true: erfolgreich geladen
//               false: Fehler beim Laden der Werte
bool Wkl::doLoad(std::string profilname)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "4604: IP List: Loading Profile\r\n", "4604", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (xmlInit(profilname))
	{
		einstellungenInit();
		schreibeLog(WkLog::WkLog_ZEIT, "4605: IP List: Profile \"" + profilname + "\" successfully loaded\r\n", "4605", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		return true;
	}
	schreibeLog(WkLog::WkLog_ZEIT, "4310: IP List: Unable to load Profile \"" + profilname + "\"\r\n", "4310", 
		WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	return false;
}


// bool doSave:
// zum Sichern der Einstellungen in ein Profil
// return Werte: false: erfolgreich gesichert
//               true: Fehler beim Sichern
bool Wkl::doSave(std::string profilname)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "4606: IP List: Saving Profile\r\n", "4606", 
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
		schreibeLog(WkLog::WkLog_ZEIT, "4606: IP List: Profile \"" + profilname + "\" successfully saved\r\n", "4606", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4302: IP List: Unable to save Profile \"" + profilname + "\"\r\n", "4302", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// bool doRemove:
// zum Löschen eines Profils
// return Werte: false: erfolgreich gelöscht
//               true: Fehler beim Löschen
bool Wkl::doRemove(std::string profilName)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "4614: IP List: Removing Profile\r\n", "4614", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (profilName == "" || profilName == "DEFAULT")
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4304: IP List: Unable to remove Profile \"" + profilName + "\" - Default Profile cannot be deleted\r\n", "4304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
		return fehler;
	}

	fehler = xmlInit(profilName, 2);

	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4613: IP List: Profile \"" + profilName + "\" successfully removed\r\n", "4613", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "4304: IP List: Unable to remove Profile \"" + profilName + "\"\r\n", "4304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Wkl::sets(TiXmlDocument *document, bool guiStats, WkLogFile *lf)
{
	doc = document;

	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;

	logfile = lf;
}


// bool einstellungenInit()
// Zum Laden der Werte, nachdem die richtige Position des Profils in wktools.xml ausgemacht wurde.
bool Wkl::einstellungenInit()
{
	TiXmlElement *stringSets = pElemProf->FirstChildElement("Settings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"sVz", "aVz", "logfile", "intfNr", "search", "Intf", "IPRange"};
	std::string attInt[] = {"Tool"};

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
				wklProps->einstellungenInit(i, vars[i]);
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
				wklProps->einstellungenInit(i, vari[i]);
			}
		}
	}

	if (!guiStatus)
	{
		// GUI Mode
		wklProps->enableDisable();
	}

	if (logfile != NULL)
	{
		logfile->logFileSets(vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE]);
	}

	return true;
}


void Wkl::guiSets(CWklPropGrid *wklProp, WkLog *ausgabe)
{
	wklProps = wklProp;
	logAusgabe = ausgabe;
}


wxArrayString Wkl::ladeProfile()
{
	wxArrayString profile;

	// Grund Elemente für Wkl
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWkl;		// das Element, das auf Wkl zeigt

	// Wkl im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		profile.Empty();
	}
	else
	{
		pElemWkl = pElem->FirstChildElement("wkl");
		pElemProf = pElemWkl->FirstChildElement("Profile");

		for (profilIndex = 0; pElemProf; profilIndex++)
		{
			std::string profname = pElemProf->Attribute("name");
			profile.Add(profname.c_str());

			pElemProf = pElemProf->NextSiblingElement();
		}
	}

	return profile;
}


void Wkl::cancelIt()
{
// TODO:	derErsetzer->cancel = true;

	wxCommandEvent event(wkEVT_IPL_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


void Wkl::kilIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "4607: IP List killed!\r\n", "4607", 
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);

	ende = true;
	if (mitThread)
	{
		thrd.interrupt();
	}

	wxCommandEvent event(wkEVT_IPL_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


// xmlInit
//********
// Speichern: 0 - laden; 1 - speichern; 2 - löschen
bool Wkl::xmlInit(std::string profilName, int speichern)
{
	if (profilName == "")
	{
		profilName = "DEFAULT";
	}

	// Grund Elemente für Wkl
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWkl;		// das Element, das auf Wkl zeigt

	// Wkl im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		return false;
	}

	pElemWkl = pElem->FirstChildElement("wkl");
	pElemProf = pElemWkl->FirstChildElement("Profile");

	for (profilIndex = 0; pElemProf; profilIndex++)
	{
		std::string profname = pElemProf->Attribute("name");
		if (profname == profilName)
		{
			if (speichern == 2)
			{
				bool rmret = pElemWkl->RemoveChild(pElemProf);
				return !rmret;
			}
			return true;
		}

		pElemProf = pElemProf->NextSiblingElement();
	}

	if (speichern)
	{
		pElemProf = new TiXmlElement("Profile");		
		pElemWkl->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", profilName);

		TiXmlElement *stringSets = new TiXmlElement("Settings");
		TiXmlElement *intSets = new TiXmlElement("IntSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);

		std::string attString[] = {"sVz", "aVz", "logfile", "intfNr", "search", "Intf", "IPRange"};
		std::string attInt[] = {"Tool"};

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


bool Wkl::doTest(std::string profilname)
{
	// Test, ob alle Eingaben gültig sind
	bool fehler = false;

	// Test 1: Search Directory:
	std::string fehlerSDir = "4303: Wrong or missing entry for SEARCH DIRECTORY!";

	wxFileName fn;
	if (!fn.DirExists(vars[CWklPropGrid::WKL_PROPID_STR_SDIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSDir, "4303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 2: Ausgabe Directory:
	std::string fehlerODir = "4303: Wrong or missing entry for OUTPUT DIRECTORY!";

	wxFileName fn2;
	if (!fn2.DirExists(vars[CWklPropGrid::WKL_PROPID_STR_ODIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODir, "4303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 3: Ausgabedatei:	
	std::string fehlerODat = "4303: Wrong or missing entry for INTERFACE!";
	if (vars[CWklPropGrid::WKL_PROPID_STR_INTERFACE] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODat, "4303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	
	// Test 4: Logfile:	
	if (vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE] == "")
	{
		vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE] = vars[CWklPropGrid::WKL_PROPID_STR_ODIR] + "log.txt";
	}

	// Test 5: InterfaceNr:	
	std::string fehlerIDat = "4303: Wrong or missing entry for INTERFACE NUMBER!";
	if (vars[CWklPropGrid::WKL_PROPID_STR_INTFNR] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerIDat, "4303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	return fehler;
}


bool Wkl::doTestGui(std::string profilname)
{
	if  (!(xmlInit(profilname, true)))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n4101: Init Error -> could not read settings\r\n", "4101", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	TiXmlElement *stringSets = pElemProf->FirstChildElement("Settings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	if (!stringSets)
	{
		stringSets = new TiXmlElement("Settings");
		pElemProf->LinkEndChild(stringSets);
	}
	
	if (!intSets)
	{
		intSets = new TiXmlElement("IntSettings");
		pElemProf->LinkEndChild(intSets);
	}


	std::string attString[] = {"sVz", "aVz", "logfile", "intfNr", "search", "Intf", "IPRange"};
	std::string attInt[] = {"Tool"};

	bool fehler = false;

	vari[CWklPropGrid::WKL_PROPID_INT_TOOL] = wklProps->leseIntSetting(CWklPropGrid::WKL_PROPID_INT_TOOL);
	intSets->SetAttribute(attInt[CWklPropGrid::WKL_PROPID_INT_TOOL], vari[CWklPropGrid::WKL_PROPID_INT_TOOL]);

	if (vari[CWklPropGrid::WKL_PROPID_INT_TOOL] == 0)
	{
		// Test 1: Search Directory:
		std::string fehlerSDir = "4303: Wrong or missing entry for SEARCH DIRECTORY!";


		std::string svz = wklProps->leseStringSetting(CWklPropGrid::WKL_PROPID_STR_SDIR);
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

		vars[CWklPropGrid::WKL_PROPID_STR_SDIR] = svz;

		std::ifstream ipaFile(vars[CWklPropGrid::WKL_PROPID_STR_SDIR].c_str());
		wxFileName fn;

		if (!fn.DirExists(vars[CWklPropGrid::WKL_PROPID_STR_SDIR]))
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSDir, "4303", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_SDIR], vars[CWklPropGrid::WKL_PROPID_STR_SDIR]);
		}
	}


	// Test 2: Ausgabe Directory:
	std::string fehlerODir = "4303: Wrong or missing entry for OUTPUT DIRECTORY!";

	std::string avz = wklProps->leseStringSetting(CWklPropGrid::WKL_PROPID_STR_ODIR);
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
		vars[CWklPropGrid::WKL_PROPID_STR_ODIR] = avz;

	}

	wxFileName fn2;
	if (!fn2.DirExists(vars[CWklPropGrid::WKL_PROPID_STR_ODIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODir, "4303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_ODIR], vars[CWklPropGrid::WKL_PROPID_STR_ODIR]);
	}

	if (vari[CWklPropGrid::WKL_PROPID_INT_TOOL] == 0)
	{
		// Test 3: Interface:	
		std::string fehlerODat = "4303: Wrong or missing entry for INTERFACE!";

		vars[CWklPropGrid::WKL_PROPID_STR_INTERFACE] = wklProps->leseStringSetting(CWklPropGrid::WKL_PROPID_STR_INTERFACE);
		if (vars[CWklPropGrid::WKL_PROPID_STR_INTERFACE] == "")
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODat, "4303", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_INTERFACE], vars[CWklPropGrid::WKL_PROPID_STR_INTERFACE]);
		}
	}

	// Test 4: Logfile:	
	vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE] = wklProps->leseStringSetting(CWklPropGrid::WKL_PROPID_STR_LOGFILE);
	if (vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE] == "")
	{
		vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE] = vars[CWklPropGrid::WKL_PROPID_STR_ODIR] + "log.txt";
		stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_LOGFILE], vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE]);
		wklProps->einstellungenInit(CWklPropGrid::WKL_PROPID_STR_LOGFILE, vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE]);
	}
	else
	{
		stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_LOGFILE], vars[CWklPropGrid::WKL_PROPID_STR_LOGFILE]);
	}

	if (vari[CWklPropGrid::WKL_PROPID_INT_TOOL] == 0)
	{
		// Test 5: Interface number:	
		std::string fehlerIDat = "4303: Wrong or missing entry for INTERFACE NUMBER!";

		vars[CWklPropGrid::WKL_PROPID_STR_INTFNR] = wklProps->leseStringSetting(CWklPropGrid::WKL_PROPID_STR_INTFNR);
		if (vars[CWklPropGrid::WKL_PROPID_STR_INTFNR] == "")
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerIDat, "4303", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_INTFNR], vars[CWklPropGrid::WKL_PROPID_STR_INTFNR]);
		}
	}

	if (vari[CWklPropGrid::WKL_PROPID_INT_TOOL] == 1)
	{
		// Test 6: IP Range:	
		std::string fehlerODat = "4303: Wrong or missing entry for IP RANGE!";

		vars[CWklPropGrid::WKL_PROPID_STR_IPRANGE] = wklProps->leseStringSetting(CWklPropGrid::WKL_PROPID_STR_IPRANGE);
		if (vars[CWklPropGrid::WKL_PROPID_STR_IPRANGE].size() < 7)
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODat, "4303", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			fehler = true;
		}
		else
		{
			stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_IPRANGE], vars[CWklPropGrid::WKL_PROPID_STR_IPRANGE]);
		}
	}

	vars[CWklPropGrid::WKL_PROPID_STR_PATTERN] = wklProps->leseStringSetting(CWklPropGrid::WKL_PROPID_STR_PATTERN);
	stringSets->SetAttribute(attString[CWklPropGrid::WKL_PROPID_STR_PATTERN], vars[CWklPropGrid::WKL_PROPID_STR_PATTERN]);
	
	
	return fehler;
}


void Wkl::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
#ifdef _WINDOWS_
	//Sleep(20);
#else
	usleep(30000);
#endif				

	WkLog::evtData evtDat;
	evtDat.farbe = farbe;
	evtDat.format = format;
	evtDat.groesse = groesse;
	evtDat.logEintrag = logEintrag;
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


void Wkl::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


// OnWklFertig
// Wird durch Nachricht aufgerufen, wenn der Hauptthread (damit das GUI nicht blockiert) fertig ist
void Wkl::OnWklFertig(wxCommandEvent &event)
{
	schreibeLog(WkLog::WkLog_ZEIT, "4608: IP List END\r\n", "4608", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	if (wkMain != NULL)
	{
		wxCommandEvent event(wkEVT_WKL_FERTIG);
		event.SetInt(1);
		wxPostEvent(wkMain, event);
	}
	else
	{
		// Programm beenden, wenn kein GUI vorhanden
		wxGetApp().ExitMainLoop();
	}
}