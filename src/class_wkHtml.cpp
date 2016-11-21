#include "class_wkHtml.h"

WkHtml::WkHtml(wxWindow *parent, wxWindowID id, const wxPoint& pos,
						   const wxSize& size, long style, const wxString& name)
						   : wxHtmlWindow(parent, id, pos, size, style, name) 
{
	   //-------------------------------------------------------------------------
	   // use default wxHtmlWindow constructor
	   //-------------------------------------------------------------------------
}

void WkHtml::OnLinkClicked(const wxHtmlLinkInfo& link) 
{
	//-------------------------------------------------------------------------
	// If help doc link is external, launch a browser.
	// Replace the condition and function to invoke external browser as required.
	//-------------------------------------------------------------------------


	if (link.GetHref().StartsWith(_T("http://")))
		wxLaunchDefaultBrowser(link.GetHref());
	else
		wxHtmlWindow::OnLinkClicked(link);
}

