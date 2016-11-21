#pragma once
#include "stdwx.h"

#include <wx/propgrid/manager.h>

#include <string>

class CWzrdPropGrid :
	public wxPanel
{
public:
	CWzrdPropGrid(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CWzrdPropGrid(void);
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

	enum PROPID_S {							// STRING Property IDs
		WZRD_PROPID_STR_IPRANGE,
		WZRD_PROPID_STR_WORKDIR,
		WZRD_PROPID_STR_USER,
		WZRD_PROPID_STR_LOPW,
		WZRD_PROPID_STR_ENPW,
		WZRD_PROPID_STR_PROF
	};
	enum PROPID_I {							// INT Property IDs
		WZRD_PROPID_INT_WHAT
	};

private:
	wxPropertyGridManager *wzrdGrid;
	wxPGId propIds[6];					// String Settings
	wxPGId propIdi[1];					// INT Settings
	int pageID;							// wxProgGrid Page ID
	wxWindow *parent;					// Parent Window zum Versenden für Nachrichten
	void OnPropertyGridChange(			// Zeigt an, wenn eine Option geändert wurde
		wxPropertyGridEvent& event
		);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
