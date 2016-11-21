#include <iostream>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>

#include "stdwx.h"
#include "class_ersetzer.h"


// Konstruktor:
//*************
// Log File muss angegeben werden
Ersetzer::Ersetzer(std::string logFile, WkLog *logA, Wkn *wkE)
{
	mods = 0;
	logAusgabe = logA;
	flexible = FALSE;
	cycle = 1;
	tagEnde = 0;
	tagStart = 0;
	wkErsetzer = wkE;
	cancel = false;
//	logAusgabeDatei.rdbuf()->open(logFile.c_str(), std::ios_base::out | std::ios_base::app);
	
}


// Destruktor:
//************
Ersetzer::~Ersetzer()
{
}


// schreibeLogfile:
//*****************
// Zum Beschreiben des Log Files
void Ersetzer::schreibeLogfile(std::string logInput)
{
	//logAusgabeDatei << logInput;
}


// ersetze:
//*********
// zum Starten des Vervielfältigers
uint Ersetzer::ersetze()
{
	cancel = false;

	std::string dbgA;
	// Die Wertedatei in den std::string werte einlesen
	std::string werte;
	std::ifstream wdat(werteDatei.c_str(), std::ios_base::in);
	getline(wdat, werte, '\0');
	wdat.close();

	// Die Vorlagen in den std::string vorlage einlesen
	std::string vorlage;
	std::ifstream vdat(vorlageDatei.c_str(), std::ios_base::in);
	getline(vdat, vorlage, '\0');
	vdat.close();

	// Die Variablen und den Variablenkennzeichner aus der Wertedatei herausfiltern
	dbgA = "\r\n1701: Found Variables:\r\n";
	
	schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "1701", WkLog::WkLog_BLAU);

	size_t crlfPos = werte.find_first_of("\n");
#ifndef _WINDOWS_
	if (werte[crlfPos]-1 == '\r')
	{
		werte.erase(werte[crlfPos]-1, 1);
	}
#endif
	std::string vars = werte.substr(0, crlfPos);
	werte.erase(0, crlfPos+1);
	char varkz = vars[0];
	std::string doppelVarkz = vars.substr(0,1) + vars.substr(0,1);

	// Die Variablen in einen Vector einlesen, damit in weiterer Folge beim 
	// Einlesen der Werte sehr schnell darauf zugegriffen werden kann
	std::vector <std::string> variablen;
	size_t pos1 = 2;
	size_t pos2 = 0;
	
	std::string abschluss = seps;			// nur relevant, wenn useSepEnd true
	
	// Seperatoren festlegen:
	// Default \n\0 plus GUI Eingabe
	seps += "\n\0";
	// Bei Verwendung von flexiblen Datenfiles wurden die Zeileinumbrüche in den Zellen durch 0xFF ersetzt
	// Daher wird 0xFF als Seperator hinzugefügt
	if (flexible)
	{
		seps+="\xFF";
	}
	
	while (pos2 != vars.npos)
	{
		pos2 = vars.find(varkz, pos1);
		std::string ph = vars.substr(pos1-2, pos2-pos1+1);
		if (useSepEnd)
		{
			ph += abschluss;
		}
		variablen.push_back(ph);
		pos1 = pos2+2;

		dbgA = ph + "\n";

		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, ""); 
	}

	// Überprüfen, ob die Dateinamenvariable überhautp bei den Variablen vorkommt.
	// Schritt 1: Überprüfen, ob die Dateinamenvariable erweitert ist

	std::string fnZusatz = "";
	if (useSepEnd)
	{
		size_t spPos = dateiVar.find(";");
		spPos++;
		if (spPos < dateiVar.length())
		{
			fnZusatz = dateiVar.substr(spPos, dateiVar.length() - spPos);
			dateiVar.erase(spPos);
		}
	}

	bool datVarTest = false;
	std::ios_base::openmode ausgabeArt = std::ios_base::out | std::ios_base::app | std::ios_base::binary;
	if (modus != EINEDATEI)
	{
		if (!append)
		{
			ausgabeArt = std::ios_base::out | std::ios_base::binary;
		}
		for(size_t i = 0; i < variablen.size(); i++)
		{
			if (variablen[i] == dateiVar)
			{
				datVarTest = true;
				break;
			}
		}
	}
	else
	{
		datVarTest = true;
	}

	if (!datVarTest)
	{
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\r\n1305: Filename Variable not found in Variables!\r\n", "1305", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 1;
	}
		
	
	// Die Werte in eine Multimap einlesen, die die Variablen als Keys hat

	// Alle benötigten Elemente definieren...
	std::multimap <std::string, std::string> werteM;
	std::multimap <std::string, std::string>::iterator werteIter, werteAnfang, werteEnde;
	typedef std::pair <std::string, std::string> pss;
	pos1 = 0;
	pos2 = 0;
	// ...und dann die Werte einlesen...	
	dbgA = "1702: Reading data lines.\r\n";
	schreibeLog(WkLog::WkLog_ABSATZ, dbgA, "1702", WkLog::WkLog_BLAU);

	if (werte[werte.length()] != '\n')
	{
		werte += "\n";
	}

	while(pos2 < werte.npos)
	{
		bool dummy = false;		// Wenn Subvariablen eingefügt wurden, dann wird am Ende eine Dummy Variable benötigt.
		for (size_t i = 0; i < variablen.size(); i++)
		{
			pos2 = werte.find_first_of(seps, pos1);
			
			if (pos2 == werte.npos)
			{
				break;
			}
			
			std::string ph = "";
			if (pos2-pos1 != 0)
			{
				ph = werte.substr(pos1, pos2-pos1);
				if (ph[0] == '\"')
				{
					ph.erase(0, 1);
				}
				size_t pos3 = ph.length()-1;
				if (ph[pos3] == '\"')
				{
					ph.erase(pos3, 1);
				}

				if (werte[pos2] == '\xFF')
				{
					ph = "\xFF" + variablen[i] + "\xFF" + ph;	
					dummy = true;
					werteM.insert(pss(variablen[i], ph));
					i--;
				}
				else if (dummy)
				{
					ph = "\xFF" + variablen[i] + "\xFF" + ph;
					werteM.insert(pss(variablen[i], ph));
					werteM.insert(pss(variablen[i], "DummyVariable"));
					dummy = false;
				}
				else
				{
					werteM.insert(pss(variablen[i], ph));
				}
			}
			else
			{
				werteM.insert(pss(variablen[i], ph));
			}

			pos1 = pos2+1;
		}
	}

	// Statusanzeige initialiseren
	
	size_t anzahl = werteM.size()/variablen.size();
	std::string vorSeps = "\n \t/,.;:_-\\|(){}[]\"<>";
	if (useSepEnd)
	{
		vorSeps = abschluss;
	}

	// Starten der Vervielfältigung:
	// Es wird so lange die Vorlage kopiert, bis die Werte MultiMap leer ist.
	dbgA = "\r\n1610: Starting Duplication.\r\n";
	schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "1610", WkLog::WkLog_GRUEN);


	bool fehler = false;
	while (!werteM.empty() || !fehler)
	{
		if (cancel)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "1611: ConfigMaker cancelled!\r\n", "1611", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			break;
		}
		dbgA = "\n1703: New instance with the following data:\n";
		for (uint i = 0; i < variablen.size(); i++)
		{
			dbgA += variablen[i];
			dbgA += ": ";
			werteIter = werteM.find(variablen[i]);
			if (werteIter == werteM.end())
			{
				dbgA += "\tNo more data found!\n\nEND\n\n";
				fehler = true;
				break;
			}
			bool standard = true;
			size_t pos = 0;
			std::string wert = werteIter->second;
			while ((pos = wert.find(variablen[i]+"\xFF")) != wert.npos)
			{
				pos = wert.find_last_of("\xFF") + 1;
				wert = wert.substr(pos);
				dbgA += wert;
				dbgA += "\t";
				werteIter++;
				if (werteIter == werteM.end())
				{
					dbgA += "ERROR - no more data found!\n\n";
					fehler = true;
					break;
				}
				else
				{
					wert = werteIter->second;
				}
				standard = false;
			}

			if (standard)
			{
				dbgA += werteIter->second;
			}

			dbgA += "\n";
		}


		schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "");

		if (fehler)
		{
			break;
		}

		int ck = cycle;
		std::string tc = vorlage;
		// So lange in einer Vorlage ersetzen, bis die Anzahl der angegebenen Durchläufe erreicht ist.
		while (ck)
		{
			
			pos1 = 0;
			pos2 = 0;
			tagStart = 0;
			tagEnde = 0;
			
			// Nach Variablen in der Vorlage suchen, bis das Dateiende erreicht ist
			while(pos1 < tc.npos)
			{
				pos2 = tc.find(varkz, pos1);
				
				if (pos2 == tc.npos)
				{
					break;
				}
				
				pos1 = tc.find_first_of(vorSeps, pos2);
				std::string ph = "";
				if (!useSepEnd)
				{
					ph = tc.substr(pos2, pos1-pos2);
				}
				else
				{
					ph = tc.substr(pos2, pos1+1-pos2);				
				}
				
				
				werteIter = werteM.find(ph);
				if (werteIter != werteM.end())
				{
					bool notag = true;
					std::string wert = "";
					if (tags)
					{
						if (ph[1] == varkz)
						{
							std::string bla = "";
							// ENTER
							bla = doppelVarkz + "n";
							if (ph.find(bla) != ph.npos)
							{
								wert = "\n";
							}
							// Tabulator
							bla = doppelVarkz + "t";
							if (ph.find(bla) != ph.npos)
							{
								wert = "\t";
							}
							// Space
							bla = doppelVarkz + "s";
							if (ph.find(bla) != ph.npos)
							{
								wert = " ";
							}
							// $$b
							bla = doppelVarkz + "b";
							if (ph.find(bla) != ph.npos)
							{
								wert = "";
								tagStart = pos2;
							}
							// $$e
							bla = doppelVarkz + "e";
							if (ph.find(bla) != ph.npos)
							{
								wert = "";
								tagEnde = pos2;
							}
							// Anzahl, wie oft vervielfältigt werden soll
							bla = doppelVarkz + "i";
							if (ph.find(bla) != ph.npos)
							{
								wert = "";
								int anzahl = boost::lexical_cast<int>(werteIter->second);
								for (int i = 0; i < anzahl; i++)
								{
									wert += tc.substr(tagStart, tagEnde-tagStart);
								}
								pos2 = tagStart;
							}

							notag = false;
						}
					}
					if (notag)
					{
						wert = werteIter->second;
						if (wert.find(ph+"\xFF") != wert.npos)
						{
							size_t pos = wert.find_last_of("\xFF") + 1;
							wert = wert.substr(pos);
							werteM.erase(werteIter);
						}
					}

					if (!useSepEnd)
					{
						tc = tc.replace(pos2, pos1-pos2, wert);
					}
					else
					{
						tc = tc.replace(pos2, pos1+1-pos2, wert);
					}
					pos1 = pos2+wert.length();
				}
				else
				{
					pos1 = pos2+1;
				}
			}
			ck--;
		}
		// Ausgabe der veränderten Vorlage
		
		std::string dateiname;
		
		switch (modus)
		{
		case PZED:
			werteIter = werteM.find(dateiVar);
			dateiname = ausgabeVz + (werteIter->second);
			dateiname += fnZusatz;
			dateiname += ".txt";
			break;
		case EINEDATEI:
			dateiname = ausgabeVz + "all_in_one.txt";
			break;
		default:
			break;
		}

		std::ofstream ausgabe(dateiname.c_str(), ausgabeArt);
		ausgabe << tc;
		ausgabe.close();
		
		
		if (fehler)
		{
			break;
		}
		else
		{
			// Löschen der verwendeten Werte aus der Werte MultiMap
			for (uint i = 0; i < variablen.size(); i++)
			{
				werteIter = werteM.lower_bound(variablen[i]);
				std::string zwischen = werteIter->second;
				while(zwischen.find(variablen[i]+"\xFF") != zwischen.npos)
				{
					werteM.erase(werteIter);
					werteIter = werteM.lower_bound(variablen[i]);
					zwischen = werteIter->second;
				}
				werteM.erase(werteIter);
			}
		}
	}

	dbgA = "\n\n1612: --- FINISHED ---\n\n";
	schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "1612", WkLog::WkLog_GRUEN);


	return mods;
}


void Ersetzer::wknEinstellungen(std::string sep, uint md, int ck, bool flex, bool tg)
{
	modus = md;
	cycle = ck;
	seps = sep;
	flexible = flex;
	tags = tg;
}


void Ersetzer::ersetzeInit(std::string wDat, std::string vDat, std::string datVar, std::string aVz, int app, bool useSep)
{
	werteDatei = wDat;					// Wertedatei
	vorlageDatei = vDat;				// Vorlagendatei
	dateiVar = datVar;					// Dateivariable
	ausgabeVz = aVz;					// Ausgabeverzeichnis
	append = app;						// Ausgabe anhängen
	useSepEnd = useSep;					// Seperator als Variableende benutzen

}


void Ersetzer::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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


