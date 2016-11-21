#pragma once
#include "stdwx.h"

#include <wx/propgrid/manager.h>

#include <string>

class CSchedPropGrid :
	public wxPanel
{
public:
	CSchedPropGrid(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CSchedPropGrid(void);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		std::string wert								// Wert des Objektes
		);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		int wert								// Wert des Objektes
		);
	std::string leseStringSetting(				// zum Auslesen von STRING Werten
		int index								// Index des Objektes
		);
	int leseIntSetting(					// zum Auslesen von INT Werten
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
	void ladeProfile(						// Zum Laden von Profilen, wenn das Tool geändert wurde
		wxArrayString profs						// Neue Profilnamen zum Laden
		);

	enum PROPID_S {							// STRING Property IDs
		SCHED_PROPID_STR_SCHEDFILE = 0,			// Scheduler File
		SCHED_PROPID_STR_DATE,
		SCHED_PROPID_STR_TIME,
		SCHED_PROPID_STR_USER,
		SCHED_PROPID_STR_PASS,
		SCHED_PROPID_STR_PROFILE,
		SCHED_PROPID_STR_NAME,
		SCHED_PROPID_STR_TOOL,
		SCHED_PROPID_STR_REC
	};
	enum PROPID_I {							// INT Property IDs
		SCHED_PROPID_INT_COL1 = 0,
		SCHED_PROPID_INT_COL2,
		SCHED_PROPID_INT_COL3,
		SCHED_PROPID_INT_COL4,
		SCHED_PROPID_INT_COL5,
		SCHED_PROPID_INT_COL6,
		SCHED_PROPID_INT_COL7,
		SCHED_PROPID_INT_COL8,
	};

private:
	wxWindow *parent;					// Parent Window zum Versenden für Nachrichten
	wxPropertyGridManager *schedGrid;
	wxPGId propIds[7];					// String Settings
	wxPGId propIdi[10];					// INT Settings
	int pageID;							// wxProgGrid Page ID
	void OnPropertyGridChange(			// Zeigt an, wenn eine Option geändert wurde
		wxPropertyGridEvent& event
		);
	void OnPropertyChangeButtonStyle(	// Zum Ändern des Button Layouts vom "Name" Property
		wxPropertyGridEvent& event
		);
	void OnNeuerEintrag(wxCommandEvent &event);

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
