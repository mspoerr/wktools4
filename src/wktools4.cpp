/***************************************************************
* Name:      wktoolsApp.cpp
* Purpose:   Code for Application Class
* Author:    Mathias Spoerr (wktools@spoerr.org)
* Created:   2007-11-03
* Copyright: Mathias Spoerr (http://www.spoerr.org/wktools)
* License:
**************************************************************/
// Änderungen
///////////////////////////////////////////////////////////////
//
// 07.01.2008 - mspoerr:
//     - Erweiterungen des Basisgerüsts
//       - Kommentare eingeführt
//       - toolSets() erweitert
//       - Abfrage in OnInit(), damit toolSets nur bei gültiger CLI Eingabe ausgeführt wird
//       - FEHLER Variable eingeführt, damit es nur wenige Ausgabepunkte für Fehlermeldungen gibt.

#include "wktools4.h"
#include "class_versionCheck.h"
#include <wx/stdpaths.h>
#include "version.h"
#include <boost/functional/hash.hpp>
#include <fstream>
#include <sstream>

//#ifdef __WXDEBUG__
//#define new WXDEBUG_NEW
//#endif

IMPLEMENT_APP(Cwktools4App)



Cwktools4App::~Cwktools4App(void)
{
	delete erstellen;
	delete einspielen;
	delete logFile;
#ifdef WKTOOLS_MAPPER
	delete mapper;
#endif
	delete iplist;
	delete sqlConn;
	delete dMaps;
	delete wizard;
	delete wkFehler;
}

bool Cwktools4App::OnInit()
{
	frame = NULL;
	erstellen = NULL;
	einspielen = NULL;
#ifdef WKTOOLS_MAPPER
	mapper = NULL;
#endif
	logFile = NULL;
	sqlConn = NULL;
	iplist = NULL;
	dMaps = NULL;
	wizard = NULL;
	wkFehler = NULL;

	// call default behaviour (mandatory)
	if (!wxApp::OnInit())
		return false;

	appPfad = "";
	xmlLoc = "";
	swapXml = false;

	wxStandardPaths sp;
	appPfad = sp.GetDataDir();
	wxSetWorkingDirectory(appPfad);
	
	if (!leseWktoolsXml())
	{
		wxLogError(appPfad);
		wxLogError("wktools.xml not found or corrupt! Terminating.");
		std::cout << std::endl << "wktools.xml not found or corrupt! Terminating." << std::endl;
		return false;
	}


	// Wenn die Ausgabe in ein Logfile gesichert werden soll
	// Im GUI Mode wird es immer gesichert.
	wkFehler = new WKErr;
	wkErrStatusLesen();
	if (saveOutput || !silent_mode)
	{
		logFile = new WkLogFile(wkFehler);
	}

	sqlConn = new CSqlConn;
	sqlOption();
	
	// Tool Controls erstellen und initialisieren
	erstellen = new Wkn();
	erstellen->sets(&doc, silent_mode, logFile);
	einspielen = new Wke();
	einspielen->sets(&doc, silent_mode, logFile, sqlConn, dMaps, wkFehler);
	iplist = new Wkl();
	iplist->sets(&doc, silent_mode, logFile);
#ifdef WKTOOLS_MAPPER
	mapper = new Wkm();
	mapper->sets(&doc, silent_mode, logFile);
#endif
	wizard = new Wzrd();
	wizard->sets(&doc, silent_mode, logFile);


	//silent_mode = true;
	if (!silent_mode)
	{
		// Show the frame
		frame = new Cwktools4Frame;
		frame->sets(&doc, silent_mode, wkFehler);
		frame->erstellen = erstellen;
		frame->einspielen = einspielen;
#ifdef WKTOOLS_MAPPER
		frame->mapper=mapper;
#endif
		frame->iplist = iplist;
		frame->wizard = wizard;

		frame->Create(NULL, wxID_ANY, "wktools Version 4", wxDefaultPosition, wxDefaultSize);
		//frame->SetStatusText(_T("w k t o o l s"));
		frame->Show(TRUE);
		SetTopWindow(frame);

		sqlConn->logAusgabe = frame->logAusgabe;

		toolSetsGui();
		
		// Version Check
		updateMessage = true;
		update = true;

		vCheckOption();
		frame->updateMessage = updateMessage;

		if (update)
		{
			VersionCheck *check = new VersionCheck(frame->logAusgabe);
			if ( check->Create() == wxTHREAD_NO_ERROR )
			{
				check->Run();
			}
		}
		if (FEHLER != "")
		{
			frame->logAusgabe->schreibeLog(WkLog::WkLog_ABSATZ, FEHLER, "", 0, WkLog::WkLog_TUERKIS, WkLog::WkLog_FETT);
		}
	}
	else
	{
		std::cout << FEHLER;
	}
	

	if (tv && pv)
	{
		if (!toolSets())
		{
			std::cout << FEHLER;
			return false;
		}
	}


	return true;
	
} 

int Cwktools4App::OnExit()
{
	// Das wktools.xml wird jetzt direkt vom GUI Mode aus geschrieben (wktools4Frame)
	//if (tv && pv)
	//{
	//	// Im CLI Modus das wktools xml nicht schreiben
	//}
	//else
	//{
	//	schreibeWktoolsXml();
	//}
	if (silent_mode)
	{
		schreibeReport();
	}
	return 0;
}


int Cwktools4App::OnRun()
{
	int exitcode = wxApp::OnRun();
	//wxTheClipboard->Flush();
	if (exitcode!=0)
		return exitcode;
}


// void OnInitCmdLine
// zum Initialisieren der CLI
void Cwktools4App::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc (g_cmdLineDesc);
	// must refuse '/' as parameter starter or cannot use "/path" style paths
	parser.SetSwitchChars (wxT("-"));
}

// bool OnCmdLineParsed
// zum Parsen der CLI Parameter
// return Werte: true: Parsen ok
bool Cwktools4App::OnCmdLineParsed(wxCmdLineParser& parser)
{
	bool versionOutput = parser.Found("v");

	if (versionOutput)
	{
		std::string verAusgabe = "\nwktools Version: " + WKTOOLSVERSION + "\n\n";
		std::cout << verAusgabe;
#ifdef _WINDOWS_
		wxLogMessage(verAusgabe.c_str());
#endif

		tv = true;
		pv = true;
		return false;
	}
	tv = false;
	pv = false;
	saveOutput = true;
	silent_mode = false;

	silent_mode = parser.Found("s");

	saveOutput = parser.Found("S");
	tv = parser.Found("t", &tool);
	pv = parser.Found("p", &profil);

	//tv = true;
	//pv = true;
	//tool = "cfgm";
	//profil = "RB-BO-Config";
	//silent_mode = true;
	//saveOutput = true;


	return true;
}


// bool toolSets
// Wenn CLI Parameter vorhanden, dann soll bei Bedarf das richtige Tool mit den entsprechenden Parametern geladen werden
// return Werte: true: alles ok
//				 false: Probleme beim Laden des Tools
bool Cwktools4App::toolSets()
{
	if (tool == "cfgm")
	{
//		wxLogError("TOOL");
		if (!erstellen->doLoad(profil.c_str()))
		{
			FEHLER += "0301: Error on loading Settings for \"ConfigMaker\". Please check wktools.xml!\n";
			return false;
		}

		erstellen->doIt();
	}
	else if (tool == "cfg")
	{
		if (!einspielen->doLoad(profil.c_str()))
		{
			FEHLER += "0301: Error on loading Settings for \"Configure Devices\". Please check wktools.xml!\n";
			return false;
		}

		einspielen->doIt();
	}
	else if (tool == "ipl")
	{
		if (!iplist->doLoad(profil.c_str()))
		{
			FEHLER += "0301: Error on loading Settings for \"IP List\". Please check wktools.xml!\n";
			return false;
		}

		iplist->doIt();
	}
#ifdef WKTOOLS_MAPPER
	else if (tool == "map")
	{
		if (!mapper->doLoad(profil.c_str()))
		{
			FEHLER += "0301: Error on loading Settings for \"Mapper\". Please check wktools.xml!\n";
			return false;
		}

		mapper->doIt();
	}
#endif
	else if (tool == "cmp")
	{
	}
	else
	{
		FEHLER += "0302: Unknown tool!\n";
		return false;
	}
	return true;
}


// int leseWktoolsXml
// zum Einlesen von wktools.xml
// return Werte: 0: Fehler
//               1: Datei erfolgreich geladen
int Cwktools4App::leseWktoolsXml()
{
	// wktools.xml einlesen und Hash erstellen; Dann wieder schließen
	std::string xmlFile;
	std::ifstream eingabe("wktools.xml", std::ios_base::in);
	getline(eingabe, xmlFile, '\0');
	eingabe.close();
	boost::hash<std::string> string_hash;
	xmlHashDef = string_hash(xmlFile);
	
	doc.LoadFile("wktools.xml");
	bool loadOkay = doc.LoadFile();

	if (!loadOkay)
	{
		return 0;
		FEHLER += "0201: Error on loading wktools.xml!\n";
	}

	if (!swapXml)
	{
		xmlOption();
	}
	dMaps = new CDirMappingData(&doc);
	dMaps->loeschen();
	dMaps->dirMapsEinlesen();

	if (xmlLoc != "")
	{
#ifdef _WINDOWS_
		size_t dirEndPos = xmlLoc.find_last_of("/");
		if (dirEndPos != xmlLoc.npos)
		{
			std::string dir = xmlLoc.substr(0, dirEndPos+1);
			dMaps->werteIter = dMaps->dirMapsLin.find(dir);
			
			if (dMaps->werteIter != dMaps->dirMapsLin.end())
			{
				xmlLoc.replace(0, dirEndPos+1, dMaps->werteIter->second);
			}
		}
		
#else
		size_t dirEndPos = xmlLoc.find_last_of("\\");
		if (dirEndPos != xmlLoc.npos)
		{
			std::string dir = xmlLoc.substr(0, dirEndPos+1);
			dMaps->werteIter = dMaps->dirMapsWin.find(dir);

			if (dMaps->werteIter != dMaps->dirMapsWin.end())
			{
				xmlLoc.replace(0, dirEndPos+1, dMaps->werteIter->second);
			}
		}
#endif

		// wktools.xml einlesen und Hash erstellen; Dann wieder schließen
		std::string xmlFile;
		std::ifstream eingabe(xmlLoc.c_str(), std::ios_base::in);
		getline(eingabe, xmlFile, '\0');
		eingabe.close();
		boost::hash<std::string> string_hash;
		xmlHashAlt = string_hash(xmlFile);

		loadOkay = doc.LoadFile(xmlLoc);
		if (!loadOkay)
		{
			FEHLER += "0204: Error on loading alternate wktools.xml! Using default instead.\n";
			doc.LoadFile("wktools.xml");
			xmlLoc = "";
		}
		else
		{
			FEHLER += "0604: Alternate wktools.xml loaded: " + xmlLoc + "\n";
			dMaps->dirMapsEinlesen();
		}

	}

	return 1;
}


// int schreibeWktoolsXml
// zum Sichern der Werte in wktools.xml
// return Werte: 1: alles ok
int Cwktools4App::schreibeWktoolsXml()
{
	if (xmlLoc != "")
	{
		doc.SaveFile(xmlLoc);
		
		// wktools.xml einlesen und Hash erstellen; Dann wieder schließen
		// Um den Hash upzudaten
		std::string xmlFile;
		std::ifstream eingabe(xmlLoc.c_str(), std::ios_base::in);
		getline(eingabe, xmlFile, '\0');
		eingabe.close();
		boost::hash<std::string> string_hash;
		xmlHashAlt = string_hash(xmlFile);
	}
	else
	{
		doc.SaveFile("wktools.xml");

		// wktools.xml einlesen und Hash erstellen; Dann wieder schließen
		// Um den Hash upzudaten
		std::string xmlFile;
		std::ifstream eingabe("wktools.xml", std::ios_base::in);
		getline(eingabe, xmlFile, '\0');
		eingabe.close();
		boost::hash<std::string> string_hash;
		xmlHashDef = string_hash(xmlFile);

	}
	return 0;
}

// int checkWktoolsXml
// Zum Prüfen, ob das wktools.xml woanders verändert wurde. Wenn nein -> 0; Wenn ja -> 1
int Cwktools4App::checkWktoolsXmlChange()
{
	if (xmlLoc != "")
	{
		// wktools.xml einlesen und Hash erstellen; Dann wieder schließen
		std::string xmlFile;
		std::ifstream eingabe(xmlLoc.c_str(), std::ios_base::in);
		getline(eingabe, xmlFile, '\0');
		eingabe.close();
		boost::hash<std::string> string_hash;
		size_t xmlAltVgl = string_hash(xmlFile);

		if (xmlAltVgl == xmlHashAlt)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		// wktools.xml einlesen und Hash erstellen; Dann wieder schließen
		std::string xmlFile;
		std::ifstream eingabe("wktools.xml", std::ios_base::in);
		getline(eingabe, xmlFile, '\0');
		eingabe.close();
		boost::hash<std::string> string_hash;
		size_t xmlDefVgl = string_hash(xmlFile);

		if (xmlDefVgl == xmlHashDef)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	return 0;
}



void Cwktools4App::toolSetsGui()
{
	erstellen->guiSets(frame->wknSettings, frame->logAusgabe, frame->wknTabelle);
	erstellen->doLoad("DEFAULT");
	einspielen->guiSets(frame->wkeSettings, frame->logAusgabe, frame->dieDevGrp);
	einspielen->doLoad("DEFAULT");
	iplist->guiSets(frame->wklSettings, frame->logAusgabe);
	iplist->doLoad("DEFAULT");
#ifdef WKTOOLS_MAPPER
	mapper->guiSets(frame->wkmSettings, frame->logAusgabe);
	mapper->doLoad("DEFAULT");
#endif
	wizard->guiSets(frame->wzrdSettings, frame->logAusgabe);
	wizard->doLoad("DEFAULT");
}



void Cwktools4App::vCheckOption()
{
	// Grund Elemente für Versions Check
	TiXmlHandle hDoc(&doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSettings;// das Element, das auf Versions check Items zeigt
	TiXmlElement *pElemVCheck;	// das Element, das auf Versions check Items zeigt

	// vCheck im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{

	}
	else
	{
		pElemSettings = pElem->FirstChildElement("Settings");
		pElemVCheck = pElemSettings->FirstChildElement("Updater");
		if (pElemVCheck)
		{
			pElemVCheck->QueryIntAttribute("ud", &update);
			pElemVCheck->QueryIntAttribute("um", &updateMessage);
		}
	}
}


void Cwktools4App::xmlOption()
{
	TiXmlHandle hDoc(&doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSettings;// das Element, das auf Versions check Items zeigt
	TiXmlElement *pElemMisc;	// das Element, das auf Misc Einstellungen zeigt

	// vCheck im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{

	}
	else
	{
		pElemSettings = pElem->FirstChildElement("Settings");
		pElemMisc = pElemSettings->FirstChildElement("MiscSettings");
		
		if (pElemMisc)
		{
			if (pElemMisc->Attribute("xmlLoc"))
			{
				xmlLoc = pElemMisc->Attribute("xmlLoc");
			}
		}
	}
}



void Cwktools4App::sqlOption()
{
	// Grund Elemente für SQL Einstellungen
	TiXmlHandle hDoc(&doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSettings;// das Element, das auf Versions check Items zeigt
	TiXmlElement *pElemSql;		// das Element, das auf SQL Settings Items zeigt

	TiXmlElement *pElemWke = NULL;
	TiXmlElement *pElemStrSets = NULL;
	TiXmlElement *pElemProf = NULL;

	// vCheck im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{

	}
	else
	{
		int useSql = 0;
		std::string sqlSrv = "";
		std::string sqlUsr = "";
		std::string sqlPwd = "";
		std::string descr = "";

		pElemSettings = pElem->FirstChildElement("Settings");
		pElemSql = pElemSettings->FirstChildElement("SQLSettings");

		if (profil == "")
		{
			profil = "DEFAULT";
		}
		pElemWke = pElem->FirstChildElement("wke");
		pElemProf = pElemWke->FirstChildElement("Profile");
		for (int i = 0; pElemProf; i++)
		{
			std::string profname = pElemProf->Attribute("name");
			if (profname == profil)
			{
				pElemStrSets = pElemProf->FirstChildElement("Settings");
				if (pElemStrSets->Attribute("hostid"))
				{
					descr = pElemStrSets->Attribute("hostid");
				}
				else
				{
					descr = "";
				}
				break;
			}
			pElemProf = pElemProf->NextSiblingElement();
		}


		if (pElemSql)
		{
			pElemSql->QueryIntAttribute("UseSql", &useSql);

			if (pElemSql->Attribute("sqlSrv"))
			{
				sqlSrv = pElemSql->Attribute("sqlSrv");
			}
			if (pElemSql->Attribute("sqlUsr"))
			{
				sqlUsr = pElemSql->Attribute("sqlUsr");
			}
			if (pElemSql->Attribute("sqlPwd"))
			{
				sqlPwd = pElemSql->Attribute("sqlPwd");
			}
			int ret = sqlConn->sets(useSql, sqlSrv, sqlUsr, sqlPwd, descr);
			if (ret)
			{
				FEHLER += "0205: Unable to connect to the SQL database!\n";
			}
		}
	}
}


void Cwktools4App::doSwapXml()
{
	swapXml = !swapXml;
	xmlLoc = "";
	leseWktoolsXml();
	

	erstellen->sets(&doc, silent_mode, logFile);
	einspielen->sets(&doc, silent_mode, logFile, sqlConn, dMaps, wkFehler);
	iplist->sets(&doc, silent_mode, logFile);
#ifdef WKTOOLS_MAPPER
	mapper->sets(&doc, silent_mode, logFile);
#endif
	wizard->sets(&doc, silent_mode, logFile);

	erstellen->doLoad("DEFAULT");
	einspielen->doLoad("DEFAULT");
	iplist->doLoad("DEFAULT");
#ifdef WKTOOLS_MAPPER
	mapper->doLoad("DEFAULT");
#endif
	wizard->doLoad("DEFAULT");
}


void Cwktools4App::wkErrStatusLesen()
{
	// Grund Elemente für Versions Check
	TiXmlHandle hDoc(&doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSettings;// das Element, das auf Settings Items zeigt
	TiXmlElement *pElemFehler;	// das Element, das auf Fehler Items zeigt

	// vCheck im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{

	}
	else
	{
		pElemSettings = pElem->FirstChildElement("Settings");
		pElemFehler = pElemSettings->FirstChildElement("wkError");
		if (pElemFehler)
		{
			std::multimap <std::string, int>::iterator errStatIter, errStatAnfang, errStatEnde;
			for (errStatIter = wkFehler->errStatus.begin(); errStatIter != wkFehler->errStatus.end(); ++errStatIter)
			{
				std::string fname = "F" + errStatIter->first;
				pElemFehler->QueryIntAttribute(fname.c_str(), &errStatIter->second);
			}
		}
	}
}


void Cwktools4App::wkErrStatusSchreiben()
{
	// Grund Elemente für Versions Check
	TiXmlHandle hDoc(&doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSettings;// das Element, das auf Settings Items zeigt
	TiXmlElement *pElemFehler;	// das Element, das auf Fehler Items zeigt

	// vCheck im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{

	}
	else
	{
		pElemSettings = pElem->FirstChildElement("Settings");
		pElemFehler = pElemSettings->FirstChildElement("wkError");
		if (!pElemFehler)
		{
			pElemFehler = new TiXmlElement("wkError");
			pElemSettings->LinkEndChild(pElemFehler);
		}
		std::multimap <std::string, int>::iterator errStatIter, errStatAnfang, errStatEnde;
		for (errStatIter = wkFehler->errStatus.begin(); errStatIter != wkFehler->errStatus.end(); ++errStatIter)
		{
			std::string fname = "F" + errStatIter->first;
			pElemFehler->SetAttribute(fname.c_str(), errStatIter->second);
		}
	}
}


void Cwktools4App::schreibeReport()
{
	std::string repFile= "";

	TiXmlHandle hDoc(&doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSettings;// das Element, das auf Versions check Items zeigt
	TiXmlElement *pElemMisc;	// das Element, das auf Misc Einstellungen zeigt

	// vCheck im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{

	}
	else
	{
		pElemSettings = pElem->FirstChildElement("Settings");
		pElemMisc = pElemSettings->FirstChildElement("MiscSettings");

		if (pElemMisc)
		{
			int repStat = 0;
			pElemMisc->QueryIntAttribute("Report", &repStat);

			if (repStat)
			{
				if (pElemMisc->Attribute("ReportFile"))
				{
					repFile = dirUmbauen(pElemMisc->Attribute("ReportFile"));
				}
				else
				{
					repFile = "wktools_report.txt";
				}

				if (repFile == "")
				{
					repFile = "wktools_report.txt";
				}
			}
		}
	}

	if (repFile != "")
	{
		std::string logAusgabe = "";
		if (!silent_mode)
		{
			// Reduzierte Log Ausgabe in Report File schreiben
			logAusgabe = frame->logAusgabe->getLogInhalt();			
		}
		else
		{
			// Report String vom LogFile 
			logAusgabe = logFile->getReportString();			
		}

		std::ofstream reportAusgabe;
		reportAusgabe.rdbuf()->open(repFile.c_str(), std::ios_base::out | std::ios_base::app);
		reportAusgabe << logAusgabe;
		reportAusgabe.close();

	}
}

std::string Cwktools4App::dirUmbauen(std::string dirAlt)
{
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
