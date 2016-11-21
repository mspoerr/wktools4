#ifndef __SCHEDULER__
#define __SCHEDULER__


#include "stdwx.h"
#include "class_schedulerPropGrid.h"
#include "wktools4Frame.h"
#include "class_schedPanel.h"


class Cwktools4Frame;			// Vorwärtsdekleration


class Scheduler : public wxEvtHandler
{
private:
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	TiXmlElement *pElemProf;			// das Element, das auf das WKN Profil zeigt
	CSchedPropGrid *schedProps;			// Das Settings Fenster
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Cwktools4Frame *wkMain;				// Mainframe
	SchedPanel *schedPanel;				// Scheduler Anzeige
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	int schedSPanzahl;					// SpaltenAnzahl im Scheduler File
	int aendereZeile;					// Zeile, die es zu ändern gilt
	int toolID;							// Tool ID ddes Tools, das gerade ausgeführt wird
	bool aenderung;						// Zeile wurde angeklickt und kann geändert werden
	volatile bool stopThread;			// Soll der Thread gestoppt werden?
	boost::thread thrd;					// Worker Thread, damit das GUI nicht blockiert wird
	std::string startZeit;					// StartZeit für den Task
	std::string startDatum;					// StartDatum für den Task
	bool nameEindeutig(					// Check, ob Name eindeutig
		std::string name							// Name
		);
	void konvertiereZeit(				// Konvertiere Zeit in schTask lesbares Format
		std::string zeit							// Zeit zu konvertieren
		);
	bool eintragErstellen(				// Eintrag in Tabelle erstellen (wird von neuer Eintrag aufgerufen)
		int zeile,							// Zeile, in der der Eintrag erstellt werden soll
		bool neueZeile,						// soll eine neue Zeile hinzugefügt werden?
		std::string eName						// Name
		);
	bool checkDateTimeVal(				// Check DateTime Eingabe
		std::string dateTime,					// DateTime std::string zum checken
		int type							// Type: 0: Date; 1: Time
		);
	int testeListenFile(				// Test das Scheduler File vor dem Laden
		std::string lifi							// Scheduler File Name
		);	
	void statusAendern();				// Status der Status Spalte ändern
	void startTasks();					// Starten der Scheduler Tasks
	void OnSchedulerFertig(wxCommandEvent &event);

public:
	Scheduler();
	~Scheduler();
	
	void doIt();						// starte Tasks
	void cancelIt();					// cancel Tasks
	void kilIt();						// kill Tasks
	void ladeSchedProfile();				// Zum Laden der Profile im SchedPropGrid
	void ladeSchedFile(						// Laden eines Schedulerfiles
		wxString filename						// Filename
		);
	void sichereSchedFile(					// Spechern der SchedFile Tabelle
		wxString filename							// Filename
		);
	bool aktualisiereSchedTabelle();		// Lade die Scheduler Tabelle neu
	bool sichereSchedTabelle();				// Speichere den Inhalt der Sched Tabelle in ein File
	void sets(								// wichtige allgemeine Grundeinstellungen
		TiXmlDocument *document,				// XML Doc von wktools.xml
		bool guiStats							// Silent oder GUI Mode; Wichtig
		);
	void guiSets(							// wichtige Grundeinstellungen
		CSchedPropGrid *schedProp,				// PropertyGrid
		WkLog *ausgabe,							// Pointer auf das Ausgabefenster
		SchedPanel *schedPanel					// SchedPanel
		);
	void schreibeLog(						// Funktion für die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	void setMainFrame(						// zum Setzen vom MainFrame
		Cwktools4Frame *fp						// MainFrame
		);	
	bool neuerEintrag();					// Neuen Eintrag in der Scheduler Tabelle erstellen
	void bearbeiteEintrag(					// Zeile wurde in der Tabelle markiert -> lade Daten in PropGrid
		int selRow								// markierte Zeile
		);				
	void loescheEintrag(					// Zeile wurde in der Tabelle zunm Löschen markiert; Wichtig für schuled Items
		int selRow								// zu löschende Zeile
		);				

	DECLARE_EVENT_TABLE()


};


#endif