#include "class_tabellePanel.h"

BEGIN_EVENT_TABLE(TabellePanel, wxPanel)
	EVT_GRID_CELL_RIGHT_CLICK (TabellePanel::OnCellRClick)
	EVT_MENU(wk_ID_MENU_REMOVEITEM, TabellePanel::OnRemove)
	EVT_MENU(wk_ID_MENU_ADDITEM_AFTER, TabellePanel::OnAddA)
	EVT_MENU(wk_ID_MENU_ADDITEM_BEF, TabellePanel::OnAddB)
END_EVENT_TABLE()

TabellePanel::TabellePanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	tabellenAnsicht = new Tabelle(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	
	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(tabellenAnsicht, 0, wxALL|wxEXPAND, 0);
	
	tabellenAnsicht->CreateGrid(1, 1);
	tabellenAnsicht->SetColSize(0, 70);
	tabellenAnsicht->SetRowLabelSize(30);

	row = 0;

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

TabellePanel::~TabellePanel(void)
{
}


void TabellePanel::OnCellRClick(wxGridEvent &event)
{
	//wxPoint point = event.GetPosition();
	//wxMenu *menu = new wxMenu;
	//menu->Append(wk_ID_MENU_REMOVEITEM, _T("Remove Item"));
	//menu->Append(wk_ID_MENU_ADDITEM_AFTER, _T("Add new item after this line"));
	//menu->Append(wk_ID_MENU_ADDITEM_BEF, _T("Add new item before this line"));

	//row = event.GetRow();
	//tabellenAnsicht->ClearSelection();
	//tabellenAnsicht->SelectRow(row, true);
	//PopupMenu(menu, point); 
}



void TabellePanel::OnRemove(wxCommandEvent &event)
{
	wxArrayInt rows = tabellenAnsicht->GetSelectedRows();
	tabellenAnsicht->DeleteRows(rows[0]);
}


void TabellePanel::OnAddA(wxCommandEvent &event)
{
	wxArrayInt rows = tabellenAnsicht->GetSelectedRows();
	tabellenAnsicht->InsertRows(rows[0]+1, 1);
}


void TabellePanel::OnAddB(wxCommandEvent &event)
{
	wxArrayInt rows = tabellenAnsicht->GetSelectedRows();
	tabellenAnsicht->InsertRows(rows[0], 1);
}

void TabellePanel::zeileHinzu()
{
	tabellenAnsicht->AppendRows();
}

void TabellePanel::spalteHinzu()
{
	tabellenAnsicht->AppendCols();
}


void TabellePanel::createList()
{
	tabellenAnsicht->DeleteRows(0, tabellenAnsicht->GetNumberRows()-1);
	tabellenAnsicht->DeleteCols(0, tabellenAnsicht->GetNumberCols()-1);
	tabellenAnsicht->ClearGrid();
	wxColour hintergrund("#ffffff");
	tabellenAnsicht->SetCellBackgroundColour(hintergrund , 0, 0);
	tabellenAnsicht->ForceRefresh();
}