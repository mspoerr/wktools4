#pragma once
#include "stdwx.h"
#include <wx/grid.h>

class DevGrp :
	public wxGrid
{
public:
	DevGrp(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~DevGrp(void);

	void unCheck(					// Checkbox Aus/Ein
		bool zuruecksetzen = false		// wenn true, dann AUS
		);
	bool getCheckStatus();			// Checkbox Status abfragen

private:
	// Any class wishing to process wxWindows events must use this macro
	bool chkStatus;
	virtual void DrawColLabel( wxDC& dc, int col );
	DECLARE_EVENT_TABLE()
};
