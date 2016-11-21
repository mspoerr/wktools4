/***************************************************************
* Name:      class_wke.h
* Purpose:   Defines Configure Devices I/O Class
* Created:   2008-08-22
**************************************************************/

#ifndef __DIP__
#define __DIP__


#include "stdwx.h"
#include "class_dipPropGrid.h"
#include "class_wkLog.h"
#include "wktools4Frame.h"
#include "class_wkLogFile.h"
#include "class_utaparse.h"
#include "class_intfparse.h"
#include "class_tabellePanel.h"

class Cwktools4Frame;			// Vorwärtsdekleration
class UTAParse;
class IntfParse;

class Dip : public wxEvtHandler
{
private:
	// Werte Variablen: Settings, die aus der wktools.xml Datei gelesen werden,
	//                  oder im GUI definiert werden

	std::string vars[5];						// Array für die String Variablen
	int vari[16];							// Array für die Int Variablen

	
	std::string attString[5];
	std::string attInt[16];

	int a1;
	int a2;


	// Allgemeine Einstellungen:
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	TiXmlElement *pElemProf;			// das Element, das auf das WKN Profil zeigt
	CDipPropGrid *dipProps;				// Das Settings Fenster
	int profilIndex;					// Index des Profils in wktools.xml
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Cwktools4Frame *wkMain;				// Mainframe
	WkLogFile *logfile;					// Zeiger auf das LogFile
	UTAParse *derParser;				// Inventory Parser Objekt
	IntfParse *derIntfParser;			//	Interface Info Parser Objekt
	TabellePanel *ergebnis;				// Ergebnisanzeige

//	volatile Verbinder *serVerbinder;	// Zeiger auf den im seriellen Modus verwendeten Verbinder
	volatile bool stopThread;			// Soll der Thread gestoppt werden?
	volatile bool ende;					// EndeAnzeiger
	bool mitThread;						// mit oder ohne Thread

	std::string ausgabeVz;					// Ausgabeverzeichnis für die show Ausgabe
	std::string configDatei;					// Config Datei

	bool einstellungenInit();			// Werte zum Profil werden geladen
	bool doTest(						// teste Eingaben (nur xml File)
		std::string profilName
		);
	bool doTestGui(						// teste Eingaben (GUI)
		std::string profilName
		);
	void startParser();					// Thread Start Inventory Parser
	void startIntfParser();				// Thread Start Interface Information Parser
	void ausgabe();						// Egebnis in Tablle ausgeben

	void OnDipFertig(wxCommandEvent &event);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	boost::thread thrd;					// Worker Thread, damit das GUI nicht blockiert wird

	Dip();								// Konstruktor
	~Dip();								// Destruktor

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
		CDipPropGrid *dipProp,				// XML Doc von wktools.xml
		WkLog *ausgabe,						// Pointer auf das Ausgabefenster
		TabellePanel *table					// Pointer auf die Ergebnistabelle
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
