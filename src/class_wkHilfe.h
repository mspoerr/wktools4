#pragma once
#include "stdwx.h"
#include "class_wkHtml.h"

class CWkHilfe :
	public wxPanel
{
public:
	CWkHilfe(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CWkHilfe(void);
	void ladeSeite(wxString seite);

private:
	// Any class wishing to process wxWindows events must use this macro
	WkHtml *anzeige;
	DECLARE_EVENT_TABLE()
};
