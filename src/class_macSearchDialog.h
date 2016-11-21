#pragma once
#include "stdwx.h"
#ifdef WKTOOLS_MAPPER
#include "class_wkmdb.h"
#include "class_wkLog.h"
#include "class_wkmEnd2End.h"

class MacSearchDialog : public wxFrame
{
public:
	MacSearchDialog(const wxString& title);
	void setEinstellungen(
		std::string dbName, 
		WkLog *logA
		);

private:
	void OnClose(wxCloseEvent &event);
	void OnQuit(wxCommandEvent &event);
	void OnClear(wxCommandEvent &event);
	void OnSearch(wxCommandEvent &event);
	void OnEnd2End(wxCommandEvent &event);
	void OnBoxSelect(wxCommandEvent &event);

	std::string macSearch(		// MAC Adresse finden
		std::string macAddress,		// MAC Adresse
		std::string trennzeichen,	// Trennzeichen bei der Rückgabe
		bool popup,					// Fehlermeldungspopup?
		bool history				// History?
		);
	std::string ipSearch(		// IP Adresse finden
		std::string ipAddress		// IP Adresse
		);
	std::string intfSearch(		// Interface finden
		std::string box,			// Box
		std::string intf			// Interface
		);
	std::string csvSearch(		// CSV File Name
		std::string filename
		);
	std::string end2end(		// End2End Analyse
		std::string host1,			// IP 1
		std::string host2			// IP 2
		);

	wxTextCtrl *macEingabe;
	wxTextCtrl *ipEingabe;
	wxTextCtrl *e2eIp1;
	wxTextCtrl *e2eIp2;
	wxTextCtrl *ergebnisAusgabe;
	wxComboBox *boxSelect;
	wxComboBox *intfSelect;
	wxTextCtrl *csvOperation;

	WkmDB *dieDB;				// Datenbank für die Einträge
	WkLog *logAusgabe;			// Logsuagabe Fenster

	void schreibeLog(						// Funktion für die LogAusgabe
		int type,								// Log Type
		std::string logEintrag,					// Text für den Logeintrag
		std::string log2,						// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,		// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);

	DECLARE_EVENT_TABLE()

};

#endif // #ifdef WKTOOLS_MAPPER
