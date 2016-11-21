#pragma once
#include "stdwx.h"
#include <wx/grid.h>

class Tabelle :
	public wxGrid
{
public:
	Tabelle(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~Tabelle(void);

private:
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
