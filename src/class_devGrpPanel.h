#pragma once
#include "stdwx.h"
#include "class_devGrp.h"
#include <wx/grid.h>

class DevGrpPanel :
	public wxPanel
{
public:
	DevGrpPanel(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~DevGrpPanel(void);

	DevGrp *devGrpAnsicht;				// Zeiger auf die DeviceGroup Anzeige
	bool xmlInit(						// Initialisieren der XML Settings
		TiXmlDocument *document				// XML Doc von wktools.xml
		);
	void zeileHinzu();					// Zeile hinzufügen
	void setVisible(					// User/Passwort Spalten ausblenden
		bool visible						// true: sichtbar; false: ausblenden
		);

private:
	void OnLabelLClick(wxGridEvent &event);
	void OnCellRClick(wxGridEvent &event);
	void OnCellLClick(wxGridEvent &event);
	void OnShowConfig(wxCommandEvent &event);
	void OnRemove(wxCommandEvent &event);
	void OnAddA(wxCommandEvent &event);
	void OnAddB(wxCommandEvent &event);

	int row;
	int colSize2;						// Size of Column 2
	int colSize3;						// Size of Column 3
	int colSize4;						// Size of Column 4
	
	TiXmlDocument *doc;

	enum MENUENTRIES {					// Context Menü Einträge
		wk_ID_MENU_SHOWCONFIGFILE,
		wk_ID_MENU_REMOVEITEM,
		wk_ID_MENU_ADDITEM_AFTER,
		wk_ID_MENU_ADDITEM_BEF
	};
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
