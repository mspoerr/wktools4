#pragma once
#include "stdwx.h"
#include "class_devGrp.h"
#include <wx/grid.h>

#include <vector>

class SchedPanel :
	public wxPanel
{
public:
	SchedPanel(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~SchedPanel(void);

	DevGrp *schedAnsicht;				// Zeiger auf die DeviceGroup Anzeige
	bool xmlInit(						// Initialisieren der XML Settings
		TiXmlDocument *document				// XML Doc von wktools.xml
		);
	void zeileHinzu();					// Zeile hinzufügen

	enum SPALTEN {						// Spaltennummern
		wk_SCHED_TABLE_CHECK = 0,
		wk_SCHED_TABLE_STATE,
		wk_SCHED_TABLE_NAME,
		wk_SCHED_TABLE_TOOL,
		wk_SCHED_TABLE_PROFILE,
		wk_SCHED_TABLE_TIME,
		wk_SCHED_TABLE_REPEAT
	};

private:
	void OnLabelLClick(wxGridEvent &event);
	void OnCellRClick(wxGridEvent &event);
	void OnCellLClick(wxGridEvent &event);
	void OnShowConfig(wxCommandEvent &event);
	void OnRemove(wxCommandEvent &event);
	void OnMoveUp(wxCommandEvent &event);
	void OnMoveDown(wxCommandEvent &event);

	void speichereZeile();					// Zeilendaten werden gesichert

	int row;
	wxWindow *parent;					// Parent Window zum Versenden für Nachrichten

	std::vector<wxString> zeilenDaten;		// gespeicherte ZeilenDaten

	enum MENUENTRIES {					// Context Menü Einträge
		wk_ID_MENU_SHOWCONFIGFILE,
		wk_ID_MENU_REMOVEITEM,
		wk_ID_MENU_MOVEUP,
		wk_ID_MENU_MOVEDOWN
	};
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
