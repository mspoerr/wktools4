#ifndef __WKTOOLSVERSIONCHECK__
#define __WKTOOLSVERSIONCHECK__

#include "stdwx.h"
#include "class_wkLog.h"

#include <string>

class VersionCheck : public wxThread
{
public:
	VersionCheck(WkLog *lAsg);
	~VersionCheck();
	virtual void *Entry();				// wxThread Entry Funktion

private:
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	struct evtData {					// Daten, die mit dem Event an das Logfenster gegeben werden
		int type;
		std::string logEintrag;
		int farbe;
		int format;
		int groesse;
	};
	bool vglVersion(					// Vergleiche die zwei Versionen; true, wenn v1 größer; false, wenn gleich
		wxString v1,						// Version 1
		wxString v2							// Version 2
		);
	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		std::string logEintrag,				// Text für den Logeintrag
		std::string log2,					// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);

};

#endif