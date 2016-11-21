
#pragma once
#include "stdwx.h"
#ifdef WKTOOLS_MAPPER

#include <wx/propgrid/manager.h>

#include <string>

class CWkmPropGrid :
	public wxPanel
{
public:
	CWkmPropGrid(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	virtual ~CWkmPropGrid(void);
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

	enum PROPID_S {							// STRING Property IDs
		WKM_PROPID_STR_SDIR = 0,				// Search Directory
		WKM_PROPID_STR_OFILE,					// Output File
		WKM_PROPID_STR_LOGFILE,					// Log File
		WKM_PROPID_STR_PATTERN,					// Search Pattern
		WKM_PROPID_STR_UNITEDGELENGTH,			// FMMM Ausgabe: Unit Edge Length
		WKM_PROPID_STR_LAYERDISTANCE,			// Hierarchic Ausgabe: Layer Distance
		WKM_PROPID_STR_NODEDISTANCE,			// Hierarchic Ausgabe: Node Distance
		WKM_PROPID_STR_WEIHTBALANCING,			// Hierarchic Ausgabe: Weight Balancing
		WKM_PROPID_STR_HOSTFILE					// File mit Daten nicht erfasster Endgeräte zum Importieren in die Datenbank
	};
	enum PROPID_I {							// INT Property IDs
		WKM_PROPID_INT_PARSE = 0,				// Neu Parsen
		WKM_PROPID_INT_COMBINE,					// Auswerten
		WKM_PROPID_INT_CDP,						// CDP NW Geräte Infos verwenden
		WKM_PROPID_INT_CDPHOSTS,				// CDP Endgeräte Infos verwenden
		WKM_PROPID_INT_UNKNOWNSTP,				// STP Unknown Infos verwenden
		WKM_PROPID_INT_L2,						// L2 Topologie ausgeben
		WKM_PROPID_INT_L3,						// L3 Topologie ausgeben
		WKM_PROPID_INT_ORGANIC,					// Organische Ausgabe
		WKM_PROPID_INT_HIERARCHIC,				// Hierarchische Ausgabe
		WKM_PROPID_INT_ORTHO,					// Orthogonal Layout
		WKM_PROPID_INT_CLEARDB,					// Datenbank, bis auf den letzten Durchlauf löschen
		WKM_PROPID_INT_CLEARDB_ALL,				// Die komplette DB löschen
		WKM_PROPID_INT_IMPORT,					// Daten importieren
		WKM_PROPID_INT_DNS_RESOLUTION,			// DNS Auflösung der Engeräte
		WKM_PROPID_INT_ENDSYSTEMS,				// Endsysteme im Visio ausgeben
		WKM_PROPID_INT_IMP_ES,					// Endsysteme in die device Table eintragen (Parser)
		WKM_PROPID_INT_INMEMDB,					// In-Memory DB
		WKM_PROPID_INT_INTFSTYLE,				// Interface Style
		WKM_PROPID_INT_L3_ROUTING,				// L3 Routing Neughborship Topologie ausgeben
		WKM_PROPID_INT_ROUTENEIGHBORS,			// RoutingProtokoll Nachbaren und Next-Hop IP Nachbaren im Visio ausgeben
		WKM_PROPID_INT_INMEMDBNEW				// In-Memory DB - New (for xml setting without hyphen)
};

private:
	wxPropertyGridManager *wkmGrid;
	wxPGId propIds[9];					// String Settings
	wxPGId propIdi[21];					// Int Settings
	int pageID;							// wxProgGrid Page ID
	void OnPropertyGridChange(			// Zeigt an, wenn eine Option geändert wurde
		wxPropertyGridEvent& event
		);

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};

#endif