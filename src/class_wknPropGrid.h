#pragma once
#include "stdwx.h"

#include <wx/propgrid/manager.h>

#include <string>

class CWknPropGrid :
	public wxPanel
{
public:
	CWknPropGrid(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CWknPropGrid(void);
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
	void setFarbe(							// zum Ein/Ausschalten der Markierungen f�r die wichtigsten Einstellungen
		bool status								// TRUE: Markieren; FALSE: Keine Markierung
		);
	int getSplitterPos();					// zum Feststellen der SplitterPosition
	void setSplitterPos(					// zum Setzen der SplitterPosition
		int pos									// Position
		);

	enum PROPID_S {							// STRING Property IDs
		WKN_PROPID_STR_DATAFILE,
		WKN_PROPID_STR_CONFIGFILE,
		WKN_PROPID_STR_OUTPUTDIR,
		WKN_PROPID_STR_FILENAMEVAR,
		WKN_PROPID_STR_ADDSEP,
		WKN_PROPID_STR_REPLACELF,
		WKN_PROPID_STR_LOGFILE
	};
	enum PROPID_I {							// INT Property IDs
		WKN_PROPID_INT_NONSFORMAT,
		WKN_PROPID_INT_FLEXDF,
		WKN_PROPID_INT_USETAGS,
		WKN_PROPID_INT_CYCLECOUNT,
		WKN_PROPID_INT_USESEP,
		WKN_PROPID_INT_MODUS,
		WKN_PROPID_INT_APPEND
	};

private:
	wxPropertyGridManager *wknGrid;
	wxPGId propIds[7];					// String Settings
	wxPGId propIdi[7];					// INT Settings
	int pageID;							// wxProgGrid Page ID
	wxWindow *parent;					// Parent Window zum Versenden f�r Nachrichten
	void OnPropertyGridChange(			// Zeigt an, wenn eine Option ge�ndert wurde
		wxPropertyGridEvent& event
		);
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};
