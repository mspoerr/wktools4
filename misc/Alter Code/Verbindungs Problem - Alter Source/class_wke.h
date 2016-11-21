/***************************************************************
* Name:      class_wke.h
* Purpose:   Defines Configure Devices I/O Class
* Created:   2008-08-22
**************************************************************/

#ifndef __WKE__
#define __WKE__


#include "stdwx.h"
#include "class_wkePropGrid.h"
#include "class_wkLog.h"
#include "wktools4Frame.h"
#include "class_devGrpPanel.h"
#include "class_verbinder.h"

#include <string>
#include <deque>
#include <queue>
#include <map>
using namespace std;

typedef unsigned short int uint;
typedef pair <string, string> pss;
typedef deque<string> dqstring;
typedef queue<string> qstring;


class Cwktools4Frame;			// Vorwärtsdekleration
class Verbinder;


class Wke : public wxEvtHandler
{
private:
	// Werte Variablen: Settings, die aus der wktools.xml Datei gelesen werden,
	//                  oder im GUI definiert werden

	string vars[14];						// Array für die String Variablen
	int vari[21];							// Array für die Int Variablen

	// Allgemeine Einstellungen:
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	TiXmlElement *pElemProf;			// das Element, das auf das WKN Profil zeigt
	CWkePropGrid *wkeProps;				// Das Settings Fenster
	int profilIndex;					// Index des Profils in wktools.xml
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Cwktools4Frame *wkMain;				// Mainframe
	DevGrpPanel *devGrp;				// DeviceGroup Fenster

	boost::thread thrd;					// Worker Thread, damit das GUI nicht blockiert wird
	volatile Verbinder *serVerbinder;	// Zeiger auf den im seriellen Modus verwendeten Verbinder
	volatile bool stopThread;			// Soll der Thread gestoppt werden?
	volatile bool ueFehlerJetzt;		// Fehler während des Ausführens festgestellt, wird bei jedem Host zurückgesetzt
	volatile bool ueFehler; 			// Fehler während des Ausführens festgestellt
	volatile bool ende;					// EndeAnzeiger

	bool devGroupIK;					// Zeigt an, ob das Devicegroup File inkonsistent ist
	bool devGroupInit;					// True, wenn bereits einmal in DevGroup geschrieben wurde
	bool devGroupV2;					// Version 2 vom DevGroup File wird verwnedet
	int devGroupSPanzahl;				// Anzahl der Strichpunkte pro Zeile im DevGroupFile
	int derzeitigeIP;					// IP mit dieser ID wird gerade bearbeitet
	bool plll;							// Paralleler Modus oder nicht

	string ausgabeVz;					// Ausgabeverzeichnis für die show Ausgabe
	string configDatei;					// Config Datei
	string goodIP;						// alle IPs, bei denen es keinen Fehler gab
	string badIP;						// alle IPs, bei denen es eine Warnung gab
	string worstIP;						// alle IPs, bei denen es einen schwerwiegenden Fehler gab
	string tempProt;					// für Multihop, damit festgestellt werden kann, ob immer das selbe Protokoll eingestellt ist

	qstring dgSpalten[11];				//  Spalten 1-11

	struct evtData {					// Daten, die mit dem Event an das Logfenster gegeben werden
		int type;
		string logEintrag;
		int farbe;
		int format;
		int groesse;
		string ipa;							// IP Adresse
	};

	bool einstellungenInit();			// Werte zum Profil werden geladen
	bool doTest(						// teste Eingaben (nur xml File)
		string profilName
		);
	bool doTestGui(						// teste Eingaben (GUI)
		string profilName
		);
	bool testDGfile(					// zum Testen, ob das DeviceGroup File konsistent ist.
		string dgfile
		);
	bool testIPAfile(					// zum Testen, ob das IP Adress File konsistent ist
		string ipfile
		);
	bool cfgTestFile(					// Zum Testen, ob das Config File gültig ist
		string fname						// Filename
		);
	bool cfgTestDummy(					// Dummy Config File Tester; Für Inventory und Intf Config Files
		string dummyString					// Dummy
		);
	void startVerbinder();				// Thread Start
	bool startSeriellDevGroup();		// Start Seriell Einspielen
	bool ladeDevGrp();					// Laden der DeviceGroup in die qstrings

	void OnWkeFertig(wxCommandEvent &event);
	void OnWkeInfo(wxCommandEvent &event);
	void OnWkeFehler(wxCommandEvent &event);
	void OnWkeDebugFehler(wxCommandEvent &event);
	void OnWkeSystemFehler(wxCommandEvent &event);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	Wke();								// Konstruktor
	~Wke();								// Destruktor

	void doIt();						// starte Tool
	void cancelIt();					// cancel Tool
	void kilIt();						// kill Tool
	bool doLoad(						// lade Profil
		string profilName
		);
	bool doSave(						// sichere Profil
		string profilName
		);
	bool xmlInit(						// Initialisieren der XML Settings
		string profilName,					// Profilname, der gesucht werden soll
		bool speichern = false				// Falls der Profilname nicht gefunden wird, dann soll er angelegt werden
		);
	void sets(							// wichtige allgemeine Grundeinstellungen
		TiXmlDocument *document,			// XML Doc von wktools.xml
		bool guiStats						// Silent oder GUI Mode; Wichtig
		);
	void guiSets(						// wichtige Grundeinstellungen
		CWkePropGrid *wkeProp,				// XML Doc von wktools.xml
		WkLog *ausgabe,						// Pointer auf das Ausgabefenster
		DevGrpPanel *dg						// Pointer auf das DeviceGroup Fenster
		);
	wxArrayString ladeProfile();		// zum Laden der Profile
	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		string logEintrag,					// Text für den Logeintrag
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void setMainFrame(					// zum Setzen vom MainFrame
		Cwktools4Frame *fp					// MainFrame
		);	
	void schreibeDevGroup(				// Schreibe das DeviceGroup File
		string filename	= ""				// Name vom DeviceGroup File
		);
	void aktualisiereDevGroup();		// zum Aktualisieren der Device Group Ansicht

	enum spalten {				// Aufzählung der Spalten
		WKE_DGS_CHECK,				// Checkbox
		WKE_DGS_HOSTNAME,			// Hostname
		WKE_DGS_USERNAME,			// Username
		WKE_DGS_LOPW,				// Login Passwort
		WKE_DGS_ENPW,				// Enable Passwort
		WKE_DGS_CONFIG,				// Config File
		WKE_DGS_PROTOKOLL,			// Protokoll
		WKE_DGS_PORT,				// Port
		WKE_DGS_TSC,				// Terminal Server Connection
		WKE_DGS_MHCOMMAND,			// Multihop Command
		WKE_DGS_REMARK				// Bemerkung
	};
	enum ZeilenFarben {			// Mögliche Zeilenfarben
		WKE_ZF_WEISS,
		WKE_ZF_GRUEN,
		WKE_ZF_GELB,
		WKE_ZF_ROT
	};
};



#endif
