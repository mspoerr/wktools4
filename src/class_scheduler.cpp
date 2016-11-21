#include "class_scheduler.h"

#include <wx/stdpaths.h>


#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
namespace bg = boost::gregorian;
namespace bt = boost::posix_time;


DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHEDULER_FERTIG)

BEGIN_EVENT_TABLE(Scheduler, wxEvtHandler)
EVT_COMMAND(wxID_ANY, wkEVT_SCHEDULER_FERTIG, Scheduler::OnSchedulerFertig)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Scheduler::Scheduler() : wxEvtHandler()
{
	schedProps = NULL;
	logAusgabe = NULL;
	wkMain = NULL;
	stopThread = false;
	toolID = 0;
	startZeit = "";
	startDatum = "";
}


// Destruktor:
Scheduler::~Scheduler()
{
}


void Scheduler::ladeSchedProfile()
{
	wxArrayString profile;
	profile.Empty();

	std::string tool = schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_TOOL);

	if (tool == "ConfigMaker")
	{
		profile = wkMain->erstellen->ladeProfile();
	}
	else if (tool == "Configure Devices")
	{
		profile = wkMain->einspielen->ladeProfile();
	}
	else if (tool == "IP List")
	{
		profile = wkMain->iplist->ladeProfile();
	}
	else if (tool == "Mapper")
	{
		profile = wkMain->mapper->ladeProfile();
	}
	
	schedProps->ladeProfile(profile);
}


void Scheduler::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Scheduler::sets(TiXmlDocument *document, bool guiStats)
{
	doc = document;

	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;

}


void Scheduler::guiSets(CSchedPropGrid *schedProp, WkLog *ausgabe, SchedPanel *sp)
{
	schedProps = schedProp;
	logAusgabe = ausgabe;
	schedPanel = sp;

	ladeSchedProfile();
}


void Scheduler::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
	WkLog::evtData evtDat;
	evtDat.farbe = farbe;
	evtDat.format = format;
	evtDat.groesse = groesse;
	evtDat.logEintrag = logEintrag;
	evtDat.logEintrag2 = log2;
	evtDat.type = type;

	LogEvent evt(EVT_LOG_MELDUNG);
	evt.SetData(evtDat);

	if (logAusgabe != NULL)
	{
		wxPostEvent(logAusgabe, evt);
	}
}


bool Scheduler::nameEindeutig(std::string name)
{	
	for (int i = 0; i < schedPanel->schedAnsicht->GetNumberRows(); i++)
	{
		if (schedPanel->schedAnsicht->GetCellValue(i, 2) == name)
		{
			return false;
		}
	}

	return true;
}


bool Scheduler::neuerEintrag()
{
	std::string fehler = "";
	std::string eName = schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_NAME);
	
	if (eName == "")
	{
		fehler = "5301: Name field is empty. Please specify a unique name for the Scheduler entry!";

		schreibeLog(WkLog::WkLog_ZEIT, fehler, "5301", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return false;

	}
	else if (nameEindeutig(eName))
	{
		int zeile = schedPanel->schedAnsicht->GetNumberRows();
		eintragErstellen(zeile, true, eName);
	}
	else if (aenderung)
	{
		eintragErstellen(aendereZeile, false, eName);
	}
	else
	{
		fehler = "5302: Name value is not unique. Please specify a unique name for the Scheduler entry!";

		schreibeLog(WkLog::WkLog_ZEIT, fehler, "5302", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);

		return false;
	}
	
	return true;
}


bool Scheduler::eintragErstellen(int zeile, bool neueZeile, std::string eName)
{
	bool dt = false;		// DateTime vorhanden

	std::string eWH = schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_REC);
	if (eWH != "On Demand")
	{
		// Check Zeiteingabe
		if (!checkDateTimeVal(schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_TIME), 1))
		{
			return false;
		}
		else
		{
			dt = true;
		}
	}

	if (neueZeile)
	{
		schedPanel->zeileHinzu();
	}

	schedPanel->schedAnsicht->SetCellValue(zeile, SchedPanel::wk_SCHED_TABLE_STATE, "-- READY --");
	schedPanel->schedAnsicht->SetCellValue(zeile, SchedPanel::wk_SCHED_TABLE_NAME, eName);
	schedPanel->schedAnsicht->SetCellValue(zeile, SchedPanel::wk_SCHED_TABLE_TOOL, schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_TOOL));
	schedPanel->schedAnsicht->SetCellValue(zeile, SchedPanel::wk_SCHED_TABLE_PROFILE, schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_PROFILE));
	schedPanel->schedAnsicht->SetCellValue(zeile, SchedPanel::wk_SCHED_TABLE_REPEAT, schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_REC));

	if (dt)
	{
		schedPanel->schedAnsicht->SetCellValue(zeile, SchedPanel::wk_SCHED_TABLE_TIME, schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_TIME));

	}
	else
	{
		schedPanel->schedAnsicht->SetCellValue(zeile, SchedPanel::wk_SCHED_TABLE_TIME, "");
	}
	
	aenderung = false;
}


bool Scheduler::checkDateTimeVal(std::string dateTime, int type)
{
	if (!type)
	{
	}
	else
	{
		// Zeit
		try
		{
			bt::ptime t(bt::time_from_string(dateTime));
		}
		catch (std::exception& e)
		{
			std::string fehler = "5303: Wrong format for TIME. Please use CCYY-MM-DD HH:MM:SS.mmm!";

			schreibeLog(WkLog::WkLog_ZEIT, fehler, "5303", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);

			return false;
		}
	}
	return true;
}


bool Scheduler::aktualisiereSchedTabelle()
{
	if (testeListenFile(schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_SCHEDFILE)))
	{
		// Nicht OK
		return false;
	}
	else
	{
		// OK
		if (schedPanel->schedAnsicht->GetNumberRows())
		{
			schedPanel->schedAnsicht->ClearGrid();
			schedPanel->schedAnsicht->DeleteRows(0, schedPanel->schedAnsicht->GetNumberRows()-1);
			schedPanel->schedAnsicht->unCheck(true);
		}
		std::ifstream ganzeDatei(schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_SCHEDFILE).c_str(), std::ios_base::in);
		std::string zeile;
		size_t pos1, pos2;
		int zaehler = 0;
		while (!ganzeDatei.eof())
		{
			getline(ganzeDatei, zeile, '\n');

			if (zeile.size() > 5)
			{
				if (zaehler == schedPanel->schedAnsicht->GetNumberRows())
				{
					schedPanel->zeileHinzu();
				}
				schedPanel->schedAnsicht->SetCellValue(zaehler, SchedPanel::wk_SCHED_TABLE_CHECK, "");

				pos1 = 0;
				pos2 = 0;
				std::string temp = "";
				for (int i = 0, j = 0; j < schedSPanzahl; i++, j++)
				{
					pos2 = zeile.find(";", pos1);
					if (pos2 == zeile.npos)
					{
						break;
					}

					temp = zeile.substr(pos1, pos2-pos1);
					pos1 = pos2+1;
					if (temp == "x")
					{
						schedPanel->schedAnsicht->SetCellValue(zaehler, SchedPanel::wk_SCHED_TABLE_CHECK, "1");
					}
					else
					{
						// Kompatibilität zu Version 3 Files -> Wenn Spalte Datum erreicht wird, dann wird sie übersprungen
						if (j == 5)
						{
							i--;
							continue;
						}
						schedPanel->schedAnsicht->SetCellValue(zaehler, i, temp.c_str());
					}
				}
			}
			zaehler++;
		}
	}

	schedPanel->schedAnsicht->ForceRefresh();
	return true;
}

// Scheduler File testen; 
// Zur Kompatibilität mit Version 3 File gibt es 8 Spalten, obwohl die Datumsspalte nicht mehr benötigt wird.
// Die Datumsspalte wird dann ausgelassen
int Scheduler::testeListenFile(std::string listfile)
{
	if (listfile == "")
	{
		return 10;
	}
	std::string lifi;
	std::ifstream eingabe(listfile.c_str(), std::ios_base::in);
	getline(eingabe, lifi, '\0');
	eingabe.close();

	
	// Vor den Tests: Zum Schluss muss ein Enter vorkommen
	lifi += "\n";


	std::string fehlerMeldung = "";

	// Test 1: Ist überhaupt ein ";" vorhanden
	size_t letzterSP = lifi.rfind(";");
	if (letzterSP == lifi.npos)
	{
		fehlerMeldung = "5304: Scheduler: FILE inconsistent! -> no \";\" found";

		schreibeLog(WkLog::WkLog_ZEIT, fehlerMeldung, "5304", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);

		return 1;
	}

	// Test 1.1:
	// Falls das Listen File im Excel editiert wurde, sind am Zeilenende keine ";"
	// -> Den Test 3 ändern -> "modulo 7" statt "modulo 8"
	size_t zeilenende = lifi.find("\n");
	size_t strichpunkt = lifi.rfind(";", zeilenende);
	int lifiSPanzahl = 0;
	if (strichpunkt +2 < zeilenende)
	{
		lifiSPanzahl = 7;
	}
	else
	{
		lifiSPanzahl = 8;
	}

	// Test 1.2
	// Ebenso kann es vorkommen, dass die erste Spalte fehlt
	// -> hinzufügen
	size_t erstesZeichen = lifi.find_first_of("x;");
	if (erstesZeichen > 0)
	{
		fehlerMeldung = "5305: Scheduler: FILE inconsistent! -> First Column (Checkboxes) missing!";

		schreibeLog(WkLog::WkLog_ZEIT, fehlerMeldung, "5305", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);

		return 4;
	}

	// Test 2: Anzahl der ";" modulo Zeilen muss 0 sein

	// Zählen der ";"
	long spAnzahl = 0;
	for (size_t i = 0; i < lifi.size(); i++)
	{
		if (lifi[i] == ';')
		{
			spAnzahl++;
		}
	}

	// Zählen der Zeilen. Etwaige Leerzeilen am Schluss werden gelöscht.
	letzterSP = lifi.find("\n", letzterSP);
	lifi.erase(letzterSP+1, lifi.npos);

	long zeilenAnzahl = 0;
	for (size_t i = 0; i < lifi.size(); i++)
	{
		if (lifi[i] == '\n')
		{
			zeilenAnzahl++;
		}
	}

	if (spAnzahl%zeilenAnzahl)
	{
		fehlerMeldung = "5306: Scheduler: FILE inconsistent! -> Not all rows have the same number of columns or \";\" is missing at the end of some rows";
		schreibeLog(WkLog::WkLog_ZEIT, fehlerMeldung, "5306", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 2;
	}

	// Test 3: Anzahl der ";" modulo 8 muss 0 sein
	if (spAnzahl%lifiSPanzahl)
	{
		fehlerMeldung = "5307: Scheduler: FILE inconsistent! -> Not all rows have 8 columns";
		schreibeLog(WkLog::WkLog_ZEIT, fehlerMeldung, "5307", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 3;
	}
	schedSPanzahl = lifiSPanzahl;


	return 0;
}


void Scheduler::ladeSchedFile(wxString filename)
{
	schedProps->einstellungenInit(CSchedPropGrid::SCHED_PROPID_STR_SCHEDFILE, filename.c_str());
	aktualisiereSchedTabelle();
}


void Scheduler::sichereSchedFile(wxString filename)
{
	schedProps->einstellungenInit(CSchedPropGrid::SCHED_PROPID_STR_SCHEDFILE, filename.c_str());
	sichereSchedTabelle();
}


bool Scheduler::sichereSchedTabelle()
{
	std::string filename = schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_SCHEDFILE);

	if (filename == "")
	{
		return false;
	}

	std::ofstream schedAusgabe;
	schedAusgabe.rdbuf()->open(filename.c_str(), std::ios_base::out);
	for (int i = 0; i < schedPanel->schedAnsicht->GetNumberRows(); i++)
	{
		for (int j = 0; j < 7; j++)
		{
			if (j == SchedPanel::wk_SCHED_TABLE_CHECK)
			{
				std::string chkbxStat = schedPanel->schedAnsicht->GetCellValue(i, j).c_str();
				if (chkbxStat != "")
				{
					schedAusgabe << "x";
				}
				schedAusgabe << ";";
			}
			else if (j == 5)
			{
				std::string siTxt = schedPanel->schedAnsicht->GetCellValue(i, j).c_str();
				schedAusgabe << ";" << siTxt << ";";
			}
			else
			{
				std::string siTxt = schedPanel->schedAnsicht->GetCellValue(i, j).c_str();
				schedAusgabe << siTxt << ";";
			}
		}
		schedAusgabe << "\n";
	}
	schedAusgabe.close();

	return true;
}


void Scheduler::statusAendern()
{
	for (int i = 0; i < schedPanel->schedAnsicht->GetNumberRows(); i++)
	{
		std::string chkbxStat = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_CHECK).c_str();
		if (chkbxStat != "")
		{
			std::string wh = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_REPEAT).c_str();
			if (wh == "On Demand")
			{
				schedPanel->schedAnsicht->SetCellValue(i, SchedPanel::wk_SCHED_TABLE_STATE, "-- RUN NOW --");
			}
			else
			{
				schedPanel->schedAnsicht->SetCellValue(i, SchedPanel::wk_SCHED_TABLE_STATE, "-- SCHEDULED --");
			}
		}
	}
}


void Scheduler::bearbeiteEintrag(int selRow)
{
	std::string name = schedPanel->schedAnsicht->GetCellValue(selRow, SchedPanel::wk_SCHED_TABLE_NAME).c_str();
	
	schedProps->einstellungenInit(CSchedPropGrid::SCHED_PROPID_STR_TOOL, 
		schedPanel->schedAnsicht->GetCellValue(selRow, SchedPanel::wk_SCHED_TABLE_TOOL).c_str());

	ladeSchedProfile();

	schedProps->einstellungenInit(CSchedPropGrid::SCHED_PROPID_STR_NAME, name);
	schedProps->einstellungenInit(CSchedPropGrid::SCHED_PROPID_STR_TIME, 
		schedPanel->schedAnsicht->GetCellValue(selRow, SchedPanel::wk_SCHED_TABLE_TIME).c_str());
	schedProps->einstellungenInit(CSchedPropGrid::SCHED_PROPID_STR_REC, 
		schedPanel->schedAnsicht->GetCellValue(selRow, SchedPanel::wk_SCHED_TABLE_REPEAT).c_str());
	schedProps->einstellungenInit(CSchedPropGrid::SCHED_PROPID_STR_PROFILE, 
		schedPanel->schedAnsicht->GetCellValue(selRow, SchedPanel::wk_SCHED_TABLE_PROFILE).c_str());

	aendereZeile = selRow;
	aenderung = true;
}


void Scheduler::loescheEintrag(int selRow)
{
	if (schedPanel->schedAnsicht->GetCellValue(selRow, SchedPanel::wk_SCHED_TABLE_REPEAT) != "On Demand")
	{
		std::string name = schedPanel->schedAnsicht->GetCellValue(selRow, SchedPanel::wk_SCHED_TABLE_NAME).c_str();

#ifdef _WINDOWS_
		std::string sTaskCommand = "schtasks /Delete /TN ";
		sTaskCommand += "wktools-" + name;
		sTaskCommand += " /F";

		std::string schedReturn = "5611: ";

		char psBuffer[128];
		FILE *chkdsk;
		if((chkdsk = _popen(sTaskCommand.c_str(), "rt")) == NULL )
		{
			std::string fehlermeldung = "5312: Failed to schedule task - Scheduled Item Name: " + name;
		}

		while( !feof( chkdsk ) )
		{
			if( fgets( psBuffer, 128, chkdsk ) != NULL )
				schedReturn += psBuffer;
		}
		/* Close pipe and print return value of CHKDSK. */
		_pclose( chkdsk );
		schreibeLog(WkLog::WkLog_ZEIT, schedReturn, "5312", WkLog::WkLog_TUERKIS, WkLog::WkLog_FETT);
#else
		// TODO: Linux Cron Job löschen
#endif
	}

	schedPanel->schedAnsicht->DeleteRows(selRow);
}


void Scheduler::doIt()
{
	stopThread = false;
	schreibeLog(WkLog::WkLog_ZEIT, "5602: Starting Scheduler\r\n", "5602", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	thrd = boost::thread(boost::bind(&Scheduler::startTasks,this));
}


void Scheduler::startTasks()
{
	toolID = 0;
	statusAendern();

	std::string cronCommand = "";
	std::string sTaskCommand = "schtasks /Create /TN ";

	for (int i = 0; i < schedPanel->schedAnsicht->GetNumberRows(); i++)
	{
		std::string name = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_NAME).c_str();

		std::string checkStat = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_CHECK).c_str();
		if (checkStat != "")
		{
			sTaskCommand += "wktools-" + name;
			if (stopThread)
			{
				schedPanel->schedAnsicht->SetCellValue(i, SchedPanel::wk_SCHED_TABLE_STATE, "xx CANCELLED xx");
				schedPanel->schedAnsicht->ForceRefresh();
			}
			else
			{
				std::string tool = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_TOOL).c_str();
				std::string prof = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_PROFILE).c_str();
				std::string wh = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_REPEAT).c_str();

				bool runnow = true;

				if (wh != "On Demand")
				{
					runnow = false;
				}

				if (runnow)
				{
					std::string meldung = "5604: Processing profile " + tool + " // " + prof;
					schreibeLog(WkLog::WkLog_ZEIT, "5602: Starting Scheduler\r\n", "5602", 
						WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

					schedPanel->schedAnsicht->SetCellValue(i, SchedPanel::wk_SCHED_TABLE_STATE, "-- RUNNING --");
					schedPanel->schedAnsicht->ForceRefresh();
				}
				else
				{
					std::string meldung = "5605: Scheduled profile " + tool + " // " + prof;
					schreibeLog(WkLog::WkLog_ZEIT, "5602: Starting Scheduler\r\n", "5602", 
						WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

					std::string user = "";
					std::string pass = "";
					std::string zeit = "";
					user = schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_USER);
					pass = schedProps->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_PASS);
					zeit = schedPanel->schedAnsicht->GetCellValue(i, SchedPanel::wk_SCHED_TABLE_TIME);
					konvertiereZeit(zeit);

					if (user != "")
					{
						sTaskCommand += " /RU " + user;
					}
					if (pass != "")
					{
						sTaskCommand += " /RP " + pass;
					}

					wxString appPfad = "";
					wxStandardPaths sp;
					appPfad = sp.GetExecutablePath();

					std::string freq = "";
					
					if (wh == "Once")
					{
						sTaskCommand += " /SC ONCE ";
					}
					else if (wh == "Daily")
					{
						sTaskCommand += " /SC DAILY ";
					}
					else if (wh == "Weekly")
					{
						sTaskCommand += " /SC WEEKLY ";
					}
					else if (wh == "Monthly")
					{
						sTaskCommand += " /SC MONTHLY ";
					}
					else
					{
						std::string meldung = "5308: Unknown Repeat Item! - Scheduled Item Name: " + name;
						schreibeLog(WkLog::WkLog_ZEIT, "5602: Starting Scheduler\r\n", "5602", 
							WkLog::WkLog_ROT, WkLog::WkLog_FETT);

						continue;
					}

					schedPanel->schedAnsicht->SetCellValue(i, SchedPanel::wk_SCHED_TABLE_STATE, "-- SCHEDULED --");
					schedPanel->schedAnsicht->ForceRefresh();
#ifdef _WINDOWS_
					sTaskCommand += " /TR \"\\\"" + appPfad;
					sTaskCommand += "\\\" -s -S "; 
#else
						// TODO: cron befüllen
#endif
				}

				if (tool == "ConfigMaker")
				{
					if (runnow)
					{
						wkMain->erstellen->doLoad(prof);
						wkMain->erstellen->doIt();
						toolID = Cwktools4Frame::wk_ID_WKN;
						wkMain->erstellen->thrd.join();
						toolID = 0;
					}
					else
					{
#ifdef _WINDOWS_
						sTaskCommand += "-t cfgm -p " + prof;
						sTaskCommand += "\" ";
#else
						// TODO: cron befüllen
#endif
					}
				}
				else if (tool == "Configure Devices")
				{
					if (runnow)
					{
						wkMain->einspielen->doLoad(prof);
						wkMain->einspielen->aktualisiereDevGroup();

						wkMain->einspielen->doIt();
						toolID = Cwktools4Frame::wk_ID_WKE;
						wkMain->einspielen->thrd->join();
						toolID = 0;
					}
					else
					{
#ifdef _WINDOWS_
						sTaskCommand += "-t cfg -p " + prof;
						sTaskCommand += "\" ";
#else
#endif
					}
				}
				else if (tool == "IP List")
				{
					if (runnow)
					{
						wkMain->iplist->doLoad(prof);
						wkMain->iplist->doIt();
						toolID = Cwktools4Frame::wk_ID_WKL;
						wkMain->iplist->thrd.join();
						toolID = 0;
					}
					else
					{
#ifdef _WINDOWS_
						sTaskCommand += "-t ipl -p " + prof;
						sTaskCommand += "\" ";
#else
#endif
					}
				}
				else if (tool == "Mapper")
				{
					if (runnow)
					{
						wkMain->mapper->doLoad(prof);
						wkMain->mapper->doIt();
						toolID = Cwktools4Frame::wk_ID_WKM;
						wkMain->mapper->thrd.join();
						toolID = 0;
					}
					else
					{
#ifdef _WINDOWS_
						sTaskCommand += "-t wkm -p " + prof;
						sTaskCommand += "\" ";
#else
						// TODO: Linux
#endif
					}
				}
				if (runnow)
				{
					schedPanel->schedAnsicht->SetCellValue(i, SchedPanel::wk_SCHED_TABLE_STATE, "-- COMPLETED --");
					schedPanel->schedAnsicht->ForceRefresh();
				}
				else
				{
#ifdef _WINDOWS_
					// TODO: at ommand ausführen und responce parsen wenn möglich
					sTaskCommand += " /ST " + startZeit + " /SD " + startDatum + " /F";
					std::string schedReturn = "5610: ";
					
					char psBuffer[128];
					FILE *chkdsk;
					if((chkdsk = _popen(sTaskCommand.c_str(), "rt")) == NULL )
					{
						std::string fehlermeldung = "5308: Failed to schedule task - Scheduled Item Name: " + name;
					}
					
					while( !feof( chkdsk ) )
					{
						if( fgets( psBuffer, 128, chkdsk ) != NULL )
							schedReturn += psBuffer;
					}
					/* Close pipe and print return value of CHKDSK. */
					_pclose( chkdsk );
					schreibeLog(WkLog::WkLog_ZEIT, schedReturn, "5610", WkLog::WkLog_TUERKIS, WkLog::WkLog_FETT);
#else

#endif
				}
			}
		}
	}

	sichereSchedTabelle();
	
	if (true)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "5603: Scheduler finished\r\n", "5603", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	}

	wxCommandEvent event(wkEVT_SCHEDULER_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);

}


void Scheduler::OnSchedulerFertig(wxCommandEvent &event)
{
	schreibeLog(WkLog::WkLog_ZEIT, "5609: Scheduler END\r\n", "5609", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);

	if (wkMain != NULL)
	{
		wxCommandEvent event(wkEVT_SCHED_FERTIG);
		event.SetInt(1);
		wxPostEvent(wkMain, event);
	}
}


void Scheduler::cancelIt()
{
	switch (toolID)
	{
	case Cwktools4Frame::wk_ID_WKN:
		wkMain->erstellen->cancelIt();
		break;
	case Cwktools4Frame::wk_ID_WKE:
		wkMain->einspielen->cancelIt();
		break;
	case Cwktools4Frame::wk_ID_WKL:
		wkMain->iplist->cancelIt();
		break;
		break;
	case Cwktools4Frame::wk_ID_WKM:
		wkMain->mapper->cancelIt();
		break;
	default:
		break;
	}
	toolID = 0;
	stopThread = true;

	wxCommandEvent event(wkEVT_SCHEDULER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);

}


void Scheduler::kilIt()
{
	switch (toolID)
	{
	case Cwktools4Frame::wk_ID_WKN:
		wkMain->erstellen->kilIt();
		break;
	case Cwktools4Frame::wk_ID_WKE:
		wkMain->einspielen->kilIt();
		break;
	case Cwktools4Frame::wk_ID_WKL:
		wkMain->iplist->kilIt();
		break;
	case Cwktools4Frame::wk_ID_WKM:
		wkMain->mapper->kilIt();
		break;
	default:
		break;
	}
	toolID = 0;
	thrd.interrupt();
	schreibeLog(WkLog::WkLog_ZEIT, "5608: Scheduler killed!\r\n", "5608", 
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);

	wxCommandEvent event(wkEVT_SCHEDULER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);

}


void Scheduler::konvertiereZeit(std::string zeit)
{
	// Haben: CCYY:MM:DD HH:MM:SS.mmm
	// Brauchen: zwei strings - MM/DD/YYYY und HH:MM:SS
	if (zeit.find(".") != zeit.npos)
	{
		zeit.erase(zeit.find("."), zeit.npos);
	}
	startZeit = zeit.substr(zeit.find(" ")+1, zeit.npos);
	startDatum = zeit.substr(0, zeit.find(" "));

	startDatum.erase(4, 1);
	startDatum.erase(6, 1);
	startDatum += startDatum.substr(4, 2);
	startDatum += startDatum.substr(0, 4);
	startDatum.erase(0, 6);
	startDatum.insert(2, "/");
	startDatum.insert(5, "/");
}