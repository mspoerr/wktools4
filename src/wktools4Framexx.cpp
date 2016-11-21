/***************************************************************
* Name:      wktoolsMain.cpp
* Purpose:   Code for Application Frame
* Author:    Mathias Spoerr (wktools@spoerr.org)
* Created:   2007-11-03
* Copyright: Mathias Spoerr (http://www.spoerr.org/wktools)
* License:   Freeware

**************************************************************/
#include "wktools4Frame.h"
#include <wx/aboutdlg.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

// XPM Files für Tool Selector Bar
#include "res/about.xpm"
#include "res/dip.xpm"
#include "res/wkn.xpm"
#include "res/wkc.xpm"
#include "res/wke.xpm"
#include "res/wkp.xpm"
#include "res/wkl.xpm"
#include "res/doku.xpm"
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
#include "toolbar/search.xpm"
#include "toolbar/save.xpm"
#include "toolbar/settings.xpm"
#include "toolbar/logReduce.xpm"
#include "toolbar/dgfRefresh.xpm"

#include "res/htms.h"


#ifdef __WXDEBUG__
#define new WXDEBUG_NEW
#endif

// Funktionszeiger
//void(Wkn::*wkn_ssc)();		// Funktionszeiger für den Gebrauch vom Start/Stop Button für Wkn
//wkn_ssc = &Wkn::doIt();
//(erstellen->*wkn_ssc())();


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
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKN_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKE_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKC_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKP_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKL_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_WKDP_FERTIG)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_VCHECK_FERTIG)

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
	EVT_TOOL_RANGE(wk_ID_PROFILE_LOAD, wk_ID_PROFILE_SAVE, Cwktools4Frame::OnToolProfLoadSave)
	EVT_TOOL_RANGE(wk_ID_START, wk_ID_KILL, Cwktools4Frame::OnToolStartStop)
	EVT_TOOL(wk_ID_LOGRED, Cwktools4Frame::OnLogReduce)
	EVT_TOOL(wxID_OPEN, Cwktools4Frame::OnOpen)
	EVT_TOOL(wxID_SAVE, Cwktools4Frame::OnSave)
	EVT_TOOL(wxID_CUT, Cwktools4Frame::OnCut)
	EVT_TOOL(wxID_COPY, Cwktools4Frame::OnCopy)
	EVT_TOOL(wxID_PASTE, Cwktools4Frame::OnPaste)
	EVT_TOOL(wxID_DELETE, Cwktools4Frame::OnDelete)
	EVT_TOOL(wk_ID_DEVGRP_REFRESH, Cwktools4Frame::OnDevGrpRefresh)
	EVT_SPLITTER_DCLICK(wxID_MAINSPLITTER, Cwktools4Frame::OnDMainClickSash)
	EVT_SPLITTER_DCLICK(wxID_RSPLITTER, Cwktools4Frame::OnDRClickSash)
	EVT_COMMAND(wxID_ANY, wkEVT_FONTAENDERUNG, Cwktools4Frame::OnNewSettingsFont)
	EVT_COMMAND(wxID_ANY, wkEVT_HIGHLIGHT, Cwktools4Frame::OnHighlight)
	EVT_COMMAND(wxID_ANY, wkEVT_WKN_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKE_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKC_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKP_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKL_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_WKDP_FERTIG, Cwktools4Frame::OnToolFertig)
	EVT_COMMAND(wxID_ANY, wkEVT_VCHECK_FERTIG, Cwktools4Frame::OnVCheckFertig)
END_EVENT_TABLE()


Cwktools4Frame::~Cwktools4Frame(void)
{
	delete toolSelector;
	delete wkHilfe;
	delete logAusgabe;
	delete wkeSettings;
	delete wknSettings;
	delete rSplitter;
	delete mainSplitter;
	delete profilCombo;
	delete dieDevGrp;
}


bool Cwktools4Frame::Create(wxWindow *pParent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
	wxFrame::Create(pParent, id, title, pos, size, style, name);
	m_horzToolbar = true;
	m_horzText = false;
	m_rows = 1;

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
			int wkeSpos = 0;
			int glSpos = 0;
			pElemSize = pElemSets->FirstChildElement("toolSizing");
			pElemSize->QueryIntAttribute("wkn", &wknSpos);
			pElemSize->QueryIntAttribute("wke", &wkeSpos);
			
			
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

			if (!wknSpos)
			{
				wknSpos = mainSplitterPos/2;
			}
			if (!wkeSpos)
			{
				wkeSpos = mainSplitterPos/2;
			}
			wknSettings->setSplitterPos(wknSpos);
			wkeSettings->setSplitterPos(wkeSpos);

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
			wkeSettings->resetFont(fontSets);
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

	
	SetIcon(wxICON(APPICON));

	wxMenu *menuFile = new wxMenu;
	menuFile->Append( wxID_CLOSE, wxT("E&xit") );

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append( wxID_ABOUT, wxT("&About...") );

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, wxT("&File") );
	menuBar->Append( menuHelp, wxT("&Help") );

	SetMenuBar( menuBar );

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

	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_START, wxT("Start"), wxBitmap(doit_xpm), wxT("Start"), wxITEM_CHECK);
	m_pToolBar->AddSeparator();
	//m_pToolBar->AddTool(wxID_ANY, wxT("Cancel"), wxBitmap(cancel_xpm), wxT("Cancel"));
	m_pToolBar->AddTool(wk_ID_KILL, wxT("Kill"), wxBitmap(kill_xpm), wxT("Kill"));
	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_SETTINGS, wxT("Settings"), wxBitmap(settings_xpm), wxT("Settings"), wxITEM_CHECK);
	m_pToolBar->AddSeparator();
	m_pToolBar->AddTool(wk_ID_LOGRED, wxT("Hide Debug Output"), wxBitmap(logReduce_xpm), wxT("Hide Debug Output"), wxITEM_CHECK);
	m_pToolBar->AddTool(wk_ID_DEVGRP_REFRESH, wxT("DeviceGroup Window Refresh"), wxBitmap(dgfRefresh_xpm), wxT("DeviceGroup Window Refresh"));

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
	toolSelector->AddTool(wk_ID_WKN, wxT("Configmaker"), wxBitmap(wkn_xpm), wxT("ConfigMaker"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_WKE, wxT("Configure Devices"), wxBitmap(wke_xpm), wxT("Configure Devices"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_WKL, wxT("IP List"), wxBitmap(wkl_xpm), wxT("IP List"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_WKC, wxT("Compare Files"), wxBitmap(wkc_xpm), wxT("Compare Files"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_WKDP, wxT("Device Information Parser"), wxBitmap(dip_xpm), wxT("Device Information Parser"), wxITEM_CHECK);
	toolSelector->AddSeparator();
	toolSelector->AddTool(wk_ID_WKP, wxT("PIX Session Status"), wxBitmap(wkp_xpm), wxT("PIX Session Status"), wxITEM_CHECK);
	toolSelector->AddSeparator();
	toolSelector->AddTool(wk_ID_SCHED, wxT("Scheduler"), wxBitmap(sched_xpm), wxT("Scheduler"), wxITEM_CHECK);
	toolSelector->AddSeparator();
	toolSelector->AddTool(wk_ID_ABOUT, wxT("About"), wxBitmap(doku_xpm), wxT("About"), wxITEM_CHECK);
	toolSelector->AddTool(wk_ID_LIC, wxT("License"), wxBitmap(lic_xpm), wxT("License"), wxITEM_CHECK);
	toolSelector->Realize();

	// Top Level Sizer
	wxFlexGridSizer *fgs = new wxFlexGridSizer(1, 2, 0, 0);
	fgs->AddGrowableCol(1);
	fgs->AddGrowableRow(0);
	fgs->Add(toolSelector, 0, wxEXPAND);



	// Splitter für die Tools einfügen
	mainSplitter = new wxSplitterWindow(mainFrame, wxID_MAINSPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3D);
	rPanel = new wxPanel(mainSplitter, wxID_ANY);
	rSplitter = new wxSplitterWindow(rPanel, wxID_RSPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3D);
	logAusgabe = new WkLog(rSplitter, wxID_ANY);
	dgPanel = new wxPanel(rSplitter, wxID_ANY);
	dieDevGrp = new DevGrp(dgPanel, wxID_ANY);
	dieDevGrp->xmlInit(doc);

	// Sizer für die DeviceGroup
	wxFlexGridSizer *gs = new wxFlexGridSizer(1, 1, 0, 0);
	gs->AddGrowableCol(0);
	gs->AddGrowableRow(0);

	// Sizer für den rechten Splitter
	wxFlexGridSizer *fgsr = new wxFlexGridSizer(1, 1, 0, 0);
	fgsr->AddGrowableCol(0);
	fgsr->AddGrowableRow(0);

	wknSettings = new CWknPropGrid(mainSplitter, wxID_ANY);
	wkeSettings = new CWkePropGrid(mainSplitter, wxID_ANY);
	wkHilfe = new CWkHilfe(mainSplitter, wxID_ANY);
	globalSettings = new CSettings(mainSplitter, wxID_ANY);
	wkeSettings->Hide();
	wkHilfe->Hide();
	globalSettings->Hide();

	mainSplitter->SplitVertically(wknSettings, rPanel, 200);
	mainSplitter->SetMinimumPaneSize(1);

	dieDevGrp->Hide();
	rSplitter->Initialize(logAusgabe);
	rSplitter->SetMinimumPaneSize(1);

	gs->Add(dieDevGrp, 0, wxEXPAND, 0);
	dgPanel->SetSizer(gs);
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
	erstellen->setMainFrame(mainFrame);
	einspielen->setMainFrame(mainFrame);

	//create the statusbar
	CreateStatusBar();
	SetStatusText(wxT("Ready"));

	// Sizer 
	SetSizer(fgs);
	SetAutoLayout(true);
	Layout();

}


void Cwktools4Frame::OnClose(wxCloseEvent& event)
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

	switch (toolNeu)
	{
	case wk_ID_WKE:
		rSplitterPos = rSplitter->GetSashPosition();
	case wk_ID_WKN:
	case wk_ID_WKC:
	case wk_ID_WKL:
	case wk_ID_WKDP:
	case wk_ID_WKP:
		mainSplitterPos = mainSplitter->GetSashPosition();
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



		// Tool Splitter Positionen speichern
		pElemSize = pElemSets->FirstChildElement("toolSizing");
		if (!pElemSize)
		{
			pElemSize = new TiXmlElement("toolSizing");
			pElemSets->LinkEndChild(pElemSize);
		}
		pElemSize->SetAttribute("wkn", wknSettings->getSplitterPos());
		pElemSize->SetAttribute("wke", wkeSettings->getSplitterPos());
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

	}
	
	
	event.Skip();
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
	Close();
}

void Cwktools4Frame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo info;
    info.SetName(wxT("wktools4"));
    info.SetVersion(wxT("1.0.0 Beta"));
    info.SetDescription(wxT("This program does something great."));
    info.SetCopyright(wxT("(C) 2008 Me <my@email.addre.ss>"));

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
		}
		
		m_replacewindow = mainSplitter->GetWindow1();
		
		
		if (event.GetId() == wk_ID_NEWS)
		{
			toolAlt = wk_ID_NEWS;
			mainSplitter->ReplaceWindow(m_replacewindow, wkHilfe);
			m_replacewindow->Hide();
			
			wkHilfe->ladeSeite(wktools);
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
		if (event.GetId() == wk_ID_WKN)
		{
			toolAlt = wk_ID_WKN;
			mainSplitter->ReplaceWindow(m_replacewindow, wknSettings);
			m_replacewindow->Hide();
			wknSettings->Show();
			
			profile = erstellen->ladeProfile();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(wknSettings, rPanel, mainSplitterPos);
			}
			if (rSplitter->IsSplit())
			{
				rSplitterPos = rSplitter->GetSashPosition();
				rSplitter->Unsplit(rSplitter->GetWindow1());
			}
		}

		if (event.GetId() == wk_ID_WKE)
		{
			toolAlt = wk_ID_WKE;
			mainSplitter->ReplaceWindow(m_replacewindow, wkeSettings);
			m_replacewindow->Hide();
			wkeSettings->Show();

			profile = einspielen->ladeProfile();

			if (!mainSplitter->IsSplit())
			{
				mainSplitter->SplitVertically(wkeSettings, rPanel, mainSplitterPos);
			}
			if (!rSplitter->IsSplit())
			{
				rSplitter->SplitHorizontally(dgPanel, logAusgabe, rSplitterPos);
			}

		}

		if (event.GetId() == wk_ID_WKC)
		{
			toolAlt = wk_ID_WKC;
		}

		if (event.GetId() == wk_ID_WKL)
		{
			toolAlt = wk_ID_WKL;
		}

		if (event.GetId() == wk_ID_WKP)
		{
			toolAlt = wk_ID_WKP;
		}

		if (event.GetId() == wk_ID_WKDP)
		{
			toolAlt = wk_ID_WKDP;
		}

		if (event.GetId() == wk_ID_SCHED)
		{
			toolAlt = wk_ID_SCHED;
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
			
			wkHilfe->ladeSeite(about);
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
void Cwktools4Frame::sets(TiXmlDocument *document, bool guiStats)
{
	doc = document;
	guiStatus = guiStats;
}


void Cwktools4Frame::OnToolProfLoadSave(wxCommandEvent& event)
{
	int tid = event.GetId();
	string profilName = profilCombo->GetValue();

	if (tid == wk_ID_PROFILE_LOAD)
	{
		switch (toolNeu)
		{
		case wk_ID_NEWS:
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
			break;
		case wk_ID_WKDP:
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
			break;
		case wk_ID_WKDP:
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

}


void Cwktools4Frame::OnToolStartStop(wxCommandEvent& event)
{
	int tid = event.GetId();
	m_pToolBar->EnableTool(wk_ID_KILL, true);
	
	if (tid == wk_ID_START)
	{
		// Wenn Start schon gedrückt, dann muss der Vorgang abgebrochen werden
		if (!m_pToolBar->GetToolState(wk_ID_START))
		{
			// TODO: Cancel einbauen
			switch (toolStartId)
			{
			case wk_ID_NEWS:
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
				break;
			case wk_ID_WKDP:
				break;
			case wk_ID_WKP:
				break;
			case wk_ID_SETTINGS:
				break;
			default:
				break;
			}
		}
		else
		{
			switch (toolNeu)
			{
			case wk_ID_NEWS:
				toolStartId = wk_ID_NEWS;
				break;
			case wk_ID_WKN:
				toolStartId = wk_ID_WKN;
				erstellen->doIt();
				break;
			case wk_ID_WKE:
				toolStartId = wk_ID_WKE;
				einspielen->doIt();
				break;
			case wk_ID_WKC:
				toolStartId = wk_ID_WKC;
				break;
			case wk_ID_WKL:
				toolStartId = wk_ID_WKL;
				break;
			case wk_ID_WKDP:
				toolStartId = wk_ID_WKDP;
				break;
			case wk_ID_WKP:
				toolStartId = wk_ID_WKP;
				break;
			case wk_ID_SETTINGS:
				toolStartId = wk_ID_SETTINGS;
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
		case wk_ID_WKN:
			erstellen->kilIt();
			break;
		case wk_ID_WKE:
			einspielen->kilIt();
			break;
		case wk_ID_WKC:
			break;
		case wk_ID_WKL:
			break;
		case wk_ID_WKDP:
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


void Cwktools4Frame::OnToolFertig()
{
	m_pToolBar->EnableTool(wk_ID_KILL, false);
	m_pToolBar->ToggleTool(wk_ID_START, false);
}


void Cwktools4Frame::OnToolFertig(wxCommandEvent &event)
{
	OnToolFertig();
}


void Cwktools4Frame::ToolEnableDisable()
{
	switch (toolNeu)
	{
	case wk_ID_WKN:
	case wk_ID_WKC:
	case wk_ID_WKL:
	case wk_ID_WKDP:
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, false);
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
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, true);
		m_pToolBar->EnableTool(wxID_OPEN, false);
		m_pToolBar->EnableTool(wxID_SAVE, true);
		m_pToolBar->EnableTool(wxID_COPY, true);
		m_pToolBar->EnableTool(wxID_CUT, true);
		m_pToolBar->EnableTool(wxID_DELETE, true);
		m_pToolBar->EnableTool(wxID_PASTE, true);
		break;
	case wk_ID_WKP:
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, true);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, true);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, false);
		m_pToolBar->EnableTool(wxID_OPEN, true);
		m_pToolBar->EnableTool(wxID_SAVE, true);
		m_pToolBar->EnableTool(wxID_COPY, true);
		m_pToolBar->EnableTool(wxID_CUT, true);
		m_pToolBar->EnableTool(wxID_DELETE, true);
		m_pToolBar->EnableTool(wxID_PASTE, true);
		break;
	case wk_ID_SETTINGS:
	default:
		m_pToolBar->EnableTool(wk_ID_KILL, false);
		m_pToolBar->EnableTool(wk_ID_START, false);
		m_pToolBar->EnableTool(wk_ID_PROFILE_LOAD, false);
		m_pToolBar->EnableTool(wk_ID_PROFILE_SAVE, false);
		m_pToolBar->EnableTool(wk_ID_DEVGRP_REFRESH, false);
		m_pToolBar->EnableTool(wxID_OPEN, false);
		m_pToolBar->EnableTool(wxID_SAVE, false);
		m_pToolBar->EnableTool(wxID_COPY, false);
		m_pToolBar->EnableTool(wxID_CUT, false);
		m_pToolBar->EnableTool(wxID_DELETE, false);
		m_pToolBar->EnableTool(wxID_PASTE, false);
		break;
	}
}


void Cwktools4Frame::OnNewSettingsFont(wxCommandEvent &event)
{
	wxFont fontNeu = globalSettings->getFont();
	wknSettings->resetFont(fontNeu);
	wkeSettings->resetFont(fontNeu);
	//wknSettings->Layout();
}


void Cwktools4Frame::OnCopy(wxCommandEvent& event)
{
	logAusgabe->logFenster->Copy();
}


void Cwktools4Frame::OnCut(wxCommandEvent& event)
{
	logAusgabe->logFenster->Cut();
}


void Cwktools4Frame::OnPaste(wxCommandEvent& event)
{
	logAusgabe->logFenster->Paste();
}


void Cwktools4Frame::OnDelete(wxCommandEvent& event)
{
	logAusgabe->logFenster->Clear();
}


void Cwktools4Frame::OnOpen(wxCommandEvent& event)
{
	wxString wildcard =	wxT("TXT files (*.txt)|*.txt");
	wxFileDialog dialog(this, "", "", "", wildcard, wxOPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString path = dialog.GetPath();
		logAusgabe->logFenster->LoadFile(path);
	}
	else
	{
		logAusgabe->schreibeLog(WkLog::WkLog_ABSATZ, "GENERAL: Unable to open file!", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}	
}


void Cwktools4Frame::OnSave(wxCommandEvent& event)
{
	wxString wildcard =	wxT("TXT files (*.txt)|*.txt");
	wxFileDialog dialog(this, "", "", "", wildcard, wxSAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString path = dialog.GetPath();
		logAusgabe->logFenster->SaveFile(path);
	}
	else
	{
		logAusgabe->schreibeLog(WkLog::WkLog_ABSATZ, "GENERAL: Unable to save file!", 
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
	wkeSettings->setFarbe(event.GetInt());
}


void Cwktools4Frame::OnDevGrpRefresh(wxCommandEvent& event)
{
	wxMessageBox("BLA");
}
