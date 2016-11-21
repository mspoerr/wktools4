#include "class_schedulerPropGrid.h"
#include <wx/propgrid/advprops.h>
#include "class_enhancedStringProperty.h"

DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHED_NEU)

BEGIN_EVENT_TABLE(CSchedPropGrid, wxPanel)
EVT_PG_CHANGED(wxID_ANY, CSchedPropGrid::OnPropertyGridChange)
EVT_PG_SELECTED(wxID_ANY, CSchedPropGrid::OnPropertyChangeButtonStyle)
EVT_COMMAND(wxID_ANY, wkEVT_SCHED_NEU, CSchedPropGrid::OnNeuerEintrag)
END_EVENT_TABLE()

CSchedPropGrid::CSchedPropGrid(wxWindow* pt, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(pt, id, pos, size, style, name)
{
	parent = pt;

	schedGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);

	schedGrid->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);


	pageID = schedGrid->AddPage(wxT("Scheduler"));

	wxPGId rootID = schedGrid->Append(new wxPropertyCategory(wxT("Scheduler Settings"), wxT("sched")));
	propIds[SCHED_PROPID_STR_SCHEDFILE] = schedGrid->AppendIn(rootID, new wxFileProperty(wxT("SchedFile"), wxPG_LABEL));

	// New Scheduler Entry
	wxPGId se = schedGrid->Append(new wxPropertyCategory(wxT("New Scheduler Entry")));

	// Tool
	wxArrayString toolArrMod;
	toolArrMod.Add(wxT("ConfigMaker"));
	toolArrMod.Add(wxT("Configure Devices"));
	toolArrMod.Add(wxT("IP List"));
	toolArrMod.Add(wxT("Device Information Parser"));
	toolArrMod.Add(wxT("Mapper"));

	propIds[SCHED_PROPID_STR_TOOL] = schedGrid->AppendIn(se, new wxEnumProperty(wxT("Tool Option"), wxPG_LABEL, toolArrMod));

	// Profile
	wxArrayString profArrMod;
	profArrMod.Add(wxT("ConfigMaker"));
	profArrMod.Add(wxT("Configure Devices"));
	profArrMod.Add(wxT("IP List"));
	profArrMod.Add(wxT("Device Information Parser"));
	profArrMod.Add(wxT("Mapper"));

	propIds[SCHED_PROPID_STR_PROFILE] = schedGrid->AppendIn(se, new wxEnumProperty(wxT("Profile"), wxPG_LABEL));

	// Date/Time
	//propIds[SCHED_PROPID_STR_DATE] = schedGrid->AppendIn(se, new wxStringProperty(wxT("Date"), wxPG_LABEL, ""));
	propIds[SCHED_PROPID_STR_TIME] = schedGrid->AppendIn(se, new wxStringProperty(wxT("Time"), wxPG_LABEL, ""));
	propIds[SCHED_PROPID_STR_USER] = schedGrid->AppendIn(se, new wxStringProperty(wxT("Username"), wxPG_LABEL, ""));
	propIds[SCHED_PROPID_STR_PASS] = schedGrid->AppendIn(se, new wxStringProperty(wxT("Password"), wxPG_LABEL, ""));


	// Recurrence
	wxArrayString recArrMod;
	recArrMod.Add(wxT("On Demand"));
	recArrMod.Add(wxT("Once"));
	recArrMod.Add(wxT("Daily"));
	recArrMod.Add(wxT("Weekly"));
	recArrMod.Add(wxT("Monthly"));

	propIds[SCHED_PROPID_STR_REC] = schedGrid->AppendIn(se, new wxEnumProperty(wxT("Recurrence"), wxPG_LABEL, recArrMod));

	propIds[SCHED_PROPID_STR_NAME] = schedGrid->AppendIn(se, new eStringProperty(wxT("Name"), wxPG_LABEL, ""));



	//// Hilfe
	schedGrid->SetPropertyHelpString(rootID, "Colored Options are mandatory");
	schedGrid->SetPropertyHelpString(se, "Create new Scheduler entry");

	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_SCHEDFILE], 
		"File, where Scheduler entries are saved");
	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_USER], 
		"Username to start scheduled task");
	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_PASS], 
		"Password for given Username");
	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_TOOL], 
		"Tool name");
	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_PROFILE], 
		"Saved Profile");
	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_REC], 
		"How often to start the new Scheduler entry");
	//schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_DATE], 
	//	"Specified Date, when to start the new Scheduler entry\nFormat: CCYY-MM-DD");
	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_TIME], 
		"Specified Time, when to start the new Scheduler entry\nFormat: CCYY-MM-DD HH:MM:SS.mmm");
	schedGrid->SetPropertyHelpString(propIds[SCHED_PROPID_STR_NAME], 
		"Name of the new Scheduler entry");

	schedGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);

	//setFarbe(true);
	schedGrid->SetCaptionTextColour(se, wxColour("#ffffff"));

	wxPGId MyPropertyId = schedGrid->GetPropertyByName("sched");
	schedGrid->SetCaptionTextColour(MyPropertyId, wxColour(0,0,128));

	schedGrid->SetPropertyCell(rootID, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	schedGrid->SetPropertyCell(se, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));

	int dbh = schedGrid->GetDescBoxHeight();

	//wxFont testfont(10, wxDEFAULT, wxNORMAL, wxNORMAL, false);
	//schedGrid->SetFont(testfont);

	enableDisable();

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(schedGrid, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

CSchedPropGrid::~CSchedPropGrid(void)
{
}


void CSchedPropGrid::einstellungenInit(int index, int wert)
{
	schedGrid->SetPropertyValue(propIdi[index], wert);
}


void CSchedPropGrid::einstellungenInit(int index, std::string wert)
{
	schedGrid->SetPropertyValueString(propIds[index], wert.c_str());
	if (index == SCHED_PROPID_STR_REC)
	{
		enableDisable();
	}
	
	
	

//	schedGrid->SetPropertyValue(propIds[index], wert.c_str());
}


void CSchedPropGrid::resetFont(wxFont newFont)
{
	schedGrid->SetFont(newFont);
}


std::string CSchedPropGrid::leseStringSetting(int index)
{
	std::string ret = "";
	ret = schedGrid->GetPropertyValueAsString(propIds[index]);
	return ret;
}


int CSchedPropGrid::leseIntSetting(int index)
{
	wxVariant var = schedGrid->GetPropertyValue(propIdi[index]);
	int ret = var.GetInteger();
	return ret;
}


int CSchedPropGrid::getSplitterPos()
{
	wxPropertyGridPage *tempPage = schedGrid->GetPage(pageID);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CSchedPropGrid::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	schedGrid->SetSplitterPosition(pos);
}


void CSchedPropGrid::OnPropertyGridChange( wxPropertyGridEvent& event )
{
	wxPGProperty *property = event.GetProperty();

	// It may be NULL
	if ( !property )
		return;

	// Get name of changed property
	const wxString& name = property->GetName();

	if (name == "Tool Option")
	{
		// Profile laden lassen
		wxCommandEvent event(wkEVT_SCHED_LADEPROFILE);
		event.SetInt(1);
		wxPostEvent(parent, event);
	}
	else if (name == "SchedFile")
	{
		// SchedFile in die Tabellenansicht laden
		wxCommandEvent event(wkEVT_SCHED_LADEFILE);
		event.SetInt(1);
		wxPostEvent(parent, event);
	}
	else if (name == "Recurrence")
	{
		enableDisable();
	}

}


void CSchedPropGrid::enableDisable()
{
	bool dtDisable = true;		// false wenn Recurrence == OnDemand
	if (leseStringSetting(SCHED_PROPID_STR_REC) == "On Demand")
	{
		dtDisable = false;
	}
	schedGrid->EnableProperty(propIds[SCHED_PROPID_STR_TIME], dtDisable);
	schedGrid->EnableProperty(propIds[SCHED_PROPID_STR_USER], dtDisable);
	schedGrid->EnableProperty(propIds[SCHED_PROPID_STR_PASS], dtDisable);
}


void CSchedPropGrid::setFarbe(bool status)
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

	//schedGrid->SetPropertyBackgroundColour(propIds[SCHED_PROPID_STR_SDIR], wxColour(farbe));
	//schedGrid->SetPropertyBackgroundColour(propIds[SCHED_PROPID_STR_ODIR], wxColour(farbe));
	//schedGrid->SetPropertyBackgroundColour(propIds[SCHED_PROPID_STR_OFILE], wxColour(farbe));
}


void CSchedPropGrid::OnPropertyChangeButtonStyle( wxPropertyGridEvent& event )
{
	wxPGProperty *property = event.GetProperty();

	// It may be NULL
	if ( !property )
		return;

	// Get name of changed property
	 const wxString& name = property->GetName();

	 if (name == "Name")
	 {
		wxPropertyGrid *theGrid = schedGrid->GetGrid();	
		wxButton *button = (wxButton*)theGrid->GetEditorControlSecondary();
		button->SetLabel("->");
		button->SetToolTip("Add to list");
		//wxSize butSize = button->GetSize();
		//butSize.SetWidth(butSize.GetWidth()*2);
		//button->SetSize(butSize);
	 }
}


void CSchedPropGrid::ladeProfile(wxArrayString profs)
{
	wxPGChoices profAuswahl;
	profAuswahl.Set(profs);
	
	schedGrid->SetPropertyChoices(propIds[SCHED_PROPID_STR_PROFILE], profAuswahl);
}


void CSchedPropGrid::OnNeuerEintrag(wxCommandEvent &event)
{
	wxPostEvent(parent, event);

}