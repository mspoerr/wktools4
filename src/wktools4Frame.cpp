/***************************************************************
* Name:      wktoolsMain.cpp
* Purpose:   Code for Application Frame
* Author:    Mathias Spoerr (wktools@spoerr.org)
* Created:   2007-11-03
* Copyright: Mathias Spoerr (http://www.spoerr.org/wktools)
* License:   Freeware

**************************************************************/
#include "wktools4Frame.h"
#include "wktools4.h"
#include <wx/aboutdlg.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

// XPM Files für Tool Selector Bar
#include "res/about.xpm"
#include "res/wkn.xpm"
#include "res/wkc.xpm"
#include "res/wke.xpm"
#include "res/wkp.xpm"
#include "res/wkl.xpm"
#include "res/wkm.xpm"
#include "res/doku.xpm"
#include "res/wzrd.xpm"
#include "res/wktools41.xpm"
#include "res/lic.xpm"
#include "res/sched.xpm"

// XPM Files für Toolbar
#include "toolbar/cancel.xpm"
#include "toolbar/copy.xpm"
#include "toolbar/cut.xpm"
#include "toolbar/delete.xpm"
#include "toolbar/doit.xpm"
#include "toolbar/open1.xpm"
#include "toolbar/kill.xpm"
#include "toolbar/paste.xpm"
#include "toolbar/profileload.xpm"
#include "toolbar/profilesave.xpm"
#include "toolbar/profileremove.xpm"
#include "toolbar/search.xpm"
#include "toolbar/save.xpm"
#include "toolbar/settings.xpm"
#include "toolbar/logReduce.xpm"
#include "toolbar/dgfRefresh.xpm"
#include "toolbar/dgfSave.xpm"
#include "toolbar/xmlSwitch.xpm"
#include "toolbar/hostSearch.xpm"
#include "toolbar/macdbUpdate.xpm"



//helper functions
enum wxbuildinfoformat {
short_f, long_f };

	wxString wxbuildinfo(wxbuildinfoformat format)
	{
		wxString wxbuild(wxVERSION_STRING);

		if (format == long_f )
		{
#if defined(__WXMSW__)
			wxbuild << _T("-Windows");
#elif defined(__UNIX__)
			wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
			wxbuild << _T("-Unicode build");
#else
			wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
	}

	return wxbuild;
}

	//(*IdInit(wktoolsFrame)
	const long Cwktools4Frame::idMenuQuit = wxNewId();
	const long Cwktools4Frame::idMenuAbout = wxNewId();
	const long Cwktools4Frame::ID_STATUSBAR1 = wxNewId();
	//*)

enum
{
	wxID_TOOLBAR = wxID_HIGHEST + 1,
	wxID_SELECTBAR,
	wxID_MAINSPLITTER,
	wxID_RSPLITTER
};

DEFINE_LOCAL_EVENT_TYPE(wkEVT_FONTAENDERUNG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_HIGHLIGHT)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WZRD_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKN_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKM_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKE_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKC_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKP_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKL_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKDP_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_VCHECK_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHED_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKN_PROPGRIDCHANGE)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHED_LADEPROFILE)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHED_LADEFILE)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHED_ROWSELECT)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_SCHED_ROWDELETE)

BEGIN_EVENT_TABLE(Cwktools4Frame, wxFrame)
	EVT_CLOSE(Cwktools4Frame::OnClose)
	EVT_PAINT(Cwktools4Frame::OnPaint)
	EVT_ERASE_BACKGROUND(Cwktools4Frame::OnEraseBackground)
	EVT_LEFT_DOWN(Cwktools4Frame::OnLeftDown)
	EVT_LEFT_UP(Cwktools4Frame::OnLeftUp)
	EVT_MOTION(Cwktools4Frame::OnMouseMove)
	EVT_MENU(wxID_CLOSE, Cwktools4Frame::OnQuit)
	EVT_MENU(wxID_ABOUT, Cwktools4Frame::OnAbout)
	EVT_TOOL_RANGE(wk_ID_NEWS, wk_ID_SETTINGS, Cwktools4Frame::OnToolLeftClick)
	EVT_TOOL_RANGE(wk_ID_PROFILE_LOAD, wk_ID_PROFILE_REMOVE, Cwktools4Frame::OnToolProfLoadSave)
	EVT_TOOL_RANGE(wk_ID_START, wk_ID_KILL, Cwktools4Frame::OnToolStartStop)
	EVT_TOOL(wk_ID_LOGRED, Cwktools4Frame::OnLogReduce)
	EVT_TOOL(wxID_OPEN, Cwktools4Frame::OnOpen)
	EVT_TOOL(wxID_SAVE, Cwktools4Frame::OnSave)
	EVT_TOOL(wxID_CUT, Cwktools4Frame::OnCut)
	EVT_TOOL(wxID_COPY, Cwktools4Frame::OnCopy)
	EVT_TOOL(wxID_PASTE, Cwktools4Frame::OnPaste)
	EVT_TOOL(wxID_DELETE, Cwktools4Frame::OnDelete)
	EVT_TOOL(wk_ID_DEVGRP_REFRESH, Cwktools4Frame::OnDevGrpRefresh)
	EVT_TOOL(wk_ID_DEVGRP_SAVE, Cwktools4Frame::OnDevGrpSave)
	EVT_TOOL(wk_ID_XML_SWAP, Cwktools4Frame::OnXmlSwap)
	EVT_TOOL(wk_ID_HOSTSEARCH, Cwktools4Frame::OnHostSearch)
	EVT_TOOL(wk_ID_MACDBUPDATE, Cwktools4Frame::OnMacDBUpdate)
	EVT_SPLITTER_DCLICK(wxID_MAINSPLITTER, Cwktools4Frame::OnDMainClickSash)
	EVT_SPLITTER_DCLICK(wxID_RSPLITTER, Cwktools4Frame::OnDRClickSash)
	EVT_COMMAND(wxID_ANY, wkEVT_FONTAENDERUNG, Cwktools4Frame::OnNewSettingsFont)
	EVT_COMMAND(wxID_ANY, wkEVT_HIGHLIGHT, Cwktools4Frame::OnHighlight)
	EVT_COMMAND(wxID_ANY, wkEVT_WZRD_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKN_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKM_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKE_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKC_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKP_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKL_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKDP_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_SCHED_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_VCHECK_FERTIG, Cwktools4Frame::OnVCheckFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKN_PROPGRIDCHANGE, Cwktools4Frame::OnWknPropGridChange)
	EVT_COMMAND(wxID_ANY, wkEVT_SCHED_LADEPROFILE, Cwktools4Frame::OnSchedLoadProfile)
	EVT_COMMAND(wxID_ANY, wkEVT_SCHED_NEU, Cwktools4Frame::OnNeuerEintrag)
	EVT_COMMAND(wxID_ANY, wkEVT_SCHED_LADEFILE, Cwktools4Frame::OnSchedLoadFile)
	EVT_COMMAND(wxID_ANY, wkEVT_SCHED_ROWSELECT, Cwktools4Frame::OnSchedRowSelect)
	EVT_COMMAND(wxID_ANY, wkEVT_SCHED_ROWDELETE, Cwktools4Frame::OnSchedRowDelete)
END_EVENT_TABLE()


Cwktools4Frame::~Cwktools4Frame(void)
{
	delete toolSelector;
	delete wkHilfe;
	delete logAusgabe;
	delete wzrdSettings;
	delete wkeSettings;
	delete wknSettings;
#ifdef WKTOOLS_MAPPER
	delete wkmSettings;
#endif
	delete wklSettings;
	delete schedSettings;
	delete rSplitter;
	delete mainSplitter;
	delete profilCombo;
	delete htmstrs;
	//delete dieDevGrp;
}


bool Cwktools4Frame::Create(wxWindow *pParent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
	wxFrame::Create(pParent, id, title, pos, size, style, name);
	m_horzToolbar = true;
	m_horzText = false;
	m_rows = 1;

	special = true;
	wkmdbg = false;

	CreateGUIControls();
	int x=0, y=0, width=0, height=0;
	int updater = 0;
	mainSplitterPos = 0;
	rSplitterPos = 0;

	int highlight = 0;

	// Grund Elemente für Settings
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSets;	// das Element, das auf Settings zeig
	TiXmlElement *pElemSize;	// das Element, das auf die Window Sizing Einstellungen zeigt
	TiXmlElement *pElemFonts;	// das Element, das auf die Schriftart Einstellungen zeigt
	TiXmlElement *pElemUpdate;	// das Element, das auf die Update Einstellungen zeigt
	TiXmlElement *pElemMisc;	// das Element, das auf allgemeine Einstellungen
	TiXmlElement *pElemSql;		// das Element, das auf die SQL Einstellungen zeigt
	TiXmlElement *pElemSched;	// Das Element, das auf die Scheduler Einstellungen zeigt
	TiXmlElement *pElemSchedSets;	// Das Element, das auf die Scheduler Einstellungen zeigt
	// Settings im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (pElem)
	{
		pElemSets = pElem->FirstChildElement("Settings");
		pElemSize = pElemSets->FirstChildElement("sizing");
		if (pElemSize)
		{
			pElemSize->QueryIntAttribute("xPos", &x);
			pElemSize->QueryIntAttribute("yPos", &y);
			pElemSize->QueryIntAttribute("width", &width);
			pElemSize->QueryIntAttribute("height", &height);
			pElemSize->QueryIntAttribute("AusgabeSplitter", &rSplitterPos);
			pElemSize->QueryIntAttribute("MainSplitter", &mainSplitterPos);
			
			// Tool Splitter Positionen laden
			int wknSpos = 0;
			int wkmSpos = 0;
			int wkeSpos = 0;
			int dipSpos = 0;
			int wklSpos = 0;
			int wzrdSpos = 0;
			int schedSpos = 0;
			int glSpos = 0;
			pElemSize = pElemSets->FirstChildElement("toolSizing");
			pElemSize->QueryIntAttribute("wkn", &wknSpos);
			pElemSize->QueryIntAttribute("wke", &wkeSpos);
			pElemSize->QueryIntAttribute("wkdp", &dipSpos);
			pElemSize->QueryIntAttribute("wkl", &wklSpos);
			pElemSize->QueryIntAttribute("wkm", &wkmSpos);
			pElemSize->QueryIntAttribute("sched", &schedSpos);
			pElemSize->QueryIntAttribute("wzrd", &wzrdSpos);
			
			pElemSize->QueryIntAttribute("glSets", &glSpos);
			globalSettings->setSplitterPos(glSpos);

			if( width !=0 && height !=0 )
			{
				SetSize(width, height);
				if( x !=0 && y !=0 )
				{
					Move(x, y);
				}
			}
			else
			{
				this->Maximize(true);
			}

			if (!mainSplitterPos)
			{
				mainSplitterPos = 500;
			}
			mainSplitter->SetSashPosition(mainSplitterPos, true);

			if(!rSplitterPos)
			{
				rSplitterPos = 250;
			}
			rSplitter->SetSashPosition(rSplitterPos, true);

			if (!wknSpos)
			{
				wknSpos = mainSplitterPos/2;
			}
			if (!wkeSpos)
			{
				wkeSpos = mainSplitterPos/2;
			}
			if (!wklSpos)
			{
				wklSpos = mainSplitterPos/2;
			}
			if (!dipSpos)
			{
				dipSpos = mainSplitterPos/2;
			}
			if (!schedSpos)
			{
				schedSpos = mainSplitterPos/2;
			}
			if (!wkmSpos)
			{
				wkmSpos = mainSplitterPos/2;
			}
			if (!wzrdSpos)
			{
				wzrdSpos = mainSplitterPos/2;
			}

			wknSettings->setSplitterPos(wknSpos);
#ifdef WKTOOLS_MAPPER
			wkmSettings->setSplitterPos(wkmSpos);
#endif
			wkeSettings->setSplitterPos(wkeSpos);
			wklSettings->setSplitterPos(wklSpos);
			schedSettings->setSplitterPos(schedSpos);
			wzrdSettings->setSplitterPos(wzrdSpos);
		}
		
		// Schriftart Einstellungen laden
		pElemFonts = pElemSets->FirstChildElement("FontSettings");
		if (pElemFonts)
		{
			wxFont fontSets = globalSettings->getFont();
			int fGroesse = fontSets.GetPointSize();
			int fFamily = fontSets.GetFamily();
			int fStyle = fontSets.GetStyle();
			int fWeight = fontSets.GetWeight();
			int fUnderline = fontSets.GetUnderlined();
			wxString fName = fontSets.GetFaceName();

			pElemFonts->QueryIntAttribute("FontFamily", &fFamily);
			pElemFonts->QueryIntAttribute("FontSize", &fGroesse);
			pElemFonts->QueryIntAttribute("FontStyle", &fStyle);
			pElemFonts->QueryIntAttribute("FontWeight", &fWeight);
			pElemFonts->QueryIntAttribute("FontUnderline", &fUnderline);
			fName = pElemFonts->Attribute("FontName");

			fontSets.SetPointSize(fGroesse);
			fontSets.SetFamily(fFamily);
			fontSets.SetFaceName(fName);
			fontSets.SetWeight(fWeight);
			fontSets.SetUnderlined(fUnderline);
			fontSets.SetStyle(fStyle);

			globalSettings->resetFont(fontSets);
			wknSettings->resetFont(fontSets);
#ifdef WKTOOLS_MAPPER
			wkmSettings->resetFont(fontSets);
#endif
			wkeSettings->resetFont(fontSets);
			wklSettings->resetFont(fontSets);
			wzrdSettings->resetFont(fontSets);
			schedSettings->resetFont(fontSets);
		}

		// Updater Einstellungen laden
		pElemUpdate = pElemSets->FirstChildElement("Updater");
		if (pElemUpdate)
		{
			pElemUpdate->QueryIntAttribute("ud", &updater);
			globalSettings->setUpdater(updater);
		}
		// Allgemeine Einstellungen laden
		pElemMisc = pElemSets->FirstChildElement("MiscSettings");
		if (pElemMisc)
		{
			pElemMisc->QueryIntAttribute("Highlight", &highlight);
			globalSettings->setHighlight(highlight);
			wkeSettings->setFarbe(highlight);
			wknSettings->setFarbe(highlight);
#ifdef WKTOOLS_MAPPER
			wkmSettings->setFarbe(highlight);
#endif
			wklSettings->setFarbe(highlight);
			wzrdSettings->setFarbe(highlight);
			schedSettings->setFarbe(highlight);

			std::string xmlLoc = "";
			if (pElemMisc->Attribute("xmlLoc"))
			{
				xmlLoc = pElemMisc->Attribute("xmlLoc");
				if (xmlLoc != "")
				{
					globalSettings->setXmlLoc(xmlLoc);
				}
			}

			std::string extras = "";

			std::string repFile = "";
			int repStat = 0;
			pElemMisc->QueryIntAttribute("Report", &repStat);

			if (pElemMisc->Attribute("ReportFile"))
			{
				repFile = pElemMisc->Attribute("ReportFile");
			}
			globalSettings->setReport(repFile, repStat);


			
			if (pElemMisc->Attribute("extras"))
			{
				extras = pElemMisc->Attribute("extras");
				if (extras == "wkmdbg")
				{
					globalSettings->setExtras(extras);
					wkmdbg = true;
					mapper->wkmdbg = true;
				}

			}
			//if (pElemMisc->Attribute("extras"))
			//{
//				extras = pElemMisc->Attribute("extras");
//				if (extras == "UseMapper")
//				{
//#ifdef WKTOOLS_MAPPER
//					special = true;
//#else
//					special = false;
//#endif
//					globalSettings->setExtras(extras);
//					if (special)
//					{
//						toolSelector->InsertTool(6, wk_ID_WKM, wxT("Mapper"), wxBitmap(wkm_xpm), wxNullBitmap, wxITEM_CHECK);
//						toolSelector->Realize();
//					}
//
//				}
//			}
		}
		
		// SQL Server Einstellungen laden
		pElemSql = pElemSets->FirstChildElement("SQLSettings");
		if (pElemSql)
		{
			CSettings::sqlSets ssts;

			int useSql = 0;
			pElemSql->QueryIntAttribute("UseSql", &useSql);
			ssts.useSql = useSql;
			
			if (pElemSql->Attribute("sqlSrv"))
			{
				ssts.sqlSrv = pElemSql->Attribute("sqlSrv");
			}
			if (pElemSql->Attribute("sqlUsr"))
			{
				ssts.sqlUSr = pElemSql->Attribute("sqlUsr");
			}
			if (pElemSql->Attribute("sqlPwd"))
			{
				ssts.sqlPwd = pElemSql->Attribute("sqlPwd");
			}

			globalSettings->setSqlSets(ssts);
		}

		// Scheduler Einstellungen laden
		pElemSched = pElem->FirstChildElement("Scheduler");
		pElemSchedSets = pElemSched->FirstChildElement("StringSettings");
		if (pElemSchedSets)
		{
			std::string schedFile = "";
			if (pElemSchedSets->Attribute("SchedFile"))
			{
				schedFile = pElemSchedSets->Attribute("SchedFile");
				sched->ladeSchedFile(schedFile);
			}
		}
	}

	return true;
}


void Cwktools4Frame::CreateGUIControls(void)
{
	mainFrame = this;
	toolAlt = -1;
	toolStartId = -1;
	toolNeu = 0;

	htmstrs = new HtmStrings;

#ifdef _WINDOWS_
	SetIcon(wxICON(APPICON));
#else
	SetIcon(wxICON(wktools41));
#endif			

	//wxMenu *menuFile = new wxMenu;
	//menuFile->Append( wxID_CLOSE, wxT("E&xit") );

	//wxMenu *menuHelp = new wxMenu;
	//menuHelp->Append( wxID_ABOUT, wxT("&About...") );

	//wxMenuBar *menuBar = new wxMenuBar;
	//menuBar->Append( menuFile, wxT("&File") );
	//menuBar->Append( menuHelp, wxT("&Help") );

	//SetMenuBar( menuBar );

    // delete and recreate the toolbar
    m_pToolBar = GetToolBar();
    long style = m_pToolBar ? m_pToolBar->GetWindowStyle() : wxTB_FLAT | wxTB_DOCKABLE;

    delete m_pToolBar;

    SetToolBar(NULL);

    style &= ~(wxTB_HORIZONTAL | wxTB_VERTICAL | wxTB_HORZ_LAYOUT);
    style |= m_horzToolbar ? wxTB_HORIZONTAL : wxTB_VERTICAL;

    if ( style & wxTB_TEXT && !(style & wxTB_NOICONS) && m_horzText )
        style |= wxTB_HORZ_LAYOUT;

    m_pToolBar = CreateToolBar(style, wxID_TOOLBAR);

	m_pToolBar->SetToolBitmapSize(wxSize(30,30));

	m_pToolBar->AddTool(wxID_OPEN, wxT("Open"), wxBitmap(open1_xpm), wxT("Open"));
	m_pToolBar->AddTool(wxID_SAVE, wxT("Save"), wxBitmap(save_xpm), wxT("Save"));
	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wxID_COPY, wxT("Copy"), wxBitmap(copy_xpm), wxT("Copy"));
	m_pToolBar->AddTool(wxID_PASTE, wxT("Paste"), wxBitmap(paste_xpm), wxT("Paste"));
	m_pToolBar->AddTool(wxID_CUT, wxT("Cut"), wxBitmap(cut_xpm), wxT("Cut"));
	m_pToolBar->AddTool(wxID_DELETE, wxT("Delete"), wxBitmap(delete_xpm), wxT("Delete"));
	m_pToolBar->AddSeparator();

	profilCombo = new wxComboBox(m_pToolBar, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150,20) );
	m_pToolBar->AddControl(profilCombo);

	m_pToolBar->AddTool(wk_ID_PROFILE_LOAD, wxT("Load Profile"), wxBitmap(profileload_xpm), wxT("Load Profile"));
	m_pToolBar->AddTool(wk_ID_PROFILE_SAVE, wxT("Save Profile"), wxBitmap(profilesave_xpm), wxT("Save Profile"));
	m_pToolBar->AddTool(wk_ID_PROFILE_REMOVE, wxT("Remove Profile"), wxBitmap(profileremove_xpm), wxT("Remove Profile"));

	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_DEVGRP_REFRESH, wxT("DeviceGroup Window Refresh"), wxBitmap(dgfRefresh_xpm), wxT("DeviceGroup Window Refresh"));
	m_pToolBar->AddTool(wk_ID_DEVGRP_SAVE, wxT("DeviceGroup Window Save"), wxBitmap(dgfSave_xpm), wxT("DeviceGroup Window Save"));
	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_START, wxT("Start/Stop"), wxBitmap(doit_xpm), wxT("Start/Stop"), wxITEM_CHECK);
	m_pToolBar->AddSeparator();
	//m_pToolBar->AddTool(wxID_ANY, wxT("Cancel"), wxBitmap(cancel_xpm), wxT("Cancel"));
	//m_pToolBar->AddTool(wk_ID_KILL, wxT("Kill"), wxBitmap(kill_xpm), wxT("Kill"));
	//m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_SETTINGS, wxT("Settings"), wxBitmap(settings_xpm), wxT("Settings"), wxITEM_CHECK);
	m_pToolBar->AddTool(wk_ID_XML_SWAP, wxT("Swap wktools.xml"), wxBitmap(xmlSwitch_xpm), wxT("Swap wktools.xml"));
#ifdef WKTOOLS_MAPPER
	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_HOSTSEARCH, wxT("Mapper Host Search"), wxBitmap(hostSearch_xpm), wxT("Mapper Host Search"));
	m_pToolBar->AddTool(wk_ID_MACDBUPDATE, wxT("MAC OUI DB Update"), wxBitmap(macdbUpdate_xpm), wxT("MAC OUI DB Update"));
#endif
	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_LOGRED, wxT("Hide Debug Output"), wxBitmap(logReduce_xpm), wxT("Hide Debug Output"), wxITEM_CHECK);

    // after adding the buttons to the toolbar, must call Realize() to reflect the changes
    m_pToolBar->Realize();
	m_pToolBar->EnableTool(wk_ID_KILL, false);

    m_pToolBar->SetRows(m_horzToolbar ? (int)m_rows : 10 / (int)m_rows);
    
	// Tool Selector BAR hinzufügen
	toolSelector = new wxToolBar(this, wxID_SELECTBAR, wxDefaultPosition,
		wxDefaultSize, wxNO_BORDER|wxTB_FLAT|wxTB_VERTICAL);
	//toolSelector->AddSeparator();

	toolSelector->SetToolBitmapSize(wxSize(32,32));
	toolSelector->AddTool(wk_ID_NEWS, wxT("wktools"), wxBitmap(wktools41_xpm), wxT("wktools"), wxITEM_CHECK);
	toolSelector->AddSeparator();
	toolSelector->AddTool(wk_ID_WZRD, wxT("Wizard"), wxBitmap(wzrd_xpm), wxT("Wizard"), wxITEM_CHECK);
	toolSelector->AddSeparator();
	toolSelector->AddTool(wk_ID_WKN, wxT("Configmaker"), wxBitmap(wkn_xpm), wxT("ConfigMaker"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_WKE, wxT("Configure Devices"), wxBitmap(wke_xpm), wxT("Configure Devices"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_WKL, wxT("IP List"), wxBitmap(wkl_xpm), wxT("IP List"), wxITEM_CHECK);
#ifdef WKTOOLS_MAPPER
	toolSelector->AddTool(wk_ID_WKM, wxT("Mapper"), wxBitmap(wkm_xpm), wxT("Mapper"), wxITEM_CHECK);
#endif
	toolSelector->AddSeparator();
	toolSelector->AddTool(wk_ID_SCHED, wxT("Scheduler"), wxBitmap(sched_xpm), wxT("Scheduler"), wxITEM_CHECK);
	toolSelector->AddSeparator();
	toolSelector->AddTool(wk_ID_ABOUT, wxT("About"), wxBitmap(doku_xpm), wxT("About"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_LIC, wxT("License"), wxBitmap(lic_xpm), wxT("License"), wxITEM_CHECK);
	toolSelector->Realize();

	wxFlexGridSizer *fgs = new wxFlexGridSizer(1, 2, 0, 0);
	fgs->AddGrowableCol(1);
	fgs->AddGrowableRow(0);
	fgs->Add(toolSelector, 0, wxEXPAND);

	// Splitter für die Tools einfügen
	mainSplitter = new wxSplitterWindow(mainFrame, wxID_MAINSPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3D);
	rPanel = new wxPanel(mainSplitter, wxID_ANY);
	rSplitter = new wxSplitterWindow(rPanel, wxID_RSPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3D);
	logAusgabe = new WkLog(rSplitter, wxID_ANY);
	dieDevGrp = new DevGrpPanel(rSplitter, wxID_ANY);
	dieDevGrp->xmlInit(doc);
	logAusgabe->wkFehler = wkFehler;
	
	wknTabelle = new TabellePanel(rSplitter, wxID_ANY);
	dipTabelle = new TabellePanel(rSplitter, wxID_ANY);
	dirMapTabelle = new TabellePanel(rSplitter, wxID_ANY);

	schedPanel = new SchedPanel(rSplitter, wxID_ANY);
	schedPanel->xmlInit(doc);

	wxFlexGridSizer *fgsr = new wxFlexGridSizer(1, 1, 0, 0);
	fgsr->AddGrowableCol(0);
	fgsr->AddGrowableRow(0);

	wzrdSettings = new CWzrdPropGrid(mainSplitter, wxID_ANY);
	wknSettings = new CWknPropGrid(mainSplitter, wxID_ANY);
#ifdef WKTOOLS_MAPPER
	wkmSettings = new CWkmPropGrid(mainSplitter, wxID_ANY);
#endif
	wkeSettings = new CWkePropGrid(mainSplitter, wxID_ANY);
	wkeSettings->sets(dieDevGrp);
	wklSettings = new CWklPropGrid(mainSplitter, wxID_ANY);
	schedSettings = new CSchedPropGrid(mainSplitter, wxID_ANY);
	wkHilfe = new CWkHilfe(mainSplitter, wxID_ANY);
	globalSettings = new CSettings(mainSplitter, wxID_ANY);
	wzrdSettings->Hide();
	wkeSettings->Hide();
#ifdef WKTOOLS_MAPPER
	wkmSettings->Hide();
#endif
	wkHilfe->Hide();
	globalSettings->Hide();
	wklSettings->Hide();
	schedSettings->Hide();

	mainSplitter->SplitVertically(wknSettings, rPanel, 200);
	mainSplitter->SetMinimumPaneSize(1);

	dieDevGrp->Hide();
	wknTabelle->Hide();
	dipTabelle->Hide();
	schedPanel->Hide();

	dirMapTabelle->Hide();
	dirMapTabelleInit();

	rSplitter->Initialize(logAusgabe);
	rSplitter->SetMinimumPaneSize(1);

	fgsr->Add(rSplitter, 0, wxEXPAND);

	rPanel->SetSizer(fgsr);
	mainSplitter->SetMinimumPaneSize(1);

	fgs->Add(mainSplitter, 0, wxALL|wxEXPAND, 0); 

	// Per Defsault ausgewähltes Tool initialisieren
	wxCommandEvent comevent(0, wk_ID_NEWS);
	OnToolLeftClick(comevent);
	toolSelector->ToggleTool(wk_ID_NEWS, true);

	// Global Settings initialisieren
	globalSettings->setMainFrame(mainFrame);
	globalSettings->wkErrGuiErstellen();
	erstellen->setMainFrame(mainFrame);
	einspielen->setMainFrame(mainFrame);
	iplist->setMainFrame(mainFrame);
#ifdef WKTOOLS_MAPPER
	mapper->setMainFrame(mainFrame);
#endif
	wizard->setMainFrame(mainFrame);

	//create the statusbar
	//CreateStatusBar();
	//SetStatusText(wxT("Ready"));

	// Sizer 
	SetSizer(fgs);
	SetAutoLayout(true);
	Layout();

	// Scheduler
	sched = new Scheduler();
	sched->setMainFrame(mainFrame);
	sched->guiSets(schedSettings, logAusgabe, schedPanel);
	sched->sets(doc, false);
}


void Cwktools4Frame::OnClose(wxCloseEvent& event)
{
	// wktools xml schreiben
	bool closeWktools = true;
	bool saveWktools = true;
	int res = wxGetApp().checkWktoolsXmlChange();
	if (res)
	{
		int ret = wxMessageBox("wktools.xml was edited elsewhere. Do you want to overwrite it anyway?\nClick Cancel to keep wktools open and No to close without writing.", "wktools.xml Warning", wxYES_NO | wxCANCEL);
		if (ret == wxYES)
		{
			closeWktools = true;
			saveWktools = true;
		}
		if (ret == wxNO)
		{
			closeWktools = true;
			saveWktools = false;
		}
		else if (ret == wxCANCEL)
		{
			closeWktools = false;
			saveWktools = false;
		}
	}
	else
	{
		closeWktools = true;
		saveWktools = true;
	}

	if (closeWktools)
	{
		wxPoint pos = GetPosition();
		wxSize size = GetSize();

		wxFont fontSets = globalSettings->getFont();
		int fGroesse = fontSets.GetPointSize();
		int fFamily = fontSets.GetFamily();
		int fStyle = fontSets.GetStyle();
		int fWeight = fontSets.GetWeight();
		bool fUnderline = fontSets.GetUnderlined();
		wxString fName = fontSets.GetFaceName();
		int highlight = globalSettings->getHighlight();
		CSettings::sqlSets ssts = globalSettings->getSqlSets();
		std::string xmlLoc = globalSettings->getXmlLoc();
		std::string extraString = globalSettings->getExtras();
		std::string repFile = globalSettings->getReportFile();
		int repStat = globalSettings->getRepStat();

		switch (toolNeu)
		{
		case wk_ID_WKE:
		case wk_ID_WKN:
		case wk_ID_WKM:
		case wk_ID_WKL:
		case wk_ID_WZRD:
		case wk_ID_SCHED:
			rSplitterPos = rSplitter->GetSashPosition();
			break;
		default:
			break;
		}

		int updater = globalSettings->getUpdater();
		
		// Grund Elemente für Settings
		TiXmlHandle hDoc(doc);
		TiXmlElement *pElem;		// ein Element
		TiXmlElement *pElemSets;	// das Element, das auf Settings zeigt
		TiXmlElement *pElemSize;	// das Element, das auf die Window Sizing Einstellungen zeigt
		TiXmlElement *pElemFonts;	// das Element, das auf die Schriftart Einstellungen zeigt
		TiXmlElement *pElemUpdate;	// das Element, das auf die Update Einstellungen zeigt
		TiXmlElement *pElemMisc;	// das Element, das auf die Allgemeinen Einstellungen zeigt
		TiXmlElement *pElemSched;	// Das Element, das auf die Scheduler Einstellungen zeigt
		TiXmlElement *pElemSchedSets;	// Das Element, das auf die Scheduler Einstellungen zeigt
		TiXmlElement *pElemSqlSets;	// Das Element, das auf die SQL Server Einstellungen zeigt.

		// Settings im wktools.xml finden
		pElem = hDoc.FirstChildElement().Element();
		// should always have a valid root but handle gracefully if it does
		if (pElem)
		{
			// Allgemeine Einstellungen speichern
			pElemSets = pElem->FirstChildElement("Settings");
			pElemSize = pElemSets->FirstChildElement("sizing");
			if (!pElemSize)
			{
				pElemSize = new TiXmlElement("sizing");
				pElemSets->LinkEndChild(pElemSize);
			}
			pElemSize->SetAttribute("xPos", pos.x);
			pElemSize->SetAttribute("yPos", pos.y);
			pElemSize->SetAttribute("width", size.GetWidth());
			pElemSize->SetAttribute("height", size.GetHeight());
			pElemSize->SetAttribute("AusgabeSplitter", rSplitterPos);
			pElemSize->SetAttribute("MainSplitter", mainSplitterPos);

			// Updater Einstellungen speichern
			pElemUpdate = pElemSets->FirstChildElement("Updater");
			if (!pElemUpdate)
			{
				pElemUpdate = new TiXmlElement("Updater");
				pElemSets->LinkEndChild(pElemUpdate);
			}
			pElemUpdate->SetAttribute("ud", updater);
			pElemUpdate->SetAttribute("um", updateMessage);

			// Misc Einstellungen
			pElemMisc = pElemSets->FirstChildElement("MiscSettings");
			if (!pElemMisc)
			{
				pElemMisc = new TiXmlElement("MiscSettings");
				pElemSets->LinkEndChild(pElemMisc);
			}
			pElemMisc->SetAttribute("Highlight", highlight);
			pElemMisc->SetAttribute("xmlLoc", xmlLoc);
			pElemMisc->SetAttribute("extras", extraString);
			pElemMisc->SetAttribute("Report", repStat);
			pElemMisc->SetAttribute("ReportFile", repFile);

			// SQL Server Einstellungen
			pElemSqlSets = pElemSets->FirstChildElement("SQLSettings");
			if (!pElemSqlSets)
			{
				pElemSqlSets = new TiXmlElement("SQLSettings");
				pElemSets->LinkEndChild(pElemSqlSets);
			}
			pElemSqlSets->SetAttribute("UseSql", ssts.useSql);
			pElemSqlSets->SetAttribute("sqlSrv", ssts.sqlSrv);
			pElemSqlSets->SetAttribute("sqlUsr", ssts.sqlUSr);
			pElemSqlSets->SetAttribute("sqlPwd", ssts.sqlPwd);

			// Tool Splitter Positionen speichern
			pElemSize = pElemSets->FirstChildElement("toolSizing");
			if (!pElemSize)
			{
				pElemSize = new TiXmlElement("toolSizing");
				pElemSets->LinkEndChild(pElemSize);
			}
			pElemSize->SetAttribute("wkn", wknSettings->getSplitterPos());
#ifdef WKTOOLS_MAPPER
			pElemSize->SetAttribute("wkm", wkmSettings->getSplitterPos());
#endif
			pElemSize->SetAttribute("wke", wkeSettings->getSplitterPos());
			pElemSize->SetAttribute("wkl", wklSettings->getSplitterPos());
			pElemSize->SetAttribute("wzrd", wzrdSettings->getSplitterPos());
			pElemSize->SetAttribute("sched", schedSettings->getSplitterPos());
			pElemSize->SetAttribute("glSets", globalSettings->getSplitterPos());

			// Schrift Einstellungen speichern
			pElemFonts = pElemSets->FirstChildElement("FontSettings");
			if (!pElemFonts)
			{
				pElemFonts = new TiXmlElement("FontSettings");
				pElemSets->LinkEndChild(pElemFonts);
			}
			pElemFonts->SetAttribute("FontFamily", fFamily);
			pElemFonts->SetAttribute("FontName", fName);
			pElemFonts->SetAttribute("FontSize", fGroesse);
			pElemFonts->SetAttribute("FontStyle", fStyle);
			pElemFonts->SetAttribute("FontWeight", fWeight);
			pElemFonts->SetAttribute("FontUnderline", fUnderline);

			// Scheduler Einstellungen speichern
			pElemSched = pElem->FirstChildElement("Scheduler");
			if (!pElemSched)
			{
				pElemSched = new TiXmlElement("Scheduler");
				pElem->LinkEndChild(pElemSched);
			}
			
			pElemSchedSets = pElemSched->FirstChildElement("StringSettings");
			if (!pElemSchedSets)
			{
				pElemSchedSets = new TiXmlElement("StringSettings");
				pElemSched->LinkEndChild(pElemSchedSets);
			}
			pElemSchedSets->SetAttribute("SchedFile", schedSettings->leseStringSetting(CSchedPropGrid::SCHED_PROPID_STR_SCHEDFILE));
			sched->sichereSchedTabelle();
		}

		dirMapTabelleSichern();

		// Report File schreiben
		wxGetApp().schreibeReport();
		
		if (saveWktools)
		{
			wxGetApp().wkErrStatusSchreiben();
			wxGetApp().schreibeWktoolsXml();
		}

		
		event.Skip();
	}
}

void Cwktools4Frame::OnPaint(wxPaintEvent & event)
{
	///TODO: Remove the below line, for custom drawing.
	event.Skip();
}

void Cwktools4Frame::OnEraseBackground(wxEraseEvent& event)
{
	///TODO: Remove the below line, for custom drawing.
	event.Skip();
}

void Cwktools4Frame::OnLeftDown(wxMouseEvent& event)
{
	event.Skip();
}

void Cwktools4Frame::OnLeftUp(wxMouseEvent& event)
{
	event.Skip();
}

void Cwktools4Frame::OnMouseMove(wxMouseEvent& event)
{
	event.Skip();
}

void Cwktools4Frame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	// wktools xml schreiben
	int res = wxGetApp().checkWktoolsXmlChange();
	if (res)
	{
		int ret = wxMessageBox("wktools.xml was edited elsewhere. Do you want to overwrite it anyway?\nClick Cancel to keep wktools open and No to close without writing.", "wktools.xml Warning", wxYES_NO | wxCANCEL);
		if (ret == wxYES)
		{
			Close();
		}
		if (ret == wxNO)
		{

		}
		else if (ret == wxCANCEL)
		{
			
		}
	}
	else
	{
		Close();
	}

}

void Cwktools4Frame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo info;
    info.SetName(wxT("wktools4"));
    info.SetVersion(wxT("Alpha3"));
    info.SetDescription(wxT("www.spoerr.org/wktools"));
    info.SetCopyright(wxT("(c) 2009 Mathias Spoerr <wktools@spoerr.org>"));

    wxAboutBox(info);
	m_pToolBar->ToggleTool( wxID_ABOUT, false );
}


void Cwktools4Frame::OnToolLeftClick(wxCommandEvent& event)
{
	toolNeu = event.GetId();
	ToolEnableDisable();

	if (toolNeu != toolAlt)
	{
		wxArrayString profile;
		profile.Empty();
		profilCombo->Clear();

		toolSelector->ToggleTool(toolAlt, false);
		if (toolAlt == wk_ID_SETTINGS)
		{
			m_pToolBar->ToggleTool(wk_ID_SETTINGS, false);
			globalSettings->wkErrGuiSave();
		}
		
		m_replacewindow = mainSplitter->GetWindow1();
		m_replacewindowR = rSplitter->GetWindow1();
		
		
		if (event.GetId() == wk_ID_NEWS)
		{
			toolAlt = wk_ID_NEWS;
			mainSplitter->ReplaceWindow(m_replacewindow, wkHilfe);
			m_replacewindow->Hide();
			
			wkHilfe->ladeSeite(htmstrs->wktools.c_str());
			wkHilfe->Show();
			if (rSplitter->IsSplit())
			{
				rSplitterPos = rSplitter->GetSashPosition();
				rSplitter->Unsplit();
			}
			if (mainSplitter->IsSplit())
			{
				mainSplitterPos = mainSplitter->GetSashPosition();
				mainSplitter->Unsplit();
			}
		}
		if (event.GetId() == wk_ID_WZRD)
		{
			toolAlt = wk_ID_WZRD;
			mainSplitter->ReplaceWindow(m_replacewindow, wzrdSettings);
			m_replacewindow->Hide();
			wzrdSettings->Show();

			rSplitter->ReplaceWindow(m_replacewindowR, dieDevGrp);
			m_replacewindowR->Hide();
			dieDevGrp->Show();
			
			profile = wizard->ladeProfile();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(wzrdSettings, rPanel, mainSplitterPos);
			}
			if (!rSplitter->IsSplit())
			{
				if (rSplitterPos < 10)
				{
					rSplitterPos = 150;
				}
				rSplitter->SplitHorizontally(dieDevGrp, logAusgabe, rSplitterPos);
			}
		}
		if (event.GetId() == wk_ID_WKN)
		{
			toolAlt = wk_ID_WKN;
			mainSplitter->ReplaceWindow(m_replacewindow, wknSettings);
			m_replacewindow->Hide();
			rSplitter->ReplaceWindow(m_replacewindowR, wknTabelle);
			m_replacewindowR->Hide();
			wknSettings->Show();
			wknTabelle->Show();
			
			profile = erstellen->ladeProfile();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(wknSettings, rPanel, mainSplitterPos);
			}
			if (!rSplitter->IsSplit())
			{
				if (rSplitterPos < 10)
				{
					rSplitterPos = 150;
				}
				rSplitter->SplitHorizontally(wknTabelle, logAusgabe, rSplitterPos);
			}
		}

		if (event.GetId() == wk_ID_WKE)
		{
			toolAlt = wk_ID_WKE;
			mainSplitter->ReplaceWindow(m_replacewindow, wkeSettings);
			m_replacewindow->Hide();
			wkeSettings->Show();
			rSplitter->ReplaceWindow(m_replacewindowR, dieDevGrp);
			m_replacewindowR->Hide();
			dieDevGrp->Show();

			profile = einspielen->ladeProfile();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(wkeSettings, rPanel, mainSplitterPos);
			}
			if (!rSplitter->IsSplit())
			{
				if (rSplitterPos < 10)
				{
					rSplitterPos = 150;
				}
				rSplitter->SplitHorizontally(dieDevGrp, logAusgabe, rSplitterPos);
			}

		}

		if (event.GetId() == wk_ID_WKC)
		{
			toolAlt = wk_ID_WKC;
		}

		if (event.GetId() == wk_ID_WKL)
		{
			toolAlt = wk_ID_WKL;
			mainSplitter->ReplaceWindow(m_replacewindow, wklSettings);
			m_replacewindow->Hide();
			wklSettings->Show();

			profile = iplist->ladeProfile();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(wklSettings, rPanel, mainSplitterPos);
			}
			if (rSplitter->IsSplit())
			{
				rSplitterPos = rSplitter->GetSashPosition();
				rSplitter->Unsplit(rSplitter->GetWindow1());
			}
		}

#ifdef WKTOOLS_MAPPER
		if (event.GetId() == wk_ID_WKM)
		{
			toolAlt = wk_ID_WKM;
			mainSplitter->ReplaceWindow(m_replacewindow, wkmSettings);
			m_replacewindow->Hide();
			wkmSettings->Show();

			profile = mapper->ladeProfile();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(wkmSettings, rPanel, mainSplitterPos);
			}
			if (rSplitter->IsSplit())
			{
				rSplitterPos = rSplitter->GetSashPosition();
				rSplitter->Unsplit(rSplitter->GetWindow1());
			}
		}
#endif

		if (event.GetId() == wk_ID_SCHED)
		{
			toolAlt = wk_ID_SCHED;
			mainSplitter->ReplaceWindow(m_replacewindow, schedSettings);
			m_replacewindow->Hide();
			schedSettings->Show();
			rSplitter->ReplaceWindow(m_replacewindowR, schedPanel);
			m_replacewindowR->Hide();
			schedPanel->Show();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(schedSettings, rPanel, mainSplitterPos);
			}
			if (!rSplitter->IsSplit())
			{
				rSplitter->SplitHorizontally(schedPanel, logAusgabe, rSplitterPos);
			}
		}

		if (event.GetId() == wk_ID_LIC)
		{
			toolAlt = wk_ID_LIC;
			mainSplitter->ReplaceWindow(m_replacewindow, wkHilfe);
			m_replacewindow->Hide();

			wkHilfe->ladeSeite(lic);
			wkHilfe->Show();
			if (rSplitter->IsSplit())
			{
				rSplitterPos = rSplitter->GetSashPosition();
				rSplitter->Unsplit();
			}
			if (mainSplitter->IsSplit())
			{
				mainSplitterPos = mainSplitter->GetSashPosition();
				mainSplitter->Unsplit();
			}
		}

		if (event.GetId() == wk_ID_ABOUT)
		{
			toolAlt = wk_ID_ABOUT;
			mainSplitter->ReplaceWindow(m_replacewindow, wkHilfe);
			m_replacewindow->Hide();
			
			wkHilfe->ladeSeite(htmstrs->about.c_str());
			wkHilfe->Show();
			if (rSplitter->IsSplit())
			{
				rSplitterPos = rSplitter->GetSashPosition();
				rSplitter->Unsplit();
			}
			if (mainSplitter->IsSplit())
			{
				mainSplitterPos = mainSplitter->GetSashPosition();
				mainSplitter->Unsplit();
			}
		}
		if (event.GetId() == wk_ID_SETTINGS)
		{
			toolAlt = wk_ID_SETTINGS;
			mainSplitter->ReplaceWindow(m_replacewindow, globalSettings);
			m_replacewindow->Hide();
			globalSettings->Show();
			rSplitter->ReplaceWindow(m_replacewindowR, dirMapTabelle);
			m_replacewindowR->Hide();
			dirMapTabelle->Show();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(globalSettings, rPanel, mainSplitterPos);
			}
			if (rSplitter->IsSplit())
			{
				rSplitter->Unsplit();
//				rSplitter->SplitHorizontally(dirMapTabelle, logAusgabe, rSplitterPos);
			}
		}
		for (int i = 0; i < profile.GetCount(); i++)
		{
			profilCombo->Append(profile[i]);
		}
	}
	else
	{
		toolSelector->ToggleTool(toolAlt, true);
		if (toolAlt == wk_ID_SETTINGS)
		{
			m_pToolBar->ToggleTool(wk_ID_SETTINGS, true);
		}
	}

}


void Cwktools4Frame::OnDMainClickSash(wxSplitterEvent& event)
{
	if (mainSplitter->GetSashPosition() != 1)
	{
		mainSplitterPos = mainSplitter->GetSashPosition();	
		mainSplitter->SetSashPosition(1);
	}
	else
	{
		mainSplitter->SetSashPosition(mainSplitterPos);
	}
}


void Cwktools4Frame::OnDRClickSash(wxSplitterEvent& event)
{
	if (rSplitter->GetSashPosition() != 1)
	{
		rSplitterPos = rSplitter->GetSashPosition();	
		rSplitter->SetSashPosition(1);
	}
	else
	{
		rSplitter->SetSashPosition(rSplitterPos);
	}
}

// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Cwktools4Frame::sets(TiXmlDocument *document, bool guiStats, WKErr *wkF)
{
	doc = document;
	guiStatus = guiStats;
	wkFehler = wkF;
}


void Cwktools4Frame::OnToolProfLoadSave(wxCommandEvent& event)
{
	int tid = event.GetId();
	std::string profilName = profilCombo->GetValue().c_str();

	if (tid == wk_ID_PROFILE_LOAD)
	{
		switch (toolNeu)
		{
		case wk_ID_NEWS:
			break;
		case wk_ID_WZRD:
			wizard->doLoad(profilName);
			break;
		case wk_ID_WKN:
			erstellen->doLoad(profilName);
			break;
		case wk_ID_WKE:
			einspielen->doLoad(profilName);
			break;
		case wk_ID_WKC:
			break;
		case wk_ID_WKL:
			iplist->doLoad(profilName);
			break;
		case wk_ID_WKM:
#ifdef WKTOOLS_MAPPER
			mapper->doLoad(profilName);
#endif
			break;
		case wk_ID_WKP:
			break;
		case wk_ID_SETTINGS:
			break;
		default:
			break;
		}
	}
	else if (tid == wk_ID_PROFILE_SAVE)
	{
		wxArrayString profile;
		profilCombo->Clear();

		switch (toolNeu)
		{
		case wk_ID_NEWS:
			break;
		case wk_ID_WZRD:
			wizard->doSave(profilName);
			profile = wizard->ladeProfile();
			break;
		case wk_ID_WKN:
			erstellen->doSave(profilName);
			profile = erstellen->ladeProfile();
			break;
		case wk_ID_WKE:
			einspielen->doSave(profilName);
			profile = einspielen->ladeProfile();
			break;
		case wk_ID_WKC:
			break;
		case wk_ID_WKL:
			iplist->doSave(profilName);
			profile = iplist->ladeProfile();
			break;
		case wk_ID_WKM:
#ifdef WKTOOLS_MAPPER
			mapper->doSave(profilName);
			profile = mapper->ladeProfile();
#endif
			break;
		case wk_ID_WKP:
			break;
		case wk_ID_SETTINGS:
			break;
		default:
			break;
		}
		for (int i = 0; i < profile.GetCount(); i++)
		{
			profilCombo->Append(profile[i]);
		}
	}
	else if (tid == wk_ID_PROFILE_REMOVE)
	{
		wxArrayString profile;
		profilCombo->Clear();

		int answer = wxMessageBox("Delete Profile?", "Confirm",	wxYES_NO, this);
		if (answer == wxNO)
		{
			return;
		}

		switch (toolNeu)
		{
		case wk_ID_NEWS:
			break;
		case wk_ID_WZRD:
			wizard->doRemove(profilName);
			profile = wizard->ladeProfile();
			break;
		case wk_ID_WKN:
			erstellen->doRemove(profilName);
			profile = erstellen->ladeProfile();
			break;
		case wk_ID_WKE:
			einspielen->doRemove(profilName);
			profile = einspielen->ladeProfile();
			break;
		case wk_ID_WKC:
			break;
		case wk_ID_WKL:
			iplist->doRemove(profilName);
			profile = iplist->ladeProfile();
			break;
		case wk_ID_WKM:
#ifdef WKTOOLS_MAPPER
			mapper->doRemove(profilName);
			profile = mapper->ladeProfile();
#endif
			break;
		case wk_ID_WKP:
			break;
		case wk_ID_SETTINGS:
			break;
		default:
			break;
		}
		for (int i = 0; i < profile.GetCount(); i++)
		{
			profilCombo->Append(profile[i]);
		}
	}

	// wktools xml schreiben

	// wktools xml schreiben
	int res = wxGetApp().checkWktoolsXmlChange();
	if (res)
	{
		int ret = wxMessageBox("wktools.xml was edited elsewhere. Do you want to overwrite it anyway?", "wktools.xml Warning", wxYES_NO);
		if (ret == wxYES)
		{
			wxGetApp().schreibeWktoolsXml();
		}
		if (ret == wxNO)
		{

		}
	}
	else
	{
		wxGetApp().schreibeWktoolsXml();
	}
}


void Cwktools4Frame::OnToolStartStop(wxCommandEvent& event)
{
	int tid = event.GetId();
	//m_pToolBar->EnableTool(wk_ID_KILL, true);
	
	if (tid == wk_ID_START)
	{
		// Wenn Start schon gedrückt, dann muss der Vorgang abgebrochen werden
		if (!m_pToolBar->GetToolState(wk_ID_START))
		{
			m_pToolBar->SetToolNormalBitmap(wk_ID_START, wxBitmap(doit_xpm));

			switch (toolStartId)
			{
			case wk_ID_NEWS:
				break;
			case wk_ID_WZRD:
				wizard->cancelIt();
				break;
			case wk_ID_WKN:
				erstellen->cancelIt();
				break;
			case wk_ID_WKE:
				einspielen->cancelIt();
				break;
			case wk_ID_WKC:
				break;
			case wk_ID_WKL:
				iplist->cancelIt();
				break;
			case wk_ID_WKM:
#ifdef WKTOOLS_MAPPER
				mapper->cancelIt();
#endif
				break;
			case wk_ID_WKP:
				break;
			case wk_ID_SETTINGS:
				break;
			case wk_ID_SCHED:
				sched->cancelIt();
				break;
			default:
				break;
			}
		}
		else
		{
			m_pToolBar->SetToolNormalBitmap(wk_ID_START, wxBitmap(kill_xpm));

			switch (toolNeu)
			{
			case wk_ID_NEWS:
				toolStartId = wk_ID_NEWS;
				break;
			case wk_ID_WZRD:
				toolStartId = wk_ID_WZRD;
				wzrdSettings->Update();
				wizard->doIt();
				break;
			case wk_ID_WKN:
				toolStartId = wk_ID_WKN;
				wknSettings->Update();
				erstellen->doIt();
				break;
			case wk_ID_WKE:
				toolStartId = wk_ID_WKE;
				wkeSettings->Update();
				einspielen->doIt();
				break;
			case wk_ID_WKC:
				toolStartId = wk_ID_WKC;
				break;
			case wk_ID_WKL:
				{
					toolStartId = wk_ID_WKL;
					wklSettings->Update();
					int subtool = wklSettings->leseIntSetting(CWklPropGrid::WKL_PROPID_INT_TOOL);
					if (subtool == 1)
					{
						wxMessageDialog *msg = new wxMessageDialog(this,wxT("Please make sure that wktools is started with Admin privileges. Otherwise no RAW Sockets are possible, which are needed to ping each device first. If ping does not work, Scanning will take very long. Proceed?"), wxT("Start Scanner?") ,wxYES_NO | wxICON_EXCLAMATION);
						if (msg->ShowModal() == wxID_YES)
						{
							iplist->doIt();
						}
						else
						{
							wxCommandEvent event(wkEVT_WKL_FERTIG);
							event.SetInt(1);
							wxPostEvent(this, event);
						}
					}
					else
					{
						iplist->doIt();
					}
				}
				break;
			case wk_ID_WKM:
#ifdef WKTOOLS_MAPPER
				toolStartId = wk_ID_WKM;
				wkmSettings->Update();
				mapper->doIt();
#endif
				break;
			case wk_ID_WKP:
				toolStartId = wk_ID_WKP;
				break;
			case wk_ID_SETTINGS:
				toolStartId = wk_ID_SETTINGS;
				break;
			case wk_ID_SCHED:
				toolStartId = wk_ID_SCHED;
				sched->doIt();
				break;
			default:
				break;
			}
		}
	}
	else if (tid == wk_ID_KILL)
	{
		switch (toolNeu)
		{
		case wk_ID_NEWS:
			break;
		case wk_ID_WZRD:
			wizard->kilIt();
			break;
		case wk_ID_WKN:
			erstellen->kilIt();
			break;
		case wk_ID_WKE:
			einspielen->kilIt();
			break;
		case wk_ID_WKC:
			break;
		case wk_ID_WKL:
			iplist->kilIt();
			break;
		case wk_ID_WKP:
			break;
		case wk_ID_SETTINGS:
			break;
		default:
			break;
		}
	}
}


void Cwktools4Frame::toolFertig()
{
	m_pToolBar->EnableTool(wk_ID_KILL, false);
	m_pToolBar->ToggleTool(wk_ID_START, false);
	m_pToolBar->SetToolNormalBitmap(wk_ID_START, wxBitmap(doit_xpm));
}


void Cwktools4Frame::OnToolFertig(wxCommandEvent &event)
{
//	Sleep(1000);
	toolFertig();
#ifdef _WINDOWS_
	::PostMessage((HWND)logAusgabe->debugFenster->GetHWND(), WM_VSCROLL, SB_BOTTOM, 0); 
	::PostMessage((HWND)logAusgabe->logFenster->GetHWND(), WM_VSCROLL, SB_BOTTOM, 0); 
#endif
}


void Cwktools4Frame::ToolEnableDisable()
{
	switch (toolNeu)
	{
	case wk_ID_WKM:
#ifdef WKTOOLS_MAPPER
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_REMOVE, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, false);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_SAVE, false);
		m_pToolBar->EnableTool(wxID_OPEN, false);
		m_pToolBar->EnableTool(wxID_SAVE, true);
		m_pToolBar->EnableTool(wxID_COPY, true);
		m_pToolBar->EnableTool(wxID_CUT, true);
		m_pToolBar->EnableTool(wxID_DELETE, true);
		m_pToolBar->EnableTool(wxID_PASTE, true);
		m_pToolBar->EnableTool(wk_ID_HOSTSEARCH, true);
		m_pToolBar->EnableTool(wk_ID_MACDBUPDATE, true);
#endif
		break;
	case wk_ID_WZRD:
	case wk_ID_WKN:
	case wk_ID_WKC:
	case wk_ID_WKL:
#ifdef WKTOOLS_MAPPER
		m_pToolBar->EnableTool(wk_ID_HOSTSEARCH, false);
		m_pToolBar->EnableTool(wk_ID_MACDBUPDATE, false);
#endif
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_REMOVE, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, false);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_SAVE, false);
		m_pToolBar->EnableTool(wxID_OPEN, false);
		m_pToolBar->EnableTool(wxID_SAVE, true);
		m_pToolBar->EnableTool(wxID_COPY, true);
		m_pToolBar->EnableTool(wxID_CUT, true);
		m_pToolBar->EnableTool(wxID_DELETE, true);
		m_pToolBar->EnableTool(wxID_PASTE, true);
		break;
	case wk_ID_WKE:
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_REMOVE, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_SAVE, true);
		m_pToolBar->EnableTool(wxID_OPEN, false);
		m_pToolBar->EnableTool(wxID_SAVE, true);
		m_pToolBar->EnableTool(wxID_COPY, true);
		m_pToolBar->EnableTool(wxID_CUT, true);
		m_pToolBar->EnableTool(wxID_DELETE, true);
		m_pToolBar->EnableTool(wxID_PASTE, true);
#ifdef WKTOOLS_MAPPER
		m_pToolBar->EnableTool(wk_ID_HOSTSEARCH, false);
		m_pToolBar->EnableTool(wk_ID_MACDBUPDATE, false);
#endif
		break;
	case wk_ID_WKP:
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_SAVE, false);
		m_pToolBar->EnableTool(wxID_OPEN, true);
		m_pToolBar->EnableTool(wxID_SAVE, true);
		m_pToolBar->EnableTool(wxID_COPY, true);
		m_pToolBar->EnableTool(wxID_CUT, true);
		m_pToolBar->EnableTool(wxID_DELETE, true);
		m_pToolBar->EnableTool(wxID_PASTE, true);
#ifdef WKTOOLS_MAPPER
		m_pToolBar->EnableTool(wk_ID_HOSTSEARCH, false);
		m_pToolBar->EnableTool(wk_ID_MACDBUPDATE, false);
#endif
		break;
	case wk_ID_SCHED:
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, true);
		m_pToolBar->EnableTool(wxID_OPEN, true);
		m_pToolBar->EnableTool(wxID_SAVE, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, false);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, false);
		m_pToolBar->EnableTool(wk_ID_PROFILE_REMOVE, false);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, false);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_SAVE, false);
		m_pToolBar->EnableTool(wxID_COPY, false);
		m_pToolBar->EnableTool(wxID_CUT, false);
		m_pToolBar->EnableTool(wxID_DELETE, false);
		m_pToolBar->EnableTool(wxID_PASTE, false);
#ifdef WKTOOLS_MAPPER
		m_pToolBar->EnableTool(wk_ID_HOSTSEARCH, false);
		m_pToolBar->EnableTool(wk_ID_MACDBUPDATE, false);
#endif
		break;
	case wk_ID_SETTINGS:
	default:
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, false);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, false);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, false);
		m_pToolBar->EnableTool(wk_ID_PROFILE_REMOVE, false);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, false);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_SAVE, false);
		m_pToolBar->EnableTool(wxID_OPEN, false);
		m_pToolBar->EnableTool(wxID_SAVE, false);
		m_pToolBar->EnableTool(wxID_COPY, false);
		m_pToolBar->EnableTool(wxID_CUT, false);
		m_pToolBar->EnableTool(wxID_DELETE, false);
		m_pToolBar->EnableTool(wxID_PASTE, false);
#ifdef WKTOOLS_MAPPER
		m_pToolBar->EnableTool(wk_ID_HOSTSEARCH, false);
		m_pToolBar->EnableTool(wk_ID_MACDBUPDATE, false);
#endif
		break;
	}
}


void Cwktools4Frame::OnNewSettingsFont(wxCommandEvent &event)
{
	wxFont fontNeu = globalSettings->getFont();
	wknSettings->resetFont(fontNeu);
#ifdef WKTOOLS_MAPPER
	wkmSettings->resetFont(fontNeu);
#endif
	wkeSettings->resetFont(fontNeu);
	wklSettings->resetFont(fontNeu);
	wzrdSettings->resetFont(fontNeu);
	schedSettings->resetFont(fontNeu);
}


void Cwktools4Frame::OnCopy(wxCommandEvent& event)
{
	logAusgabe->logFenster->Copy();
	logAusgabe->debugFenster->Copy();
}


void Cwktools4Frame::OnCut(wxCommandEvent& event)
{
	logAusgabe->logFenster->Cut();
	logAusgabe->debugFenster->Cut();
}


void Cwktools4Frame::OnPaste(wxCommandEvent& event)
{
	logAusgabe->logFenster->Paste();
	logAusgabe->debugFenster->Paste();
}


void Cwktools4Frame::OnDelete(wxCommandEvent& event)
{
	logAusgabe->logFenster->Clear();
	logAusgabe->debugFenster->Clear();
}


void Cwktools4Frame::OnOpen(wxCommandEvent& event)
{
	wxString wildcard =	wxT("TXT files (*.txt)|*.txt");
	wxFileDialog dialog(this, "", "", "", wildcard, wxOPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
	if (dialog.ShowModal() == wxID_OK)
	{
		if (toolAlt == wk_ID_SCHED)
		{
			wxString path = dialog.GetPath();
			sched->ladeSchedFile(path);
		}
		else
		{
			wxString path = dialog.GetPath();
			logAusgabe->logFenster->LoadFile(path);
		}
	}
	else
	{
		logAusgabe->schreibeLog(WkLog::WkLog_ABSATZ, "0202: GENERAL: Unable to open file!", "0202", 0,
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}	
}


void Cwktools4Frame::OnSave(wxCommandEvent& event)
{
	wxString wildcard =	wxT("TXT files (*.txt)|*.txt");
	wxFileDialog dialog(this, "", "", "", wildcard, wxSAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
	if (dialog.ShowModal() == wxID_OK)
	{
		if (toolAlt == wk_ID_SCHED)
		{
			wxString path = dialog.GetPath();
			sched->sichereSchedFile(path);
		}
		else
		{
			wxString path = dialog.GetPath();
			logAusgabe->logFenster->SaveFile(path);
		}
	}
	else
	{
		logAusgabe->schreibeLog(WkLog::WkLog_ABSATZ, "0203: GENERAL: Unable to save file!", "0203", 0, 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}	
}


void Cwktools4Frame::OnVCheckFertig(wxCommandEvent& event)
{
	int tid = event.GetInt();
	if (tid == 1)
	{
		// Neuere Version verfügbar
		if (updateMessage)
		{
			wxString msg = wxT("Version Checker: A new version of wktools4 is available. Please visit www.spoerr.org/wktools for downloads and changelog.\nPress Cancel if you do not want to see this dialog again.");

			wxMessageDialog *umDialog = new wxMessageDialog(this, msg, "wktools4 Version Check",
				wxOK|wxCANCEL|wxICON_INFORMATION);
			if (umDialog->ShowModal() == wxID_CANCEL)
			{
				updateMessage = 0;
			}
		}
	}
	else if (tid == 0)
	{
		// Aktuellste Version wird verwendet => Zurücksetzen von updateMessage
		updateMessage = 1;
	}
	else
	{
		// Fehler beim Version Check
		updateMessage = 1;
	}

}


void Cwktools4Frame::OnLogReduce(wxCommandEvent& event)
{
	logAusgabe->keinDebug(event.IsChecked());
}


void Cwktools4Frame::OnHighlight(wxCommandEvent &event)
{
	wknSettings->setFarbe(event.GetInt());
#ifdef WKTOOLS_MAPPER
	wkmSettings->setFarbe(event.GetInt());
#endif
	wkeSettings->setFarbe(event.GetInt());
	wklSettings->setFarbe(event.GetInt());
	wzrdSettings->setFarbe(event.GetInt());
	schedSettings->setFarbe(event.GetInt());
}


void Cwktools4Frame::OnDevGrpRefresh(wxCommandEvent& event)
{
	einspielen->aktualisiereDevGroup();
	einspielen->schreibeDevGroup();
	wkeSettings->Update();
}


void Cwktools4Frame::OnDevGrpSave(wxCommandEvent& event)
{
	einspielen->schreibeDevGroup();
}


void Cwktools4Frame::OnXmlSwap(wxCommandEvent& event)
{
	wxGetApp().doSwapXml();
}


void Cwktools4Frame::OnHostSearch(wxCommandEvent& event)
{
#ifdef WKTOOLS_MAPPER
	mapper->doMacSearch();
#endif
}


void Cwktools4Frame::OnMacDBUpdate(wxCommandEvent& event)
{
#ifdef WKTOOLS_MAPPER
	wxMessageDialog *msg = new wxMessageDialog(this,wxT("Loading OUI DB will take a while."), wxT("OUI DB Update?") ,wxOK | wxCANCEL | wxICON_EXCLAMATION);
	if ( msg->ShowModal() == wxID_OK )
	{
		mapper->doMacUpdate();
	}
#endif
}


void Cwktools4Frame::OnWknPropGridChange(wxCommandEvent &event)
{
	erstellen->doTestDataFileOnly();
}


void Cwktools4Frame::OnSchedLoadProfile(wxCommandEvent &event)
{
	sched->ladeSchedProfile();
}


void Cwktools4Frame::OnNeuerEintrag(wxCommandEvent &event)
{
	sched->neuerEintrag();
}


void Cwktools4Frame::OnSchedLoadFile(wxCommandEvent &event)
{
	sched->aktualisiereSchedTabelle();
}


void Cwktools4Frame::OnSchedRowSelect(wxCommandEvent &event)
{
	int selRow = event.GetInt();
	sched->bearbeiteEintrag(selRow);
}


void Cwktools4Frame::OnSchedRowDelete(wxCommandEvent &event)
{
	int selRow = event.GetInt();
	sched->loescheEintrag(selRow);
}


void Cwktools4Frame::dirMapTabelleInit()
{
	// Tabelle erstellen
	dirMapTabelle->tabellenAnsicht->InsertCols();
	dirMapTabelle->tabellenAnsicht->SetColLabelValue(0, "Windows Directory");
	dirMapTabelle->tabellenAnsicht->SetColLabelValue(1, "Linux Directory");
	dirMapTabelle->tabellenAnsicht->InsertRows(0, 50);
	dirMapTabelle->tabellenAnsicht->SetColSize(0, 300);
	dirMapTabelle->tabellenAnsicht->SetColSize(1, 300);

	// Einträge aus wktools.xml auslesen und in die Tabelle eintragen
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSets;	// das Element, das auf Settings zeig
	TiXmlElement *pElemDirMaps;	// das Element, das auf die DirMapping Einträge zeigt

	// Settings im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (pElem)
	{
		pElemSets = pElem->FirstChildElement("Settings");
		pElemDirMaps = pElemSets->FirstChildElement("DirMapping");

		for (int i = 0; pElemDirMaps; i++)
		{
			std::string winDir;
			std::string linDir;
			if (pElemDirMaps->Attribute("Win"))
			{
				winDir = pElemDirMaps->Attribute("Win");
				linDir = pElemDirMaps->Attribute("Linux");
				if (dirMapTabelle->tabellenAnsicht->GetNumberRows() < i)
				{
					dirMapTabelle->zeileHinzu();
				}
				dirMapTabelle->tabellenAnsicht->SetCellValue(i, 0, winDir.c_str());
				dirMapTabelle->tabellenAnsicht->SetCellValue(i, 1, linDir.c_str());
			}
			pElemDirMaps = pElemDirMaps->NextSiblingElement();
		}
	}
}


void Cwktools4Frame::dirMapTabelleSichern()
{
	// Einträge aus wktools.xml auslesen und in die Tabelle eintragen
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSets;	// das Element, das auf Settings zeigt
	TiXmlElement *pElemDirMaps;	// das Element, das auf die DirMapping Einträge zeigen

	// Settings im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (pElem)
	{
		pElemSets = pElem->FirstChildElement("Settings");
		pElemDirMaps = pElemSets->FirstChildElement("DirMapping");
		
		// Alle bestehenden Einträge aus dem xml File entfernen und dann neu anlegen
		for (int i = 0; pElemDirMaps; i++)
		{
			pElemSets->RemoveChild(pElemDirMaps);
			pElemDirMaps = pElemSets->FirstChildElement("DirMapping");
		}
		
		// Einträge im xml File neu befüllen
		for (int i = 0; i < dirMapTabelle->tabellenAnsicht->GetNumberRows(); i++)
		{
			std::string winDirTab = dirMapTabelle->tabellenAnsicht->GetCellValue(i, 0).c_str();
			std::string linDirTab = dirMapTabelle->tabellenAnsicht->GetCellValue(i, 1).c_str();

			if (winDirTab != "")
			{
				// Neuen Eintrag in XML File erstellen
				pElemDirMaps = new TiXmlElement("DirMapping");		
				pElemSets->LinkEndChild(pElemDirMaps);
				pElemDirMaps->SetAttribute("Win", winDirTab);
				pElemDirMaps->SetAttribute("Linux", linDirTab);
			}
			else
			{
				break;
			}
		}
	}
}