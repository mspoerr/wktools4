#pragma once
#include "stdwx.h"
#include "class_tabelle.h"
#include <wx/grid.h>

class TabellePanel :
	public wxPanel
{
public:
	TabellePanel(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~TabellePanel(void);

	Tabelle *tabellenAnsicht;			// Zeiger auf die Tabellen Anzeige
	void zeileHinzu();					// Zeile hinzufügen
	void spalteHinzu();					// Spalte hinzufügen
	void createList();					// Liste erstellen

private:
	void OnCellRClick(wxGridEvent &event);
	void OnRemove(wxCommandEvent &event);
	void OnAddA(wxCommandEvent &event);
	void OnAddB(wxCommandEvent &event);

	int row;

	enum MENUENTRIES {					// Context Menü Einträge
		wk_ID_MENU_REMOVEITEM,
		wk_ID_MENU_ADDITEM_AFTER,
		wk_ID_MENU_ADDITEM_BEF
	};
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
