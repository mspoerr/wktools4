#include "class_tabelle.h"

BEGIN_EVENT_TABLE(Tabelle, wxGrid)
END_EVENT_TABLE()

Tabelle::Tabelle(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxGrid(parent, id, pos, size, style, name)
{

}

Tabelle::~Tabelle(void)
{

}


