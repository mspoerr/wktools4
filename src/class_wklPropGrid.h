#pragma once
#include "stdwx.h"

#include <wx/propgrid/manager.h>

#include <string>

class CWklPropGrid :
	public wxPanel
{
public:
	CWklPropGrid(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CWklPropGrid(void);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		std::string wert						// Wert des Objektes
		);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		int wert								// Wert des Objektes
		);
	std::string leseStringSetting(				// zum Auslesen von STRING Werten
		int index								// Index des Objektes
		);
	int leseIntSetting(						// zum Auslesen von INT Werten
		int index								// Index des Objektes
		);
	void resetFont(							// zum Setzen der Schriftart
		wxFont newFont							// neue Schriftart
		);
	void setFarbe(							// zum Ein/Ausschalten der Markierungen für die wichtigsten Einstellungen
		bool status								// TRUE: Markieren; FALSE: Keine Markierung
		);
	int getSplitterPos();					// zum Feststellen der SplitterPosition
	void setSplitterPos(					// zum Setzen der SplitterPosition
		int pos									// Position
		);
	void enableDisable();					// Zum Aktivieren/Deaktivieren von Optionen

	enum PROPID_S {							// STRING Property IDs
		WKL_PROPID_STR_SDIR = 0,				// Search Directory
		WKL_PROPID_STR_ODIR,					// Output Directory
		WKL_PROPID_STR_LOGFILE,					// Log File
		WKL_PROPID_STR_INTFNR,					// Interface
		WKL_PROPID_STR_PATTERN,					// Search Pattern
		WKL_PROPID_STR_INTERFACE,				// Interface
		WKL_PROPID_STR_IPRANGE					// IP Adress Range zum Scannen
	};
	enum PROPID_I {							// INT Property IDs
		WKL_PROPID_INT_TOOL = 0					// Tool ID (0=Intf Parser; 1=Port Scanner)
	};


private:
	wxPropertyGridManager *wklGrid;
	wxPGId propIds[7];					// String Settings
	wxPGId propIdi[1];					// INT Settings
	int pageID;							// wxProgGrid Page ID
	void OnPropertyGridChange(			// Zeigt an, wenn eine Option geändert wurde
		wxPropertyGridEvent& event
		);

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
