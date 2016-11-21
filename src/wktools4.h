/***************************************************************
* Name:      wktoolsApp.h
* Purpose:   Defines Application Class
* Author:    Mathias Spoerr (wktools@spoerr.org)
* Created:   2007-11-03
* Copyright: Mathias Spoerr (http://www.spoerr.org/wktools)
* License:
**************************************************************/

#pragma once
#include "stdwx.h"
#include "wktools4Frame.h"
#include "class_wkLogFile.h"
#include "class_sqlConn.h"
#include "class_dirMappingData.h"
#include "class_wkErrMessages.h"

#include <wx/cmdline.h>

#include "class_wkn.h"
#include "class_wke.h"
#include "class_wkl.h"
#ifdef WKTOOLS_MAPPER
#include "class_wkm.h"
#endif

#include "class_wzrd.h"

// Pfad Seperator herausfinden

class Cwktools4App : public wxApp
{
private:
	bool silent_mode;					// kein GUI zeigen
	bool saveOutput;					// LOG Ausgabe in das definierte File sichern
	bool tv;							// Tool vorhanden
	bool pv;							// Profil vorhanden
	bool swapXml;						// Anderes wktools.xml laden?
	int update;							// Update?
	int updateMessage;					// Update Nachricht
	wxString tool;						// Verwendetes Tool
	wxString profil;					// Profil zu laden
	TiXmlDocument doc;					// XML Doc von wktools.xml
	std::string FEHLER;						// letzter Fehler wird abgespeichert
	Cwktools4Frame *frame;				// Hauptframe
	WkLogFile *logFile;					// LogFileausgabe
	CSqlConn *sqlConn;					// SQL Fehlerausgabe
	CDirMappingData *dMaps;				// Directory Mappings

	size_t xmlHashDef;					// Hash über das Default wktools xml zum Feststellen, ob es zwischenzeitlich verändert wurde
	size_t xmlHashAlt;					// Hash über das Alternate wktools xml zum Feststellen, ob es zwischenzeitlich verändert wurde
	std::string xmlLoc;					// wktools.xml Pfad

	wxString appPfad;					// Programm Pfad

	bool toolSets();					// Zum Initialisieren der Tool/Profil relevanten CLI Eingaben
	void toolSetsGui();					// Zum Initialisieren der Tool/Profil relevanten CLI Eingaben

	void vCheckOption();				// liefert die Version Check Infos
	void sqlOption();					// setzt die SQL Optionen
	void xmlOption();					// setzt die wktools.xml Optionen

	void wkErrStatusLesen();			// Einlesen vom Status der Fehlermeldungen aus wktools.xml
	std::string dirUmbauen(				// Umwandeln von Windows<->Linux Directory Format
		std::string dirAlt
		);		
	
public:
	virtual ~Cwktools4App(void);
	Wkn *erstellen;						// Erstellen
	Wke *einspielen;					// Einspielen
	Wkl *iplist;						// IP List
#ifdef WKTOOLS_MAPPER
	Wkm *mapper;						// Mapper
#endif
	Wzrd *wizard;						// Wizard
	WKErr *wkFehler;					// Fehler Objekt
	virtual int OnExit();
	void doSwapXml();					// wktools.xml swappen -> wenn zentrales geladen, dann lade lokales und umgekehrt
	int schreibeWktoolsXml();			// schreiben von wktools.xml
	int checkWktoolsXmlChange();		// schauen, ob wktools.xml woanders verändert wurde
	void wkErrStatusSchreiben();		// Schreiben vom Status der Fehlermeldungen in wktools.xml
	void schreibeReport();				// Report File schreiben

protected:
	virtual bool OnInit();
	virtual int OnRun();
	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

	// Settings Funktionen und Variablen
	// Zum globalen Laden und Abspeichern der Einstellungen und Profile
	int leseWktoolsXml();				// erstes Einlesen der wktools.xml
};

DECLARE_APP(Cwktools4App)


static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	// Hilfe
	{wxCMD_LINE_SWITCH, "h", "help",       "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{wxCMD_LINE_SWITCH, "v", "version",    "displays wktools Version"},

	// bool Optionen
	{wxCMD_LINE_SWITCH, "s", "silent",     "disables the GUI" },
	{wxCMD_LINE_SWITCH, "S", "Save",       "Save Output in the related log file for the tool" },

	// Optionen mit weiteren Angaben
	{wxCMD_LINE_OPTION, "t", "tool",       "specifies the Tool to start (cfgm, cfg, ipl, map)", wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR},
	{wxCMD_LINE_OPTION, "p", "profile",    "Profile to load", wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR},

	// ENDE
	{wxCMD_LINE_NONE }
};