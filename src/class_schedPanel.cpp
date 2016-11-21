#include "class_schedPanel.h"

#ifdef _WINDOWS_
#include <shellapi.h>
#else
#endif


BEGIN_EVENT_TABLE(SchedPanel, wxPanel)
	EVT_GRID_LABEL_LEFT_CLICK (SchedPanel::OnLabelLClick)
	EVT_GRID_CELL_RIGHT_CLICK (SchedPanel::OnCellRClick)
	EVT_GRID_CELL_LEFT_CLICK (SchedPanel::OnCellLClick)
	EVT_MENU(wk_ID_MENU_SHOWCONFIGFILE, SchedPanel::OnShowConfig)
	EVT_MENU(wk_ID_MENU_REMOVEITEM, SchedPanel::OnRemove)
	EVT_MENU(wk_ID_MENU_MOVEUP, SchedPanel::OnMoveUp)
	EVT_MENU(wk_ID_MENU_MOVEDOWN, SchedPanel::OnMoveDown)
END_EVENT_TABLE()

SchedPanel::SchedPanel(wxWindow* pt, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(pt, id, pos, size, style, name)
{
	schedAnsicht = new DevGrp(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	schedAnsicht->CreateGrid(0, 7);
	//schedAnsicht->EnableDragColMove(true);
	
	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(schedAnsicht, 0, wxALL|wxEXPAND, 0);

	row = 0;
	parent = pt;

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

SchedPanel::~SchedPanel(void)
{
}

// xmlInit
// Zum Initialisieren der Einstellungen und laden der Spaltenbreite für die DeviceGroup Spalten
bool SchedPanel::xmlInit(TiXmlDocument *document)
{
	// Grund Elemente für Wke
	TiXmlHandle hDoc(document);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWke;		// das Element, das auf Wke zeigt
	TiXmlElement *pElemColSize;	// das Element, das auf Spaltenbreite Einstellungen zeigt

	// Wke im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		return false;
	}


	pElemWke = pElem->FirstChildElement("Scheduler");
	pElemColSize = pElemWke->FirstChildElement("ColumnSettings");
	if (pElemColSize)
	{
		std::string colKey[] = {"Col1", "Col2", "Col3", "Col4", "Col5", "Col6", "Col7"};
		std::string label[] = {"", "State", "Name", "Tool", "Profile", "Time", "Repeat"};

		for (int i = 0; i < 7; i++)
		{
			int x = 0;
			pElemColSize->QueryIntAttribute(colKey[i], &x);
			schedAnsicht->SetColSize(i, x);
			schedAnsicht->SetColLabelValue(i, label[i].c_str());
		}
		schedAnsicht->SetColSize(0, 20);

		//schedAnsicht->SetColLabelValue(0, wxT(wxChar(9801),1));

		schedAnsicht->SetColFormatBool(0);
		schedAnsicht->SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
		schedAnsicht->SetRowLabelSize(30);

		for( int i = 0; i < schedAnsicht->GetNumberRows(); i++ )
		{
			schedAnsicht->SetCellRenderer(i, 0, new wxGridCellBoolRenderer);
			schedAnsicht->SetCellEditor  (i, 0, new wxGridCellBoolEditor);
		} 

		return true;
	}

	return false;
}


void SchedPanel::OnLabelLClick(wxGridEvent &event)
{
	if (event.GetCol() == 0)
	{
		schedAnsicht->unCheck();
		if (schedAnsicht->getCheckStatus())
		{
			for( int i = 0; i < schedAnsicht->GetNumberRows(); i++ )
			{
				schedAnsicht->SetCellValue(i, 0, "1");
			} 
		}
		else
		{
			for( int i = 0; i < schedAnsicht->GetNumberRows(); i++ )
			{
				schedAnsicht->SetCellValue("", i, 0);
			} 

		}
	}
	else if (event.GetRow() != -1)
	{
		wxCommandEvent evt(wkEVT_SCHED_ROWSELECT);
		evt.SetInt(event.GetRow());
		wxPostEvent(parent, evt);
		event.Skip();
	}
	else
	{
		event.Skip();
	}
	schedAnsicht->ForceRefresh();
}


void SchedPanel::OnCellRClick(wxGridEvent &event)
{
	wxPoint point = event.GetPosition();
	wxMenu *menu = new wxMenu;

	menu->Append(wk_ID_MENU_REMOVEITEM, _T("Remove Item"));
	menu->Append(wk_ID_MENU_MOVEUP, _T("Move UP"));
	menu->Append(wk_ID_MENU_MOVEDOWN, _T("Move DOWN"));

	row = event.GetRow();
	schedAnsicht->ClearSelection();
	schedAnsicht->SelectRow(row, true);
	PopupMenu(menu, point); 
}


void SchedPanel::OnCellLClick(wxGridEvent &event)
{
	if (event.GetCol() == 0)
	{
		wxString chkbxStat = schedAnsicht->GetCellValue(event.GetRow(), 0);
		if (chkbxStat != "")
		{
			schedAnsicht->SetCellValue("", event.GetRow(), 0);
		}
		else
		{
			schedAnsicht->SetCellValue(event.GetRow(), 0, "1");

		}
		schedAnsicht->ForceRefresh();
	}
	else
	{
		//row = event.GetRow();
		//schedAnsicht->ClearSelection();
		//schedAnsicht->SelectRow(row, true);
		event.Skip();
	}
}


void SchedPanel::OnShowConfig(wxCommandEvent &event)
{
	wxString confName = schedAnsicht->GetCellValue(row, 5);

#ifdef _WINDOWS_
	HINSTANCE hInst = ShellExecute(NULL, _T("open"), confName, NULL, NULL, SW_SHOWNORMAL);
#else
	wxString cmd = "xdg-open " + confName;
	system(cmd);
#endif
}


void SchedPanel::OnRemove(wxCommandEvent &event)
{
	wxCommandEvent evt(wkEVT_SCHED_ROWDELETE);
	evt.SetInt(row);
	wxPostEvent(parent, evt);
}


void SchedPanel::OnMoveUp(wxCommandEvent &event)
{
	if (row != 0)
	{
		speichereZeile();

		schedAnsicht->InsertRows(row-1, 1, true);
		for (int i = 0; i < 7; i++)
		{
			schedAnsicht->SetCellValue(row-1, i, zeilenDaten[i]);
		}
		schedAnsicht->DeleteRows(row+1);
	}
}


void SchedPanel::OnMoveDown(wxCommandEvent &event)
{
	if (row != schedAnsicht->GetNumberRows()-1)
	{
		speichereZeile();

		schedAnsicht->InsertRows(row+2, 1, true);
		for (int i = 0; i < 7; i++)
		{
			schedAnsicht->SetCellValue(row+2, i, zeilenDaten[i]);
		}
		schedAnsicht->DeleteRows(row);
	}
}

void SchedPanel::zeileHinzu()
{
	schedAnsicht->AppendRows();
	schedAnsicht->SetCellRenderer(schedAnsicht->GetNumberRows()-1, 0, new wxGridCellBoolRenderer);
	schedAnsicht->SetCellEditor(schedAnsicht->GetNumberRows()-1, 0, new wxGridCellBoolEditor);
	schedAnsicht->SetReadOnly(schedAnsicht->GetNumberRows()-1, wk_SCHED_TABLE_STATE, true);
}


void SchedPanel::speichereZeile()
{
	zeilenDaten.clear();
	for (int i = 0; i < 7; i++)
	{
		zeilenDaten.push_back(schedAnsicht->GetCellValue(row, i));
	}

}