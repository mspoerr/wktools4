#include "class_intfparse.h"


IntfParse::IntfParse(std::string suchPfad, std::string ausgabePfad, std::string ausgabeDatei, int slc, std::string logfile, WkLog *lga, Dip *dip)
{
	ausgabeDat = ausgabeDatei;
	ausgabeVz = ausgabePfad;
	aDatName = ausgabeVz+ausgabeDat;

	suchVz = suchPfad;
	snmpLocCol = slc;

	logAusgabe = lga;
	devInfo = dip;

	stop = false;

	//	logAusgabeDatei.rdbuf()->open(logFile.c_str(), std::ios_base::out | std::ios_base::app);}
}

IntfParse::~IntfParse()
{

}


int IntfParse::startParser(std::string pattern, bool append, bool noDescr)
{
	std::string dbgA = "";

	// Suchkriterien einstellen mithilfe der definierten Pattern
	// Als erstes werden die strings getrennt und in einen std::vector geschrieben
	std::vector<std::string> exps;
	size_t ppos1 = 0;
	size_t ppos2 = 0;
	while (ppos2 != pattern.npos)
	{
		ppos2 = pattern.find(";", ppos1);
		std::string ph = pattern.substr(ppos1, ppos2-ppos1);
		exps.push_back(ph);
		ppos1 = ppos2+1;
	}
	size_t laenge = exps.size();


	// Den Pfad durchsuchen.
	const fs::path dir_path(suchVz);
	if (!sdir(dir_path))
	{
		dbgA = "3202: Specified Path does not exist!";
		schreibeLog(WkLog::WkLog_FEHLER, dbgA, "3202", WkLog::WkLog_ROT);
		return 1;
	}

	// Soll der Header geschrieben werden?
	bool headerSchreiben = true;

	std::string aDatName = ausgabeVz+ausgabeDat;
	// Ausgabefile öffenen und vorbereiten
	if (append)
	{
		std::ifstream wdat(aDatName.c_str());
		if (wdat.is_open())
		{
			headerSchreiben = false;
		}
		wdat.close();
		ausgabe.rdbuf()->open(aDatName.c_str(), std::ios_base::out | std::ios_base::app);
	}
	else
	{
		ausgabe.rdbuf()->open(aDatName.c_str(), std::ios_base::out);
	}

	ausgabe << "Hostname;Intf-Name;Intf status;";
	if (!noDescr)
	{
		ausgabe << "Description;";
	}
	ausgabe << "IP address;MTU;Encapsulation;Duplex;Speed;";
	ausgabe << "I bits/s;I pkts/s;O bits/s;O pkts/s;load-interval;";
	ausgabe << "I pkts;I bytes;I buffer;I Broadcasts;I runts;I giants;I throttles;";
	ausgabe << "I errors;O pkts;O bytes;O buffer;O underruns;O errors;Intf resets;\n";

	if (ausgabe.fail())
	{
		dbgA = "3201: Failed to write to " + aDatName;
		schreibeLog(WkLog::WkLog_FEHLER, dbgA, "3201", WkLog::WkLog_ROT);
		return 1;
	}

	while (!files.empty())
	{
		if (stop)
		{
			dbgA = "3609: Device Information Parser stopped";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "3609", WkLog::WkLog_ROT);
			return 0;
		}
		std::string dateiname = files.front().string();
		files.pop();

		bool vorhanden = false;
		for (size_t i = 0; i < laenge; i++)
		{
			if (dateiname.find(exps[i]) != dateiname.npos)
			{
				vorhanden = true;
				break;
			}
		}
		if (vorhanden)
		{
			dbgA = "3610: " + dateiname + "\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "3610", WkLog::WkLog_BLAU);

			std::ifstream konfigDatei(dateiname.c_str(), std::ios_base::in);
			std::string ganzeDatei;

			getline(konfigDatei, ganzeDatei, '\0');
			konfigDatei.close();

			// Wenn die Datei leer ist, wird dieser Durchlauf beendet.
			if (ganzeDatei.empty())
			{
				dbgA = "3611: File is empty\n";
				schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "3611", WkLog::WkLog_BLAU);
			}
			else
			{
				bool catos = false;					// CatOS Daten?
				std::string hostname = "";				// Hostname

				std::queue<std::string> intfName;				// Interface Name
				std::queue<std::string> intfStatus;			// Interface Status
				std::queue<std::string> descr;				// Description
				std::queue<std::string> ipAddress;			// IP Address
				std::queue<std::string> mtu;					// MTU
				std::queue<std::string> encaps;				// Encapsulation
				std::queue<std::string> duplex;				// Duplex
				std::queue<std::string> speed;				// Speed
				
				std::queue<std::string> ibits;				// Input Bits/s
				std::queue<std::string> ipkts;				// Input Packets/s
				std::queue<std::string> obits;				// Output Bits/s
				std::queue<std::string> opkts;				// Output Packets/s
				std::queue<std::string> loadint;				// Load Interval

				std::queue<std::string> ibyte;				// Input total Bytes
				std::queue<std::string> ipkt;					// Input total Packets
				std::queue<std::string> ibuffer;				// Input total Packets
				std::queue<std::string> broadc;				// Broadcasts
				std::queue<std::string> runts;				// Runts
				std::queue<std::string> giants;				// Giants
				std::queue<std::string> throttles;			// Throttles
				std::queue<std::string> ierrors;				// Input errors
				
				std::queue<std::string> obyte;				// Output total Bytes
				std::queue<std::string> opkt;					// Output total Packets
				std::queue<std::string> obuffer;				// Underruns
				std::queue<std::string> underrrun;			// Underruns
				std::queue<std::string> oerrors;				// Output Errors
				std::queue<std::string> resets;				// Interface Resets


				size_t pos1 = ganzeDatei.find("show interface");

				// SHOW VER
				//////////////////////////////////////////////////////////////////////////
				size_t aktPos1 = 0;
				size_t aktPos2 = 0;

				aktPos1 = ganzeDatei.find(" uptime is");
				aktPos2 = ganzeDatei.rfind("\n", aktPos1);
				if (aktPos1 != ganzeDatei.npos)
				{
					hostname = ganzeDatei.substr(aktPos2+1, aktPos1 - aktPos2 - 1);
				}
				else if ((aktPos1 = ganzeDatei.find("Mod Port Model")) < pos1)
				{				
					catos = true;
				}
				else
				{
					aktPos1 = 0;
				}

				size_t intfpos = ganzeDatei.find("line protocol", pos1);

				while (intfpos < ganzeDatei.npos)
				{
					// Interface Name
					aktPos1 = intfpos;
					intfpos = ganzeDatei.find("line protocol", intfpos + 10);
					
					aktPos1 = ganzeDatei.rfind("\n", aktPos1) + 1;
					aktPos2 = ganzeDatei.find(" ", aktPos1);
					intfName.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

					// Interface Status
					aktPos1 = ganzeDatei.find("is", aktPos2) + 3;
					aktPos2 = ganzeDatei.find(",", aktPos1);
					std::string temp = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					temp += "/";
					aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos2);
					aktPos1 = ganzeDatei.rfind("is", aktPos2) + 3;
					temp += ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
					intfStatus.push(temp);

					// Description, falls vorhanden
					if ((aktPos1 = ganzeDatei.find("Description: ", aktPos2)) < intfpos)
					{
						aktPos1 += 13;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						descr.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						descr.push("");
					}

					// IP Adresse falls vorhanden
					if ((aktPos1 = ganzeDatei.find("Internet address is ", aktPos2)) < intfpos)
					{
						aktPos1 += 20;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						ipAddress.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						ipAddress.push("");
					}

					// MTU
					aktPos1 = ganzeDatei.find("MTU", aktPos2) + 4;
					aktPos2 = ganzeDatei.find(" ", aktPos1);
					mtu.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

					// Encapsulation
					aktPos1 = ganzeDatei.find("Encapsulation ", aktPos2) + 14;
					aktPos2 = ganzeDatei.find_first_of(",\r\n", aktPos1);
					encaps.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

					// Duplex und Speed falls vorhanden
					if ((aktPos1 = ganzeDatei.find("duplex", aktPos2)) < intfpos)
					{
						aktPos1 = ganzeDatei.rfind(" ", aktPos1);
						aktPos2 = ganzeDatei.find(",", aktPos1);
						duplex.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
						
						aktPos1 = aktPos2+2;
						aktPos2 = ganzeDatei.find_first_of(",\r\n", aktPos1);
						speed.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						duplex.push("");
						speed.push("");
					}
					
					// Load Interval
					if ((aktPos1 = ganzeDatei.find(" input rate ", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind("  ", aktPos1) + 2;
						loadint.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						loadint.push("");
					}

					// Input Bits/s
					if ((aktPos1 = ganzeDatei.find("input rate ", aktPos2)) < intfpos)
					{
						aktPos1 += 11;
						aktPos2 = ganzeDatei.find(" ", aktPos1);
						ibits.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
						
						// Input Pkts/s
						if ((aktPos1 = ganzeDatei.find(" packets/sec", aktPos2)) < intfpos)
						{
							aktPos2 = aktPos1;
							aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
							ipkts.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
						}
						else
						{
							ipkts.push("");
						}
					}
					else
					{
						ibits.push("");
						ipkts.push("");
					}
					// Output Bits/s
					if ((aktPos1 = ganzeDatei.find("output rate ", aktPos2)) < intfpos)
					{
						aktPos1 += 12;
						aktPos2 = ganzeDatei.find(" ", aktPos1);
						obits.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
						// Output Pkts/s
						if ((aktPos1 = ganzeDatei.find(" packets/sec", aktPos2)) < intfpos)
						{
							aktPos2 = aktPos1;
							aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
							opkts.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
						}
						else
						{
							opkts.push("");
						}
					}
					else
					{
						obits.push("");
						opkts.push("");
					}
					
					// Input total Pkts
					if ((aktPos1 = ganzeDatei.find(" packets input", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						ipkt.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						ipkt.push("");
					}

					// Input total Bytes
					if ((aktPos1 = ganzeDatei.find(" bytes", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						ibyte.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						ibyte.push("");
					}
					
					// Input Buffer errors
					if ((aktPos1 = ganzeDatei.find(" no buffer", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						ibuffer.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						ibuffer.push("");
					}

					// Input Broadcasts
					if ((aktPos1 = ganzeDatei.find("Received ", aktPos2)) < intfpos)
					{
						aktPos1 += 9;
						aktPos2 = ganzeDatei.find("broadcasts", aktPos1)-1;
						broadc.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						broadc.push("");
					}
					// Input Runts
					if ((aktPos1 = ganzeDatei.find(" runts", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						runts.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						runts.push("");
					}
					// Input Giants
					if ((aktPos1 = ganzeDatei.find(" giants", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						giants.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						giants.push("");
					}
					// Input Throttles
					if ((aktPos1 = ganzeDatei.find(" throttles", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						throttles.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						throttles.push("");
					}
					// Input Erros
					if ((aktPos1 = ganzeDatei.find(" input errors", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						ierrors.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						ierrors.push("");
					}
					// Output total Pkts
					if ((aktPos1 = ganzeDatei.find(" packets output", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos2 - 1) + 1;
						opkt.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						opkt.push("");
					}
					// Output total Bytes
					if ((aktPos1 = ganzeDatei.find(" bytes", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
						obyte.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						obyte.push("");
					}
					

					// Output underruns
					if ((aktPos1 = ganzeDatei.find(" underruns", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
						underrrun.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						underrrun.push("");
					}
					// Output errors
					if ((aktPos1 = ganzeDatei.find(" output errors", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
						oerrors.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						oerrors.push("");
					}
					// Interface Resets
					if ((aktPos1 = ganzeDatei.find(" interface resets", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
						resets.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						resets.push("");
					}
					
					// Output Buffer Errors
					if ((aktPos1 = ganzeDatei.find(" output buffer failures", aktPos2)) < intfpos)
					{
						aktPos2 = aktPos1;
						aktPos1 = ganzeDatei.rfind(" ", aktPos1 - 1) + 1;
						obuffer.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
					}
					else
					{
						obuffer.push("");
					}

				}




				// File Ausgabe
				//////////////////////////////////////////////////////////////////////////
				// Hostname   Location   Devicetype   S/N   Memory   bootfile

				while (!intfName.empty())
				{
					ausgabe << hostname << ";" << intfName.front() << ";" << intfStatus.front() << ";";
					
					if (!noDescr)
					{
						ausgabe << descr.front() << ";";
					}
					
					ausgabe << ipAddress.front() << ";" << mtu.front() << ";";
					ausgabe << encaps.front() << ";" << duplex.front() << ";" << speed.front() << ";";
					ausgabe << ibits.front() << ";" << ipkts.front() << ";" << obits.front() << ";";
					ausgabe << opkts.front() << ";" << loadint.front() << ";" << ipkt.front() << ";";
					ausgabe << ibyte.front() << ";" << ibuffer.front() << ";" << broadc.front() << ";";
					ausgabe << runts.front() << ";" << giants.front() << ";" << throttles.front() << ";";
					ausgabe << ierrors.front() << ";" << opkt.front() << ";" << obyte.front() << ";";
					ausgabe << obuffer.front() << ";" << underrrun.front() << ";" << oerrors.front() << ";";
					ausgabe << resets.front() << ";\n";

					intfName.pop();
					intfStatus.pop();
					descr.pop();
					ipAddress.pop();
					mtu.pop();
					encaps.pop();
					duplex.pop();
					speed.pop();
					ibits.pop();
					ipkts.pop();
					obits.pop();
					opkts.pop();
					loadint.pop();
					ipkt.pop();
					ibyte.pop();
					ibuffer.pop();
					broadc.pop();
					runts.pop();
					giants.pop();
					throttles.pop();
					ierrors.pop();
					opkt.pop();
					obyte.pop();
					obuffer.pop();
					underrrun.pop();
					oerrors.pop();
					resets.pop();
				}

			}

		}
	}

	ausgabe.close();
	return 0;
}

void IntfParse::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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


// schreibeLogfile:
//*****************
// Zum Beschreiben des Log Files
void IntfParse::schreibeLogfile(std::string logInput)
{
	//logAusgabeDatei << logInput;
}


bool IntfParse::sdir(fs::path pfad)
{
	if ( !exists( pfad ) ) 
	{
		return false;
	}

	std::string dbgA = "3609: Searching Directory " + pfad.string() + "\n";
	schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "3609", WkLog::WkLog_BLAU);

	fs::directory_iterator end_itr; // default construction yields past-the-end
	for ( fs::directory_iterator itr( pfad ); itr != end_itr; ++itr )
	{
		if ( fs::is_directory(itr->status()) )
		{
			// Pfad gefunden
			sdir(itr->path());
		}
		else
		{
			// File gefunden
			files.push(itr->path());
		}
	}
	return true;
}
