#pragma once
#include "stdwx.h"
#include "class_logEvent.h"
#include "class_wkErrMessages.h"

#include <fstream>


class WkLogFile : public wxEvtHandler
{
private:
	std::ofstream logAusgabe;			// Log Ausgabestream
	std::string reportString;			// Report String
	WKErr *wkFehler;					// WK Fehlermeldungen
	
	void OnLog(							// Funktion, die auf LogEvent ausgeführt wird und das Logfile beschreibt
		LogEvent &event						// Event Daten
		);

	void schreibeLog(						// Log Eintrag schreiben
		int type,								// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,						// Fehlercode
		int report,								// In den Report mitaufnehmen?
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10,						// Schriftgröße
		int severity = WkLog::WkLog_INFO
		);
	
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	WkLogFile(WKErr *wkFehler);
	~WkLogFile();

	void logFileSets(						// LogFile Einstellungen
		std::string logfile							// Logfile
		);
	std::string getReportString();			// report String zurückgeben


};