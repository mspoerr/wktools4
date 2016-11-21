
#include "class_wkmPropGrid.h"
#ifdef WKTOOLS_MAPPER


BEGIN_EVENT_TABLE(CWkmPropGrid, wxPanel)
EVT_PG_CHANGED(wxID_ANY, CWkmPropGrid::OnPropertyGridChange)
END_EVENT_TABLE()

CWkmPropGrid::CWkmPropGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	wkmGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);

	wkmGrid->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);


	pageID = wkmGrid->AddPage(wxT("Mapper Settings"));

	wxPGId rootID = wkmGrid->Append(new wxPropertyCategory(wxT("Mapper"), wxT("wkm")));

	// Input
	wxPGId ipt = wkmGrid->Append(new wxPropertyCategory(wxT("Input")));
	propIds[WKM_PROPID_STR_SDIR] = wkmGrid->AppendIn(ipt, new wxDirProperty(wxT("Search Directory"), wxPG_LABEL, ::wxGetUserHome()));
	propIds[WKM_PROPID_STR_PATTERN] = wkmGrid->AppendIn(ipt, new wxStringProperty(wxT("Search Pattern"), wxPG_LABEL, ""));
	propIds[WKM_PROPID_STR_HOSTFILE] = wkmGrid->AppendIn(ipt, new wxFileProperty(wxT("File with Import data"), wxPG_LABEL, ""));
	propIdi[WKM_PROPID_INT_IMPORT] = wkmGrid->AppendIn(ipt, new wxBoolProperty(wxT("Import"), wxPG_LABEL, false));
	propIdi[WKM_PROPID_INT_IMP_ES] = wkmGrid->AppendIn(ipt, new wxBoolProperty(wxT("Import EndSystems"), wxPG_LABEL, false));
	propIdi[WKM_PROPID_INT_DNS_RESOLUTION] = wkmGrid->AppendIn(ipt, new wxBoolProperty(wxT("Reverse Lookup for End Systems"), wxPG_LABEL, false));

	// Output
	wxPGId opt = wkmGrid->Append(new wxPropertyCategory(wxT("Output")));
	propIds[WKM_PROPID_STR_OFILE] = wkmGrid->AppendIn(opt, new wxFileProperty(wxT("Output File"), wxPG_LABEL));
	propIds[WKM_PROPID_STR_LOGFILE] = wkmGrid->AppendIn(opt, new wxFileProperty(wxT("LogFile"), wxPG_LABEL));
	wxPGId Ooptis = wkmGrid->Append(new wxPropertyCategory(wxT("Visio Output - Options")));
	propIdi[WKM_PROPID_INT_L2] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("L2 Topolgy"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_L3] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("L3 Topolgy"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_L3_ROUTING] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("L3 Routing Neighborship"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_ORGANIC] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Organic Layout"), wxPG_LABEL, true));
	propIds[WKM_PROPID_STR_UNITEDGELENGTH] = wkmGrid->AppendIn(propIdi[WKM_PROPID_INT_ORGANIC], new wxStringProperty(wxT("Unit Edge Length"), wxPG_LABEL, "100.0"));
	propIdi[WKM_PROPID_INT_HIERARCHIC] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Hierarchic Layout"), wxPG_LABEL, true));
	propIds[WKM_PROPID_STR_LAYERDISTANCE] = wkmGrid->AppendIn(propIdi[WKM_PROPID_INT_HIERARCHIC], new wxStringProperty(wxT("Layer Distance"), wxPG_LABEL, "50.0"));
	propIds[WKM_PROPID_STR_NODEDISTANCE] = wkmGrid->AppendIn(propIdi[WKM_PROPID_INT_HIERARCHIC], new wxStringProperty(wxT("Node Distance"), wxPG_LABEL, "50.0"));
	propIds[WKM_PROPID_STR_WEIHTBALANCING] = wkmGrid->AppendIn(propIdi[WKM_PROPID_INT_HIERARCHIC], new wxStringProperty(wxT("Weight Balancing"), wxPG_LABEL, "0.1"));
	propIdi[WKM_PROPID_INT_ORTHO] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Orthogonal Layout"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_CDP] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Show CDP network devices"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_CDPHOSTS] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Show CDP end systems"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_UNKNOWNSTP] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Unknown STP neighbors"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_ROUTENEIGHBORS] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Unknown RP neighbors"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_ENDSYSTEMS] = wkmGrid->AppendIn(Ooptis, new wxBoolProperty(wxT("Draw EndSystems"), wxPG_LABEL, false));

	wxArrayString hcArrMod;
	hcArrMod.Add(wxT("Classic Interface Style"));
	hcArrMod.Add(wxT("Compact Interface Style"));
	hcArrMod.Add(wxT("Compact Interface Style/No Frame"));
	propIdi[WKM_PROPID_INT_INTFSTYLE] = wkmGrid->AppendIn(Ooptis, new wxEnumProperty(wxT("Interface Style Option"), wxPG_LABEL, hcArrMod));

	

	// Options
	wxPGId optis = wkmGrid->Append(new wxPropertyCategory(wxT("Options")));
	propIdi[WKM_PROPID_INT_PARSE] = wkmGrid->AppendIn(optis, new wxBoolProperty(wxT("Parse NEW"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_COMBINE] = wkmGrid->AppendIn(optis, new wxBoolProperty(wxT("Analyze NEW"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_CLEARDB] = wkmGrid->AppendIn(optis, new wxBoolProperty(wxT("Clean up DB"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_CLEARDB_ALL] = wkmGrid->AppendIn(optis, new wxBoolProperty(wxT("Clear DB"), wxPG_LABEL, true));
	propIdi[WKM_PROPID_INT_INMEMDB] = wkmGrid->AppendIn(optis, new wxBoolProperty(wxT("In-Memory DB"), wxPG_LABEL, true));

	//// Hilfe
	wkmGrid->SetPropertyHelpString(rootID, "Colored Options are mandatory");
	wkmGrid->SetPropertyHelpString(ipt, "Input Settings");
	wkmGrid->SetPropertyHelpString(opt, "Output Settings");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_SDIR], 
		"Search Directory");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_PATTERN], 
		"Only files with filenames containing this pattern will be included for parsing. If using more than one search pattern, use \";\" as seperator. Sample: pattern1;pattern2;pattern3;... ");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_HOSTFILE], 
		"File with devices to import to the database.");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_IMPORT], "Import -> YES/NO");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_IMP_ES], "Import EndSystems into device table; This is required for EndSystems Report and EndSystems Visio output, but may be very time consuming");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_OFILE], 
		"Output Visio .vdx File.");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_LOGFILE], "");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_PARSE], "Check, if you want to parse all files and refill the database; If the database already exists and you only want to recalculate the neighborship table, then uncheck this option.");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_COMBINE], "Check, if you want to analyze the database entries again.");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_CLEARDB], "Clean up Database: Delete all entries, except from the last cycle.");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_CLEARDB_ALL], "Delete all Database entries.");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_INMEMDB], "Loads DB in memory and performs all operations in memory. DB is saved to disk when tool has finished.");

	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_L2], "Calculate L2 Topology");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_L3], "Calculate L3 Topology");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_L3_ROUTING], "Calculate L3 Routing Neighborships");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_ORGANIC], "Run FMMM algorithm");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_HIERARCHIC], "Run Sugiyama algorithm (Disabled at the moment)");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_CDP], "Show network devices learned by CDP in output graph.");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_CDPHOSTS], "Show end systems learned by CDP in output graph; i.e. IP Phones, ACS...");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_UNKNOWNSTP], "Show Interfaces with unknown STP neighbors");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_ROUTENEIGHBORS], "Show Unknown Routing Protocol neighbors and unknown Routing Next Hop IPs");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_ENDSYSTEMS], "Show End Systems (PCs and Servers) in Visio Output");
	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_ORTHO], "Run Orthogonal algorithm (Disabled at the moment)");

	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_DNS_RESOLUTION], "Lookup DNS names for Endsystem IP addresses; This may be very time consuming.");

	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_UNITEDGELENGTH], "The unit edge length.");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_LAYERDISTANCE], "The minimal allowed y-distance between layers.");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_NODEDISTANCE], "The minimal allowed x-distance between nodes on a layer.");
	wkmGrid->SetPropertyHelpString(propIds[WKM_PROPID_STR_WEIHTBALANCING], "The weight for balancing successors below a node; 0.0 means no balancing.");

	wkmGrid->SetPropertyHelpString(propIdi[WKM_PROPID_INT_INTFSTYLE], "Interface Style in Visio Output");

	wkmGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);
	//wkmGrid->Collapse(optis);

	//setFarbe(true);
	wkmGrid->SetCaptionTextColour(ipt, wxColour("#ffffff"));

	wxPGId MyPropertyId = wkmGrid->GetPropertyByName("wkm");
	wkmGrid->SetCaptionTextColour(MyPropertyId, wxColour(0,0,128));

	wkmGrid->SetPropertyCell(rootID, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	wkmGrid->SetPropertyCell(ipt, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkmGrid->SetPropertyCell(opt, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkmGrid->SetPropertyCell(optis, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));

	int dbh = wkmGrid->GetDescBoxHeight();

	//wxFont testfont(10, wxDEFAULT, wxNORMAL, wxNORMAL, false);
	//wkmGrid->SetFont(testfont);

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(wkmGrid, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

CWkmPropGrid::~CWkmPropGrid(void)
{
}


void CWkmPropGrid::einstellungenInit(int index, int wert)
{
	wkmGrid->SetPropertyValue(propIdi[index], wert);
}


void CWkmPropGrid::einstellungenInit(int index, std::string wert)
{
	wkmGrid->SetPropertyValue(propIds[index], wert.c_str());
}


void CWkmPropGrid::resetFont(wxFont newFont)
{
	wkmGrid->SetFont(newFont);
}


int CWkmPropGrid::leseIntSetting(int index)
{
	wxVariant var = wkmGrid->GetPropertyValue(propIdi[index]);
	int ret = var.GetInteger();
	return ret;
}


std::string CWkmPropGrid::leseStringSetting(int index)
{
	std::string ret = "";
	ret = wkmGrid->GetPropertyValueAsString(propIds[index]);
	return ret;
}


int CWkmPropGrid::getSplitterPos()
{
	wxPropertyGridPage *tempPage = wkmGrid->GetPage(pageID);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CWkmPropGrid::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	wkmGrid->SetSplitterPosition(pos);
}


void CWkmPropGrid::OnPropertyGridChange( wxPropertyGridEvent& event )
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


void CWkmPropGrid::enableDisable()
{
	bool intfDisable = true;		// false wenn Tool == Interface Parser

	bool parseNew = leseIntSetting(WKM_PROPID_INT_PARSE);
	if (parseNew)
	{
		einstellungenInit(WKM_PROPID_INT_COMBINE, 1);
	}

}


void CWkmPropGrid::setFarbe(bool status)
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

	wkmGrid->SetPropertyBackgroundColour(propIds[WKM_PROPID_STR_SDIR], wxColour(farbe));
	wkmGrid->SetPropertyBackgroundColour(propIds[WKM_PROPID_STR_OFILE], wxColour(farbe));
}

#endif