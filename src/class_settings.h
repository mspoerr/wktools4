#pragma once
#include "stdwx.h"

#include <wx/propgrid/manager.h>
#include "wktools4Frame.h"

class Cwktools4Frame;			// Vorwärtsdekleration

class CSettings : public wxPanel
{
public:
	CSettings(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CSettings(void);

	wxFont getFont();						// zum Laden der neuen Schriftart Einstellungen
	void resetFont(wxFont font);			// zum Setzen der Schriftarteinstellungen
	void setMainFrame(						// zum Setzen vom MainFrame
		Cwktools4Frame *fp						// MainFrame
		);	
	int getUpdater();						// liefert Updater Einstellungen
	void setUpdater(						// setzt Updater Einstellungen
		int uSet								// Updater Setting
		);						
	enum WK_SETTINGS {						// Setting IDs
		WK_SETS_SETTINGS,
		WK_SETS_ERRSETS,
		WK_SETS_FONT,
		WK_SETS_UPDATE,
		WK_SETS_FUTURE,
		WK_SETS_MARKIERUNG,
		WK_SETS_SQL,						// Fehlerausgabe in SQL Server - JA/NEIN
		WK_SETS_SQLSRV,						// IP Adresse/Hostname vom SQL Server
		WK_SETS_SQLUSER,					// SQL User
		WK_SETS_SQLPASS,					// SQL Passwort für den SQL User
		WK_SETS_XMLLOC,						// wktools.xml Lokation, wenn nicht das lokale verwendet wird
		WK_SETS_REPORT,						// Report Ausgabe ein/ausschalten
		WK_SETS_REPORTFILE					// File, in das der Report geschrieben werden soll; erstellen, wenn nicht vorhanden
	};
	int getSplitterPos();					// zum Feststellen der SplitterPosition
	void setSplitterPos(					// zum Setzen der SplitterPosition
		int pos									// Position
		);
	int getHighlight();						// liefert die Highlight Option Einstellung
	void setHighlight(						// setzt die Highlight Option
		int farbe
		);
	struct sqlSets {						// SQL Server Einstellungen
		bool useSql;
		std::string sqlSrv;
		std::string sqlUSr;
		std::string sqlPwd;
	};

	void setReport(							// Report Einstellungen setzen
		std::string reportFile,					// Filenanme
		int repStat								// Checkbox Status
		);
		
	void setSqlSets(						// SQL-Server Einstellungen setzen
		sqlSets ssts							// Zu setzende Einstellungen
		);
	sqlSets getSqlSets();					// Auslesen der SQL Einstellungen

	std::string getXmlLoc();						// wktools.xml Einstellung lesen
	void setXmlLoc(							// Neues wktools.xml setzen
		std::string xml
		);

	std::string getExtras();				// Zum Auslesen der "For Future Use" Textbox
	std::string getReportFile();			// Zum Auslesen der "ReportFile" Textbox
	int getRepStat();						// liefert die Enable/Disable Report Option Einstellung
	void setExtras(							// Zum Setzen der "For Future Use" Textbox
		std::string exString							// String, der gesetzt werden soll
		);						
	void wkErrGuiErstellen();				// Erstellen der Fehlermeldungsoptionen
	void wkErrGuiSave();					// Fehlermeldungs Einstellungen speichern


private:
	// Any class wishing to process wxWindows events must use this macro
	wxPropertyGridManager *glSettings;
	wxFont schriftart;
	Cwktools4Frame *wkMain;
	wxPGId setId[13];						// String Settings


	DECLARE_EVENT_TABLE()
	void OnPropertyGridChange(wxPropertyGridEvent& event);
};
