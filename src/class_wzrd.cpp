/***************************************************************
* Name:      class_wzrd.h
* Purpose:   Defines IP List I/O Class
* Created:   2011-01-07
**************************************************************/
// Änderungen
///////////////////////////////////////////////////////////////
//

#include "class_wzrd.h"
#include "wktools4.h"

DEFINE_LOCAL_EVENT_TYPE(wkEVT_WIZARD_FERTIG)

BEGIN_EVENT_TABLE(Wzrd, wxEvtHandler)
EVT_COMMAND(wxID_ANY, wkEVT_WIZARD_FERTIG, Wzrd::OnWzrdFertig)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Wzrd::Wzrd() : wxEvtHandler()
{
	vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] = "";
	vars[CWzrdPropGrid::WZRD_PROPID_STR_ENPW] = "";
	vars[CWzrdPropGrid::WZRD_PROPID_STR_LOPW] = "";
	vars[CWzrdPropGrid::WZRD_PROPID_STR_USER] = "";
	vars[CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE] = "";
	vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF] = "Prof_1";


	profilIndex = 0;
	logAusgabe = NULL;
	wkMain = NULL;

	stopThread = false;
	ende = false;

	asgbe = "";

	a1 = 6;
	a2 = 1;
	wzrdPhase = 0;
}


// Destruktor:
Wzrd::~Wzrd()
{
}


// void DoIt:
// Tool Start; Es werden die Einstellungen überprüft und an den Parser übergeben
void Wzrd::doIt()
{
	schreibeLog(WkLog::WkLog_ABSATZ, "7601: Wizard START\r\n", "7601", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	if (wzrdPhase == 0)
	{
		bool fehler = doSave("");
		if (!fehler)
		{
			thrd = boost::thread(boost::bind(&Wzrd::startWzrd,this));
		}
		else
		{
			schreibeLog(WkLog::WkLog_ZEIT, "7402: Cannot Start because of wrong or missing entries!\r\n", "7402", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);

			wxCommandEvent event(wkEVT_WIZARD_FERTIG);
			event.SetInt(2);
			wxPostEvent(this, event);
		}
	}
	else if (wzrdPhase == 1)
	{
		thrd = boost::thread(boost::bind(&Wzrd::startWzrd2,this));
	}
	else
	{
		wzrdPhase = 0;
	}

}


void Wzrd::startWzrd()
{
	// Einmal Start gedrückt
	wzrdPhase = 1;
	doSave(vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF]);
	initScanner();
	wkMain->iplist->doIt(false);

	wxFileName fn;
	fn.Mkdir(vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "output", 0777, wxPATH_MKDIR_FULL);

	initWke();
	initCfg();
	if (vari[CWzrdPropGrid::WZRD_PROPID_INT_WHAT] == 0)
	{
		// Mapper
		wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION, 3);
		wkMain->einspielen->dgInit();
	}
	else if (vari[CWzrdPropGrid::WZRD_PROPID_INT_WHAT] == 1)
	{
		// Config sichern
		wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_CONFIGFILEOPTION, 0);
		wkMain->einspielen->dgInit();
	}
	
	wxMessageBox("Please check the DeviceGroup view if everything is correct and press \"Start\" again.");

	wxCommandEvent event(wkEVT_WIZARD_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);

}


void Wzrd::startWzrd2()
{
	// Zweimal Start gedrückt
	wzrdPhase = 2;

	if (vari[CWzrdPropGrid::WZRD_PROPID_INT_WHAT] == 0)
	{
#ifdef WKTOOLS_MAPPER
		// Mapper
		wkMain->einspielen->doIt(false);
		initMapper();
		wkMain->mapper->doIt(false);
#endif
	}
	else if (vari[CWzrdPropGrid::WZRD_PROPID_INT_WHAT] == 1)
	{
		// Config sichern
		wkMain->einspielen->doIt(false);
	}

	wzrdPhase = 0;

	wxCommandEvent event(wkEVT_WIZARD_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);

}


// bool doLoad:
// zum Laden eines Profils. Die Einstellungen werden überprüft, bevor sie geladen werden
// return Werte: true: erfolgreich geladen
//               false: Fehler beim Laden der Werte
bool Wzrd::doLoad(std::string profilname)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "7604: Wizard: Loading Profile\r\n", "7604", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (xmlInit(profilname))
	{
		einstellungenInit();
		schreibeLog(WkLog::WkLog_ZEIT, "7605: Wizard: Profile \"" + profilname + "\" successfully loaded\r\n", "7605", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		return true;
	}
	schreibeLog(WkLog::WkLog_ZEIT, "7301: Wizard: Unable to load Profile \"" + profilname + "\"\r\n", "7301", 
		WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	return false;
}


// bool doSave:
// zum Sichern der Einstellungen in ein Profil
// return Werte: false: erfolgreich gesichert
//               true: Fehler beim Sichern
bool Wzrd::doSave(std::string profilname)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "7606: Wizard: Saving Profile\r\n", "7606", 
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
		schreibeLog(WkLog::WkLog_ZEIT, "7606: Wizard: Profile \"" + profilname + "\" successfully saved\r\n", "7606", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "7302: Wizard: Unable to save Profile \"" + profilname + "\"\r\n", "7302", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// bool doRemove:
// zum Löschen eines Profils
// return Werte: false: erfolgreich gelöscht
//               true: Fehler beim Löschen
bool Wzrd::doRemove(std::string profilName)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "7614: Wizard: Removing Profile\r\n", "7614", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (profilName == "" || profilName == "DEFAULT")
	{
		schreibeLog(WkLog::WkLog_ZEIT, "7304: Wizard: Unable to remove Profile \"" + profilName + "\" - Default Profile cannot be deleted\r\n", "7304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
		return fehler;
	}

	fehler = xmlInit(profilName, 2);

	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "7613: Wizard: Profile \"" + profilName + "\" successfully removed\r\n", "7613", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "7304: Wizard: Unable to remove Profile \"" + profilName + "\"\r\n", "7304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Wzrd::sets(TiXmlDocument *document, bool guiStats, WkLogFile *lf)
{
	doc = document;

	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;

	logfile = lf;
}


// bool einstellungenInit()
// Zum Laden der Werte, nachdem die richtige Position des Profils in wktools.xml ausgemacht wurde.
bool Wzrd::einstellungenInit()
{
	TiXmlElement *stringSets = pElemProf->FirstChildElement("Settings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"wDir", "User", "enpw", "lopw",  "IPRange", "Profile"};
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
				wzrdProps->einstellungenInit(i, vars[i]);
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
				wzrdProps->einstellungenInit(i, vari[i]);
			}
		}
	}

	//if (!guiStatus)
	//{
	//	// GUI Mode
	//	wzrdProps->enableDisable();
	//}

	return true;
}


void Wzrd::guiSets(CWzrdPropGrid *wzrdProp, WkLog *ausgabe)
{
	wzrdProps = wzrdProp;
	logAusgabe = ausgabe;
}


wxArrayString Wzrd::ladeProfile()
{
	wxArrayString profile;

	// Grund Elemente für Wzrd
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWzrd;		// das Element, das auf Wzrd zeigt

	// Wzrd im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		profile.Empty();
	}
	else
	{
		pElemWzrd = pElem->FirstChildElement("wzrd");
		pElemProf = pElemWzrd->FirstChildElement("Profile");

		for (profilIndex = 0; pElemProf; profilIndex++)
		{
			std::string profname = pElemProf->Attribute("name");
			profile.Add(profname.c_str());

			pElemProf = pElemProf->NextSiblingElement();
		}
	}

	return profile;
}


void Wzrd::cancelIt()
{
	// TODO:	derErsetzer->cancel = true;

	wxCommandEvent event(wkEVT_WIZARD_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


void Wzrd::kilIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "7607: Wizard killed!\r\n", "7607", 
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);

	ende = true;
	thrd.interrupt();

	wxCommandEvent event(wkEVT_WIZARD_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


// xmlInit
//********
// Speichern: 0 - laden; 1 - speichern; 2 - löschen
bool Wzrd::xmlInit(std::string profilName, int speichern)
{
	if (profilName == "")
	{
		profilName = "DEFAULT";
	}

	// Grund Elemente für Wzrd
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWzrd;		// das Element, das auf Wzrd zeigt

	// Wzrd im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		return false;
	}

	pElemWzrd = pElem->FirstChildElement("wzrd");
	if (!pElemWzrd)
	{
		pElemWzrd = new TiXmlElement("wzrd");
		pElem->LinkEndChild(pElemWzrd);
	}

	pElemProf = pElemWzrd->FirstChildElement("Profile");
	if (!pElemProf)
	{
		pElemProf = new TiXmlElement("Profile");
		pElemWzrd->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", "DEFAULT");

		TiXmlElement *stringSets = new TiXmlElement("Settings");
		TiXmlElement *intSets = new TiXmlElement("IntSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);
		std::string attString[] = {"wDir", "User", "enpw", "lopw",  "IPRange", "Profile"};
		std::string attInt[] = {"Tool"};

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
				bool rmret = pElemWzrd->RemoveChild(pElemProf);
				return !rmret;
			}
			return true;
		}

		pElemProf = pElemProf->NextSiblingElement();
	}

	if (speichern)
	{
		pElemProf = new TiXmlElement("Profile");		
		pElemWzrd->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", profilName);

		TiXmlElement *stringSets = new TiXmlElement("Settings");
		TiXmlElement *intSets = new TiXmlElement("IntSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);

		std::string attString[] = {"wDir", "User", "enpw", "lopw",  "IPRange", "Profile"};
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


bool Wzrd::doTest(std::string profilname)
{
	// Test, ob alle Eingaben gültig sind
	bool fehler = false;

	// Test 1: Search Directory:
	std::string fehlerWorkDir = "7303: Wrong or missing entry for SEARCH DIRECTORY!\n";

	wxFileName fn;
	if (!fn.DirExists(vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWorkDir, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 2: IP RANGE:
	std::string fehlerStr = "7303: Wrong or missing entry for IP RANGE!\n";

	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 3: USER:
	fehlerStr = "7303: Wrong or missing entry for Username\n";

	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_USER] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	// Test 4: Login PW:
	fehlerStr = "7303: Wrong or missing entry for Login Password!\n";

	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_LOPW] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	// Test 5: Enable PW
	fehlerStr = "7303: Wrong or missing entry for Enable Password!\n";

	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_ENPW] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	// Test 6: Profile
	fehlerStr = "7303: Wrong or missing entry for Profile!\n";

	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	return fehler;
}


bool Wzrd::doTestGui(std::string profilname)
{
	if  (!(xmlInit(profilname, true)))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n7101: Init Error -> could not read settings\r\n", "7101", 
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


	std::string attString[] = {"wDir", "User", "enpw", "lopw",  "IPRange", "Profile"};
	std::string attInt[] = {"Tool"};

	bool fehler = false;

	vari[CWzrdPropGrid::WZRD_PROPID_INT_WHAT] = wzrdProps->leseIntSetting(CWzrdPropGrid::WZRD_PROPID_INT_WHAT);
	intSets->SetAttribute(attInt[CWzrdPropGrid::WZRD_PROPID_INT_WHAT], vari[CWzrdPropGrid::WZRD_PROPID_INT_WHAT]);

	// Test 1: Work Directory:
	std::string fehlerWorkDir = "7303: Wrong or missing entry for WORK DIRECTORY!\n";

	std::string wvz = wzrdProps->leseStringSetting(CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR);
	if (wvz != "")
	{
#ifdef _WINDOWS_
		if (wvz[wvz.length()-1] != '\\') 
		{
			wvz.append(std::string("\\")); // append '\'
		}
#else
		if (wvz[wvz.length()-1] != '/') 
		{
			wvz.append(std::string("/")); // append '/'
		}
#endif
	}
	vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] = wvz;
	std::ifstream ipaFile(vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR].c_str());

	wxFileName fn;
	if (!fn.DirExists(vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWorkDir, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR], vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR]);
	}

	// Test 2: IP RANGE:
	std::string fehlerStr = "7303: Wrong or missing entry for IP RANGE!\n";
	
	vars[CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE] = wzrdProps->leseStringSetting(CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE);
	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE], vars[CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE]);
	}

	// Test 3: USER:
	fehlerStr = "7303: Wrong or missing entry for Username!\n";

	vars[CWzrdPropGrid::WZRD_PROPID_STR_USER] = wzrdProps->leseStringSetting(CWzrdPropGrid::WZRD_PROPID_STR_USER);
	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_USER] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWzrdPropGrid::WZRD_PROPID_STR_USER], vars[CWzrdPropGrid::WZRD_PROPID_STR_USER]);
	}

	// Test 4: Login PW:
	fehlerStr = "7303: Wrong or missing entry for Login Password!\n";

	vars[CWzrdPropGrid::WZRD_PROPID_STR_LOPW] = wzrdProps->leseStringSetting(CWzrdPropGrid::WZRD_PROPID_STR_LOPW);
	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_LOPW] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWzrdPropGrid::WZRD_PROPID_STR_LOPW], vars[CWzrdPropGrid::WZRD_PROPID_STR_LOPW]);
	}

	// Test 5: Enable PW
	fehlerStr = "7303: Wrong or missing entry for Enable Password!\n";

	vars[CWzrdPropGrid::WZRD_PROPID_STR_ENPW] = wzrdProps->leseStringSetting(CWzrdPropGrid::WZRD_PROPID_STR_ENPW);
	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_ENPW] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWzrdPropGrid::WZRD_PROPID_STR_ENPW], vars[CWzrdPropGrid::WZRD_PROPID_STR_ENPW]);
	}

	// Test 6: Profile
	fehlerStr = "7303: Wrong or missing entry for Profile!\n";

	vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF] = wzrdProps->leseStringSetting(CWzrdPropGrid::WZRD_PROPID_STR_PROF);
	if (vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerStr, "7303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CWzrdPropGrid::WZRD_PROPID_STR_PROF], vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF]);
	}

	return fehler;
}


void Wzrd::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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


void Wzrd::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


// OnWzrdFertig
// Wird durch Nachricht aufgerufen, wenn der Hauptthread (damit das GUI nicht blockiert) fertig ist
void Wzrd::OnWzrdFertig(wxCommandEvent &event)
{
	if (wzrdPhase == 1)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "7610: Please check the DeviceGroup view if everything is correct and press \"Start\" again.\r\n", "7610", 
			WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
		if (wkMain != NULL)
		{
			wxCommandEvent event(wkEVT_WZRD_FERTIG);
			event.SetInt(1);
			wxPostEvent(wkMain, event);
		}
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "7608: Wizard END\r\n", "7608", 
			WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
		schreibeLog(WkLog::WkLog_ZEIT, "7609: The result is stored at " + vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + asgbe + "\r\n", "7609", 
			WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
		if (wkMain != NULL)
		{
			wxCommandEvent event(wkEVT_WZRD_FERTIG);
			event.SetInt(1);
			wxPostEvent(wkMain, event);
		}
		else
		{
			// Programm beenden, wenn kein GUI vorhanden
			wxGetApp().ExitMainLoop();
		}
	}

}


// initScanner
// Zum Initialisieren der Portscanner Einstellungen
void Wzrd::initScanner()
{
	wkMain->wklSettings->einstellungenInit(CWklPropGrid::WKL_PROPID_INT_TOOL, 1);
	wkMain->wklSettings->einstellungenInit(CWklPropGrid::WKL_PROPID_STR_IPRANGE, vars[CWzrdPropGrid::WZRD_PROPID_STR_IPRANGE]);
	wkMain->wklSettings->einstellungenInit(CWklPropGrid::WKL_PROPID_STR_ODIR, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR]);
	wkMain->iplist->doSave(vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF]);
}


// initMapper
// Zum Initialisieren der Mapper Einstellungen
#ifdef WKTOOLS_MAPPER
void Wzrd::initMapper()
{
#ifdef _WINDOWS_
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_STR_SDIR, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "output\\");
#else
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_STR_SDIR, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "output/");
#endif

	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_STR_PATTERN, "");
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_STR_OFILE, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF] + ".vdx");
	asgbe = vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF] + ".vdx";
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_STR_LOGFILE, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "log.txt");
	
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_L2, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_L3, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_ORGANIC, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_HIERARCHIC, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_PARSE, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_CDP, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_CDPHOSTS, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_UNKNOWNSTP, 1);
	wkMain->wkmSettings->einstellungenInit(CWkmPropGrid::WKM_PROPID_INT_COMBINE, 1);

	wkMain->mapper->doSave(vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF]);
}
#endif


// initWke
// Zum Initialisieren der WKE Einstellungen
void Wzrd::initWke()
{
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_DEVGRP, 0);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_DEVGRPFILE, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "devgrp.txt");
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_HOSTLFILE, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "ipPortList.txt");
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_CONFIGFILE, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "config.txt");
	
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_USER, vars[CWzrdPropGrid::WZRD_PROPID_STR_USER]);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_ENPW, vars[CWzrdPropGrid::WZRD_PROPID_STR_ENPW]);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_LOPW, vars[CWzrdPropGrid::WZRD_PROPID_STR_LOPW]);

	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_MODUS, 2);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_TYPE, 0);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_SHOW, 0);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_SHOWAPPEND, 0);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_LOGFILE, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "log.txt");

#ifdef _WINDOWS_
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_ODIR, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "output\\");
#else
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_ODIR, vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "output/");
#endif

	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_STR_EXEC, "enable");
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_RAUTE, 1);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_LOGSY, 0);
	wkMain->wkeSettings->einstellungenInit(CWkePropGrid::WKE_PROPID_INT_LOGINMODE, 0);

	wkMain->einspielen->doSave(vars[CWzrdPropGrid::WZRD_PROPID_STR_PROF]);
}


// initCfg:
//=========
// Zum Initialisieren der notwendigen Einstellungen zum Config absaugen
void Wzrd::initCfg()
{
	std::string ausgabeDatei = vars[CWzrdPropGrid::WZRD_PROPID_STR_WORKDIR] + "config.txt";
	std::ofstream ausgabe(ausgabeDatei.c_str(), std::ios_base::out| std::ios_base::binary);
	ausgabe << "show run\n";
	ausgabe << "exit\n";
	ausgabe.close();

}