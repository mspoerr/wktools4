#pragma once
#include "stdwx.h"
#include "class_devGrpPanel.h"

#include <wx/propgrid/manager.h>

#include <string>

class CWkePropGrid :
	public wxPanel
{
public:
	CWkePropGrid(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CWkePropGrid(void);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		std::string wert								// Wert des Objektes
		);
	void einstellungenInit(					// zum Initialisieren der Einstellungen
		int index,								// Index des zu initialisierenden Objektes
		int wert								// Wert des Objektes
		);
	int leseIntSetting(						// zum Auslesen von INT Werten
		int index								// Index des Objektes
		);
	std::string leseStringSetting(				// zum Auslesen von STRING Werten
		int index								// Index des Objektes
		);
	void resetFont(							// zum Setzen der Schriftart
		wxFont newFont							// neue Schriftart
		);
	void setFarbe(							// zum Ein/Ausschalten der Markierungen für die wichtigsten Einstellungen
		bool status								// TRUE: Markieren; FALSE: Keine Markierung
		);
	int getSplitterPos();					// zum Feststellen der SplitterPosition
	void setSplitterPos(					// zum Setzen der SplitterPosition
		int pos									// Position
		);
	void enableDisable();					// Zum Aktivieren/Deaktivieren von Optionen

	void sets(								// Zum Setzen spezieller Einstellungen
		DevGrpPanel *dgp						// DeviceGroup Panel
		);

	enum PROPID_S {							// STRING Property IDs
		WKE_PROPID_STR_HOSTLFILE = 0,
		WKE_PROPID_STR_CONFIGFILE,
		WKE_PROPID_STR_DYNCONFIGFOLDER,
		WKE_PROPID_STR_DEFCONFIGFILE,
		WKE_PROPID_STR_ERWEITERUNG,
		WKE_PROPID_STR_DEVGRPFILE,
		WKE_PROPID_STR_USER,
		WKE_PROPID_STR_LOPW,
		WKE_PROPID_STR_ENPW,
		WKE_PROPID_STR_ODIR,
		WKE_PROPID_STR_EXEC,
		WKE_PROPID_STR_LOGFILE,
		WKE_PROPID_STR_MHOPCOMMAND,
		WKE_PROPID_STR_PORT,
		WKE_PROPID_STR_SHP,
		WKE_PROPID_STR_HOSTID
	};
	enum PROPID_I {							// INT Property IDs
		WKE_PROPID_INT_MODUS = 0,
		WKE_PROPID_INT_TYPE,
		WKE_PROPID_INT_SHOW,
		WKE_PROPID_INT_TERMSERVER,
		WKE_PROPID_INT_CONFIGFILEOPTION,
		WKE_PROPID_INT_DYNPW,
		WKE_PROPID_INT_VISIBLEPW,
		WKE_PROPID_INT_RAUTE,
		WKE_PROPID_INT_LOGSY,
		WKE_PROPID_INT_DEVGRP,
		WKE_PROPID_INT_SHOWAPPEND,
		WKE_PROPID_INT_LOGINMODE,
		WKE_PROPID_INT_DBG_SEND,
		WKE_PROPID_INT_DBG_RECEIVE,
		WKE_PROPID_INT_DBG_FUNCTION,
		WKE_PROPID_INT_DBG_BUFTEST,
		WKE_PROPID_INT_DBG_HOSTNAME,
		WKE_PROPID_INT_DBG_BUFTESTDET,
		WKE_PROPID_INT_DBG_SHOW,
		WKE_PROPID_INT_DBG_RGX,
		WKE_PROPID_INT_F2301,
		WKE_PROPID_INT_DBG_HEX,
		WKE_PROPID_INT_DBG_SPECIAL1,
		WKE_PROPID_INT_DBG_SPECIAL2,
		WKE_PROPID_INT_SHOWDATE
	};

private:
	wxPropertyGridManager *wkeGrid;
	DevGrpPanel *dgp;					// DeviceGroup Panel, um Visible PW Einstellung zu setzen
	wxPGId propIds[16];					// String Settings
	wxPGId propIdi[25];					// INT Settings
	int pageID;							// wxProgGrid Page ID
	void OnPropertyGridChange(			// Zeigt an, wenn eine Option geändert wurde
		wxPropertyGridEvent& event
		);

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
