#include "class_settings.h"

#include <wx/propgrid/advprops.h>

BEGIN_EVENT_TABLE(CSettings, wxPanel)
	EVT_PG_CHANGED(wxID_ANY, CSettings::OnPropertyGridChange)
END_EVENT_TABLE()

CSettings::CSettings(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	glSettings = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxPG_DESCRIPTION|wxPG_DEFAULT_STYLE);
	
	glSettings->SetExtraStyle(wxPG_EX_GREY_LABEL_WHEN_DISABLED);

	glSettings->AddPage(wxT("Global Settings"));

	setId[WK_SETS_SETTINGS] = glSettings->Append(new wxPropertyCategory(wxT("Global Settings"), wxT("gls")));

	setId[WK_SETS_UPDATE] = glSettings->Append(new wxBoolProperty(wxT("Automatic Version Check"), wxPG_LABEL, true));
	setId[WK_SETS_MARKIERUNG] = glSettings->Append(new wxBoolProperty(wxT("Highlight mandatory options"), wxT("highlight"), true));
	setId[WK_SETS_XMLLOC] = glSettings->Append(new wxFileProperty(wxT("wktools.xml Location"), wxPG_LABEL));
	setId[WK_SETS_FUTURE] = glSettings->Append(new wxStringProperty(wxT("For future use"), wxPG_LABEL, "wktools4"));
	setId[WK_SETS_REPORT] = glSettings->Append(new wxBoolProperty(wxT("Enable/Disable Report"), wxT("Report"), true));
	setId[WK_SETS_REPORTFILE] = glSettings->Append(new wxFileProperty(wxT("Report File"), wxPG_LABEL));
	setId[WK_SETS_FONT] = glSettings->Append(new wxFontProperty(wxT("Settings Font"), wxT("fontset"), GetFont()));

	wxPGId sqlId = glSettings->Append(new wxPropertyCategory(wxT("SQL Settings"), wxT("sqls")));
	setId[WK_SETS_SQL] = glSettings->AppendIn(sqlId, new wxBoolProperty(wxT("Write Errors into SQL database"), wxPG_LABEL, false));
	setId[WK_SETS_SQLSRV] = glSettings->AppendIn(sqlId, new wxStringProperty(wxT("SQL Server"), wxPG_LABEL, ""));
	setId[WK_SETS_SQLUSER] = glSettings->AppendIn(sqlId, new wxStringProperty(wxT("SQL User"), wxPG_LABEL, ""));
	setId[WK_SETS_SQLPASS] = glSettings->AppendIn(sqlId, new wxStringProperty(wxT("SQL Password"), wxPG_LABEL, ""));

#ifndef USESQL
	setId[WK_SETS_SQL]->Hide(true);
	setId[WK_SETS_SQLSRV]->Hide(true);
	setId[WK_SETS_SQLUSER]->Hide(true);
	setId[WK_SETS_SQLPASS]->Hide(true);
	sqlId->Hide(true);
#endif
	
	
	glSettings->SetPropertyHelpString(setId[WK_SETS_SETTINGS], 
		"For all tools");
	glSettings->SetPropertyHelpString(setId[WK_SETS_UPDATE], 
		"Enable (TRUE) or Disable (FALSE) Automatic Version check. If enabled, wktools downloads version4.txt from www.spoerr.org to see, if a newer version is available");
	glSettings->SetPropertyHelpString(setId[WK_SETS_FONT], 
		"Font settings for the Tools Property windows");
	glSettings->SetPropertyHelpString(setId[WK_SETS_MARKIERUNG], 
		"When selected, the most important settings are marked.");

	glSettings->SetPropertyCell(setId[WK_SETS_SETTINGS], 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));

	int dbh = glSettings->GetDescBoxHeight();

	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(glSettings, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();
}

CSettings::~CSettings(void)
{
}


wxFont CSettings::getFont()
{
	wxVariant bla = glSettings->GetPropertyValue("fontset");
	const wxFont *txfont = wxGetVariantCast(bla, wxFont);		

	return *txfont;
}


void CSettings::resetFont(wxFont font)
{
	wxVariant fvar;
	fvar << font;
	glSettings->ChangePropertyValue(setId[WK_SETS_FONT], fvar);
	glSettings->SetFont(font);
}


void CSettings::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	// Get name of changed property
	const wxString& name = event.GetPropertyName();
	if (name == "fontset")
	{
		wxVariant bla = glSettings->GetPropertyValue("fontset");
		const wxFont *txfont = wxGetVariantCast(bla, wxFont);		
		glSettings->SetFont(*txfont);
		
		wxCommandEvent event(wkEVT_FONTAENDERUNG);
		event.SetInt(1);
		//wkMain->ProcessEvent(event);
		
		wxPostEvent(wkMain, event);
	}

	if (name == "highlight")
	{
		int hl = glSettings->GetPropertyValueAsBool(setId[WK_SETS_MARKIERUNG]);
		wxCommandEvent event(wkEVT_HIGHLIGHT);
		event.SetInt(hl);

		wxPostEvent(wkMain, event);
	}

	if (name == "Report")
	{
		bool report = glSettings->GetPropertyValueAsBool(setId[WK_SETS_REPORT]);
	}
	
}


void CSettings::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


int CSettings::getUpdater()
{
	int ret = glSettings->GetPropertyValueAsBool(setId[WK_SETS_UPDATE]);
	return ret;
}


void CSettings::setUpdater(int uSet)
{
	glSettings->SetPropertyValue(setId[WK_SETS_UPDATE], uSet);
}


int CSettings::getSplitterPos()
{
	wxPropertyGridPage *tempPage = glSettings->GetPage(0);
	int spos = tempPage->GetSplitterPosition(0);
	return spos;
}


void CSettings::setSplitterPos(int pos)
{
	this->SetSize(500,500);
	glSettings->SetSplitterPosition(pos);
}


void CSettings::setHighlight(int farbe)
{
	glSettings->SetPropertyValue(setId[WK_SETS_MARKIERUNG], farbe);
}


int CSettings::getHighlight()
{
	int ret = glSettings->GetPropertyValueAsBool(setId[WK_SETS_MARKIERUNG]);
	return ret;
}


int CSettings::getRepStat()
{
	int ret = glSettings->GetPropertyValueAsBool(setId[WK_SETS_REPORT]);
	return ret;
}


std::string CSettings::getReportFile()
{
	std::string ret = glSettings->GetPropertyValueAsString(setId[WK_SETS_REPORTFILE]).c_str();
	return ret;
}


void CSettings::setXmlLoc(std::string xml)
{
	glSettings->SetPropertyValue(setId[WK_SETS_XMLLOC], xml.c_str());
}


std::string CSettings::getXmlLoc()
{
	std::string ret = glSettings->GetPropertyValueAsString(setId[WK_SETS_XMLLOC]).c_str();
	return ret;
}


void CSettings::setSqlSets(sqlSets ssts)
{
	glSettings->SetPropertyValue(setId[WK_SETS_SQLSRV], ssts.sqlSrv.c_str());
	glSettings->SetPropertyValue(setId[WK_SETS_SQLUSER], ssts.sqlUSr.c_str());
	glSettings->SetPropertyValue(setId[WK_SETS_SQLPASS], ssts.sqlPwd.c_str());
	glSettings->SetPropertyValue(setId[WK_SETS_SQL], ssts.useSql);
}


CSettings::sqlSets CSettings::getSqlSets()
{
	sqlSets ret;
	ret.sqlSrv = glSettings->GetPropertyValueAsString(setId[WK_SETS_SQLSRV]);
	ret.sqlUSr = glSettings->GetPropertyValueAsString(setId[WK_SETS_SQLUSER]);
	ret.sqlPwd = glSettings->GetPropertyValueAsString(setId[WK_SETS_SQLPASS]);
	ret.useSql = glSettings->GetPropertyValueAsBool(setId[WK_SETS_SQL]);

	return ret;
}


std::string CSettings::getExtras()
{
	return glSettings->GetPropertyValueAsString(setId[WK_SETS_FUTURE]).c_str();
}


void CSettings::setExtras(std::string exStr)
{
	glSettings->SetPropertyValue(setId[WK_SETS_FUTURE], exStr.c_str());
}


void CSettings::wkErrGuiErstellen()
{
	setId[WK_SETS_ERRSETS] = glSettings->Append(new wxPropertyCategory(wxT("Error Message Settings"), wxT("errs")));
	glSettings->SetPropertyCell(setId[WK_SETS_ERRSETS], 0, wxPG_LABEL, wxNullBitmap, wxColour("#ffffff"), wxColour("#000080"));
	glSettings->SetPropertyHelpString(setId[WK_SETS_ERRSETS], "Please select error id's you don't want to log\nUnchecked id's will be logged.");

	std::multimap <std::string, int>::iterator errStatIter, errStatAnfang, errStatEnde;
	std::multimap <std::string, std::string>::iterator errDetIter, errDetAnfang, errDetEnde;
	errDetIter = wkMain->wkFehler->errDetail.begin();
	int i = 100;
	for (errStatIter = wkMain->wkFehler->errStatus.begin(); errStatIter != wkMain->wkFehler->errStatus.end(); ++errStatIter, ++errDetIter)
	{
		wxPGId aktid = glSettings->AppendIn(setId[WK_SETS_ERRSETS], new wxBoolProperty(wxT(errStatIter->first), wxPG_LABEL, errStatIter->second));

		glSettings->SetPropertyHelpString(aktid, errDetIter->second.c_str());

		i++;
	}
	glSettings->Collapse(setId[WK_SETS_ERRSETS]);
	glSettings->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);

	this->SetAutoLayout(true);
	this->Layout();

}


void CSettings::wkErrGuiSave()
{
	wxPropertyGridIterator it;

	wxPropertyGrid *tempGrid = glSettings->GetGrid();
	std::multimap <std::string, int>::iterator errStatIter;

	for (it = tempGrid->GetIterator(wxPG_ITERATE_DEFAULT, tempGrid->GetPropertyByName("0201")); !it.AtEnd(); it++)
	{
		wxPGProperty* p = *it;
		wxVariant var = tempGrid->GetPropertyValue(p);
		int ret = var.GetInteger();
		wxString label = tempGrid->GetPropertyLabel(p);
		errStatIter = wkMain->wkFehler->errStatus.find(label.c_str());
		errStatIter->second=ret;
	}


}


void CSettings::setReport(std::string rf, int rs)
{
	glSettings->SetPropertyValue(setId[WK_SETS_REPORTFILE], rf.c_str());
	glSettings->SetPropertyValue(setId[WK_SETS_REPORT], rs);
}