#include "class_wzrdPropGrid.h"

BEGIN_EVENT_TABLE(CWzrdPropGrid, wxPanel)
EVT_PG_CHANGED(wxID_ANY, CWzrdPropGrid::OnPropertyGridChange)
END_EVENT_TABLE()

CWzrdPropGrid::CWzrdPropGrid(wxWindow* pt, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(pt, id, pos, size, style, name)
{
	parent = pt;

	wzrdGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);

	wzrdGrid->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);

	pageID = wzrdGrid->AddPage(wxT("Wizard Settings"));

	wxPGId rootID = wzrdGrid->Append(new wxPropertyCategory(wxT("Wizard"), wxT("wzrd")));

	// GLOBAL Settings
	wxPGId gId = wzrdGrid->Append(new wxPropertyCategory(wxT("Global Settings")));
	// Modus befüllen
	wxArrayString arrMod;
	arrMod.Add(wxT("Draw Network Map"));
	arrMod.Add(wxT("Save Config"));
	propIdi[WZRD_PROPID_INT_WHAT] = wzrdGrid->AppendIn(gId, new wxEnumProperty(wxT("Modus"), wxPG_LABEL, arrMod));

	propIds[WZRD_PROPID_STR_WORKDIR] = wzrdGrid->AppendIn(gId, new wxDirProperty(wxT("Work Directory"), wxPG_LABEL));
	propIds[WZRD_PROPID_STR_IPRANGE] = wzrdGrid->AppendIn(gId, new wxStringProperty(wxT("IP Range"), wxPG_LABEL, ""));

	propIds[WZRD_PROPID_STR_USER] = wzrdGrid->AppendIn(gId, new wxStringProperty(wxT("Username"), wxPG_LABEL, ""));
	propIds[WZRD_PROPID_STR_LOPW] = wzrdGrid->AppendIn(gId, new wxStringProperty(wxT("Login Password"), wxPG_LABEL, ""));
	wzrdGrid->SetPropertyAttribute(propIds[WZRD_PROPID_STR_LOPW], wxPG_STRING_PASSWORD, true);
	propIds[WZRD_PROPID_STR_ENPW] = wzrdGrid->AppendIn(gId, new wxStringProperty(wxT("Enable Password"), wxPG_LABEL, ""));
	wzrdGrid->SetPropertyAttribute(propIds[WZRD_PROPID_STR_ENPW], wxPG_STRING_PASSWORD, true);

	propIds[WZRD_PROPID_STR_PROF] = wzrdGrid->AppendIn(gId, new wxStringProperty(wxT("Profile Name"), wxPG_LABEL, ""));
	
	// Hilfe
	wzrdGrid->SetPropertyHelpString(rootID, "Colored Options are mandatory");
	wzrdGrid->SetPropertyHelpString(gId, "See Documentation Section x.x");
	wzrdGrid->SetPropertyHelpString(propIdi[WZRD_PROPID_INT_WHAT], 
		"Select the job you want to do.");
	wzrdGrid->SetPropertyHelpString(propIds[WZRD_PROPID_STR_WORKDIR], 
		"Directory where all temporary files stored and the result is saved.");
	wzrdGrid->SetPropertyHelpString(propIds[WZRD_PROPID_STR_IPRANGE], 
		"IP Address Range to scan.\nDefine Ranges with \"-\" and seperate entries with \";\".Only ranges and single IPs are supported.\nUsage example: 192.168.1.1;192.168.1.10-192.168.1.100;192.168.1.150-192.168.3.5");
	wzrdGrid->SetPropertyHelpString(propIds[WZRD_PROPID_STR_USER], 
		"Username to use when connecting to the devices");
	wzrdGrid->SetPropertyHelpString(propIds[WZRD_PROPID_STR_LOPW], 
		"Login Password");
	wzrdGrid->SetPropertyHelpString(propIds[WZRD_PROPID_STR_ENPW], 
		"Enable Password");
	wzrdGrid->SetPropertyHelpString(propIds[WZRD_PROPID_STR_PROF], 
		"Profile Name - This string is used as Profile Name for saving all tool settings, so they can be used again later.");

	wzrdGrid->SetPropertyCell(rootID, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	wzrdGrid->SetPropertyCell(gId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));

	wxPGId MyPropertyId = wzrdGrid->GetPropertyByName("wzrd");
	wzrdGrid->SetCaptionTextColour(MyPropertyId, wxColour(0,0,128));

	//setFarbe(true);

	int dbh = wzrdGrid->GetDescBoxHeight();

	//wxFont testfont(10, wxDEFAULT, wxNORMAL, wxNORMAL, false);
	//wzrdGrid->SetFont(testfont);

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(wzrdGrid, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();
}


CWzrdPropGrid::~CWzrdPropGrid(void)
{
}


void CWzrdPropGrid::einstellungenInit(int index, int wert)
{
	wzrdGrid->SetPropertyValue(propIdi[index], wert);
}


void CWzrdPropGrid::einstellungenInit(int index, std::string wert)
{
	wzrdGrid->SetPropertyValue(propIds[index], wert.c_str());
}


void CWzrdPropGrid::resetFont(wxFont newFont)
{
	wzrdGrid->SetFont(newFont);
}


int CWzrdPropGrid::leseIntSetting(int index)
{
	wxVariant var = wzrdGrid->GetPropertyValue(propIdi[index]);
	int ret = var.GetInteger();
	return ret;
}


std::string CWzrdPropGrid::leseStringSetting(int index)
{
	std::string ret = "";
	ret = wzrdGrid->GetPropertyValueAsString(propIds[index]);
	return ret;
}


int CWzrdPropGrid::getSplitterPos()
{
	wxPropertyGridPage *tempPage = wzrdGrid->GetPage(pageID);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CWzrdPropGrid::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	wzrdGrid->SetSplitterPosition(pos);
}


void CWzrdPropGrid::setFarbe(bool status)
{
	wxString farbe = "";
	if (status)
	{
		farbe = "#fffb92";
	}
	else
	{
		farbe = "#ffffff";
	}

	wzrdGrid->SetPropertyBackgroundColour(propIdi[WZRD_PROPID_INT_WHAT], wxColour(farbe));
	wzrdGrid->SetPropertyBackgroundColour(propIds[WZRD_PROPID_STR_WORKDIR], wxColour(farbe));
	wzrdGrid->SetPropertyBackgroundColour(propIds[WZRD_PROPID_STR_IPRANGE], wxColour(farbe));
	wzrdGrid->SetPropertyBackgroundColour(propIds[WZRD_PROPID_STR_USER], wxColour(farbe));
	wzrdGrid->SetPropertyBackgroundColour(propIds[WZRD_PROPID_STR_LOPW], wxColour(farbe));
	wzrdGrid->SetPropertyBackgroundColour(propIds[WZRD_PROPID_STR_ENPW], wxColour(farbe));
	wzrdGrid->SetPropertyBackgroundColour(propIds[WZRD_PROPID_STR_PROF], wxColour(farbe));
}


void CWzrdPropGrid::OnPropertyGridChange( wxPropertyGridEvent& event )
{
	//wxPGProperty *property = event.GetProperty();

	//// It may be NULL
	//if ( !property )
	//	return;

	//// Get name of changed property
	//const wxString& name = property->GetName();

	//if (name == "df")
	//{
	//	wxCommandEvent event(wkEVT_WKN_PROPGRIDCHANGE);
	//	event.SetInt(1);
	//	wxPostEvent(parent, event);
	//}
}


