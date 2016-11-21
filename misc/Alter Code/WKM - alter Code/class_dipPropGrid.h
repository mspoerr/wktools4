#pragma once
#include "stdwx.h"

#include <wx/propgrid/manager.h>

#include <string>

class CDipPropGrid :
	public wxPanel
{
public:
	CDipPropGrid(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CDipPropGrid(void);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		std::string wert								// Wert des Objektes
		);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		int wert								// Wert des Objektes
		);
	int leseIntSetting(						// zum Auslesen von INT Werten
		int index								// Index des Objektes
		);
	std::string leseStringSetting(				// zum Auslesen von STRING Werten
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
		DIP_PROPID_STR_SDIR = 0,				// Search Directory
		DIP_PROPID_STR_ODIR,					// Output Directory
		DIP_PROPID_STR_LOGFILE,					// Log File
		DIP_PROPID_STR_OFILE,					// Output File
		DIP_PROPID_STR_PATTERN					// Search Pattern
	};
	enum PROPID_I {							// INT Property IDs
		DIP_PROPID_INT_SNMPLOCCOL = 0,			// SNMP Location Column
		DIP_PROPID_INT_TOOL,
		DIP_PROPID_INT_APPEND,
		DIP_PROPID_INT_COL1,
		DIP_PROPID_INT_COL2,
		DIP_PROPID_INT_COL3,
		DIP_PROPID_INT_COL4,
		DIP_PROPID_INT_COL5,
		DIP_PROPID_INT_COL6,
		DIP_PROPID_INT_COL7,
		DIP_PROPID_INT_COL8,
		DIP_PROPID_INT_COL9,
		DIP_PROPID_INT_COL10,
		DIP_PROPID_INT_COL11,
		DIP_PROPID_INT_DESCRIPTION,
		DIP_PROPID_INT_CHASSISONLY
	};

private:
	wxPropertyGridManager *dipGrid;
	wxPGId propIds[5];					// String Settings
	wxPGId propIdi[16];					// INT Settings
	int pageID;							// wxProgGrid Page ID
	void OnPropertyGridChange(			// Zeigt an, wenn eine Option geändert wurde
		wxPropertyGridEvent& event
		);

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
