/***************************************************************
* Name:      wktoolsMain.h
* Purpose:   Defines Application Frame
* Author:    Mathias Spoerr (wktools@spoerr.org)
* Created:   2007-11-03
* Copyright: Mathias Spoerr (http://www.spoerr.org/wktools)
* License:
**************************************************************/

#pragma once
#include "stdwx.h"
#include <wx/dcbuffer.h>
#include <wx/toolbar.h>
#include <wx/splitter.h>
#include <wx/notebook.h>

#include "class_wkn.h"
#include "class_wke.h"
#include "class_wkl.h"
#ifdef WKTOOLS_MAPPER
#include "class_wkm.h"
#endif
#include "class_wzrd.h"
#include "class_scheduler.h"
#include "class_wklPropGrid.h"
#include "class_wknPropGrid.h"
#include "class_wkePropGrid.h"
#ifdef WKTOOLS_MAPPER
#include "class_wkmPropGrid.h"
#endif
#include "class_wzrdPropGrid.h"
#include "class_schedulerPropGrid.h"
#include "class_wkHilfe.h"
#include "class_settings.h"
#include "class_wkLog.h"
#include "class_devGrpPanel.h"
#include "class_tabellePanel.h"
#include "class_schedPanel.h"
#include "htmstrings.h"
#include "class_wkErrMessages.h"


class CSettings;			// Vorwärtsdekleration
class Wkn;
class Wke;
class Wkl;
#ifdef WKTOOLS_MAPPER
class Wkm;
#endif
class Wzrd;
class Scheduler;

class Cwktools4Frame : public wxFrame
{
private:
	wxToolBar *m_pToolBar;				// Toolbar
	wxToolBar *toolSelector;			// Tool Selector Bar (linke Seite)
	HtmStrings *htmstrs;				// HTML Seiten
	bool m_horzToolbar;
	bool m_horzText;
	size_t m_rows;
	int toolAlt;						// Index vom angezeigten Tool
	int toolNeu;						// Index vom neuen Tool
	int toolStartId;					// Wenn der Button gedrückt ist, dann darf nur das aktive Tool einen Cancel Event auslösen
	int mainSplitterPos;				// Position vom Hauptsplitter (vertikal)
	int rSplitterPos;					// Position vom Splitter in der rechten Pane vom Hauptsplitter
	wxSplitterWindow *mainSplitter;		// Hauptsplitter
	wxSplitterWindow *rSplitter;		// Untersplitter rechts
	wxWindow *m_replacewindow;			// Das Fenster, das ausgetauscht wird, wenn ein neues Toll angezeigt werden soll
	wxWindow *m_replacewindowR;			// Das R Panel Fenster, das ausgetauscht wird, wenn ein neues Toll angezeigt werden soll
	CWkHilfe *wkHilfe;					// Hilfe Fenster
	Cwktools4Frame *mainFrame;			// Main Frame
	wxComboBox *profilCombo;			// Combo Box in der Toolbar für Profilauswahl
	wxPanel *rPanel;					// Panel für die rechte Seite vom Hauptsplitter
	TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
	bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
	bool special;						// Spezial Tools freischalten? true -> ja
	bool wkmdbg;						// WKM Debug aktivieren
	void dirMapTabelleInit();			// Initialisieren der Dir Mapping Tabelle
	void dirMapTabelleSichern();		// Speichern der Einträge der Dir Mapping Tabelle im wktools.xml


public:
	bool Create(wxWindow *pParent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, 
		long style = wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLIP_CHILDREN, 
		const wxString& name = wxT("Main Frame"));
	
	virtual ~Cwktools4Frame(void);
	void sets(							// wichtige Grundeinstellungen
		TiXmlDocument *document,			// XML Doc von wktools.xml
		bool guiStats,						// Silent oder GUI Mode; Wichtig
		WKErr *wkFehler						// WkError
		);
	WKErr *wkFehler;					// WK Fehlermeldungen
	Wkn *erstellen;						// Erstellen
	Wke *einspielen;					// Einspielen
	Wkl *iplist;						// IP List
#ifdef WKTOOLS_MAPPER
	Wkm *mapper;						// Mapper
#endif
	Wzrd *wizard;						// Wizard
	Scheduler *sched;					// Scheduler
	CWknPropGrid *wknSettings;			// WKN Settings Fenster
#ifdef WKTOOLS_MAPPER
	CWkmPropGrid *wkmSettings;			// WKM Settings Fenster
#endif
	CWkePropGrid *wkeSettings;			// WKE Settings Fenster
	CWklPropGrid *wklSettings;			// IPL Settings
	CWzrdPropGrid *wzrdSettings;		// Wizard Settings
	CSchedPropGrid *schedSettings;		// Scheduler Settings
	CSettings *globalSettings;			// Globale Einstellungen
	WkLog *logAusgabe;					// Log Fenster
	DevGrpPanel *dieDevGrp;				// DeviceGroup Ansicht
	TabellePanel *wknTabelle;			// Tabellenansicht für WKN
	TabellePanel *dipTabelle;			// Tabellenansicht für DIP
	TabellePanel *dirMapTabelle;			// Dir Mapping Tabelle für Linux <-> Windows Interop
	SchedPanel *schedPanel;				// Scheduler Tabelle
	int updateMessage;					// Update Popup?
	
	enum Tools {			// Aufzählung der Tools und wichtiger Buttons in der Toolbar
		wk_ID_NEWS,
		wk_ID_WZRD,
		wk_ID_WKN,
		wk_ID_WKE,
		wk_ID_WKL,
		wk_ID_WKC,
		wk_ID_WKDP,
		wk_ID_WKP,
		wk_ID_WKM,
		wk_ID_SCHED,
		wk_ID_ABOUT,
		wk_ID_LIC,
		wk_ID_DOC,
		wk_ID_SETTINGS,
		wk_ID_PROFILE_LOAD,
		wk_ID_PROFILE_SAVE,
		wk_ID_PROFILE_REMOVE,
		wk_ID_START,
		wk_ID_KILL,
		wk_ID_LOGRED,
		wk_ID_DEVGRP_REFRESH,
		wk_ID_DEVGRP_SAVE,
		wk_ID_HOSTSEARCH,
		wk_ID_MACDBUPDATE,
		wk_ID_XML_SWAP
	};

private:
	void OnClose(wxCloseEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnEraseBackground(wxEraseEvent &event);
	void OnLeftDown(wxMouseEvent &event);
	void OnLeftUp(wxMouseEvent &event);
	void OnMouseMove(wxMouseEvent &event);
	void OnQuit(wxCommandEvent& WXUNUSED(event));
	void OnAbout(wxCommandEvent& WXUNUSED(event));
	void OnToolLeftClick(wxCommandEvent& event);
	void OnToolProfLoadSave(wxCommandEvent& event);
	void OnToolStartStop(wxCommandEvent& event);
	void OnLogReduce(wxCommandEvent &event);
	void OnCopy(wxCommandEvent &event);
	void OnPaste(wxCommandEvent &event);
	void OnCut(wxCommandEvent &event);
	void OnDelete(wxCommandEvent &event);
	void OnOpen(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnDMainClickSash(wxSplitterEvent& event);
	void OnDRClickSash(wxSplitterEvent& event);
	void toolFertig();
	void ToolEnableDisable();
	void OnNewSettingsFont(wxCommandEvent &event);
	void OnHighlight(wxCommandEvent &event);
	void OnToolFertig(wxCommandEvent &event);
	void OnVCheckFertig(wxCommandEvent &event);
	void OnDevGrpRefresh(wxCommandEvent &event);
	void OnDevGrpSave(wxCommandEvent &event);
	void OnXmlSwap(wxCommandEvent &event);
	void OnHostSearch(wxCommandEvent &event);
	void OnMacDBUpdate(wxCommandEvent &event);
	void OnWknPropGridChange(wxCommandEvent &event);
	void OnSchedLoadProfile(wxCommandEvent &event);
	void OnNeuerEintrag(wxCommandEvent &event);
	void OnSchedLoadFile(wxCommandEvent &event);
	void CreateGUIControls(void);
	void OnSchedRowSelect(wxCommandEvent &event);
	void OnSchedRowDelete(wxCommandEvent &event);

	//(*Identifiers(wktoolsFrame)
	static const long idMenuQuit;
	static const long idMenuAbout;
	static const long ID_STATUSBAR1;
	//*)

	//(*Declarations(wktoolsFrame)
	wxStatusBar* StatusBar1;

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};

