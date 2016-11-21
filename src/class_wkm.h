#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER
#include "class_wkmPropGrid.h"
#include "class_wkLog.h"
#include "wktools4Frame.h"
#include "class_wkLogFile.h"

#include "class_wkmParser.h"
#include "class_wkmParserDB.h"
#include "class_wkmdb.h"
#include "class_wkmCombiner2.h"
#include "class_wkmAusgabe.h"
#include "class_wkm.h"
#include "class_wkmImport.h"
#include "class_macSearchDialog.h"
#include "class_wkmMacUpdate.h"

class Cwktools4Frame;			// Vorwärtsdeklaration

class Wkm : public wxEvtHandler
{
private:
	// Werte Variablen: Settings, die aus der wktools.xml Datei gelesen werden,
	//                  oder im GUI definiert werden

	std::string vars[9];				// Array für die String Variablen
	int vari[21];						// Array für die Int Variablen

	std::string attString[9];
	std::string attInt[21];

	int a1;
	int a2;

	bool mitThread;						// mit oder ohne Thread

	static const int wkmDbVersion = 14;	// DB Version -> Immer wenn neue Elemente in DB hinzugefügt werden, wird die DB Version um 1 erhöht

	// Allgemeine Einstellungen:
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	TiXmlElement *pElemProf;			// das Element, das auf das WKN Profil zeigt
	CWkmPropGrid *wkmProps;				// Das Settings Fenster
	int profilIndex;					// Index des Profils in wktools.xml
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Cwktools4Frame *wkMain;				// Mainframe
	WkLogFile *logfile;					// Zeiger auf das LogFile
	WkmParser *derParser;				// Zeiger auf den Parser
	WkmParserDB *parserdb;				// Parser DB Operations
	WkmCombiner2 *derCombiner;			// Zeiger auf den Combiner
	WkmDB *dieDB;						// Zeiger auf die DB

	//	volatile Verbinder *serVerbinder;	// Zeiger auf den im seriellen Modus verwendeten Verbinder
	volatile bool stopThread;			// Soll der Thread gestoppt werden?
	volatile bool ende;					// EndeAnzeiger

	std::string ausgabeVz;				// Ausgabeverzeichnis für die show Ausgabe
	std::string configDatei;			// Config Datei

	bool einstellungenInit();			// Werte zum Profil werden geladen
	bool doTest(						// teste Eingaben (nur xml File)
		std::string profilName
		);
	bool doTestGui(						// teste Eingaben (GUI)
		std::string profilName
		);
	void startParser();					// Thread Start Parser

	void OnWkmFertig(wxCommandEvent &event);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	boost::thread thrd;					// Worker Thread, damit das GUI nicht blockiert wird
	bool wkmdbg;						// WKM Debug aktivieren

	Wkm();								// Konstruktor
	~Wkm();								// Destruktor

	int doMacSearch();					// MAC Search Dialog starten
	int doMacUpdate();					// MAC Vendor Update
	void doIt(							// starte Tool
		bool wt = true						// mit oder ohne neuen Thread
		);			
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
		CWkmPropGrid *dipProp,				// XML Doc von wktools.xml
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
