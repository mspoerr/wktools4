#pragma once
#include "stdwx.h"
#include "class_wkErrMessages.h"
#include <wx/html/htmlwin.h>
#include "class_logEvent.h"

#include <string>

class WkLog :
	public wxPanel
{
public:
	WkLog(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~WkLog(void);
	
	wxTextCtrl *logFenster;
	wxTextCtrl *debugFenster;
	WKErr *wkFehler;

	enum TYPE {
		WkLog_ABSATZ,
		WkLog_ZEIT,
		WkLog_NORMALTYPE,
	};
	
	enum FARBE {					// verfügbare Farbeinstellungen
		WkLog_WKTOOLS,
		WkLog_SCHWARZ,
		WkLog_GRUEN,
		WkLog_ROT,
		WkLog_BLAU,
		WkLog_TUERKIS,
		WkLog_GRAU,
		WkLog_DUNKELROT,
		WkLog_HELLGRUEN,
	};

	enum FORMAT {					// verfügbare Format Einstellungen
		WkLog_NORMALFORMAT,
		WkLog_FETT,
		WkLog_KURSIV,
		WkLog_UNTERS
	};

	enum TOOL {						// Tool IDs - For future Use
		WkLog_SYSTEM = 0,
		WkLog_WKN,
		WkLog_WKE,
		WkLog_WKL,
		WkLog_WKDP,
		WkLog_SCHED
	};

	enum SEVERITY {					// SEVERITY des Logs
		WkLog_INFO,
		WkLog_FEHLER,
		WkLog_SYSTEMFEHLER,
		WkLog_DEBUGFEHLER
	};

	void schreibeLog(				// Log Eintrag schreiben
		int type,						// Log Type
		std::string logEintrag,			// Text für den Logeintrag
		std::string log2,				// Fehlercode
		int report,						// In den Report mitaufnehmen?
		int farbe = WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog_NORMALFORMAT,// Format
		int groesse = 10,				// Schriftgröße
		int severity = WkLog_INFO
		);
	void schreibeLogDummy(			// Dummy Log Eintrag schreiben
		int typeDummy,
		std::string logDummy,	
		std::string log2Dummy,			// Fehlercode
		int report,						// In den Report mitaufnehmen?
		int farbeDummy = WkLog_SCHWARZ,
		int formatDummy = WkLog_NORMALFORMAT,
		int groesseDummy = 10,
		int severity = WkLog_INFO
		);
	void keinDebug(					// Debug Anzeige Ein/Aus schalten
		bool einAus						// True: Aus; False: Ein
		);
	std::string getLogInhalt();		// Inhalt vom Log Fenster als String zurückgeben
	struct evtData {					// Daten, die mit dem Event an das Logfenster gegeben werden
		int type;
		std::string logEintrag;
		std::string logEintrag2;			// Erweiterter LogEintrag, falls benötigt (In wke für SQL Ausgabe verwendet)
		std::string logEintrag3;			// Noch mehr Zusatzinfos für SQL
		int farbe;
		int format;
		int groesse;
		std::string ipa;					// IP Adresse
		int report;
		int severity;
		int toolSpecific;					// Tool-spezifischer Wert
		int toolSpecific2;					// Tool-spezifischer Wert #2
		int tool;							// Tool-ID

		//master comment
	};
private:
	bool zeitangabe;				// gibt an, ob eine Zeitangabe beim Log babei ist.
	wxFlexGridSizer *gs;			// Sizer für die Log Ausgabe
	
	//Branch2 commend

	void OnSchreibeLog(LogEvent &event);

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

	//Branch1
};
