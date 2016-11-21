/***************************************************************
* Name:      class_wzrd.h
* Purpose:   Defines IP List I/O Class
* Created:   2011-01-07
**************************************************************/

#ifndef __WZRD__
#define __WZRD__

#include "stdwx.h"
#include "class_wzrdPropGrid.h"
#include "class_wkLog.h"
#include "wktools4Frame.h"
#include "class_wkLogFile.h"

#include <string>

class Cwktools4Frame;			// Vorwärtsdekleration

class Wzrd : public wxEvtHandler
{
private:
	// Werte Variablen: Settings, die aus der wktools.xml Datei gelesen werden,
	//                  oder im GUI definiert werden

	std::string vars[6];				// Array für die String Variablen
	int vari[1];						// Array für die Int Variablen

	int a1;								// Anzahl der String Settings
	int a2;								// Anzahl der INT Settings

	// Allgemeine Einstellungen:
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	int wzrdPhase;						// Phasen: 0 - Init; 1 - Start gedrückt; 2 - Start zum zweiten Mal gedrückt
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	TiXmlElement *pElemProf;			// das Element, das auf das WZRD Profil zeigt
	CWzrdPropGrid *wzrdProps;			// Das Settings Fenster
	int profilIndex;					// Index des Profils in wktools.xml
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Cwktools4Frame *wkMain;				// Mainframe
	WkLogFile *logfile;					// Zeiger auf das LogFile

	volatile bool stopThread;			// Soll der Thread gestoppt werden?
	volatile bool ende;					// EndeAnzeiger

	std::string asgbe;					// Ausgabe (für die Log Ausgabe interessant, damit der User weiß, wo das Ergebnis gespeichert wurde)

	bool einstellungenInit();			// Werte zum Profil werden geladen
	bool doTest(						// teste Eingaben (nur xml File)
		std::string profilName
		);
	bool doTestGui(						// teste Eingaben (GUI)
		std::string profilName
		);
	void startWzrd();					// Thread Start Wizard
	void startWzrd2();					// Thread Start Wizard; Phase 2
	void initScanner();					// PortScanner Einstellungen initialisieren
#ifdef WKTOOLS_MAPPER
	void initMapper();					// Mapper Einstellungen initialisieren
#endif
	void initWke();						// WKE Einstellungen initialisieren
	void initCfg();						// Config Speichern - Einstellungen initialisieren

	void OnWzrdFertig(wxCommandEvent &event);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	boost::thread thrd;					// Worker Thread, damit das GUI nicht blockiert wird

	Wzrd();								// Konstruktor
	~Wzrd();								// Destruktor

	void doIt();						// starte Tool
	void cancelIt();					// cancel Tool
	void kilIt();						// kill Tool
	bool doLoad(						// lade Profil
		std::string profilName
		);
	bool doSave(						// sichere Profil
		std::string profilName
		);
	bool doRemove(						// lösche Profil
		std::string profilName
		);
	bool xmlInit(						// Initialisieren der XML Settings
		std::string profilName,				// Profilname, der gesucht werden soll
		int speichern = 0					// Falls der Profilname nicht gefunden wird, dann soll er angelegt werden; Wenn auf 2, dann löschen
		);
	void sets(							// wichtige allgemeine Grundeinstellungen
		TiXmlDocument *document,			// XML Doc von wktools.xml
		bool guiStats,						// Silent oder GUI Mode; Wichtig
		WkLogFile *lf						// Zeiger auf das Logfile Objekt; !=NULL, wenn in das Logfile geschrieben werden soll
		);
	void guiSets(						// wichtige Grundeinstellungen
		CWzrdPropGrid *wzrdProp,			// XML Doc von wktools.xml
		WkLog *ausgabe						// Pointer auf das Ausgabefenster
		);
	wxArrayString ladeProfile();		// zum Laden der Profile
	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		std::string logEintrag,				// Text für den Logeintrag
		std::string log2,					// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void setMainFrame(					// zum Setzen vom MainFrame
		Cwktools4Frame *fp					// MainFrame
		);	
};



#endif
