#pragma once

#include "stdwx.h"
#include "wx/html/htmlwin.h"


class WkHtml : public wxHtmlWindow
{
public:
	WkHtml(wxWindow *parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxHW_SCROLLBAR_AUTO, const wxString& name = _T("wktools"));
	void OnLinkClicked(const wxHtmlLinkInfo& link);
};


