#include "class_utaparse.h"

#include <boost/tokenizer.hpp>


UTAParse::UTAParse(std::string suchPfad, std::string ausgabePfad, std::string ausgabeDatei, int slc, std::string logfile, WkLog *lga, Dip *dip)
{
	ausgabeDat = ausgabeDatei;
	ausgabeVz = ausgabePfad;
	aDatName = ausgabeVz+ausgabeDat;

	suchVz = suchPfad;
	snmpLocCol = slc;

	logAusgabe = lga;
	devInfo = dip;

	stop = false;

	//	logAusgabeDatei.rdbuf()->open(logFile.c_str(), std::ios_base::out | std::ios_base::app);
}



UTAParse::~UTAParse()
{

}

// Definition der nötigen Funktionszeiger
void (UTAParse::*spalte1Aus)(std::string);
void (UTAParse::*spalte2Aus)(std::string);
void (UTAParse::*spalte3Aus)(std::string);
void (UTAParse::*spalte4Aus)(std::string);
void (UTAParse::*spalte5Aus)(std::string);
void (UTAParse::*spalte6Aus)(std::string);
void (UTAParse::*spalte7Aus)(std::string);
void (UTAParse::*spalte8Aus)(std::string);
void (UTAParse::*spalte9Aus)(std::string);
void (UTAParse::*spalte10Aus)(std::string);
void (UTAParse::*spalte11Aus)(std::string);

int UTAParse::startParser(std::string pattern, bool append, oo outOpt, bool nurChassis)
{
	std::string slc = "";						// Platzhalter für die SNMP Location Spalten
	std::string sph = "Location/Slot;";			// Platzhalter für die Location in der ersten Zeile
	for (int i = 1; i < snmpLocCol; i++)
	{
		slc += ";";
		sph += ";";
	}
	
	int returnValue = 0;

	spalte1Aus = &UTAParse::schreibeNein;
	spalte2Aus = &UTAParse::schreibeNein;
	spalte3Aus = &UTAParse::schreibeNein;
	spalte4Aus = &UTAParse::schreibeNein;
	spalte5Aus = &UTAParse::schreibeNein;
	spalte6Aus = &UTAParse::schreibeNein;
	spalte7Aus = &UTAParse::schreibeNein;
	spalte8Aus = &UTAParse::schreibeNein;
	spalte9Aus = &UTAParse::schreibeNein;
	spalte10Aus = &UTAParse::schreibeNein;
	spalte11Aus = &UTAParse::schreibeNein;
	
	
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
	
	if (outOpt.spalte1)
	{
		spalte1Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte2)
	{
		spalte2Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte3)
	{
		spalte3Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte4)
	{
		spalte4Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte5)
	{
		spalte5Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte6)
	{
		spalte6Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte7)
	{
		spalte7Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte8)
	{
		spalte8Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte9)
	{
		spalte9Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte10)
	{
		spalte10Aus = &UTAParse::schreibeJa;
	}
	if (outOpt.spalte11)
	{
		spalte11Aus = &UTAParse::schreibeJa;
	}

	if (headerSchreiben)
	{
		(*this.*spalte1Aus)("Hostname;");
		(*this.*spalte2Aus)(sph);
		(*this.*spalte3Aus)("DeviceType;");
		(*this.*spalte4Aus)("Description;");
		(*this.*spalte5Aus)("S/N;");
		(*this.*spalte6Aus)("HW Revision;");
		(*this.*spalte7Aus)("Memory;");
		(*this.*spalte8Aus)("Bootfile;");
		(*this.*spalte9Aus)("SW Version;");
		(*this.*spalte10Aus)("License Info;");
		(*this.*spalte11Aus)("Flash Size");
		ausgabe << "\n";
	}
	
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

		lic = "";
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
				varsInit();

				pos1 = ganzeDatei.find("show snmp");
				pos2 = ganzeDatei.find("show module", pos1);
				pos3 = ganzeDatei.find("show c7200", pos2);
				pos4 = ganzeDatei.find("show diag", pos3);
				pos5 = ganzeDatei.find("show invent", pos4);
				pos6 = ganzeDatei.find("show hardware", pos5);
				pos7 = ganzeDatei.find("show system", pos5);
				pos8 = ganzeDatei.find("show file system", pos5);

				pos1dsl = ganzeDatei.find("show hardware chassis");
				pos2dsl = ganzeDatei.find("show hardware", pos1dsl);
				pos3dsl = ganzeDatei.find("show hardware slot", pos2dsl);

				pos1iph = ganzeDatei.find("HTTP/1.1 200 OK");
				pos2iph = ganzeDatei.find("<HTML>");
				pos3iph = ganzeDatei.find("<html>");

				nodiag = false;
				c72er = false;
				gsr = false;

				if ((pos1iph != ganzeDatei.npos) || (pos2iph != ganzeDatei.npos) || (pos3iph != ganzeDatei.npos))
				{
					testPhones(ganzeDatei);
					ipphone = true;
				}
				else
				{
					if (!nurChassis)
					{
						if ((pos3 == ganzeDatei.npos) && (pos1dsl != ganzeDatei.npos))
						{
							dslam = true;
						}
						else if (pos5 == ganzeDatei.npos)
						{
							dbgA = "3612: Cannot parse file due to lack of information!\n";
							dbgA += "Maybe not all needed commands executed for IOS/CatOS/PixOS devices.\n";
							dbgA += "Please use all commands listed in the documentation!\n";
							schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "3612", WkLog::WkLog_ROT);
							returnValue = 1;
							continue;
						}
					}

					// SHOW VER
					//////////////////////////////////////////////////////////////////////////
					// Nexus oder nicht?
					aktPos1 = aktPos2 = 0;
					aktPos1 = ganzeDatei.find("Nexus Operating System");
					if (aktPos1 != ganzeDatei.npos)
					{
						nxosbox = true;
					}
					else
					{
						aktPos1 = ganzeDatei.find(", Version") + 10;
						aktPos2 = ganzeDatei.find_first_of(", ", aktPos1);
						if (aktPos1 != ganzeDatei.npos)
						{
							iosversion = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
						}				
						else
						{
							aktPos1 = 0;
						}

						aktPos1 = ganzeDatei.find(" uptime is");
						aktPos2 = ganzeDatei.rfind("\n", aktPos1);
						if (aktPos2 == ganzeDatei.npos)
						{
							aktPos2 = ganzeDatei.rfind("\r", aktPos1);
						}
						if (aktPos1 != ganzeDatei.npos)
						{
							aktPos2 = ganzeDatei.find_first_not_of(" \r\n", aktPos2);
							hostname = ganzeDatei.substr(aktPos2, aktPos1 - aktPos2);
							iosbox = true;
						}
						else if ((aktPos1 = ganzeDatei.find("Mod Port Model")) < pos1)
						{				
							catos = true;
						}
						else if ((aktPos1 = ganzeDatei.find("Hardware: ")) < pos1)
						{
							pixos = true;
						}
						else
						{
							aktPos1 = 0;
							iosbox = true;
						}
					}
					

					if (iosbox)
					{
						if ((aktPos1 = ganzeDatei.find("System image file is \"", aktPos1)) != ganzeDatei.npos)
						{
							aktPos2 = ganzeDatei.find("\"", aktPos1 + 22);
							if (aktPos2 != ganzeDatei.npos)
							{
								aktPos1 += 22;
								version = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
							}
							else
							{
								aktPos2 = aktPos1;
							}
						}

						// Switch Lizenzen auslesen
						if ((aktPos1 = ganzeDatei.find("License Level: ", aktPos1)) != ganzeDatei.npos)
						{
							aktPos1 += 15;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							lic = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);

							if ((aktPos1 = ganzeDatei.find("License Type: ", aktPos1)) != ganzeDatei.npos)
							{
								aktPos1 += 14;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								lic += "/";
								lic += ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
							}

						}


						bool processor = true;
						if ((aktPos1 = ganzeDatei.find("processor", aktPos2)) == ganzeDatei.npos)
						{
							processor = false;
							aktPos1 = ganzeDatei.find("(revision", aktPos2);
						}
						if (aktPos1 < pos1)
						{
							if ((aktPos1 = ganzeDatei.rfind("isco ", aktPos1)) == ganzeDatei. npos)
							{

							}
							else	
							{
								aktPos2 = ganzeDatei.find(" ", aktPos1 + 5);
								if (aktPos2 != ganzeDatei.npos)
								{
									aktPos1 += 5;
									deviceType = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
									if (deviceType.find("GRP") != deviceType.npos)
									{
										gsr = true;
									}
								}
								else
								{
									aktPos2 = aktPos1;
								}
							}
						}

						if ((aktPos1 = ganzeDatei.find("(", aktPos2)) == ganzeDatei.npos)
						{

						}
						else if (processor)
						{
							aktPos2 = ganzeDatei.find(")", aktPos1 + 1);
							if (aktPos2 != ganzeDatei.npos)
							{
								aktPos1++;
								prozessor = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
							}
							else
							{
								aktPos2 = aktPos1;
							}
						}
						if ((aktPos1 = ganzeDatei.find("(revision ", aktPos2)) != ganzeDatei.npos)
						{
							aktPos1	+= 10;
							aktPos2 = ganzeDatei.find(")", aktPos1);
							hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
						}
						else if ((aktPos1 = ganzeDatei.find("Revision", aktPos2)) != ganzeDatei.npos)
						{
							if ((aktPos1 < pos1dsl) && (aktPos1 < pos1))
							{
								aktPos1	+= 9;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
							}
							else
							{
								aktPos1 = aktPos2;
							}
						}
						else
						{
							aktPos1 = aktPos2;
						}


						if ((aktPos1 = ganzeDatei.find("with ", aktPos2)) == ganzeDatei.npos)
						{

						}
						else
						{
							aktPos2 = ganzeDatei.find(" ", aktPos1 + 5);
							if (aktPos2 != ganzeDatei.npos)
							{
								aktPos1 += 5;
								RAM = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
							}
							else
							{
								aktPos2 = aktPos1;
							}
						}

						if ((aktPos1 = ganzeDatei.find("Processor board ID", aktPos2)) == ganzeDatei.npos)
						{

						}
						else
						{
							aktPos2 = ganzeDatei.find_first_of("\r\n,", aktPos1 + 18);
							if (aktPos2 != ganzeDatei.npos)
							{
								aktPos1 += 18;
								chassisSN = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
							}
						}

						// Lizenzinfos bei ISR G2 Router auslesen
						if ((aktPos1 = ganzeDatei.find("License Info:", aktPos2)) < pos1)
						{
							if ((aktPos1 = ganzeDatei.find("ipbase", aktPos1)) < pos1)
							{
								aktPos2 = ganzeDatei.find("Configuration register", aktPos2);

								std::string speicherAlle = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
								std::string speicherZeile = "";
								std::string tempSpeicher = "";
								std::string temptemp = "";
								lic = "";

								aktPos1 = -1;
								aktPos2 = 0;
								int off[] = {14,14,14,14};
								boost::offset_separator f1(off, off + 4, false, true);
								for (; aktPos2 < speicherAlle.npos;)
								{
									aktPos2 = speicherAlle.find("\n", aktPos1 + 1);
									speicherZeile = speicherAlle.substr(aktPos1 + 1, aktPos2 - aktPos1);
									aktPos1 = aktPos2;
									if (speicherZeile.size() > 35)
									{
										int i = 0;
										boost::tokenizer<boost::offset_separator> tok(speicherZeile,f1);
										for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
										{
											switch (i)
											{
											case 0:
												temptemp = *beg;
												tempSpeicher = temptemp.substr(0, temptemp.find(" ")) + "/";
												break;
											case 1:
												temptemp = *beg;
												tempSpeicher += temptemp.substr(0, temptemp.find(" ")) + "/";
												break;
											case 2:
												temptemp = *beg;
												tempSpeicher += temptemp.substr(0, temptemp.find(" "));
												break;
											default:
												break;
											}
											i++;
										}
										lic += tempSpeicher + " || ";
									}
								}
							}




						}

						// Herausfinden, ob es sich um einen AccessPoint handelt
						if ((aktPos1 = ganzeDatei.find("Top Assembly Serial Number", aktPos2)) < pos1)
						{
							aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);	
							if ((aktPos1 = ganzeDatei.find("Model Number", aktPos2)) < pos1)
							{
								aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								deviceType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
							}
						}

						// Model Number bei Switches auslesen
						else if ((aktPos1 = ganzeDatei.find("Model revision number", aktPos2)) < pos1)
						{
							aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

							if ((aktPos1 = ganzeDatei.find("Model number", aktPos2)) < pos1)
							{
								aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								deviceType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
							}

							if ((aktPos1 = ganzeDatei.find("System serial number", aktPos2)) < pos1)
							{
								aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
							}

							// Wenn 3750 Stack, dann Infos zu den anderen Switches auslesen und als Modulinfo abspeichern
							while ( (aktPos1 = ganzeDatei.find("Model revision number", aktPos2)) < pos1)
							{
								aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

								aktPos1 = ganzeDatei.find("Model number", aktPos2);
								aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));


								aktPos1 = ganzeDatei.find("System serial number", aktPos2);
								aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
								modulBesch.push("");
								modulNr.push("StackMember");
							}
						}

						//			std::cout << hostname << " " << version << " " << deviceType << " " << prozessor << " " << RAM << " " << chassisSN << std::endl;

						// SHOW SNMP
						//////////////////////////////////////////////////////////////////////////

						if ((aktPos1 = ganzeDatei.find("Location: ", pos1)) == ganzeDatei.npos)
						{

						}
						else
						{
							if ((aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1)) != ganzeDatei.npos)
							{
								aktPos1 += 10;
								standort = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
							}
							else
							{
								aktPos2 = aktPos1;
							}
						}

						if (!dslam)
						{

							if (!nurChassis)
							{
								// SHOW MODULE
								//////////////////////////////////////////////////////////////////////////

								if ((pos3-pos2 > 300) && (pos6 - pos5 < 300))   // keine "show invent" Infos
								{
									// Modulinfos excl HW Revision
									size_t modPos1 = ganzeDatei.find("Mod ", pos2);
									size_t modPos2 = ganzeDatei.find("MAC addresses", modPos1 + 4);
									modPos1 = ganzeDatei.rfind("---", modPos2)+4;

									aktPos1 = -1;
									aktPos2 = 0;
									std::string cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 5);
									std::string cat6kZeile = "";
									int offsets[] = {3, 7, 39, 19, 11};
									boost::offset_separator f(offsets, offsets + 5);
									for (; aktPos2 < cat6k.npos;)
									{
										aktPos2 = cat6k.find("\n", aktPos1 + 1);
										cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
										aktPos1 = aktPos2;

										if (cat6kZeile.length() < 60)
										{
											continue;
										}

										int i = 0;
										boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f);
										for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
										{
											switch (i)
											{
											case 0:
												modulNr.push(*beg);
												break;
											case 2:
												modulBesch.push(*beg);
												break;
											case 3:
												modulBez.push(*beg);
												break;
											case 4:
												modulSN.push(*beg);
												break;
											default:
												break;
											}
											i++;
										}
									}

									// HW Revision der Hauptmodule
									bool cat4k = false;
									if ((ganzeDatei.find("-+---+------------+-", pos2) != ganzeDatei.npos))
									{
										cat4k = true;
									}
									modPos1 = ganzeDatei.find("MAC addresses", pos2);
									modPos2 = ganzeDatei.find("Sub-Module", modPos1 + 4);
									if (modPos2 == ganzeDatei.npos)
									{
										modPos2 = pos3;			// show c7200
									}
									if (modPos2 != ganzeDatei.npos)
									{
										modPos1 = ganzeDatei.rfind("---", modPos2)+4;

										aktPos1 = -1;
										aktPos2 = 0;
										cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 5);
										cat6kZeile = "";
										int offsets1[2];
										if (cat4k)
										{
											offsets1[0] = 36;
											offsets1[1] = 3;
										}
										else
										{
											offsets1[0] = 40;
											offsets1[1] = 4;									
										}
										boost::offset_separator f1(offsets1, offsets1 + 2);

										for (; aktPos2 < cat6k.npos;)
										{
											aktPos2 = cat6k.find("\n", aktPos1 + 1);
											cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
											aktPos1 = aktPos2;

											if (cat6kZeile.length() < 40)
											{
												continue;
											}

											int i = 0;
											boost::tokenizer<boost::offset_separator> tok1(cat6kZeile,f1);
											for(boost::tokenizer<boost::offset_separator>::iterator beg1=tok1.begin(); beg1!=tok1.end();++beg1)
											{
												switch (i)
												{
												case 0:
													break;
												case 1:
													hwRevision.push(*beg1);
													break;
												default:
													break;
												}
												i++;
											}
										}

									}
									// Submodul Infos
									modPos1 = ganzeDatei.find("Sub-Module", pos2);
									if (modPos1 < ganzeDatei.npos)
									{
										modPos2 = ganzeDatei.find("Online Diag", modPos1 + 4);
										modPos1 = ganzeDatei.rfind("---", modPos2)+4;

										aktPos1 = -1;
										aktPos2 = 0;
										cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 5);
										cat6kZeile = "";
										int offsets2[] = {4, 28, 19, 14, 5};
										boost::offset_separator f2(offsets2, offsets2 + 5);
										for (; aktPos2 < cat6k.npos;)
										{
											aktPos2 = cat6k.find("\n", aktPos1 + 1);
											cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
											aktPos1 = aktPos2;

											if (cat6kZeile.length() < 50)
											{
												continue;
											}

											int i = 0;
											std::string temp = "";
											boost::tokenizer<boost::offset_separator> tok2(cat6kZeile,f2);
											for(boost::tokenizer<boost::offset_separator>::iterator beg2=tok2.begin(); beg2!=tok2.end();++beg2)
											{
												switch (i)
												{
												case 0:
													temp = "Submod. SL " + *beg2;
													modulNr.push(temp);
													break;
												case 1:
													modulBesch.push(*beg2);
													break;
												case 2:
													modulBez.push(*beg2);
													break;
												case 3:
													modulSN.push(*beg2);
													break;
												case 4:
													hwRevision.push(*beg2);
													break;
												default:
													break;
												}
												i++;
											}
										}

									}


								}


								// SHOW C7200
								//////////////////////////////////////////////////////////////////////////
								if (pos4 - pos3 > 300)
								{
									c72er = true;

									hwRevision.pop();
									aktPos1 = ganzeDatei.find("EEPROM", pos3);

									aktPos1 = ganzeDatei.find("Hardware revision", aktPos1) + 18;
									aktPos2 = ganzeDatei.find(" ", aktPos1);
									hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

									aktPos2 = ganzeDatei.find("EEPROM", aktPos1+1);
									aktPos1 = ganzeDatei.find("Hardware revision", aktPos2);
									if (aktPos1 > pos4)
									{
										aktPos1 = ganzeDatei.find("Hardware Revision", aktPos2);
									}
									aktPos1 += 18;
									
									aktPos2 = ganzeDatei.find(" ", aktPos1);
									hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

									aktPos1 = ganzeDatei.find("Serial ", aktPos1);
									aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
									aktPos1 = ganzeDatei.rfind(" ", aktPos2-4);
									npeSN = ganzeDatei.substr(aktPos1+1, aktPos2-aktPos1-1);

									/*				if ((aktPos1 = ganzeDatei.find("FRU", aktPos2)) <= pos4)
									{
									aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
									aktPos1 = ganzeDatei.rfind(" ", aktPos2-4);
									npeBez = ganzeDatei.substr(aktPos1+1, aktPos2-aktPos1);
									std::cout << npeBez << std::endl;
									}*/

								}

								// SHOW DIAG
								//////////////////////////////////////////////////////////////////////////
								// Überspringen, wenn show inventory verfügbar ist, 72er Router oder GSR
								
								if((pos6 - pos5 < 300) || c72er || gsr)
								{
									if ((pos5-pos4 > 300))
									{
										size_t slotpos = pos4;

										if (pos3-pos2 > 300)
										{
											// OSM Module bei 76er
											if ((slotpos = ganzeDatei.find("\nSlot", pos4)) < pos5)
											{
												while(slotpos < pos5)
												{
													aktPos1 = slotpos + 6;
													slotpos = ganzeDatei.find("\nSlot", aktPos1);
													tempModNr.push(ganzeDatei.substr(aktPos1, 1));

													aktPos2 = ganzeDatei.find("MBytes Total", aktPos1) - 1;
													aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
													tempMem.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1) + "MB");
												}
											}
										}
										else if ((slotpos = ganzeDatei.find("Physical slot", pos4)) < pos5)
										{
											// 75er    -    75er    -    75er
											//////////////////////////////////////////////////////////////////////////
											size_t chassisPos = ganzeDatei.find("(virtual)", pos4);
											size_t processorSlotpos = ganzeDatei.find("Processor", pos4);
											while (slotpos < chassisPos)
											{
												aktPos1 = slotpos;

												if (processorSlotpos < slotpos)
												{
													aktPos1 = processorSlotpos;
													aktPos1 = ganzeDatei.rfind("Slot", aktPos1) + 5;
													aktPos2 = ganzeDatei.find(":", aktPos1);
													modulNr.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

													aktPos2 = ganzeDatei.find(",", aktPos2);
													aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
													modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
													modulBez.push("");
													processorSlotpos = ganzeDatei.find("Processor", aktPos1);

													aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
													aktPos2 = ganzeDatei.find(",", aktPos1);
													hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

													aktPos1 = ganzeDatei.find("Serial", aktPos2) + 15;
													aktPos2 = ganzeDatei.find(" ", aktPos1);
													modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

													modulRouteMem.push("");
													modulVer.push("");

													aktPos1 = slotpos;
												}

												slotpos += 10;
												slotpos = ganzeDatei.find("Physical slot", slotpos);

												aktPos1 = ganzeDatei.rfind("Slot", aktPos1) + 5;
												aktPos2 = ganzeDatei.find(":", aktPos1);
												modulNr.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos2 = ganzeDatei.find("controller,", aktPos2) + 10;
												aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
												modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
												modulBez.push("");							

												aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
												aktPos2 = ganzeDatei.find(",", aktPos1);
												hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("Serial", aktPos2) + 15;
												aktPos2 = ganzeDatei.find(" ", aktPos1);
												modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("Memory Size:", aktPos2) + 13;
												aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
												modulRouteMem.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));


												size_t bevorSN = aktPos2;
												aktPos1 = ganzeDatei.find("VIP Software", aktPos2);							
												aktPos1 = ganzeDatei.find("Version ", aktPos1) + 8;
												aktPos2 = ganzeDatei.find(",", aktPos1);
												modulVer.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));


												// PAs
												while((aktPos1 = ganzeDatei.find("PA Bay", bevorSN)) < slotpos)
												{
													modulNr.push(ganzeDatei.substr(aktPos1, 8)); 

													modulRouteMem.push("");
													modulVer.push("");

													aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1) + 2;
													aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
													aktPos1 = ganzeDatei.rfind("\t", aktPos2) + 1;
													modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
													modulBez.push("");

													aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
													aktPos2 = ganzeDatei.find(",", aktPos1);
													hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

													aktPos1 = ganzeDatei.find("Serial number: ", aktPos1) + 15;
													aktPos2 = ganzeDatei.find(" ", aktPos1);
													modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
													bevorSN = aktPos2;
												}

											}
											aktPos1 = ganzeDatei.find("HW rev", aktPos2) + 7;
											aktPos2 = ganzeDatei.find(",", aktPos1);
											hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

											aktPos1 = ganzeDatei.find("Serial number", chassisPos) + 15;
											aktPos2 = ganzeDatei.find(" ", aktPos1);
											chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
										}
										else if ((slotpos = ganzeDatei.find("FRU NUMBER", pos4)) < pos5)
										{
											// AS5300
											slotpos = ganzeDatei.find("Slot", pos4);
											while (slotpos < pos5)
											{
												aktPos1 = slotpos + 5;
												slotpos = ganzeDatei.find("Slot", aktPos1);
												modulNr.push(ganzeDatei.substr(aktPos1, 1));

												aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1) + 2;
												aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
												modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("Hardware Version ", aktPos2) + 17;
												aktPos2 = ganzeDatei.find(",", aktPos1);
												hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("Serial ", aktPos2);
												aktPos2 = ganzeDatei.find(",", aktPos1);
												aktPos1 = ganzeDatei.rfind(" ", aktPos2) + 1;
												modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												if ((aktPos1 = ganzeDatei.find("FRU", aktPos2)) < slotpos)
												{
													aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
													aktPos2--;
													for (; ganzeDatei[aktPos2] == ' '; aktPos2--)
													{
													}
													aktPos1 = ganzeDatei.rfind(":", aktPos2-4) + 2;
													std::string bez = ganzeDatei.substr(aktPos1, aktPos2-aktPos1+1);
													if (bez == "UNKNOWN_BOARD_ID")
													{
														modulBez.push("");
													}
													else
													{
														modulBez.push(bez);
													}
												}
												else
												{
													modulBez.push("");
													aktPos1 = aktPos2;
												}
											}
										}
										else if ((slotpos = ganzeDatei.find("Slot", pos4)) < pos5)
										{
											// Cisco Allgemein
											while (slotpos < pos5)
											{
												bool wic = false;
												size_t wicPos = ganzeDatei.find("WIC Slot", aktPos1);
												if (slotpos-wicPos < 10)
												{
													wic = true;
												}
												aktPos1 = slotpos + 5;
												slotpos = ganzeDatei.find("Slot", aktPos1);
												if (wic)
												{
													modulNr.push("WIC " + ganzeDatei.substr(aktPos1, 1));
												}
												else
												{
													modulNr.push(ganzeDatei.substr(aktPos1, 1));
												}

												aktPos1 = ganzeDatei.find_first_of("\t ", aktPos1);
												aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
												while (ganzeDatei[aktPos1+1] == ' ')
												{
													aktPos1++;
												}
												modulBesch.push(ganzeDatei.substr(aktPos1+1, aktPos2-aktPos1-1));

												if ((aktPos1 = ganzeDatei.find("Hardware Revision", aktPos2)) < slotpos)
												{
													aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
													aktPos2 = ganzeDatei.find_first_of("\r\n\t", aktPos1);
												}
												else if ((aktPos1 = ganzeDatei.find("Hardware revision", aktPos2)) < slotpos)
												{
													aktPos1 += 18;
													aktPos2 = ganzeDatei.find_first_of(" \t", aktPos1);
												}
												else
												{
													aktPos1 = aktPos2;
												}
												hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("Serial ", aktPos2);
												aktPos2 = ganzeDatei.find_first_of("(\t\r\n", aktPos1);
												size_t snPos = ganzeDatei.find("Part number", aktPos1);
												if(snPos < aktPos2)
												{
													aktPos2 = ganzeDatei.rfind(" ", snPos);
													while (ganzeDatei[aktPos2] == ' ')
													{
														aktPos2--;
													}
													aktPos2++;
												}

												aktPos1 = ganzeDatei.rfind(" ", aktPos2-4) + 1;
												modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												if ((aktPos1 = ganzeDatei.find("FRU", aktPos2)) < slotpos)
												{
													aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
													aktPos2--;
													for (; ganzeDatei[aktPos2] == ' '; aktPos2--)
													{
													}
													aktPos1 = ganzeDatei.rfind(" ", aktPos2-4) + 1;
													modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1+1));
												}
												else if ((aktPos1 = ganzeDatei.find("Product Number", aktPos2)) < slotpos)
												{
													aktPos2 = ganzeDatei.find_first_of("\t\r\n", aktPos1);
													aktPos2--;
													for (; ganzeDatei[aktPos2] == ' '; aktPos2--)
													{
													}
													aktPos1 = ganzeDatei.rfind(" ", aktPos2-4) + 1;
													modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1+1));
												}
												else
												{
													modulBez.push("");
													aktPos1 = aktPos2;
												}

											}
										}
										else if ((slotpos = ganzeDatei.find("SLOT", pos4)) < pos5)
										{
											// GSR - GSR - GSR - GSR - GSR
											while (slotpos < pos5)
											{
												aktPos1 = slotpos + 5;
												slotpos = ganzeDatei.find("SLOT", aktPos1);
												modulNr.push(ganzeDatei.substr(aktPos1, 2));

												aktPos1 = ganzeDatei.find(":", aktPos1) + 2;
												aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
												modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("rev", aktPos2) + 4;
												aktPos2 = ganzeDatei.find_first_of("\r\n\t", aktPos1);
												hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("S/N", aktPos1) + 3;
												aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
												modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												if ((aktPos1 = ganzeDatei.find("Linecard/Module: ", aktPos2)) < slotpos)
												{
													aktPos1 += 17;
													aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
													modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
												}
												else
												{
													modulBez.push("");
													aktPos1 = aktPos2;
												}

												if ((aktPos1 = ganzeDatei.find("Route Memory: ", aktPos2)) < slotpos)
												{
													aktPos1 += 14;
													aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
													modulRouteMem.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
												}
												else
												{
													aktPos1 = aktPos2;
													modulRouteMem.push("");
												}

												aktPos1 = ganzeDatei.find("Packet Memory: ", aktPos2);
												if (aktPos1 < slotpos)
												{
													aktPos1 += 15;
													aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
													modulPacketMem.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
												}
												else
												{
													aktPos1 = aktPos2;
													modulPacketMem.push("");
												}
											}
										}
									}
									else
									{
										nodiag = true;
									}
								}
								else
								{
									nodiag = true;
								}
								

								// SHOW INVENT
								//////////////////////////////////////////////////////////////////////////
								if(pos6 - pos5 > 300)
								{
									if ((chassisSN == "") || (beschreibung == ""))
									{
										if ((aktPos1 = ganzeDatei.find("hassis", pos5)) != ganzeDatei.npos)
										{
											aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
											aktPos2 = ganzeDatei.find("\"", aktPos1);
											beschreibung = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

											aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
											aktPos2 = ganzeDatei.find(" ", aktPos1);
											deviceType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

											aktPos1 = ganzeDatei.find("SN: ", aktPos1) + 4;
											aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
											chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
										}
									}
									if(nodiag && !c72er)
									{
										bool ed = true;		// erster Durchlauf wird ausgelassen, da im ersten Eitrag das Chassis steht
										aktPos2 = pos5;
										while (1)
										{
											if ((aktPos1 = ganzeDatei.find("NAME", aktPos2)) != ganzeDatei.npos)
											{
												if (ed)
												{
													ed = false;
													aktPos2 = aktPos1 + 10;
													continue;
												}
												aktPos1 += 7;
												
												aktPos2 = ganzeDatei.find("\"", aktPos1);
												std::string modNr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
												modulNr.push(modNr);
												if (!tempModNr.empty())
												{
													if (modNr == tempModNr.front())
													{
														modulRouteMem.push(tempMem.front());
														tempMem.pop();
														tempModNr.pop();
													}
													else
													{
														modulRouteMem.push("");
													}
												}
												else
												{
													modulRouteMem.push("");
												}

												aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
												aktPos2 = ganzeDatei.find("\"", aktPos1);
												size_t aktPos3 = ganzeDatei.rfind("Rev. ", aktPos2);
												if ((aktPos3 < aktPos1) || (aktPos3 == ganzeDatei.npos))
												{
													modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
													hwRevision.push("");
												}
												else
												{
													aktPos3 += 5;
													modulBesch.push(ganzeDatei.substr(aktPos1, aktPos3-aktPos1));
													hwRevision.push(ganzeDatei.substr(aktPos3, aktPos2-aktPos3));
												}



												aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
												aktPos2 = ganzeDatei.find(" ", aktPos1);
												modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

												aktPos1 = ganzeDatei.find("SN", aktPos2) + 4;
												aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
												modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
											}
											else
											{
												break;
											}

										}
									}

								}

								// SHOW HARDWARE
								//////////////////////////////////////////////////////////////////////////
								// LS1010
								//////////////////////////////////////////////////////////////////////////
								if(((ganzeDatei.find("EEPROM", pos6) != ganzeDatei.npos)) && (chassisSN == ""))
								{
									size_t modPos1 = ganzeDatei.find("Slot", pos6);
									size_t modPos2 = ganzeDatei.find("EEPROM", modPos1);
									modPos2 = ganzeDatei.rfind("\n", modPos2);
									modPos1 = ganzeDatei.rfind("---", modPos2) + 4;

									aktPos1 = 0;
									aktPos2 = 0;

									std::string modulTabelle = ganzeDatei.substr(modPos1, modPos2-modPos1-4);
									std::string modulZeile = "";
									int offsets[] = {5, 14, 16, 8, 22, 4};
									boost::offset_separator f(offsets, offsets+6, false);
									for (; aktPos2 < modulTabelle.npos;)
									{
										aktPos2 = modulTabelle.find("\n", aktPos1 + 1);
										modulZeile = modulTabelle.substr(aktPos1 + 1, aktPos2 - aktPos1);

										if (modulZeile.size() < 10)
										{
											aktPos1++;
											continue;
										}

										aktPos1 = aktPos2;

										int i = 0;
										boost::tokenizer<boost::offset_separator> tok(modulZeile, f);

										for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
										{
											switch (i)
											{
											case 0:
												modulNr.push(*beg);
												break;
											case 1:
												modulBesch.push(*beg);
												break;
											case 3:
												modulSN.push(*beg);
												break;
											case 5:
												hwRevision.push(*beg);
												break;
											default:
												break;
											}
											i++;
										}
										modulBez.push("");
									}

									aktPos1 = ganzeDatei.find("--\n", modPos2) + 16;
									chassisSN = ganzeDatei.substr(aktPos1, 8);
								}

								// show file system
								//////////////////////////////////////////////////////////////////////////
								if (pos8 != ganzeDatei.npos)
								{
									size_t flashPos1 = ganzeDatei.find("Prefixes", pos8);
									if (flashPos1 != ganzeDatei.npos)
									{
										size_t flashPos2 = ganzeDatei.rfind(":", ganzeDatei.npos) + 1;
										if ((flashPos2 - flashPos1) < 3000)
										{
											filesystem = true;
										}
									}
									else
									{
										flash = "";
									}
								}
								else
								{
									flash = "";
								}

							}
						}
						else
						{
							// 62xx  -  62xx  -  62xx
							//////////////////////////////////////////////////////////////////////////

							aktPos1 = ganzeDatei.find("BackPlane EEPROM:", pos1dsl);

							aktPos1 = ganzeDatei.find("Hardware Revision", aktPos1);
							aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

							aktPos1 = ganzeDatei.find("Chassis Serial Number    : ", aktPos1) + 27;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

							aktPos1 = ganzeDatei.find("I/O Card EEPROM:", aktPos2);

							aktPos1 = ganzeDatei.find("Hardware Revision", aktPos1);
							aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

							aktPos1 = ganzeDatei.find("PCB Serial Number        : ", aktPos1) + 27;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
							modulNr.push("I/O Card");
							modulBesch.push("");


							if ((aktPos1 = ganzeDatei.find("Slot 1 Power Module EEPROM:", aktPos2)) < ganzeDatei.npos)
							{
								aktPos1 = ganzeDatei.find("Hardware Revision", aktPos1);
								aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

								aktPos1 = ganzeDatei.find("Chassis Serial Number    : ", aktPos1) + 27;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
								modulNr.push("Power Supply 1");
								modulBesch.push("");
							}
							if ((aktPos1 = ganzeDatei.find("Slot 2 Power Module EEPROM:", aktPos2)) < ganzeDatei.npos)
							{
								aktPos1 = ganzeDatei.find("Hardware Revision", aktPos1);
								aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

								aktPos1 = ganzeDatei.find("Chassis Serial Number    : ", aktPos1) + 27;
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
								modulNr.push("Power Supply 2");
								modulBesch.push("");
							}

							aktPos1 = ganzeDatei.find("I/O Card: ", pos2dsl) + 10;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

							if ((aktPos1 = ganzeDatei.find("Power Supply Module 1: ", aktPos2)) < ganzeDatei.npos)
							{
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
							}
							if ((aktPos1 = ganzeDatei.find("Power Supply Module 2: ", aktPos2)) < ganzeDatei.npos)
							{
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
							}

							size_t slotpos = ganzeDatei.find("Slot", pos3dsl);
							size_t dcardpos = ganzeDatei.find("Daughtercard", pos3dsl);
							size_t mboardpos = ganzeDatei.find("Motherboard", pos3dsl);
							while (slotpos != ganzeDatei.npos)
							{
								aktPos1 = slotpos + 5;
								slotpos = ganzeDatei.find("Slot", aktPos1);

								if (slotpos-aktPos1 > 500)
								{
									aktPos2 = ganzeDatei.find(":", aktPos1);
									std::string nr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
									modulNr.push(nr);

									aktPos1 = aktPos2+1;
									aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
									std::string bez = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
									modulBez.push(bez);

									aktPos1 = ganzeDatei.find("Hardware Revision", aktPos1);
									aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
									aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
									hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));


									if (dcardpos < slotpos)
									{
										// NI DCARD
										modulBesch.push("NI2 Daughtercard");
										aktPos1 = ganzeDatei.find("PCB Serial Number        : ", aktPos2) + 27;
										aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
										modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

										// NI MBoard
										modulBesch.push("NI2 Motherboard");
										modulBez.push(bez);
										modulNr.push(nr);

										aktPos1 = ganzeDatei.find("Hardware Revision", aktPos1);
										aktPos1 = ganzeDatei.find(":", aktPos1) + 1;
										aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
										hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

										dcardpos = ganzeDatei.find("Daughtercard", dcardpos + 10);
										mboardpos = ganzeDatei.find("Motherboard", mboardpos + 10);
									}
									else
									{
										modulBesch.push("");
									}

									aktPos1 = ganzeDatei.find("PCB Serial Number        : ", aktPos2) + 27;
									aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
									modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
								}
							}

						}
					}
					else if (catos)
					{
						// CATOS - CATOS - CATOS - CATOS
						//////////////////////////////////////////////////////////////////////////

						// SHOW VER
						//////////////////////////////////////////////////////////////////////////
						aktPos1 = ganzeDatei.find("NmpSW: ") + 7;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						version = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						iosversion = version;

						aktPos1 = ganzeDatei.find("Hardware Version: ", aktPos2) + 18;
						aktPos2 = ganzeDatei.find(" ", aktPos1);
						hwRevision.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

						aktPos1 = ganzeDatei.find("Model: ") + 7;
						aktPos2 = ganzeDatei.find(" ", aktPos1);
						deviceType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = ganzeDatei.find("Serial #: ", aktPos2) + 10;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						std::string psSN[] = {"",""};
						aktPos1 = ganzeDatei.find("PS1  Module: ", aktPos2);
						if (aktPos1 != ganzeDatei.npos)
						{
							aktPos1 = ganzeDatei.find("Serial #: ", aktPos1) + 10;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							psSN[0] = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
							aktPos1 = ganzeDatei.find("Serial #: ", aktPos2) + 10;
							if (aktPos1-aktPos2 < 100)
							{
								aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
								psSN[1] = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
								hwRevision.push("");
							}
						}

						aktPos1 = ganzeDatei.find("DRAM", aktPos2);
						aktPos1 = ganzeDatei.find("---", aktPos1);
						aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);
						aktPos2 = ganzeDatei.find("Uptime", aktPos1);

						std::string speicherAlle = ganzeDatei.substr(aktPos1, aktPos2 - aktPos1);
						std::string speicherZeile = "";
						std::string tempSpeicher = "";
						std::string temptemp = "";

						aktPos1 = -1;
						aktPos2 = 0;
						int off[] = {7, 7, 17, 7, 9, 7};
						boost::offset_separator f1(off, off + 6, false);
						for (; aktPos2 < speicherAlle.npos;)
						{
							aktPos2 = speicherAlle.find("\n", aktPos1 + 1);
							speicherZeile = speicherAlle.substr(aktPos1 + 1, aktPos2 - aktPos1);
							aktPos1 = aktPos2;
							if (speicherZeile.size() > 50)
							{
								int i = 0;
								boost::tokenizer<boost::offset_separator> tok(speicherZeile,f1);
								for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
								{
									switch (i)
									{
									case 1:
										temptemp = *beg;
										while (temptemp[0] == ' ')
										{
											temptemp.erase(0, 1);
										}

										RAM = temptemp;
										break;
									case 3:
										temptemp = *beg;
										while (temptemp[0] == ' ')
										{
											temptemp.erase(0, 1);
										}

										tempSpeicher += temptemp;
										tempSpeicher += ";";
										break;
									case 4:
										temptemp = *beg;
										while (temptemp[0] == ' ')
										{
											temptemp.erase(0, 1);
										}

										tempSpeicher += temptemp;
										tempSpeicher += ";";
										flash = "Flash:;" + tempSpeicher;
										break;
									default:
										break;
									}
									i++;
								}
							}
						}


						// SHOW SNMP
						//////////////////////////////////////////////////////////////////////////
						// nichts interessantes für CatOS

						// SHOW MODULE
						//////////////////////////////////////////////////////////////////////////

						if (!nurChassis)
						{
							if (pos3 - pos2 > 300)
							{
								size_t modPos1 = ganzeDatei.find("Mod ", pos2);
								size_t modPos2 = ganzeDatei.find_first_of("\r\n", modPos1);
								std::string cat6k;
								std::string cat6kZeile;

								if (modPos2-modPos1 > 73)
								{
									// Steinalt CatOS
									modPos2 = ganzeDatei.find("Mod ", modPos2);
									modPos1 = ganzeDatei.rfind("---", modPos2)+4;

									aktPos1 = -1;
									aktPos2 = 0;
									cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 4);
									cat6kZeile = "";
									int offsets[] = {2, 28, 22, 10, 9};
									boost::offset_separator f(offsets, offsets + 5, false);
									for (; aktPos2 < cat6k.npos;)
									{
										aktPos2 = cat6k.find("\n", aktPos1 + 1);
										cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
										aktPos1 = aktPos2;
										if (cat6kZeile.size() > 50)
										{
											int i = 0;
											boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f);
											for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
											{
												switch (i)
												{
												case 0:
													modulNr.push(*beg);
													break;
												case 2:
													modulBesch.push(*beg);
													break;
												case 3:
													modulBez.push(*beg);
													break;
												case 4:
													modulSN.push(*beg);
												default:
													break;
												}
												i++;
											}
										}
									}
									
									modPos2 = ganzeDatei.find("Mod ", modPos2 + 4) - 2;
									modPos1 = ganzeDatei.rfind("---", modPos2)+4;
									aktPos1 = -1;
									aktPos2 = 0;
									cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1);
									cat6kZeile = "";
									int offsets1[] = {43, 4};
									boost::offset_separator f1(offsets1, offsets1 + 2, false);
									for (; aktPos2 < cat6k.npos;)
									{
										aktPos2 = cat6k.find("\n", aktPos1 + 1);
										cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
										aktPos1 = aktPos2;
										if (cat6kZeile.size() > 50)
										{
											int i = 0;
											boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f1);
											for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
											{
												switch (i)
												{
												case 1:
													hwRevision.push(*beg);
													break;
												default:
													break;
												}
												i++;
											}
										}
									}
								}
								else
								{
									modPos2 = ganzeDatei.find("Mod ", modPos2);
									modPos1 = ganzeDatei.rfind("---", modPos2)+4;

									aktPos1 = -1;
									aktPos2 = 0;
									cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1 - 4);
									cat6kZeile = "";
									int offsets[] = {2, 13, 26, 19};
									boost::offset_separator f(offsets, offsets + 4, false);
									for (; aktPos2 < cat6k.npos;)
									{
										aktPos2 = cat6k.find("\n", aktPos1 + 1);
										cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
										aktPos1 = aktPos2;
										if (cat6kZeile.size() > 50)
										{
											int i = 0;
											boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f);
											for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
											{
												switch (i)
												{
												case 0:
													modulNr.push(*beg);
													break;
												case 2:
													modulBesch.push(*beg);
													break;
												case 3:
													modulBez.push(*beg);
													break;
												default:
													break;
												}
												i++;
											}
										}
									}
									modPos2 = ganzeDatei.find("Mod ", modPos2 + 4) - 2;
									modPos1 = ganzeDatei.rfind("---", modPos2)+4;
									aktPos1 = -1;
									aktPos2 = 0;
									cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1);
									cat6kZeile = "";
									int offsets1[] = {25, 11};
									boost::offset_separator f1(offsets1, offsets1 + 2, false);
									for (; aktPos2 < cat6k.npos;)
									{
										aktPos2 = cat6k.find("\n", aktPos1 + 1);
										cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
										aktPos1 = aktPos2;
										if (cat6kZeile.size() > 35)
										{
											int i = 0;
											boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f1);
											for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
											{
												switch (i)
												{
												case 1:
													modulSN.push(*beg);
													break;
												default:
													break;
												}
												i++;
											}
										}
									}

									modPos2 = ganzeDatei.find("Mod ", modPos2 + 4);
									if (modPos2 == ganzeDatei.npos)
									{
										modPos2 = pos3;			// show c7200
									}
									modPos2 -= 2;
									modPos1 = ganzeDatei.rfind("---", modPos2)+4;
									aktPos1 = -1;
									aktPos2 = 0;
									cat6k = ganzeDatei.substr(modPos1, modPos2 - modPos1);
									cat6kZeile = "";
									int offsets3[] = {43, 4};
									boost::offset_separator f3(offsets3, offsets3 + 2, false);
									for (; aktPos2 < cat6k.npos;)
									{
										aktPos2 = cat6k.find("\n", aktPos1 + 1);
										cat6kZeile = cat6k.substr(aktPos1 + 1, aktPos2 - aktPos1);
										aktPos1 = aktPos2;
										if (cat6kZeile.size() > 45)
										{
											int i = 0;
											boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f3);
											for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
											{
												switch (i)
												{
												case 1:
													hwRevision.push(*beg);
													break;
												default:
													break;
												}
												i++;
											}
										}
									}

									// ????? Was habe ich mir dabei gedacht?
									//bool oldCatOS = true;
									//if (oldCatOS)
									//{

									//}
								}
								if ((modPos2 = ganzeDatei.find("Mod Sub-Type", pos2)) < ganzeDatei.npos)
								{
									modPos1 = ganzeDatei.find("---", modPos2);
									modPos1 = ganzeDatei.find_first_of("\r\n", modPos1);
									modPos1 = ganzeDatei.find_first_not_of("\r\n", modPos1);
									modPos2 = ganzeDatei.rfind("\n", pos3);
									aktPos1 = -1;
									aktPos2 = 0;
									cat6k = ganzeDatei.substr(modPos1, modPos2-modPos1);

									int offsets2[5];
									// Bei den Submodulen gibt es Unterschiede zwischen alten CatOS Versionen und neuen
									size_t lpos = cat6k.find_first_of("\r\n", aktPos1 + 1);
									if (lpos < 41)
									{
										offsets2[0] = 4;
										offsets2[1] = 9;
										offsets2[2] = 10;
										offsets2[3] = 11;
										offsets2[4] = 3;
									}
									else
									{
										offsets2[0] = 4;
										offsets2[1] = 24;
										offsets2[2] = 20;
										offsets2[3] = 12;
										offsets2[4] = 3;
									}
									cat6kZeile = "";
									boost::offset_separator f2(offsets2, offsets2 + 5, false);
									for (; aktPos2 < cat6k.npos;)
									{
										aktPos2 = cat6k.find_first_of("\r\n", aktPos1 + 1);
										cat6kZeile = cat6k.substr(aktPos1+1, aktPos2 - aktPos1);
										aktPos1 = aktPos2;
										if (cat6kZeile.size() > 35)
										{
											int i = 0;
											boost::tokenizer<boost::offset_separator> tok(cat6kZeile,f2);
											for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
											{
												switch (i)
												{
												case 0:
													{
														std::string mod = *beg + " (Submodule)";
														modulNr.push(mod);
														break;
													}
												case 1:
													modulBesch.push(*beg);
													break;
												case 2:
													modulBez.push(*beg);
													break;
												case 3:
													modulSN.push(*beg);
													break;
												case 4:
													hwRevision.push(*beg);
												default:
													break;
												}
												i++;
											}
										}
									}
								}
							}
						}

						// SHOW SYSTEM
						//////////////////////////////////////////////////////////////////////////

						if (pos7 != ganzeDatei.npos)
						{
							// Power Supply
							aktPos1 = ganzeDatei.find("PS1-Type", pos7);
							aktPos1 = ganzeDatei.find("---", aktPos1);
							aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);
							aktPos1 = ganzeDatei.find_first_not_of("\r\n", aktPos1);

							aktPos2 = ganzeDatei.find("  ", aktPos1);
							std::string ps1 = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
							if (ps1.find("none") == ps1.npos)
							{
								modulNr.push("PS1");
								modulBesch.push("");
								modulBez.push(ps1);
								modulSN.push(psSN[0]);
								hwRevision.push("");
							}

							aktPos1 = ganzeDatei.find_first_not_of(" ", aktPos2);
							aktPos2 = ganzeDatei.find("  ", aktPos1);
							std::string ps2 = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
							if (ps2.find("none") == ps2.npos)
							{
								modulBez.push(ps2);
								modulNr.push("PS2");
								modulBesch.push("");
								modulSN.push(psSN[1]);
								hwRevision.push("");
							}

							// Hostname
							aktPos1 = ganzeDatei.find("System Name", pos7);
							aktPos1 = ganzeDatei.find("--", aktPos1);
							aktPos1 = ganzeDatei.find_first_of("\r\n", aktPos1);
							aktPos1 = ganzeDatei.find_first_not_of("\r\n", aktPos1);
							aktPos2 = ganzeDatei.find(" ", aktPos1);
							hostname = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						}
					}
					else if (nxosbox)
					{
						// NX OS
						//////////////////////////////////////////////////////////////////////////
						// show version
						//////////////////////////////////////////////////////////////////////////
						aktPos1 = 0;
						aktPos2 = 0;
						
						aktPos1 = ganzeDatei.find("system:");
						if (aktPos1 < pos1)
						{
							aktPos1 += 19;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							iosversion = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						}
						aktPos1 = ganzeDatei.find("system image file is:", aktPos2);
						if (aktPos1 < pos1)
						{
							aktPos1 += 25;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							version = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						}
						aktPos1 = ganzeDatei.find("with ", aktPos2);
						if (aktPos1 < pos1)
						{
							aktPos1 += 5;
							aktPos2 = ganzeDatei.find(" ", aktPos1);
							aktPos2 = ganzeDatei.find(" ", aktPos2+1);
							RAM = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						}
						aktPos1 = ganzeDatei.find("Device name: ", aktPos2);
						if (aktPos1 < pos1)
						{
							aktPos1 += 13;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							hostname = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						}

						// show snmp
						//////////////////////////////////////////////////////////////////////////
						aktPos1 = ganzeDatei.find("sys location: ", pos1);
						if (aktPos1 < pos2)
						{
							aktPos1 += 14;
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							standort = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						}

						// show invent
						//////////////////////////////////////////////////////////////////////////
						if((pos6 - pos5 > 300))
						{
							if (chassisSN == "")
							{
								if ((aktPos1 = ganzeDatei.find("hassis", pos5)) != ganzeDatei.npos)
								{
									aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
									aktPos2 = ganzeDatei.find("\"", aktPos1);
									beschreibung = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

									aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
									aktPos2 = ganzeDatei.find(" ", aktPos1);
									deviceType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
								
									aktPos1 = ganzeDatei.find("SN: ", aktPos1) + 4;
									aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
									chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
								}
							}
							
							bool ed = true;		// erster Durchlauf wird ausgelassen, da im ersten Eitrag das Chassis steht
							aktPos2 = pos5;
							while (1)
							{
								if ((aktPos1 = ganzeDatei.find("NAME", aktPos2)) != ganzeDatei.npos)
								{
									if (ed)
									{
										ed = false;
										aktPos2 = aktPos1 + 10;
										continue;
									}
									aktPos1 += 7;

									aktPos2 = ganzeDatei.find("\"", aktPos1);
									std::string modNr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
									modulNr.push(modNr);
									if (!tempModNr.empty())
									{
										if (modNr == tempModNr.front())
										{
											modulRouteMem.push(tempMem.front());
											tempMem.pop();
											tempModNr.pop();
										}
										else
										{
											modulRouteMem.push("");
										}
									}
									else
									{
										modulRouteMem.push("");
									}

									aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
									aktPos2 = ganzeDatei.find("\"", aktPos1);
									size_t aktPos3 = ganzeDatei.rfind("Rev. ", aktPos2);
									if ((aktPos3 < aktPos1) || (aktPos3 == ganzeDatei.npos))
									{
										modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
										hwRevision.push("");
									}
									else
									{
										aktPos3 += 5;
										modulBesch.push(ganzeDatei.substr(aktPos1, aktPos3-aktPos1));
										hwRevision.push(ganzeDatei.substr(aktPos3, aktPos2-aktPos3));
									}



									aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
									aktPos2 = ganzeDatei.find(" ", aktPos1);
									modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

									aktPos1 = ganzeDatei.find("SN", aktPos2) + 4;
									aktPos2 = ganzeDatei.find_first_of(" \r\n", aktPos1);
									modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));
								}
								else
								{
									break;
								}

							}
						}
					}
					else if (pixos)
					{
						// PIX, ASA, FWSM
						//////////////////////////////////////////////////////////////////////////
						// show version
						//////////////////////////////////////////////////////////////////////////
						aktPos1 = 0;
						aktPos2 = 0;
						std::string tempflash = "";

						int revZaehler = 0;		// zum Zählen, ob genug HW Revisions ausgelesen wurden

						standort = "";
						if ((aktPos1 = ganzeDatei.find("Version ") + 8) < pos1)
						{
							aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
							iosversion = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
							version = iosversion;
						}
						if ((aktPos1 = ganzeDatei.find(" up ", aktPos2) + 4) < pos1)
						{
							aktPos1 = ganzeDatei.rfind("\n", aktPos1) + 1;
							aktPos2 = ganzeDatei.find(" ", aktPos1);
							hostname = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
						}
						aktPos1 = ganzeDatei.find("Hardware:   ", aktPos2) + 12;
						aktPos2 = ganzeDatei.find(",", aktPos1);
						deviceType = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = aktPos2 + 2;
						aktPos2 = ganzeDatei.find("RAM", aktPos1) -1;
						RAM = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = ganzeDatei.find("Flash", aktPos2);
						aktPos1 = ganzeDatei.find(",", aktPos1) + 2;
						aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
						tempflash = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						aktPos1 = ganzeDatei.find("license.", aktPos2);
						if (aktPos1 != ganzeDatei.npos)
						{
							aktPos1--;
							aktPos2 = ganzeDatei.rfind("has a", aktPos1) + 4;
							aktPos2 = ganzeDatei.find(" ", aktPos2) + 1;
							lic = ganzeDatei.substr(aktPos2, aktPos1-aktPos2);
						}
						// LIC INFO PARSER EINFÜGEN

						aktPos1 = ganzeDatei.find("Serial Number: ", aktPos2) + 15;
						aktPos2 = ganzeDatei.find_first_of("\r\n ", aktPos1);
						chassisSN = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);

						// show module
						//////////////////////////////////////////////////////////////////////////

						if (!nurChassis)
						{
							std::string asaMod = "";
							std::string asaModZeile = "";
							if (pos3-pos2 > 300)
							{
								size_t modpos1 = 0;
								size_t modpos2 = 0;

								modpos1 = ganzeDatei.find("Mod MAC Address", pos2);
								modpos2 = ganzeDatei.find("Mod", modpos1+10);

								modpos1 = ganzeDatei.rfind("---", modpos2) + 4;
								asaMod = ganzeDatei.substr(modpos1, modpos2-modpos1);
								aktPos1 = -1;
								aktPos2 = 0;
								int offsets3[] = {38, 4};
								boost::offset_separator f3(offsets3, offsets3 + 2, false);
								for (; aktPos2 < asaMod.npos;)
								{
									aktPos2 = asaMod.find("\n", aktPos1 + 1);
									asaModZeile = asaMod.substr(aktPos1 + 1, aktPos2 - aktPos1);
									aktPos1 = aktPos2;
									if (asaModZeile.size() > 45)
									{
										int i = 0;
										boost::tokenizer<boost::offset_separator> tok(asaModZeile,f3);
										for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
										{
											switch (i)
											{
											case 1:
												hwRevision.push(*beg);
												revZaehler++;
												break;
											default:
												break;
											}
											i++;
										}
									}
								}
							}
							else
							{
								hwRevision.push("");
							}


							// show invent
							//////////////////////////////////////////////////////////////////////////
							if(pos6 - pos5 > 300)
							{
								bool ed = true;		// erster Durchlauf wird ausgelassen, da im ersten Eitrag das Chassis steht
								aktPos2 = pos5;
								while (1)
								{
									if ((aktPos1 = ganzeDatei.find("Name", aktPos2)) != ganzeDatei.npos)
									{
										if (ed)
										{
											ed = false;
											aktPos2 = aktPos1 + 10;
											revZaehler--;
											continue;
										}
										aktPos1 += 7;
										aktPos2 = ganzeDatei.find("\"", aktPos1);
										std::string modNr = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
										modulNr.push(modNr);

										aktPos1 = ganzeDatei.find("DESCR", aktPos2) + 8;
										aktPos2 = ganzeDatei.find("\"", aktPos1);
										modulBesch.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

										aktPos1 = ganzeDatei.find("PID", aktPos2) + 5;
										aktPos2 = ganzeDatei.find(" ", aktPos1);
										modulBez.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

										aktPos1 = ganzeDatei.find("SN", aktPos2) + 4;
										aktPos2 = ganzeDatei.find_first_of("\r\n", aktPos1);
										modulSN.push(ganzeDatei.substr(aktPos1, aktPos2-aktPos1));

										if (revZaehler < 1)
										{
											hwRevision.push("");
										}
										revZaehler--;
									}
									else
									{
										break;
									}

								}
							}

							// show file system
							//////////////////////////////////////////////////////////////////////////
							if (pos8 != ganzeDatei.npos)
							{
								size_t flashPos1 = ganzeDatei.find("Prefixes", pos8);
								if (flashPos1 != ganzeDatei.npos)
								{
									size_t flashPos2 = ganzeDatei.rfind(":", ganzeDatei.npos) + 1;
									if ((flashPos2 - flashPos1) < 3000)
									{
										filesystem = true;
									}
								}
								else
								{
									flash = tempflash;
								}
							}
							else
							{
								flash = tempflash;
							}
						}

					}
					else
					{
						dbgA = "3403: No valid file!\n";
						schreibeLog(WkLog::WkLog_NORMALTYPE, dbgA, "3403", WkLog::WkLog_ROT);
						returnValue = 1;
						continue;
						// nix -> Fehlerausgabe
					}

					// show file system für alle
					//////////////////////////////////////////////////////////////////////////
					if (filesystem)
					{
						size_t flashPos1 = ganzeDatei.find("Prefixes", pos8);
						flashPos1 = ganzeDatei.find(" ", flashPos1);

						size_t flashPos2 = ganzeDatei.rfind(":", ganzeDatei.npos) + 1;

						size_t prefPos1 = 0;
						size_t prefPos2 = 0;
						std::string tempflash = "";
						std::string temptemp = "";
						bool naechsteZeile = false;

						aktPos1 = -1;
						aktPos2 = 0;
						std::string speicher = ganzeDatei.substr(flashPos1, flashPos2 - flashPos1);
						std::string speicherZeile = "";
						int offsets[] = {12, 12, 20};
						boost::offset_separator f(offsets, offsets + 3, false);
						for (; aktPos2 < speicher.npos;)
						{
							aktPos2 = speicher.find_first_of("\r\n", aktPos1 + 1);
							speicherZeile = speicher.substr(aktPos1 + 1, aktPos2 - aktPos1);
							aktPos1 = aktPos2;
							if (speicherZeile.size() > 45)
							{
								int i = 0;
								boost::tokenizer<boost::offset_separator> tok(speicherZeile,f);
								for(boost::tokenizer<boost::offset_separator>::iterator beg=tok.begin(); beg!=tok.end();++beg)
								{
									switch (i)
									{
									case 0:			// Size
										temptemp = *beg;
										while ((temptemp[0] == ' ') || (temptemp[0] == '*'))
										{
											temptemp.erase(0, 1);
										}

										if (temptemp[0] == '-')
										{
											naechsteZeile = true;
										}
										else if (temptemp[0] == '0')
										{
											naechsteZeile = true;
										}
										tempflash += temptemp;
										tempflash += ";";
										break;
									case 1:			// Free
										temptemp = *beg;
										while (temptemp[0] == ' ')
										{
											temptemp.erase(0, 1);
										}

										if (temptemp[0] == '-')
										{
											naechsteZeile = true;
										}
										else if (temptemp[0] == '0')
										{
											naechsteZeile = true;
										}
										tempflash += temptemp;
										tempflash += ";";
										break;
									case 2:			// Prefix - Type
										if (!naechsteZeile)
										{
											prefPos2 = speicherZeile.find(":")+1;
											prefPos1 = speicherZeile.rfind(" ", prefPos2-2);
											flash += speicherZeile.substr(prefPos1, prefPos2-prefPos1) + ";" + tempflash;
										}
										naechsteZeile = false;
										tempflash = "";
										break;
									default:
										break;
									}
									i++;
								}
							}
						}
					}
				}


				
				// File Ausgabe
				//////////////////////////////////////////////////////////////////////////
				// Hostname   Location   Devicetype   S/N   Memory   bootfile

				(*this.*spalte1Aus)(hostname + ";");
				(*this.*spalte2Aus)(standort + ";");
				(*this.*spalte3Aus)(deviceType + ";");
				(*this.*spalte4Aus)(beschreibung + ";");
				(*this.*spalte5Aus)(chassisSN + ";");
				if (!hwRevision.empty())
				{
					(*this.*spalte6Aus)(hwRevision.front() + ";");
					hwRevision.pop();
				}
				(*this.*spalte7Aus)(RAM + ";");
				(*this.*spalte8Aus)(version + ";");
				(*this.*spalte9Aus)(iosversion + ";");
				(*this.*spalte10Aus)(lic + ";");
				(*this.*spalte11Aus)(flash + ";");
				ausgabe << "\n";


				if (!nurChassis)
				{
					if (npeSN != "")
					{
						(*this.*spalte1Aus)(hostname + ";");
						(*this.*spalte2Aus)(";" + slc);
						(*this.*spalte3Aus)(prozessor + ";");
						(*this.*spalte4Aus)(";");
						(*this.*spalte5Aus)(npeSN + ";");
						(*this.*spalte6Aus)(hwRevision.front());
						hwRevision.pop();
						(*this.*spalte7Aus)(";");
						(*this.*spalte8Aus)(";");
						(*this.*spalte9Aus)(";");
						(*this.*spalte10Aus)(lic + ";");
						(*this.*spalte11Aus)(";");
						ausgabe << "\n";
					}
					while (!modulNr.empty())
					{
						(*this.*spalte1Aus)(hostname + ";");
						(*this.*spalte2Aus)(modulNr.front() + ";" + slc);
						modulNr.pop();

						(*this.*spalte3Aus)(modulBez.front() + ";");
						modulBez.pop();

						(*this.*spalte4Aus)(modulBesch.front() + ";");
						modulBesch.pop();

						(*this.*spalte5Aus)(modulSN.front() + ";");
						modulSN.pop();

						if (!hwRevision.empty())
						{
							(*this.*spalte6Aus)(hwRevision.front() + ";");
							hwRevision.pop();
						}
						else
						{
							(*this.*spalte6Aus)(";");
						}


						if (!modulRouteMem.empty())
						{
							if (modulRouteMem.front() != "")
							{
								(*this.*spalte7Aus)(modulRouteMem.front());
							}
							modulRouteMem.pop();
							(*this.*spalte7Aus)(";");

							if (!modulPacketMem.empty())
							{
								if (modulPacketMem.front() != "")
								{
									(*this.*spalte7Aus)("\n");								
									(*this.*spalte1Aus)(";");
									(*this.*spalte2Aus)(";" + slc);
									(*this.*spalte3Aus)(";");
									(*this.*spalte4Aus)(";");
									(*this.*spalte5Aus)(";");
									(*this.*spalte6Aus)(";");
									(*this.*spalte7Aus)(modulPacketMem.front() + ";");
									(*this.*spalte8Aus)(";");
									(*this.*spalte9Aus)(";");
									(*this.*spalte10Aus)(";");
									(*this.*spalte11Aus)(";");
								}
								modulPacketMem.pop();
							}
						}
						else
						{
							(*this.*spalte7Aus)(";");
							(*this.*spalte8Aus)(";");
							(*this.*spalte9Aus)(";");
							(*this.*spalte10Aus)(";");
							(*this.*spalte11Aus)(";");
						}

						if (!modulVer.empty())
						{
							(*this.*spalte8Aus)(modulVer.front() + ";");
							(*this.*spalte9Aus)(";");
							(*this.*spalte10Aus)(";");
							(*this.*spalte11Aus)(";");
							modulVer.pop();
							ausgabe << "\n";
						}
						else
						{
							(*this.*spalte8Aus)(";");
							(*this.*spalte9Aus)(";");
							(*this.*spalte10Aus)(";");
							(*this.*spalte11Aus)(";");
							ausgabe << "\n";
						}
					}
				}
			}
		}
	}
	ausgabe.close();
	return returnValue;
}


void UTAParse::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
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
void UTAParse::schreibeLogfile(std::string logInput)
{
	//logAusgabeDatei << logInput;
}


bool UTAParse::sdir(fs::path pfad)
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


void UTAParse::varsInit()
{
	catos = false;					// CatOS Daten?
	dslam = false;					// Handelt es sich bei den eingelesenen Date um einen DSLAM?
	pixos = false;					// PIX, ASA, FWSM Daten?
	iosbox = false;					// Gerät mit IOS?
	nxosbox = false;				// Gerät mit NX OS?
	ipphone = false;				// IP Phone?
	filesystem = false;				// Filesystem?

	hostname = "";					// Hostname
	version = "";					// IOS 
	deviceType = "";				// Gerätetype
	prozessor = "";					// Prozessor Type
	RAM = "";						// RAM 
	chassisSN = "";					// S/N vom Chassis
	npeSN = "";						// S/N von der NPE
	npeBez = "";					// NPE Type
	standort = "";					// SNMP Location
	iosversion = "";				// IOS Versionsnummer
	flash = "";						// Flashgröße
	beschreibung = "";				// Beschreibung

	while (!modulRouteMem.empty())
	{
		modulRouteMem.pop();
	}
	while (!modulPacketMem.empty())
	{
		modulPacketMem.pop();
	}
	while (!tempModNr.empty())
	{
		tempModNr.pop();
	}
	while (!tempMem.empty())
	{
		tempMem.pop();
	}
	while (!modulNr.empty())
	{
		modulNr.pop();
	}
	while (!modulBez.empty())
	{
		modulBez.pop();
	}
	while (!modulSN.empty())
	{
		modulSN.pop();
	}
	while (!hwRevision.empty())
	{
		hwRevision.pop();
	}
	while (!modulBesch.empty())
	{
		modulBesch.pop();
	}
	while (!modulVer.empty())
	{
		modulVer.pop();
	}

	pos1 = pos2 = pos3 = pos4 = pos5 = pos6 = pos7 = pos8 = 0;
	pos1dsl = pos2dsl = pos3dsl = 0;
	aktPos1 = aktPos2 = 0;
	pos1iph = 0;
}


void UTAParse::testPhones(std::string ganzeDatei)
{
	size_t htmlPos = ganzeDatei.find("MAC");
	if (htmlPos != ganzeDatei.npos)
	{
		std::string htmlParse = ganzeDatei.substr(htmlPos);

		size_t aktPos1 = 0;
		size_t aktPos2 = 0;

		// Beschreibung
		if ((aktPos1 = ganzeDatei.find("Cisco IP Phone")) != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("<(", aktPos1);
			beschreibung = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		else if ((aktPos1 = ganzeDatei.find("Cisco Unified")) != ganzeDatei.npos)
		{
			aktPos2 = ganzeDatei.find_first_of("<(", aktPos1);
			beschreibung = ganzeDatei.substr(aktPos1, aktPos2-aktPos1);
		}
		else
		{
			beschreibung = "";
		}

		// alles andere
		aktPos1 = aktPos2 = 0;
		
		// alle html Tags löschen, so dass nur noch <>;& und die Texte da sind
		while (1)
		{
			aktPos1 = htmlParse.find_first_of("<&;", aktPos1);
			if (aktPos1 == htmlParse.npos)
			{
				break;
			}
			aktPos1++;
			aktPos2 = htmlParse.find_first_of(">;", aktPos1);
			htmlParse = htmlParse.erase(aktPos1, aktPos2-aktPos1);
		}
		
		// MAC Adresse
		aktPos1 = htmlParse.find_first_of("<>;&");
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		std::string macAdresse = htmlParse.substr(aktPos1, aktPos2-aktPos1);

		// Hostname
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos2);
		aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		hostname = htmlParse.substr(aktPos1, aktPos2-aktPos1);

		// Telefonnummer
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos2);
		aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		standort = htmlParse.substr(aktPos1, aktPos2-aktPos1);

		// Version
		aktPos1 = htmlParse.find("ersion", aktPos2) + 7;
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		iosversion = htmlParse.substr(aktPos1, aktPos2-aktPos1);

		// HW Revision
		aktPos1 = htmlParse.find("evision", aktPos2) + 8;
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		hwRevision.push(htmlParse.substr(aktPos1, aktPos2-aktPos1));

		// Seriennummer
		aktPos1 = htmlParse.find("eri") + 4;
		aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		chassisSN = htmlParse.substr(aktPos1, aktPos2-aktPos1);

		// Type
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t ", aktPos2);
		aktPos1 = htmlParse.find_first_of("<>;&", aktPos1);
		aktPos1 = htmlParse.find_first_not_of("<>;&\r\n\t", aktPos1);
		aktPos2 = htmlParse.find_first_of("<>;&\r\n\t ", aktPos1);
		deviceType = htmlParse.substr(aktPos1, aktPos2-aktPos1);

	}
	else
	{
		std::string errstring = "3501: Device not yet supported. Please send the file to wktools@spoerr.org " + dateiname;
		errstring += "\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, errstring, "3501", WkLog::WkLog_ROT);
	}

}


void UTAParse::testType(std::string ganzeDatei)
{

}

void UTAParse::schreibeJa(std::string text)
{
	ausgabe << text;
}


void UTAParse::schreibeNein(std::string nix)
{

}