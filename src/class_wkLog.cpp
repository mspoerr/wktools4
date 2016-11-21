#include "class_wkLog.h"


//#include <wx/datetime.h>

DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHREIBELOG)
DEFINE_EVENT_TYPE( EVT_LOG_MELDUNG )


BEGIN_EVENT_TABLE(WkLog, wxPanel)
	EVT_LOG(wxID_ANY, WkLog::OnSchreibeLog)
END_EVENT_TABLE()

WkLog::WkLog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxPanel(parent, id, pos, size, style, name)
{
	logFenster = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_RICH|wxTE_RICH2|wxTE_DONTWRAP);
	logFenster->Hide();
	
	debugFenster = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_RICH|wxTE_RICH2|wxTE_DONTWRAP);
	//debugFenster->Show(true);

	wxFont txt(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
	logFenster->SetDefaultStyle(wxTextAttr("#000080", *wxWHITE, txt, wxTEXT_ALIGNMENT_DEFAULT));
	debugFenster->SetDefaultStyle(wxTextAttr("#000080", *wxWHITE, txt, wxTEXT_ALIGNMENT_DEFAULT));

	schreibeLog(WkLog_ZEIT, "WKTOOLS LOG WINDOW\r\n\r\n", "", 0, WkLog_WKTOOLS, WkLog_FETT, 12);

	gs = new wxFlexGridSizer(1, 2, 0, 0);

	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);
	gs->Add(debugFenster, 0, wxALL|wxEXPAND, 0);

	this->SetSizer(gs);
	this->SetAutoLayout(true);
	this->Layout();

}

WkLog::~WkLog(void)
{
	delete logFenster;
	delete debugFenster;
}


void WkLog::schreibeLog(int type, std::string logEintrag, std::string log2, int report, int farbe, int format, int groesse, int severity)
{
	zeitangabe = false;

	wxFont txt(groesse, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Lucida Console");
	wxTextAttr txtAttr("#000080", *wxWHITE, txt);
	wxColour textfarbe("#000080");

	time_t zeit = time(NULL);
	char *fehlerZeit = asctime(localtime(&zeit));
	fehlerZeit[24] = 0x00;

	std::string eintrag = "";

	switch (type)
	{
	case WkLog_ABSATZ:
		eintrag = "\r\n-------------------------------------------------------\r\n";
		eintrag += fehlerZeit;
		eintrag += ":\t";
		zeitangabe = true;
		break;
	case WkLog_ZEIT:
		eintrag = "\r\n";
		eintrag += fehlerZeit;
		eintrag += ":\t";
		zeitangabe = true;
		break;
	case WkLog_NORMALTYPE:
	default:
		break;
	}
	eintrag += logEintrag;

	switch (farbe)
	{
	case WkLog_WKTOOLS:
		textfarbe = "#000080";
		break;
	case WkLog_SCHWARZ:
		textfarbe = "#000000";
		break;
	case WkLog_GRUEN:
		textfarbe = "#008000";
		break;
	case WkLog_ROT:
		textfarbe = "#ff0000";
		break;
	case WkLog_BLAU:
		textfarbe = "#0000ff";
		break;
	case WkLog_TUERKIS:
		textfarbe = "#008080";
		break;
	case WkLog_GRAU:
		textfarbe = "#808080";
		break;
	case WkLog_DUNKELROT:
		textfarbe = "#dc143c";
		break;
	case WkLog_HELLGRUEN:
		textfarbe = "#00ff00";
		break;
	default:
		break;
	}
	txtAttr.SetTextColour(textfarbe);

	switch (format)
	{
	case WkLog_NORMALFORMAT:
		txt.SetWeight(wxFONTWEIGHT_NORMAL);
		break;
	case WkLog_FETT:
		txt.SetWeight(wxFONTWEIGHT_BOLD);
		break;
	case WkLog_KURSIV:
		txt.SetStyle(wxFONTSTYLE_ITALIC);
		break;
	case WkLog_UNTERS:
		txt.SetUnderlined(true);
		break;
	default:
		break;
	}
	txtAttr.SetFont(txt);

	int dPos1 = debugFenster->GetLastPosition();
	int dPos2 = dPos1 + eintrag.length();

	debugFenster->AppendText(eintrag);
	debugFenster->SetStyle(dPos1, dPos2, txtAttr);
	debugFenster->Update();

	if (zeitangabe || report)
	{
		int lPos1 = logFenster->GetLastPosition();
		int lPos2 = lPos1 + eintrag.length();
		logFenster->AppendText(eintrag);
		logFenster->SetStyle(lPos1, lPos2, txtAttr);
		logFenster->Update();
	}
}


void WkLog::schreibeLogDummy(int typeDummy, std::string logDummy, std::string log2, int reportDummy, int farbeDummy, int formatDummy, int groesseDummy, int severity)
{

}


void WkLog::OnSchreibeLog(LogEvent &event)
{
	boost::any data = event.GetData();
	if (data.type() == typeid(evtData))
	{
		evtData evtDat = event.GetData<evtData>();
		
		int doNotLog = wkFehler->getFehlerStatus(evtDat.logEintrag2);
		if (doNotLog == 0)
		{
			if (evtDat.report != 1)
			{
				evtDat.report = 0;
			}

			schreibeLog(evtDat.type, evtDat.logEintrag, evtDat.logEintrag2, evtDat.report, evtDat.farbe, evtDat.format, evtDat.groesse);
		}
	}
}


void WkLog::keinDebug(bool einAus)
{
	if (einAus)
	{
		debugFenster->Hide();
		logFenster->Show(true);

		gs->Clear();
		gs->Add(logFenster, 0, wxALL|wxEXPAND, 0);
		this->Layout();
	}
	else
	{
		logFenster->Hide();
		debugFenster->Show(true);
		
		gs->Clear();
		gs->Add(debugFenster, 0, wxALL|wxEXPAND, 0);
		this->Layout();
	}
}


std::string WkLog::getLogInhalt()
{
	std::string ret = logFenster->GetRange(0, logFenster->GetLastPosition()).c_str();
	return ret;
}