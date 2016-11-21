/***************************************************************
 * Name:      class_wkn.cpp
 * Purpose:   Code for ConfigMaker I/O Class
 * Created:   2007-11-03
 **************************************************************/
// Änderungen
///////////////////////////////////////////////////////////////
//
// 06.11.2007 - mspoerr:
//     - Erstellen des Files
// 04.01.2008 - mspoerr:
//     - Erweiterungen des Basisgerüsts
// 07.01.2008 - mspoerr:
//     - Erweiterungen des Basisgerüsts
//       - void sets() hinzugefügt
//       - doLoad() mit Code befüllt
// 15.01.2008 - mspoerr:
//     - Erweiterungen des Basisgerüsts
//       - doLoad() abgeschlossen
//       - einstellungenInit() hinzugefügt und mit Code befüllt


#include "class_wkn.h"
#include "class_ersetzer.h"

#include <fstream>
#include <vector>

#include <boost/tokenizer.hpp>

#include "wktools4.h"

DEFINE_LOCAL_EVENT_TYPE(wkEVT_ERSTELLER_FERTIG)

BEGIN_EVENT_TABLE(Wkn, wxEvtHandler)
EVT_COMMAND(wxID_ANY, wkEVT_ERSTELLER_FERTIG, Wkn::OnErstellerFertig)
END_EVENT_TABLE()

// Konstruktor:
// Initialisieren der Variablen
Wkn::Wkn() : wxEvtHandler()
{
	vari[CYCLE] = 1;
	vari[MODUS] = 0;
	vari[APPEND] = 1;
	vari[NONSTANDARD] = 0;
	vari[USETAGS] = 0;
	vari[FLEXIBLE] = 0;
	vari[USESEPEND] = 0;

	profilIndex = 0;
	wknProps = NULL;
	logAusgabe = NULL;
	wkMain = NULL;

	varkz = '$';
	
// #ifdef _WINDOWS_
// 
// #else
// 
// #endif
}


// Destruktor:
Wkn::~Wkn()
{
}


// void DoIt:
// Tool Start; Es werden die Einstellungen überprüft und an den Ersetzer übergeben
void Wkn::doIt()
{

	schreibeLog(WkLog::WkLog_ABSATZ, "1601: ConfigMaker START\r\n", "1601", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
	
	bool fehler = doSave("");

	if (!fehler)
	{
		derErsetzer = new Ersetzer(vars[LOGDATEI], logAusgabe, this);

		schreibeLog(WkLog::WkLog_ZEIT, "1602: Starting Duplication\r\n", "1602", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		
		derErsetzer->wknEinstellungen(vars[SEPERATOR], vari[MODUS], vari[CYCLE], vari[FLEXIBLE], vari[USETAGS]);
		derErsetzer->ersetzeInit(vars[WERTEDATEI], vars[VORLAGE], vars[DATVAR], vars[AVZ], vari[APPEND], vari[USESEPEND]);

		thrd = boost::thread(boost::bind(&Wkn::startErsetzer,this));
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1402: Cannot Start because of wrong or missing entries!\r\n", "1402", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		
		wxCommandEvent event(wkEVT_ERSTELLER_FERTIG);
		event.SetInt(2);
		wxPostEvent(this, event);
	}

}


// void startErsetzer
// Einstiegsfunktion in den Thread
void Wkn::startErsetzer()
{
	if (derErsetzer->ersetze())
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1401: Duplication stopped with errors\r\n", "1401", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1603: Duplication finished\r\n", "1603", 
			WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	}

	wxCommandEvent event(wkEVT_ERSTELLER_FERTIG);
	event.SetInt(1);
	wxPostEvent(this, event);

}


// bool doLoad:
// zum Laden eines Profils. Die Einstellungen werden überprüft, bevor sie geladen werden
// return Werte: true: erfolgreich geladen
//               false: Fehler beim Laden der Werte
bool Wkn::doLoad(std::string profilname)
{
	schreibeLog(WkLog::WkLog_ABSATZ, "1604: ConfigMaker: Loading Profile\r\n", "1604", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (xmlInit(profilname))
	{
		einstellungenInit();
		schreibeLog(WkLog::WkLog_ZEIT, "1605: ConfigMaker: Profile \"" + profilname + "\" successfully loaded\r\n", "1605", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
		return true;
	}
	schreibeLog(WkLog::WkLog_ZEIT, "1301: ConfigMaker: Unable to load Profile \"" + profilname + "\"\r\n", "1301", 
		WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	return false;
}


// bool doSave:
// zum Sichern der Einstellungen in ein Profil
// return Werte: false: erfolgreich gesichert
//               true: Fehler beim Sichern
bool Wkn::doSave(std::string profilname)
{
	bool fehler = false;
	
	schreibeLog(WkLog::WkLog_ABSATZ, "1606: ConfigMaker: Saving Profile\r\n", "1606", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	if (guiStatus)
	{
		fehler = doTest(profilname);
	}
	else
	{
		fehler = doTestGui(profilname);
	}
	
	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1607: ConfigMaker: Profile \"" + profilname + "\" successfully saved\r\n", "1607", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1302: ConfigMaker: Unable to save Profile \"" + profilname + "\"\r\n", "1302", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// bool doRemove:
// zum Löschen eines Profils
// return Werte: false: erfolgreich gelöscht
//               true: Fehler beim Löschen
bool Wkn::doRemove(std::string profilName)
{
	bool fehler = false;

	schreibeLog(WkLog::WkLog_ABSATZ, "1614: ConfigMaker: Removing Profile\r\n", "1614", 
		WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);

	if (profilName == "" || profilName == "DEFAULT")
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1306: ConfigMaker: Unable to remove Profile \"" + profilName + "\" - Default Profile cannot be deleted\r\n", "1306", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
		return fehler;
	}

	fehler = xmlInit(profilName, 2);

	if (!fehler)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1613: ConfigMaker: Profile \"" + profilName + "\" successfully removed\r\n", "1613", 
			WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1306: ConfigMaker: Unable to remove Profile \"" + profilName + "\"\r\n", "1306", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	return fehler;
}


// int testDataFile:
// zum Testen, ob das Datenfile konsistent ist
// return Werte: siehe "enum testRet"
int Wkn::testDataFile(std::string datfile)
{
	std::string dfile;
	std::string fehlerWDat = "";
	std::ifstream eingabe(datfile.c_str(), std::ios_base::in);
	getline(eingabe, dfile, '\0');
	eingabe.close();

	// Test 1: Ist überhaupt das angegebene Trennzeichen vorhanden
	size_t letzterSP = dfile.rfind(vars[SEPERATOR]);
	if (letzterSP == dfile.npos)
	{
		fehlerWDat = "\r\n1201: DATA FILE inconsistent! -> Specified Seperator not found\r\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1201", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 1;
	}

	// Test 2: Anzahl der Seps modulo Zeilen muss 0 sein

	// Zählen der Seps
	long sepAnzahl = 0;
	for (size_t i = 0; i < dfile.size(); i++)
	{
		if (dfile[i] == vars[SEPERATOR])
		{
			sepAnzahl++;
		}
	}


	// Wenn kein ENTER am Schluss, dann wird eines hinzugefügt.
	dfile.erase(letzterSP+1, dfile.npos);
	letzterSP = dfile.find("\n", letzterSP);
	if (letzterSP == dfile.npos)
	{
		dfile += "\n";
		letzterSP = dfile.rfind(";") + 1;
	}


	int zeilenAnzahl = 0;
	for (size_t i = 0; i < dfile.size(); i++)
	{
		if (dfile[i] == '\n')
		{
			zeilenAnzahl++;
		}
	}

	if ((sepAnzahl+zeilenAnzahl)%zeilenAnzahl)
	{
		fehlerWDat = "\r\n1201: DATA FILE inconsistent! -> Not all rows have the same number of columns\r\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1201", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 2;
	}

	// Test 3: Es dürfen keine CR SPACE /,.;:_-\|()[]{}<>  in der ersten Zeile vorkommen
	size_t pos1 = dfile.find("\n", 0);
	std::string ersteZeile = dfile.substr(0, pos1);

	// Laut Doku verbotene Zeichen für die Variablen. Das zusätzliche Trennzeichen muss gelöscht werden
	std::string verboteneZeichen = "\r\n /,.;:_-\\|()[]{}<>";
	size_t pos2 = verboteneZeichen.find(vars[SEPERATOR]);
	verboteneZeichen.erase(pos2, 1);

	size_t vzPos = ersteZeile.find_first_of(verboteneZeichen);
	if (vzPos != ersteZeile.npos)
	{
		// Test, ob Windows File unter Linux verwendet wird
#ifdef _WINDOWS_
		fehlerWDat = "\r\n1201: DATA FILE inconsistent! -> Forbidden characters used for placeholders\r\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1201", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 3;
#else
		if (ersteZeile[vzPos] != '\r')
		{
			fehlerWDat = "\r\n1201: DATA FILE inconsistent! -> Forbidden characters used for placeholders\r\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1201", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			return 3;
		}
#endif
	}

	// Test 4: Das Variablenkennzeichen muss genausooft in der ersten Zeile vorkommen, wie es Spalten gibt
	// Dieser Test wird umgangen, wenn Tags verwendet werden
	if (!vari[USETAGS])
	{
		int spaltenAnzahl = (sepAnzahl+zeilenAnzahl)/zeilenAnzahl;
		int vkzAnzahl = 0;
		for (size_t i = 0; i < ersteZeile.size(); i++)
		{
			if (ersteZeile[i] == ersteZeile[0])
			{
				vkzAnzahl++;
			}
		}

		if (spaltenAnzahl != vkzAnzahl)
		{
			fehlerWDat = "\r\n1201: DATA FILE inconsistent! -> Not all placeholders have the same beginning character\r\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1201", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			return 4;
		}
	}

	return 0;
}


void Wkn::testDataFileAdvanced(std::string datfile)
{
	datAusgabe->createList();
	bool letzteZeileLoeschen = false;

	std::string verboteneZeichen = "\r\n /,.;:_-\\|()[]{}<>";
	size_t pos2 = verboteneZeichen.find(vars[SEPERATOR]);
	verboteneZeichen.erase(pos2, 1);

	std::ifstream eingabe(datfile.c_str(), std::ios_base::in);

	std::string seps = vars[SEPERATOR] + "\r\n";

	typedef boost::tokenizer<boost::char_separator<char> > 
		tokenizer;
	boost::char_separator<char> sep(seps.c_str(), "", boost::keep_empty_tokens);

	int spaltenZahlErsteZeile = 0;		// Spaltenanzahl vom ersten Durchlauf
	int i = 0;		// Zeilenanzahl
	int j = 0;		// Spaltenanzahl

	for (i = 0; eingabe.good(); i++)
	{
		// Zeile hinzufügen, wenn zu wenig vorhanden
		if (datAusgabe->tabellenAnsicht->GetNumberRows() == i)
		{
			datAusgabe->zeileHinzu();
		}
		
		std::string dfile;
		getline(eingabe, dfile, '\n');

		tokenizer tokens(dfile, sep);
		j = 0;		// Spaltenanzahl zurücksetzen
		for (tokenizer::iterator tok_iter = tokens.begin();	tok_iter != tokens.end(); ++tok_iter)
		{
			// Spalte hinzufügen
			if (datAusgabe->tabellenAnsicht->GetNumberCols() <= j)
			{
				datAusgabe->spalteHinzu();
			}
			std::string cellVal = *tok_iter;
			datAusgabe->tabellenAnsicht->SetCellValue(i, j, cellVal);

			// Spezielle Fehler in erster Zeile
			if (!i)
			{
				// Fehler anzeigen, wenn varkz nicht überall gleich
				// Dieser Test wird umgangen, wenn Tags verwendet werden
				if (!vari[USETAGS])
				{
					if (cellVal[0] != dfile[0])
					{
						wxColour hintergrund("#ff0000");
						datAusgabe->tabellenAnsicht->SetCellBackgroundColour(hintergrund, i, j);
						// datAusgabe->tabellenAnsicht->SetRowLabelValue(i, "!!!!!");
					}
				}
				
				// Fehler anzeigen, wenn verbotene Zeichen in der ersten Zeile
				size_t vzPos = cellVal.find_first_of(verboteneZeichen);
				if (vzPos != cellVal.npos)
				{
					wxColour hintergrund("#ff0000");
					datAusgabe->tabellenAnsicht->SetCellBackgroundColour(hintergrund, i, j);
					// datAusgabe->tabellenAnsicht->SetRowLabelValue(i, "!!!!!");
				}
			}

			j++;
		}
		// Fehler anzeigen, wenn der Seperator in der Zeile nicht vorkommt
		if (j == 1)
		{
			for (int k = 0; k < datAusgabe->tabellenAnsicht->GetNumberCols(); k++)
			{
				wxColour hintergrund("#ff0000");
				datAusgabe->tabellenAnsicht->SetCellBackgroundColour(hintergrund, i, k);
				// datAusgabe->tabellenAnsicht->SetRowLabelValue(i, "!!!!!");
			}
		}

		// Fehler anzeigen, wenn nicht gleich viele Spalten in jeder Zeile
		if (!i)
		{
			spaltenZahlErsteZeile = j;
		}
		else if (spaltenZahlErsteZeile != j)
		{
			for (int k = 0; k < datAusgabe->tabellenAnsicht->GetNumberCols(); k++)
			{
				wxColour hintergrund("#ff0000");
				datAusgabe->tabellenAnsicht->SetCellBackgroundColour(hintergrund, i, k);
				// datAusgabe->tabellenAnsicht->SetRowLabelValue(i, "!!!!!");
			}
		}
	}
	eingabe.close();

	// Damit am Schluss nicht eine rote Zeile angezeigt wird, wird diese gelöscht:
	if (j < spaltenZahlErsteZeile && j < 2)
	{
		datAusgabe->tabellenAnsicht->DeleteRows(datAusgabe->tabellenAnsicht->GetNumberRows()-1);
	}


}


// bool testNonStandardDataFile:
// zum Testen von non-standard Datenfiles; Wenn es nicht konform ist, dann wird es modifiziert und abgespeichert
// return Werte: true: wurde verändert
//               false: wurde nicht verändert
bool Wkn::testNonStandardDataFile(std::string datfile)
{
	std::string ez = vars[ERSATZZEICHEN];	// Ersatzzeichen
	bool lfonly = false;		// Zeigt an, ob das File neu geschrieben werden muss
	if (vari[FLEXIBLE])
	{
		ez = "\xFF";
	}

	std::string dfile;
	std::string dfileOld;
	std::ifstream eingabe(datfile.c_str(), std::ios_base::in|std::ios_base::binary);
	getline(eingabe, dfile, '\0');
	dfileOld = dfile;
	eingabe.close();

	// Überprüfen, ob das Variablenfile mit einer Variable beginnt. 
	// Wenn nicht, dann alles bis zur ersten Variable löschen
	size_t pos1 = 0;
	if ((pos1 = dfile.find(vars[DATVAR][0])) != 0)
	{
		dfile.erase(0, pos1);
		lfonly = true;
	}



	// Ungültige Sonderzeichen löschen
	// Schauen, ob der Zeilenumbruch aus \r\n oder nur \n besteht
	if (dfile.find("\r\n") != dfile.npos)
	{
		size_t lfPos = 0;
		while (1)
		{
			lfPos = dfile.find("\n", lfPos+1);
			if (lfPos == dfile.npos)
			{
				break;
			}
			else if (dfile[lfPos-1] != '\r')
			{
				lfonly = true;
				dfile.erase(lfPos, 1);
				dfile.insert(lfPos, ez);
			}
			else
			{
#ifndef _WINDOWS_
				dfile.erase(lfPos-1, 1);
#endif
			}
		}
	}
	else
	{
		schreibeLog(WkLog::WkLog_ZEIT, "1501: ConfigMaker: Line breaks within cells not supported under Linux\r\n", "1501", 
			WkLog::WkLog_ROT, WkLog::WkLog_NORMALFORMAT);	
	}
	if (lfonly)
	{
		std::ofstream ausgabe(datfile.c_str(), std::ios_base::out|std::ios_base::binary);
		ausgabe << dfile;
		ausgabe.close();
		std::string datfileOld = datfile + ".old";
		std::ofstream ausgabe1(datfileOld.c_str(), std::ios_base::out|std::ios_base::binary);
		ausgabe1 << dfileOld;
		ausgabe1.close();

		return TRUE;
	}


	return FALSE;
}


// void sets:
// Zum Setzen wichtiger Grundeinstellungen
void Wkn::sets(TiXmlDocument *document, bool guiStats, WkLogFile *lf)
{
	doc = document;
	
	// GUI STATUS:
	// true: keine Gui; false: GUI
	guiStatus = guiStats;

	logfile = lf;
}


// bool einstellungenInit()
// Zum Laden der Werte, nachdem die richtige Position des Profils in wktools.xml ausgemacht wurde.
bool Wkn::einstellungenInit()
{
	TiXmlElement *stringSets = pElemProf->FirstChildElement("StringSettings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("INTSettings");

	std::string atts[] = {"df", "config", "output", "fv", "sep", "lf", "log"};
	std::string atti[] = {"format", "flex", "tags", "cycle", "useSep", "modus", "append"};

	int anzahls = 7;
	int anzahli = 7;

	if (stringSets)
	{
		for (int i = 0; i < anzahls; i++)
		{
			if (stringSets->Attribute(atts[i]))
			{
				vars[i] = *(stringSets->Attribute(atts[i]));
			}
			else
			{
				vars[i] = "";
			}
			if (!guiStatus)
			{
				// GUI Mode
				wknProps->einstellungenInit(i, vars[i]);
			}
		}
	}

	if (intSets)
	{
		for (int i = 0; i < anzahli; i++)
		{
			int ret = intSets->QueryIntAttribute(atti[i], &vari[i]);
			if (ret != TIXML_SUCCESS)
			{
				vari[i] = 0;
			}
			if (!guiStatus)
			{
				// GUI Mode
				wknProps->einstellungenInit(i, vari[i]);
			}
		}
	}

	if (logfile != NULL)
	{
		logfile->logFileSets(vars[CWknPropGrid::WKN_PROPID_STR_LOGFILE]);
	}

	//Einstellungen im GUI Modus überprüfen, damit dann auch das Datenfile ausgegeben werden kann
	if (!guiStatus)
	{
		doTestDataFileOnly();
	}

	return true;
}


void Wkn::guiSets(CWknPropGrid *wknProp, WkLog *ausgabe, TabellePanel *table)
{
	wknProps = wknProp;
	logAusgabe = ausgabe;
	datAusgabe = table;
}


wxArrayString Wkn::ladeProfile()
{
	wxArrayString profile;
	
	// Grund Elemente für WKN
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWkn;		// das Element, das auf WKN zeigt

	// WKN im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		profile.Empty();
	}
	else
	{
		pElemWkn = pElem->FirstChildElement("wkn");
		pElemProf = pElemWkn->FirstChildElement("Profile");

		for (profilIndex = 0; pElemProf; profilIndex++)
		{
			std::string profname = pElemProf->Attribute("name");
			profile.Add(profname.c_str());

			pElemProf = pElemProf->NextSiblingElement();
		}
	}

	return profile;
}


void Wkn::cancelIt()
{
	derErsetzer->cancel = true;
	
	wxCommandEvent event(wkEVT_ERSTELLER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);

}


void Wkn::kilIt()
{
	thrd.interrupt();
	schreibeLog(WkLog::WkLog_ZEIT, "1608: ConfigMaker killed!\r\n", "1608", 
		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	
	wxCommandEvent event(wkEVT_ERSTELLER_FERTIG);
	event.SetInt(2);
	wxPostEvent(this, event);

}

// xmlInit
//********
// Speichern: 0 - laden; 1 - speichern; 2 - löschen
bool Wkn::xmlInit(std::string profilName, int speichern)
{
	if (profilName == "")
	{
		profilName = "DEFAULT";
	}
	
	// Grund Elemente für WKN
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemWkn;		// das Element, das auf WKN zeigt

	// WKN im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		return false;
	}

	pElemWkn = pElem->FirstChildElement("wkn");
	pElemProf = pElemWkn->FirstChildElement("Profile");

	for (profilIndex = 0; pElemProf; profilIndex++)
	{
		std::string profname = pElemProf->Attribute("name");
		if (profname == profilName)
		{
			if (speichern == 2)
			{
				bool rmret = pElemWkn->RemoveChild(pElemProf);
				return !rmret;
			}

			return true;
		}

		pElemProf = pElemProf->NextSiblingElement();
	}
	
	if (speichern == 1)
	{
		pElemProf = new TiXmlElement("Profile");		
		pElemWkn->LinkEndChild(pElemProf);
		pElemProf->SetAttribute("name", profilName);

		TiXmlElement *stringSets = new TiXmlElement("StringSettings");
		TiXmlElement *intSets = new TiXmlElement("INTSettings");

		pElemProf->LinkEndChild(stringSets);
		pElemProf->LinkEndChild(intSets);

		std::string attString[] = {"df", "lf", "config", "log", "output", "fv", "sep"};
		std::string attInt[] = {"format", "flex", "tags", "cycle", "useSep", "modus", "append"};
		int a1 = 7;
		int a2 = 7;
		
		for (int i = 0; i < a1; i++)
		{
			stringSets->SetAttribute(attString[i].c_str(), "");
		}

		for (int i = 0; i < a2; i++)
		{
			intSets->SetAttribute(attInt[i].c_str(), 0);
		}
		return true;
	}

	// TODO: FEHLER
	profilIndex = 65535;
	return false;
}


bool Wkn::doTest(std::string profilname)
{
	bool fehler = false;

	// Test 5 vorgeszogen: Seperator:
	std::string fehlerSep = "\r\n1303: Missing entry for ADDITIONAL SEPERATOR\r\n";

	if (vars[SEPERATOR] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSep, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 1: Wertedatei:
	std::string fehlerWDat = "\r\n1303: Wrong or missing entry for DATA FILE\r\n";

	std::ifstream wdat(vars[WERTEDATEI].c_str());
	if (!wdat.is_open())
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		wdat.close();
		if (vari[NONSTANDARD])
		{
			testNonStandardDataFile(vars[WERTEDATEI]);
		}
		fehler = testDataFile(vars[WERTEDATEI]);
	}

	// Test 2: Vorlagendatei:
	std::string fehlerVDat = "\r\n1303: Wrong or missing entry for CONFIG FILE\r\n";

	std::ifstream vdat(vars[VORLAGE].c_str());
	if (!vdat.is_open())
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerVDat, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		vdat.close();
	}


	// Test 4: dateinamenVariable:
	std::string fehlerDatVar = "\r\n1303: Missing entry for FILENAME VARIABLE\r\n";

	if (vars[DATVAR] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDatVar, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	// Test 6: Modus:
	// TODO: Modus schreiben

	switch (vari[MODUS])
	{
	case 0:
		vari[MODUS] = Ersetzer::PZED;
		break;
	case 1:
		vari[MODUS] = Ersetzer::EINEDATEI;
		break;
	default:
		break;
	}

	// Test 7: Ausgabeverzeichnis:
	std::string fehleravz = "\r\n1303: Wrong or missing entry for OUTPUT DIRECTORY\r\n";
	wxFileName fn;

	//if (vars[AVZ].Find("\\", vars[AVZ].GetLength()) != -1)
	//{
	//	vars[AVZ].Delete(vars[AVZ].GetLength()-1);
	//}

	if (!fn.DirExists(vars[AVZ]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehleravz, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
#ifdef _WINDOWS_
		vars[AVZ] += "\\";
#else
		vars[AVZ] += "/";
#endif
	}

	// Test 3: Logdatei:
	if (vars[LOGDATEI] == "")
	{
		vars[LOGDATEI] = vars[AVZ] + "log.txt";
	}


	// Cycle sollte ungleich 0 sein
	std::string fehlerck = "\r\n1304: CYCLE COUNT must not be 0!\r\n";

	if (!vari[CYCLE])
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerck, "1304", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}

	return fehler;
}


bool Wkn::doTestGui(std::string profilname)
{
	bool fehler = false;

	if  (!(xmlInit(profilname, 1)))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n1101: Init Error -> could not read settings\r\n", "1101", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return true;
	}
	TiXmlElement *stringSets = pElemProf->FirstChildElement("StringSettings");
	TiXmlElement *intSets = pElemProf->FirstChildElement("INTSettings");
	std::string atts[] = {"df", "config", "output", "fv", "sep", "lf", "log"};
	std::string atti[] = {"format", "flex", "tags", "cycle", "useSep", "modus", "append"};


	// Test, ob alle Eingaben gültig sind

	// Test 5 vorgeszogen: Seperator:
	vars[SEPERATOR] = wknProps->leseStringSetting(SEPERATOR);
	std::string fehlerSep = "\r\n1303: Missing entry for ADDITIONAL SEPERATOR\r\n";

	if (vars[SEPERATOR] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerSep, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(atts[SEPERATOR], vars[SEPERATOR]);
	}

	// Test 1: Wertedatei:
	std::string fehlerWDat = "\r\n1303: Wrong or missing entry for DATA FILE\r\n";

	vars[WERTEDATEI] = wknProps->leseStringSetting(WERTEDATEI);
	std::ifstream wdat(vars[WERTEDATEI].c_str());
	if (!wdat.is_open())
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		wdat.close();
		if (vari[NONSTANDARD])
		{
			testNonStandardDataFile(vars[WERTEDATEI]);
		}
		fehler = testDataFile(vars[WERTEDATEI]);
		stringSets->SetAttribute(atts[WERTEDATEI], vars[WERTEDATEI]);
	}

	// Test 2: Vorlagendatei:
	std::string fehlerVDat = "\r\n1303: Wrong or missing entry for CONFIG FILE\r\n";
	vars[VORLAGE] = wknProps->leseStringSetting(VORLAGE);

	std::ifstream vdat(vars[VORLAGE].c_str());
	if (!vdat.is_open())
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerVDat, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		vdat.close();
		stringSets->SetAttribute(atts[VORLAGE], vars[VORLAGE]);
	}


	// Test 4: dateinamenVariable:
	std::string fehlerDatVar = "\r\n1303: Missing entry for FILENAME VARIABLE\r\n";
	vars[DATVAR] = wknProps->leseStringSetting(DATVAR);

	if (vars[DATVAR] == "")
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerDatVar, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(atts[DATVAR], vars[DATVAR]);
	}

	// Test 6: Modus:
	vari[MODUS] = wknProps->leseIntSetting(MODUS);
	intSets->SetAttribute(atti[MODUS], vari[MODUS]);

	switch (vari[MODUS])
	{
	case 0:
		vari[MODUS] = Ersetzer::PZED;
		break;
	case 1:
		vari[MODUS] = Ersetzer::EINEDATEI;
		break;
	default:
		break;
	}

	// Test 7: Ausgabeverzeichnis:
	std::string fehleravz = "\r\n1303: Wrong or missing entry for OUTPUT DIRECTORY\r\n";
	wxFileName fn;

	//if (vars[AVZ].Find("\\", vars[AVZ].GetLength()) != -1)
	//{
	//	vars[AVZ].Delete(vars[AVZ].GetLength()-1);
	//}

	vars[AVZ] = wknProps->leseStringSetting(AVZ);
	if (!fn.DirExists(vars[AVZ]))
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehleravz, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		stringSets->SetAttribute(atts[AVZ], vars[AVZ]);
#ifdef _WINDOWS_
		vars[AVZ] += "\\";
#else
		vars[AVZ] += "/";
#endif
	}

	// Test 3: Logdatei:
	vars[LOGDATEI] = wknProps->leseStringSetting(LOGDATEI);
	if (vars[LOGDATEI] == "")
	{
		vars[LOGDATEI] = vars[AVZ] + "log.txt";
		stringSets->SetAttribute(atts[LOGDATEI], vars[LOGDATEI]);
		wknProps->einstellungenInit(LOGDATEI, vars[LOGDATEI]);
	}
	else
	{
		stringSets->SetAttribute(atts[LOGDATEI], vars[LOGDATEI]);
	}

	vari[APPEND] = wknProps->leseIntSetting(APPEND);
	intSets->SetAttribute(atti[APPEND], vari[APPEND]);
	vari[USESEPEND] = wknProps->leseIntSetting(USESEPEND);
	intSets->SetAttribute(atti[USESEPEND], vari[USESEPEND]);
	vari[NONSTANDARD] = wknProps->leseIntSetting(NONSTANDARD);
	intSets->SetAttribute(atti[NONSTANDARD], vari[NONSTANDARD]);
	vari[USETAGS] = wknProps->leseIntSetting(USETAGS);
	intSets->SetAttribute(atti[USETAGS], vari[USETAGS]);
	vari[FLEXIBLE] = wknProps->leseIntSetting(FLEXIBLE);
	intSets->SetAttribute(atti[FLEXIBLE], vari[FLEXIBLE]);
	vars[ERSATZZEICHEN] = wknProps->leseStringSetting(ERSATZZEICHEN);
	stringSets->SetAttribute(atts[ERSATZZEICHEN], vars[ERSATZZEICHEN]);

	// Cycle sollte ungleich 0 sein
	std::string fehlerck = "\r\n1304: CYCLE COUNT must not be 0!\r\n";
	vari[CYCLE] = wknProps->leseIntSetting(CYCLE);

	if (!vari[CYCLE])
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerck, "1304", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		intSets->SetAttribute(atti[CYCLE], vari[CYCLE]);
	}

	return fehler;
}


void Wkn::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
	WkLog::evtData evtDat;
	evtDat.farbe = farbe;
	evtDat.format = format;
	evtDat.groesse = groesse;
	evtDat.logEintrag = logEintrag;
	evtDat.logEintrag2 = log2;
	evtDat.type = type;
	evtDat.report = 0;

	LogEvent evt(EVT_LOG_MELDUNG);
	evt.SetData(evtDat);

	if (logAusgabe != NULL)
	{
		wxPostEvent(logAusgabe, evt);
	}

	if (logfile != NULL)
	{
		wxPostEvent(logfile, evt);
	}
}


void Wkn::setMainFrame(Cwktools4Frame *fp)
{
	wkMain = fp;
}


void Wkn::OnErstellerFertig(wxCommandEvent &event)
{
	schreibeLog(WkLog::WkLog_ZEIT, "1609: ConfigMaker END\r\n", "1609", 
		WkLog::WkLog_WKTOOLS, WkLog::WkLog_FETT);
	
	if (wkMain != NULL)
	{
		wxCommandEvent event(wkEVT_WKN_FERTIG);
		event.SetInt(1);
		wxPostEvent(wkMain, event);
	}
	else
	{
		// Programm beenden, wenn kein GUI vorhanden
		wxMessageBox("BLA!");
		wxGetApp().ExitMainLoop();
	}
}


bool Wkn::doTestDataFileOnly()
{
	bool fehler = false;
	std::string fehlerWDat = "\r\n1303: Wrong or missing entry for DATA FILE\r\n";
	vars[WERTEDATEI] = wknProps->leseStringSetting(WERTEDATEI);
	std::ifstream wdat(vars[WERTEDATEI].c_str());
	if (!wdat.is_open())
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, fehlerWDat, "1303", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		fehler = true;
	}
	else
	{
		wdat.close();
		if (vari[NONSTANDARD])
		{
			testNonStandardDataFile(vars[WERTEDATEI]);
		}
		fehler = testDataFile(vars[WERTEDATEI]);
		testDataFileAdvanced(vars[WERTEDATEI]);
	}

	return fehler;
}