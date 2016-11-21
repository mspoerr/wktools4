#include "class_dipPropGrid.h"


BEGIN_EVENT_TABLE(CDipPropGrid, wxPanel)
EVT_PG_CHANGED(wxID_ANY, CDipPropGrid::OnPropertyGridChange)
END_EVENT_TABLE()

CDipPropGrid::CDipPropGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	dipGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);

	dipGrid->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);


	pageID = dipGrid->AddPage(wxT("Device Information Parser Settings"));

	wxPGId rootID = dipGrid->Append(new wxPropertyCategory(wxT("Device Information Parser"), wxT("dip")));

	// Tool
	wxArrayString toolArrMod;
	toolArrMod.Add(wxT("Inventory Parser"));
	toolArrMod.Add(wxT("Interface Parser"));

	propIdi[DIP_PROPID_INT_TOOL] = dipGrid->AppendIn(rootID, new wxEnumProperty(wxT("Tool Option"), wxPG_LABEL, toolArrMod));

	// Input
	wxPGId ipt = dipGrid->Append(new wxPropertyCategory(wxT("Input")));
	propIds[DIP_PROPID_STR_SDIR] = dipGrid->AppendIn(ipt, new wxDirProperty(wxT("Search Directory"), wxPG_LABEL, ::wxGetUserHome()));
	propIds[DIP_PROPID_STR_PATTERN] = dipGrid->AppendIn(ipt, new wxStringProperty(wxT("Search Pattern"), wxPG_LABEL, ""));

	// Output
	wxPGId opt = dipGrid->Append(new wxPropertyCategory(wxT("Output")));
	propIds[DIP_PROPID_STR_ODIR] = dipGrid->AppendIn(opt, new wxDirProperty(wxT("Output Directory"), wxPG_LABEL, ::wxGetUserHome()));
	propIds[DIP_PROPID_STR_OFILE] = dipGrid->AppendIn(opt, new wxStringProperty(wxT("Output File"), wxPG_LABEL, ""));
	propIdi[DIP_PROPID_INT_APPEND] = dipGrid->AppendIn(opt, new wxBoolProperty(wxT("Append"), wxPG_LABEL, true));	
	propIds[DIP_PROPID_STR_LOGFILE] = dipGrid->AppendIn(opt, new wxFileProperty(wxT("LogFile"), wxPG_LABEL));

	// Inventory Settings
	wxPGId inv = dipGrid->Append(new wxPropertyCategory(wxT("Inventory Options")));
	propIdi[DIP_PROPID_INT_SNMPLOCCOL] = dipGrid->AppendIn(inv, new wxIntProperty(wxT("SNMP Location Columns"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_CHASSISONLY] = dipGrid->AppendIn(inv, new wxBoolProperty(wxT("Chassis Info only"), wxPG_LABEL, false));	
	
	wxPGId invcol = dipGrid->AppendIn(inv, new wxPropertyCategory(wxT("Show the following columns")));	
	propIdi[DIP_PROPID_INT_COL1] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Hostname"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL2] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Location/Slot"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL3] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Device Type"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL4] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Description"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL5] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Serial Number"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL6] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Hardware Revision"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL7] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Memory"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL8] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Bootfile"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL9] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("SW Version"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL10] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("License"), wxPG_LABEL, false));	
	propIdi[DIP_PROPID_INT_COL11] = dipGrid->AppendIn(invcol, new wxBoolProperty(wxT("Flash Info"), wxPG_LABEL, false));	

	// Interface Information Options
	wxPGId intfi = dipGrid->Append(new wxPropertyCategory(wxT("Interface Information Options")));
	propIdi[DIP_PROPID_INT_DESCRIPTION] = dipGrid->AppendIn(intfi, new wxBoolProperty(wxT("No Intf Descriptions"), wxPG_LABEL, false));	

	//// Hilfe
	dipGrid->SetPropertyHelpString(rootID, "Colored Options are mandatory");
	dipGrid->SetPropertyHelpString(ipt, "Input Settings");
	dipGrid->SetPropertyHelpString(opt, "Output Settings");
	dipGrid->SetPropertyHelpString(propIds[DIP_PROPID_STR_SDIR], 
		"Search Directory");
	dipGrid->SetPropertyHelpString(propIds[DIP_PROPID_STR_PATTERN], 
		"Only files with filenames containing this pattern will be included for parsing. If using more than one search pattern, use \";\" as seperator. Sample: pattern1;pattern2;pattern3;... ");
	dipGrid->SetPropertyHelpString(propIds[DIP_PROPID_STR_ODIR], 
		"Directory, where the result will be saved to.");
	dipGrid->SetPropertyHelpString(propIds[DIP_PROPID_STR_OFILE], 
		"Filename, where results will be saved to.");
	dipGrid->SetPropertyHelpString(propIdi[DIP_PROPID_INT_APPEND], 
		"Append results if the output file exists.");
	dipGrid->SetPropertyHelpString(propIds[DIP_PROPID_STR_LOGFILE], 
		"");
	dipGrid->SetPropertyHelpString(inv, "Inventory Options");
	dipGrid->SetPropertyHelpString(propIdi[DIP_PROPID_INT_SNMPLOCCOL], 
		"Number of SNMP Location columns");
	dipGrid->SetPropertyHelpString(propIdi[DIP_PROPID_INT_CHASSISONLY], 
		"Only collect chassis information.");
	dipGrid->SetPropertyHelpString(invcol, 
		"Inventory Columns. Unchecked Information will not be presented in the results.");
	dipGrid->SetPropertyHelpString(intfi, "Interface Information Options");
	dipGrid->SetPropertyHelpString(propIdi[DIP_PROPID_INT_DESCRIPTION], 
		"Don't collect interface descriptions");

	dipGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);
	dipGrid->Collapse(invcol);
	dipGrid->Collapse(intfi);

	//setFarbe(true);
	dipGrid->SetCaptionTextColour(ipt, wxColour("#ffffff"));

	wxPGId MyPropertyId = dipGrid->GetPropertyByName("dip");
	dipGrid->SetCaptionTextColour(MyPropertyId, wxColour(0,0,128));

	dipGrid->SetPropertyCell(rootID, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	dipGrid->SetPropertyCell(ipt, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	dipGrid->SetPropertyCell(opt, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	dipGrid->SetPropertyCell(inv, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	dipGrid->SetPropertyCell(invcol, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	dipGrid->SetPropertyCell(intfi, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));

	int dbh = dipGrid->GetDescBoxHeight();

	//wxFont testfont(10, wxDEFAULT, wxNORMAL, wxNORMAL, false);
	//dipGrid->SetFont(testfont);

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(dipGrid, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

CDipPropGrid::~CDipPropGrid(void)
{
}


void CDipPropGrid::einstellungenInit(int index, int wert)
{
	dipGrid->SetPropertyValue(propIdi[index], wert);
}


void CDipPropGrid::einstellungenInit(int index, std::string wert)
{
	dipGrid->SetPropertyValue(propIds[index], wert.c_str());
}


void CDipPropGrid::resetFont(wxFont newFont)
{
	dipGrid->SetFont(newFont);
}


int CDipPropGrid::leseIntSetting(int index)
{
	wxVariant var = dipGrid->GetPropertyValue(propIdi[index]);
	int ret = var.GetInteger();
	return ret;
}


std::string CDipPropGrid::leseStringSetting(int index)
{
	std::string ret = "";
	ret = dipGrid->GetPropertyValueAsString(propIds[index]);
	return ret;
}


int CDipPropGrid::getSplitterPos()
{
	wxPropertyGridPage *tempPage = dipGrid->GetPage(pageID);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CDipPropGrid::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	dipGrid->SetSplitterPosition(pos);
}


void CDipPropGrid::OnPropertyGridChange( wxPropertyGridEvent& event )
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


void CDipPropGrid::enableDisable()
{
	bool intfDisable = true;		// false wenn Tool == Interface Parser
	if (leseIntSetting(DIP_PROPID_INT_TOOL))
	{
		// Interface Parser
		intfDisable = false;
	}
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_CHASSISONLY], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_SNMPLOCCOL], intfDisable);

	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL1], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL2], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL3], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL4], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL5], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL6], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL7], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL8], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL9], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL10], intfDisable);
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_COL11], intfDisable);
	
	dipGrid->EnableProperty(propIdi[DIP_PROPID_INT_DESCRIPTION], !intfDisable);
}


void CDipPropGrid::setFarbe(bool status)
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

	dipGrid->SetPropertyBackgroundColour(propIds[DIP_PROPID_STR_SDIR], wxColour(farbe));
	dipGrid->SetPropertyBackgroundColour(propIds[DIP_PROPID_STR_ODIR], wxColour(farbe));
	dipGrid->SetPropertyBackgroundColour(propIds[DIP_PROPID_STR_OFILE], wxColour(farbe));
}