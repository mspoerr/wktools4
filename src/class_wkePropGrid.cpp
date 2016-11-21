#include "class_wkePropGrid.h"


BEGIN_EVENT_TABLE(CWkePropGrid, wxPanel)
	EVT_PG_CHANGED(wxID_ANY, CWkePropGrid::OnPropertyGridChange)
END_EVENT_TABLE()

CWkePropGrid::CWkePropGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	dgp = NULL;
	wkeGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);

	wkeGrid->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);


	pageID = wkeGrid->AddPage(wxT("Configure Devices Settings"));

	wxPGId rootID = wkeGrid->Append(new wxPropertyCategory(wxT("Configure Devices"), wxT("wke")));

	// DeviceGroupFile Settings
	wxPGId dgId = wkeGrid->Append(new wxPropertyCategory(wxT("DeviceGroup Settings")));
	propIdi[WKE_PROPID_INT_DEVGRP] = wkeGrid->AppendIn(dgId, new wxBoolProperty(wxT("Use DeviceGroup File"), wxPG_LABEL, false));
	propIds[WKE_PROPID_STR_DEVGRPFILE] = wkeGrid->AppendIn(dgId, new wxFileProperty(wxT("DeviceGroup File"), wxPG_LABEL));
	
	// Config und Hostfile Optionen
	// ConfigFile Options befüllen
	wxArrayString hcArrMod;
	hcArrMod.Add(wxT("Static File"));
	hcArrMod.Add(wxT("Inventory (Depricated)"));
	hcArrMod.Add(wxT("Interface (Depricated)"));
	hcArrMod.Add(wxT("Mapper"));
	hcArrMod.Add(wxT("Dynamic"));
	hcArrMod.Add(wxT("Dynamic+Default"));

	wxPGId hcsId = wkeGrid->Append(new wxPropertyCategory(wxT("Host & Configfile Settings")));
	propIds[WKE_PROPID_STR_HOSTLFILE] = wkeGrid->AppendIn(hcsId, new wxFileProperty(wxT("HostFile"), wxPG_LABEL));
	propIds[WKE_PROPID_STR_CONFIGFILE] = wkeGrid->AppendIn(hcsId, new wxFileProperty(wxT("ConfigFile"), wxPG_LABEL));
	propIdi[WKE_PROPID_INT_CONFIGFILEOPTION] = wkeGrid->AppendIn(hcsId, new wxEnumProperty(wxT("ConfigFile Option"), wxPG_LABEL, hcArrMod));
	propIds[WKE_PROPID_STR_DYNCONFIGFOLDER] = wkeGrid->AppendIn(propIdi[WKE_PROPID_INT_CONFIGFILEOPTION], new wxDirProperty(wxT("Dyn Config Folder"), wxPG_LABEL, ::wxGetUserHome()));
	propIds[WKE_PROPID_STR_DEFCONFIGFILE] = wkeGrid->AppendIn(propIdi[WKE_PROPID_INT_CONFIGFILEOPTION], new wxFileProperty(wxT("Default ConfigFile"), wxPG_LABEL));
	propIds[WKE_PROPID_STR_ERWEITERUNG] = wkeGrid->AppendIn(propIdi[WKE_PROPID_INT_CONFIGFILEOPTION], new wxStringProperty(wxT("File Extension"), wxPG_LABEL, ".txt"));
	
	// Password Settings
	wxPGId pwId = wkeGrid->Append(new wxPropertyCategory(wxT("User/Password Settings")));
	propIds[WKE_PROPID_STR_USER] = wkeGrid->AppendIn(pwId, new wxStringProperty(wxT("Username"), wxPG_LABEL, ""));
	propIds[WKE_PROPID_STR_LOPW] = wkeGrid->AppendIn(pwId, new wxStringProperty(wxT("Login Password"), wxPG_LABEL, ""));
	propIds[WKE_PROPID_STR_ENPW] = wkeGrid->AppendIn(pwId, new wxStringProperty(wxT("Enable Password"), wxPG_LABEL, ""));
	propIdi[WKE_PROPID_INT_VISIBLEPW] = wkeGrid->AppendIn(pwId, new wxBoolProperty(wxT("Visible"), wxPG_LABEL, true));	
	propIdi[WKE_PROPID_INT_DYNPW] = wkeGrid->AppendIn(pwId, new wxBoolProperty(wxT("Dynamic PW"), wxPG_LABEL, false));	

	// Modus
	// Modus Options befüllen
	wxArrayString mdArrMod;
	mdArrMod.Add(wxT("COM1"));
	mdArrMod.Add(wxT("COMx"));
	mdArrMod.Add(wxT("Telnet"));
	mdArrMod.Add(wxT("SSH"));
	mdArrMod.Add(wxT("HTTP"));
	mdArrMod.Add(wxT("HTTPS"));
	mdArrMod.Add(wxT("CLI"));
	
	// Type Options befüllen
	wxArrayString tpArrMod;
	tpArrMod.Add(wxT("SingleHop"));
	tpArrMod.Add(wxT("MultiHop"));

	// Terminalserver Options befüllen
	wxArrayString tsArrMod;
	tsArrMod.Add(wxT("None"));
	tsArrMod.Add(wxT("Authentication"));
	tsArrMod.Add(wxT("No Authentication"));

	wxPGId mdId = wkeGrid->Append(new wxPropertyCategory(wxT("Connection Settings")));
	propIdi[WKE_PROPID_INT_MODUS] = wkeGrid->AppendIn(mdId, new wxEnumProperty(wxT("Modus"), wxPG_LABEL, mdArrMod));
	propIdi[WKE_PROPID_INT_TYPE] = wkeGrid->AppendIn(mdId, new wxEnumProperty(wxT("Type"), wxPG_LABEL, tpArrMod));
	propIds[WKE_PROPID_STR_PORT] = wkeGrid->AppendIn(mdId, new wxStringProperty(wxT("Telnet/SSH/HTTP/COM Port"), wxPG_LABEL, ""));
	propIdi[WKE_PROPID_INT_TERMSERVER] = wkeGrid->AppendIn(mdId, new wxEnumProperty(wxT("Terminal Server Connection"), wxPG_LABEL, tsArrMod));
	propIds[WKE_PROPID_STR_MHOPCOMMAND] = wkeGrid->AppendIn(mdId, new wxStringProperty(wxT("Multihop Command"), wxPG_LABEL, "telnet $ip"));

	// Output Settings
	// Show Ausgabe Options befüllen
	wxArrayString shArrMod;
	shArrMod.Add(wxT("Hostname"));
	shArrMod.Add(wxT("IP Address"));
	shArrMod.Add(wxT("One File"));

	wxPGId oId = wkeGrid->Append(new wxPropertyCategory(wxT("Output Settings")));
	propIdi[WKE_PROPID_INT_SHOW] = wkeGrid->AppendIn(oId, new wxEnumProperty(wxT("Show Output"), wxPG_LABEL, shArrMod));
	propIds[WKE_PROPID_STR_SHP] = wkeGrid->AppendIn(oId, new wxStringProperty(wxT("Show Filename Pattern"), wxPG_LABEL, ""));
	propIdi[WKE_PROPID_INT_SHOWDATE] = wkeGrid->AppendIn(oId, new wxBoolProperty(wxT("Append Date to filename"), wxPG_LABEL, true));
	propIdi[WKE_PROPID_INT_SHOWAPPEND] = wkeGrid->AppendIn(oId, new wxBoolProperty(wxT("Append"), wxPG_LABEL, false));
	propIds[WKE_PROPID_STR_LOGFILE] = wkeGrid->AppendIn(oId, new wxFileProperty(wxT("LogFile"), wxPG_LABEL));
	propIds[WKE_PROPID_STR_ODIR] = wkeGrid->AppendIn(oId, new wxDirProperty(wxT("Output Directory"), wxPG_LABEL));

	// Advanced Settings
	// Falsches PW Optionen befüllen
	wxArrayString wpArrMod;
	wpArrMod.Add(wxT("\"exit\""));
	wpArrMod.Add(wxT("command"));
	wxPGId advId = wkeGrid->Append(new wxPropertyCategory(wxT("Advanced Settings")));
	propIds[WKE_PROPID_STR_EXEC] = wkeGrid->AppendIn(advId, new wxStringProperty(wxT("Exec Mode"), wxPG_LABEL, "enable"));
	propIdi[WKE_PROPID_INT_RAUTE] = wkeGrid->AppendIn(advId, new wxBoolProperty(wxT("Wait for reply"), wxPG_LABEL, true));	
	propIdi[WKE_PROPID_INT_LOGSY] = wkeGrid->AppendIn(advId, new wxBoolProperty(wxT("Configure \"logging sync\""), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_LOGINMODE] = wkeGrid->AppendIn(advId, new wxEnumProperty(wxT("Command in case of wrong password"), wxPG_LABEL, wpArrMod));
	propIds[WKE_PROPID_STR_HOSTID] = wkeGrid->AppendIn(advId, new wxStringProperty(wxT("Default Host ID"), wxPG_LABEL, ""));
	propIdi[WKE_PROPID_INT_F2301] = wkeGrid->AppendIn(advId, new wxBoolProperty(wxT("Show 2301"), wxPG_LABEL, true));	

	// Debug Options
	wxPGId dbgId = wkeGrid->Append(new wxPropertyCategory(wxT("Debug Options")));
	propIdi[WKE_PROPID_INT_DBG_SEND] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Send Data"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_RECEIVE] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Receive Data"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_BUFTEST] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("BufTest Return Value"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_BUFTESTDET] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("BufTest Detail"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_FUNCTION] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Function Calls"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_HOSTNAME] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Get Hostname"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_SHOW] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Show"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_RGX] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Regex"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_HEX] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("HEX output for Send/Receive Data"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_SPECIAL1] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Special 1"), wxPG_LABEL, false));	
	propIdi[WKE_PROPID_INT_DBG_SPECIAL2] = wkeGrid->AppendIn(dbgId, new wxBoolProperty(wxT("Special 2"), wxPG_LABEL, false));	

	//// Hilfe
	wkeGrid->SetPropertyHelpString(rootID, "Colored Options are mandatory");
	wkeGrid->SetPropertyHelpString(dgId, "See Documentation Section 5.1");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_DEVGRP], 
		"Enable DeviceGroup File usage");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_DEVGRPFILE], 
		"DeviceGroup Filename Path");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_HOSTLFILE], 
		"See Documentation Section 5.2");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_CONFIGFILE], 
		"See Documentation Section 5.3");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_CONFIGFILEOPTION], 
		"Choose between \"config file templates\", \"static config file\" or \"dynamic config files\". See Documentation Section 5.3 for more information.");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_DYNCONFIGFOLDER], 
		"Location of the specific config files");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_DEFCONFIGFILE], 
		"This config is used, when no specific config file is found");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_ERWEITERUNG], 
		"The extension for the specific config files. Keep this field empty if there is no extension. Don't forget the dot (.)");
	wkeGrid->SetPropertyHelpString(pwId, 
		"Username/Password credentials to access the devices");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_VISIBLEPW], 
		"Uncheck, if you want to hide the entered Username/Password values");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_DYNPW], 
		"Check, if you want to use dynamic Username Passwords. The Username/Password credentials are then specified int the config files");
	wkeGrid->SetPropertyHelpString(mdId, 
		"Connection settigs");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_MODUS], 
		"Protocol to access the device");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_TYPE], 
		"See Documentation Section 5.4.3");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_PORT], 
		"* Port where the remote SSH/Telnet/HTTP service is listening\n* COM Port, when COMx is used.");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_TERMSERVER], 
		"Use this option if you want to connect via a Cisco Terminal server");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_MHOPCOMMAND], 
		"Command used when hopping from one device to a second one. Variables may be used. See Documentation Section 5.4.4 for more information.");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_SHOW], 
		"How to save output from \"show\" commands. See Documentation Section 5.4.5 for more information.");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_SHOWDATE],
		"Append current Date to filename.");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_SHOWAPPEND],
		"Check if you want to append the show output to an existing file.");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_SHP], 
		"To specify more unique filenames.");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_LOGFILE], 
		"");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_ODIR], 
		"Directory where all show output files are saved");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_EXEC], 
		"What should be sent at login Mode (router>)? Default: \"enable\"");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_RAUTE], 
		"Only send data, when the prompt is received from remote host. See Documentation Section 5.4.5 for more information");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_LOGSY], 
		"Configure \"logging sync\": See Documentation Section 5.4.5 for more information");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_LOGINMODE], 
		"Command in case of wrong password: See Documentation Section 5.4.5 for more information");
	wkeGrid->SetPropertyHelpString(dbgId, 
		"Debug Options -> only helpful to find bugs.");
	wkeGrid->SetPropertyHelpString(propIds[WKE_PROPID_STR_HOSTID], 
		"Special Use: If SQL Logging is enabled, this field could be used for the INSERT command.");
	wkeGrid->SetPropertyHelpString(propIdi[WKE_PROPID_INT_F2301], 
		"Deselect if you do not want to see 2301 error messages in the log output.");

	wkeGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);
	wkeGrid->Collapse(dbgId);
	wkeGrid->Collapse(advId);
	wkeGrid->Collapse(propIdi[WKE_PROPID_INT_CONFIGFILEOPTION]);
	//wkeGrid->Collapse(pwId);

	//setFarbe(true);
	wkeGrid->SetCaptionTextColour(dgId, wxColour("#ffffff"));

	wxPGId MyPropertyId = wkeGrid->GetPropertyByName("wke");
	wkeGrid->SetCaptionTextColour(MyPropertyId, wxColour(0,0,128));

	wkeGrid->SetPropertyCell(rootID, 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	wkeGrid->SetPropertyCell(hcsId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkeGrid->SetPropertyCell(pwId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkeGrid->SetPropertyCell(mdId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkeGrid->SetPropertyCell(advId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkeGrid->SetPropertyCell(dbgId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkeGrid->SetPropertyCell(dgId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));
	wkeGrid->SetPropertyCell(oId, 0, wxPG_LABEL, wxNullBitmap, wxColour("#000080"));

	int dbh = wkeGrid->GetDescBoxHeight();

	//wxFont testfont(10, wxDEFAULT, wxNORMAL, wxNORMAL, false);
	//wkeGrid->SetFont(testfont);

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(wkeGrid, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

CWkePropGrid::~CWkePropGrid(void)
{
}


void CWkePropGrid::einstellungenInit(int index, int wert)
{
	wkeGrid->SetPropertyValue(propIdi[index], wert);
}


void CWkePropGrid::einstellungenInit(int index, std::string wert)
{
	wkeGrid->SetPropertyValue(propIds[index], wert.c_str());
}


void CWkePropGrid::resetFont(wxFont newFont)
{
	wkeGrid->SetFont(newFont);
}


int CWkePropGrid::leseIntSetting(int index)
{
	wxVariant var = wkeGrid->GetPropertyValue(propIdi[index]);
	int ret = var.GetInteger();
	return ret;
}


std::string CWkePropGrid::leseStringSetting(int index)
{
	std::string ret = "";
	ret = wkeGrid->GetPropertyValueAsString(propIds[index]);
	return ret;
}


int CWkePropGrid::getSplitterPos()
{
	wxPropertyGridPage *tempPage = wkeGrid->GetPage(pageID);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CWkePropGrid::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	wkeGrid->SetSplitterPosition(pos);
}


void CWkePropGrid::OnPropertyGridChange( wxPropertyGridEvent& event )
{
	//wxPGProperty *property = event.GetProperty();

	// It may be NULL
	//if ( !property )
	//	return;

	// Get name of changed property
	// const wxString& name = property->GetName();

	// Get resulting value
	// wxVariant value = property->GetValue();

	// Nachricht schicken, dass die Spalten in der DeviceGroup Ansicht angepasst werden müssen...
	// TODO: xxxx

	enableDisable();
}


void CWkePropGrid::enableDisable()
{
	// Wenn DeviceGroup verwendet wird, alles andere deaktivieren
	bool useDevGrp = leseIntSetting(WKE_PROPID_INT_DEVGRP);
	
	wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_HOSTLFILE], !useDevGrp);
	wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_PORT], !useDevGrp);
	wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_MHOPCOMMAND], !useDevGrp);
	
	wkeGrid->EnableProperty(propIdi[WKE_PROPID_INT_CONFIGFILEOPTION], !useDevGrp);
	wkeGrid->EnableProperty(propIdi[WKE_PROPID_INT_TERMSERVER], !useDevGrp);

	switch (leseIntSetting(WKE_PROPID_INT_CONFIGFILEOPTION))
	{
	case 0:			// Static File
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_CONFIGFILE], true && !useDevGrp);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DYNCONFIGFOLDER], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DEFCONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_ERWEITERUNG], false);
		wkeGrid->EnableProperty(propIdi[WKE_PROPID_INT_DYNPW], false);
		einstellungenInit(WKE_PROPID_INT_DYNPW, 0);
		break;
	case 1:			// Inventory
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_CONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DYNCONFIGFOLDER], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DEFCONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_ERWEITERUNG], false);
		wkeGrid->EnableProperty(propIdi[WKE_PROPID_INT_DYNPW], false);
		einstellungenInit(WKE_PROPID_INT_DYNPW, 0);
		break;
	case 2:			// Interface
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_CONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DYNCONFIGFOLDER], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DEFCONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_ERWEITERUNG], false);
		wkeGrid->EnableProperty(propIdi[WKE_PROPID_INT_DYNPW], false);
		einstellungenInit(WKE_PROPID_INT_DYNPW, 0);
		break;
	case 3:			// Dynamic
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_CONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DYNCONFIGFOLDER], true && !useDevGrp);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DEFCONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_ERWEITERUNG], true && !useDevGrp);
		wkeGrid->EnableProperty(propIdi[WKE_PROPID_INT_DYNPW], true && !useDevGrp);
		break;
	case 4:			// Dynamic & Default File
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_CONFIGFILE], false);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DYNCONFIGFOLDER], true && !useDevGrp);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_DEFCONFIGFILE], true && !useDevGrp);
		wkeGrid->EnableProperty(propIds[WKE_PROPID_STR_ERWEITERUNG], true && !useDevGrp);
		wkeGrid->EnableProperty(propIdi[WKE_PROPID_INT_DYNPW], true && !useDevGrp);
		break;
	default:
		break;
	}

	// User/PW Settings
	bool dynpw = leseIntSetting(WKE_PROPID_INT_DYNPW);
	bool visible = leseIntSetting(WKE_PROPID_INT_VISIBLEPW);
	int ctrlStrUPW[] = {WKE_PROPID_STR_USER, WKE_PROPID_STR_LOPW, WKE_PROPID_STR_ENPW};

	for (int i = 0; i < 3; i++)
	{
		wkeGrid->EnableProperty(propIds[ctrlStrUPW[i]], !(useDevGrp || dynpw));
		wkeGrid->SetPropertyAttribute(propIds[ctrlStrUPW[i]], wxPG_STRING_PASSWORD, !visible);
	}
	if (dgp != NULL)
	{
		dgp->setVisible(visible);
	}


	// Modus Settings
	if (leseIntSetting(WKE_PROPID_INT_MODUS) < 2)
	{
		einstellungenInit(WKE_PROPID_INT_TYPE, 1);
	}

	else if (leseIntSetting(WKE_PROPID_INT_MODUS) == 4)
	{
		einstellungenInit(WKE_PROPID_INT_TYPE, 0);
	}

	// Logsy Settings
	if (!leseIntSetting(WKE_PROPID_INT_RAUTE) && leseIntSetting(WKE_PROPID_INT_LOGSY))
	{
		einstellungenInit(WKE_PROPID_INT_LOGSY, 0);
	}	

}


void CWkePropGrid::setFarbe(bool status)
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

	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_DEVGRPFILE], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIdi[WKE_PROPID_INT_DEVGRP], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_HOSTLFILE], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_CONFIGFILE], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIdi[WKE_PROPID_INT_CONFIGFILEOPTION], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIdi[WKE_PROPID_INT_MODUS], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIdi[WKE_PROPID_INT_TYPE], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIdi[WKE_PROPID_INT_SHOW], wxColour(farbe));
	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_ODIR], wxColour(farbe));

	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_DYNCONFIGFOLDER], wxColour("#ffffff"));
	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_DEFCONFIGFILE], wxColour("#ffffff"));
	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_ERWEITERUNG], wxColour("#ffffff"));
	
	wkeGrid->SetPropertyBackgroundColour(propIds[WKE_PROPID_STR_LOGFILE], wxColour("#ffffff"));
}


void CWkePropGrid::sets(DevGrpPanel *devgrppnl)
{
	dgp = devgrppnl;
}