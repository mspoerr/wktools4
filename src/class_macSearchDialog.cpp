#include "class_macSearchDialog.h"
#ifdef WKTOOLS_MAPPER

#include <boost/algorithm/string.hpp> 
#include <boost/algorithm/string/erase.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>

DEFINE_LOCAL_EVENT_TYPE(wkEVT_SEARCH)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_CLOSE)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_CLEAR)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_END2END)
DEFINE_LOCAL_EVENT_TYPE(wkEVT_BOXSELECT)

BEGIN_EVENT_TABLE(MacSearchDialog, wxFrame)
	EVT_CLOSE(MacSearchDialog::OnClose)
	EVT_COMMAND(wxID_ANY, wkEVT_CLOSE, MacSearchDialog::OnQuit)
	EVT_COMMAND(wxID_ANY, wkEVT_CLEAR, MacSearchDialog::OnClear)
	EVT_COMMAND(wxID_ANY, wkEVT_SEARCH, MacSearchDialog::OnSearch)
	EVT_COMMAND(wxID_ANY, wkEVT_END2END, MacSearchDialog::OnEnd2End)
	EVT_COMMAND(wxID_ANY, wkEVT_BOXSELECT, MacSearchDialog::OnSearch)
END_EVENT_TABLE()



MacSearchDialog::MacSearchDialog(const wxString& title)
: wxFrame(NULL, -1, title, wxPoint(-1, -1), wxSize(500, 500))
{ 
#ifdef _WINDOWS_
	SetIcon(wxICON(APPICON));
#else
	SetIcon(wxICON(wktools41));
#endif		
	
	wxPanel *panel = new wxPanel(this, -1);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	// MAC Adress Eingabe
	wxBoxSizer *hbox1 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *st1 =  new wxStaticText(panel, wxID_ANY, wxT("MAC Address"));
	hbox1->Add(st1, 0, wxRIGHT, 20);
	macEingabe = new wxTextCtrl(panel, wxID_ANY);
	hbox1->Add(macEingabe, 1);
	vbox->Add(hbox1, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(-1, 10);

	// IP Adress Eingabe
	wxBoxSizer *hbox10 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *st10 =  new wxStaticText(panel, wxID_ANY, wxT("IP Address"));
	hbox10->Add(st10, 0, wxRIGHT, 32);
	ipEingabe = new wxTextCtrl(panel, wxID_ANY);
	hbox10->Add(ipEingabe, 1);
	vbox->Add(hbox10, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(-1, 10);

	// CSV Input Operation
	wxBoxSizer *hbox12 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *st12 =  new wxStaticText(panel, wxID_ANY, wxT("csv file path"));
	hbox12->Add(st12, 0, wxRIGHT, 26);
	csvOperation = new wxTextCtrl(panel, wxID_ANY);
	hbox12->Add(csvOperation, 1);
	vbox->Add(hbox12, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(-1, 10);

	// End2End IP Adress Eingabe
	wxBoxSizer *hbox13 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *st13 = new wxStaticText(panel, wxID_ANY, wxT("H1/H2"));
	hbox13->Add(st13, 0, wxRIGHT, 54);
	e2eIp1 = new wxTextCtrl(panel, wxID_ANY);
	e2eIp2 = new wxTextCtrl(panel, wxID_ANY);
	hbox13->Add(e2eIp1, 1);
	hbox13->Add(e2eIp2, 1);
	vbox->Add(hbox13, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(-1, 10);

	// Box/Interface Eingabe
	wxBoxSizer *hbox11 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *st11 =  new wxStaticText(panel, wxID_ANY, wxT("BOX / Interface"));
	hbox11->Add(st11, 0, wxRIGHT, 9);
	boxSelect = new wxComboBox(panel, wkEVT_BOXSELECT, "", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY|wxCB_SORT);
	intfSelect = new wxComboBox(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY|wxCB_SORT);
	hbox11->Add(boxSelect, 1);
	hbox11->Add(intfSelect, 1);
	vbox->Add(hbox11, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	Connect(wkEVT_BOXSELECT, wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(MacSearchDialog::OnBoxSelect));

	//// Checkboxen
	//vbox->Add(-1, 25);
	//wxBoxSizer *hbox4 = new wxBoxSizer(wxHORIZONTAL);
	//wxCheckBox *cb1 = new wxCheckBox(panel, wxID_ANY, wxT("Trunks"));
	//hbox4->Add(cb1);
	//wxCheckBox *cb2 = new wxCheckBox(panel, wxID_ANY, wxT("History"));
	//hbox4->Add(cb2, 0, wxLEFT, 10);
	//vbox->Add(hbox4, 0, wxLEFT, 10);

	// Result
	vbox->Add(-1, 15);
	wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *st2 = new wxStaticText(panel, wxID_ANY, wxT("Result"));

	hbox2->Add(st2, 0);
	vbox->Add(hbox2, 0, wxLEFT | wxTOP, 10);

	vbox->Add(-1, 10);

	wxBoxSizer *hbox3 = new wxBoxSizer(wxHORIZONTAL);
	ergebnisAusgabe = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);

	hbox3->Add(ergebnisAusgabe, 1, wxEXPAND);
	vbox->Add(hbox3, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);

	// Buttons
	vbox->Add(-1, 25);
	wxBoxSizer *hbox5 = new wxBoxSizer(wxHORIZONTAL);
	wxButton *btn13 = new wxButton(panel, wkEVT_END2END, wxT("E2E"));
	hbox5->Add(btn13, 0);
	wxButton *btn1 = new wxButton(panel, wkEVT_SEARCH, wxT("Search"));
	hbox5->Add(btn1, 0, wxLEFT | wxBOTTOM, 5);
	wxButton *btn2 = new wxButton(panel, wkEVT_CLEAR, wxT("Clear"));
	hbox5->Add(btn2, 0, wxLEFT | wxBOTTOM , 5);
	wxButton *btn3 = new wxButton(panel, wkEVT_CLOSE, wxT("Close"));
	hbox5->Add(btn3, 0, wxLEFT | wxBOTTOM , 5);
	vbox->Add(hbox5, 0, wxALIGN_RIGHT | wxRIGHT, 10);
	Connect(wkEVT_CLOSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MacSearchDialog::OnQuit));
	Connect(wkEVT_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MacSearchDialog::OnClear));
	Connect(wkEVT_SEARCH, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MacSearchDialog::OnSearch));
	Connect(wkEVT_END2END, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MacSearchDialog::OnEnd2End));

	panel->SetSizer(vbox);

	Centre();
}


void MacSearchDialog::OnClear(wxCommandEvent& event)
{
	macEingabe->Clear();
	ipEingabe->Clear();
	ergebnisAusgabe->Clear();
	intfSelect->Clear();
}


std::string MacSearchDialog::macSearch(std::string macAddr, std::string trennzeichen, bool popup, bool history)
{
	// Als erstes das MAC Adressformat anpassen:
	boost::algorithm::to_lower(macAddr);
	if (macAddr.find(".") == macAddr.npos)
	{
		// Dann alle - und : löschen, falls vorhanden
		boost::algorithm::erase_all(macAddr, "-");
		boost::algorithm::erase_all(macAddr, ":");

		// Dann die "." einfügen
		macAddr.insert(8, ".");
		macAddr.insert(4, ".");
	}

	std::string ergebnis = "";
	std::string sString = "SELECT DISTINCT intf_id, intfname,intfType,description FROM interfaces INNER JOIN nlink ON nlink.interfaces_intf_id=interfaces.intf_id ";
	sString += "INNER JOIN neighbor ON neighbor.neighbor_id=nlink.neighbor_neighbor_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE neighbor.l2_addr LIKE '";
	sString += macAddr + "' AND l2l3 LIKE 'L2' AND device.hwtype=2 "; // 18.11.2014: Entfernt --->AND intf_id NOT IN (SELECT int_vlan.interfaces_intf_id FROM int_vlan) ";<---
	sString += "AND intf_id NOT IN (SELECT dI_intf_id FROM neighborship WHERE dI_intf_id1 IN (SELECT dI_intf_id1 FROM neighborship ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=neighborship.dI_intf_id1 INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	sString += "WHERE device.hwtype=2))	AND intf_id NOT IN (SELECT dI_intf_id1 FROM neighborship WHERE dI_intf_id IN (SELECT dI_intf_id FROM neighborship ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=neighborship.dI_intf_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.hwtype=2)) ORDER BY intf_id DESC";

	bool ersterDurchlauf = true;
	std::vector<std::vector<std::string>> intf = dieDB->query(sString.c_str());

	if (!intf.empty())
	{
		for (std::vector<std::vector<std::string>>::iterator it = intf.begin(); it < intf.end(); ++it)
		{
			// Check ob Interface BoundTo_id eingetragen ist und ob das entsprechende Interface zu einer Nachbarschaft gehört
			sString = "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship UNION ALL SELECT dI_intf_id1 FROM neighborship) ";
			sString += "WHERE col IN (SELECT boundTo_id FROM interfaces WHERE intf_id=" + it->at(0) + ")";
			std::vector<std::vector<std::string>> check = dieDB->query(sString.c_str());
			if (!check.empty())
			{
				continue;
			}
			// Check ob Interface channel_intf_id eingetragen ist und ob das entsprechende Interface zu einer Nachbarschaft gehört
			sString = "SELECT col FROM (SELECT dI_intf_id AS col FROM neighborship UNION ALL SELECT dI_intf_id1 FROM neighborship) ";
			sString += "WHERE col IN (SELECT channel_intf_id FROM interfaces WHERE intf_id=" + it->at(0) + ")";
			check = dieDB->query(sString.c_str());
			if (!check.empty())
			{
				continue;
			}

			sString = "SELECT hostname FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id WHERE devInterface.interfaces_int_id=" + it->at(0);
			std::vector<std::vector<std::string>> host = dieDB->query(sString.c_str());
			if (!host.empty())
			{
				ergebnis += "\r\nResult for " + macAddr + trennzeichen;
				ergebnis += "Switch: " + host[0][0] + trennzeichen;
				ergebnis += "Interface: " + it->at(1) + trennzeichen;
				ergebnis += "Interface Description: " + it->at(3);

				if (it->at(2) == "Channel")
				{
					ergebnis += " (Channel Members:";
					sString = "SELECT intfName FROM interfaces WHERE channel_intf_id=" + it->at(0);
					std::vector<std::vector<std::string>> chMem = dieDB->query(sString.c_str());
					for (std::vector<std::vector<std::string>>::iterator chit = chMem.begin(); chit < chMem.end(); ++chit)
					{
						ergebnis += " " + chit->at(0);
					}
					ergebnis += ")";
				}
				ergebnis += trennzeichen;
			}
			else
			{
				std::string dbgA = "\n6208: DB error (hostSearch 0001). Please contact wktools@spoerr.org";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "6208",
					WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				wxMessageBox(dbgA);
				break;
			}


			std::string oui = macAddr.substr(0, 7);
			sString = "SELECT vendor FROM macOUI WHERE oui LIKE '" + oui + "'";
			std::vector<std::vector<std::string>> vendor = dieDB->query(sString.c_str());
			if (!vendor.empty())
			{
				ergebnis += "Vendor: " + vendor[0][0] + "\r\n";
			}

			if (!history)
			{
				break;
			}
			
			if (ersterDurchlauf)
			{
				ersterDurchlauf = false;
				ergebnis += "\r\nHistory:\r\n";
			}
		}
	}
	else if (popup)
	{
		wxMessageBox("MAC Address " + macAddr + " not found!");
	}
	else
	{
		ergebnis += "\r\nResult for " + macAddr + trennzeichen;
		ergebnis += "UNKNOWN";
	}
	return ergebnis;
}


std::string MacSearchDialog::ipSearch(std::string ipAddr)
{
	WkmEnd2End *e2e = new WkmEnd2End(logAusgabe, dieDB);
	WkmEnd2End::hostDetails hDet = e2e->ipSearch(ipAddr);

	std::string ergebnis = "";

	if (hDet.anzahl)
	{
		for (int i = 0; i < hDet.anzahl; i++)
		{
			if (i == 1)
			{
				ergebnis += "\r\nHistory:\r\n";
			}

			ergebnis += "Result for " + ipAddr + " (Timestamp: " + hDet.timestamp[i] + "):\r\n";
			ergebnis += "MAC Address: " + hDet.hMac[i] + "\r\n";
			ergebnis += "Switch: " + hDet.hHostname[i] + "\r\n";
			ergebnis += "Interface: " + hDet.hIntfName[i] + "\r\n";
			ergebnis += "Interface Description: " + hDet.hIntfDescr[i];

			if (hDet.hIntfType[i] == "Channel")
			{
				ergebnis += " (Channel Members:";
				std::string sString = "SELECT intfName FROM interfaces WHERE channel_intf_id=" + hDet.hintf_id[i];
				std::vector<std::vector<std::string>> chMem = dieDB->query(sString.c_str());
				for (std::vector<std::vector<std::string>>::iterator chit = chMem.begin(); chit < chMem.end(); ++chit)
				{
					ergebnis += " " + chit->at(0);
				}
				ergebnis += ")";
			}
			ergebnis += "\r\n";

			if (hDet.hOuiVendor[i] != "")
			{
				ergebnis += "Vendor: " + hDet.hOuiVendor[i] + "\r\n";
			}
		}
	}
	else
	{
		wxMessageBox("IP Address " + ipAddr + " not found!");
		ergebnis = "UNKNOWN";
	}

	delete e2e;
	return ergebnis;
}


std::string MacSearchDialog::intfSearch(std::string box, std::string intf)
{
	std::string ergebnis = "";
	std::string sString = "SELECT DISTINCT l2_addr FROM neighbor INNER JOIN nlink ON nlink.neighbor_neighbor_id=neighbor.neighbor_id ";
	sString += "WHERE nlink.interfaces_intf_id IN (SELECT intf_id FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN device ON devInterface.device_dev_id=device.dev_id WHERE intfName LIKE '";
	sString += intf  + "' AND device.hostname LIKE '" + box + "')";

	bool ersterDurchlauf = true;
	std::vector<std::vector<std::string>> l2A = dieDB->query(sString.c_str());
	for (std::vector<std::vector<std::string>>::iterator it = l2A.begin(); it < l2A.end(); ++it)
	{
		ergebnis += "Result for " + box + "/" + intf + ":\r\n";
		ergebnis += "MAC Address: " + it->at(0) + "\r\n";
		sString = "SELECT DISTINCT l3_addr FROM neighbor WHERE l3_addr NOT NULL AND l2_addr LIKE '" + it->at(0) + "'";
		std::vector<std::vector<std::string>> l3A = dieDB->query(sString.c_str());
		for (std::vector<std::vector<std::string>>::iterator it1 = l3A.begin(); it1 < l3A.end(); ++it1)
		{
			ergebnis += "IP Address: " + l3A[0][0] + "\r\n";
		}

		std::string oui = it->at(0).substr(0, 7);
		sString = "SELECT vendor FROM macOUI WHERE oui LIKE '" + oui + "'";
		std::vector<std::vector<std::string>> vendor = dieDB->query(sString.c_str());
		if (!vendor.empty())
		{
			ergebnis += "Vendor: " + vendor[0][0] + "\r\n";
		}

		if (ersterDurchlauf)
		{
			ersterDurchlauf = false;
			ergebnis += "\r\nHistory:\r\n";
		}
	}
	return ergebnis;
}


std::string MacSearchDialog::csvSearch(std::string csvFileName)
{
	std::string ergebnis = "";
	
	bool fehler = false;
	std::ifstream wdat(csvFileName.c_str(), std::ios_base::in);
	if (!wdat.is_open())
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "File not found!", "1303",
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		std::string werte;
		std::string seps = ",;\r\n\t";
		size_t pos1 = 0;
		size_t pos2 = 0;

		while (getline(wdat, werte))
		{
			// MAC Adresse, die immer ganz am Zeilenanfang stehen muss, raussuchen und verarbeiten
			pos2 = werte.find_first_of(seps, pos1);
			std::string macAdresse = werte.substr(pos1, pos2-pos1);
			ergebnis+= macSearch(macAdresse, ",", false, false);
		}

	}

	wdat.close();

	return ergebnis;
}


void MacSearchDialog::OnEnd2End(wxCommandEvent& event)
{
	std::string host1 = e2eIp1->GetValue();
	std::string host2 = e2eIp2->GetValue();

	std::string ergebnis = "";
	std::string ueberschrift = "";

	ergebnis = end2end(host1, host2);

	ergebnis += "\r\n--------------------------------------------\r\n\r\n";
	ergebnisAusgabe->AppendText(ergebnis.c_str());

	schreibeLog(WkLog::WkLog_NORMALTYPE, ueberschrift, "",
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	schreibeLog(WkLog::WkLog_NORMALTYPE, ergebnis, "",
		WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
}


std::string MacSearchDialog::end2end(std::string box, std::string intf)
{
	std::string ergebnis = "";

	WkmEnd2End *e2e = new WkmEnd2End(logAusgabe, dieDB);

	ergebnis = e2e->doEnd2End(box, intf);

	return ergebnis;
}


void MacSearchDialog::OnSearch(wxCommandEvent& event)
{
	std::string macAddr = macEingabe->GetValue();
	std::string ipAddr = ipEingabe->GetValue();
	std::string box = boxSelect->GetValue();
	std::string intf = intfSelect->GetValue();
	std::string csvInput = csvOperation->GetValue();

	std::string ergebnis = "";
	std::string ueberschrift = "";

	if (macAddr != "")
	{
		ueberschrift = "\r\nSearch Host Result for " + macAddr + "\r\n"; 
		ergebnis = macSearch(macAddr, ":\r\n", true, true);
	}
	else if (ipAddr != "")
	{
		ueberschrift = "\r\nSearch Host Result for " + ipAddr + "\r\n";
		ergebnis = ipSearch(ipAddr);
	}
	else if (intf != "")
	{
		ueberschrift = "\r\nSearch Host Result for " + box + "/" + intf + "\r\n";
		ergebnis = intfSearch(box, intf);
	}
	else if (csvInput != "")
	{
		ueberschrift = "\r\nSearch Host Result for " + csvInput + "\r\n";
		ergebnis = csvSearch(csvInput);
	}

	ergebnis += "\r\n--------------------------------------------\r\n\r\n";
	ergebnisAusgabe->AppendText(ergebnis.c_str());

	schreibeLog(WkLog::WkLog_NORMALTYPE, ueberschrift, "", 
		WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	
	schreibeLog(WkLog::WkLog_NORMALTYPE, ergebnis, "", 
		WkLog::WkLog_SCHWARZ, WkLog::WkLog_NORMALFORMAT);
}


void MacSearchDialog::OnQuit(wxCommandEvent& event)
{
	Close();
}


void MacSearchDialog::setEinstellungen(std::string dbName, WkLog *logA)
{
	dieDB = new WkmDB(dbName, true);

	logAusgabe = logA;
	
	// Box Auswahl befüllen
	std::string sString = "SELECT hostname FROM device WHERE (stpBridgeID NOT LIKE '' OR hwtype=2) AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	std::vector<std::vector<std::string>> devs = dieDB->query(sString.c_str());	
	for(std::vector<std::vector<std::string>>::iterator devIt = devs.begin(); devIt < devs.end(); ++devIt)
	{
		boxSelect->Append(devIt->at(0));
	}
}


void MacSearchDialog::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
	if (logAusgabe != NULL)
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
		wxPostEvent(logAusgabe, evt);
	}
}


void MacSearchDialog::OnBoxSelect(wxCommandEvent& event)
{
	// Intf Auswahl leeren
	intfSelect->Clear();

	std::string dev = boxSelect->GetValue().c_str();

	std::string sString = "SELECT intfName FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.hostname LIKE '" + dev;
	sString += "' AND device.dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl))";
	std::vector<std::vector<std::string>> intfs = dieDB->query(sString.c_str());	
	for(std::vector<std::vector<std::string>>::iterator intfIt = intfs.begin(); intfIt < intfs.end(); ++intfIt)
	{
		intfSelect->Append(intfIt->at(0));
	}

}


void MacSearchDialog::OnClose(wxCloseEvent& event)
{
	delete dieDB;
	event.Skip();
}

#endif