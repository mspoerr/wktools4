/***************************************************************
* Name:      class_wkl.h
* Purpose:   Defines IP List I/O Class
* Created:   2008-08-22
**************************************************************/

#ifndef __WKL__
#define __WKL__


#include "stdwx.h"
#include "class_wklPropGrid.h"
#include "class_wkLog.h"
#include "wktools4Frame.h"
#include "class_wkLogFile.h"
#include "class_ipl.h"

#include <string>

class Cwktools4Frame;			// Vorwärtsdekleration
class IPL;

class Wkl : public wxEvtHandler
{
private:
	// Werte Variablen: Settings, die aus der wktools.xml Datei gelesen werden,
	//                  oder im GUI definiert werden

	std::string vars[7];				// Array für die String Variablen
	int vari[1];						// Array für die Int Variablen
			
	int a1;								// Anzahl der String Settings
	int a2;								// Anzahl der INT Settings

	// Allgemeine Einstellungen:
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	TiXmlElement *pElemProf;			// das Element, das auf das WKN Profil zeigt
	CWklPropGrid *wklProps;				// Das Settings Fenster
	int profilIndex;					// Index des Profils in wktools.xml
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Cwktools4Frame *wkMain;				// Mainframe
	WkLogFile *logfile;					// Zeiger auf das LogFile
	IPL *dieListe;						// Zeiger auf die Liste

	volatile bool stopThread;			// Soll der Thread gestoppt werden?
	volatile bool ende;					// EndeAnzeiger

	std::string ausgabeVz;					// Ausgabeverzeichnis für die show Ausgabe
	std::string configDatei;					// Config Datei
	std::string goodIP;						// alle IPs, bei denen es keinen Fehler gab
	std::string badIP;						// alle IPs, bei denen es eine Warnung gab
	std::string worstIP;						// alle IPs, bei denen es einen schwerwiegenden Fehler gab
	std::string tempProt;					// für Multihop, damit festgestellt werden kann, ob immer das selbe Protokoll eingestellt ist

	bool einstellungenInit();			// Werte zum Profil werden geladen
	bool doTest(						// teste Eingaben (nur xml File)
		std::string profilName
		);
	bool doTestGui(						// teste Eingaben (GUI)
		std::string profilName
		);
	void startParser();				// Thread Start Parser
	void startScanner();			// Thread Start Port Scanner

	void OnWklFertig(wxCommandEvent &event);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	boost::thread thrd;					// Worker Thread, damit das GUI nicht blockiert wird

	Wkl();								// Konstruktor
	~Wkl();								// Destruktor

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
		CWklPropGrid *wklProp,				// XML Doc von wktools.xml
		WkLog *ausgabe						// Pointer auf das Ausgabefenster
		);
	wxArrayString ladeProfile();		// zum Laden der Profile
	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void setMainFrame(					// zum Setzen vom MainFrame
		Cwktools4Frame *fp					// MainFrame
		);	
};



#endif
