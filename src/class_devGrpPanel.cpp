#include "class_devGrpPanel.h"

#ifdef _WINDOWS_
#include <shellapi.h>
#else
#endif


BEGIN_EVENT_TABLE(DevGrpPanel, wxPanel)
	EVT_GRID_LABEL_LEFT_CLICK (DevGrpPanel::OnLabelLClick)
	EVT_GRID_CELL_RIGHT_CLICK (DevGrpPanel::OnCellRClick)
	EVT_GRID_CELL_LEFT_CLICK (DevGrpPanel::OnCellLClick)
	EVT_MENU(wk_ID_MENU_SHOWCONFIGFILE, DevGrpPanel::OnShowConfig)
	EVT_MENU(wk_ID_MENU_REMOVEITEM, DevGrpPanel::OnRemove)
	EVT_MENU(wk_ID_MENU_ADDITEM_AFTER, DevGrpPanel::OnAddA)
	EVT_MENU(wk_ID_MENU_ADDITEM_BEF, DevGrpPanel::OnAddB)
END_EVENT_TABLE()

DevGrpPanel::DevGrpPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	devGrpAnsicht = new DevGrp(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	devGrpAnsicht->CreateGrid(2, 12);
	devGrpAnsicht->SetColMinimalAcceptableWidth(0);
	
	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(devGrpAnsicht, 0, wxALL|wxEXPAND, 0);

	row = 0;

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

DevGrpPanel::~DevGrpPanel(void)
{
	// Grund Elemente für Wke
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWke;		// das Element, das auf Wke zeigt
	TiXmlElement *pElemColSize;	// das Element, das auf Spaltenbreite Einstellungen zeigt

	// Wke im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (pElem)
	{
		pElemWke = pElem->FirstChildElement("wke");
		pElemColSize = pElemWke->FirstChildElement("DG_COL");
		if (pElemColSize)
		{
			std::string colKey[] = {"Col1", "Col2", "Col3", "Col4", 
				"Col5", "Col6", "Col7", "Col8", "Col9", "Col10", "Col11", "Col12"};

			for (int i = 0; i < 12; i++)
			{
				int x = 0;
				x = devGrpAnsicht->GetColSize(i);
				pElemColSize->SetAttribute(colKey[i], x);
			}
		}
	}
}

// xmlInit
// Zum Initialisieren der Einstellungen und laden der Spaltenbreite für die DeviceGroup Spalten
bool DevGrpPanel::xmlInit(TiXmlDocument *document)
{
	// Grund Elemente für Wke
	doc = document;
	TiXmlHandle hDoc(doc);

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


	pElemWke = pElem->FirstChildElement("wke");
	pElemColSize = pElemWke->FirstChildElement("DG_COL");
	if (pElemColSize)
	{
		std::string colKey[] = {"Col1", "Col2", "Col3", "Col4", 
			"Col5", "Col6", "Col7", "Col8", "Col9", "Col10", "Col11", "Col12"};
		std::string label[] = {"", "Hostname", "Username", "Login PW", "Enable PW", "Config File",
			"Protocol", "Port", "Terminal Server", "Multihop Command", "Remark", "Remark2"};

		for (int i = 0; i < 12; i++)
		{
			int x = 40;
			pElemColSize->QueryIntAttribute(colKey[i], &x);
			if (x > 400)
			{
				x = 40;
			}
			devGrpAnsicht->SetColSize(i, x);
			devGrpAnsicht->SetColLabelValue(i, label[i].c_str());
		}
//		devGrpAnsicht->SetColSize(0, 20);

		//devGrpAnsicht->SetColLabelValue(0, wxT(wxChar(9801),1));

		devGrpAnsicht->SetColFormatBool(0);
		devGrpAnsicht->SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
		devGrpAnsicht->SetRowLabelSize(30);

		for( int i = 0; i < devGrpAnsicht->GetNumberRows(); i++ )
		{
			devGrpAnsicht->SetCellRenderer(i, 0, new wxGridCellBoolRenderer);
			devGrpAnsicht->SetCellEditor  (i, 0, new wxGridCellBoolEditor);
		} 

		return true;
	}

	return false;
}


void DevGrpPanel::OnLabelLClick(wxGridEvent &event)
{
	if (event.GetCol() == 0)
	{
		devGrpAnsicht->unCheck();
		if (devGrpAnsicht->getCheckStatus())
		{
			for( int i = 0; i < devGrpAnsicht->GetNumberRows(); i++ )
			{
				devGrpAnsicht->SetCellValue(i, 0, "1");
			} 
		}
		else
		{
			for( int i = 0; i < devGrpAnsicht->GetNumberRows(); i++ )
			{
				devGrpAnsicht->SetCellValue("", i, 0);
			} 
		}
		devGrpAnsicht->ForceRefresh();
	}
	else
	{
		event.Skip();
	}
}


void DevGrpPanel::OnCellRClick(wxGridEvent &event)
{
	wxPoint point = event.GetPosition();
	wxMenu *menu = new wxMenu;
	menu->Append(wk_ID_MENU_SHOWCONFIGFILE, _T("Show Config File"));
	menu->AppendSeparator();
	menu->Append(wk_ID_MENU_REMOVEITEM, _T("Remove Item"));
	menu->Append(wk_ID_MENU_ADDITEM_AFTER, _T("Add new item after this line"));
	menu->Append(wk_ID_MENU_ADDITEM_BEF, _T("Add new item before this line"));

	row = event.GetRow();
	devGrpAnsicht->ClearSelection();
	devGrpAnsicht->SelectRow(row, true);
	PopupMenu(menu, point); 
}


void DevGrpPanel::OnCellLClick(wxGridEvent &event)
{
	if (event.GetCol() == 0)
	{
		wxString chkbxStat = devGrpAnsicht->GetCellValue(event.GetRow(), 0);
		if (chkbxStat != "")
		{
			devGrpAnsicht->SetCellValue("", event.GetRow(), 0);
		}
		else
		{
			devGrpAnsicht->SetCellValue(event.GetRow(), 0, "1");

		}
		devGrpAnsicht->ForceRefresh();
	}
	else
	{
		event.Skip();
	}
}


void DevGrpPanel::OnShowConfig(wxCommandEvent &event)
{
	wxString confName = devGrpAnsicht->GetCellValue(row, 5);

#ifdef _WINDOWS_
	HINSTANCE hInst = ShellExecute(NULL, _T("open"), confName, NULL, NULL, SW_SHOWNORMAL);
#else
	wxString cmd = "xdg-open " + confName;
	system(cmd);
#endif
}


void DevGrpPanel::OnRemove(wxCommandEvent &event)
{
	wxArrayInt rows = devGrpAnsicht->GetSelectedRows();
	devGrpAnsicht->DeleteRows(rows[0]);
}


void DevGrpPanel::OnAddA(wxCommandEvent &event)
{
	wxArrayInt rows = devGrpAnsicht->GetSelectedRows();
	devGrpAnsicht->InsertRows(rows[0]+1, 1);
	devGrpAnsicht->SetCellRenderer(rows[0]+1, 0, new wxGridCellBoolRenderer);
	devGrpAnsicht->SetCellEditor  (rows[0]+1, 0, new wxGridCellBoolEditor);
}


void DevGrpPanel::OnAddB(wxCommandEvent &event)
{
	wxArrayInt rows = devGrpAnsicht->GetSelectedRows();
	devGrpAnsicht->InsertRows(rows[0], 1);
	devGrpAnsicht->SetCellRenderer(rows[0], 0, new wxGridCellBoolRenderer);
	devGrpAnsicht->SetCellEditor  (rows[0], 0, new wxGridCellBoolEditor);
}

void DevGrpPanel::zeileHinzu()
{
	devGrpAnsicht->AppendRows();
	devGrpAnsicht->SetCellRenderer(devGrpAnsicht->GetNumberRows()-1, 0, new wxGridCellBoolRenderer);
	devGrpAnsicht->SetCellEditor  (devGrpAnsicht->GetNumberRows()-1, 0, new wxGridCellBoolEditor);
}


void DevGrpPanel::setVisible(bool visible)
{
	if (!visible)
	{
		colSize2 = devGrpAnsicht->GetColSize(2);
		colSize3 = devGrpAnsicht->GetColSize(3);
		colSize4 = devGrpAnsicht->GetColSize(4);
		devGrpAnsicht->SetColSize(2, 0);
		devGrpAnsicht->SetColSize(3, 0);
		devGrpAnsicht->SetColSize(4, 0);
	}
	else
	{
		if (colSize2 == 0)
		{
			colSize2 = 40;
		}
		if (colSize3 == 0)
		{
			colSize3 = 40;
		}
		if (colSize4 == 0)
		{
			colSize4 = 40;
		}
		if (colSize2 > 150)
		{
			colSize2 = 40;
		}
		if (colSize3 > 150)
		{
			colSize3 = 40;
		}
		if (colSize4 > 150)
		{
			colSize4 = 40;
		}

		devGrpAnsicht->SetColSize(2, colSize2);
		devGrpAnsicht->SetColSize(3, colSize3);
		devGrpAnsicht->SetColSize(4, colSize4);
	}
	devGrpAnsicht->ForceRefresh();

	
}