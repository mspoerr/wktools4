// Class Verbinder:
// Klasse für alle Client Verbindungen und für die Auswertung der Antworten von den Geräten
//
//////////////////////////////////////////////////////////////////////////
// Änderungen
//////////////////////////////////////////////////////////////////////////
// 14.07.2006 - mspoerr:
//		- alle "\r" durch "\r\n" ersetzt
// bis 20.8.2006 - mspoerr:
//		- diverse Änderungen -> siehe todo
// 20.08.2006 - mspoerr:
//		- Bei ssh Verbindungen wird \n weggefiltert
// 22.08.2006 - mspoerr:
//		- Einführen von den debug Variablen
//		- If Schleifen zum Überprüfen der Debug Variablen dazugegeben
// 23.08.2006 - mspoerr:
//		- kleine Ausbesserungen bei den Debug Abfragen
// 24.08.2006 - mspoerr:
//		- nichtsString eingeführt und in initVerbinderVars() auf "" gesetzt
//		- nichtsString in outBufZusammensteller füttern wenn bufTest = NICHTS 
//		- loginTest umgebaut, damit im Fall der nichtsString bei der Suche nach User/PW miteinbezogen werden kann
// 25.08.2006 - mspoerr:
//		- Länge bei Prüfen auf Login String auf 8 gesetzt (früher 6), da : und SPACE nicht berücksichtigt wurden
//		- Hinzufügen der Bufferlänge bei der Debugausgabe
//		- nameAuslesen Funktionen geändert: statt das letzte "\x0A" oder "\x0D" zu suchen, wird jetzt nach "\r\n" gesucht.
//		- debugBufTestDetail eingeführt
// 12.09.2006 - mspoerr:
//		- NICHTSaendertBufTest wird ab jetzt beim ersten Aufruf der Schleife im Verbinder wieder auf false gesetzt
//		- bufTestAenderung wird jetzt im outBufZusammensteller abgefragt, damit NICHTSaendertBufTest nicht sofort wieder auf true gesetzt wird
// 12.09.2006 - mspoerr:
//		- Bei ssh war das falsche Default Port eingetragen -> geändert auf 22
// 15.09.2006 - mspoerr:
//		- NICHTSaendertBufTest wird nun auch bei COMMAND im outBufZusammensteller gesetzt
// 19.09.2006 - mspoerr:
//		- !PARAGRAPH wurde eingeführt bei outBufZusammensteller
// 23.11.2006 - mspoerr:
//		- PARAGRAPH Feature vervollständigt
// 25.01.2007 - mspoerr:
//		- Debug für show Ausgabe eingeführt
//		- Fehlerkontrolle für show Ausgabe hinzugefügt
//		- CatOSnameAuslesen angepasst, damit ein CR am Anfang auch wirklich weggelöscht wird.
// 10.02.2007 - mspoerr:
//		- loginBufTest: Erweiterung um den ":" sofort als letztes Zeichen, da bei User/Pass Abfragen manchmal kein abschließendes SPACE kommt
// 16.02.2007 - mspoerr:
//		- loginBufTest: Erweiterung um "]", damit im Falle von notwendigen Bestätigungen ein "yes" geschickt werden kann
// 08.05.2007 - mspoerr:
//		- outBufZusammensteller: Erweiterung um !LOG Tag
//		- !LOG Tag hinzufügen
//		- logIt Variable hinzugefügt und ursprüngliches "show" finden umgebaut
//		- Check, ob Config File leer hinzugefügt, wenn !LOG Tag gefunden wird
// 12.05.2007 - mspoerr:
//		- Pixnamenauslesen: eBuf.substr wurde geändert, da der erste Buchstabe verschluckt wurde
//		- Pixnamenauslesen: die Debug Ausgabe wurde um den eBuf erweitert
//		- bool fertig eingeführt: class_verbinder.h; Konstruktor; initVerbinderVars;
//		- outbufZusammensteller: wenn das letzte Command gesendet wurde, wird eine neue Variable gesetzt:
//        bool fertig
// 26.05.2007 - mspoerr:
//		- Einführen von HTTP als Verbindungsmethode
//		  - httpInit() Funktion hinzugefügt
//		  - httpVerbindung() Funktion hinzugefügt
//		  - enum stats um HTTP erweitert
// 27.05.2007 - mspoerr
//		- show Append Option hinzugefügt
//		  - showAppend Variable eingeführt
//		  - setShowAppend() eingeführt
//		  - outBufZusammensteller(): showAppend aktiviert
// 30.05.2007 - mspoerr
//		- httpErrorCodeCheck Funktion hinzugefügt
//		  - httpVerbindung umgebaut, so dass neue Funktion verwendet wird
// 16.06.2007 - mspoerr
//		- httpVerbindung um DebugFenster Ausgaben erweitert, damit bei Fehlern die richtige Meldung ausgegeben wird
// 09.01.2008 - mspoerr
//      - outBufZusammensteller(): 
//        - case NICHTS: if Schleife um (MODUS == WEITER) erweitert
//        - case COMMAND: if Schleife am Ende (Check, ob Config leer) um (MODUS == WEITER) erweitert
// 10.01.2008 - mspoerr
//      - outBufZusammensteller():
//        - case PAGERLINE: config.push_front("terminal pager lines 0"); -> \r\n am Ende entfernt
//        - Änderungen vom 09.01.2008 rückgängig gemacht
// 30.04.2008 - mspoerr
//		- Einführen von intfConf() und inventoryConf()

#include "stdwx.h"

#include <iostream>
#include <string>
#include <deque>
#include <fstream>

using namespace std;

#include "class_verbinder.h"


// Konstruktor:
//*************
// Bei Initialisierung eines neuen Verbinders muss ein log-File angegeben werden. Dieses log-File wird dann geöffnet,
// damit alle Verbinder Funktionen in ein log File schreiben können, ohne es jedesmal öffnen und schließen zu müssen.
// Es werden alle wichtigen Variablen initialisert.
Verbinder::Verbinder(WkLog *logA, Wke *wkV, asio::io_service& io_service) 
	: ioserv(io_service)
	, verbindungsSock(io_service)
	, serPort(io_service)
{
	debugSenden = false;
	debugEmpfangen = false;
	debugBufTester = false;
	debugFunctionCall = false;
	debugbufTestDetail = false;

	asgbe = logA;
	wkVerbinder = wkV;
		
	MODUS = FEHLER;
	STATUS = FEHLER;
	
	keineDatei = false;
	ersterDurchlauf = true;
	showGesendet = false;
	erfolgreich = false;
	debug = true;
	manuell = false;
	configMode = false;
	hostnameAuslesen = false;
	execKontrolle = false;
	warteRaute = true;
	teste = true;
	cancelVerbindung = false;
	vne = false;
	tscAuthe = false;
	tscNoAuthe = false;
	fertig = false;

	NICHTSaendertBufTest = 0;
	bufTestAenderung = false;
	
	outBuf = new char[BUFFER_SIZE];
	outBufZaehler = empfangsOffset = 0;

	shString = "";

	nichtsString = "";

	hostname = "";
	hostnameAlt = "";

	ulez = ulezmax = 0;
	schonAuthe = false;

	sshVersion = 2;

	nullEaterEnable = false;

	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - CONSTRUCTOR>\n", DEBUGFEHLER);
	}
// 	troubleshooting.rdbuf()->open("d:\\Cisco\\wktoolsTroubleshooting.txt", ios_base::out | ios_base::app);
// 	trcounter = 0;

}

// Destruktor:
//************
// Freimachen des Speichers, der vom Konstruktor allokiert wurde und Schließen des Log Files.
Verbinder::~Verbinder()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - DESTRUCTOR>\n", DEBUGFEHLER);
	}
}

// Definition von BUFFER_SIZE
const uint Verbinder::BUFFER_SIZE = 1025;

// Definition der nötigen Funktionszeiger
uint(Verbinder::*bufTester)(string, uint);
void(Verbinder::*nameAusleser)(string);


// setSockEinstellungen:
//**********************
// Funktion zum Auflösen der IP Adressen
bool Verbinder::setSockEinstellungen(string ipAdr, string port)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setSockEinstellungen>\n", DEBUGFEHLER);
	}
	
	tcp::resolver resolver(ioserv);
	tcp::resolver::query query(ipAdr, port);
	endpoint_iterator = resolver.resolve(query);

	return true;
}


// telnetOptionen:
//****************
// Funktion zum Abarbeiten der Telnet Funktionen. Alle Funktionen beginnen mit 0xFF und dann kommen
// Subparameter. Siehe RFC854 und ff
// Es werden hier jedoch nur wenige Optionen ausgewertet. Für alle anderen wird keine Antwort geschickt.
// Für die Optionen WILL, DO, WON'T und DON'T wird außer für wenige Ausnahmen immer eine 
// Verneinung zurückgeschickt.
// Abgeschlossen wird das zu sendende Paket mit 0xDD, da 0x00 in den Optionen vorkommen kann.
uint Verbinder::telnetOptionen(const char *buf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - telnetOptionen>\n", DEBUGFEHLER);
	}
	unsigned int bla = strlen(buf);
	for (unsigned int i = 0; i < bla; i++)
	{
		if (buf[i] == 0xFF)
		{
			switch (buf[i+1])
			{
			case 0xF0:			// SE - End of Subnegotiation parameters
				i++;
				break;
			case 0xF1:			// NOP
				i++;
				break;
			case 0xF2:			// Data Mark
				i++;
				break;
			case 0xF3:			// BREAK
				i++;
				break;
			case 0xF4:			// Interrupt Process
				i++;
				break;
			case 0xF5:			// Abort Output
				i++;
				break;
			case 0xF6:			// Are you there
				i++;
				break;
			case 0xF7:			// Erase Character
				i++;
				break;
			case 0xF8:			// Erase Line
				i++;
				break;
			case 0xF9:			// Go ahead
				i++;
				break;
			case 0xFA:			// SB - Beginning of Subnegotiation
				{
					int j = i;
					for (; (buf[j] != 0xF0); j++)
					{
					}
					i = j;
				}
				break;
			case 0xFB:			// WILL
				outBuf[outBufZaehler] = 0xFF;
				switch (buf[i+2])
				{
				case 0x01:		// OPTION: Echo
					{
						const char echoOption[] = "\xFE\x01";
						memcpy(outBuf + outBufZaehler + 1, echoOption, 2);
						break;
					}
                case 0x03:		// OPTION: Suppress go ahead
					{
						const char suppressGoAheadOption[] = "\xFD\x03";
						memcpy(outBuf + outBufZaehler + 1, suppressGoAheadOption, 2);
						break;
					}
				default:
                    outBuf[outBufZaehler+1] = 0xFE;
					outBuf[outBufZaehler+2] = buf[i+2];
					break;
				}
				outBufZaehler += 3;
				i=i+2;
				break;
			case 0xFC:			// WON'T
				i=i+2;
				break;
			case 0xFD:			// DO
				outBuf[outBufZaehler] = 0xFF;
				switch (buf[i+2])
				{
				case 0x1F:		// OPTION: Window Size
					{
						const char windowSizeOption[] = "\xFB\x1F\xFF\xFA\x1F\x00\x78\x00\x00\xFF\xF0";
						memcpy(outBuf + outBufZaehler + 1, windowSizeOption, 11);
						outBufZaehler += 12;
						i=i+11;
						break;
					}
				default:
                    outBuf[outBufZaehler+1] = 0xFC;
					outBuf[outBufZaehler+2] = buf[i+2];
					outBufZaehler += 3;
					i=i+2;
					break;
				}
				break;
			case 0xFE:			// Don't
				i=i+2;
				break;
			default:
				break;
			}
		}
	}
	outBuf[outBufZaehler] = 0xDD;
	return OPTIONENGEF;
}

// telnetOptionenFinder
//*********************
// Funktion zum Auffinden von Telnet Optionen im Eingangspuffer, in dem sich auch Benutzerdaten befinden
// Falls Telnetoptionen gefunden werden, werden diese auch gleich ausgewertet und von den Nutzdaten entfernt
uint Verbinder::telnetOptionenFinder(char *buf, size_t bufLaenge)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - telnetOptionenFinder>\n", DEBUGFEHLER);
	}
	char *telOpt = new char[bufLaenge];
	char *daten = new char[bufLaenge];
	char *faZeiger;
	size_t faLaenge = 0;
	uint option = 0;
	uint datenZaehler = 0;
	bool telOptGef = false;
	for (uint i = 0; i < bufLaenge; i++, faLaenge++)
	{
		if (buf[i] == 0xFF)
		{
			telOptGef = true;
			switch (buf[i+1])
			{
			case 0xFA:		// Suboptionen Anfang
				faZeiger = &buf[i];
				faLaenge = 0;
				option = bufLaenge;
				break;
			case 0xF0:		// Suboptionen Ende
				memcpy(telOpt, faZeiger, faLaenge + 1);
				*(telOpt + faLaenge + 1);
				option = i;
				break;
			default:
				memcpy(telOpt, &buf[i], 3);
				*(telOpt + 3);
				option = i+3;
				break;
			}
		}
		if ((option - i) <= 0)
		{
			memcpy(daten + datenZaehler, &buf[i], 1);
			datenZaehler++;
		}
	}
	if (telOptGef)
	{
		uint bufTest = telnetOptionen(telOpt);
	}
	memcpy(buf, daten, datenZaehler);
	delete[] telOpt;
	delete[] daten;

	return datenZaehler;
}
		

// IOSbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von IOS Geräten.
uint Verbinder::IOSbufTester(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - IOSbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("<IOSbufTester>", DEBUGFEHLER);
	}

	size_t bufferLaenge = eBuf.length();
	
	if (bufferLaenge)
	{
		
		
		// eBuf auf Fehlermeldung überprüfen: Fehlermeldungen beginnen immer mit CR/LF, außer bei Konsoleverbindungen.
		// Dort wird deshalb bei Verbindungsaufbau "logging synchronous" an den lines konfiguriert.
		// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
		// und daher nicht mehr untersucht werden müssen.
		if ((eBuf[empfangsOffset + 1] == 0x0A) || 0x20)
		{
			size_t prozentPos, endePos = 0;
			string fehlermeldung;

			prozentPos = eBuf.find("%", empfangsOffset);
			if (prozentPos != eBuf.npos)
			{
				endePos = eBuf.find("\x0D\x0A", prozentPos);
				if ((endePos != eBuf.npos))
				{
					fehlermeldung = eBuf.substr(prozentPos, endePos);
					fehlermeldung += zuletztGesendet;

					schreibeLog(fehlermeldung, FEHLER);
					bufStat = FEHLERMELDUNG;			
				}
			}
		}
	

		if (bufStat != FEHLERMELDUNG)
		{
			// * 0x23: # : exec mode bei Cisco. Jedes Mal, wenn dieses Zeichen am Ende des Streams vorkommt,
			//		wird eine Befehlszeile geschickt, und zwar so oft, bis das Ende der zu schickenden
			//		Konfiguration erreicht ist.
			// * default: Falls keines der oben genannten Zeichen vorkommt, wird nichts zum Gegenüber 
			//		geschickt und auf weitere Pakete gewartet.


			switch (eBuf[bufferLaenge-1])
			{
			case 0x23:			// enable und config modes: #
				// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
				switch (iosAnfangsSettings)
				{
				case TLULS:
					bufStat = TLULS;
					iosAnfangsSettings = NICHTS;
					break;
				case LOGGSYNC:
					bufStat = LOGGSYNC;
					iosAnfangsSettings = NICHTS;
					break;
				case TERMLEN:
					bufStat = TERMLEN;
					iosAnfangsSettings = NICHTS;
					break;
				default:
					if (eBuf[bufferLaenge-2] == 0x29)		// Kommt vor der "#" eine ")" vor, dann wird "configMode" gesetzt
					{
						configMode = true;
					}
					else
					{
						configMode = false;
					}
					bufStat = COMMAND;
					break;
				}
				break;
			case 0x20:				// SPACE -> kommt bei SW Upgrade vor
				if (eBuf.rfind("[yes/no]: ") != eBuf.npos)
				{
					bufStat = YES;
				}
				else if (eBuf.rfind("\x5D\x3F\x20") != eBuf.npos)
				{
					bufStat = COMMAND;
				}
				else
				{
					bufStat = NICHTS;
				}
				break;
			case 0x5D:				// ] -> kommt bei Reload, Config Overwrite usw. vor
				if (eBuf.rfind("confirm]") != eBuf.npos)
				{
					bufStat = YES;
				}
				else
				{
					bufStat = NICHTS;
				}
				break;
			case 0x0A:
				if (!warteRaute)
				{
					bufStat = COMMAND;
				}
				else
				{
					bufStat = NICHTS;
				}
				break;
			default:
				bufStat = NICHTS;
				break;
			}		
		}
	}
	else
	{
		bufStat = NICHTS;
	}
	
	if (debugbufTestDetail)
	{
		// Ausgabe: 
		// Feld 1: bufStat
		// Feld 2: STATUS
		// Feld 3: vorletztes Zeichen
		// Feld 4: vor-vorletztes Zeichen
		char bbufff[4];
		string bla9 = itoa(bufStat, bbufff, 10);
		char ebufff[4];
		string bla10 = itoa(STATUS, ebufff, 10);
		string bla11 = "<IOSBufTester><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		if (bufferLaenge)
		{
			bla11 += eBuf[bufferLaenge-1];
		}
		bla11 += "><";
		if (bufferLaenge > 1)
		{
			bla11 += eBuf[bufferLaenge-2];
		}
		bla11 += ">";
		schreibeLog(bla11, DEBUGFEHLER);
	}
	
	return bufStat;
}


// PIXbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von PIXen.
uint Verbinder::PIXbufTester(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - PIXbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("<PIXbufTester>", DEBUGFEHLER);
	}
	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("Type help or '?' for a list of available commands.") != eBuf.npos)
	{
		string fehlermeldung = "Type help or '?' for a list of available commands.";
		fehlermeldung += zuletztGesendet;

		schreibeLog(fehlermeldung, FEHLER);
		bufStat = FEHLERMELDUNG;			
	}
	if (bufStat != FEHLERMELDUNG)
	{
		// * 0x23: # : exec mode bei Cisco. Jedes Mal, wenn dieses Zeichen am Ende des Streams vorkommt,
		//		wird eine Befehlszeile geschickt, und zwar so oft, bis das Ende der zu schickenden
		//		Konfiguration erreicht ist.
		// * default: Falls keines der oben genannten Zeichen vorkommt, wird nichts zum Gegenüber 
		//		geschickt und auf weitere Pakete gewartet.

		size_t bufferLaenge = eBuf.length();

		if (bufferLaenge > 1)
		{
			switch (eBuf[bufferLaenge-2])
			{
			case 0x23:			// enable und config modes: #
				// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
				switch (iosAnfangsSettings)
				{
				case TERMLEN:
					bufStat = PAGERLINE;
					iosAnfangsSettings = NICHTS;
					break;
				default:
					if (eBuf[bufferLaenge-3] == 0x29)		// Kommt vor der "#" eine ")" vor, dann wird "configMode" gesetzt
					{
						configMode = true;
					}
					else
					{
						configMode = false;
					}
					bufStat = COMMAND;
					break;
				}
				break;
			default:
				if (eBuf.find("\r\nLogoff\r\n\r\n") != eBuf.npos)
				{
					bufStat = ENDE;
					configMode = false;
				}
				else
				{
					bufStat = NICHTS;
				}
				break;
			}		
		}
		else
		{
			bufStat = NICHTS;
		}
		if (debugbufTestDetail)
		{
			// Ausgabe: 
			// Feld 1: bufStat
			// Feld 2: STATUS
			// Feld 3: vorletztes Zeichen
			// Feld 4: vor-vorletztes Zeichen
			char bbufff[4];
			string bla9 = itoa(bufStat, bbufff, 10);
			char ebufff[4];
			string bla10 = itoa(STATUS, ebufff, 10);
			string bla11 = "<PIXBufTester><" + bla9;
			bla11 += "><";
			bla11 += bla10;
			bla11 += "><";
			if (bufferLaenge > 1)
			{
				bla11 += eBuf[bufferLaenge-2];
			}
			bla11 += "><";
			if (bufferLaenge > 2)
			{
				bla11 += eBuf[bufferLaenge-3];
			}
			bla11 += ">";
			schreibeLog(bla11, DEBUGFEHLER);
		}
	}
	return bufStat;
}


// CATbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von CatOS Geräten.
uint Verbinder::CATbufTester(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - CATbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("<CATbufTester>", DEBUGFEHLER);
	}

	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("\x0D\x0AUnknown Command ") != eBuf.npos)
	{
		string fehlermeldung = "Unknown Command";
		fehlermeldung += zuletztGesendet;

		schreibeLog(fehlermeldung, FEHLER);
		bufStat = FEHLERMELDUNG;			
	}


	if (bufStat != FEHLERMELDUNG)
	{
		// * 0x29: ) : exec mode bei Catalysten. Jedes Mal, wenn dieses Zeichen am Ende des Streams vorkommt,
		//		wird eine Befehlszeile geschickt, und zwar so oft, bis das Ende der zu schickenden
		//		Konfiguration erreicht ist.
		// * default: Falls keines der oben genannten Zeichen vorkommt, wird nichts zum Gegenüber 
		//		geschickt und auf weitere Pakete gewartet.

		size_t bufferLaenge = eBuf.length();

		if (bufferLaenge > 1)
		{
			switch (eBuf[bufferLaenge-2])
			{
			case 0x29:			// enable und config modes: )
				// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
				switch (iosAnfangsSettings)
				{
				case TERMLEN:
					bufStat = SETLENGTH;
					iosAnfangsSettings = NICHTS;
					break;
				default:
					bufStat = COMMAND;
					break;
				}
				break;
			case 0x0D:
				if (!warteRaute)
				{
					bufStat = COMMAND;
				}
				else
				{
					bufStat = NICHTS;
				}
				break;
			default:
				bufStat = NICHTS;
				break;
			}	
		}
		else
		{
			bufStat = NICHTS;
		}
		if (debugbufTestDetail)
		{
			// Ausgabe: 
			// Feld 1: bufStat
			// Feld 2: STATUS
			// Feld 3: vorletztes Zeichen
			// Feld 4: vor-vorletztes Zeichen
			char bbufff[4];
			string bla9 = itoa(bufStat, bbufff, 10);
			char ebufff[4];
			string bla10 = itoa(STATUS, ebufff, 10);
			string bla11 = "<CatBufTester><" + bla9;
			bla11 += "><";
			bla11 += bla10;
			bla11 += "><";
			if (bufferLaenge > 1)
			{
				bla11 += eBuf[bufferLaenge-2];
			}
			bla11 += "><";
			bla11 += ">";
			schreibeLog(bla11, DEBUGFEHLER);
		}
	}
	return bufStat;
}


// UTAbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von der UTA Managmentstation.
uint Verbinder::UTAbufTester(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - UTAbufTester>\n", DEBUGFEHLER);
	}

	if (debugBufTester)
	{
		schreibeLog("<UTAbufTester>", DEBUGFEHLER);
	}
	
	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("command not found") != eBuf.npos)
	{
		string fehlermeldung = "command not found";
		fehlermeldung += zuletztGesendet;

		schreibeLog(fehlermeldung, FEHLER);
		bufStat = FEHLERMELDUNG;			
	}
	if (bufStat != FEHLERMELDUNG)
	{
		// * 0x24: $ : exec mode bei UTA Konsole. Jedes Mal, wenn dieses Zeichen am Ende des Streams vorkommt,
		//		wird eine Befehlszeile geschickt, und zwar so oft, bis das Ende der zu schickenden
		//		Konfiguration erreicht ist.
		// * default: Falls keines der oben genannten Zeichen vorkommt, wird nichts zum Gegenüber 
		//		geschickt und auf weitere Pakete gewartet.

		size_t bufferLaenge = eBuf.length();

		if (bufferLaenge > 1)
		{
			switch (eBuf[bufferLaenge-2])
			{
			case 0x24:			// enable und config modes: $
				// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
				configMode = false;
				bufStat = COMMAND;
				break;
			default:
				bufStat = NICHTS;
				break;
			}	
		}
		else
		{
			bufStat = NICHTS;
		}
		if (debugbufTestDetail)
		{
			// Ausgabe: 
			// Feld 1: bufStat
			// Feld 2: STATUS
			// Feld 3: vorletztes Zeichen
			// Feld 4: vor-vorletztes Zeichen
			char bbufff[4];
			string bla9 = itoa(bufStat, bbufff, 10);
			char ebufff[4];
			string bla10 = itoa(STATUS, ebufff, 10);
			string bla11 = "<UTABufTester><" + bla9;
			bla11 += "><";
			bla11 += bla10;
			bla11 += "><";
			if (bufferLaenge > 1)
			{
				bla11 += eBuf[bufferLaenge-2];
			}
			bla11 += "><";
			bla11 += ">";
			schreibeLog(bla11, DEBUGFEHLER);
		}
	}
	return bufStat;
}



// loginbufTester
//***************
// Funktion zum Auswerten der gesendeten Daten solange der login Vorgang noch nicht abgeschlossen ist,
// und zum Feststellen der Gerätetype. Zur Zeit kann zwischen UTA Managment Station, IOS, CatOS und PIXOS unterschieden werden.
// Achtung bei CatOS! Nur wenn der Prompt folgendermaßen ein ">" als Abschluss hat, funktioniert das Programm.
uint Verbinder::loginBufTester(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - loginBufTester>\n", DEBUGFEHLER);
	}

	if (debugBufTester)
	{
		schreibeLog("<loginBufTester>", DEBUGFEHLER);
	}
	size_t bufferLaenge = eBuf.length();

	if (bufferLaenge > 1)
	{
		switch (eBuf[bufferLaenge-1])
		{
		case 0x20:			// Wenn SPACE, dann überprüfen, welches Zeichen davor kommt
			switch (eBuf[bufferLaenge-2])
			{
			case 0x3A:		// :
				bufStat = loginTest(eBuf, bufStat);
				break;
			case 0x3E:		// >
				if ((eBuf.find('\x00') + 1) < eBuf.size())
				{
					nameAusleser = &Verbinder::PIXnameauslesen;
					if (enPWf == TERMLEN)
					{
						enPWf = PAGERLINE;
					}
				}
				else
				{
					nameAusleser = &Verbinder::CATnameauslesen;
				}
				bufStat = loginModeTest(eBuf, bufStat);
				break;
			case 0x24:		// $
				if (STATUS != WEITER)
				{
					bufTester = &Verbinder::UTAbufTester;
					nameAusleser = &Verbinder::UTAnameauslesen;
					iosAnfangsSettings = NICHTS;
				}
				bufStat = enableModeTest(eBuf, bufStat);
				break;
			case 0x23:		// #
				if (STATUS != WEITER)
				{
					bufTester = &Verbinder::PIXbufTester;
					nameAusleser = &Verbinder::PIXnameauslesen;
					iosAnfangsSettings = TERMLEN;
				}
				bufStat = enableModeTest(eBuf, bufStat);
				break;
			case 0x29:		// )
				if (eBuf.find("> (enable) "))
				{
					if (STATUS != WEITER)
					{
						bufTester = &Verbinder::CATbufTester;
						nameAusleser = &Verbinder::CATnameauslesen;
						iosAnfangsSettings = TERMLEN;
						nullEaterEnable = true;
					}
					bufStat = enableModeTest(eBuf, bufStat);
				}
				else
					bufStat = NICHTS;
				break;
			default:
				bufStat = NICHTS;
				break;
			}		
			break;
		case 0x3E:			// login mode: >
			nameAusleser = &Verbinder::IOSnameauslesen;
			bufStat = loginModeTest(eBuf, bufStat);
			break;
		case 0x23:			// enable und config modes: #
			if (STATUS != WEITER)
			{
				bufTester = &Verbinder::IOSbufTester;
				nameAusleser = &Verbinder::IOSnameauslesen;
			}
			bufStat = enableModeTest(eBuf, bufStat);
			break;
		case 0x3A:		// : -> Bei IOS login Banner kommt der : ohne nachfolgendes SPACE
			bufStat = loginTest(eBuf, bufStat);
			break;
		case 0x5D:				// ] -> kommt im IOS bei Reload oder Config Overwrite... vor
			if (eBuf.rfind("[confirm]") != eBuf.npos)
			{
				bufStat = YES;
			}
			else
			{
				bufStat = NICHTS;
			}
			break;
		default:
			bufStat = NICHTS;
			break;
		}		
	}
	else
	{
		bufStat = NICHTS;
	}
	
	if (debugbufTestDetail)
	{
		// Ausgabe: 
		// Feld 1: bufStat
		// Feld 2: STATUS
		// Feld 3: vorletztes Zeichen
		// Feld 4: vor-vorletztes Zeichen
		char bbufff[4];
		string bla9 = itoa(bufStat, bbufff, 10);
		char ebufff[4];
		string bla10 = itoa(STATUS, ebufff, 10);
		string bla11 = "<loginBufTester><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		if (bufferLaenge)
		{
			bla11 += eBuf[bufferLaenge-1];
		}
		bla11 += "><";
		if (bufferLaenge > 1)
		{
			bla11 += eBuf[bufferLaenge-2];
		}
		bla11 += ">";
		schreibeLog(bla11, DEBUGFEHLER);
	}
	
	return bufStat;
}


// loginTest:
//***********
// Testen des Puffers auf User/Pass Abfrage und welches Passwort gefragt wird.
uint Verbinder::loginTest(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - loginTest>\n", DEBUGFEHLER);
	}

	if (debugBufTester)
	{
		schreibeLog("<loginTest>", DEBUGFEHLER);
	}

	string loginTestString = eBuf;
	
	if (loginTestString.length() < 8)
	{
		schreibeLog("\n<DEBUG loginTest: Using nichtsString>\n", DEBUGFEHLER);
		loginTestString = nichtsString + loginTestString;
		nichtsString = "";
	}
	

	size_t bufferLaenge = loginTestString.length();

	string loginAuswahl[] = {"sername", "assword"};
	uint loginGroesse = 2;

	for (uint i = 0; i < loginGroesse; i++)
	{
		if (loginTestString.find(loginAuswahl[i], bufferLaenge - 9) != loginTestString.npos)
		{
			switch (i)
			{
			case 0:
				if ((bufStat == USERNAME) || (bufStat == FEHLERMELDUNGALT))
				{
					ulez++;
					if (ulez == ulezmax)
					{
						bufStat = WZNIP;
						fehler = "Invalid Username or Password!";
					}
					else 
					{
						bufStat = USERNAME;
					}
				}
				else
				{
					if (!schonAuthe)
					{
						ulez = 0;
					}
					else
					{
						ulez++;
					}
					bufStat = USERNAME;
					schonAuthe = true;
				}
				break;
			case 1:
				if (bufStat == LOGINPASS)
				{
					ulez++;
					if (ulez == ulezmax)
					{
						bufStat = WZNIP;
						fehler = "Invalid Login Password!";
					}
					else
					{
						bufStat = LOGINPASS;
					}
				}
				else
				{
					if (!schonAuthe)
					{
						ulez = 0;
						schonAuthe = true;
					}
					bufStat = LOGINPASS;
				}
				if (execKontrolle)
				{
					if (!schonEnableAuthe)
					{
						ulez = 0;
						schonEnableAuthe = true;
						bufStat = ENABLEPASS;
					}
					else
					{
						ulez++;
						if (ulez == ulezmax)
						{
							bufStat = WZNIP;
							fehler = "Invalid Enable Password!";
						}
						else
						{
							bufStat = ENABLEPASS;
						}
					}
				}
				break;
			default:
				bufStat = NICHTS;
				break;
			}
		}
	}
	// Falls eine User/PW Abfrage kommt, wurde eine Verbindung erfolgreich aufgebaut.
	erfolgreich = true;
	return bufStat;
}


// loginModeTest:
//***************
// Testen des Puffers, wenn sich das Gerät im login Modus befindet
uint Verbinder::loginModeTest(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - loginModeTest>\n", DEBUGFEHLER);
	}

	if (hostnameAuslesen)
	{
		(*this.*nameAusleser)(eBuf);
		//nameauslesen(eBuf);
	}

	if (STATUS == WEITER)
	{
		bufStat = CTELNET;
		STATUS = GUT;
	}
	else if (STATUS == WEITERVERWEDNEN)
	{
		bufStat = COMMAND;
		STATUS = GUT;
	}
	else if (erfolgreich)
	{
		if (!execKontrolle)
		{
			bufStat = ENABLEMODUS;
			execKontrolle = true;
		}
		// Falls "execKontrolle" wahr ist, wurde schon einmal erfolglos versucht, in den enable Modus zu wechseln
		// oder es ist kein enable Passwort vorhanden. Das kann auch eine gewollte Situation sein:
		// * Weiter Telnet auf den nächsten Host
		// * Abfrage diverser "show" Befehle.
		else
		{
			if (ulezmaxEn > 1)
			{
				bufStat = ENABLEPASSAGAIN;
				ulezmaxEn--;
			}
			else
			{
				switch(enPWf)
				{
				case EXIT:
					bufStat = EXIT;
					erfolgreich = false;
					break;
				case COMMAND:
					bufStat = COMMAND;
					break;
				case TERMLEN:
					bufStat = TERMLEN;
					enPWf = COMMAND;
					break;
				case PAGERLINE:
					bufStat = PAGERLINE;
					enPWf = COMMAND;
					break;
				case SETLENGTH:
					bufStat = SETLENGTH;
					enPWf = COMMAND;
				default:
					break;
				}
			}
		}
	}
	else
	{
		fehler = "Could not connect to " + ip;
		bufStat = WZNIP;
	}
	return bufStat;
}


// enableModeTest:
//****************
// Funktion zum Auswerten des Puffers, wenn sich das Gerät im enable Modus befindet
uint Verbinder::enableModeTest(string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - enableModeTest>\n", DEBUGFEHLER);
	}

	if (STATUS == WEITER)
	{
		bufStat = CTELNET;
		STATUS = GUT;
	}
	else if (STATUS == WEITERVERWEDNEN)
	{
		bufStat = COMMAND;
		STATUS = GUT;
	}
	else if (erfolgreich)
	{
		bufStat = ENTER;
	}
	else
	{
		fehler = "Could not connect to " + ip;
		bufStat = WZNIP;
	}
	if (hostnameAuslesen)
	{
		(*this.*nameAusleser)(eBuf);
	}

	if (debugbufTestDetail)
	{
		// Ausgabe: 
		// Feld 1: bufStat
		// Feld 2: STATUS
		// Feld 3: vorletztes Zeichen
		// Feld 4: vor-vorletztes Zeichen
		char bbufff[4];
		string bla9 = itoa(bufStat, bbufff, 10);
		char ebufff[4];
		string bla10 = itoa(STATUS, ebufff, 10);
		string bla11 = "<enableModeTest><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		bla11 += "><";
		bla11 += ">";
		schreibeLog(bla11, DEBUGFEHLER);
	}
	return bufStat;
}


// upFinder:
//**********
// Funktion zum Auffinden der User/PW in einer Konfiguration. Die User/PW sollten immer am Anfang der Konfiguration stehen,
// damit nicht die gesamte Konfiguration durchsucht werden muss, denn sobald Username, login Password und enable Password gefunden
// wurden, wird diese Konfiguration nicht mehr weiter durchsucht. Die User/PW müssen mit wie folgt in der Konfiguration angegeben werden:
// *!user = username
// *!lopw = password
// *!enpw = password
// An die Funktion wird die Konfigurationszeile und die Anzahl der schon gefundenen User/PW übergeben.
uint Verbinder::upFinder(string up, uint upwZaehler)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - upFinder>\n", DEBUGFEHLER);
	}

	string upw[] = {"!user", "!lopw", "!enpw"};
	string userpass;		// Hier wird der User/PW zwischengespeichert
	// Drei Durchläufe, da es drei Sachen zu finden gilt.
	string zw;	// temp. Buffer
	for (uint i = upwZaehler; i < 3; i++)
	{
		userpass = strfind(upw[i], up, 8);
		if (!userpass.empty())
		{
			// Falls etwas gefunden wurde, wird nun geschaut, was es ist und der upwZaehler um eins erhöht
			switch (i)
			{
			case 0:
				zw  = userpass + "\r\n";
				username.push_back(zw);
				i = 3;
				break;
			case 1:
				zw = userpass + "\r\n";
				loginpass.push_back(zw); 
				i = 3;
				break;
			case 2:
				zw = userpass + "\r\n";
				enablepass.push_back(zw);
				break;
			}
			upwZaehler++;
		}
	}

	return upwZaehler;
}


// outBufZusammensteller:
//***********************
// Funktion, die den Sendepuffer anhand der ausgewerteten Daten von z.B.: IOSbufTester zusammenstellt.
uint Verbinder::outBufZusammensteller(uint bufStat, string buf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - outBufZusammensteller>\n", DEBUGFEHLER);
	}

	if (showGesendet)
	{
		if (showDat)
		{
			if (debugShowAusgabe)
			{
				schreibeLog("\n<DEBUG Show Output: Verbinder - showAusgabe mit buf fuellen>\n<BEGIN>\n", DEBUGFEHLER);
				schreibeLog(buf, DEBUGFEHLER);
				schreibeLog("\n<DEBUG Show Output: Verbinder - showAusgabe mit buf fuellen>\n<END>\n", DEBUGFEHLER);
			}
			showAusgabe << buf;
		}
		else
		{
			shString += buf;
		}
	}

	switch (bufStat)
	{
	case UNDEFINIERT:
		schreibeLog("Unknown error!", SYSTEMFEHLER);
		break;
	case NICHTS:
		nichtsString += buf; 
		//Für den Fall, dass bei Multihop verschiedene Gerätetypen verwendet werden.
		//Dann wird bei nichts senden geprüft, ob die Config leer ist und kein show Befehl abgesetzt wurde
		//Sollte das der Fall sein, wird der Buftester bei *Verbindung zurück auf loginBufTester gesetzt, 
		//damit das ursprüngliche Gerät wieder erkannt werden kann.
		if (config.empty() && !showGesendet && !bufTestAenderung)// && (MODUS == WEITER))
		{
			NICHTSaendertBufTest = true;
			strcpy(outBuf, "");
		}
		else
		{
			strcpy(outBuf, "");
		}
		break;
	case ENTER:
		strcpy(outBuf, "\r\n");
		break;
	case ENABLEPASSAGAIN:
	case ENABLEMODUS:
		strcpy(outBuf, enable.c_str());
		break;
	case USERNAME:
		if (username.empty())
		{
			bufStat = UNDEFINIERT;
			fehler = "No Username found!";
		}
		strcpy(outBuf, username[ulez].c_str());
		break;
	case LOGINPASS:
		if (loginpass.empty())
		{
			bufStat = UNDEFINIERT;
			fehler = "No Login Password found!";
		}
		strcpy(outBuf, loginpass[ulez].c_str());
		if (tscAuthe)
		{
			strcat(outBuf, "\r\n");
		}
		break;
	case ENABLEPASS:		
		strcpy(outBuf, enablepass[ulez].c_str());
		break;
	case COMMAND:

		if (showGesendet)
		{
			showGesendet = false;
			if (showDat)
			{
				if (debugShowAusgabe)
				{
					schreibeLog("\n<DEBUG Show Output: Verbinder - Show Ausgabe schließen>\n", DEBUGFEHLER);
				}

				showAusgabe.close();
				if (showAusgabe.fail())
				{
					schreibeLog("Show I/O Error!", SYSTEMFEHLER);
					
					if (debugShowAusgabe)
					{
						schreibeLog("\n<DEBUG Show Output: Verbinder - Show Ausgabe schließen fehlgeschlagen>\n", DEBUGFEHLER);
					}
				}
			}
		}
		if (!config.empty())
		{
			bool logIt = false;			// Soll die folgende Ausgabe mitgeloggt werden? Erweiterung von show Ausgabe um den !LOG Tag
			if (teste)
			{				
				// Test, ob die Verbindung erfolgreich war...
				if ((hostname == hostnameAlt) && (STATUS == GUT))
				{
					// ...wenn nicht, dann wird die Config gelöscht und ein ENTER gesendet
					config.clear();
					strcpy(outBuf, "\r\n");
					schreibeLog("Could not connect to " + ip, SYSTEMFEHLER);
					break;
				}	
			}
			string configZeile = config.front() + "\r\n";
			strcpy(outBuf, configZeile.c_str());
			config.pop_front();

		
			// Das Feature "Warten auf Bestätigung" kann auch in der Konfig aktiviert und abgeschalten werden
			if (configZeile.substr(0,11) == "!BEGINBLOCK")
			{
				warteRaute = false;
			}
			else if (configZeile.substr(0,9) == "!ENDBLOCK")
			{
				warteRaute = true;
			}
			else if (configZeile.substr(0,5) == "!user" || configZeile.substr(0,5) == "!lopw" || configZeile.substr(0,5) == "!enpw")
			{
				strcpy(outBuf, "\r\n");
			}
			else if (configZeile.substr(0,15) == "!CTRL+SHIFT+6 X")
			{
				strcpy(outBuf, "\x1e\x78\r\n");
			}
			else if (configZeile.substr(0,5) == "!CRLF")
			{
				strcpy(outBuf, "\r\n");
			}
			else if (configZeile.substr(0,4) == "show")
			{
				logIt = true;
			}
			else if (configZeile.substr(0,4) == "!LOG")
			{
				if (!config.empty())
				{
					configZeile = config.front() + "\r\n";
					strcpy(outBuf, configZeile.c_str());
					config.pop_front();
					logIt = true;
				}
			}
			else if (configZeile.substr(0,10) == "!PARAGRAPH")
			{
				// Alles was zwischen !PARAGRAPHen steht wird in einem Paket gesendet
				
				bool anfang = true;			// Zur Kennzeichnung des Anfangs
				string configZeile = "";
				while (config.size())
				{
					if (configZeile.substr(0,10) != "!PARAGRAPH")
					{
						configZeile += config.front() + "\r\n";
						config.pop_front();
						anfang = false;
					}
					else if (!anfang)
					{
						config.pop_front();
						break;
					}
				}
				if (configZeile.size() < 1025)
				{
					strcpy(outBuf, configZeile.c_str());
				}
				else
				{
					schreibeLog("PARAGRAPH too large - Please contact wktools@spoerr.org", FEHLER);
				}
			}
			if (logIt)
			{
				showDat = false;
				string dateiname;
				switch (logOpt)
				{
				case siN:
					dateiname = ausgabePfad + "show-command-output.txt";
					showDat = true;
					break;
				case ipN:
					{
						char *zeit = new char[9];
						getDate(zeit);
						dateiname = ip + "-";
						dateiname += zeit;
						dateiname += ".txt";
						dateiname = ausgabePfad + dateiname;
						showDat = true;
					}
					break;
				case hoN:
					{
						char *zeit = new char[9];
						getDate(zeit);
						dateiname = hostname + "-";
						dateiname += zeit;
						dateiname += ".txt";
						dateiname = ausgabePfad + dateiname;
						showDat = true;
					}
				case poi:
					{
					}
					break;
				default:
					schreibeLog("Unknown Show Output Option - Please check your settings!", SYSTEMFEHLER);
					break;
				}
				bufStat = SHOW;
				showGesendet = true;
				if (showDat)
				{
					if (debugShowAusgabe)
					{
						schreibeLog("\n<DEBUG Show Output: Verbinder - show Ausgabe oeffnen>\n", DEBUGFEHLER);
						schreibeLog("\n<DEBUG Show Output: Verbinder - Hostname Show Ausgabe>\nHostname: ", DEBUGFEHLER);
					}

					if (!showAppend)
					{
						showAusgabe.rdbuf()->open(dateiname.c_str(), ios_base::out);
						showAppend = true;
					}
					else
					{
						showAusgabe.rdbuf()->open(dateiname.c_str(), ios_base::out | ios_base::app);
					}

					if (showAusgabe.fail())
					{
						schreibeLog("Show I/O Error!", SYSTEMFEHLER);

						if (debugShowAusgabe)
						{
							schreibeLog("\n<DEBUG Show Output: Verbinder - Show Ausgabe oeffnen fehlgeschlagen>\n", DEBUGFEHLER);
						}
					}

				}
			}

			// Wenn der letzte Befehl gesendet wurde und dies kein show Befehl ist,
			// dann wird der buftester umgeschrieben. 
			// Bei NICHTS allein ist es zu wenig, da es vorkommen kann, dass die gesamte letzte Antort im Empfangsbuffer steht
			// Dann hat man das Problem, dass der Buftester nicht umgestellt wird.

			if (config.empty() && !showGesendet)// && (MODUS == WEITER))
			{
				NICHTSaendertBufTest = true;
			}

			// Erfassen, ob die Config Queue leer ist, um bei SSH eine falsche Fehleranzeige zu vermeiden
			if (config.empty())
			{
				fertig = true;
			}

			configMode = false;
		}
		else if (configMode)
		{
			strcpy(outBuf, "exit\r\n");
			configMode = false;
		}
		else
		{
			bufStat = ENDE;
			strcpy(outBuf, "");
			fertig = true;
		}
		break;
	case TLULS:
		strcpy(outBuf, "terminal length 0\r\n");
		config.push_front("end");
		config.push_front("logging sync");
		config.push_front("line vty 0 4");
		config.push_front("logging sync");
		config.push_front("line con 0");
		config.push_front("conf t");
		break;
	case TERMLEN:
		strcpy(outBuf, "terminal length 0\r\n");
		break;
	case LOGGSYNC:
		strcpy(outBuf, "conf t\r\n");
		config.push_front("end");
		config.push_front("logging sync");
		config.push_front("line vty 0 4");
		config.push_front("logging sync");
		config.push_front("line con 0");
		break;
	case PAGERLINE:
		strcpy(outBuf, "pager line 0\r\n");
		config.push_front("terminal pager lines 0");
		break;
	case SETLENGTH:
		strcpy(outBuf, "set length 0\r\n");
		break;
	case FEHLERMELDUNG:
		strcpy(outBuf, "\r\n");
		bufStat = FEHLERMELDUNGALT;
		break;
	case FEHLERMELDUNGALT:
		strcpy(outBuf, "\r\n");
		break;
	case SHOW:
		strcpy(outBuf, "");
		bufStat = NICHTS;
		break;
	case CTELNET:
		strcpy(outBuf, multiHop.c_str());
		strcat(outBuf, ip.c_str());
		strcat(outBuf, multiHopPost.c_str());
		strcat(outBuf, "\r\n");
		hostnameAuslesen = true;
		if (tscAuthe || tscNoAuthe)
		{
			strcat(outBuf, "\r\n");
		}
		break;
	case EXIT:
		strcpy(outBuf, "exit\r\n");
		break;
	case YES:
		strcpy(outBuf, "yes\r\n");
		break;
	case NO:
		strcpy(outBuf, "no\r\n");
		break;
	case WZNIP:
	default:
		strcpy(outBuf, "");
		break;
	}
	outBufZaehler = strlen(outBuf);
	
	if (debugbufTestDetail)
	{
		// Ausgabe: 
		// Feld 1: bufStat
		// Feld 2: STATUS
		// Feld 3: vorletztes Zeichen
		// Feld 4: vor-vorletztes Zeichen
		char bbufff[4];
		string bla9 = itoa(bufStat, bbufff, 10);
		char ebufff[4];
		string bla10 = itoa(STATUS, ebufff, 10);
		string bla11 = "<outBufZusammensteller><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		bla11 += "><";
		bla11 += ">";
		schreibeLog(bla11, DEBUGFEHLER);
	}
	
	return bufStat;
}


// nameauslesen:
//**************
// Funktion zum Auslesen des Hostnames aus einem Puffer
void Verbinder::nameauslesen(string eBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - nameauslesen>\n", DEBUGFEHLER);
	}

	size_t gesamt = eBuf.size();
	size_t hnLaenge1 = eBuf.find_last_of('\x0D');
	size_t hnLaenge2 = eBuf.find_last_of('\x0A');
	if (hnLaenge1 > hnLaenge2)
	{
        hostname = eBuf.substr(hnLaenge1+2, gesamt-hnLaenge1-4);
	}
	else
	{
		hostname = eBuf.substr(hnLaenge2+1, gesamt-hnLaenge2-2);
	}
	hostnameAuslesen = false;
}

// IOSnameauslesen:
//*****************
// Funktion zum Auslesen des Hostnamens aus IOS Geräten
void Verbinder::IOSnameauslesen(string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - IOSnameauslesen>\n", DEBUGFEHLER);
	}
	
	string eBuf = zuletztEmpfangen + neBuf;
	size_t gesamt = eBuf.size();
	size_t hnLaenge = eBuf.find_last_of("\r\n");
	hostnameAlt = hostname;
	hostname = eBuf.substr(hnLaenge+1, gesamt-hnLaenge-2);
	hostnameAuslesen = false;
	
	if (debugHostname)
	{
		string logm = "\n\n<<IOS Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}
}


// PIXnameauslesen:
//*****************
// Funktion zum Hostnameauslesen aus PIXen
void Verbinder::PIXnameauslesen(string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - PIXnameauslesen>\n", DEBUGFEHLER);
	}

	string eBuf = zuletztEmpfangen + neBuf;
	size_t gesamt = eBuf.size();
	size_t hnLaenge = eBuf.find_last_of("\r\n");
	hostnameAlt = hostname;
	hostname = eBuf.substr(hnLaenge+1, gesamt-hnLaenge-3);
	hostnameAuslesen = false;

	if (debugHostname)
	{
		string logm = "\n\n<<PIX Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		logm += "ebuf = <<";
		logm += eBuf + ">>\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}

}


// CATnameauslesen:
//*****************
// Funktion zum Hostnameauslesen aus Switches mit CatOS
void Verbinder::CATnameauslesen(string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - CATnameauslesen>\n", DEBUGFEHLER);
	}

	string eBuf = zuletztEmpfangen + neBuf;
	size_t hnLaengeP = eBuf.find_last_of("\r\n");
	if (hnLaengeP == eBuf.npos)
	{
		hnLaengeP = 0;
	}
	else
	{
		hnLaengeP++;
	}
	size_t hnLaenge = eBuf.find_last_of('\x3E');
	size_t hnl = hnLaenge - hnLaengeP;
	hostnameAlt = hostname;
	hostname = eBuf.substr(hnLaengeP, hnl);
	hostnameAuslesen = false;
	
	if (debugHostname)
	{
		string logm = "\n\n<<CatOS Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}

}


// UTAnameauslesen:
//*****************
// Funktion zum Hostnameauslesen aus Switches mit CatOS
void Verbinder::UTAnameauslesen(string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - UTAnameauslesen>\n", DEBUGFEHLER);
	}

	hostnameAlt = hostname;
	hostname = "UTA";
	hostnameAuslesen = false;

	if (debugHostname)
	{
		string logm = "\n\n<<UTA Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}

}


// telnetInit:
//************
// Initialisierung der Netzwerkeinstellungen und des Sockets für die Telnetverbindung
void Verbinder::telnetInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - telnetInit>\n", DEBUGFEHLER);
	}
}

// consoleInit:
//*************
// Initialisierung der Seriellen Schnittstelle
void Verbinder::consoleInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - consoleInit>\n", DEBUGFEHLER);
	}

//	DCB konsoleEinstellungen;		// Konsole Einstelleungen (Speed, Fehlerkontrolle, Stop Bits...)
	bool status;					// Konsole Status
	string com;						// Com Schnittstelle
	
	// Welche Serielle Schnittstelle soll verwendet werden?
	if (MODUS == COM1)
	{
#ifdef _WIN32
		// windows uses com ports, this depends on what com port your cable is plugged in to.
		com = "COM1";
#else
		// *nix com ports
		com = "dev/ttyS1";		
#endif	
	}

	if (MODUS == COMx)
	{
#ifdef _WIN32
		// windows uses com ports, this depends on what com port your cable is plugged in to.
		com = "COM" + vPort;
#else
		// *nix com ports
		com = "dev/" + vPort;
#endif
	}
//
//	// Öffnen der Seriellen Schnittstelle (wird gleich wie ein File behandelt)
//	konsole = CreateFile(com, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
//
//	if (konsole == INVALID_HANDLE_VALUE)
//	{
//		schreibeLog("Could not connect to serial interface! ", SYSTEMFEHLER);
//	}
//
//	// COM Einstellungen auslesen....
//	status = GetCommState(konsole, &konsoleEinstellungen);
//	if (!status)
//	{
//		schreibeLog("Could not connect to serial interface! ", SYSTEMFEHLER);
//	}
//	
//	// ...ändern....

//	serPort.assign()
//	BuildCommDCB("baud=9600 parity=N data=8 stop=1", &konsoleEinstellungen);
//	konsoleEinstellungen.fBinary = true;
//	konsoleEinstellungen.fParity = false;
//	konsoleEinstellungen.fOutxCtsFlow = false;
//	konsoleEinstellungen.fOutxDsrFlow = false;
//	konsoleEinstellungen.fDtrControl = DTR_CONTROL_DISABLE;
//	konsoleEinstellungen.fDsrSensitivity = false;
//	konsoleEinstellungen.fTXContinueOnXoff = false;
//	konsoleEinstellungen.fNull = true;
//	konsoleEinstellungen.fErrorChar = false;
//	konsoleEinstellungen.fAbortOnError = false;
////	konsoleEinstellungen.wReserved = 0;
//	konsoleEinstellungen.fRtsControl = RTS_CONTROL_DISABLE;
//
//	// ... und die Änderungen wieder setzen
//	status = SetCommState(konsole, &konsoleEinstellungen);
//	if (!status)
//	{
//		schreibeLog("Failure while initializing the serial interface!", SYSTEMFEHLER);
//	}
//
//	/*// EV_TXEMPTY... alle chars wurden gesendet und der OutputBuffer ist leer
//	status = SetCommMask(konsole, EV_RXCHAR | EV_TXEMPTY);
//	if(!status)
//	{
//		//bla.fehlerAusgabe("Es konnte nicht auf die serielle Schnittstelle zugegriffen werden! ");
//		//bla.abbrechen();
//	}*/
//	
//	// Ändern der COM timeouts...
//	COMMTIMEOUTS timeouts;
//	timeouts.ReadIntervalTimeout = 20;
//	timeouts.ReadTotalTimeoutConstant = 10;
//	timeouts.ReadTotalTimeoutMultiplier = 100;
//	timeouts.WriteTotalTimeoutConstant = 10;
//	timeouts.WriteTotalTimeoutMultiplier = 100;
//	
//	//... und setzen 
//	if (!SetCommTimeouts(konsole, &timeouts))
//	{
//		schreibeLog("Communcation settings error!", SYSTEMFEHLER);
//	}
//
//	// Setzen der Comm Events. WaitCommEvent() wartet dann so lange, bis eines dieser Events auftritt:
//	// EV_RXCHAR... mindestens ein char wurde empfangen und ist im InputBuffer
//	if (!SetCommMask(konsole, EV_RXCHAR))
//	{
//		schreibeLog("Communcation settings error!", SYSTEMFEHLER);
//	}
}


// sshInit:
//*********
// zum Initialisieren aller nötigen SSH Parameter
void Verbinder::sshInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - sshInit>\n", DEBUGFEHLER);
	}

	if (MODUS == SSH1)
	{
		sshVersion = 1;
	}
	cryptInit();
}


// httpInit:
//**********
// zum Initialisieren der HTTP Parameter
void Verbinder::httpInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - httpInit>\n", DEBUGFEHLER);
	}
}


// setPort:
//*********
// Funktion zum Setzen vom Verbindungsport, da Telnet nicht immer 23 und SSH nicht immer 22 sein muss
// Terminal Server Verbindungssettings werden ebenso mit dieser Funktion gesetzt
// wird auch für COM Ports genutzt
void Verbinder::setPort(string verbPort, string tscm)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setPort>\n", DEBUGFEHLER);
	}

	vPort = verbPort;
	tscAuthe = false;
	tscNoAuthe = false;

	if (tscm == "No Authentication")
	{
		tscNoAuthe = true;
	}
	else if (tscm == "Authentication")
	{
		tscAuthe = true;
	}
}


// setDefaultDatei:
//*****************
// Funktion zum Setzen der default Datei. wird bei dynamischen Konfigurationen mit default Datei verwendet.
void Verbinder::setDefaultDatei(string datei)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setDefaultDatei>\n", DEBUGFEHLER);
	}

	defaultDatei = datei;
}


// statConfig:
//*************
// Funktion zum Einlesen der Konfigurationsdatei. Die Konfig kann dynamisch sein, muss aber vom Caller
// richtig gehandelt werden
dqstring Verbinder::statConfig(string dateiName, bool dynPWstat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - statConfig>\n", DEBUGFEHLER);
	}

	keineDatei = false;
	dqstring config;
	string zeile;

	ifstream conf(dateiName.c_str(), ios_base::in);

	uint upwZaehler = 0;					// Zaehler für dynamische User/PW, wieviele USer/PW schon gefunden wurden	

	while (!conf.eof())
	{
		getline(conf, zeile, '\n');
		if (!zeile.empty())
		{
			config.push_back(zeile);
		}
	}
	defaultConfig = config;
	conf.close();
	
	return defaultConfig;
}


// inventoryConf:
//***************
// Funktion zum Einlesen der passenden Inventory Config
dqstring Verbinder::inventoryConf(string dummyString, bool dummybool)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - inventoryConf>\n", DEBUGFEHLER);
	}

	string commands[] = {"show ver", "show snmp", "show module", "show c7200", "show diag",
		"show invent", "show hardware", "show system", "show file system", "exit"};
	uint anzahl = 10;

	dqstring config;
	for (uint i = 0; i < anzahl; i++)
	{
		config.push_back(commands[i]+"\r\n");
	}

	return config;
}


// intfConf:
//**********
// Funktion zum Einlesen der passenden Interface Statistik Config
dqstring Verbinder::intfConf(string dummyString, bool dummybool)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - intfConf>\n", DEBUGFEHLER);
	}

	string commands[] = {"show ver", "show interface"};
	uint anzahl = 2;

	dqstring config;
	for (uint i = 0; i < anzahl; i++)
	{
		config.push_back(commands[i]+"\r\n");
	}

	return config;
}


void Verbinder::setMultiHop(string mh)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setMultiHop>\n", DEBUGFEHLER);
	}

	size_t pos = mh.find("$ip");

	if (pos != mh.npos)
	{
		multiHop = mh.substr(0, pos) + " ";
		multiHopPost = mh.substr(pos+3, mh.size()-pos-3);
	}
	else
	{
		multiHop = mh + " ";
		multiHopPost = "";
	}
}

void Verbinder::setEinstellungen(string ena, int logo, string ausgabeVz, string mh, int enaPFf, int raute)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setEinstellungen>\n", DEBUGFEHLER);
	}

	// enable Settings setzen
	enable = ena + "\r\n";

	// log Optionen setzen
	switch (logo)
	{
	case 0:		// Hostname
		logOpt = hoN;
		break;
	case 1:		// IP Address
		logOpt = ipN;
		break;
	case 2:		// One File
		logOpt = siN;
		break;
	case 3:		// Pointer
		logOpt = poi;
		break;
	default:
		break;
	}
	
	// Einstellungen für enable Passwort falsch
	if (enaPFf == COMMAND)
	{
		enPWf = TERMLEN;
	}
	else
	{
        enPWf = enaPFf;
	}

	ausgabePfad = ausgabeVz;

	multiHop = mh + " ";

	warteRaute = raute;
}


// setModus:
//**********
// Funktion zum Setzen des Modus
void Verbinder::setModus(uint modus)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setModus>\n", DEBUGFEHLER);
	}

	MODUS = modus;
}


// setShowAppend:
//***************
// Funktion zum Setzen der showAppend Variable, die angibt, ob die show Ausgabe an ein bestehendes File 
// angehängt werden soll.
void Verbinder::setShowAppend(bool showApp)
{
	showAppend = showApp;
}


// setUserPass:
//*************
// Funktion zum Setzen der User/Passworte bei Verwendung von statischen User/Passworten
void Verbinder::setUserPass(string user, string lopw, string enpw)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setUserPass>\n", DEBUGFEHLER);
	}

	if (user.find(";;") != user.npos)
	{
		size_t pos1 = 0;
		size_t pos2 = 0;
		string zw;

		while(pos2 != user.npos)
		{
			pos2 = user.find(";;", pos1);
			zw = user.substr(pos1, pos2-pos1) + "\r\n";
			pos1 = pos2 + 2;
			username.push_back(zw);
		}
		
		pos1 = pos2 = 0;
		while(pos2 != lopw.npos)
		{
			pos2 = lopw.find(";;", pos1);
			zw = lopw.substr(pos1, pos2-pos1) + "\r\n";
			pos1 = pos2 + 2;
			loginpass.push_back(zw);
		}
		
		pos1 = pos2 = 0;
		while(pos2 != enpw.npos)
		{
			pos2 = enpw.find(";;", pos1);
			zw = enpw.substr(pos1, pos2-pos1) + "\r\n";
			pos1 = pos2 + 2;
			enablepass.push_back(zw);
		}
	}
	else
	{	
		string ep = enpw + "\r\n";
		string lp = lopw + "\r\n";
		string un = user + "\r\n";

		username.clear();
		enablepass.clear();
		loginpass.clear();

		for (uint i = 0; i < 3; i++)
		{
			username.push_back(un);
			enablepass.push_back(ep);
			loginpass.push_back(lp);
		}
	}

	ulezmax = username.size();
}


// setLogsy:
//**********
// zum Setzen von iosAnfangsSettings -> soll "logging sync" konfiguriert werden oder nicht
void Verbinder::setLogsy(uint logsy)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - setLogsy>\n", DEBUGFEHLER);
	}

	iosAnfangsSettings = logsy;
}


// initVerinderVars:
//******************
// zum Initialisieren der mehrfach benutzten Variablen für Telnet, SSH und Console
void Verbinder::initVerbinderVars()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - initVerbinderVars>\n", DEBUGFEHLER);
	}

	erfolgreich = false;			// wurde eine Verbindung erfolgreich hergestellt oder nicht
	execKontrolle = false;			// Zum Feststellen, ob schon einmal versucht wurde, in den enable Modus zu wechseln
	showGesendet = false;			// wurde ein "show" Befehl abgesetzt?
	hostnameAuslesen = true;		// Soll der Hostname ausgelesen werden, um die Datei der "show" Befehl Ausgabe mit diesem zu benennen
	empfangsOffset = 0;				// Ab welchem Zeichen soll ausgewertet werden?
	configMode = false;				// Gerät ist nicht im Konfigurationsmodus
	schonAuthe = false;				// noch nicht erfolgreich authentifiziert
	schonEnableAuthe = false;		// noch kein erfolgreiches Enable Passwort gesendet
	shString = "";					// show String zurücksetzen
	zuletztEmpfangen = "";			// keine alten Daten
	ulezmaxEn = ulezmax;			// die maximalen Enable Passworte werden gleich den maximalen Usernamen gesetzt
	nichtsString = "";				// Alle Empfangsdaten vom vorigen Host werden gelöscht.
	NICHTSaendertBufTest = false;	// Zurücksetzen vom NICHTS Zähler
	bufTestAenderung = false;		// keine Änderung des Buftesters
	nullEaterEnable = false;		// NullEater ausschalten
	fertig = false;					// es sind noch Commands in der Queue
//	tscAuthe = false;				// Terminalserver mit Authentication zurücksetzen
//	tscNoAuthe = false;				// Terminal Server mit keiner Authentication zurücksetzen

	if (!warteRaute)
	{
		config.push_back("!ENDBLOCK");
	}

	bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
}


// telnetVerbindung:
//******************
// Das ist die Funktion, wenn telnet als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::telnetVerbindung(string adresse, uint status, uint modus, dqstring conf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - telnetVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	if (vPort == "")
	{
		vPort = "23";
	}
	else
	{
		if (!atoi(vPort.c_str()))
		{
			vPort = "23";
		}
	}

	initVerbinderVars();

	// Bei einer neuen Verbindung wird der Socket für diese erstellt und die IP Adressen, Ports... gesetzt. 
	// Danach wird eine Verbindung mit dem Remote Host hergestellt
	if (STATUS == NEU)
	{

		// Setzen der Socketeinstellungen, mit Namensauflösung
		if (!setSockEinstellungen(ip, vPort)) 
		{
			return SCHLECHT;
		}
		
		asio::error_code errorcode = asio::error::host_not_found;
		tcp::resolver::iterator end;

		// Herstellen einer Verbindung
		while (errorcode && endpoint_iterator != end)
		{
			verbindungsSock.close();
			verbindungsSock.connect(*endpoint_iterator++, errorcode);
		}
		if (errorcode)
		{
			string error = "Could not connect to " + ip;
			schreibeLog(error, SYSTEMFEHLER);
			return SOCKETERROR;
		}
		else
		{
			erfolgreich = true;
		}


		if (tscNoAuthe || tscAuthe)
		{
			// "term len 0" muss gesendet werden. Falls zusätzlich noch "logging sync" konfiguriert wird, wird das hier hinzugefügt
			if (iosAnfangsSettings == LOGGSYNC)
			{
				iosAnfangsSettings = TLULS;
			}
			else
			{
				iosAnfangsSettings = TERMLEN;
			}

			if (tscNoAuthe)
			{
				Sleep(2000);
				asio::write(verbindungsSock, asio::buffer("\r\n", 2));
				tscNoAuthe = false;
			}
		}
	}


	size_t bytes = 1;						// Anzahl der empfangenen Bytes
	uint bufTest = UNDEFINIERT;			// Anfangswert von bufTest -> wird dazu verwendet, um das Ergebnis der Empfangspufferauswertung abzuspeichern
	string eBuf;						// Empfangsdaten in c++-string-Form

	for(uint k = 0; bytes; k++)
	{
		asio::streambuf iSBuf;
		istream iStream(&iSBuf);
		
		outBufZaehler = 0;

		// Falls der status gleich "WEITER" oder "WEITERVERWENDEN" und der erste Durchlauf der for Schleife stattfindet, 
		// wird das Datenempfangen übersprungen, da es im Normalfall keine Daten zu empfangen gibt. Es muss erst etwas
		// gesendet werden, damit das Gegenüber eine verwertbare Antwort schickt.
		// Um feststellen zu können, ob das Empfangen übersprungen wurde, wird die Variable "empf" eingeführt.
		bool empf = false;

		asio::error_code error;
		if (!((STATUS == WEITER || STATUS == WEITERVERWEDNEN) && (!k)))
		{
			bytes = asio::read(verbindungsSock, iSBuf,	asio::transfer_at_least(1), error);
			if (error != asio::error::eof)
			{
				// TODO: Fehlerausgabe
			}
			empf = true;
		}
		// Umwandeln des Input Streams in ein char[]
		char *iBuf = new char[bytes+1];
		iStream.read(iBuf, bytes);
		iBuf[bytes] = 0x00;

		// Wenn Multihop und letztes Command gesendet wurde, dann soll der Buftester auf loginBufTest
		// gestellt werden, aber erst wenn die Antwort auf dei letzten gesendeten Daten empfangen wurde
		// Dies wird angenommen, wenn mehr als 15 Bytes auf einmal empfangen wurden.
		if (NICHTSaendertBufTest && (bytes > 15))
		{
			// Der loginBufTester muss aber informiert werden, dass es sich jetzt um kein neues Gerät handelt.
			bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
			strcpy(outBuf, "");
			bufTestAenderung = true;
			NICHTSaendertBufTest = false;
		}

		if (!empf)
		{
			strcpy(outBuf, "\r\n");
			outBufZaehler = 2;
			bufTest = ENTER;
		}
		else
		{
			if (bytes == 0)
			{
				if (!k)
				{
					string error = "Connection error with " + ip;
					schreibeLog(error, SYSTEMFEHLER);
					return SCHLECHT;
				}
				else
				{
					// Wenn die Config leer ist, wurde wahrscheinlich ein "exit" gesendet
					// Dann schlägt der Empfang natürlich fehl.
					// In diesem Fall wird sofort aus der for Schleife ausgestiegen
					if (config.empty())
					{
						break;
					}
					else
					{
						string error = "No connection with " + ip;
						schreibeLog(error, SYSTEMFEHLER);
						return NOCONN;
					}
				}
			}

			// Hin und wieder kann es vorkommen, dass Telnet Optionen in den Daten versteckt sind. 
			// Damit diese nicht verloren gehen oder unvorhergesehene Probleme verursachen, werden die Daten nach
			// Telnet Optionen durchsucht
			uint tofLaenge = telnetOptionenFinder(iBuf, bytes);
			if (tofLaenge != bytes)
			{
				size_t sentBytes = asio::write(verbindungsSock, asio::buffer(outBuf, outBufZaehler));
			}
			// Auswerten der Daten
			eBuf.assign(iBuf, tofLaenge);
			delete[] iBuf;		// iBuf löschen

			if (eBuf.length())
			{
				if (nullEaterEnable)
				{
					// Bei CatOS können "NULL" Zeichen mitten in den Daten vorkommen. Um spätere Probleme zu beseitigen,
					// werden diese herausgefiltert
					eBuf = nullEater(eBuf);
				}

				bufTest = (*this.*bufTester)(eBuf, bufTest);

				if (debugBufTester)
				{
					char bufff[20];
					string bla11 = itoa(bufTest, bufff, 10);
					string bla12 = "\n<BufTest = " + bla11;
					bla12+= ">\n";
					schreibeLog(bla12, DEBUGFEHLER);
				}

				if (debugEmpfangen)
				{
					char ebufff[20];
					string bla10 = itoa(eBuf.length(), ebufff, 10);
					string bla12 = "<Received Data Bytes: " + bla10;
					bla12 += " / Received = <";
					bla12 += eBuf;
					bla12 += ">";
					schreibeLog(bla12, DEBUGFEHLER);
				}
				bufTest = outBufZusammensteller(bufTest, eBuf);
			}
			else
			{
				bufTest = NICHTS;
			}
			// Im debug Modus werden alle Daten am Bildschirm ausgegeben
			if (!(bytes - tofLaenge))
			{
				if (bufTest == FEHLERMELDUNGALT)
				{
					schreibeLog(eBuf, DEBUGFEHLER);
				}
				else
				{
					schreibeLog(eBuf, INFO);
				}
			}
			zuletztEmpfangen = eBuf;
		}
		// Falls ein Fehler aufgetreten ist...
		if ((bufTest == UNDEFINIERT) || (bufTest == WZNIP))
		{
			string error = fehler + " => Next IP Address!";
			schreibeLog(error, SYSTEMFEHLER);
			break;
		}

		// Überprüfen, ob die Verbindung abgebrochen werden soll
		if (cancelVerbindung)
		{
			break;
		}

		uint sentBytes = 0;

		// Wenn Pakete im Ausgangspuffer sind, werden diese jetzt gesendet, 
		// sonst wird auf weitere Daten vom Gegenüber gewartet.
		if (outBufZaehler)
		{
			sentBytes = asio::write(verbindungsSock, asio::buffer(outBuf, outBufZaehler));

			if (debugSenden)
			{
				char sbufff[20];
				string bla10 = itoa(outBufZaehler, sbufff, 10);
				string bla12 = "<Sent Data Bytes: " + bla10;
				bla12 += " / Send = <";
				bla12 += outBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}
	//		int wsaFehler;
	//		if (wsaFehler = WSAGetLastError())
	//		{
	//			schreibeLog("No valid socket!", SYSTEMFEHLER);
	//			break;
	//		}
			// Abspeichern der gesendeten Daten in zuletzGesendet, um im Fehlerfall noch einaml auf diese zurückgreifen zu können.
			zuletztGesendet = outBuf;
			if (sentBytes != outBufZaehler)
			{
				schreibeLog("Could not send all data!", SYSTEMFEHLER);
			}
		}

		if (bufTest == ENDE && warteRaute)
		{
			break;
		}
	}		

	// Falls STEL verwendet wird, oder bei MTEL die letzte IP Adresse abgearbeitet wurde...
	if ((MODUS == NEU) || (MODUS == ENDE))
	{
		verbindungsSock.close();
	}
	//	schreibeLog("\n<<<<<<<<<<<WZNIP>>>>>>>>>>>>>\n", DEBUGFEHLER);

	return GUT;
}

// consoleVerbindung:
//*******************
// Das ist die Funktion, wenn konsole als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::consoleVerbindung(string adresse, uint status, uint modus, dqstring conf)
{
//	if (debugFunctionCall)
//	{
//		schreibeLog("\n<DEBUG Function Call: Verbinder - consoleVerbindung>\n", DEBUGFEHLER);
//	}
//
//	DWORD gelesen = 0;			// wie viele Daten wurden von der seriellen Schnittstelle gelesen?
//	DWORD geschrieben = 0;		// wie viele Daten wurden an die serielle Schnittstelle gesendet?
//	DWORD ioStatus;				// ist ein Event auf der seriellen Schnittstelle aufgetreten?
//	bool schreiben;				// war der Schreibvorgang erfolgreich?
//	
//	config = conf;				// Konfiguration, die in den aktuellen Host eingespielt werden soll
//	ip = adresse;				// IP Adresse des zu bearbeitenden Hosts
//	int bufTest = UNDEFINIERT;	// wird verwendet, um den Status der Empfangsdatenauswertung abzuspeichern
//	MODUS = modus;				// Modus -> global NEU oder WEITER
//	STATUS = status;			// Status von MODUS -> NEU oder WEITER
//
//	manuellI = &Verbinder::comManuellI;		// Falls der manuelle Modus aufgerufen wird, wird die Konsoleversion verwendet
//
//	initVerbinderVars();
//
//	// "term len 0" muss gesendet werden. Falls zusätzlich noch "logging sync" konfiguriert wird, wird das hier hinzugefügt
//	if (iosAnfangsSettings == LOGGSYNC)
//	{
//		iosAnfangsSettings = TLULS;
//	}
//	else
//	{
//		iosAnfangsSettings = TERMLEN;
//	}
//
////	cout << "\nInitial WRITE\n";
//	schreiben = WriteFile(konsole, "\r\n", 1, &geschrieben, NULL);
//
//	if (!schreiben)
//	{
//		schreibeLog("Error while writing to the serial interface!", SYSTEMFEHLER);
//	}
//
//	// Solange Daten zu senden sind....
//	while(1)
//	{
//		outBufZaehler = 0;		// Wie viele Daten sind zu senden
//		char konsoleBuf;		// welches Zeichen wurde gerade von der seriellen Schnittstelle empfangen
//		string zwischenBuf;		// Puffer zum abspeichern der empfangenen Daten
//
//		// Auslesen der seriellen Schnittstelle
//		if (WaitCommEvent(konsole, &ioStatus, NULL))
//		{
//			do 
//			{
//				ReadFile(konsole, &konsoleBuf, 1, &gelesen, NULL);
//				if (gelesen)
//					zwischenBuf += konsoleBuf;
//			}
//			while (gelesen);
//		}
//
//		if (STATUS == NEU)
//		{
//			erfolgreich = true;
//		}
//		
//		if (NICHTSaendertBufTest && (gelesen > 15))
//		{
//			// Der loginBufTester muss aber informiert werden, dass es sich jetzt um kein neues Gerät handelt.
//			bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
//			strcpy(outBuf, "");
//			bufTestAenderung = true;
//			NICHTSaendertBufTest = false;
//		}
//
//		if (nullEaterEnable)
//		{
//			// Bei CatOS können "NULL" Zeichen mitten in den Daten vorkommen. Um spätere Probleme zu beseitigen,
//			// werden diese herausgefiltert
//			zwischenBuf = nullEater(zwischenBuf);
//		}
//
//		// Auswerten der empfangenen Daten
//		bufTest = (*this.*bufTester)(zwischenBuf, bufTest);
//		bufTest = outBufZusammensteller(bufTest, zwischenBuf);
//		if (bufTest == FEHLERMELDUNGALT)
//		{
//			schreibeLog(zwischenBuf, DEBUGFEHLER);
//		}
//		else
//		{
//			schreibeLog(zwischenBuf, INFO);
//		}
//		leseInputEvents();
//
//		zuletztEmpfangen = zwischenBuf;
//		
//		// Falls ein Fehler aufgetreten ist...
//		if ((bufTest == UNDEFINIERT) || (bufTest == WZNIP))
//		{
//			string error = fehler + " => Next IP Address!";
//			schreibeLog(error, SYSTEMFEHLER);
//			break;
//		}
//		
//		// Überprüfen, ob die Verbindung abgebrochen werden soll
//		if (cancelVerbindung)
//		{
//			break;
//		}
//
//		// Senden von Daten, je nachdem wie die empfangenen Daten ausgewertet wurden
//		if (outBufZaehler)
//		{
//			schreiben = WriteFile(konsole, outBuf, outBufZaehler, &geschrieben, NULL);
//			// Da bei einer Konsoleverbindung die Daten zwei Mal empfangen werden, wird das epfangsOffset gesetzt
//			empfangsOffset = geschrieben;
//			if (!schreiben)
//			{
//				schreibeLog("Error while writing to the serial interface!", SYSTEMFEHLER);
//			}
//
//			strncpy(zuletztGesendet, outBuf, outBufZaehler);
//			zuletztGesendet[outBufZaehler] = 0x00;
//
//			if ((geschrieben) != outBufZaehler)
//			{
//				schreibeLog("Could not send all data!", SYSTEMFEHLER);
//			}
//		}
//
//		if (bufTest == ENDE && warteRaute)
//		{				
//			break;
//		}
//	}
	return GUT;
}


// sshVerbindung
//**************
// ssh Verbindung zum gewünschten Host aufbauen
uint Verbinder::sshVerbindung(string adresse, uint status, uint modus, dqstring conf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - sshVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	uint sshPort = 0;			// Portnummer
	bool weiterEnter = false;	// Wenn STATUS != NEU dann soll ein Enter als erstes geschickt werden.

	if (vPort == "")
	{
		sshPort = 22;
	}
	else
	{
		if (!(sshPort = atoi(vPort.c_str())))
		{
			sshPort = 22;
		}
	}
	initVerbinderVars();
	

	// "term len 0" muss gesendet werden. Falls zusätzlich noch "logging sync" konfiguriert wird, wird das hier hinzugefügt
	if (iosAnfangsSettings == LOGGSYNC)
	{
		iosAnfangsSettings = TLULS;
	}
	else
	{
		iosAnfangsSettings = TERMLEN;
	}

	// Bei einer neuen Verbindung wird die Crypt Session neu erstellt. 
	// Danach wird eine Verbindung mit dem Remote Host hergestellt
	if (STATUS == NEU)
	{
		string uname;
		string logpw;
		size_t pos = 0;

		pos = username[0].find_first_of("\r\n");
		uname = username[0].substr(0, pos);
		
		pos = loginpass[0].find_first_of("\r\n");
		logpw = loginpass[0].substr(0, pos);

		int retKey = 0;
		// Create the session
		retKey = cryptCreateSession(&cryptSession, CRYPT_UNUSED, CRYPT_SESSION_SSH);
		// Add the server name, user name, and password
		retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_SERVER_NAME,
			adresse.c_str(), adresse.size());
		retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_SERVER_PORT, sshPort);		
		retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_USERNAME,
			uname.c_str(), uname.size());
		retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_PASSWORD,
			logpw.c_str(), logpw.size());
		retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_VERSION, sshVersion);
		// Activate the session
		retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_ACTIVE, 1);
		if (retKey != CRYPT_OK)
		{
			schreibeLog("Connection Error!", SYSTEMFEHLER);
			cryptDestroySession(cryptSession);
			return SOCKETERROR;
		}
		else
		{
			erfolgreich = true;
		}
		vne = false;
	}
	else
	{
		weiterEnter = true;
	}

	int bytes = 0;						// Anzahl der empfangenen Bytes
	uint bufTest = UNDEFINIERT;			// Anfangswert von bufTest -> wird dazu verwendet, um das Ergebnis der Empfangspufferauswertung abzuspeichern
	string eBuf;						// Empfangsdaten in c++-string-Form
	char *buf = new char[BUFFER_SIZE];	// Empfangsdaten, die dann in einem c++-String abgespeichert werden
			
	while(1)
	{
		if (vne || weiterEnter)
		{
			strcpy(outBuf, "\r\n");
			outBufZaehler = 2;
			bufTest = ENTER;
			vne = false;
			weiterEnter = false;
		}
		else
		{
			outBufZaehler = 0;

			int retKey = cryptPopData(cryptSession, buf, BUFFER_SIZE-1, &bytes);
			if (retKey != CRYPT_OK)
			{
				if (fertig)
				{
					break;
				}
				else
				{
					schreibeLog("Connection Error!", SYSTEMFEHLER);
					return SOCKETERROR;
					break;
				}
			}

			buf[bytes] = 0x00;
			


			if (!bytes)
			{
				continue;
			}

			if (NICHTSaendertBufTest && (bytes > 15))
			{
				// Der loginBufTester muss aber informiert werden, dass es sich jetzt um kein neues Gerät handelt.
				bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
				strcpy(outBuf, "");
				bufTestAenderung = true;
				NICHTSaendertBufTest = false;
			}

			// Auswerten der Daten
			eBuf = buf;
			if (nullEaterEnable)
			{
				// Bei CatOS können "NULL" Zeichen mitten in den Daten vorkommen. Um spätere Probleme zu beseitigen,
				// werden diese herausgefiltert
				eBuf = nullEater(eBuf);
			}

			bufTest = (*this.*bufTester)(eBuf, bufTest);
			if (debugBufTester)
			{
				char bufff[20];
				string bla11 = itoa(bufTest, bufff, 10);
				string bla12 = "\n<BufTest = " + bla11;
				bla12+= ">\n";
				schreibeLog(bla12, DEBUGFEHLER);
			}

			if (debugEmpfangen)
			{
				char ebufff[20];
				string bla10 = itoa(eBuf.length(), ebufff, 10);
				string bla12 = "<Received Data Bytes: " + bla10;
				bla12 += " / Received = <";
				bla12 += eBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}
			bufTest = outBufZusammensteller(bufTest, eBuf);
			// Im debug Modus werden alle Daten am Bildschirm ausgegeben
			if (bufTest == FEHLERMELDUNGALT)
			{
				schreibeLog(eBuf, DEBUGFEHLER);
			}
			else
			{
				schreibeLog(eBuf, INFO);
			}
		
			zuletztEmpfangen = eBuf;

			// Falls ein Fehler aufgetreten ist...
			if ((bufTest == UNDEFINIERT) || (bufTest == WZNIP))
			{
				vne = true;
				string error = fehler + " => Next IP Address!";
				schreibeLog(error, SYSTEMFEHLER);
				break;
			}

			
			// Überprüfen, ob die Verbindung abgebrochen werden soll
			if (cancelVerbindung)
			{
				break;
			}

			// Wenn Pakete im Ausgangspuffer sind, werden diese jetzt gesendet, 
			// sonst wird auf weitere Daten vom Gegenüber gewartet.
		}

		int sentBytes = 0;

		if (outBufZaehler)
		{
			// Bei SSH gibt es ein Problem wenn \r\n gesendet wird 
			// -> es darf nur \r gesendet werden.
			// Darum wird der outbufZaehler um eins reduziert.
			outBufZaehler--;
			
			int retKey = cryptPushData(cryptSession, outBuf, outBufZaehler, &sentBytes);

			if (retKey != CRYPT_OK)
			{
				schreibeLog("Connection Error!", SYSTEMFEHLER);
				cryptDestroySession(cryptSession);
				break;
			}
			cryptFlushData(cryptSession);
			// Abspeichern der gesendeten Daten in zuletzGesendet, um im Fehlerfall noch einaml auf diese zurückgreifen zu können.
			zuletztGesendet = outBuf;
			
			if (debugSenden)
			{
				char sbufff[20];
				string bla10 = itoa(outBufZaehler, sbufff, 10);
				string bla12 = "<Sent Data Bytes: " + bla10;
				bla12 += " / Send = <";
				bla12 += outBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}

			
			if (sentBytes != outBufZaehler)
			{
				schreibeLog("Could not send all data!", SYSTEMFEHLER);
			}
		}

		if (bufTest == ENDE && warteRaute)
		{
			break;
		}
	}

	// Falls STEL verwendet wird, oder bei MTEL die letzte IP Adresse abgearbeitet wurde...
	if ((MODUS == NEU) || (MODUS == ENDE))
	{
		cryptDestroySession(cryptSession);
	}

	delete[] buf;
	return GUT;
}


// httpVerbindung:
//******************
// Das ist die Funktion, wenn HTTP als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::httpVerbindung(string adresse, uint status, uint modus, dqstring conf)
{
	// Initialisierung START
	// ist viel dabei, was bei HTTP nicht benötigt wird; Ist aber egal, wenn es trotzdem gemacht wird

	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - httpVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	uint telnetPort = 0;		// Portnummer
	if (vPort == "")
	{
		vPort = "80";
	}
	else
	{
		if (!atoi(vPort.c_str()))
		{
			vPort = "80";
		}
	}

	// Setzen der Socketeinstellungen, mit Namensauflösung
	if (!setSockEinstellungen(ip, vPort)) 
	{
		return SCHLECHT;
	}

	asio::error_code errorcode = asio::error::host_not_found;
	tcp::resolver::iterator end;

	// Herstellen einer Verbindung
	while (errorcode && endpoint_iterator != end)
	{
		verbindungsSock.close();
		verbindungsSock.connect(*endpoint_iterator++, errorcode);
	}
	if (errorcode)
	{
		string error = "Could not connect to " + ip;
		schreibeLog(error, SYSTEMFEHLER);
		return SOCKETERROR;
	}
	else
	{
		erfolgreich = true;
	}

	// GET mit User defined URL senden
	string cfg = "";
	if (!config.empty())
	{
		cfg = config.front();
	}

	string sendeDaten = "GET / " + cfg;
	sendeDaten += " HTTP/1.1\r\n";
	sendeDaten += "Host: " + adresse + "\r\n";
	sendeDaten += "User-Agent: wktools3\r\n";
	sendeDaten += "Accept: text/plain, text/html\r\n";
	sendeDaten += "\r\n";

	size_t sentBytes = 0;

	sentBytes = asio::write(verbindungsSock, asio::buffer(sendeDaten, sendeDaten.length()));

	if (sentBytes != sendeDaten.length())
	{
		schreibeLog("Could not send all data!", SYSTEMFEHLER);
		return SCHLECHT;
	}

	if (debugSenden)
	{
		char sbufff[20];
		string bla10 = itoa(sendeDaten.length(), sbufff, 10);
		string bla12 = "<Sent Data Bytes: " + bla10;
		bla12 += " / Send = <";
		bla12 += sendeDaten;
		bla12 += ">";
		schreibeLog(bla12, DEBUGFEHLER);
	}

	// Daten EMPFANGEN
	size_t bytes = 0;					// Anzahl der empfangenen Bytes
	string eBuf = "";					// Empfangsdaten in c++-string-Form
	asio::streambuf iSBuf;
	istream iStream(&iSBuf);

	bytes = asio::read_until(verbindungsSock, iSBuf, "\r\n");


	// Check that response is OK.
	string http_version;
	iStream >> http_version;
	uint status_code;
	iStream >> status_code;
	string status_message;
	getline(iStream, status_message);

	if (debugEmpfangen)
	{
		eBuf = http_version + status_code + status_message;
		char ebufff[20];
		string bla10 = itoa(eBuf.length(), ebufff, 10);
		string bla12 = "<Received Data Bytes: " + bla10;
		bla12 += " / Received = <";
		bla12 += eBuf;
		bla12 += ">";
		schreibeLog(bla12, DEBUGFEHLER);
	}

	if (!iStream || http_version.substr(0, 5) != "HTTP/")
	{
		schreibeLog("Invalid response!", SYSTEMFEHLER);
		return SCHLECHT;
	}
	if (status_code != 200)
	{
		string err = "Response returned with status code " + status_code;
		schreibeLog(err, SYSTEMFEHLER);
		return SCHLECHT;
	}

	// Read the response headers, which are terminated by a blank line.
	bytes = asio::read_until(verbindungsSock, iSBuf, "\r\n\r\n");

	char *iBuf = new char[bytes+1];
	iStream.read(iBuf, bytes);
	iBuf[bytes] = 0x00;

	eBuf.assign(iBuf);
	delete[] iBuf;		// iBuf löschen

	if (debugEmpfangen)
	{
		char ebufff[20];
		string bla10 = itoa(eBuf.length(), ebufff, 10);
		string bla12 = "<Received Data Bytes: " + bla10;
		bla12 += " / Received = <";
		bla12 += eBuf;
		bla12 += ">";
		schreibeLog(bla12, DEBUGFEHLER);
	}

	// Read until EOF, writing data to output as we go.
	string empfangsDaten = "";
	asio::error_code error = asio::error::host_not_found;
	while (bytes = asio::read(verbindungsSock, iSBuf, asio::transfer_at_least(1), error))
	{
		iBuf = new char[bytes+1];
		iStream.read(iBuf, bytes);
		iBuf[bytes] = 0x00;

		eBuf.assign(iBuf);
		delete[] iBuf;		// iBuf löschen

		schreibeLog(eBuf, INFO);
		empfangsDaten += eBuf;
		
		// Debug Ausgabe
		if (debugEmpfangen)
		{
			char ebufff[20];
			string bla10 = itoa(eBuf.length(), ebufff, 10);
			string bla12 = "<Received Data Bytes: " + bla10;
			bla12 += " / Received = <";
			bla12 += eBuf;
			bla12 += ">";
			schreibeLog(bla12, DEBUGFEHLER);
		}

	}
	if (error != asio::error::eof)
	{
//		TODO: throw asio::system_error(error);		
	}

	// Ausgabe der Daten im Debugfenster
	string datname = ausgabePfad + ip + ".htm";
	showAusgabe.rdbuf()->open(datname.c_str(), ios_base::out);
	showAusgabe	<< empfangsDaten;
	showAusgabe.close();

	verbindungsSock.close();

	return GUT;
}


// getDate:
//*********
// zum Auslesen des Datums
void Verbinder::getDate(char *datum)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - getDate>\n", DEBUGFEHLER);
	}

	time_t zeit = time(NULL);
	tm *jetzt = localtime(&zeit);

	//char datum[9];
	strftime(datum, 9, "%Y%m%d", jetzt);
}


// strfind:
//*********
// Overloaded Funktion zum Auffinden eines Substrings
// Version: Finden von Werten in beliebigen Strings
string Verbinder::strfind(string zuFinden, string basisString, int stellenAnzahl)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - strfind>\n", DEBUGFEHLER);
	}

	size_t pos1;					// Position 1 (Anfang) für die Suche im String
	size_t pos2;					// Position 2 (Ende) für die Suche im String
	size_t groesse;					// Länge des gesuchten Teilstrings

	zuFinden = zuFinden + " = ";
	pos1 = basisString.find(zuFinden, 0);
	if (pos1 != basisString.npos)
	{
		pos1 += stellenAnzahl;
		pos2 = basisString.find("\n", pos1);
		groesse = pos2 - pos1;

		if (groesse)
		{
			string gefunden = basisString.substr(pos1, groesse);
			return gefunden;
		}
	}

	return "ERROR";
}


// nullEater:
//***********
// Zum Entfernen von NULL aus CatOS Empfangsdaten
string Verbinder::nullEater(string catOSdaten)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - nullEater>\n", DEBUGFEHLER);
	}

	size_t pos1 = catOSdaten.size();
	for (size_t i = 0; i < pos1; i++)
	{
		if (catOSdaten[i] == '\0')
		{
			catOSdaten.erase(i, 1);
			pos1--;
		}
	}

	return catOSdaten;

}


string Verbinder::getShow()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - getShow>\n", DEBUGFEHLER);
	}

	return shString;
}


void Verbinder::schreibeLog(string log, uint dringlichkeit)
{
	if (asgbe != NULL)
	{
		evtData evtDat;
		
		string *lg = new string;
		lg->assign(log);
		
		wxCommandEvent evt(wkEVT_EINSPIELEN_INFO);
		switch (dringlichkeit)
		{
		case INFO:
			evtDat.farbe = WkLog::WkLog_SCHWARZ;
			evtDat.format = WkLog::WkLog_NORMALFORMAT;
			evtDat.logEintrag = log;
			evtDat.type = WkLog::WkLog_NORMALTYPE;
			evtDat.groesse = 10;
			evtDat.ipa = ip;
			evt.SetEventType(wkEVT_EINSPIELEN_INFO);
			break;
		case FEHLER:
			evtDat.farbe = WkLog::WkLog_ROT;
			evtDat.format =  WkLog::WkLog_NORMALFORMAT;
			evtDat.logEintrag = log;
			evtDat.type = WkLog::WkLog_ZEIT;
			evtDat.groesse = 10;
			evtDat.ipa = ip;
			evt.SetEventType(wkEVT_EINSPIELEN_FEHLER);
			break;
		case SYSTEMFEHLER:
			evtDat.farbe = WkLog::WkLog_ROT;
			evtDat.format = WkLog::WkLog_FETT;
			evtDat.logEintrag = log;
			evtDat.type = WkLog::WkLog_ZEIT;
			evtDat.groesse = 10;
			evtDat.ipa = ip;
			evt.SetEventType(wkEVT_EINSPIELEN_SYSTEMFEHLER);
			break;
		case DEBUGFEHLER:
			evtDat.farbe = WkLog::WkLog_ROT;
			evtDat.format = WkLog::WkLog_NORMALFORMAT;
			evtDat.logEintrag = log;
			evtDat.type = WkLog::WkLog_NORMALTYPE;
			evtDat.groesse = 10;
			evtDat.ipa = ip;
			evt.SetEventType(wkEVT_EINSPIELEN_DEBUGFEHLER);
			break;
		default:
			break;
		}	

//		wxCommandEvent event(wkEVT_SCHREIBELOG);
		evt.SetClientData(&evtDat);
		wxPostEvent(wkVerbinder, evt);

		// Ganz wichtig, damit die Logausgabe nicht mit sich selber über Kreuz kommt, wenn der Thread derweil weitermacht
		wxSafeYield();
	}

}


void Verbinder::skipVerbindungsTest()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n<DEBUG Function Call: Verbinder - skipVerbindungsTest>\n", DEBUGFEHLER);
	}

	teste = false;
}


void Verbinder::setDebug(bool dbgSend, bool dbgReceive, bool dbgFunction, bool dbgBufTester, bool dbgHostname, bool dbgBufTestDet, bool dbgShowAusgabe)
{
	debugBufTester = dbgBufTester;
	debugFunctionCall = dbgFunction;
	debugSenden = dbgSend;
	debugEmpfangen = dbgReceive;
	debugHostname = dbgHostname;
	debugbufTestDetail = dbgBufTestDet;
	debugShowAusgabe = dbgShowAusgabe;
}
