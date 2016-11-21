#include "class_wklPropGrid.h"


BEGIN_EVENT_TABLE(CWklPropGrid, wxPanel)
EVT_PG_CHANGED(wxID_ANY, CWklPropGrid::OnPropertyGridChange)
END_EVENT_TABLE()

CWklPropGrid::CWklPropGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	wklGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);

	wklGrid->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);


	pageID = wklGrid->AddPage(wxT("IP List Settings"));

	wxPGId rootID = wklGrid->Append(new wxPropertyCategory(wxT("IP List"), wxT("wkl")));

	// Tool
	wxArrayString toolArrMod;
	toolArrMod.Add(wxT("Configfile Parser"));
	toolArrMod.Add(wxT("Portscanner"));
	propIdi[WKL_PROPID_INT_TOOL] = wklGrid->AppendIn(rootID, new wxEnumProperty(wxT("Tool Option"), wxPG_LABEL, toolArrMod));
	
	// Config/Interface Parser Options
	wxPGId ciopt = wklGrid->Append(new wxPropertyCategory(wxT("Config/Interface Parser Options")));
	propIds[WKL_PROPID_STR_SDIR] = wklGrid->AppendIn(ciopt, new wxDirProperty(wxT("Search Directory"), wxPG_LABEL, ::wxGetUserHome()));
	propIds[WKL_PROPID_STR_PATTERN] = wklGrid->AppendIn(ciopt, new wxStringProperty(wxT("Search Pattern"), wxPG_LABEL, ""));

	// Interface
	wxArrayString intfArrMod;
	wxString interfaces[] = {"FastEthernet", "Ethernet", "GigabitEthernet", "Loopback", "Serial", "Tunnel",
		"Multilink", "Async", "BVI", "Vlan", "POS", "ATM", "BRI", "Dialer", "FDDI", "TokenRing"};
	int intArrayAnzahl = 16;	// Wie viele Interfaces sind bekannt

	for (int i = 0; i < intArrayAnzahl; i++)
	{
		intfArrMod.Add(interfaces[i]);
	}
	propIds[WKL_PROPID_STR_INTERFACE] = wklGrid->AppendIn(ciopt, new wxEditEnumProperty(wxT("Interface"), wxPG_LABEL, intfArrMod));
	propIds[WKL_PROPID_STR_INTFNR] = wklGrid->AppendIn(ciopt, new wxStringProperty(wxT("Interface number"), wxPG_LABEL, ""));

	// PortScanner Options
	wxPGId psopt = wklGrid->Append(new wxPropertyCategory(wxT("Port Scanner Options")));
	propIds[WKL_PROPID_STR_IPRANGE] = wklGrid->AppendIn(psopt, new wxStringProperty(wxT("IP Address List"), wxPG_LABEL, ""));

	// Output Options
	wxPGId oopt = wklGrid->Append(new wxPropertyCategory(wxT("Output Options")));
	propIds[WKL_PROPID_STR_ODIR] = wklGrid->AppendIn(oopt, new wxDirProperty(wxT("Output Directory"), wxPG_LABEL, ::wxGetUserHome()));
	propIds[WKL_PROPID_STR_LOGFILE] = wklGrid->AppendIn(oopt, new wxFileProperty(wxT("LogFile"), wxPG_LABEL));
	

	//// Hilfe
	wklGrid->SetPropertyHelpString(rootID, "Colored Options are mandatory");
	wklGrid->SetPropertyHelpString(psopt, "Port Scanner Options");
	wklGrid->SetPropertyHelpString(ciopt, "Options for Interface Parser tool - Search IP addresses of specific interfaces in config files.");
	wklGrid->SetPropertyHelpString(oopt, "Output Options");
	wklGrid->SetPropertyHelpString(propIds[WKL_PROPID_STR_SDIR], 
		"Search Directory");
	wklGrid->SetPropertyHelpString(propIds[WKL_PROPID_STR_PATTERN], 
		"Only files with filenames containing this pattern will be included for parsing. If using more than one search pattern, use \";\" as seperator. Sample: pattern1;pattern2;pattern3;... ");
	wklGrid->SetPropertyHelpString(propIds[WKL_PROPID_STR_ODIR], 
		"Directory, where the result will be saved to.");
	wklGrid->SetPropertyHelpString(propIds[WKL_PROPID_STR_LOGFILE], 
		"");
	wklGrid->SetPropertyHelpString(propIds[WKL_PROPID_STR_INTERFACE], 
		"The name of the interface from which the IP should be collected.");
	wklGrid->SetPropertyHelpString(propIds[WKL_PROPID_STR_IPRANGE], 
		"IP Address Range to scan.\nDefine Ranges with \"-\" and seperate entries with \";\".Only ranges and single IPs are supported.\nUsage example: 192.168.1.1;192.168.1.10-192.168.1.100;192.168.1.150-192.168.3.5");
	wklGrid->SetPropertyHelpString(propIdi[WKL_PROPID_INT_TOOL], 
		"Select the tool to use.");

	wklGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);

	//setFarbe(true);
	wklGrid->SetCaptionTextColour(rootID, wxColour("#ffffff"));

	wxPGId MyPropertyId = wklGrid->GetPropertyByName("wkl");
	wklGrid->SetCaptionTextColour(MyPropertyId, wxColour(0,0,128));

	wklGrid->SetPropertyCell(rootID, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	wklGrid->SetPropertyCell(ciopt, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wklGrid->SetPropertyCell(psopt, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wklGrid->SetPropertyCell(oopt, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));

	int dbh = wklGrid->GetDescBoxHeight();

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(wklGrid, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

CWklPropGrid::~CWklPropGrid(void)
{
}


void CWklPropGrid::einstellungenInit(int index, int wert)
{
	wklGrid->SetPropertyValue(propIdi[index], wert);
}


void CWklPropGrid::einstellungenInit(int index, std::string wert)
{
	wklGrid->SetPropertyValue(propIds[index], wert.c_str());
}


void CWklPropGrid::resetFont(wxFont newFont)
{
	wklGrid->SetFont(newFont);
}


std::string CWklPropGrid::leseStringSetting(int index)
{
	std::string ret = "";
	ret = wklGrid->GetPropertyValueAsString(propIds[index]);
	return ret;
}


int CWklPropGrid::leseIntSetting(int index)
{
	wxVariant var = wklGrid->GetPropertyValue(propIdi[index]);
	int ret = var.GetInteger();
	return ret;
}


int CWklPropGrid::getSplitterPos()
{
	wxPropertyGridPage *tempPage = wklGrid->GetPage(pageID);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CWklPropGrid::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	wklGrid->SetSplitterPosition(pos);
}


void CWklPropGrid::OnPropertyGridChange( wxPropertyGridEvent& event )
{
	//wxPGProperty *property = event.GetProperty();

	// It may be NULL
	//if ( !property )
	//	return;

	// Get name of changed property
	// const wxString& name = property->GetName();

	// Get resulting value
	// wxVariant value = property->GetValue();

	enableDisable();
}


void CWklPropGrid::enableDisable()
{
	bool intfDisable = true;		// false wenn Tool == Interface Parser
	if (leseIntSetting(WKL_PROPID_INT_TOOL))
	{
		// Interface Parser
		intfDisable = false;
	}
	wklGrid->EnableProperty(propIds[WKL_PROPID_STR_SDIR], intfDisable);
	wklGrid->EnableProperty(propIds[WKL_PROPID_STR_INTFNR], intfDisable);
	wklGrid->EnableProperty(propIds[WKL_PROPID_STR_PATTERN], intfDisable);
	wklGrid->EnableProperty(propIds[WKL_PROPID_STR_INTERFACE], intfDisable);

	wklGrid->EnableProperty(propIds[WKL_PROPID_STR_IPRANGE], !intfDisable);
}


void CWklPropGrid::setFarbe(bool status)
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

	wklGrid->SetPropertyBackgroundColour(propIds[WKL_PROPID_STR_SDIR], wxColour(farbe));
	wklGrid->SetPropertyBackgroundColour(propIds[WKL_PROPID_STR_ODIR], wxColour(farbe));
	wklGrid->SetPropertyBackgroundColour(propIds[WKL_PROPID_STR_INTERFACE], wxColour(farbe));
	wklGrid->SetPropertyBackgroundColour(propIds[WKL_PROPID_STR_INTFNR], wxColour(farbe));
	wklGrid->SetPropertyBackgroundColour(propIds[WKL_PROPID_STR_IPRANGE], wxColour(farbe));
}