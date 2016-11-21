#include "class_wknPropGrid.h"

BEGIN_EVENT_TABLE(CWknPropGrid, wxPanel)
	EVT_PG_CHANGED(wxID_ANY, CWknPropGrid::OnPropertyGridChange)
END_EVENT_TABLE()

CWknPropGrid::CWknPropGrid(wxWindow* pt, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(pt, id, pos, size, style, name)
{
	parent = pt;
	
	wknGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);

	wknGrid->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);

	pageID = wknGrid->AddPage(wxT("Configmaker Settings"));

	wxPGId rootID = wknGrid->Append(new wxPropertyCategory(wxT("Configmaker"), wxT("wkn")));

	// Data File Settings
	wxPGId dfId = wknGrid->Append(new wxPropertyCategory(wxT("Data File Settings")));
	propIds[WKN_PROPID_STR_DATAFILE] = wknGrid->AppendIn(dfId, new wxFileProperty(wxT("Data File"), "df", ""));
	propIdi[WKN_PROPID_INT_NONSFORMAT] = wknGrid->AppendIn(dfId, new wxBoolProperty(wxT("Non-Standard Format"), wxPG_LABEL, false));
	propIdi[WKN_PROPID_INT_FLEXDF] = wknGrid->AppendIn(dfId, new wxBoolProperty(wxT("Flexible Data File"), wxPG_LABEL, false));
	propIdi[WKN_PROPID_INT_USETAGS] = wknGrid->AppendIn(dfId, new wxBoolProperty(wxT("Use Tags"), wxPG_LABEL, false));
	propIds[WKN_PROPID_STR_REPLACELF] = wknGrid->AppendIn(dfId, new wxStringProperty(wxT("Replace LF"), wxPG_LABEL, " // "));

	// File Settings
	propIds[WKN_PROPID_STR_CONFIGFILE] = wknGrid->Append(new wxFileProperty(wxT("Config File"), wxPG_LABEL));
	propIds[WKN_PROPID_STR_LOGFILE] = wknGrid->Append(new wxFileProperty(wxT("Log File"), wxPG_LABEL));
	propIds[WKN_PROPID_STR_OUTPUTDIR] = wknGrid->Append(new wxDirProperty(wxT("Output Directory"), wxPG_LABEL));

	// Settings
	wxPGId sId = wknGrid->Append(new wxPropertyCategory(wxT("Settings")));
	propIdi[WKN_PROPID_INT_CYCLECOUNT] = wknGrid->AppendIn(sId, new wxIntProperty(wxT("Cycle Count"), wxPG_LABEL));
	propIds[WKN_PROPID_STR_FILENAMEVAR] = wknGrid->AppendIn(sId, new wxStringProperty(wxT("Filename Variable"), wxPG_LABEL));
	propIds[WKN_PROPID_STR_ADDSEP] = wknGrid->AppendIn(sId, new wxStringProperty(wxT("Additional Separator"), wxPG_LABEL));
	propIdi[WKN_PROPID_INT_USESEP] = wknGrid->AppendIn(sId, new wxBoolProperty(wxT("Use Separator as Placeholder End"), wxPG_LABEL, false));
	// Modus befüllen
	wxArrayString arrMod;
	arrMod.Add(wxT("One File per Line"));
	arrMod.Add(wxT("Whole output in one file"));
	propIdi[WKN_PROPID_INT_APPEND] = wknGrid->AppendIn(sId, new wxBoolProperty(wxT("Append Output"), wxPG_LABEL, false));	
	propIdi[WKN_PROPID_INT_MODUS] = wknGrid->AppendIn(sId, new wxEnumProperty(wxT("Modus"), wxPG_LABEL, arrMod));
	
	// Hilfe
	wknGrid->SetPropertyHelpString(rootID, "Colored Options are mandatory");
	wknGrid->SetPropertyHelpString(dfId, "See Documentation Section 3.1");
	wknGrid->SetPropertyHelpString(sId, "See Documentation Section 3.3");
	wknGrid->SetPropertyHelpString(propIds[WKN_PROPID_STR_DATAFILE], 
		"Data File: See Documentation Section 3.1");
	wknGrid->SetPropertyHelpString(propIdi[WKN_PROPID_INT_NONSFORMAT], 
		"Use for Excel generated DataFiles with linebreaks within cells to replace the linebreaks");
	wknGrid->SetPropertyHelpString(propIdi[WKN_PROPID_INT_FLEXDF], 
		"Use for Excel generated DataFiles with linebreaks within cells to use them as separator");
	wknGrid->SetPropertyHelpString(propIdi[WKN_PROPID_INT_USETAGS], 
		"Use Tags in Variables - See docu section 3.1.1.3 for more information");
	wknGrid->SetPropertyHelpString(propIds[WKN_PROPID_STR_REPLACELF], 
		"Replace linebreaks within cells from Excel generated DataFiles with this sequence");
	wknGrid->SetPropertyHelpString(propIds[WKN_PROPID_STR_CONFIGFILE], 
		"Config File: See Documentation Section 3.2");
	wknGrid->SetPropertyHelpString(propIds[WKN_PROPID_STR_LOGFILE], 
		"Log File: Only used, when wktools is started from CLI with options");
	wknGrid->SetPropertyHelpString(propIds[WKN_PROPID_STR_OUTPUTDIR], 
		"Output Directory: Where the result is saved");
	wknGrid->SetPropertyHelpString(propIdi[WKN_PROPID_INT_CYCLECOUNT], 
		"How many cycles shall the tool do with one file? Default = 1");
	wknGrid->SetPropertyHelpString(propIds[WKN_PROPID_STR_FILENAMEVAR], 
		"Filename Variable: The value of this placeholder will be taken as filename for the output file");
	wknGrid->SetPropertyHelpString(propIds[WKN_PROPID_STR_ADDSEP], 
		"Additional Separator: This char is used as separator in the Data File");
	wknGrid->SetPropertyHelpString(propIdi[WKN_PROPID_INT_USESEP], 
		"Use this option to use the placeholder as end marker of the variable");
	wknGrid->SetPropertyHelpString(propIdi[WKN_PROPID_INT_APPEND], 
		"If the output file already exists, the file will be replaced or the new output will be appended");
	wknGrid->SetPropertyHelpString(propIdi[WKN_PROPID_INT_MODUS], 
		"Modus: The result will be stored in one file or in separate files");

	wknGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);

	wknGrid->SetPropertyCell(rootID, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	wknGrid->SetPropertyCell(dfId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wknGrid->SetPropertyCell(sId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));

	wxPGId MyPropertyId = wknGrid->GetPropertyByName("wkn");
	wknGrid->SetCaptionTextColour(MyPropertyId, wxColour(0,0,128));

	//setFarbe(true);

	int dbh = wknGrid->GetDescBoxHeight();
	
	//wxFont testfont(10, wxDEFAULT, wxNORMAL, wxNORMAL, false);
	//wknGrid->SetFont(testfont);

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(wknGrid, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();
}


CWknPropGrid::~CWknPropGrid(void)
{
}


void CWknPropGrid::einstellungenInit(int index, int wert)
{
	wknGrid->SetPropertyValue(propIdi[index], wert);
}


void CWknPropGrid::einstellungenInit(int index, std::string wert)
{
	wknGrid->SetPropertyValue(propIds[index], wert.c_str());
}


void CWknPropGrid::resetFont(wxFont newFont)
{
	wknGrid->SetFont(newFont);
}


int CWknPropGrid::leseIntSetting(int index)
{
	wxVariant var = wknGrid->GetPropertyValue(propIdi[index]);
	int ret = var.GetInteger();
	return ret;
}


std::string CWknPropGrid::leseStringSetting(int index)
{
	std::string ret = "";
	ret = wknGrid->GetPropertyValueAsString(propIds[index]);
	return ret;
}


int CWknPropGrid::getSplitterPos()
{
	wxPropertyGridPage *tempPage = wknGrid->GetPage(pageID);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CWknPropGrid::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	wknGrid->SetSplitterPosition(pos);
}


void CWknPropGrid::setFarbe(bool status)
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

	wknGrid->SetPropertyBackgroundColour(propIds[WKN_PROPID_STR_DATAFILE], wxColour(farbe));
	wknGrid->SetPropertyBackgroundColour(propIds[WKN_PROPID_STR_CONFIGFILE], wxColour(farbe));
	wknGrid->SetPropertyBackgroundColour(propIds[WKN_PROPID_STR_OUTPUTDIR], wxColour(farbe));
	wknGrid->SetPropertyBackgroundColour(propIds[WKN_PROPID_STR_FILENAMEVAR], wxColour(farbe));
	wknGrid->SetPropertyBackgroundColour(propIds[WKN_PROPID_STR_ADDSEP], wxColour(farbe));
}


void CWknPropGrid::OnPropertyGridChange( wxPropertyGridEvent& event )
{
	wxPGProperty *property = event.GetProperty();

	// It may be NULL
	if ( !property )
		return;

	// Get name of changed property
	 const wxString& name = property->GetName();

	if (name == "df")
	{
		wxCommandEvent event(wkEVT_WKN_PROPGRIDCHANGE);
		event.SetInt(1);
		wxPostEvent(parent, event);
	}
}


