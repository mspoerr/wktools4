#include "class_wkHilfe.h"

#include <wx/fs_inet.h>

BEGIN_EVENT_TABLE(CWkHilfe, wxPanel)
END_EVENT_TABLE()

CWkHilfe::CWkHilfe(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	wxFileSystem::AddHandler(new wxInternetFSHandler); 

	anzeige = new WkHtml(this, wxID_ANY, wxDefaultPosition);
	anzeige->SetWindowStyle(anzeige->GetWindowStyle()|wxBORDER);

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(anzeige, 0, wxALL|wxEXPAND, 0);


	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

CWkHilfe::~CWkHilfe(void)
{
}


void CWkHilfe::ladeSeite(wxString seite)
{
	anzeige->SetPage(seite);
}

