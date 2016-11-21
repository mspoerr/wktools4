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
#include "class_wkLogFile.h"
#include "class_sqlConn.h"
#include "class_dirMappingData.h"
#include "class_wkErrMessages.h"

#include <string>
#include <deque>
#include <queue>
#include <map>

typedef unsigned int uint;
typedef std::pair <std::string, std::string> pss;
typedef std::deque<std::string> dqstring;
typedef std::queue<std::string> qstring;


class Cwktools4Frame;			// Vorwärtsdekleration
class Verbinder;


class Wke : public wxEvtHandler
{
private:
	// Werte Variablen: Settings, die aus der wktools.xml Datei gelesen werden,
	//                  oder im GUI definiert werden

	std::string vars[16];						// Array für die String Variablen
	int vari[25];							// Array für die Int Variablen

	// Allgemeine Einstellungen:
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	TiXmlElement *pElemProf;			// das Element, das auf das WKN Profil zeigt
	CWkePropGrid *wkeProps;				// Das Settings Fenster
	int profilIndex;					// Index des Profils in wktools.xml
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Cwktools4Frame *wkMain;				// Mainframe
	DevGrpPanel *devGrp;				// DeviceGroup Fenster
	WkLogFile *logfile;					// Zeiger auf das LogFile
	CDirMappingData *dMaps;				// DirMapping Infos
	WKErr *wkFehler;					// Fehlermeldungen

	volatile Verbinder *serVerbinder;	// Zeiger auf den im seriellen Modus verwendeten Verbinder
	volatile bool stopThread;			// Soll der Thread gestoppt werden?
	volatile bool ueFehler; 			// Fehler während des Ausführens festgestellt
	volatile bool ende;					// EndeAnzeiger

	bool devGroupIK;					// Zeigt an, ob das Devicegroup File inkonsistent ist
	bool devGroupInit;					// True, wenn bereits einmal in DevGroup geschrieben wurde
	bool devGroupV2;					// Version 2 vom DevGroup File wird verwendet
	bool devGroupV3;					// Version 3 vom DevGroup File wird verwendet
	bool ipPortFile;					// Zeigt an, ob das IP Adress File per wktools Portscanner generiert wurde
	int devGroupSPanzahl;				// Anzahl der Strichpunkte pro Zeile im DevGroupFile
	volatile int derzeitigeIP;			// IP mit dieser ID wird gerade bearbeitet
	std::string ipA;					// IP Adresse
	bool plll;							// Paralleler Modus oder nicht
	bool mitThread;						// mit oder ohne Thread

	std::string ausgabeVz;					// Ausgabeverzeichnis für die show Ausgabe
	std::string configDatei;					// Config Datei
	std::string goodIP;						// alle IPs, bei denen es keinen Fehler gab
	std::string badIP;						// alle IPs, bei denen es eine Warnung gab
	std::string worstIP;						// alle IPs, bei denen es einen schwerwiegenden Fehler gab
	std::string tempProt;					// für Multihop, damit festgestellt werden kann, ob immer das selbe Protokoll eingestellt ist
	std::string desc;						// Aktuelle letzte Spalte (Beschreibung) von der DeviceGroup

	std::multimap <std::string, std::string> fehlerCodes;	// Hier werden die einzelnen Fehlercodes gespeichert

	qstring dgSpalten[12];				//  Spalten 1-12

	CSqlConn *sqlConn;					// Sql Connection für Fehlerausgabe in SQL

	bool einstellungenInit();			// Werte zum Profil werden geladen
	bool doTest(						// teste Eingaben (nur xml File)
		std::string profilName
		);
	bool doTestGui(						// teste Eingaben (GUI)
		std::string profilName
		);
	bool testDGfile(					// zum Testen, ob das DeviceGroup File konsistent ist.
		std::string dgfile
		);
	bool testIPAfile(					// zum Testen, ob das IP Adress File konsistent ist
		std::string ipfile
		);
	bool cfgTestFile(					// Zum Testen, ob das Config File gültig ist
		std::string fname						// Filename
		);
	bool cfgTestDummy(					// Dummy Config File Tester; Für Inventory und Intf Config Files
		std::string dummyString					// Dummy
		);
	bool startVerbinder();				// Thread Start
	bool startSeriellDevGroup();		// Start Seriell Einspielen
	bool ladeDevGrp();					// Laden der DeviceGroup in die qstrings

	std::string dirUmbauen(					// Umwandeln von Windows<->Linux Directory Format
		std::string dirAlt
		);		
	void newlineEater(					// Zum Löschen der \n in der Linux Version
		std::string filename						// File, in dem alle \n's gelöscht werden sollen
		);

	void OnWkeFertig(wxCommandEvent &event);
	void OnWkeLog(LogEvent &event);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	Wke();								// Konstruktor
	~Wke();								// Destruktor

	void doIt(							// starte Tool
		bool wt = true						// mit oder ohne neuen Thread
		);			
	void doItNoThread();				// starte Tool ohne neuen Thread
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
		WkLogFile *lf,						// Zeiger auf das Logfile Objekt; !=NULL, wenn in das Logfile geschrieben werden soll
		CSqlConn *sqlC,						// Zeiger auf die SQL Fehler Ausgabe
		CDirMappingData *dirMap,			// DirMapping Infos
		WKErr *fp							// Fehlermeldungen
		);
	void guiSets(						// wichtige Grundeinstellungen
		CWkePropGrid *wkeProp,				// XML Doc von wktools.xml
		WkLog *ausgabe,						// Pointer auf das Ausgabefenster
		DevGrpPanel *dg						// Pointer auf das DeviceGroup Fenster
		);
	wxArrayString ladeProfile();		// zum Laden der Profile
	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2 = "",					// Zusätzlicher Text fürs Log
		int report = 0,								// report?
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int dringlichkeit = WkLog::WkLog_INFO,	// Dringlichkeit
		int groesse = 10						// Schriftgröße
		);
	void setMainFrame(					// zum Setzen vom MainFrame
		Cwktools4Frame *fp					// MainFrame
		);	
	void schreibeDevGroup(				// Schreibe das DeviceGroup File
		std::string filename	= ""				// Name vom DeviceGroup File
		);
	void aktualisiereDevGroup();		// zum Aktualisieren der Device Group Ansicht
	void dgInit();						// DeviceGroup initialisieren und alle Einträge auswählen -> nur für Wizard!

	boost::thread *thrd;				// Worker Thread, damit das GUI nicht blockiert wird
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
		WKE_DGS_REMARK,				// Bemerkung
		WKE_DGS_REMARK2				// Bemerkung
	};
	enum ZeilenFarben {			// Mögliche Zeilenfarben
		WKE_ZF_WEISS,
		WKE_ZF_GRUEN,
		WKE_ZF_GELB,
		WKE_ZF_ROT
	};
};



#endif
