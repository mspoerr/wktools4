/***************************************************************
* Name:      class_dip.cpp
* Purpose:   Code for Device Information Parser I/O Class
* Created:   2008-08-22
**************************************************************/
// Änderungen
///////////////////////////////////////////////////////////////
//


#include "class_dip.h"
#include "wktools4.h"

#include <boost/tokenizer.hpp>

DEFINE_LOCAL_EVENT_TYPE(wkEVT_PARSER_FERTIG)

BEGIN_EVENT_TABLE(Dip, wxEvtHandler)
EVT_COMMAND(wxID_ANY, wkEVT_PARSER_FERTIG, Dip::OnDipFertig)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Dip::Dip() : wxEvtHandler()
{
	vari[CDipPropGrid::DIP_PROPID_INT_SNMPLOCCOL] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_TOOL] = 0;
	vari[CDipPropGrid::DIP_PROPID_INT_APPEND] = 0;
	vari[CDipPropGrid::DIP_PROPID_INT_COL1] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL2] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL3] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL4] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL5] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL6] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL7] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL8] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL9] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL10] = 1;
	vari[CDipPropGrid::DIP_PROPID_INT_COL11] = 1;

	vari[CDipPropGrid::DIP_PROPID_INT_DESCRIPTION] = 0;
	vari[CDipPropGrid::DIP_PROPID_INT_CHASSISONLY] = 0;

	vars[CDipPropGrid::DIP_PROPID_STR_SDIR] = "";
	vars[CDipPropGrid::DIP_PROPID_STR_ODIR] = "";
	vars[CDipPropGrid::DIP_PROPID_STR_OFILE] = "output.csv";
	vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE] = "log.txt";
	vars[CDipPropGrid::DIP_PROPID_STR_PATTERN] = "";


	profilIndex = 0;
	logAusgabe = NULL;
	wkMain = NULL;

	stopThread = false;
	ende = false;
	mitThread = true;

	a1 = 5;
	a2 = 16;

}


// Destruktor:
Dip::~Dip()
{
}


// void DoIt:
// Tool Start; Es werden die Einstellungen überprüft und an den Parser übergeben
void Dip::doIt(bool wt)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "3601: Device Information Parser START\r\n", "3601", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	bool fehler = doSave("");
	if (!fehler)
	{
		if (vari[CDipPropGrid::DIP_PROPID_INT_TOOL] == 0)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "3602: Starting Inventory Parser\r\n", "3602", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			derParser = new	UTAParse(vars[CDipPropGrid::DIP_PROPID_STR_SDIR], vars[CDipPropGrid::DIP_PROPID_STR_ODIR], vars[CDipPropGrid::DIP_PROPID_STR_OFILE], 
				vari[CDipPropGrid::DIP_PROPID_INT_SNMPLOCCOL], vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE], logAusgabe, this);

			if (wt)
			{
				thrd = boost::thread(boost::bind(&Dip::startParser,this));
			}
			else
			{
				mitThread = false;
				startParser();
			}
		}
		else if (vari[CDipPropGrid::DIP_PROPID_INT_TOOL] == 1)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "3602: Starting Interface Information Parser\r\n", "3602", 
				WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

			derIntfParser = new IntfParse(vars[CDipPropGrid::DIP_PROPID_STR_SDIR], vars[CDipPropGrid::DIP_PROPID_STR_ODIR], vars[CDipPropGrid::DIP_PROPID_STR_OFILE], 
				vari[CDipPropGrid::DIP_PROPID_INT_SNMPLOCCOL], vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE], logAusgabe, this);

			if (wt)
			{
				thrd = boost::thread(boost::bind(&Dip::startIntfParser,this));
			}
			else
			{
				mitThread = false;
				startIntfParser();
			}
		}
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3402: Cannot Start because of wrong or missing entries!\r\n", "3402", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);

		wxCommandEvent event(wkEVT_PARSER_FERTIG);
		event.SetInt(2);
		wxPostEvent(this, event);
	}
}


void Dip::startParser()
{
	
	UTAParse::oo outOpt;
	outOpt.spalte1 = vari[CDipPropGrid::DIP_PROPID_INT_COL1];
	outOpt.spalte2 = vari[CDipPropGrid::DIP_PROPID_INT_COL2];
	outOpt.spalte3 = vari[CDipPropGrid::DIP_PROPID_INT_COL3];
	outOpt.spalte4 = vari[CDipPropGrid::DIP_PROPID_INT_COL4];
	outOpt.spalte5 = vari[CDipPropGrid::DIP_PROPID_INT_COL5];
	outOpt.spalte6 = vari[CDipPropGrid::DIP_PROPID_INT_COL6];
	outOpt.spalte7 = vari[CDipPropGrid::DIP_PROPID_INT_COL7];
	outOpt.spalte8 = vari[CDipPropGrid::DIP_PROPID_INT_COL8];
	outOpt.spalte9 = vari[CDipPropGrid::DIP_PROPID_INT_COL9];
	outOpt.spalte10 = vari[CDipPropGrid::DIP_PROPID_INT_COL10];
	outOpt.spalte11 = vari[CDipPropGrid::DIP_PROPID_INT_COL11];
	
	int res = derParser->startParser(vars[CDipPropGrid::DIP_PROPID_STR_PATTERN], vari[CDipPropGrid::DIP_PROPID_INT_APPEND], 
		outOpt,	vari[CDipPropGrid::DIP_PROPID_INT_CHASSISONLY]);
	
	if (res)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3401: Parser returned with errors\r\n", "3401", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3603: Parser finished\r\n", "3603", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	}

	wxCommandEvent event(wkEVT_PARSER_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);

}


void Dip::startIntfParser()
{

	int res = derIntfParser->startParser(vars[CDipPropGrid::DIP_PROPID_STR_PATTERN], vari[CDipPropGrid::DIP_PROPID_INT_APPEND], 
		vari[CDipPropGrid::DIP_PROPID_INT_DESCRIPTION]);

	if (res)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3401: Parser returned with errors\r\n", "3401", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3603: Parser finished\r\n", "3603", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	}

	wxCommandEvent event(wkEVT_PARSER_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);

}


// bool doLoad:
// zum Laden eines Profils. Die Einstellungen werden überprüft, bevor sie geladen werden
// return Werte: true: erfolgreich geladen
//               false: Fehler beim Laden der Werte
bool Dip::doLoad(std::string profilname)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "3604: Device Information Parser: Loading Profile\r\n", "3604", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (xmlInit(profilname))
	{
		einstellungenInit();
		schreibeLog(WkLog::WkLog_ZEIT, "3605: Device Information Parser: Profile \"" + profilname + "\" successfully loaded\r\n", "3605", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		return true;
	}
	schreibeLog(WkLog::WkLog_ZEIT, "3310: Device Information Parser: Unable to load Profile \"" + profilname + "\"\r\n", "3310", 
		WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	return false;
}


// bool doSave:
// zum Sichern der Einstellungen in ein Profil
// return Werte: false: erfolgreich gesichert
//               true: Fehler beim Sichern
bool Dip::doSave(std::string profilname)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "3606: Device Information Parser: Saving Profile\r\n", "3606", 
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
		schreibeLog(WkLog::WkLog_ZEIT, "3606: Device Information Parser: Profile \"" + profilname + "\" successfully saved\r\n", "3606", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3302: Device Information Parser: Unable to save Profile \"" + profilname + "\"\r\n", "3602", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// bool doRemove:
// zum Löschen eines Profils
// return Werte: false: erfolgreich gelöscht
//               true: Fehler beim Löschen
bool Dip::doRemove(std::string profilName)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "3614: Device Information Parser: Removing Profile\r\n", "3614", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (profilName == "" || profilName == "DEFAULT")
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3304: Device Information Parser: Unable to remove Profile \"" + profilName + "\" - Default Profile cannot be deleted\r\n", "3304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
		return fehler;
	}

	fehler = xmlInit(profilName, 2);

	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3613: Device Information Parser: Profile \"" + profilName + "\" successfully removed\r\n", "3613", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "3304: Device Information Parser: Unable to remove Profile \"" + profilName + "\"\r\n", "3304", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Dip::sets(TiXmlDocument *document, bool guiStats, WkLogFile *lf)
{
	doc = document;

	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;

	logfile = lf;
}


// bool einstellungenInit()
// Zum Laden der Werte, nachdem die richtige Position des Profils in wktools.xml ausgemacht wurde.
bool Dip::einstellungenInit()
{
	TiXmlElement *stringSets = pElemProf->FirstChildElement("StringSettings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"sD", "oD", "LogFile", "oFile", "Pattern"};
	std::string attInt[] = {"SNMPLocCol", "Tool", "Append", "Col1", "Col2",
		"Col3", "Col4", "Col5", "Col6", "Col7", "Col8", "Col9",
		"Col10", "Col11", "Description", "ChassisOnly"};

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
				dipProps->einstellungenInit(i, vars[i]);
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
				dipProps->einstellungenInit(i, vari[i]);
			}
		}
	}

	if (!guiStatus)
	{
		// GUI Mode
		dipProps->enableDisable();
	}

	if (logfile != NULL)
	{
		logfile->logFileSets(vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE]);
	}

	return true;
}


void Dip::guiSets(CDipPropGrid *dipProp, WkLog *ausgabe, TabellePanel *table)
{
	dipProps = dipProp;
	logAusgabe = ausgabe;
	ergebnis = table;
}


wxArrayString Dip::ladeProfile()
{
	wxArrayString profile;

	// Grund Elemente für Dip
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemDip;		// das Element, das auf Dip zeigt

	// Dip im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		profile.Empty();
	}
	else
	{
		pElemDip = pElem->FirstChildElement("wkdp");
		pElemProf = pElemDip->FirstChildElement("Profile");

		for (profilIndex = 0; pElemProf; profilIndex++)
		{
			std::string profname = pElemProf->Attribute("name");
			profile.Add(profname.c_str());

			pElemProf = pElemProf->NextSiblingElement();
		}
	}

	return profile;
}


void Dip::cancelIt()
{
	if (vari[CDipPropGrid::DIP_PROPID_INT_TOOL] == 0)
	{
		derParser->stop = true;
	}
	else if (vari[CDipPropGrid::DIP_PROPID_INT_TOOL] == 1)
	{
		derIntfParser->stop = true;
	}

	wxCommandEvent event(wkEVT_PARSER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


void Dip::kilIt()
{
	schreibeLog(WkLog::WkLog_ZEIT, "3607: Device Information Parser killed!\r\n", "3607", 
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);

	ende = true;
	if (mitThread)
	{
		thrd.interrupt();
	}

	wxCommandEvent event(wkEVT_PARSER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);
}


// xmlInit
//********
// Speichern: 0 - laden; 1 - speichern; 2 - löschen
bool Dip::xmlInit(std::string profilName, int speichern)
{
	if (profilName == "")
	{
		profilName = "DEFAULT";
	}

	// Grund Elemente für Dip
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemDip;		// das Element, das auf Dip zeigt

	// Dip im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		return false;
	}

	pElemDip = pElem->FirstChildElement("wkdp");
	pElemProf = pElemDip->FirstChildElement("Profile");

	for (profilIndex = 0; pElemProf; profilIndex++)
	{
		std::string profname = pElemProf->Attribute("name");
		if (profname == profilName)
		{
			if (speichern == 2)
			{
				bool rmret = pElemDip->RemoveChild(pElemProf);
				return !rmret;
			}
			return true;
		}

		pElemProf = pElemProf->NextSiblingElement();
	}

	if (speichern)
	{
		pElemProf = new TiXmlElement("Profile");		
		pElemDip->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", profilName);

		TiXmlElement *stringSets = new TiXmlElement("StringSettings");
		TiXmlElement *intSets = new TiXmlElement("IntSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);

		std::string attString[] = {"sD", "oD", "LogFile", "oFile", "Pattern"};
		std::string attInt[] = {"SNMPLocCol", "Tool", "Append", "Col1", "Col2",
			"Col3", "Col4", "Col5", "Col6", "Col7", "Col8", "Col9",
			"Col10", "Col11", "Description", "ChassisOnly"};

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


bool Dip::doTest(std::string profilname)
{
	// Test, ob alle Eingaben gültig sind
	bool fehler = false;

	// Test 1: Search Directory:
	std::string fehlerSDir = "3303: Wrong or missing entry for SEARCH DIRECTORY!";

	wxFileName fn;
	if (!fn.DirExists(vars[CDipPropGrid::DIP_PROPID_STR_SDIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSDir, "3303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 2: Ausgabe Directory:
	std::string fehlerODir = "3303: Wrong or missing entry for OUTPUT DIRECTORY!";

	wxFileName fn2;
	if (!fn2.DirExists(vars[CDipPropGrid::DIP_PROPID_STR_ODIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODir, "3303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 3: Ausgabedatei:	
	std::string fehlerODat = "3303: Wrong or missing entry for OUTPUT FILE!";
	if (vars[CDipPropGrid::DIP_PROPID_STR_OFILE] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODat, "3303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	
	// Test 4: Logfile:	
	if (vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE] == "")
	{
		vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE] = vars[CDipPropGrid::DIP_PROPID_STR_ODIR] + "log.txt";
	}

	return fehler;
}


bool Dip::doTestGui(std::string profilname)
{
	if  (!(xmlInit(profilname, true)))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n3101: Init Error -> could not read settings\r\n", "3101", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}

	TiXmlElement *stringSets = pElemProf->FirstChildElement("StringSettings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("IntSettings");

	std::string attString[] = {"sD", "oD", "LogFile", "oFile", "Pattern"};
	std::string attInt[] = {"SNMPLocCol", "Tool", "Append", "Col1", "Col2",
		"Col3", "Col4", "Col5", "Col6", "Col7", "Col8", "Col9",
		"Col10", "Col11", "Description", "ChassisOnly"};

	bool fehler = false;

	// Test 1: Search Directory:
	std::string fehlerSDir = "3303: Wrong or missing entry for SEARCH DIRECTORY!";


	std::string svz = dipProps->leseStringSetting(CDipPropGrid::DIP_PROPID_STR_SDIR);
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
	vars[CDipPropGrid::DIP_PROPID_STR_SDIR] = svz;

	std::ifstream ipaFile(vars[CDipPropGrid::DIP_PROPID_STR_SDIR].c_str());
	wxFileName fn;

	if (!fn.DirExists(vars[CDipPropGrid::DIP_PROPID_STR_SDIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSDir, "3303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CDipPropGrid::DIP_PROPID_STR_SDIR], vars[CDipPropGrid::DIP_PROPID_STR_SDIR]);
	}

	// Test 2: Ausgabe Directory:
	std::string fehlerODir = "3303: Wrong or missing entry for OUTPUT DIRECTORY!";

	std::string avz = dipProps->leseStringSetting(CDipPropGrid::DIP_PROPID_STR_ODIR);
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
	vars[CDipPropGrid::DIP_PROPID_STR_ODIR] = avz;

	wxFileName fn2;
	if (!fn2.DirExists(vars[CDipPropGrid::DIP_PROPID_STR_ODIR]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODir, "3303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CDipPropGrid::DIP_PROPID_STR_ODIR], vars[CDipPropGrid::DIP_PROPID_STR_ODIR]);
	}

	// Test 3: Ausgabedatei:	
	std::string fehlerODat = "3303: Wrong or missing entry for OUTPUT FILE!";

	vars[CDipPropGrid::DIP_PROPID_STR_OFILE] = dipProps->leseStringSetting(CDipPropGrid::DIP_PROPID_STR_OFILE);
	if (vars[CDipPropGrid::DIP_PROPID_STR_OFILE] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerODat, "3303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(attString[CDipPropGrid::DIP_PROPID_STR_OFILE], vars[CDipPropGrid::DIP_PROPID_STR_OFILE]);
	}

	// Test 4: Logfile:	
	if (vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE] == "")
	{
		vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE] = vars[CDipPropGrid::DIP_PROPID_STR_ODIR] + "log.txt";
		stringSets->SetAttribute(attString[CDipPropGrid::DIP_PROPID_STR_LOGFILE], vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE]);
		dipProps->einstellungenInit(CDipPropGrid::DIP_PROPID_STR_LOGFILE, vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE]);
	}
	else
	{
		stringSets->SetAttribute(attString[CDipPropGrid::DIP_PROPID_STR_LOGFILE], vars[CDipPropGrid::DIP_PROPID_STR_LOGFILE]);
	}

	vars[CDipPropGrid::DIP_PROPID_STR_PATTERN] = dipProps->leseStringSetting(CDipPropGrid::DIP_PROPID_STR_PATTERN);
	stringSets->SetAttribute(attString[CDipPropGrid::DIP_PROPID_STR_PATTERN], vars[CDipPropGrid::DIP_PROPID_STR_PATTERN]);




	vari[CDipPropGrid::DIP_PROPID_INT_APPEND] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_APPEND);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_APPEND], vari[CDipPropGrid::DIP_PROPID_INT_APPEND]);
	vari[CDipPropGrid::DIP_PROPID_INT_TOOL] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_TOOL);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_TOOL], vari[CDipPropGrid::DIP_PROPID_INT_TOOL]);
	vari[CDipPropGrid::DIP_PROPID_INT_SNMPLOCCOL] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_SNMPLOCCOL);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_SNMPLOCCOL], vari[CDipPropGrid::DIP_PROPID_INT_SNMPLOCCOL]);
	vari[CDipPropGrid::DIP_PROPID_INT_DESCRIPTION] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_DESCRIPTION);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_DESCRIPTION], vari[CDipPropGrid::DIP_PROPID_INT_DESCRIPTION]);
	vari[CDipPropGrid::DIP_PROPID_INT_CHASSISONLY] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_CHASSISONLY);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_CHASSISONLY], vari[CDipPropGrid::DIP_PROPID_INT_CHASSISONLY]);
	
	vari[CDipPropGrid::DIP_PROPID_INT_COL1] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL1);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL1], vari[CDipPropGrid::DIP_PROPID_INT_COL1]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL2] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL2);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL2], vari[CDipPropGrid::DIP_PROPID_INT_COL2]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL3] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL3);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL3], vari[CDipPropGrid::DIP_PROPID_INT_COL3]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL4] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL4);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL4], vari[CDipPropGrid::DIP_PROPID_INT_COL4]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL5] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL5);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL5], vari[CDipPropGrid::DIP_PROPID_INT_COL5]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL6] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL6);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL6], vari[CDipPropGrid::DIP_PROPID_INT_COL6]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL7] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL7);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL7], vari[CDipPropGrid::DIP_PROPID_INT_COL7]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL8] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL8);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL8], vari[CDipPropGrid::DIP_PROPID_INT_COL8]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL9] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL9);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL9], vari[CDipPropGrid::DIP_PROPID_INT_COL9]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL10] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL10);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL10], vari[CDipPropGrid::DIP_PROPID_INT_COL10]);
	vari[CDipPropGrid::DIP_PROPID_INT_COL11] = dipProps->leseIntSetting(CDipPropGrid::DIP_PROPID_INT_COL11);
	intSets->SetAttribute(attInt[CDipPropGrid::DIP_PROPID_INT_COL11], vari[CDipPropGrid::DIP_PROPID_INT_COL11]);
	
	return fehler;
}


void Dip::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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


void Dip::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


// OnDipFertig
// Wird durch Nachricht aufgerufen, wenn der Hauptthread (damit das GUI nicht blockiert) fertig ist
void Dip::OnDipFertig(wxCommandEvent &event)
{
	schreibeLog(WkLog::WkLog_ZEIT, "3608: Device Information Parser END\r\n", "3608", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	if (wkMain != NULL)
	{
		wxCommandEvent event(wkEVT_WKDP_FERTIG);
		event.SetInt(1);
		wxPostEvent(wkMain, event);
		ausgabe();
	}
	else
	{
		// Programm beenden, wenn kein GUI vorhanden
		wxGetApp().ExitMainLoop();
	}
}


void Dip::ausgabe()
{
	ergebnis->createList();

	std::string datfile = vars[CDipPropGrid::DIP_PROPID_STR_ODIR] + vars[CDipPropGrid::DIP_PROPID_STR_OFILE];
	
	std::ifstream eingabe(datfile.c_str(), std::ios_base::in);

	typedef boost::tokenizer<boost::char_separator<char> > 
		tokenizer;
	boost::char_separator<char> sep(";\r\n", "", boost::keep_empty_tokens);

	for (int i = 0; eingabe.good(); i++)
	{
		// Zeile hinzufügen, wenn zu wenig vorhanden
		if (ergebnis->tabellenAnsicht->GetNumberRows() == i)
		{
			ergebnis->zeileHinzu();
		}

		std::string dfile;
		getline(eingabe, dfile, '\n');

		tokenizer tokens(dfile, sep);
		int j = 0;		// Spaltenanzahl
		for (tokenizer::iterator tok_iter = tokens.begin();	tok_iter != tokens.end(); ++tok_iter)
		{
			// Spalte hinzufügen
			if (ergebnis->tabellenAnsicht->GetNumberCols() <= j)
			{
				ergebnis->spalteHinzu();
			}
			std::string cellVal = *tok_iter;
			ergebnis->tabellenAnsicht->SetCellValue(i, j, cellVal);

			j++;
		}
	}
	eingabe.close();

}