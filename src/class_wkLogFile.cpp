#include "class_wkLogFile.h"

BEGIN_EVENT_TABLE(WkLogFile, wxEvtHandler)
	EVT_LOG(wxID_ANY, WkLogFile::OnLog)
END_EVENT_TABLE()


WkLogFile::WkLogFile(WKErr *wkE) : wxEvtHandler()
{
	wkFehler = wkE;
	reportString = "";
}

WkLogFile::~WkLogFile()
{
	logAusgabe.close();
}

void WkLogFile::logFileSets(std::string logfile)
{
	// Falls die LogAusgabe schon von einem anderen Tool geöffnet wurde
	if (logAusgabe.is_open())
	{
		logAusgabe.close();
	}
	
	logAusgabe.rdbuf()->open(logfile.c_str(), std::ios_base::out | std::ios_base::app);
}

void WkLogFile::OnLog(LogEvent &event)
{
	boost::any data = event.GetData();
	if (data.type() == typeid(WkLog::evtData))
	{
		WkLog::evtData evtDat = event.GetData<WkLog::evtData>();
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

void WkLogFile::schreibeLog(int type, std::string logEintrag, std::string log2, int report, int farbe, int format, int groesse, int severity)
{
	bool schreibeReportString = false;
	time_t zeit = time(NULL);
	char *fehlerZeit = asctime(localtime(&zeit));
	fehlerZeit[24] = 0x00;

	std::string eintrag = "";

	switch (type)
	{
	case WkLog::WkLog_ABSATZ:
		eintrag = "\r\n-------------------------------------------------------\r\n";
		eintrag += fehlerZeit;
		eintrag += ":\t";
		break;
	case WkLog::WkLog_ZEIT:
		eintrag = "\r\n";
		eintrag += fehlerZeit;
		eintrag += ":\t";
		schreibeReportString = true;
		break;
	case WkLog::WkLog_NORMALTYPE:
	default:
		break;
	}
	eintrag += logEintrag;
	
	logAusgabe << eintrag;

	if (schreibeReportString || report)
	{
		reportString += eintrag;
	}
}


std::string WkLogFile::getReportString()
{
	return reportString;
}