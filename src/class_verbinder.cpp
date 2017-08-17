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
// 17.09.2009 - mspoerr
//		- if (config.empty() && showGesendet) in outBufZusammensteller hinzugefügt, da es manchmal Probleme mit dem letzten Command gegeben hat
//		- "assowrd" Abfrage bei loginTest() hinzugefügt, da ein Bug bei 76er IOS vorhanden ist.
// 19.09.2009 - mspoerr
//		- IOSbufTester: Fehlercheck überarbeitet
// 24.10.2009 - mspoerr
//		- Die beiden int's für eine verbesserte Logausgabe mitaufgenommen:
//			- int index
//			- int wznip
// 18.01.2010 - mspoerr
//		- outBufZusammensteller: "enterSenden" Variable für unterschiedliche ENTER Behandlung bei Windows bzw. Linux eingeführt 
// 23.01.2010 - mspoerr
//		- alle \r\n bei Sendedaten gegen \n ersetzt
// 04.06.2010 - mspoerr
//		- mapperConf eingeführt
// 30.07.2010 - mspoerr
//		- bei sshVerbindung den Empfangs-Puffer löschen, wenn WEITER, da noch alte Daten von der vorigen Verbindung drinnenstehen könnten, 
//		  die sich negativ auf die neue Verbindung auswirken könnten
// 10.10.2010 - mspoerr
//		- pixBufTester: Steuerzeichen löschen eingeführt
// 15.10.2010 - mspoerr
//		- telnetVerbindung und sshVerbindung: Timer eingeführt, um die Verbindung zu schließen, falls keine Daten mehr empfangen werden
// 19.10.2010 - mspoerr
//		- antwort (std::string) eingeführt: outBufZusammensteller
//		- Weitere Config Tags in outBufZusammensteller eingeführt: !IFRETURN && !ELSERETURN und befüllt
//		- Hilfsfunktionen eingeführt: antwortAuswerten() und commandBauen(), aber noch nicht befüllt
// 24.10.2010 - mspoerr
//		- ifreturn Vafriable eingeführt
//		- antwortAuswerten() und commandBauen() befüllt
//		- debugregex Debug Option eingeführt
//		- commandBauen bei outBufZusammensteller überall, wo die nächste Configzeile gelesen wird, hinzugefügt
// 25.10.2010 - mspoerr
//		- commandBauen fertiggestellt
// 26.10.2010 - mspoerr
//		- commandBauen und antwortAuswerten erweitert (try, catch)
// 03.11.2010 - mspoerr
//		- httpsVerbindungAsio begonnen
// 07.11.2010 - mspoerr
//		- steuerzeichenEater: lastChar Position von 38 auf 30 geändert
//		- cryptlib von 3.3.2 auf 3.2.1 geändert (in den Visual Studio Einstellungen)
//		- httpsConnectAsio fertig gemacht; Offen: Fehler bei ssl error -> noch keine Lösung
// 10.01.2011 - mspoerr
//		- NXOSbufTester eingeführt
//		- loginBufTester für NX OS erweitert
//		- PIXbufTester: "eBuf" bei den Abfragen gegen "antwort" getauscht, da Fehlermeldungen usw. öfters in mehreren Paketen geschickt werden.
//		- NXOSnameauslesen eingeführt
// 15.01.2011 - mspoerr
//		- SSHv1 Fallback bei SSHv2 eingeführt
// 19.01.2011 - mspoerr
//		- mapperConf: Zwei zusätzliche Commands hinzugefügt
// 20.01.2011 - mspoerr
//		- pixBufTester: Fehlererkennung angepasst, da die Fehlermeldung anders ist; Fehler nicht bei "pager line" ausgeben
//		- pixBufTester: Fehlererkennung wieder auf eBuf umgedreht
// 11.03.2011 - mspoerr
//		- statConfig: Fehlerkontrolle eingebaut
//		- YESY Rückgabe eingeführt (outbufZusammensteller und iosBufTester). Damit wird nur "y" zurückgesendet
//		- iosBufTester: NEU: if ((eBuf[empfangsOffset + 1] == 0x0A) || (eBuf[empfangsOffset + 1] == 0x20))
//						STATT: if ((eBuf[empfangsOffset + 1] == 0x0A) || 0x20)
//		- statConfig: Zusätzliche Prüfung wegen Leerzeilen -> Leerzeilen werden abgefangen
//		- outBufZusammensteller: Neue Config Tags: !cmd-prefix, !device, !remove-newline
//		- outBufZusammensteller: rmCR eingeführt
// 28.03.2011 - mspoerr
//		- advancedRauteCheck eingeführt und iosBufTester um den neuen Check erweitert.
// 03.04.2011 - mspoerr
//		- mapperConf: Zusätzlich Commands hinzugefügt
// 08.04.2011 - mspoerr
//		- sshVerbindung und telnetVerbindung: debug für Empfangsdaten weiter nach vor geschoben (vor bufTester Aufruf)
//		- iosBufTester: Check in der default switch Anweisung eingeführt, ob empfangsDaten Länge > 1, damit es keinen Crash gibt, wenn auf empfangsdante-2 zugegriffen wird
//		- outBufZusammensteller: customShowName eingeführt
//		- outBufZusammensteller: zusätzliches Config Tag hinzugefügt: !showOutput
//		- showAusgabeTemp eingeführt
// 13.04.2011 - mspoerr
//		- user Variable eingeführt
//		- utaNamenAusleser erweitert
//		- loginBufTester erweitert
//		- utaBufTester erweitert
// 29.05.2011 - mspoerr
//		- pixNamenAusleser geändert (for Schleife hinzugefügt, um die / gegen - zu tauschen
// 01.06.2011 - mspoerr
//		- advancedRauteCheck Abfrage bei pixBufTester hinzugefügt.
//		- SSH Fallback als Info, nicht Warning ausgeben
// 02.06.2011 - mspoerr
//		- hostnameShowOutput eingeführt, da es bei PIX und anderen Geräten Zeichen im Hostnamen geben kann, die nicht für einen Dateinamen verwendet werden können, und daher ersetzt werden müssen
// 05.06.2011 - mspoerr
//		- outBufTester: "MODUS == WEITER" bei "NICHTSaendertBufTest TRUE - COMMAND" und "NICHTSaendertBufTest TRUE - NICHTS" aktiviert 
//		- Änderungen bereits am 09.01.2008 durchgeführt, aber am 10.01.2008 wieder rückgängig gemacht; WARUM? -> Tests machen!
//		- outBufZusammensteller: FEHLER: Wenn beim letzten Command ein Fehler erkannt wird, dann wird "exit gesendet;
//		- sshVerbindung: Sende Debugs erweitert und verschoben
// 07.06.2011 - mspoerr
//		- sshVerbindung: NEU: try/catch Block in der SSH Fehlerauswertung
// 08.06.2011 - mspoerr
//		- mapperConf: "show switch virtual link port" hinzugefügt
// 16.06.2011 - mspoerr
//->?	- Änderungen vom 5.6. (MODUS==WEITER) wieder zurückgenommen, da dann das Tool immer wieder hängen bleibt
//		- PIXbufTester und IOSbufTester: Erweiterte Debugs für advancedRauteCheck
//		- PIXbufTester und IOSbufTester: advancedRauteCheck Checks angepasst (2xCheck für Länge hinzugefügt)
//		- sshVerbindung und telnetVerbindung: Timeout check angepasst 
//			-> Counter eingeführt, damit nicht gleich beim ersten Timeout abgebrochen wird
//			-> zwei Mal wird ein Enter geschickt, um bei unbekannten Rückgaben kein Problem zu haben
//			-> Beim Timeout wird nicht mehr hart abgebrochen -> es wird der Durchlauf fertig gemacht
//		- setDummyCommand eingeführt, damit bei Singlehop am Schluss immer ein valides Command steht und im Fall immer ein Exit gesendet werden kann
//			-> !wktools-internal-dummy Tag eingeführt; Wenn der in der Config vorkommt, dann wird ein "exit" gesendet
// 17.06.2011 - mspoerr
//		- SSHVerbinder und TelnetVerbinder: Check erweitert, damit bei Verbindungsunterbrechung und vorangegangenen exit kein Fehler ausgegeben wird
//			-> ist vor allem dann gut, wenn noch commands in der Queue sind.
// 18.06.2011 - mspoerr
//		- initVerbinderVars: antwort ="" hinzugefügt, da nicht immer leer bei neuen Verbindunden
//		- SSHVerbinder: Timeout Verhalten umgebaut:
//			-> Debug erweitert, damit den Status vom Timeout sieht
//			-> Weitere Details angepasst
// 19.06.2011 - mspoerr
//		- statConfig: Löschen aller \r
//		- TelnetVerbinder: Timeout Verhalten analog zu SSHVerbinder umgebaut
// 23.06.2011 - mspoerr
//		- TelnetVerbinder: Checks erweitert, um festzustellen, ob die Telnet Verbindung abgebaut wurde
// 26.06.2011 - mspoerr
//		- xxxBufTester: Check eingebaut, falls ":" zurückgegeben wird, da dann der loginBufTester irrtümlicherweise übersprungenworden sein könnte
// 27.06.2011 - mspoerr
//		- xxxBufTester: Checks erweitert, dass nur bei "assword:" oder "sername:" zurückgeschwenkt wird auf loginBufTester
// 23.07.2011 - mspoerr
//		- IOSbufTester: "case 0x20": Check eingebaut, ob der Buffer lange genug ist
// 31.07.2011 - mspoerr
//		- mapperConf: Config erweitert
// 03.08.2011 - mspoerr
//		- Neue Debugs: Hex; Special1, Special2
//		- BufTest Debug erweitert
// 04.08.2011 - mspoerr
//		- xxxxConf Funktionen: Das zusätzliche "\n" gelöscht, da dann zwei ENTER gesendet wurden und es daher bei Multihop zu Problemen gekommen ist
// 09.09.2011 - mspoerr
//		- iosBufTester: Fehlererkennung angepasst -> antwort statt eBuf als Quelle
// 01.10.2011 - mspoerr
//		- "antwort += eBuf" von outBufZusammensteller in die Verbinder Funktionen verschoben
//		- outBufZusammensteller: antwort bei FEHLERMELDUNG zurücksetzen
// 24.10.2011 - mspoerr
//		- mapperConf um "show interfaces switchport" erweitert
// 18.11.2011 - mspoerr
//		- httpVerbinder und httpsVerbinderAsio: while Schleife für config, damit mehrere Commands gesendet werden können
//		- httpVerbinder und httpsVerbinderAsio: Sleep in Read Schleife, damit es keine Timeouts gibt.
//		- mapperConfIPT() hinzugefügt
// 19.12.2011 - mspoerr
//		- telnetOptionenFinder: default Pfad vom switch: memcpy gegen absolute Werte getauscht.
//		- TelnetOptionenFinder und telnetOptionen: sp1 Debug aktiviert
// 23.12.2011 - mspoerr
//		- iosBufTester: Neue Abfrage für "[y/n] :" eingefügt
// 27.12.2011 - mspoerr
//		- telnetVerbindung: exception Handling und Fehler-Erkennung bei "write" eingeführt (Daten senden)
//		- Asio 1.5.3 aktiviert (statt 1.4.8)
//		- loginBufTester: Authentication failed im Default-Zweig hinzugefügt
// 28.12.2011 - mspoerr
//		- httpsVerbindungAsio: Authentication eingebaut
// 13.01.2012 - mspoerr
//		- loginTest: check am Ende eingeführt, dass bei ulez == ulezMax ulez zurückgesetzt wird;
// 21.01.2012 - mspoerr
//		- httpVerbindung und httpsVerbindungAsio: HTTP Fehlercode entschärft. Verbindung wird nicht mehr abgebaut.
// 22.01.2012 - mspoerr
//		- httpVerbindung und httpsVerbindungAsio: Alles vom Buffer auslesen -> mit boost::asio::streambuf::size(), anstatt Rückgabewerte von boost::asio::read
//		- mapperConfIPT: Zusätzlich noch "/" als URL hinzugefügt
// 23.01.2012 - mspoerr
//		- mapperConfIPT: "f2301 = false;" hinzugefügt
// 05.02.2012 - mspoerr
//		- Alle BufTester: "ogin" bei der Abfrage hinzugefügt, wo auf Username/Passwort geprüft wird
//		- loginTest: "ogin" in das Username/Passwort Array hinzugefügt
//					 Die Arraygröße von 2 auf 4 vergrößert und den Typo beim zweiten "assword" ausgebessert.
//		- nxOSBufTester: Bei "switch (eBuf[bufferLaenge-2]) -> case 0x23:" ein break am Ende hinzugefügt
// 04.03.2012 - mspoerr
//		- mapperConf: "show interface switchport" statt "show interfaces switchport"
// 05.03.2012 - mspoerr
//		- templateConfig und NXTERMLEN eingeführt
//		-> Wenn das Mapper Template verwendet wird und auf NX OS verbunden wird, dann als erstes "connect nxos" senden, um im Falle des Falles von UCS auf NXOS zu wechseln
// 09.04.2012 - mspoerr
//		- telnetOptionenFinder: case 0xF0: faLaenge"+3" statt "+1"
//		- telnetOptionenFinder: "option = i+2" statt "option = i"
//		- telnetOptionenFinder: Debugs (special2) erweitert
// 18.05.2012 - mspoerr
//		- ucs_nxos eingeführt
//		- nxosBufTester: Check mittels ucs_nxos, ob connect nxos erfolgreich war, um "exit" in die Config einzufügen
// 21.05.2012 - mspoerr
//		- Alle BufTester: rfind erweitern um ":" bzw ": " -> Erweiterung zu 05.02.2012
// 22.05.2012 - mspoerr
//		- Alle BufTester: loginTest komplett umgebaut:
//			- Check von 2xZeichen vor "ogin", "assword", "sername"; Wenn kein \r\n oder Space, dann überspringen
//			- Check ob "ogin", "assword", "sername" direkt vor dem ":" vorkommt
// 25.05.2012 - mspoerr
//		- Alle BufTester: Zusätzliche Debug Möglichkeiten eingebaut (bufTest), damit festgestellt werden kann, ob die letzten Änderungen keine neuen Probleme bringen
// 06.07.2012 - mspoerr
//		- commandBauenML eingeführt
//		- outBufZusammensteller für commandBauenML abgeändert
// 13.07.2012 - mspoerr
//		- commandBauenML geändert: [rgx] Tag ist nicht sinnvoll, daher [regex] Tag eingeführt
// 16.12.2012 - mspoerr
//		- loginTest: "User" hinzugefügt (für WLC)
//		- loginBufTester: WLC Abfrage hinzugefügt
//		- Neue Funktion: WLCnameauslesen
//		- Neue Funktion: WLCbufTester
//		- boost::algorithm::trim(hostname); bei allen "namenauslesen" Funktionen hinzugefügt
//		- keepWLC eingeführt, damit der bufTester bei der WLC dabeibleibt
// 17.12.2012 - mspoerr
//		- httpsVerbindungAsio, httpsInitAsio deaktiviert
//		- CryptLib 3.4.2 eingeführt
//		- Asio mit OpenSSL deaktiviert
// 20.12.2012 - mspoerr
//		- httpsVerbindung angepasst und erweitert - ist noch nicht ganz fertig
//		- OpenSSL aus der Linker Optionen genommen: D:\Programmieren\OpenSSL-Win32\lib\VC\static UND libeay32MTd.lib; ssleay32MTd.lib
//		- OpenSSL aus den Include Verzeichnissen genommen: D:\Programmieren\OpenSSL-Win32\include
//		- Asio aus den Include Verzeichnissen genommen: D:\Programmieren\asio-1.5.3\include
// 08.03.2013 - mspoerr
//		- Bei "Invalid Enable Password!" wird jetzt EXIT anstatt WZNIP zurückgegeben
// 12.03.2013 - mspoerr
//		- iosBufTester & pixBufTester: advancedRauteCheck erweitert um vglString1 Abfrage, damit bei Multihop erkannt wird, wann zurück zum ersten Host gesprungen werden muss
//		- NXOSbufTester:  advancedRauteCheck eingeführt
//		- loginTest: Alle "ulez == ulezmax" gegen "ulez >= ulezmax" getauscht
// 19.04.2013 - mspoerr
//		- CLI Protokoll eingeführt
// 05.10.2014 - mspoerr
//		- termLenCounter eingeführt und in den nötigen Funktionen hinzugefügt (outBufZusammensteller, initVerbinderVars)
// 23.11.2014 - mspoerr
//		- showAppendDate eingeführt und setShowAppend() erweitert; outBufZusammensteller entsprechend erweitert
// 01.10.2015 - mspoerr
//		- mapperConfig erweitert (dotx)
// 11.12.2015 - mspoerr
//		- outBufTester: Neuer Command "!RGXCLEARMEM" eingeführt, damit der rgxMem Speicher manuell zurückgesetzt werden kann
// 06.09.2016 - mspoerr
//		- !ENDIF und !ENDELSE eingeführt
//		- !IFRETURN und !ENDRETURN angepasst

#include "stdwx.h"

#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>    

#include "class_verbinder.h"
#include "base64.h"

//# include "sshlib.h"
#include "libsshpp.hpp"


// Konstruktor:
//*************
// Bei Initialisierung eines neuen Verbinders muss ein log-File angegeben werden. Dieses log-File wird dann geöffnet,
// damit alle Verbinder Funktionen in ein log File schreiben können, ohne es jedesmal öffnen und schließen zu müssen.
// Es werden alle wichtigen Variablen initialisert.
Verbinder::Verbinder(WkLog *logA, Wke *wkV, boost::asio::io_service& io_service/*, boost::asio::ssl::context &ctx*/) 
	: ioserv(io_service)
	, verbindungsSock(io_service)
	, serPort(io_service)
{
	debugSenden = false;
	debugEmpfangen = false;
	debugBufTester = false;
	debugFunctionCall = false;
	debugHex = false;
	debugRegex = false;
	debugSpecial1 = false;
	debugSpecial2 = false;
	debugHostname = false;
	debugShowAusgabe = false;
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
	ifreturn = false;
	rmCR = false;
	advancedRauteCheck = true;
	templateConfig = false;

	steuerzeichenGefunden = false;
	dollarGefunden = false;

	NICHTSaendertBufTest = 0;
	bufTestAenderung = false;

	outBuf = new char[BUFFER_SIZE];
	outBufZaehler = empfangsOffset = 0;

	shString = "";
	showPattern = "";
	customShowName = "";
	showAppend = false;
	showAppendDate = true;

	nichtsString = "";

	antwort = "";

	hostname = "";
	hostnameAlt = "";
	hostnameShowOutput = "";
	descr = "";
	user = "";

	ulez = ulezmax = 0;
	schonAuthe = false;

	sshVersion = 2;

	timeoutCounter = 2;

	nullEaterEnable = false;

	dummyCommand = false;

	wznip = 0;

	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - CONSTRUCTOR>\n", DEBUGFEHLER);
	}
	// 	troubleshooting.rdbuf()->open("d:\\Cisco\\wktoolsTroubleshooting.txt", std::ios_base::out | std::ios_base::app | std::ios_base::binary);
	// 	trcounter = 0;

}

// Destruktor:
//************
// Freimachen des Speichers, der vom Konstruktor allokiert wurde und Schließen des Log Files.
Verbinder::~Verbinder()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - DESTRUCTOR>\n", DEBUGFEHLER);
	}
}

// Definition von BUFFER_SIZE
const uint Verbinder::BUFFER_SIZE = 1025;

// Definition der nötigen Funktionszeiger
uint(Verbinder::*bufTester)(std::string, uint);
void(Verbinder::*nameAusleser)(std::string);


// setSockEinstellungen:
//**********************
// Funktion zum Auflösen der IP Adressen
bool Verbinder::setSockEinstellungen(std::string ipAdr, std::string port)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setSockEinstellungen>\n", DEBUGFEHLER);
	}

	try
	{
		tcp::resolver resolver(ioserv);
		tcp::resolver::query query(ipAdr, port);
		endpoint_iterator = resolver.resolve(query);
	}
	catch (std::exception& e)
	{
		std::string error = "2214: Could not connect to " + ipAdr + "/" + port + ": Most likely IP Address/hostname format problem";
		schreibeLog(error, SYSTEMFEHLER, "2214");
		return false;
	}

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
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - telnetOptionen>\n", DEBUGFEHLER);
	}
	if (debugSpecial1)
	{
		// HEX Ausgabe von den telnetOptionen
		std::stringstream hValStr;
		for (int i=0; i < strlen(buf); i++)
		{
			int hValInt = buf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <telnetOptionen HEX><" + hValStr.str() + ">", DEBUGFEHLER);
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

	if (debugSpecial2)
	{
		// HEX Ausgabe von den telnetOptionen
		std::stringstream hValStr;
		for (int i=0; i < outBufZaehler; i++)
		{
			int hValInt = outBuf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <telnetOptionenSend HEX><" + hValStr.str() + ">", DEBUGFEHLER);
	}

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
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - telnetOptionenFinder>\n", DEBUGFEHLER);
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
				memcpy(telOpt, faZeiger, faLaenge + 2);
				*(telOpt + faLaenge + 2);
				option = i+2;
				
				if (debugSpecial2)
				{
					// Detaillierte Ausgabe der Suboptionen
					std::stringstream subDbgAusgabe;
					subDbgAusgabe << "\n2718: <telnetOptionenFinder SubOpts><faLaenge: " << boost::lexical_cast<std::string>(faLaenge) << "><telOpt: ";
					for (int i=0; i < option; i++)
					{
						int hValInt = (char)telOpt[i];
						subDbgAusgabe << "0x" << std::hex << hValInt << " ";
					}
					subDbgAusgabe << ">\n";
					schreibeLog(subDbgAusgabe.str(), DEBUGFEHLER);
				}

				break;
			default:
				telOpt[option] = 0xFF;
				telOpt[option+1] = buf[i+1];
				telOpt[option+2] = buf[i+2];
				option = i+3;
				break;
			}
		}
		
		int opt = option - i;
		if (opt <= 0)
		{
			memcpy(daten + datenZaehler, &buf[i], 1);
			datenZaehler++;
		}
	}
	if (telOptGef)
	{
		if (debugSpecial2)
		{
			// HEX Ausgabe von den telnetOptionen
			std::stringstream hValStr;
			for (int i=0; i < option; i++)
			{
				int hValInt = telOpt[i];
				hValStr << "0x" << std::hex << hValInt << " ";
			}
			schreibeLog("\n2718: <telnetOptionenFinder HEX><" + hValStr.str() + ">", DEBUGFEHLER);
		}
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
uint Verbinder::IOSbufTester(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - IOSbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("\n2702: <IOSbufTester>", DEBUGFEHLER);
	}

	size_t bufferLaenge = eBuf.length();

	if (bufferLaenge)
	{
		// antwort auf bekannte Fehlermeldungen überprüfen
		if (antwort.find("% Invalid input detected at '^' marker.") != antwort.npos)
		{
			bufStat = FEHLERMELDUNG;
		}
		else if (antwort.find("% Unknown command or computer name, or unable to find computer address") != antwort.npos)
		{
			bufStat = FEHLERMELDUNG;
		}
		if (bufStat == FEHLERMELDUNG)
		{
			bufStat = FEHLERMELDUNG;			
			std::size_t prozentPos = antwort.find("%");
			std::size_t endePos = antwort.find_first_of("\r\n", prozentPos);
			if (prozentPos && (prozentPos != antwort.npos))
			{
				std::string fehlermeldung = "2301: ";
				fehlermeldung += antwort.substr(prozentPos, endePos);
				fehlermeldung += " // Last command: ";
				fehlermeldung += zuletztGesendet;
				if (f2301)
				{
					schreibeLog(fehlermeldung, FEHLER, "2301");
				}
				else
				{
					schreibeLog(fehlermeldung, INFO);
				}
			}
		}
		else
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
					if (eBuf.length() > 1)
					{
						if (eBuf[bufferLaenge-2] == 0x29)		// Kommt vor der "#" eine ")" vor, dann wird "configMode" gesetzt
						{
							configMode = true;
							bufStat = COMMAND;
						}
						else
						{
							if (advancedRauteCheck)
							{
								// Check, ob im Prompt Teile vom Hostnamen vorkommen
								std::string vglString = "";
								std::string vglString1 = "";		// Falls HostnameAlt notwendig
								size_t startPos = 0;
								size_t startPos1 = 0;
								if (eBuf.length() < hostname.length()+1)
								{
									vglString = hostname.substr(hostname.length()-eBuf.length()+1, eBuf.length()-1);
									// BSP:
									// eBuf:     RO01#
									// Hostname: ATFSLDCRO01
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<IOSbufTester - advancedRauteCheck 1>>\nvglString: " + vglString;
										schreibeLog(logm, DEBUGFEHLER);

									}
								}
								else
								{
									vglString = hostname;
									startPos = eBuf.size()-hostname.size()-3;
									if (startPos < 0)
									{
										startPos = 0;
									}
									else if (startPos > eBuf.size())
									{
										startPos = 0;
									}
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<IOSbufTester - advancedRauteCheck 2>>\nvglString: " + vglString;
										logm += "\n<startPos><eBuf.size><hostname.size> <";
										logm += boost::lexical_cast<std::string>(startPos) + "><";
										logm += boost::lexical_cast<std::string>(eBuf.size()) + "><";
										logm += boost::lexical_cast<std::string>(hostname.size()) + ">\n";
										schreibeLog(logm, DEBUGFEHLER);
									}
								}
								
								if (eBuf.length() < hostnameAlt.length()+1)
								{
									vglString1 = hostnameAlt.substr(hostnameAlt.length()-eBuf.length()+1, eBuf.length()-1);
									// BSP:
									// eBuf:     RO01#
									// Hostname: ATFSLDCRO01
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<IOSbufTester - advancedRauteCheck 1>>\nvglString1: " + vglString1;
										schreibeLog(logm, DEBUGFEHLER);

									}
								}
								else
								{
									vglString1 = hostnameAlt;
									startPos1 = eBuf.size()-hostnameAlt.size()-3;
									if (startPos1 < 0)
									{
										startPos1 = 0;
									}
									else if (startPos1 > eBuf.size())
									{
										startPos1 = 0;
									}
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<IOSbufTester - advancedRauteCheck 2>>\nvglString1: " + vglString1;
										logm += "\n<startPos><eBuf.size><hostname.size> <";
										logm += boost::lexical_cast<std::string>(startPos) + "><";
										logm += boost::lexical_cast<std::string>(eBuf.size()) + "><";
										logm += boost::lexical_cast<std::string>(hostname.size()) + ">\n";
										schreibeLog(logm, DEBUGFEHLER);
									}
								}


								if (eBuf.find(vglString, startPos) != eBuf.npos)
								{
									configMode = false;
									bufStat = COMMAND;
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<IOSbufTester - advancedRauteCheck 3>>\nvglString: Hostname found";
										schreibeLog(logm, DEBUGFEHLER);
									}
								}
								else if (eBuf.find(vglString1, startPos1) != eBuf.npos && config.empty())
								{
									configMode = false;
									bufStat = COMMAND;
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<IOSbufTester - advancedRauteCheck 4>>\nvglString1: Old Hostname found";
										schreibeLog(logm, DEBUGFEHLER);
									}
								}
							}
							else
							{
								configMode = false;
								bufStat = COMMAND;
							}
						}
					}
					else
					{
						configMode = false;
						bufStat = COMMAND;
					}
					break;
				}
				break;
			case 0x20:				// SPACE -> kommt bei SW Upgrade vor
				if (debugBufTester)
				{
					schreibeLog("\n2702: <IOSbufTester>", DEBUGFEHLER);
				}

				if (bufferLaenge > 2)
				{
					if (eBuf.rfind("[yes/no]: ") != eBuf.npos)
					{
						bufStat = YES;
					}
					else if (eBuf.rfind("[y/n] :") != eBuf.npos)
					{
						bufStat = YES;
					}
					else if (eBuf.rfind("\x5D\x3F\x20") != eBuf.npos)		// "]? "
					{
						bufStat = COMMAND;
					}
					else if (eBuf[bufferLaenge-2] == ':')			// Loginaufforderung
					{
						std::size_t p1 = eBuf.rfind("assword: ", bufferLaenge);
						std::size_t p2 = eBuf.rfind("sername: ", bufferLaenge);
						std::size_t p3 = eBuf.rfind("ogin: ", bufferLaenge);
						bool btc_true = false;
						bool btc_dbg = false;

						if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
						{
							btc_dbg = true;
						}

						if (p1 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								// TODO: DBG EINFÜGEN UND TESTEN
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p2 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p3 == bufferLaenge-6)
						{
							btc_dbg = true;
							if (bufferLaenge > 7)
							{
								if (eBuf[bufferLaenge-8] == '\n')
								{
									btc_true = true;
								}
							}
						}
						
						if (btc_dbg)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
							}
						}
						if (btc_true)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange IOS #2>", DEBUGFEHLER);
							}
							bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
							hostnameAuslesen = true;
							bufStat = loginTest(eBuf, bufStat);
						}
						else
						{
							bufStat = NICHTS;
						}
					}
					else
					{
						bufStat = NICHTS;
					}
				}
				break;
			case 0x5D:				// ] -> kommt bei Reload, Config Overwrite usw. vor
				if (eBuf.rfind("confirm]") != eBuf.npos)
				{
					bufStat = YESY;
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
			case 0x3A:		// : -> Bei IOS login Banner kommt der : ohne nachfolgendes SPACE
				{
					std::size_t p1 = eBuf.rfind("assword:", bufferLaenge);
					std::size_t p2 = eBuf.rfind("sername:", bufferLaenge);
					std::size_t p3 = eBuf.rfind("ogin:", bufferLaenge);
					bool btc_true = false;
					bool btc_dbg = false;
					if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
					{
						btc_dbg = true;
					}
					if (p1 == bufferLaenge-8)
					{
						btc_dbg = true;
						if (bufferLaenge > 9)
						{
							if (eBuf[bufferLaenge-10] == '\n')
							{
								btc_true = true;
							}
						}
					}
					else if (p2 == bufferLaenge-8)
					{
						btc_dbg = true;
						if (bufferLaenge > 9)
						{
							if (eBuf[bufferLaenge-10] == '\n')
							{
								btc_true = true;
							}
						}
					}
					else if (p3 == bufferLaenge-5)
					{
						btc_dbg = true;
						if (bufferLaenge > 6)
						{
							if (eBuf[bufferLaenge-7] == '\n')
							{
								btc_true = true;
							}
						}
					}

					if (btc_dbg)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
						}
					}
					if (btc_true)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange #1>", DEBUGFEHLER);
						}
						bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
						hostnameAuslesen = true;
						bufStat = loginTest(eBuf, bufStat);
					}
					else if (eBuf.rfind("[y/n] :") != eBuf.npos)
					{
						bufStat = YES;
					}
					else
					{
						bufStat = NICHTS;
					}
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
		std::string bla9 = boost::lexical_cast<std::string>(bufStat);
		std::string bla10 = boost::lexical_cast<std::string>(STATUS);
		std::string bla11 = "\n2703: <IOSBufTester><" + bla9;
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
	if (debugHex)
	{
		// HEX Ausgabe vom eBuf
		std::stringstream hValStr;
		for (int i=0; i < eBuf.length(); i++)
		{
			int hValInt = (char)eBuf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
	}

	return bufStat;
}


// PIXbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von PIXen.
uint Verbinder::PIXbufTester(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - PIXbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("\n2702: <PIXbufTester>", DEBUGFEHLER);
	}	
	
	size_t bufferLaenge = eBuf.length();
	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("ERROR: % Invalid input detected at '^' marker.") != eBuf.npos)
	{
		if (zuletztGesendet.find("pager line") == zuletztGesendet.npos)
		{
			std::string fehlermeldung = "2301: ERROR: % Invalid input detected at '^' marker";
			fehlermeldung += zuletztGesendet;

			if (f2301)
			{
				schreibeLog(fehlermeldung, FEHLER, "2301");
			}
			else
			{
				schreibeLog(fehlermeldung, INFO);
			}
			bufStat = FEHLERMELDUNG;			
		}
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
			if (eBuf[bufferLaenge-1] == ':')
			{
				std::size_t p1 = eBuf.rfind("assword:", bufferLaenge);
				std::size_t p2 = eBuf.rfind("sername:", bufferLaenge);
				std::size_t p3 = eBuf.rfind("ogin:", bufferLaenge);
				bool btc_true = false;
				bool btc_dbg = false;
				if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
				{
					btc_dbg = true;
				}


				if (p1 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p2 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p3 == bufferLaenge-5)
				{
					btc_dbg = true;
					if (bufferLaenge > 6)
					{
						if (eBuf[bufferLaenge-7] == '\n')
						{
							btc_true = true;
						}
					}
				}

				if (btc_dbg)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
					}
				}
				if (btc_true)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange PIX #1>", DEBUGFEHLER);
					}
					bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
					hostnameAuslesen = true;
					bufStat = loginTest(eBuf, bufStat);
				}
				else
				{
					bufStat = NICHTS;
				}
			}
			else
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
						if (eBuf.length() > 2)
						{
							if (eBuf[bufferLaenge-3] == 0x29)		// Kommt vor der "#" eine ")" vor, dann wird "configMode" gesetzt
							{
								configMode = true;
								bufStat = COMMAND;
							}
							else
							{
								if (advancedRauteCheck)
								{
									// Check, ob im Prompt Teile vom Hostnamen vorkommen
									std::string vglString = "";
									size_t startPos = 0;
									std::string vglString1 = "";		// Falls HostnameAlt notwendig
									size_t startPos1 = 0;
									if (eBuf.length() < hostname.length()+1)
									{
										vglString = hostname.substr(hostname.length()-eBuf.length()+1, eBuf.length()-1);
										// BSP:
										// eBuf:     RO01#
										// Hostname: ATFSLDCRO01
										if (debugHostname)
										{
											std::string logm = "\n\n2711: <<PIXbufTester - advancedRauteCheck 1>>\nvglString: " + vglString;
											schreibeLog(logm, DEBUGFEHLER);

										}
									}
									else
									{
										vglString = hostname;
										startPos = eBuf.size()-hostname.size()-3;
										if (startPos < 0)
										{
											startPos = 0;
										}
										else if (startPos > eBuf.size())
										{
											startPos = 0;
										}

										if (debugHostname)
										{
											std::string logm = "\n\n2711: <<PIXbufTester - advancedRauteCheck 2>>\nvglString: " + vglString;
											logm += "\n<startPos><eBuf.size><hostname.size> <";
											logm += boost::lexical_cast<std::string>(startPos) + "><";
											logm += boost::lexical_cast<std::string>(eBuf.size()) + "><";
											logm += boost::lexical_cast<std::string>(hostname.size()) + ">\n";
											schreibeLog(logm, DEBUGFEHLER);
										}

									}

									if (eBuf.length() < hostnameAlt.length()+1)
									{
										vglString1 = hostnameAlt.substr(hostnameAlt.length()-eBuf.length()+1, eBuf.length()-1);
										// BSP:
										// eBuf:     RO01#
										// Hostname: ATFSLDCRO01
										if (debugHostname)
										{
											std::string logm = "\n\n2711: <<PIXbufTester - advancedRauteCheck 1>>\nvglString1: " + vglString1;
											schreibeLog(logm, DEBUGFEHLER);
										}
									}
									else
									{
										vglString1 = hostnameAlt;
										startPos1 = eBuf.size()-hostnameAlt.size()-3;
										if (startPos1 < 0)
										{
											startPos1 = 0;
										}
										else if (startPos1 > eBuf.size())
										{
											startPos1 = 0;
										}
										if (debugHostname)
										{
											std::string logm = "\n\n2711: <<PIXbufTester - advancedRauteCheck 2>>\nvglString1: " + vglString1;
											logm += "\n<startPos><eBuf.size><hostname.size> <";
											logm += boost::lexical_cast<std::string>(startPos) + "><";
											logm += boost::lexical_cast<std::string>(eBuf.size()) + "><";
											logm += boost::lexical_cast<std::string>(hostname.size()) + ">\n";
											schreibeLog(logm, DEBUGFEHLER);
										}
									}

									if (eBuf.find(vglString, startPos) != eBuf.npos)
									{
										configMode = false;
										bufStat = COMMAND;
										if (debugHostname)
										{
											std::string logm = "\n\n2711: <<PIXbufTester - advancedRauteCheck 3>>\nvglString: Hostname found";
											schreibeLog(logm, DEBUGFEHLER);
										}
									}
									else if (eBuf.find(vglString1, startPos1) != eBuf.npos && config.empty())
									{
										configMode = false;
										bufStat = COMMAND;
										if (debugHostname)
										{
											std::string logm = "\n\n2711: <<PIXbufTester - advancedRauteCheck 4>>\nvglString1: Old Hostname found";
											schreibeLog(logm, DEBUGFEHLER);
										}
									}
								}
								else
								{
									configMode = false;
									bufStat = COMMAND;
								}
							}
						}
						else
						{
							configMode = false;
							bufStat = COMMAND;
						}
						break;
					}
					break;
				case 0x3A:		// :
					{
						std::size_t p1 = eBuf.rfind("assword: ", bufferLaenge);
						std::size_t p2 = eBuf.rfind("sername: ", bufferLaenge);
						std::size_t p3 = eBuf.rfind("ogin: ", bufferLaenge);
						bool btc_true = false;
						bool btc_dbg = false;
						if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
						{
							btc_dbg = true;
						}

						if (p1 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p2 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p3 == bufferLaenge-6)
						{
							btc_dbg = true;
							if (bufferLaenge > 7)
							{
								if (eBuf[bufferLaenge-8] == '\n')
								{
									btc_true = true;
								}
							}
						}

						if (btc_dbg)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
							}
						}
						if (btc_true)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange PIX #2>", DEBUGFEHLER);
							}
							bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
							hostnameAuslesen = true;
							bufStat = loginTest(eBuf, bufStat);
						}
						else
						{
							bufStat = NICHTS;
						}
					}
					break;
				default:
					if (antwort.find("\r\nLogoff\r\n\r\n") != antwort.npos)
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
		}
		else
		{
			bufStat = NICHTS;
		}
	}
	if (debugbufTestDetail)
	{
		// Ausgabe: 
		// Feld 1: bufStat
		// Feld 2: STATUS
		// Feld 3: letztes Zeichen
		// Feld 4: vorletztes Zeichen
		// Feld 5: vor-vorletztes Zeichen
		std::string bla9 = boost::lexical_cast<std::string>(bufStat);
		std::string bla10 = boost::lexical_cast<std::string>(STATUS);
		std::string bla11 = "\n2703: <PIXBufTester><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		bla11 += eBuf[bufferLaenge-1];
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
	if (debugHex)
	{
		// HEX Ausgabe vom eBuf
		std::stringstream hValStr;
		for (int i=0; i < eBuf.length(); i++)
		{
			int hValInt = (char)eBuf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
	}

	return bufStat;
}


// WLCbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von WLCs.
uint Verbinder::WLCbufTester(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - WLCbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("\n2702: <WLCbufTester>", DEBUGFEHLER);
	}	
	
	size_t bufferLaenge = eBuf.length();
	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("Incorrect usage.  Use the '?' or <TAB> key to list commands.") != eBuf.npos)
	{
		std::string fehlermeldung = "2301: Incorrect usage.  Use the '?' or <TAB> key to list commands.";
		fehlermeldung += zuletztGesendet;

		if (f2301)
		{
			schreibeLog(fehlermeldung, FEHLER, "2301");
		}
		else
		{
			schreibeLog(fehlermeldung, INFO);
		}
		bufStat = FEHLERMELDUNG;			
	}

	if (bufStat != FEHLERMELDUNG)
	{
		if (eBuf.find("Press Enter to continue...") != eBuf.npos)
		{
			// ENTER senden
			bufStat = ENTER;
		}
		else
		{
			// * 0x3E: > : exec mode bei Cisco WLCs. Jedes Mal, wenn dieses Zeichen am Ende des Streams vorkommt,
			//		wird eine Befehlszeile geschickt, und zwar so oft, bis das Ende der zu schickenden
			//		Konfiguration erreicht ist.
			// * default: Falls keines der oben genannten Zeichen vorkommt, wird nichts zum Gegenüber 
			//		geschickt und auf weitere Pakete gewartet.

			size_t bufferLaenge = eBuf.length();

			if (bufferLaenge > 1)
			{
				if (eBuf[bufferLaenge-1] == ':')
				{
					std::size_t p1 = eBuf.rfind("assword:", bufferLaenge);
					std::size_t p2 = eBuf.rfind("User:", bufferLaenge);
					bool btc_true = false;
					bool btc_dbg = false;
					if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos))
					{
						btc_dbg = true;
					}


					if (p1 == bufferLaenge-8)
					{
						btc_dbg = true;
						if (bufferLaenge > 9)
						{
							if (eBuf[bufferLaenge-10] == '\n')
							{
								btc_true = true;
							}
						}
					}
					else if (p2 == bufferLaenge-5)
					{
						btc_dbg = true;
						if (bufferLaenge > 6)
						{
							if (eBuf[bufferLaenge-7] == '\n')
							{
								btc_true = true;
							}
						}
					}

					if (btc_dbg)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
						}
					}
					if (btc_true)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange WLC #1>", DEBUGFEHLER);
						}
						
						bufTester = &Verbinder::loginBufTester;	
						hostnameAuslesen = true;
						bufStat = loginTest(eBuf, bufStat);
					}
					else
					{
						bufStat = NICHTS;
					}
				}
				else
				{
					switch (eBuf[bufferLaenge-1])
					{
					case 0x3E:			// enable und config modes: >
						// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
						switch (iosAnfangsSettings)
						{
						case CONFPAGE:
							bufStat = CONFPAGE;
							iosAnfangsSettings = NICHTS;
							break;
						default:
							if (eBuf.length() > 2)
							{
								if (eBuf[bufferLaenge-3] == 0x29)		// Kommt vor ">" eine ")" vor, dann wird "configMode" gesetzt
								{
									configMode = true;
									bufStat = COMMAND;
								}
								else
								{
									if (advancedRauteCheck)
									{
										// Check, ob im Prompt Teile vom Hostnamen vorkommen
										std::string vglString = "";
										size_t startPos = 0;
										if (eBuf.length() < hostname.length()+1)
										{
											vglString = hostname.substr(hostname.length()-eBuf.length()+1, eBuf.length()-1);
											// BSP:
											// eBuf:     RO01#
											// Hostname: ATFSLDCRO01
											if (debugHostname)
											{
												std::string logm = "\n\n2711: <<WLCbufTester - advancedRauteCheck 1>>\nvglString: " + vglString;
												schreibeLog(logm, DEBUGFEHLER);

											}
										}
										else
										{
											vglString = hostname;
											startPos = eBuf.size()-hostname.size()-3;
											if (startPos < 0)
											{
												startPos = 0;
											}
											else if (startPos > eBuf.size())
											{
												startPos = 0;
											}

											if (debugHostname)
											{
												std::string logm = "\n\n2711: <<WLCbufTester - advancedRauteCheck 2>>\nvglString: " + vglString;
												logm += "\n<startPos><eBuf.size><hostname.size> <";
												logm += boost::lexical_cast<std::string>(startPos) + "><";
												logm += boost::lexical_cast<std::string>(eBuf.size()) + "><";
												logm += boost::lexical_cast<std::string>(hostname.size()) + ">\n";
												schreibeLog(logm, DEBUGFEHLER);
											}

										}
										if (eBuf.find(vglString, startPos) != eBuf.npos)
										{
											configMode = false;
											bufStat = COMMAND;
											if (debugHostname)
											{
												std::string logm = "\n\n2711: <<WLCbufTester - advancedRauteCheck 3>>\nvglString: Hostname found";
												schreibeLog(logm, DEBUGFEHLER);
											}
										}
									}
									else
									{
										configMode = false;
										bufStat = COMMAND;
									}
								}
							}
							else
							{
								configMode = false;
								bufStat = COMMAND;
							}
							break;
						}
						break;
					case 0x3A:		// :
						{
							std::size_t p1 = eBuf.rfind("assword: ", bufferLaenge);
							std::size_t p3 = eBuf.rfind("User: ", bufferLaenge);
							bool btc_true = false;
							bool btc_dbg = false;
							if ((p1 !=eBuf.npos) || (p3 !=eBuf.npos))
							{
								btc_dbg = true;
							}

							if (p1 == bufferLaenge-9)
							{
								btc_dbg = true;
								if (bufferLaenge > 10)
								{
									if (eBuf[bufferLaenge-11] == '\n')
									{
										btc_true = true;
									}
								}
							}
							else if (p3 == bufferLaenge-6)
							{
								btc_dbg = true;
								if (bufferLaenge > 7)
								{
									if (eBuf[bufferLaenge-8] == '\n')
									{
										btc_true = true;
									}
								}
							}

							if (btc_dbg)
							{
								if (debugBufTester)
								{
									schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
								}
							}
							if (btc_true)
							{
								if (debugBufTester)
								{
									schreibeLog("\n2702: <bufTestChange WLC #2>", DEBUGFEHLER);
								}
								bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
								hostnameAuslesen = true;
								bufStat = loginTest(eBuf, bufStat);
							}
							else
							{
								bufStat = NICHTS;
							}
						}
						break;
					default:
						bufStat = NICHTS;
						break;
					}		
				}
			}
			else if (bufferLaenge > 2)
			{
				if (eBuf[bufferLaenge-2] == ')')
				{
					if (eBuf.find("? (y/N)") != eBuf.npos)
					{
						bufStat = YESY;
					}
				}

			}
			else
			{
				bufStat = NICHTS;
			}
		}
	}
	if (debugbufTestDetail)
	{
		// Ausgabe: 
		// Feld 1: bufStat
		// Feld 2: STATUS
		// Feld 3: letztes Zeichen
		// Feld 4: vorletztes Zeichen
		// Feld 5: vor-vorletztes Zeichen
		std::string bla9 = boost::lexical_cast<std::string>(bufStat);
		std::string bla10 = boost::lexical_cast<std::string>(STATUS);
		std::string bla11 = "\n2703: <WLCbufTester><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		bla11 += eBuf[bufferLaenge-1];
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
	if (debugHex)
	{
		// HEX Ausgabe vom eBuf
		std::stringstream hValStr;
		for (int i=0; i < eBuf.length(); i++)
		{
			int hValInt = (char)eBuf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
	}

	return bufStat;
}


// NXOSbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von NX OS Boxen.
uint Verbinder::NXOSbufTester(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - NXOSbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("\n2702: <NXOSbufTester>", DEBUGFEHLER);
	}	

	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("% Invalid command at '^' marker.") != eBuf.npos)
	{
		std::string fehlermeldung = "2301: % Invalid command at '^' marker.";
		fehlermeldung += zuletztGesendet;

		if (f2301)
		{
			schreibeLog(fehlermeldung, FEHLER, "2301");
		}
		else
		{
			schreibeLog(fehlermeldung, INFO);
		}
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
			if (eBuf[bufferLaenge-1] == ':')
			{
				std::size_t p1 = eBuf.rfind("assword:", bufferLaenge);
				std::size_t p2 = eBuf.rfind("sername:", bufferLaenge);
				std::size_t p3 = eBuf.rfind("ogin:", bufferLaenge);
				bool btc_true = false;
				bool btc_dbg = false;
				if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
				{
					btc_dbg = true;
				}
				
				if (p1 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p2 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p3 == bufferLaenge-5)
				{
					btc_dbg = true;
					if (bufferLaenge > 6)
					{
						if (eBuf[bufferLaenge-7] == '\n')
						{
							btc_true = true;
						}
					}
				}

				if (btc_dbg)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
					}
				}
				if (btc_true)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange NXOS #1>", DEBUGFEHLER);
					}
					bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
					hostnameAuslesen = true;
					bufStat = loginTest(eBuf, bufStat);
				}
				else
				{
					bufStat = NICHTS;
				}
			}
			else
			{
				switch (eBuf[bufferLaenge-2])
				{
				case 0x23:			// enable und config modes: #
					// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
					switch (iosAnfangsSettings)
					{
					case TERMLEN:
						bufStat = NXTERMLEN;
						iosAnfangsSettings = NICHTS;
						break;
					case NXTERMLEN:
						bufStat = NXTERMLEN;
						iosAnfangsSettings = NICHTS;
						break;
					default:
						if (eBuf[bufferLaenge-3] == 0x29)		// Kommt vor der "#" eine ")" vor, dann wird "configMode" gesetzt
						{
							configMode = true;
							bufStat = COMMAND;
							if ((eBuf.find("(nxos)") != eBuf.npos) && !ucs_nxos)
							{
								config.push_back("exit");
								ucs_nxos = true;
							}
						}
						else
						{
							if (advancedRauteCheck)
							{
								// Check, ob im Prompt Teile vom Hostnamen vorkommen
								std::string vglString = "";
								size_t startPos = 0;
								std::string vglString1 = "";		// Falls HostnameAlt notwendig
								size_t startPos1 = 0;
								if (eBuf.length() < hostname.length()+1)
								{
									vglString = hostname.substr(hostname.length()-eBuf.length()+1, eBuf.length()-1);
									// BSP:
									// eBuf:     RO01#
									// Hostname: ATFSLDCRO01
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<NXOSbufTester - advancedRauteCheck 1>>\nvglString: " + vglString;
										schreibeLog(logm, DEBUGFEHLER);

									}
								}
								else
								{
									vglString = hostname;
									startPos = eBuf.size()-hostname.size()-3;
									if (startPos < 0)
									{
										startPos = 0;
									}
									else if (startPos > eBuf.size())
									{
										startPos = 0;
									}

									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<NXOSbufTester - advancedRauteCheck 2>>\nvglString: " + vglString;
										logm += "\n<startPos><eBuf.size><hostname.size> <";
										logm += boost::lexical_cast<std::string>(startPos) + "><";
										logm += boost::lexical_cast<std::string>(eBuf.size()) + "><";
										logm += boost::lexical_cast<std::string>(hostname.size()) + ">\n";
										schreibeLog(logm, DEBUGFEHLER);
									}

								}

								if (eBuf.length() < hostnameAlt.length()+1)
								{
									vglString1 = hostnameAlt.substr(hostnameAlt.length()-eBuf.length()+1, eBuf.length()-1);
									// BSP:
									// eBuf:     RO01#
									// Hostname: ATFSLDCRO01
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<NXOSbufTester - advancedRauteCheck 1>>\nvglString1: " + vglString1;
										schreibeLog(logm, DEBUGFEHLER);
									}
								}
								else
								{
									vglString1 = hostnameAlt;
									startPos1 = eBuf.size()-hostnameAlt.size()-3;
									if (startPos1 < 0)
									{
										startPos1 = 0;
									}
									else if (startPos1 > eBuf.size())
									{
										startPos1 = 0;
									}
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<NXOSbufTester - advancedRauteCheck 2>>\nvglString1: " + vglString1;
										logm += "\n<startPos><eBuf.size><hostname.size> <";
										logm += boost::lexical_cast<std::string>(startPos) + "><";
										logm += boost::lexical_cast<std::string>(eBuf.size()) + "><";
										logm += boost::lexical_cast<std::string>(hostname.size()) + ">\n";
										schreibeLog(logm, DEBUGFEHLER);
									}
								}

								if (eBuf.find(vglString, startPos) != eBuf.npos)
								{
									configMode = false;
									bufStat = COMMAND;
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<NXOSbufTester - advancedRauteCheck 3>>\nvglString: Hostname found";
										schreibeLog(logm, DEBUGFEHLER);
									}
								}
								else if (eBuf.find(vglString1, startPos1) != eBuf.npos && config.empty())
								{
									configMode = false;
									bufStat = COMMAND;
									if (debugHostname)
									{
										std::string logm = "\n\n2711: <<NXOSbufTester - advancedRauteCheck 4>>\nvglString1: Old Hostname found";
										schreibeLog(logm, DEBUGFEHLER);
									}
								}
							}
							else
							{
								configMode = false;
								bufStat = COMMAND;
							}
						}
						break;
					}
					break;
				case 0x3A:		// :
					{
					std::size_t p1 = eBuf.rfind("assword: ", bufferLaenge);
					std::size_t p2 = eBuf.rfind("sername: ", bufferLaenge);
					std::size_t p3 = eBuf.rfind("ogin: ", bufferLaenge);
					bool btc_true = false;
					bool btc_dbg = false;
					if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
					{
						btc_dbg = true;
					}

					if (p1 == bufferLaenge-9)
					{
						btc_dbg = true;
						if (bufferLaenge > 10)
						{
							if (eBuf[bufferLaenge-11] == '\n')
							{
								btc_true = true;
							}
						}
					}
					else if (p2 == bufferLaenge-9)
					{
						btc_dbg = true;
						if (bufferLaenge > 10)
						{
							if (eBuf[bufferLaenge-11] == '\n')
							{
								btc_true = true;
							}
						}
					}
					else if (p3 == bufferLaenge-6)
					{
						btc_dbg = true;
						if (bufferLaenge > 7)
						{
							if (eBuf[bufferLaenge-8] == '\n')
							{
								btc_true = true;
							}
						}
					}

					if (btc_dbg)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
						}
					}
					if (btc_true)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange NXOS #2>", DEBUGFEHLER);
						}
						bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
						hostnameAuslesen = true;
						bufStat = loginTest(eBuf, bufStat);
					}
					else
					{
						bufStat = NICHTS;
					}
					break;
					}
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
			std::string bla9 = boost::lexical_cast<std::string>(bufStat);
			std::string bla10 = boost::lexical_cast<std::string>(STATUS);
			std::string bla11 = "\n2703: <NXOSBufTester><" + bla9;
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
		if (debugHex)
		{
			// HEX Ausgabe vom eBuf
			std::stringstream hValStr;
			for (int i=0; i < eBuf.length(); i++)
			{
				int hValInt = (char)eBuf[i];
				hValStr << "0x" << std::hex << hValInt << " ";
			}
			schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
		}
	}
	return bufStat;
}


// CATbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von CatOS Geräten.
uint Verbinder::CATbufTester(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - CATbufTester>\n", DEBUGFEHLER);
	}
	if (debugBufTester)
	{
		schreibeLog("\n2702: <CATbufTester>", DEBUGFEHLER);
	}

	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("\x0D\x0AUnknown Command ") != eBuf.npos)
	{
		std::string fehlermeldung = "2301: Unknown Command";
		fehlermeldung += zuletztGesendet;

		if (f2301)
		{
			schreibeLog(fehlermeldung, FEHLER, "2301");
		}
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
			if (eBuf[bufferLaenge-1] == ':')
			{
				std::size_t p1 = eBuf.rfind("assword:", bufferLaenge);
				std::size_t p2 = eBuf.rfind("sername:", bufferLaenge);
				std::size_t p3 = eBuf.rfind("ogin:", bufferLaenge);
				bool btc_true = false;
				bool btc_dbg = false;
				if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
				{
					btc_dbg = true;
				}

				if (p1 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p2 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p3 == bufferLaenge-5)
				{
					btc_dbg = true;
					if (bufferLaenge > 6)
					{
						if (eBuf[bufferLaenge-7] == '\n')
						{
							btc_true = true;
						}
					}
				}

				if (btc_dbg)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
					}
				}
				if (btc_true)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange CatOS #1>", DEBUGFEHLER);
					}
					bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
					hostnameAuslesen = true;
					bufStat = loginTest(eBuf, bufStat);
				}
				else
				{
					bufStat = NICHTS;
				}
			}
			else
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
				case 0x3A:		// :
					{
						std::size_t p1 = eBuf.rfind("assword: ", bufferLaenge);
						std::size_t p2 = eBuf.rfind("sername: ", bufferLaenge);
						std::size_t p3 = eBuf.rfind("ogin: ", bufferLaenge);
						bool btc_true = false;
						bool btc_dbg = false;
						if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
						{
							btc_dbg = true;
						}

						if (p1 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p2 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p3 == bufferLaenge-6)
						{
							btc_dbg = true;
							if (bufferLaenge > 7)
							{
								if (eBuf[bufferLaenge-8] == '\n')
								{
									btc_true = true;
								}
							}
						}

						if (btc_dbg)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
							}
						}
						if (btc_true)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange CatOS #2>", DEBUGFEHLER);
							}
							bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
							hostnameAuslesen = true;
							bufStat = loginTest(eBuf, bufStat);
						}
						else
						{
							bufStat = NICHTS;
						}
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
			std::string bla9 = boost::lexical_cast<std::string>(bufStat);
			std::string bla10 = boost::lexical_cast<std::string>(STATUS);
			std::string bla11 = "\n2703: <CatBufTester><" + bla9;
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
		if (debugHex)
		{
			// HEX Ausgabe vom eBuf
			std::stringstream hValStr;
			for (int i=0; i < eBuf.length(); i++)
			{
				int hValInt = (char)eBuf[i];
				hValStr << "0x" << std::hex << hValInt << " ";
			}
			schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
		}
	}
	return bufStat;
}


// UTAbufTester
//*************
// Funktion zum Auswerten der gesendeten Daten von der UTA Managmentstation.
uint Verbinder::UTAbufTester(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - UTAbufTester>\n", DEBUGFEHLER);
	}

	if (debugBufTester)
	{
		schreibeLog("\n2702: <UTAbufTester>", DEBUGFEHLER);
	}

	// eBuf auf Fehlermeldung überprüfen.
	// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
	// und daher nicht mehr untersucht werden müssen.
	if (eBuf.find("command not found") != eBuf.npos)
	{
		std::string fehlermeldung = "2301: command not found";
		fehlermeldung += zuletztGesendet;

		if (f2301)
		{
			schreibeLog(fehlermeldung, FEHLER, "2301");
		}
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
			if (eBuf[bufferLaenge-1] == ':')
			{
				std::size_t p1 = eBuf.rfind("assword:", bufferLaenge);
				std::size_t p2 = eBuf.rfind("sername:", bufferLaenge);
				std::size_t p3 = eBuf.rfind("ogin:", bufferLaenge);
				bool btc_true = false;
				bool btc_dbg = false;
				if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
				{
					btc_dbg = true;
				}

				if (p1 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p2 == bufferLaenge-8)
				{
					btc_dbg = true;
					if (bufferLaenge > 9)
					{
						if (eBuf[bufferLaenge-10] == '\n')
						{
							btc_true = true;
						}
					}
				}
				else if (p3 == bufferLaenge-5)
				{
					btc_dbg = true;
					if (bufferLaenge > 6)
					{
						if (eBuf[bufferLaenge-7] == '\n')
						{
							btc_true = true;
						}
					}
				}

				if (btc_dbg)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
					}
				}
				if (btc_true)
				{
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange UTA #1>", DEBUGFEHLER);
					}
					bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
					hostnameAuslesen = true;
					bufStat = loginTest(eBuf, bufStat);
				}
				else
				{
					bufStat = NICHTS;
				}
			}
			else
			{
				switch (eBuf[bufferLaenge-2])
				{
				case 0x24:			// enable und config modes: $
					// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
					configMode = false;
					bufStat = COMMAND;
					break;
				case 0x3E:			// enable und config modes: >
					// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
					configMode = false;
					bufStat = COMMAND;
					break;
				case 0x6D:			// enable und config modes: m
					if (bufferLaenge > 4)
					{
						if (eBuf[bufferLaenge-4] == 0x1B)
						{
							configMode = false;
							bufStat = COMMAND;
						}
					}
					break;
				case 0x23:			// enable und config modes: #
					// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
					configMode = false;
					bufStat = COMMAND;
					break;
				case 0x3A:		// :
					{
						std::size_t p1 = eBuf.rfind("assword: ", bufferLaenge);
						std::size_t p2 = eBuf.rfind("sername: ", bufferLaenge);
						std::size_t p3 = eBuf.rfind("ogin: ", bufferLaenge);
						bool btc_true = false;
						bool btc_dbg = false;
						if ((p1 !=eBuf.npos) || (p2 !=eBuf.npos) || (p3 !=eBuf.npos))
						{
							btc_dbg = true;
						}

						if (p1 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p2 == bufferLaenge-9)
						{
							btc_dbg = true;
							if (bufferLaenge > 10)
							{
								if (eBuf[bufferLaenge-11] == '\n')
								{
									btc_true = true;
								}
							}
						}
						else if (p3 == bufferLaenge-6)
						{
							btc_dbg = true;
							if (bufferLaenge > 7)
							{
								if (eBuf[bufferLaenge-8] == '\n')
								{
									btc_true = true;
								}
							}
						}

						if (btc_dbg)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange #-#-#-#-#-#>", DEBUGFEHLER);
							}
						}
						if (btc_true)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange UTA #2>", DEBUGFEHLER);
							}
							bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
							hostnameAuslesen = true;
							bufStat = loginTest(eBuf, bufStat);
						}
						else
						{
							bufStat = NICHTS;
						}
					}
					break;
				case 0x3f:			// Abfrage, ob SSH Verbindung ok: ?
					// Neue Befehle werden nur bei erfolgreichen Verbindungsaufbau gesendet
					if (eBuf.find("Are you sure you want to continue connecting (yes/no)") != eBuf.npos)
					{
						bufStat = YES;
					}
					else
					{
						bufStat = NO;
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
			std::string bla9 = boost::lexical_cast<std::string>(bufStat);
			std::string bla10 = boost::lexical_cast<std::string>(STATUS);
			std::string bla11 = "\n2703: <UTABufTester><" + bla9;
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
		if (debugHex)
		{
			// HEX Ausgabe vom eBuf
			std::stringstream hValStr;
			for (int i=0; i < eBuf.length(); i++)
			{
				int hValInt = (char)eBuf[i];
				hValStr << "0x" << std::hex << hValInt << " ";
			}
			schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
		}
	}
	return bufStat;
}



// loginbufTester
//***************
// Funktion zum Auswerten der gesendeten Daten solange der login Vorgang noch nicht abgeschlossen ist,
// und zum Feststellen der Gerätetype. Zur Zeit kann zwischen UTA Managment Station, IOS, CatOS und PIXOS unterschieden werden.
// Achtung bei CatOS! Nur wenn der Prompt folgendermaßen ein ">" als Abschluss hat, funktioniert das Programm.
uint Verbinder::loginBufTester(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - loginBufTester>\n", DEBUGFEHLER);
	}

	if (debugBufTester)
	{
		schreibeLog("\n2702: <loginBufTester>", DEBUGFEHLER);
	}
	size_t bufferLaenge = eBuf.length();

	if (bufferLaenge > 1)
	{
		// WLC oder nicht
		if (eBuf.rfind(") \r\n") != eBuf.npos)
		{
			// WLC
			keepWLC = true;
			bufStat = loginTest(eBuf, bufStat);
		}
		else if (keepWLC)
		{
			// WLC wurde erkannt; Jetzt muss fertig eingeloggt werden und dann loginModeTest
			switch (eBuf[bufferLaenge-1])
			{
				case 0x3A:		// :
					bufStat = loginTest(eBuf, bufStat);
					break;
				case 0x3E:		// >
					{
						bufTester = &Verbinder::WLCbufTester;
						nameAusleser = &Verbinder::WLCnameauslesen;
						iosAnfangsSettings = CONFPAGE;

						bufStat = enableModeTest(eBuf, bufStat);
					}
					break;
				default:
					bufStat = NICHTS;
					break;
			}
		}
		else
		{
			// Andere
			switch (eBuf[bufferLaenge-1])
			{
			case 0x20:			// Wenn SPACE, dann überprüfen, welches Zeichen davor kommt
				switch (eBuf[bufferLaenge-2])
				{
				case 0x3A:		// :
					bufStat = loginTest(eBuf, bufStat);
					break;
				case 0x3E:		// >
					{
						std::string bufM3 = "";
						if (bufferLaenge > 4)
						{
							bufM3 = eBuf[bufferLaenge-3];
						}
						if (bufM3 == "~" && STATUS != WEITER)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange 0x3E>", DEBUGFEHLER);
							}
							bufTester = &Verbinder::UTAbufTester;
							nameAusleser = &Verbinder::UTAnameauslesen;
							iosAnfangsSettings = NICHTS;
							bufStat = enableModeTest(eBuf, bufStat);
						}
						else if ((eBuf.find('\x00') + 1) < eBuf.size())
						{
							size_t tempos = eBuf.find('\x00') + 1;
							nameAusleser = &Verbinder::PIXnameauslesen;
							if (enPWf == TERMLEN)
							{
								enPWf = PAGERLINE;
							}
							bufStat = loginModeTest(eBuf, bufStat);
						}
						else
						{
							nameAusleser = &Verbinder::CATnameauslesen;
							bufStat = loginModeTest(eBuf, bufStat);
						}
					}
					break;
				case 0x24:		// $
					if (STATUS != WEITER)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange 0x24>", DEBUGFEHLER);
						}
						bufTester = &Verbinder::UTAbufTester;
						nameAusleser = &Verbinder::UTAnameauslesen;
						iosAnfangsSettings = NICHTS;
					}
					bufStat = enableModeTest(eBuf, bufStat);
					break;
				case 0x5B:		// [
					if (STATUS != WEITER)
					{
						if (debugBufTester)
						{
							schreibeLog("\n2702: <bufTestChange 0x5B>", DEBUGFEHLER);
						}
						bufTester = &Verbinder::UTAbufTester;
						nameAusleser = &Verbinder::UTAnameauslesen;
						iosAnfangsSettings = NICHTS;
					}
					bufStat = enableModeTest(eBuf, bufStat);
					break;
				case 0x6D:		// m
					if (STATUS != WEITER && bufferLaenge > 4)
					{
						if (eBuf[bufferLaenge-4] == 0x1B)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange 0x6D>", DEBUGFEHLER);
							}
							bufTester = &Verbinder::UTAbufTester;
							nameAusleser = &Verbinder::UTAnameauslesen;
							iosAnfangsSettings = NICHTS;
						}
					}
					bufStat = enableModeTest(eBuf, bufStat);
					break;
				case 0x23:		// #
					if (STATUS != WEITER)
					{
						if (antwort.find("Cisco Nexus Operating System (NX-OS) Software") != antwort.npos)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange 0x23>", DEBUGFEHLER);
							}
							// NX OS
							nameAusleser = &Verbinder::NXOSnameauslesen;
							iosAnfangsSettings = NXTERMLEN;
							bufTester = &Verbinder::NXOSbufTester;
						}
						else if (eBuf[bufferLaenge-3] == 0x7e)		// "~" -> möglicherweise Unix, aber sicher keine PIX oder ASA
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange 0x7e>", DEBUGFEHLER);
							}
							nameAusleser = &Verbinder::UTAnameauslesen;
							iosAnfangsSettings = NICHTS;
							bufTester = &Verbinder::UTAbufTester;
						}
						else
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange 0x23 #2>", DEBUGFEHLER);
							}
							nameAusleser = &Verbinder::PIXnameauslesen;
							iosAnfangsSettings = TERMLEN;
							bufTester = &Verbinder::PIXbufTester;
						}
					}
					bufStat = enableModeTest(eBuf, bufStat);
					break;
				case 0x29:		// )
					if (eBuf.find("> (enable) "))
					{
						if (STATUS != WEITER)
						{
							if (debugBufTester)
							{
								schreibeLog("\n2702: <bufTestChange 0x29>", DEBUGFEHLER);
							}
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
				case 0x3f:		// Abfrage, ob SSH Verbindung ok: ? -> bei ssh von Unix notwendig
					if (eBuf.find("Are you sure you want to continue connecting (yes/no)") != eBuf.npos)
					{
						bufStat = YES;
					}
					else
					{
						bufStat = NO;
					}
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
					if (debugBufTester)
					{
						schreibeLog("\n2702: <bufTestChange 0x23>", DEBUGFEHLER);
					}
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
				if (eBuf.find("% Authentication failed") != eBuf.npos)
				{
					std::string fehlermeldung = "2322: Connection error with " + ip + ": Authentication Failed.";
					schreibeLog(fehlermeldung, SYSTEMFEHLER, "2322");
					config.clear();
					bufStat = WZNIP;
				}
				else
				{
					bufStat = NICHTS;
				}
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
		std::string bla9 = boost::lexical_cast<std::string>(bufStat);
		std::string bla10 = boost::lexical_cast<std::string>(STATUS);
		std::string bla11 = "\n2703: <loginBufTester><" + bla9;
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
		bla11 += "><";
		if (bufferLaenge > 2)
		{
			bla11 += eBuf[bufferLaenge-3];
		}
		bla11 += ">";

		schreibeLog(bla11, DEBUGFEHLER);
	}
	if (debugHex)
	{
		// HEX Ausgabe vom eBuf
		std::stringstream hValStr;
		for (int i=0; i < eBuf.length(); i++)
		{
			int hValInt = (char)eBuf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
	}

	return bufStat;
}


// loginTest:
//***********
// Testen des Puffers auf User/Pass Abfrage und welches Passwort gefragt wird.
uint Verbinder::loginTest(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - loginTest>\n", DEBUGFEHLER);
	}

	if (debugBufTester)
	{
		schreibeLog("\n2702: <loginTest>", DEBUGFEHLER);
	}

	std::string loginTestString = eBuf;

	if (loginTestString.length() < 8)
	{
		if (debugBufTester)
		{
			schreibeLog("\n2704: <DEBUG loginTest: Using nichtsString>\n", DEBUGFEHLER);
		}
		loginTestString = nichtsString + loginTestString;
		nichtsString = "";
	}


	size_t bufferLaenge = loginTestString.length();

	std::string loginAuswahl[] = {"sername", "assword", "assword", "ogin", "User"};
	uint loginGroesse = 5;

	for (uint i = 0; i < loginGroesse; i++)
	{
		if (loginTestString.find(loginAuswahl[i], bufferLaenge - 9) != loginTestString.npos)
		{
			switch (i)
			{
			case 5:
			case 4:
			case 0:
				if ((bufStat == USERNAME) || (bufStat == FEHLERMELDUNGALT))
				{
					ulez++;
					if (ulez >= ulezmax)
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
					if (ulez >= ulezmax)
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
						if (ulez >= ulezmax)
						{
							bufStat = EXIT;
							fehler = "Invalid Enable Password!";
						}
						else
						{
							bufStat = ENABLEPASS;
						}
					}
				}
				break;
			case 2:
				if (bufStat == LOGINPASS)
				{
					ulez++;
					if (ulez >= ulezmax)
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
						if (ulez >= ulezmax)
						{
							bufStat = EXIT;
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
	if (ulez == ulezmax)
	{
		ulez = 0;
	}
	// Falls eine User/PW Abfrage kommt, wurde eine Verbindung erfolgreich aufgebaut.
	erfolgreich = true;
	return bufStat;
}


// loginModeTest:
//***************
// Testen des Puffers, wenn sich das Gerät im login Modus befindet
uint Verbinder::loginModeTest(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - loginModeTest>\n", DEBUGFEHLER);
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
				case NXTERMLEN:
					bufStat = NXTERMLEN;
					enPWf = COMMAND;
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
				case CONFPAGE:
					bufStat = CONFPAGE;
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
uint Verbinder::enableModeTest(std::string eBuf, uint bufStat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - enableModeTest>\n", DEBUGFEHLER);
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
		std::string bla9 = boost::lexical_cast<std::string>(bufStat);
		std::string bla10 = boost::lexical_cast<std::string>(STATUS);
		std::string bla11 = "\n2703: <enableModeTest><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		bla11 += "><";
		bla11 += ">";
		schreibeLog(bla11, DEBUGFEHLER);
	}
	if (debugHex)
	{
		// HEX Ausgabe vom eBuf
		std::stringstream hValStr;
		for (int i=0; i < eBuf.length(); i++)
		{
			int hValInt = (char)eBuf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
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
uint Verbinder::upFinder(std::string up, uint upwZaehler)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - upFinder>\n", DEBUGFEHLER);
	}

	std::string upw[] = {"!user", "!lopw", "!enpw"};
	std::string userpass;		// Hier wird der User/PW zwischengespeichert
	// Drei Durchläufe, da es drei Sachen zu finden gilt.
	std::string zw;	// temp. Buffer
	for (uint i = upwZaehler; i < 3; i++)
	{
		userpass = strfind(upw[i], up, 8);
		if (!userpass.empty())
		{
			// Falls etwas gefunden wurde, wird nun geschaut, was es ist und der upwZaehler um eins erhöht
			switch (i)
			{
			case 0:
				zw  = userpass + "\n";
				username.push_back(zw);
				i = 3;
				break;
			case 1:
				zw = userpass + "\n";
				loginpass.push_back(zw); 
				i = 3;
				break;
			case 2:
				zw = userpass + "\n";
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
uint Verbinder::outBufZusammensteller(uint bufStat, std::string buf)
{
#ifdef _WINDOWS_
	char enterSenden[] = "\n";
#else
	char enterSenden[] = "\n";
#endif

	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - outBufZusammensteller>\n", DEBUGFEHLER);
	}
	if (debugSpecial1)
	{
		size_t configLen = config.size();
		std::string confLen = boost::lexical_cast<std::string>(configLen);
		schreibeLog("\n2719: Special1: Config Lenght: " + confLen + "\n", DEBUGFEHLER);
	}

	// Die Empfangsdaten in den Antwort String schreiben
	// antwort += buf;

	if (showGesendet)
	{
		// !remove-newline ist im Config gesetzt -> alle CR löschen
		if (rmCR)
		{
			size_t crpos = 0;
			while (1)
			{
				crpos = buf.find("\r", crpos);
				if (crpos != buf.npos)
				{
					buf.erase(crpos, 1);
				}
				else
				{
					break;
				}
			}

		}

		if (showDat)
		{
			if (debugShowAusgabe)
			{
				schreibeLog("\n2705: <DEBUG Show Output: Verbinder - showAusgabe mit buf fuellen>\n<BEGIN>\n", DEBUGFEHLER);
				schreibeLog(buf, DEBUGFEHLER);
				schreibeLog("\n2706: <DEBUG Show Output: Verbinder - showAusgabe mit buf fuellen>\n<END>\n", DEBUGFEHLER);
			}
			showAusgabe << showPrefix << buf;
			showPrefix = "";
		}
		else
		{
			shString += showPrefix;
			shString += buf;
			showPrefix = "";
		}
	}

	// bufStat überschreiben wenn termLenCounter > 3, damit die Verbindung abgebrochen wird; Fehlermeldung ausgeben
	if (termLenCounter > 3)
	{
		bufStat = EXIT;
		schreibeLog("2216: Connection error with " + ip + "Internal wktools error leaded to \"terminal lenght 0\" loop.", SYSTEMFEHLER, "2216");
	}

	switch (bufStat)
	{
	case UNDEFINIERT:
		schreibeLog("2101: Unknown error!", SYSTEMFEHLER, "2101");
		break;
	case NICHTS:
		nichtsString += buf; 
		//Für den Fall, dass bei Multihop verschiedene Gerätetypen verwendet werden.
		//Dann wird bei nichts senden geprüft, ob die Config leer ist und kein show Befehl abgesetzt wurde
		//Sollte das der Fall sein, wird der Buftester bei *Verbindung zurück auf loginBufTester gesetzt, 
		//damit das ursprüngliche Gerät wieder erkannt werden kann.
		if (config.empty() && !showGesendet && !bufTestAenderung /*&& (MODUS == WEITER)*/)
		{
			NICHTSaendertBufTest = true;
			strcpy(outBuf, "");
			if (debugBufTester)
			{
				schreibeLog("\n2702: <DEBUG BufTester: NICHTSaendertBufTest TRUE - NICHTS>\n", DEBUGFEHLER);
			}
		}
		else
		{
			strcpy(outBuf, "");
		}
		break;
	case ENTER:
		strcpy(outBuf, enterSenden);
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
		user = username[ulez];
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
			strcat(outBuf, enterSenden);
		}
		break;
	case ENABLEPASS:
		//if (enablepass.size() > ulez)
		//{
			strcpy(outBuf, enablepass[ulez].c_str());
		//}
		//else
		//{
		//	bufStat = UNDEFINIERT;
		//	fehler = "Enable Password wrong!";
		//}
		break;
	case COMMAND:
		if (showGesendet)
		{
			showGesendet = false;
			if (showDat)
			{
				if (debugShowAusgabe)
				{
					schreibeLog("\n2707: <DEBUG Show Output: Verbinder - Show Ausgabe schließen>\n", DEBUGFEHLER);
				}

				showAusgabe.close();
				if (showAusgabe.fail())
				{
					schreibeLog("2201: Show I/O Error! Could not close show file", SYSTEMFEHLER, "2201");
					showAusgabe.clear();

					if (debugShowAusgabe)
					{
						schreibeLog("\n2708: <DEBUG Show Output: Verbinder - Show Ausgabe schließen fehlgeschlagen>\n", DEBUGFEHLER);
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
					strcpy(outBuf, enterSenden);
					schreibeLog("2202: Could not connect to " + ip, SYSTEMFEHLER, "2202");
					break;
				}	
			}
			std::string configZeile = config.front();
			config.pop_front();

			if (configZeile.substr(0, 12) == "!RGXCLEARMEM")
			{
				// Alle [rgxMem] Zwischensicherungen freigeben
				rgxStrings.clear();				// Regex Strings zurücksetzen
				strcpy(outBuf, enterSenden);
			}
			
			if (configZeile.substr(0,9) == "!RGXBLOCK")
			{
				// Alles was zwischen zwei !RGXBLOCK steht wird pro Regex Match vervielfältigt
				// Regex kommt gleich nach !RGXBLOCK
				std::size_t endepos = configZeile.find_first_of("\r\n");
				std::string rgxString = configZeile.substr(10, endepos-10);

				std::vector<std::string> iConfig;

				while (config.size())
				{
					if (config.front().substr(0,9) != "!RGXBLOCK")
					{
						// Command zwischenspeichern
						iConfig.push_back(config.front());
						config.pop_front();
					}
					else
					{
						config.pop_front();
						break;
					}
				}
				dqstring tempConfig = commandBauenML(antwort, iConfig, rgxString);

				// tempConfig ind config übertragen -> Letzte Einträge als erstes pushen, damit die Reihenfolge gleich bleibt
				while (!tempConfig.empty())
				{
					config.push_front(tempConfig.back());
					tempConfig.pop_back();
				}

				configZeile = config.front();
				config.pop_front();
				configZeile += enterSenden;
				strcpy(outBuf, configZeile.c_str());
			}
			else
			{
				configZeile = commandBauen(antwort, configZeile);
				configZeile += enterSenden;
				strcpy(outBuf, configZeile.c_str());
			}

			// wktools internal Config Tags
			if (configZeile.substr(0, 23) == "!wktools-internal-dummy")
			{
				configZeile = "exit";
				configZeile += enterSenden;
				strcpy(outBuf, configZeile.c_str());
			}
			
			// Rückgabe prüfen Feature; Muss unabhängig von den anderen Tags bearbeitet werden, da zusätzlich die anderen Tags, wie show, verwendet werden können.
			// Der Command selber wird immer in die nächste Zeile geschrieben
			if (configZeile.substr(0,15) == "!remove-newline")
			{
				if (!config.empty())
				{
					configZeile = commandBauen(antwort, config.front());
					configZeile += enterSenden;
					strcpy(outBuf, configZeile.c_str());
					config.pop_front();
					rmCR = true;
				}
			}

			if (configZeile.substr(0,11) == "!showOutput")
			{
				// Bsp: !showOutput $hn-$dt-$ip-config
				size_t pos1 = 0;
				configZeile.erase(0, 12);
				pos1 = configZeile.find("\r");
				if (pos1 != configZeile.npos)
				{
					configZeile.erase(pos1);

				}
				pos1 = configZeile.find("\n");
				if (pos1 != configZeile.npos)
				{
					configZeile.erase(pos1);

				}

				pos1 = configZeile.find("$hn");		// Hostname
				if (pos1 != configZeile.npos)
				{
					configZeile.replace(pos1, 3, hostnameShowOutput);
				}
				pos1 = configZeile.find("$ip");		// IP Adresse
				if (pos1 != configZeile.npos)
				{
					configZeile.replace(pos1, 3, ip);
				}
				pos1 = configZeile.find("$dt");		// Datum
				if (pos1 != configZeile.npos)
				{
					char *zeit = new char[9];
					getDate(zeit);
					configZeile.replace(pos1, 3, zeit);
				}
				pos1 = configZeile.find("$pt");		// Pfad
				if (pos1 != configZeile.npos)
				{
					configZeile.replace(pos1, 3, ausgabePfad);
				}
				customShowName = configZeile;
				logOpt = cuN;
				if (!showAppendTemp)
				{
					showAppend = false;
				}
				verzeichnisErstellen(customShowName);
				
				if (!config.empty())
				{
					configZeile = commandBauen(antwort, config.front());
					configZeile += enterSenden;
					strcpy(outBuf, configZeile.c_str());
					config.pop_front();
				}
			}

			if (configZeile.substr(0,7) == "!device")
			{
				size_t hostnamePos = 0; 
				size_t ipPos = 0; 
				hostnamePos = configZeile.find("hostname");
				if (hostnamePos == configZeile.npos)
				{
					hostnamePos = 0;
					ipPos = configZeile.find("ip");
					if (ipPos == configZeile.npos)
					{
						ipPos = 0;
					}
				}

				if (hostnamePos)
				{
					size_t postFixEnde = configZeile.find_first_of("\r\n");
					size_t postfixPos = configZeile.find_first_of(" ", hostnamePos);
					if (postfixPos == configZeile.npos)
					{
						postfixPos = postFixEnde;
					}
					showPrefix = configZeile.substr(8, hostnamePos-8) + hostname + configZeile.substr(postfixPos, postFixEnde-postfixPos) + "\n";
				}
				else if (ipPos)
				{
					size_t postFixEnde = configZeile.find_first_of("\r\n");
					size_t postfixPos = configZeile.find_first_of(" ", ipPos);
					showPrefix = configZeile.substr(8, ipPos-8) + ip + configZeile.substr(postfixPos, postFixEnde-postfixPos) + "\n";
				}
				else
				{
					schreibeLog("2321: ConfigFile Error: !device TAG - wrong format!", SYSTEMFEHLER, "2321");
				}


				// In der Show Ausgabe soll der hinter !cmd-prefix folgende String gesetzt werden und dahinter der folgende Command
				if (!config.empty())
				{
					size_t pos = configZeile.find_first_of("\r\n");
					configZeile = commandBauen(antwort, config.front());
					configZeile += enterSenden;
					strcpy(outBuf, configZeile.c_str());
					config.pop_front();
					logIt = true;
				}
			}

			if (configZeile.substr(0, 9) == "!IFRETURN")
			{
				std::size_t endepos = configZeile.find_first_of("\r\n");
				std::string rgx = configZeile.substr(10, endepos - 10);
				bool ret = antwortAuswerten(antwort, rgx);
				if (ret)
				{
					if (!config.empty())
					{
						configZeile = config.front();
						config.pop_front();
						if (configZeile.substr(0, 6) != "!ENDIF" || configZeile.substr(0, 11) != "!ELSERETURN")
						{
							configZeile = commandBauen(antwort, configZeile);
							configZeile += enterSenden;
							strcpy(outBuf, configZeile.c_str());
						}
					}
					ifreturn = true;
				}
				else
				{
					// Alle Commands für !IFRETURN skippen
					while (!config.empty())
					{
						configZeile = config.front();
						config.pop_front();
						if (configZeile.substr(0, 6) == "!ENDIF" || configZeile.substr(0, 11) == "!ELSERETURN")
						{
							strcpy(outBuf, enterSenden);
							break;
						}
					}
				}
			}
			else if (configZeile.substr(0, 11) == "!ELSERETURN")
			{
				// Wenn beim nächsten Durchlauf ein !ELSERETURN gefunden wird, dann wird auch der zugehörige Command übergangen
				if (ifreturn)
				{
					while (!config.empty())
					{
						configZeile = config.front();
						config.pop_front();
						if (configZeile.substr(0, 8) == "!ENDELSE")
						{
							strcpy(outBuf, enterSenden);
							break;
						}
					}
					ifreturn = false;
				}
				if (!config.empty())
				{
					configZeile = commandBauen(antwort, config.front());
					config.pop_front();
					configZeile += enterSenden;
					strcpy(outBuf, configZeile.c_str());
				}
			}

			if (configZeile.substr(0, 6) == "!ENDIF" || configZeile.substr(0, 8) == "!ENDELSE")
			{
				ifreturn = false;
				strcpy(outBuf, enterSenden);
			}

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
				strcpy(outBuf, enterSenden);
			}
			else if (configZeile.substr(0,15) == "!CTRL+SHIFT+6 X")
			{
				strcpy(outBuf, "\x1e\x78");
				strcat(outBuf, enterSenden);
			}
			else if (configZeile.substr(0,5) == "!CRLF")
			{
				strcpy(outBuf, enterSenden);
			}
			else if (configZeile.substr(0,6) == "!ERROR")
			{
				// config leeren und end und exit reinschreiben; Fehler ausgeben
				while (!config.empty())
				{
					config.pop_front();
				}
				strcpy(outBuf, "end");
				strcat(outBuf, enterSenden);
				config.push_front("exit");

				std::string err = "2213: IP " + ip + " / Aborting; User defined error: ";
				err += configZeile.substr(7, configZeile.length()-7);
				schreibeLog(err, SYSTEMFEHLER, "2213");
			}
			else if (configZeile.substr(0,8) == "!WARNING")
			{
				std::string err = "2317: IP " + ip + "; User defined warning: ";
				err += configZeile.substr(8, configZeile.length()-8);
				schreibeLog(err, FEHLER, "2317");
				
				if (!config.empty())
				{
					configZeile = commandBauen(antwort, config.front());
					configZeile += enterSenden;
					strcpy(outBuf, configZeile.c_str());
					config.pop_front();
				}
			}
			else if (configZeile.substr(0,5) == "!EXIT")
			{
				strcpy(outBuf, "exit");
				strcat(outBuf, enterSenden);
				config.clear();
			}
			else if (configZeile.substr(0,8) == "!ENDEXIT")
			{
				config.clear();
				strcpy(outBuf, "end");
				strcat(outBuf, enterSenden);
				config.push_front("exit");
			}
			else if (configZeile.substr(0,11) == "!cmd-prefix")
			{
				// In der Show Ausgabe soll der hinter !cmd-prefix folgende String gesetzt werden und dahinter der folgende Command
				if (!config.empty())
				{
					size_t pos = configZeile.find_first_of("\r\n");
					showPrefix += configZeile.substr(12, pos-12);
					configZeile = commandBauen(antwort, config.front());
					configZeile += enterSenden;
					strcpy(outBuf, configZeile.c_str());
					config.pop_front();
					logIt = true;
				}
			}
			else if (configZeile.substr(0,4) == "show")
			{
				logIt = true;
			}
			else if (configZeile.substr(0,4) == "!LOG")
			{
				if (!config.empty())
				{
					configZeile = commandBauen(antwort, config.front());
					configZeile += enterSenden;
					strcpy(outBuf, configZeile.c_str());
					config.pop_front();
					logIt = true;
				}
			}
			else if (configZeile.substr(0,10) == "!PARAGRAPH")
			{
				// Alles was zwischen !PARAGRAPHen steht wird in einem Paket gesendet

				bool anfang = true;			// Zur Kennzeichnung des Anfangs
				std::string configZeile = "";
				while (config.size())
				{
					if (configZeile.substr(0,10) != "!PARAGRAPH")
					{
						configZeile += config.front() + enterSenden;
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
					schreibeLog("2501: PARAGRAPH too large - Please contact wktools@spoerr.org", FEHLER, "2501");
				}
			}
			
			if (logIt)
			{
				showDat = false;
				std::string dateiname;
				switch (logOpt)
				{
				case siN:
					dateiname = ausgabePfad + "show-command-output.txt";
					showDat = true;
					break;
				case ipN:
					{
						dateiname = ip;
						dateiname += showPattern;
						if (showAppendDate)
						{
							char *zeit = new char[9];
							getDate(zeit);
							dateiname += "-";
							dateiname += zeit;
						}
						dateiname += ".txt";
						dateiname = ausgabePfad + dateiname;
						showDat = true;
					}
					break;
				case hoN:
					{
						dateiname = hostnameShowOutput;
						dateiname += showPattern;
						if (showAppendDate)
						{
							char *zeit = new char[9];
							getDate(zeit);
							dateiname += "-";
							dateiname += zeit;
						}
						dateiname += ".txt";
						dateiname = ausgabePfad + dateiname;
						showDat = true;
					}
				case poi:
					{
					}
					break;
				case cuN:
					{
						dateiname = customShowName;
						showDat = true;
					}
					break;
				default:
					schreibeLog("2302: Unknown Show Output Option - Please check your settings!", SYSTEMFEHLER, "2302");
					break;
				}
				bufStat = SHOW;
				showGesendet = true;
				if (showDat)
				{
					if (debugShowAusgabe)
					{
						schreibeLog("\n2709: <DEBUG Show Output: Verbinder - show Ausgabe oeffnen>\n", DEBUGFEHLER);
						schreibeLog("\n2710: <DEBUG Show Output: Verbinder - Hostname Show Ausgabe>\nHostname: " + hostnameShowOutput, DEBUGFEHLER);
						schreibeLog("\n2710: <DEBUG Show Output: Verbinder - Filename: " + dateiname, DEBUGFEHLER);
					}

					if (!showAppend)
					{
						showAusgabe.rdbuf()->open(dateiname.c_str(), std::ios_base::out| std::ios_base::binary);
						showAppend = true;
					}
					else
					{
						showAusgabe.rdbuf()->open(dateiname.c_str(), std::ios_base::out | std::ios_base::app| std::ios_base::binary);
					}

					if (!showAusgabe.is_open() || showAusgabe.fail())
					{
						schreibeLog("2201: Show I/O Error! - Could not open show Output file: " + dateiname, SYSTEMFEHLER, "2201");
						showAusgabe.clear();

						if (debugShowAusgabe)
						{
							schreibeLog("\n2708: <DEBUG Show Output: Verbinder - Show Ausgabe oeffnen fehlgeschlagen>\n", DEBUGFEHLER);
						}
					}
				}
			}

			// Wenn der letzte Befehl gesendet wurde und dies kein show Befehl ist,
			// dann wird der buftester umgeschrieben. 
			// Bei NICHTS allein ist es zu wenig, da es vorkommen kann, dass die gesamte letzte Antort im Empfangsbuffer steht
			// Dann hat man das Problem, dass der Buftester nicht umgestellt wird.

			if (config.empty() && !showGesendet /*&& (MODUS == WEITER)*/)
			{
				NICHTSaendertBufTest = true;
				if (debugBufTester)
				{
					schreibeLog("\n2702: <DEBUG BufTester: NICHTSaendertBufTest TRUE - COMMAND>\n", DEBUGFEHLER);
				}

			}

			if (config.empty() && showGesendet)// && (MODUS == WEITER))
			{
				bufStat = NICHTS;
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
			strcpy(outBuf, "exit");
			strcat(outBuf, enterSenden);
			configMode = false;
		}
		else
		{
			bufStat = ENDE;
			strcpy(outBuf, "");
			fertig = true;
		}

		antwort = "";
		break;
	case TLULS:
		strcpy(outBuf, "terminal length 0");
		strcat(outBuf, enterSenden);
		config.push_front("end");
		config.push_front("logging sync");
		config.push_front("line vty 0 4");
		config.push_front("logging sync");
		config.push_front("line con 0");
		config.push_front("conf t");
		termLenCounter++;
		break;
	case TERMLEN:
		strcpy(outBuf, "terminal length 0");
		strcat(outBuf, enterSenden);
		termLenCounter++;
		break;
	case CONFPAGE:
		strcpy(outBuf, "config paging disable");
		strcat(outBuf, enterSenden);
		termLenCounter++;
		break;
	case NXTERMLEN:
		strcpy(outBuf, "terminal length 0");
		strcat(outBuf, enterSenden);
		if (templateConfig)
		{
			config.push_front("connect nxos");
		}
		termLenCounter++;
		break;
	case LOGGSYNC:
		strcpy(outBuf, "conf t");
		strcat(outBuf, enterSenden);
		config.push_front("end");
		config.push_front("logging sync");
		config.push_front("line vty 0 4");
		config.push_front("logging sync");
		config.push_front("line con 0");
		break;
	case PAGERLINE:
		strcpy(outBuf, "pager line 0");
		strcat(outBuf, enterSenden);
		config.push_front("terminal pager lines 0");
		termLenCounter++;
		break;
	case SETLENGTH:
		strcpy(outBuf, "set length 0");
		strcat(outBuf, enterSenden);
		termLenCounter++;
		break;
	case FEHLERMELDUNG:
		if (config.empty())
		{
			strcpy(outBuf, "exit\n");
		}
		else
		{
			strcpy(outBuf, "\n");
		}
		bufStat = FEHLERMELDUNGALT;
		antwort = "";
		break;
	case FEHLERMELDUNGALT:
		if (config.empty())
		{
			strcpy(outBuf, "exit\n");
		}
		else
		{
			strcpy(outBuf, "\n");
		}
		break;
	case SHOW:
		strcpy(outBuf, "");
		bufStat = NICHTS;
		break;
	case CTELNET:
		strcpy(outBuf, multiHop.c_str());
		strcat(outBuf, ip.c_str());
		strcat(outBuf, multiHopPost.c_str());
		strcat(outBuf, enterSenden);
		hostnameAuslesen = true;
		if (tscAuthe || tscNoAuthe)
		{
			strcat(outBuf, enterSenden);
		}
		break;
	case EXIT:
		strcpy(outBuf, "exit");
		strcat(outBuf, enterSenden);
		break;
	case YES:
		strcpy(outBuf, "yes");
		strcat(outBuf, enterSenden);
		break;
	case YESY:
		strcpy(outBuf, "y");
		strcat(outBuf, enterSenden);
		break;
	case NO:
		strcpy(outBuf, "no");
		strcat(outBuf, enterSenden);
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
		std::string bla9 = boost::lexical_cast<std::string>(bufStat);
		std::string bla10 = boost::lexical_cast<std::string>(STATUS);
		std::string bla11 = "\n2703: <outBufZusammensteller><" + bla9;
		bla11 += "><";
		bla11 += bla10;
		bla11 += "><";
		bla11 += "><";
		bla11 += ">";
		schreibeLog(bla11, DEBUGFEHLER);
	}
	if (debugHex)
	{
		// HEX Ausgabe vom eBuf
		std::stringstream hValStr;
		for (int i=0; i < buf.length(); i++)
		{
			int hValInt = (char)buf[i];
			hValStr << "0x" << std::hex << hValInt << " ";
		}
		schreibeLog("\n2718: <eBuf HEX><" + hValStr.str() + ">", DEBUGFEHLER);
	}

	return bufStat;
}


// nameauslesen:
//**************
// Funktion zum Auslesen des Hostnames aus einem Puffer
void Verbinder::nameauslesen(std::string eBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - nameauslesen>\n", DEBUGFEHLER);
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
	boost::algorithm::trim(hostname);
	hostnameShowOutput = hostname;
	hostnameAuslesen = false;
}

// IOSnameauslesen:
//*****************
// Funktion zum Auslesen des Hostnamens aus IOS Geräten
void Verbinder::IOSnameauslesen(std::string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - IOSnameauslesen>\n", DEBUGFEHLER);
	}

	std::string eBuf = zuletztEmpfangen + neBuf;
	size_t gesamt = eBuf.size();
	size_t hnLaenge = eBuf.find_last_of("\r\n");
	hostnameAlt = hostname;
	hostname = eBuf.substr(hnLaenge+1, gesamt-hnLaenge-2);
	boost::algorithm::trim(hostname);
	hostnameShowOutput = hostname;
	hostnameAuslesen = false;

	if (debugHostname)
	{
		std::string logm = "\n\n2711: <<IOS Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}
}


// PIXnameauslesen:
//*****************
// Funktion zum Hostnameauslesen aus PIXen
void Verbinder::PIXnameauslesen(std::string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - PIXnameauslesen>\n", DEBUGFEHLER);
	}

	std::string eBuf = zuletztEmpfangen + neBuf;
	size_t gesamt = eBuf.size();
	size_t hnLaenge = eBuf.find_last_of("\r\n");
	hostnameAlt = hostname;
	hostname = eBuf.substr(hnLaenge+1, gesamt-hnLaenge-3);
	boost::algorithm::trim(hostname);
	hostnameShowOutput = hostname;
	for (int i=0; i < hostnameShowOutput.length(); i++)
	{
		if (hostnameShowOutput[i] == '/')
		{
			hostnameShowOutput = hostnameShowOutput.replace(i, 1, "-");
		}
	}
	hostnameShowOutput = hostname.substr(0, hostname.find_first_of("/\\+"));
	hostnameAuslesen = false;

	if (debugHostname)
	{
		std::string logm = "\n\n2711: <<PIX Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		logm += "ebuf = <<";
		logm += eBuf + ">>\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}

}


// WLCnameauslesen:
//*****************
// Funktion zum Hostnameauslesen aus WLCs
void Verbinder::WLCnameauslesen(std::string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - WLCnameauslesen>\n", DEBUGFEHLER);
	}

	std::string eBuf = zuletztEmpfangen + neBuf;
	size_t gesamt = eBuf.size();
	size_t hnPos1 = eBuf.find_last_of("(") + 1;
	size_t hnPos2 = eBuf.find(")", hnPos1);
	hostnameAlt = hostname;
	hostname = eBuf.substr(hnPos1, hnPos2-hnPos1);
	boost::algorithm::trim(hostname);
	hostnameShowOutput = hostname;
	hostnameAuslesen = false;

	if (debugHostname)
	{
		std::string logm = "\n\n2711: <<WLC Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		logm += "ebuf = <<";
		logm += eBuf + ">>\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}

}


// NXOSnameauslesen:
//*****************
// Funktion zum Hostnameauslesen aus NX OS Boxen
void Verbinder::NXOSnameauslesen(std::string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - NXOSnameauslesen>\n", DEBUGFEHLER);
	}

	std::string eBuf = zuletztEmpfangen + neBuf;
	size_t gesamt = eBuf.size();
	size_t hnLaenge = eBuf.find_last_of("\r\n");
	hostnameAlt = hostname;
	hostname = eBuf.substr(hnLaenge+1, gesamt-hnLaenge-3);
	boost::algorithm::trim(hostname);
	hostnameShowOutput = hostname.substr(0, hostname.find_first_of("/\\+"));
	hostnameAuslesen = false;

	if (debugHostname)
	{
		std::string logm = "\n\n2711: <<NXOS Name auslesen>>\nAlt: " + hostnameAlt;
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
void Verbinder::CATnameauslesen(std::string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - CATnameauslesen>\n", DEBUGFEHLER);
	}

	std::string eBuf = zuletztEmpfangen + neBuf;
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
	boost::algorithm::trim(hostname);
	hostnameShowOutput = hostname;
	hostnameAuslesen = false;

	if (debugHostname)
	{
		std::string logm = "\n\n2711: <<CatOS Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}

}


// UTAnameauslesen:
//*****************
// Funktion zum Hostnameauslesen aus Switches mit CatOS
void Verbinder::UTAnameauslesen(std::string neBuf)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - UTAnameauslesen>\n", DEBUGFEHLER);
	}

	std::string eBuf = zuletztEmpfangen + neBuf;
	size_t gesamt = eBuf.size();
	
	hostnameAlt = hostname;
	
	if (eBuf.find(user + "@") != eBuf.npos)
	{
		// Variante 1: user@hostname:
		size_t v1pos = eBuf.find(user + "@");
		v1pos = eBuf.find("@", v1pos)+1;
		size_t v1posend = eBuf.find(":", v1pos);
		hostname = eBuf.substr(v1pos, v1posend-v1pos);
	}
	else if (eBuf.find_last_of("$") != eBuf.npos)
	{
		// Variante 2: $hostname:
		size_t v2pos = eBuf.find_last_of("$")+1;
		size_t v2posend = eBuf.find(":", v2pos); 
		if (v2posend != eBuf.npos)
		{
			hostname = eBuf.substr(v2pos, v2posend-v2pos);
		}
		else
		{
			// Variante 3: Default
			hostname = "UTAv3";
		}
	}
	else
	{
		// Variante 3: Default
		hostname = "UTA";
	}
	
	boost::algorithm::trim(hostname);
	hostnameShowOutput = hostname;

	if (debugHostname)
	{
		std::string logm = "\n\n2711: <<UTA Name auslesen>>\nAlt: " + hostnameAlt;
		logm += "\nNeu: " + hostname;
		logm += "\n\n";
		logm += "ebuf = <<";
		logm += neBuf + ">>\n\n";
		schreibeLog(logm, DEBUGFEHLER);
	}

}


// cliInit:
//************
// Initialisierung der Einstellungen zum Aufruf eines CLI Befehls in der Shell
// return: true: Fehler
bool Verbinder::cliInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - cliInit>\n", DEBUGFEHLER);
	}
	return false;
}


// telnetInit:
//************
// Initialisierung der Netzwerkeinstellungen und des Sockets für die Telnetverbindung
// return: true: Fehler
bool Verbinder::telnetInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - telnetInit>\n", DEBUGFEHLER);
	}
	return false;
}

// consoleInit:
//*************
// Initialisierung der Seriellen Schnittstelle
// return: true: Fehler
bool Verbinder::consoleInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - consoleInit>\n", DEBUGFEHLER);
	}

	std::string com;						// Com Schnittstelle

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
	boost::system::error_code err;

	serPort.open(com, err);
	if (err)
	{
		schreibeLog("2102: Could not connect to serial interface! ", SYSTEMFEHLER, "2102");
		return true;
	}
	boost::asio::serial_port_base::baud_rate baud(9600);
	boost::asio::serial_port_base::character_size csize(8);
	boost::asio::serial_port_base::flow_control flow(boost::asio::serial_port_base::flow_control::none);
	boost::asio::serial_port_base::parity parity(boost::asio::serial_port_base::parity::none);
	boost::asio::serial_port_base::stop_bits stop(boost::asio::serial_port_base::stop_bits::one);

	serPort.set_option(baud);
	serPort.set_option(csize);
	serPort.set_option(flow);
	serPort.set_option(parity);
	serPort.set_option(stop);

	return false;
}


// sshInit:
//*********
// zum Initialisieren aller nötigen SSH Parameter
// return: true: Fehler
bool Verbinder::sshInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - sshInit>\n", DEBUGFEHLER);
	}

	cryptInit();

	return false;
}


// httpInit:
//**********
// zum Initialisieren der HTTP Parameter
// return: true: Fehler
bool Verbinder::httpInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - httpInit>\n", DEBUGFEHLER);
	}

	return false;
}


// httpsInit:
//*********
// zum Initialisieren aller nötigen HTTPS Parameter
// return: true: Fehler
bool Verbinder::httpsInit()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - httpsInit>\n", DEBUGFEHLER);
	}

	int bla = cryptInit();

	return false;
}


// httpsInitAsio:
//***************
// zum Initialisieren aller nötigen HTTPS Parameter (ASIO Version)
// return: true: Fehler
//bool Verbinder::httpsInitAsio()
//{
//	if (debugFunctionCall)
//	{
//		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - httpsInitAsio>\n", DEBUGFEHLER);
//	}
//
//	return false;
//}


// setPort:
//*********
// Funktion zum Setzen vom Verbindungsport, da Telnet nicht immer 23 und SSH nicht immer 22 sein muss
// Terminal Server Verbindungssettings werden ebenso mit dieser Funktion gesetzt
// wird auch für COM Ports genutzt
void Verbinder::setPort(std::string verbPort, std::string tscm)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setPort>\n", DEBUGFEHLER);
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
void Verbinder::setDefaultDatei(std::string datei)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setDefaultDatei>\n", DEBUGFEHLER);
	}

	defaultDatei = datei;
}


// statConfig:
//*************
// Funktion zum Einlesen der Konfigurationsdatei. Die Konfig kann dynamisch sein, muss aber vom Caller
// richtig gehandelt werden
dqstring Verbinder::statConfig(std::string dateiName, bool dynPWstat)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - statConfig>\n", DEBUGFEHLER);
	}

	keineDatei = false;
	dqstring config;
	std::string zeile;

	std::ifstream conf(dateiName.c_str(), std::ios_base::in | std::ios_base::binary);
	char zeilenende = '\n';
	//conf.get(zeilenende);
	//if (!conf.good())
	//{
	//	zeilenende = '\r';
	//	conf.clear();
	//}
	//conf.seekg(conf.beg);

	if (!conf.is_open())
	{
		// Config File kann nicht geöffnet werden
		defaultConfig.push_back("ERROR!");
	}
	else
	{
		while (!conf.eof())
		{
			getline(conf, zeile, zeilenende);
			
			// \r am Ende löschen
			size_t crpos = 0;
			crpos = zeile.rfind("\r");
			if (crpos != zeile.npos)
			{
				zeile.erase(crpos, 1);
			}
			
			while (zeile[0] == ' ')			// Vorangestellte Leerzeichen löschen
			{
				zeile.erase(0, 1);
			}
			if (!zeile.empty())
			{
				if (zeile.find_first_of("\r\n") > 1)
				{
					config.push_back(zeile);
				}
			}
		}
		defaultConfig = config;
	}

	templateConfig = false;			// Config Template Markierung zurücksetzen

	conf.close();
	return defaultConfig;
}


// inventoryConf:
//***************
// Funktion zum Einlesen der passenden Inventory Config
dqstring Verbinder::inventoryConf(std::string dummyString, bool dummybool)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - inventoryConf>\n", DEBUGFEHLER);
	}

	std::string commands[] = {"show ver", "show snmp", "show module", "show c7200", "show diag",
		"show inventory", "show hardware", "show system", "show file system", "exit"};
	uint anzahl = 10;

	dqstring config;
	for (uint i = 0; i < anzahl; i++)
	{
		config.push_back(commands[i]);
	}

	f2301 = false;
	templateConfig = false;			// Config Template Markierung zurücksetzen

	return config;
}


// leereConf:
//***************
// Funktion zum Einlesen der passenden Inventory Config
dqstring Verbinder::leereConf(std::string dummyString, bool dummybool)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - leereConf>\n", DEBUGFEHLER);
	}

	dqstring config;
	config.push_back("");

	templateConfig = false;			// Config Template Markierung zurücksetzen

	return config;
}



// mapperConf:
//***************
// Funktion zum Einlesen der passenden Mapper Config
dqstring Verbinder::mapperConf(std::string dummyString, bool dummybool)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - mapperConf>\n", DEBUGFEHLER);
	}

	std::string commands[] = {"show version", "show interface", "show arp", "show mac-address-table", "show spanning-tree detail",
		"show cdp neigh det", "show ip arp", "show mac address-table", "show vpc" ,"show ip route | excl B |Null0|connected", "show ip bgp sum", 
		"show ip ospf neigh det", "show ip eigrp neigh det", "show crypto ipsec sa", "show ospf neigh det", "show eigrp neigh det", "show route | excl connected|Null", 
		"show switch virtual link port", "show snmp", "show spanning-tree", "show interface switchport", "show module", "show c7200", "show diag",	"show inventory", 
		"show hardware", "show system", "show file system", "show power inline", "show ip route vrf all", "show vrf interface", "show ip vrf detail", 
		"show ip arp vrf all", "show failover", "show fex", "show ip ospf", "show ip ospf interface", "show ospf", "show ospf interface", "show ip bgp neighbors", 
		"show standby brief", "show hsrp brief", "show dot1x all", "show mab all", "show authentication session", 
		"show vlan", "show spanning-tree mst", "show ip dhcp snooping", "show ip dhcp snooping binding", "show ip device tracking all", "show rep topology detail", 
		"sh ip vrf detail", "!RGXBLOCK (?<=\\nVRF )[a-zA-Z0-9\\-\\_]+", "show ip route vrf [regex] | excl Null0|connected", "show ip arp vrf [regex]", "!RGXBLOCK", "exit" };
	
	uint anzahl = 57;

	dqstring config;
	for (uint i = 0; i < anzahl; i++)
	{
		config.push_back(commands[i]);
	}

	f2301 = false;
	templateConfig = true;

	return config;
}


// mapperConfIPT:
//***************
// Funktion zum Einlesen der passenden Mapper Config
dqstring Verbinder::mapperConfIPT(std::string dummyString, bool dummybool)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - mapperConfIPT>\n", DEBUGFEHLER);
	}

	std::string commands[] = {"DeviceInformation", "NetworkConfiguration", "PortInformation?1", "DeviceInfo", "NetworkCfg", "NetworkInformation", "Device_Information.htm", "Network_Setup.htm", 
		"CDP_Information.htm", "Ethernet_Information.htm", "WLANStatistics", "localmenus.cgi?func=604", "localmenus.cgi?func=219", "localmenus.cgi?func=601"};
	//std::string commands[] = {"CGI/Java/Serviceability?adapter=device.statistics.device", "CGI/Java/Serviceability?adapter=device.statistics.configuration",
	//	"CGI/Java/Serviceability?adapter=device.statistics.port.network"};
	uint anzahl = 14;

	dqstring config;
	for (uint i = 0; i < anzahl; i++)
	{
		config.push_back(commands[i]);
	}
	f2301 = false;
	templateConfig = false;			// Config Template Markierung zurücksetzen

	return config;
}


// intfConf:
//**********
// Funktion zum Einlesen der passenden Interface Statistik Config
dqstring Verbinder::intfConf(std::string dummyString, bool dummybool)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - intfConf>\n", DEBUGFEHLER);
	}

	std::string commands[] = {"show ver", "show interface"};
	uint anzahl = 2;

	dqstring config;
	for (uint i = 0; i < anzahl; i++)
	{
		config.push_back(commands[i]);
	}

	f2301 = false;
	templateConfig = false;			// Config Template Markierung zurücksetzen

	return config;
}


void Verbinder::setMultiHop(std::string mh)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setMultiHop>\n", DEBUGFEHLER);
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

void Verbinder::setEinstellungen(std::string ena, int logo, std::string ausgabeVz, std::string fnp, std::string mh, int enaPFf, int raute, int fehler2301)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setEinstellungen>\n", DEBUGFEHLER);
	}

	// enable Settings setzen
	enable = ena + "\n";

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

	showPattern = fnp;

	multiHop = mh + " ";

	warteRaute = raute;

	f2301 = fehler2301;
}


// setModus:
//**********
// Funktion zum Setzen des Modus
void Verbinder::setModus(uint modus)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setModus>\n", DEBUGFEHLER);
	}

	MODUS = modus;
}


// setShowAppend:
//***************
// Funktion zum Setzen der showAppend Variable, die angibt, ob die show Ausgabe an ein bestehendes File 
// angehängt werden soll.
void Verbinder::setShowAppend(bool showApp, bool showAppDate)
{
	showAppend = showApp;
	showAppendTemp = showApp;
	showAppendDate = showAppDate;
}


// setUserPass:
//*************
// Funktion zum Setzen der User/Passworte bei Verwendung von statischen User/Passworten
void Verbinder::setUserPass(std::string user, std::string lopw, std::string enpw)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setUserPass>\n", DEBUGFEHLER);
	}

	if (user.find(";;") != user.npos)
	{
		size_t pos1 = 0;
		size_t pos2 = 0;
		std::string zw;

		while(pos2 != user.npos)
		{
			pos2 = user.find(";;", pos1);
			zw = user.substr(pos1, pos2-pos1) + "\n";
			pos1 = pos2 + 2;
			username.push_back(zw);
		}

		pos1 = pos2 = 0;
		while(pos2 != lopw.npos)
		{
			pos2 = lopw.find(";;", pos1);
			zw = lopw.substr(pos1, pos2-pos1) + "\n";
			pos1 = pos2 + 2;
			loginpass.push_back(zw);
		}

		pos1 = pos2 = 0;
		while(pos2 != enpw.npos)
		{
			pos2 = enpw.find(";;", pos1);
			zw = enpw.substr(pos1, pos2-pos1) + "\n";
			pos1 = pos2 + 2;
			enablepass.push_back(zw);
		}
	}
	else
	{	
		std::string ep = enpw + "\n";
		std::string lp = lopw + "\n";
		std::string un = user + "\n";

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
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setLogsy>\n", DEBUGFEHLER);
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
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - initVerbinderVars>\n", DEBUGFEHLER);
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
	antwort = "";
	NICHTSaendertBufTest = false;	// Zurücksetzen vom NICHTS Zähler
	bufTestAenderung = false;		// keine Änderung des Buftesters
	nullEaterEnable = false;		// NullEater ausschalten
	fertig = false;					// es sind noch Commands in der Queue
	steuerzeichenGefunden = false;	// es wurden keine Steuerzeichen empfangen
	dollarGefunden = false;			// PIX/ASA Steuerzeichen Prüfung
	ifreturn = false;				// IRETURN Option vorhanden
	rmCR = false;					// !remove-newline gesetzt
	ucs_nxos = false;				// ucs_nxos zurücksetzen; Zeigt an, wenn Verbindung auf UCS gemacht wurde
	keepWLC = false;				// wenn WLC bearbeitet wird, dann true; Muss nach den Commands zurückgesetzt werden
	advancedRauteCheck = true;		// es soll der erweiterte Prompt Check durchgeführt werden (nur IOS)
	//	tscAuthe = false;				// Terminalserver mit Authentication zurücksetzen
	//	tscNoAuthe = false;				// Terminal Server mit keiner Authentication zurücksetzen

	rgxStrings.clear();				// Regex Strings zurücksetzen

	timeoutCounter = 2;
	termLenCounter = 0;

	if (!warteRaute)
	{
		config.push_back("!ENDBLOCK");
	}

	if (dummyCommand)
	{
		config.push_back("!wktools-internal-dummy");
	}

	bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
	if (debugBufTester)
	{
		schreibeLog("\n2702: <bufTestChange initVerbinderVars>", DEBUGFEHLER);
		schreibeLog("\n2702: <DEBUG BufTester: NICHTSaendertBufTest FALSE\ninitVerbinderVars>\n", DEBUGFEHLER);
	}

}


// telnetVerbindung:
//******************
// Das ist die Funktion, wenn telnet als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::telnetVerbindung(std::string adresse, uint status, uint modus, dqstring conf, int idx)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - telnetVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird
	if (vPort == "")
	{
		vPort = "23";
	}
	else
	{
		try
		{
			boost::lexical_cast<int>(vPort);
		}
		catch (boost::bad_lexical_cast &)
		{
			vPort = "23";			
		}
	}

	initVerbinderVars();
	uint sentBytes = 0;

	// Bei einer neuen Verbindung wird der Socket für diese erstellt und die IP Adressen, Ports... gesetzt. 
	// Danach wird eine Verbindung mit dem Remote Host hergestellt
	if (STATUS == NEU)
	{

		// Setzen der Socketeinstellungen, mit Namensauflösung
		if (!setSockEinstellungen(ip, vPort)) 
		{
			return SCHLECHT;
		}
		
		boost::system::error_code errorcode = boost::asio::error::host_not_found;
		tcp::resolver::iterator end;

		// Herstellen einer Verbindung
		while (errorcode && endpoint_iterator != end)
		{
			verbindungsSock.close();
			verbindungsSock.connect(*endpoint_iterator++, errorcode);
		}
		if (errorcode)
		{
			std::string error = "2202: Could not connect to " + ip;
			schreibeLog(error, SYSTEMFEHLER, "2202");
			return SOCKETERROR;
		}
		else
		{
			erfolgreich = true;
		}


		boost::asio::ip::tcp::endpoint endpoint = verbindungsSock.remote_endpoint();
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
#ifdef _WINDOWS_
				Sleep(2000);
#else
				sleep(2);
#endif				
				boost::asio::write(verbindungsSock, boost::asio::buffer("\n", 2));
				if (sentBytes != 2)
				{
					schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
				}

				tscNoAuthe = false;
			}
		}
	}
	else
	{
		// Check ob der Socket noch ok ist. Wenn nicht, abbrechen
		boost::asio::ip::tcp::endpoint endpoint;
		try
		{
			endpoint = verbindungsSock.remote_endpoint();
		}
		catch (boost::system::system_error se)
		{
			std::string error = "2215: Multihop Error - No connection with " + ip;
			schreibeLog(error, SYSTEMFEHLER, "2215");
			return NOCONN;
		}
	}


	size_t bytes = 1;					// Anzahl der empfangenen Bytes
	uint bufTest = UNDEFINIERT;			// Anfangswert von bufTest -> wird dazu verwendet, um das Ergebnis der Empfangspufferauswertung abzuspeichern
	std::string eBuf;						// Empfangsdaten in c++-std::string-Form

	for(uint k = 0; bytes; k++)
	{
		boost::asio::streambuf iSBuf;
		std::istream iStream(&iSBuf);
		
		outBufZaehler = 0;

		// Falls der status gleich "WEITER" oder "WEITERVERWENDEN" und der erste Durchlauf der for Schleife stattfindet, 
		// wird das Datenempfangen übersprungen, da es im Normalfall keine Daten zu empfangen gibt. Es muss erst etwas
		// gesendet werden, damit das Gegenüber eine verwertbare Antwort schickt.
		// Um feststellen zu können, ob das Empfangen übersprungen wurde, wird die Variable "empf" eingeführt.
		bool empf = false;
		
		boost::system::error_code error;

		if (!((STATUS == WEITER || STATUS == WEITERVERWEDNEN) && (!k)))
		{
			int wcounter = 0;
			while (1)
			{
				size_t bytes_readable = 0;
				try
				{
					boost::asio::socket_base::bytes_readable command(true);
					verbindungsSock.io_control(command);
					bytes_readable = command.get();
				}
				catch (boost::exception &e)
				{
					std::string fehlermeldung = "2203: Connection error with " + ip + ": Unable to read data from socket";
					schreibeLog(fehlermeldung, SYSTEMFEHLER, "2203");
					return SCHLECHT;
				}

				//if (debugSpecial1)
				//{
				//	std::string bytesReadable = boost::lexical_cast<std::string>(bytes_readable);
				//	schreibeLog("\n2719: Special1: Telnet Bytes Readable: " + bytesReadable + "\n", DEBUGFEHLER);
				//}


				if (!verbindungsSock.is_open())
				{
					if (zuletztGesendet.find("exit") != zuletztGesendet.npos)
					{
						empf = true;
						config.clear();
						bytes = 0;
						break;
					}
					else if (!config.empty())
					{
						std::string fehlermeldung = "2203: Connection error with " + ip + ": Unable to read data from socket";
						schreibeLog(fehlermeldung, SYSTEMFEHLER, "2203");
						return SCHLECHT;
					}
					else
					{
						empf = true;
						config.clear();
						bytes = 0;
						break;
					}
				}
				else if (bytes_readable || config.empty() || (zuletztGesendet.find("exit") != zuletztGesendet.npos))
				{
					bytes = boost::asio::read(verbindungsSock, iSBuf,	boost::asio::transfer_at_least(1), error);
					if (error && (zuletztGesendet.find("exit") != zuletztGesendet.npos))
					{
						empf = true;
						config.clear();
						bytes = 0;
						break;
					}
					else if (error && !config.empty())
					{
						std::string error = "2203: Connection error with " + ip + ": Unable to read data from socket";
						schreibeLog(error, SYSTEMFEHLER, "2203");
						return SCHLECHT;
					}
					empf = true;
					timeoutCounter = 2;
					break;
				}
				else
				{
					// Falls 30 Sekunden (300x100ms) nichts empfangen wurde, dann soll die Verbindung abgebrochen werden
					if (wcounter > 100)
					{
						if (!timeoutCounter)
						{
							std::string errMessage = "2212: Timeout - no data received within the last 30 seconds from " + adresse;
							schreibeLog(errMessage, SYSTEMFEHLER, "2212");
							empf = false;
							cancelVerbindung = true;
							break;
							//return NOCONN;
						}
						else
						{
							timeoutCounter--;
							empf = false;
							if (debugBufTester)
							{
								std::string bla11 = boost::lexical_cast<std::string>(timeoutCounter);
								std::string bla12 = "\n2712: <BufTest Telnet Timeoutcounter: " + bla11;
								bla12+= ">\n";
								schreibeLog(bla12, DEBUGFEHLER);
							}
							break;
						}
					}

#ifdef _WINDOWS_
					Sleep(100);
#else
					usleep(100000);
#endif				
					wcounter++;
				}
			}
		}

		// Umwandeln des Input Streams in ein char[]
		char *iBuf = new char[bytes+1];
		iStream.read(iBuf, bytes);
		iBuf[bytes] = 0x00;

		// HEX Ausgabe vom iBuf
		if (debugHex)
		{
			std::stringstream hValStr;
			for (int i=0; i < bytes; i++)
			{
				int hValInt = (char)iBuf[i];
				hValStr << "0x" << std::hex << hValInt << " ";
			}
			schreibeLog("\n2718: <Telnet iBuf><HEX><" + hValStr.str() + ">", DEBUGFEHLER);
			
			//std::ostringstream result;
			//result << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
			//std::copy(eBuf.begin(), eBuf.end(), std::ostream_iterator<unsigned int>(result, " "));
			//schreibeLog("\n2703: <Telnet iBuf><HEX Var 2><" + result.str() + ">", DEBUGFEHLER);
		}




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
			keepWLC = false;
			if (debugBufTester)
			{
				schreibeLog("\n2702: <bufTestChange telnetVerbindung>", DEBUGFEHLER);
				schreibeLog("\n2702: <DEBUG BufTester: NICHTSaendertBufTest FALSE>\n", DEBUGFEHLER);
			}

		}

		if (!empf)
		{
			strcpy(outBuf, "\n");
			outBufZaehler = 1;
			bufTest = ENTER;
			if (debugBufTester)
			{
				std::string bla12 = "\n2712: <BufTest TelnetVerbinder empf=false - ENTER>\n";
				schreibeLog(bla12, DEBUGFEHLER);
			}
		}
		else
		{
			if (bytes == 0)
			{
				if (!k)
				{
					std::string error = "2204: Connection error with " + ip;
					schreibeLog(error, SYSTEMFEHLER, "2204");
					return SOCKETERROR;
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
					else if (zuletztGesendet.find("exit") != zuletztGesendet.npos)
					{
						break;
					}
					else
					{
						std::string error = "2205: No connection with " + ip;
						schreibeLog(error, SYSTEMFEHLER, "2205");
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
				size_t sentBytes = boost::asio::write(verbindungsSock, boost::asio::buffer(outBuf, outBufZaehler));
				if (sentBytes != outBufZaehler)
				{
					schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
				}
			}
			// Auswerten der Daten
			eBuf.assign(iBuf, tofLaenge);
			
			// HEX Ausgabe von eBuf, nachdem die Telnet Optionen gefiltert wurden
			if (debugSpecial2)
			{
				std::stringstream hValStr;
				for (int i=0; i < tofLaenge; i++)
				{
					int hValInt = (char)eBuf[i];
					hValStr << "0x" << std::hex << hValInt << " ";
				}
				schreibeLog("\n2718: <Telnet eBuf nach TelOpt><HEX><" + hValStr.str() + ">", DEBUGFEHLER);
			}

			delete[] iBuf;		// iBuf löschen

			if (eBuf.length())
			{
				if (nullEaterEnable)
				{
					// Bei CatOS können "NULL" Zeichen mitten in den Daten vorkommen. Um spätere Probleme zu beseitigen,
					// werden diese herausgefiltert
					eBuf = nullEater(eBuf);
				}
				// BEL am Ende löschen (0x07)
				if (eBuf[eBuf.length()-1] == 0x07)
				{
					eBuf.erase(eBuf.length()-1);
				}

				
				if (debugEmpfangen)
				{
					std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
					std::string bla12 = "2713: <Received Data Bytes: " + bla10;
					bla12 += " / Received = <";
					bla12 += eBuf;
					bla12 += ">";
					schreibeLog(bla12, DEBUGFEHLER);
				}

				eBuf = steuerzeichenEater(eBuf);

				// Die Empfangsdaten in den Antwort String schreiben
				antwort += eBuf;

				bufTest = (*this.*bufTester)(eBuf, bufTest);

				if (debugBufTester)
				{
					std::string bla11 = boost::lexical_cast<std::string>(bufTest);
					std::string bla12 = "\n2712: <BufTest = " + bla11;
					bla12+= ">\n";
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
			std::string error = "2303: " + fehler + " => Next IP Address!";
			schreibeLog(error, SYSTEMFEHLER, "2303");
			break;
		}

		// Überprüfen, ob die Verbindung abgebrochen werden soll
		if (cancelVerbindung)
		{
			break;
		}

		sentBytes = 0;

		// Wenn Pakete im Ausgangspuffer sind, werden diese jetzt gesendet, 
		// sonst wird auf weitere Daten vom Gegenüber gewartet.
		if (outBufZaehler)
		{
			if (debugSpecial1)
			{
				std::string obufZ = boost::lexical_cast<std::string>(outBufZaehler);
				schreibeLog("\n2719: Special1: outBufZaehler vor SendeDaten: " + obufZ + "\n", DEBUGFEHLER);
				std::string bla12 = " outBuf = <";
				bla12 += outBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}

			try
			{
				sentBytes = boost::asio::write(verbindungsSock, boost::asio::buffer(outBuf, outBufZaehler), error);
				if (error)
				{
					std::string fehlermeldung = "2203: Connection error with " + ip + ": Unable to send data #1";
					schreibeLog(fehlermeldung, SYSTEMFEHLER, "2203");
					return SCHLECHT;
				}
			}
			catch (std::exception &e)
			{
				std::string fehlermeldung = "2203: Connection error with " + ip + ": Unable to send data #2";
				schreibeLog(fehlermeldung, SYSTEMFEHLER, "2203");
				return SCHLECHT;
			}

			if (debugSenden)
			{
				std::string bla10 = boost::lexical_cast<std::string>(outBufZaehler);
				std::string bla12 = "2714: <Sent Data Bytes: " + bla10;
				bla12 += " / Send = <";
				bla12 += outBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}

			// Abspeichern der gesendeten Daten in zuletzGesendet, um im Fehlerfall noch einaml auf diese zurückgreifen zu können.
			zuletztGesendet = outBuf;
			if (sentBytes != outBufZaehler)
			{
				schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
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
	wznip = WZNIP;
	schreibeLog("\n2612: Next IP\n", INFO, "2612");
	wznip = 0;

	return GUT;
}

// consoleVerbindung:
//*******************
// Das ist die Funktion, wenn konsole als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::consoleVerbindung(std::string adresse, uint status, uint modus, dqstring conf, int idx)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - consoleVerbindung>\n", DEBUGFEHLER);
	}

	config = conf;				// Konfiguration, die in den aktuellen Host eingespielt werden soll
	ip = adresse;				// IP Adresse des zu bearbeitenden Hosts
	int bufTest = UNDEFINIERT;	// wird verwendet, um den Status der Empfangsdatenauswertung abzuspeichern
	MODUS = modus;				// Modus -> global NEU oder WEITER
	STATUS = status;			// Status von MODUS -> NEU oder WEITER
	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird
	size_t bytes = 1;			// Anzahl der empfangenen Bytes

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

	//	std::cout << "\nInitial WRITE\n";
	uint sentBytes = 0;
	sentBytes = boost::asio::write(serPort, boost::asio::buffer("\n", 2));
	if (sentBytes != 2)
	{
		schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
	}

	// Solange Daten zu senden sind....
	while(1)
	{
		outBufZaehler = 0;		// Wie viele Daten sind zu senden
		std::string zwischenBuf;		// Puffer zum abspeichern der empfangenen Daten

		boost::asio::streambuf iSBuf;
		std::istream iStream(&iSBuf);

		// Auslesen der seriellen Schnittstelle
		bool empf = false;

		boost::system::error_code error;
		bytes = boost::asio::read(serPort, iSBuf,	boost::asio::transfer_at_least(1), error);
		if (error)
		{
			std::string error = "2207: Unable to read data from serial port!";
			schreibeLog(error, SYSTEMFEHLER, "2207");
			return SCHLECHT;

		}
		empf = true;

		// Umwandeln des Input Streams in ein char[]
		char *iBuf = new char[bytes+1];
		iStream.read(iBuf, bytes);
		iBuf[bytes] = 0x00;

		if (STATUS == NEU)
		{
			erfolgreich = true;
		}

		if (NICHTSaendertBufTest && (bytes > 15))
		{
			// Der loginBufTester muss aber informiert werden, dass es sich jetzt um kein neues Gerät handelt.
			bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
			strcpy(outBuf, "");
			bufTestAenderung = true;
			NICHTSaendertBufTest = false;
			keepWLC = false;
			if (debugBufTester)
			{
				schreibeLog("\n2702: <DEBUG BufTester: NICHTSaendertBufTest FALSE>\n", DEBUGFEHLER);
				schreibeLog("\n2702: <bufTestChange consoleVerbindung>", DEBUGFEHLER);
			}

		}
		zwischenBuf.assign(iBuf, bytes);

		if (nullEaterEnable)
		{
			// Bei CatOS können "NULL" Zeichen mitten in den Daten vorkommen. Um spätere Probleme zu beseitigen,
			// werden diese herausgefiltert
			zwischenBuf = nullEater(zwischenBuf);
		}

		// Die Empfangsdaten in den Antwort String schreiben
		antwort += zwischenBuf;

		// Auswerten der empfangenen Daten
		bufTest = (*this.*bufTester)(zwischenBuf, bufTest);
		bufTest = outBufZusammensteller(bufTest, zwischenBuf);
		if (bufTest == FEHLERMELDUNGALT)
		{
			schreibeLog(zwischenBuf, DEBUGFEHLER);
		}
		else
		{
			schreibeLog(zwischenBuf, INFO);
		}

		zuletztEmpfangen = zwischenBuf;

		// Falls ein Fehler aufgetreten ist...
		if ((bufTest == UNDEFINIERT) || (bufTest == WZNIP))
		{
			std::string error = "2303: " + fehler + " => Next IP Address!";
			schreibeLog(error, SYSTEMFEHLER, "2303");
			break;
		}

		// Überprüfen, ob die Verbindung abgebrochen werden soll
		if (cancelVerbindung)
		{
			break;
		}

		sentBytes = 0;
		
		if (outBufZaehler)
		{
			// Senden von Daten, je nachdem wie die empfangenen Daten ausgewertet wurden
			sentBytes = boost::asio::write(serPort, boost::asio::buffer(outBuf, outBufZaehler));

			// Da bei einer Konsoleverbindung die Daten zwei Mal empfangen werden, wird das epfangsOffset gesetzt
			// empfangsOffset = geschrieben;

			zuletztGesendet = outBuf;
			if ((sentBytes) != outBufZaehler)
			{
				schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
			}
		}

		if (bufTest == ENDE && warteRaute)
		{				
			break;
		}
	}
	wznip = WZNIP;
	schreibeLog("\n2612: Next IP\n", INFO, "2612");
	wznip = 0;

	return GUT;
}


// sshVerbindung
//**************
// ssh Verbindung zum gewünschten Host aufbauen
/*
--------------------
alte version, basierend auf cryptlib
--------------------

uint Verbinder::sshVerbindung(std::string adresse, uint status, uint modus, dqstring conf, int idx)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - sshVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	uint sshPort = 0;			// Portnummer
	bool weiterEnter = false;	// Wenn STATUS != NEU dann soll ein Enter als erstes geschickt werden.
	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird

	if (vPort == "")
	{
		sshPort = 22;
	}
	else
	{
		try
		{
			sshPort = boost::lexical_cast<int>(vPort);
		}
		catch (boost::bad_lexical_cast &)
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

	int bytes = 0;						// Anzahl der empfangenen Bytes
	uint bufTest = UNDEFINIERT;			// Anfangswert von bufTest -> wird dazu verwendet, um das Ergebnis der Empfangspufferauswertung abzuspeichern
	std::string eBuf;					// Empfangsdaten in c++-std::string-Form
	char *buf = new char[BUFFER_SIZE];	// Empfangsdaten, die dann in einem c++-String abgespeichert werden
	bool sshFallback = false;			// Fallback auf SSHv1 aktivieren?


	// Bei einer neuen Verbindung wird die Crypt Session neu erstellt. 
	// Danach wird eine Verbindung mit dem Remote Host hergestellt
	if (STATUS == NEU)
	{
		std::string uname;
		std::string logpw;
		size_t pos = 0;

		pos = username[0].find_first_of("\r\n");
		uname = username[0].substr(0, pos);
		user = uname;

		pos = loginpass[0].find_first_of("\r\n");
		logpw = loginpass[0].substr(0, pos);

		while (1)
		{
			int retKeyPos = 0;
			int retKey = 0;
			// Create the session - Pos 0
			retKey = cryptCreateSession(&cryptSession, CRYPT_UNUSED, CRYPT_SESSION_SSH);
			// Add the server name, user name, and password
			if (retKey == CRYPT_OK)
			{
				retKeyPos++;
			}
			// Pos 1
			retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_SERVER_NAME,
				adresse.c_str(), adresse.size());
			if (retKey == CRYPT_OK)
			{
				retKeyPos++;
			}
			// Pos 2
			retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_SERVER_PORT, sshPort);
			if (retKey == CRYPT_OK)
			{
				retKeyPos++;
			}
			// Pos 3
			retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_USERNAME,
				uname.c_str(), uname.size());
			if (retKey == CRYPT_OK)
			{
				retKeyPos++;
			}
			retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_PASSWORD,
				logpw.c_str(), logpw.size());

			if (retKey == CRYPT_OK)
			{
				retKeyPos++;
			}
			// Pos 4
			if (sshFallback)
			{
				sshVersion = 1;
			}
			else
			{
				sshVersion = 2;
			}
			retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_VERSION, sshVersion);

			if (retKey == CRYPT_OK)
			{
				retKeyPos++;
			}
			// Activate the session - Pos 5
			retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_ACTIVE, 1);

			if (retKey != CRYPT_OK)
			{
				std::string errStr = "";
				int errorCode, errorStringLength;
				char *errorString = new char[1000];
				cryptGetAttribute(cryptSession, CRYPT_ATTRIBUTE_ERRORTYPE, &errorCode);
				cryptGetAttributeString(cryptSession, CRYPT_ATTRIBUTE_ERRORMESSAGE, errorString, &errorStringLength);
				try
				{
					errStr.assign(errorString, errorStringLength);
				}
				catch (const std::exception& ex)
				{
					errStr = "Unknown SSH error! - Error Length: ";
					sshInit();
				}
				delete errorString;

				std::string bla10 = boost::lexical_cast<std::string>(retKey);
				std::string bla11 = boost::lexical_cast<std::string>(retKeyPos);
				std::string bla12 = boost::lexical_cast<std::string>(sshVersion);

				std::string errMessage1 = "2411: SSH error <" + bla11 + "><" + bla10 + "><" + bla12 + ">" + errStr;
				schreibeLog(errMessage1, FEHLER, "2411");

				cryptDestroySession(cryptSession);

				std::string errMessage = " :Connection error with " + adresse;

				// SSH Fallback wird auskommentiert, da SSHv1 mit der aktuellen Cryptlib Version nicht unterstützt wird
				//if (!sshFallback)
				//{
				//	errMessage = "\n2406" + errMessage;
				//	errMessage += " - Fallback to SSHv1";
				//	sshFallback = true;
				//	schreibeLog(errMessage, INFO, "2406");
				//	continue;
				//}
				//else
				//{
				errMessage = "2204" + errMessage;
				schreibeLog(errMessage, SYSTEMFEHLER, "2204");
				return SOCKETERROR;
				//}
			}
			else
			{
				erfolgreich = true;
			}
			vne = false;
			break;
		}
	}
	else
	{
		int retKey = cryptPopData(cryptSession, buf, BUFFER_SIZE - 1, &bytes);
		weiterEnter = true;
	}

	int wcounter = 0;

	while (1)
	{
		bool empf = false;			// Wurden Daten empfangen?

		if (vne || weiterEnter)
		{
			strcpy(outBuf, "\n");
			outBufZaehler = 1;
			bufTest = ENTER;
			vne = false;
			weiterEnter = false;
		}
		else
		{
			outBufZaehler = 0;

			int retKey = cryptPopData(cryptSession, buf, BUFFER_SIZE - 1, &bytes);
			if (retKey != CRYPT_OK)
			{
				if (fertig)
				{
					break;
				}
				else if (zuletztGesendet.find("exit") != zuletztGesendet.npos)
				{
					break;
				}
				else
				{
					std::string errMessage = "2205: No connection with " + adresse;
					schreibeLog(errMessage, SYSTEMFEHLER, "2205");
					return NOCONN;
					break;
				}
			}

			buf[bytes] = 0x00;

			if (!bytes)
			{
				// Falls 30 Sekunden (300x100ms) nichts empfangen wurde, dann soll die Verbindung abgebrochen werden
				if (wcounter > 100)
				{
					if (!timeoutCounter)
					{
						std::string errMessage = "2212: Timeout - no data received within the last 30 seconds from " + adresse;
						schreibeLog(errMessage, SYSTEMFEHLER, "2212");
						empf = false;
						cancelVerbindung = true;
						break;
						//return NOCONN;
					}
					else
					{
						empf = false;
						bytes = 2;
						if (debugBufTester)
						{
							std::string bla11 = boost::lexical_cast<std::string>(timeoutCounter);
							std::string bla12 = "\n2712: <BufTest Timeoutcounter: " + bla11;
							bla12 += ">\n";
							schreibeLog(bla12, DEBUGFEHLER);
						}
						timeoutCounter--;
					}
				}

#ifdef _WINDOWS_
				Sleep(100);
#else
				usleep(100000);
#endif			
				if (!bytes)
				{
					wcounter++;
					continue;
				}
			}
			else
			{
				empf = true;
				timeoutCounter = 2;
			}
			wcounter = 0;

			// HEX Ausgabe vom buf
			if (debugHex)
			{
				std::stringstream hValStr;
				for (int i = 0; i < bytes; i++)
				{
					int hValInt = (char)buf[i];
					hValStr << "0x" << std::hex << hValInt << " ";
				}
				schreibeLog("\n2718: <buf><HEX><" + hValStr.str() + ">", DEBUGFEHLER);
			}

			if (!empf)
			{
				strcpy(outBuf, "\n");
				outBufZaehler = 1;
				bufTest = ENTER;
				if (debugBufTester)
				{
					std::string bla12 = "\n2712: <BufTest SSHVerbinder empf=false - ENTER>\n";
					schreibeLog(bla12, DEBUGFEHLER);
				}

			}
			else
			{
				if (NICHTSaendertBufTest && (bytes > 15))
				{
					// Der loginBufTester muss aber informiert werden, dass es sich jetzt um kein neues Gerät handelt.
					bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
					strcpy(outBuf, "");
					bufTestAenderung = true;
					NICHTSaendertBufTest = false;
					keepWLC = false;
					if (debugBufTester)
					{
						schreibeLog("\n2702: <DEBUG BufTester: NICHTSaendertBufTest FALSE>\n", DEBUGFEHLER);
						schreibeLog("\n2702: <bufTestChange sshVerbindung>", DEBUGFEHLER);
					}

				}

				// Auswerten der Daten
				eBuf = buf;
				if (nullEaterEnable)
				{
					// Bei CatOS können "NULL" Zeichen mitten in den Daten vorkommen. Um spätere Probleme zu beseitigen,
					// werden diese herausgefiltert
					eBuf = nullEater(eBuf);
				}

				// BEL am Ende löschen (0x07)
				if (eBuf[eBuf.length() - 1] == 0x07)
				{
					eBuf.erase(eBuf.length() - 1);
				}
				if (debugEmpfangen)
				{
					std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
					std::string bla12 = "2713: <Received Data Bytes: " + bla10;
					bla12 += " / Received = <";
					bla12 += eBuf;
					bla12 += ">";
					schreibeLog(bla12, DEBUGFEHLER);
				}

				eBuf = steuerzeichenEater(eBuf);

				// Die Empfangsdaten in den Antwort String schreiben
				antwort += eBuf;

				bufTest = (*this.*bufTester)(eBuf, bufTest);
				if (debugBufTester)
				{
					std::string bla11 = boost::lexical_cast<std::string>(bufTest);
					std::string bla12 = "\n2712: <BufTest = " + bla11;
					bla12 += ">\n";
					schreibeLog(bla12, DEBUGFEHLER);
				}

				bufTest = outBufZusammensteller(bufTest, eBuf);
				// Im debug Modus werden alle Daten am Bildschirm ausgegeben
				if (bufTest == FEHLERMELDUNGALT)
				{
					// schreibeLog(eBuf, DEBUGFEHLER);
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
					std::string error = "2303: " + fehler + " => Next IP Address!";
					schreibeLog(error, SYSTEMFEHLER, "2303");
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
		}

		int sentBytes = 0;

		if (outBufZaehler)
		{
			// Bei SSH gibt es ein Problem wenn \r\n gesendet wird 
			// -> es darf nur \r gesendet werden.
			// Darum wird der outbufZaehler um eins reduziert.
			// outBufZaehler--;
			int retKey = cryptPushData(cryptSession, outBuf, outBufZaehler, &sentBytes);
			// HEX Ausgabe vom outbuf
			if (debugHex)
			{
				std::stringstream hValStr;
				for (int i = 0; i < outBufZaehler; i++)
				{
					int hValInt = (char)outBuf[i];
					hValStr << "0x" << std::hex << hValInt << " ";
				}
				schreibeLog("\n2718: <outBuf><HEX><" + hValStr.str() + ">", DEBUGFEHLER);
			}

			if (retKey != CRYPT_OK)
			{
				std::string errMessage = "2205: No connection with " + adresse;
				schreibeLog(errMessage, SYSTEMFEHLER, "2205");
				cryptDestroySession(cryptSession);
				break;
			}
			cryptFlushData(cryptSession);
			// Abspeichern der gesendeten Daten in zuletzGesendet, um im Fehlerfall noch einaml auf diese zurückgreifen zu können.
			zuletztGesendet = outBuf;

			if (debugSenden)
			{
				std::string sBytes = boost::lexical_cast<std::string>(sentBytes);
				std::string bla10 = boost::lexical_cast<std::string>(outBufZaehler);
				std::string bla12 = "2714: <Sent Bytes: " + sBytes + "><Sent Data Bytes: " + bla10;
				bla12 += " / Send = <";
				bla12 += outBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}


			if (sentBytes != outBufZaehler)
			{
				schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
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
	wznip = WZNIP;
	schreibeLog("\n2612: Next IP\n", INFO, "2612");
	wznip = 0;

	delete[] buf;
	return GUT;
}
*/
//-------------------
//Neue Version, basierend auf libssh
//-------------------

uint Verbinder::sshVerbindung(std::string adresse, uint status, uint modus, dqstring conf, int idx)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - sshVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	uint sshPort = 0;			// Portnummer
	bool weiterEnter = false;	// Wenn STATUS != NEU dann soll ein Enter als erstes geschickt werden.
	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird

	if (vPort == "")
	{
		sshPort = 22;
	}
	else
	{
		try
		{
			sshPort = boost::lexical_cast<int>(vPort);
		}
		catch (boost::bad_lexical_cast &)
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

	int bytes = 0;						// Anzahl der empfangenen Bytes
	uint bufTest = UNDEFINIERT;			// Anfangswert von bufTest -> wird dazu verwendet, um das Ergebnis der Empfangspufferauswertung abzuspeichern
	std::string eBuf;					// Empfangsdaten in c++-std::string-Form
	char *buf = new char[BUFFER_SIZE];	// Empfangsdaten, die dann in einem c++-String abgespeichert werden
	bool sshFallback = false;			// Fallback auf SSHv1 aktivieren?


										// Bei einer neuen Verbindung wird die Crypt Session neu erstellt. 
										// Danach wird eine Verbindung mit dem Remote Host hergestellt
	if (STATUS == NEU)
	{
		std::string uname;
		std::string logpw;
		size_t pos = 0;

		pos = username[0].find_first_of("\r\n");
		uname = username[0].substr(0, pos);
		user = uname;

		pos = loginpass[0].find_first_of("\r\n");
		logpw = loginpass[0].substr(0, pos);

		while (1)
		{
			int retKeyPos = 0;
			// Create the session - Pos 0
			int retKey;

			session = ssh_new();								//!!TODO!! SSH_FREE()
			// Add the server name, user name, and password
			if (session != NULL)	//new session successfuly created
			{
				retKeyPos++;
			}
			// Pos 1
			retKey = ssh_options_set(session, SSH_OPTIONS_HOST, adresse.c_str());
			if (retKey == NULL)
			{
				retKeyPos++;
			}
			// Pos 2
			retKey = ssh_options_set(session, SSH_OPTIONS_PORT, &sshPort);
			if (retKey == NULL)
			{
				retKeyPos++;
			}
			// Pos 3
			retKey = ssh_options_set(session, SSH_OPTIONS_USER, uname.c_str());
			if (retKey == NULL)
			{
				retKeyPos++;
			}
			// Pos 4
			if (sshFallback)
			{
				sshVersion = 1;
				retKey = ssh_options_set(session, SSH_OPTIONS_SSH1, &sshVersion);
			}
			else
			{
				sshVersion = 1;
				retKey = ssh_options_set(session, SSH_OPTIONS_SSH2, &sshVersion);
			}

			if (retKey == NULL)
			{
				retKeyPos++;
			}
			// Activate the session - Pos 5
			std::string errStr = ""; 
			retKey = ssh_connect(session);
			if (retKey != SSH_OK)
				errStr += ssh_get_error(session);
			//retKeyPos++;

			retKey = ssh_userauth_password(session, NULL, logpw.c_str());
			if (retKey != SSH_OK)
			{
				ssh_disconnect(session);
				errStr += " | ";
				errStr += ssh_get_error(session);
			}
			//retKeyPos++;

			channel = ssh_channel_new(session);
			if (channel != NULL)
			{
				//kontroliertes beenden bei NULL?
				retKeyPos++;
			}

			retKey = ssh_channel_open_session(channel);
			if (retKey != SSH_OK)
			{
				ssh_channel_free(channel);
				ssh_disconnect(session);
				errStr += " | ";
				errStr = ssh_get_error(session);
			}
			//retKeyPos++;

			retKey = ssh_channel_request_pty(channel);
			if (retKey != SSH_OK)
			{
				ssh_channel_free(channel);
				ssh_disconnect(session);
				errStr += " | ";
				errStr = ssh_get_error(session);
			}

			retKey = ssh_channel_request_shell(channel);
			if (retKey != SSH_OK)
			{
				ssh_channel_free(channel);
				ssh_disconnect(session);
				errStr += " | ";
				errStr += ssh_get_error(session);
			}

			if (!errStr.empty())
			{
				std::string bla10 = boost::lexical_cast<std::string>(retKey);
				std::string bla11 = boost::lexical_cast<std::string>(retKeyPos);
				std::string bla12 = boost::lexical_cast<std::string>(sshVersion);

				std::string errMessage1 = "2411: SSH error <" + bla11 + "><" + bla10 + "><" + bla12 + ">" + errStr;
				schreibeLog(errMessage1, FEHLER, "2411");

				ssh_free(session);

				std::string errMessage = " :Connection error with " + adresse;

				if (!sshFallback)
				{
					errMessage = "\n2406" + errMessage;
					errMessage += " - Fallback to SSHv1";
					sshFallback = true;
					schreibeLog(errMessage, INFO, "2406");
					continue;
				}
				else
				{
					errMessage = "2204" + errMessage;
					schreibeLog(errMessage, SYSTEMFEHLER, "2204");
					return SOCKETERROR;
				}
			}
			else
			{
				erfolgreich = true;
			}
			vne = false;
			break;
		}
	}
	else
	{
		int retKey = ssh_channel_read(channel, buf, BUFFER_SIZE - 1, false);
		//errorcheck of retKey?
		weiterEnter = true;
	}

	int wcounter = 0;

  //while(ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel))
	while (1)
	{
		bool empf = false;			// Wurden Daten empfangen?

		if (vne || weiterEnter)
		{
			strcpy(outBuf, "\n");
			outBufZaehler = 1;
			bufTest = ENTER;
			vne = false;
			weiterEnter = false;
		}
		else
		{
			outBufZaehler = 0;

			int retKey = ssh_channel_read(channel, buf, BUFFER_SIZE - 1, &bytes);
			if (retKey <= 0)
			{
				if (fertig)
				{
					break;
				}
				else if (zuletztGesendet.find("exit") != zuletztGesendet.npos)
				{
					break;
				}
				else
				{
					if (retKey == SSH_ERROR)
					{
						ssh_channel_free(channel);
						ssh_disconnect(session);
						std::string errStr = ssh_get_error(session);

						schreibeLog("2204: " + errStr, SYSTEMFEHLER, "2204");
						return ERROR;
					}
					else
					{
						std::string errMessage = "2205: No connection with " + adresse;
						schreibeLog(errMessage, SYSTEMFEHLER, "2205");
						return NOCONN;
						break;
					}
				}
			}

			buf[bytes] = 0x00;

			if (!bytes)
			{
				// Falls 30 Sekunden (300x100ms) nichts empfangen wurde, dann soll die Verbindung abgebrochen werden
				if (wcounter > 100)
				{
					if (!timeoutCounter)
					{
						std::string errMessage = "2212: Timeout - no data received within the last 30 seconds from " + adresse;
						schreibeLog(errMessage, SYSTEMFEHLER, "2212");
						empf = false;
						cancelVerbindung = true;
						break;
						//return NOCONN;
					}
					else
					{
						empf = false;
						bytes = 2;
						if (debugBufTester)
						{
							std::string bla11 = boost::lexical_cast<std::string>(timeoutCounter);
							std::string bla12 = "\n2712: <BufTest Timeoutcounter: " + bla11;
							bla12 += ">\n";
							schreibeLog(bla12, DEBUGFEHLER);
						}
						timeoutCounter--;
					}
				}

#ifdef _WINDOWS_
				Sleep(100);
#else
				usleep(100000);
#endif			
				if (!bytes)
				{
					wcounter++;
					continue;
				}
			}
			else
			{
				empf = true;
				timeoutCounter = 2;
			}
			wcounter = 0;

			// HEX Ausgabe vom buf
			if (debugHex)
			{
				std::stringstream hValStr;
				for (int i = 0; i < bytes; i++)
				{
					int hValInt = (char)buf[i];
					hValStr << "0x" << std::hex << hValInt << " ";
				}
				schreibeLog("\n2718: <buf><HEX><" + hValStr.str() + ">", DEBUGFEHLER);
			}

			if (!empf)
			{
				strcpy(outBuf, "\n");
				outBufZaehler = 1;
				bufTest = ENTER;
				if (debugBufTester)
				{
					std::string bla12 = "\n2712: <BufTest SSHVerbinder empf=false - ENTER>\n";
					schreibeLog(bla12, DEBUGFEHLER);
				}

			}
			else
			{
				if (NICHTSaendertBufTest && (bytes > 15))
				{
					// Der loginBufTester muss aber informiert werden, dass es sich jetzt um kein neues Gerät handelt.
					bufTester = &Verbinder::loginBufTester;	// Den bufTester auf login stellen
					strcpy(outBuf, "");
					bufTestAenderung = true;
					NICHTSaendertBufTest = false;
					keepWLC = false;
					if (debugBufTester)
					{
						schreibeLog("\n2702: <DEBUG BufTester: NICHTSaendertBufTest FALSE>\n", DEBUGFEHLER);
						schreibeLog("\n2702: <bufTestChange sshVerbindung>", DEBUGFEHLER);
					}

				}

				// Auswerten der Daten
				eBuf = buf;
				if (nullEaterEnable)
				{
					// Bei CatOS können "NULL" Zeichen mitten in den Daten vorkommen. Um spätere Probleme zu beseitigen,
					// werden diese herausgefiltert
					eBuf = nullEater(eBuf);
				}

				// BEL am Ende löschen (0x07)
				if (eBuf[eBuf.length() - 1] == 0x07)
				{
					eBuf.erase(eBuf.length() - 1);
				}
				if (debugEmpfangen)
				{
					std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
					std::string bla12 = "2713: <Received Data Bytes: " + bla10;
					bla12 += " / Received = <";
					bla12 += eBuf;
					bla12 += ">";
					schreibeLog(bla12, DEBUGFEHLER);
				}

				eBuf = steuerzeichenEater(eBuf);

				// Die Empfangsdaten in den Antwort String schreiben
				antwort += eBuf;

				bufTest = (*this.*bufTester)(eBuf, bufTest);
				if (debugBufTester)
				{
					std::string bla11 = boost::lexical_cast<std::string>(bufTest);
					std::string bla12 = "\n2712: <BufTest = " + bla11;
					bla12 += ">\n";
					schreibeLog(bla12, DEBUGFEHLER);
				}

				bufTest = outBufZusammensteller(bufTest, eBuf);
				// Im debug Modus werden alle Daten am Bildschirm ausgegeben
				if (bufTest == FEHLERMELDUNGALT)
				{
					// schreibeLog(eBuf, DEBUGFEHLER);
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
					std::string error = "2303: " + fehler + " => Next IP Address!";
					schreibeLog(error, SYSTEMFEHLER, "2303");
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
		}

		int sentBytes = 0;

		if (outBufZaehler)
		{
			// Bei SSH gibt es ein Problem wenn \r\n gesendet wird 
			// -> es darf nur \r gesendet werden.
			// Darum wird der outbufZaehler um eins reduziert.
			// outBufZaehler--;
			int retKey = ssh_channel_write(channel, outBuf, outBufZaehler);
			// HEX Ausgabe vom outbuf
			if (debugHex)
			{
				std::stringstream hValStr;
				for (int i = 0; i < outBufZaehler; i++)
				{
					int hValInt = (char)outBuf[i];
					hValStr << "0x" << std::hex << hValInt << " ";
				}
				schreibeLog("\n2718: <outBuf><HEX><" + hValStr.str() + ">", DEBUGFEHLER);
			}

			if (retKey == SSH_ERROR)
			{
				std::string errMessage = "2205: ";
				errMessage += ssh_get_error(session);
				schreibeLog(errMessage, SYSTEMFEHLER, "2205");
				ssh_channel_close(channel);
				ssh_channel_free(channel);
				ssh_free(session);
				break;
			}
			ssh_blocking_flush(session, -1);
			// Abspeichern der gesendeten Daten in zuletzGesendet, um im Fehlerfall noch einaml auf diese zurückgreifen zu können.
			zuletztGesendet = outBuf;

			if (debugSenden)
			{
				std::string sBytes = boost::lexical_cast<std::string>(sentBytes);
				std::string bla10 = boost::lexical_cast<std::string>(outBufZaehler);
				std::string bla12 = "2714: <Sent Bytes: " + sBytes + "><Sent Data Bytes: " + bla10;
				bla12 += " / Send = <";
				bla12 += outBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}


			if (sentBytes != outBufZaehler)
			{
				schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
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
		ssh_free(session);
		ssh_channel_close(channel);
		ssh_channel_free(channel);
	}
	wznip = WZNIP;
	schreibeLog("\n2612: Next IP\n", INFO, "2612");
	wznip = 0;

	delete[] buf;
	return GUT;
}


// httpVerbindung:
//******************
// Das ist die Funktion, wenn HTTP als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::httpVerbindung(std::string adresse, uint status, uint modus, dqstring conf, int idx)
{
	// Initialisierung START
	// ist viel dabei, was bei HTTP nicht benötigt wird; Ist aber egal, wenn es trotzdem gemacht wird

	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - httpVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	uint telnetPort = 0;		// Portnummer
	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird
	if (vPort == "")
	{
		vPort = "80";
	}
	else
	{
		try
		{
			boost::lexical_cast<int>(vPort);
		}
		catch (boost::bad_lexical_cast &)
		{
			vPort = "80";			
		}
	}

	std::string empfangsDaten = "";

	while (!config.empty())
	{
		// Setzen der Socketeinstellungen, mit Namensauflösung
		if (!setSockEinstellungen(ip, vPort)) 
		{
			return SCHLECHT;
		}

		boost::system::error_code errorcode = boost::asio::error::host_not_found;
		tcp::resolver::iterator end;

		// Herstellen einer Verbindung
		while (errorcode && endpoint_iterator != end)
		{
			verbindungsSock.close();
			verbindungsSock.connect(*endpoint_iterator++, errorcode);
		}
		if (errorcode)
		{
			std::string error = "2202: Could not connect to " + ip;
			schreibeLog(error, SYSTEMFEHLER, "2202");
			return SOCKETERROR;
		}
		else
		{
			erfolgreich = true;
		}

		// GET mit User defined URL senden
		std::string cfg = "";
		if (!config.empty())
		{
			cfg = config.front();
			config.pop_front();
		}

		std::string sendeDaten = "GET /" + cfg;
		sendeDaten += " HTTP/1.1\r\n";
		sendeDaten += "Host: " + adresse + "\r\n";
		sendeDaten += "User-Agent: wktools3\r\n";
		sendeDaten += "Accept: text/plain, text/html\r\n";
		sendeDaten += "\r\n";

		size_t sentBytes = 0;

		sentBytes = boost::asio::write(verbindungsSock, boost::asio::buffer(sendeDaten, sendeDaten.length()));

		if (debugSenden)
		{
			std::string bla10 = boost::lexical_cast<std::string>(sendeDaten.length());
			std::string bla12 = "2714: <Sent Data Bytes: " + bla10;
			bla12 += " / Send = <";
			bla12 += sendeDaten;
			bla12 += ">";
			schreibeLog(bla12, DEBUGFEHLER);
		}
		
		if (sentBytes != sendeDaten.length())
		{
			schreibeLog("2304: Could not send all data!", SYSTEMFEHLER, "2304");
			return SCHLECHT;
		}

		// Daten EMPFANGEN
		std::size_t bytes = 0;					// Anzahl der empfangenen Bytes
		std::string eBuf = "";					// Empfangsdaten in c++-std::string-Form
		boost::asio::streambuf iSBuf;

		try
		{
			bytes = boost::asio::read_until(verbindungsSock, iSBuf, "\r\n", errorcode);
		}
		catch (boost::system::error_code& err)
		{
			continue;
		}


		// Check that response is OK.
		std::istream iStream(&iSBuf);
		std::string http_version;
		iStream >> http_version;
		uint status_code;
		iStream >> status_code;
		std::string status_message;
		std::getline(iStream, status_message);

		if (debugEmpfangen)
		{
			eBuf = http_version + status_code + status_message;
			std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
			std::string bla12 = "2713: <Received Data Bytes: " + bla10;
			bla12 += " / Received = <";
			bla12 += eBuf;
			bla12 += ">";
			schreibeLog(bla12, DEBUGFEHLER);
		}

		if (!iStream || http_version.substr(0, 5) != "HTTP/")
		{
			schreibeLog("2305: Invalid response!", FEHLER, "2305");
			continue;
		}
		if (status_code != 200 && f2301)
		{
			std::string err = "2306: Response returned with status code " + status_code;
			schreibeLog(err, FEHLER, "2306");
			//return SCHLECHT;
		}

		// Read the response headers, which are terminated by a blank line.
		bytes = boost::asio::read_until(verbindungsSock, iSBuf, "\r\n\r\n");

		char *iBuf = new char[bytes+1];
		iStream.read(iBuf, bytes);
		iBuf[bytes] = 0x00;

		eBuf.assign(iBuf);
		delete[] iBuf;		// iBuf löschen

		if (debugEmpfangen)
		{
			std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
			std::string bla12 = "2713: <Received Data Bytes: " + bla10;
			bla12 += " / Received = <";
			bla12 += eBuf;
			bla12 += ">";
			schreibeLog(bla12, DEBUGFEHLER);
		}

		// Read until EOF, writing data to output as we go.
		boost::system::error_code error;
		while (bytes = boost::asio::read(verbindungsSock, iSBuf, boost::asio::transfer_at_least(1), error))
		{
#ifdef _WINDOWS_
			Sleep(100);
#else
			usleep(100000);
#endif				

			std::size_t g = iSBuf.size();

			iBuf = new char[g+1];
			iStream.read(iBuf, g);
			iBuf[g] = 0x00;

			eBuf.assign(iBuf);
			delete[] iBuf;		// iBuf löschen

			schreibeLog(eBuf, INFO);
			empfangsDaten += eBuf;

			// Debug Ausgabe
			if (debugEmpfangen)
			{
				std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
				std::string bla12 = "2713: <Received Data Bytes: " + bla10;
				bla12 += " / Received = <";
				bla12 += eBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}

			boost::asio::socket_base::bytes_readable command(true);
			verbindungsSock.io_control(command);
			size_t bytes_readable = command.get();

			if (!bytes_readable)
			{
				if (eBuf.find("</html>") != eBuf.npos || eBuf.find("</HTML>") != eBuf.npos || error == boost::asio::error::eof)
				{
					error = boost::asio::error::eof;
					break;
				}
			}

		}
		if (error != boost::asio::error::eof)
		{
			std::string error = "2208: Connection error with " + ip + ": Problems with reading data from socket";
			schreibeLog(error, SYSTEMFEHLER, "2208");
			return SCHLECHT;
		}
	}

	// Ausgabe der Daten im Debugfenster
	std::string datname = ausgabePfad + ip + ".htm";
	showAusgabe.rdbuf()->open(datname.c_str(), std::ios_base::out | std::ios_base::binary);
	showAusgabe	<< empfangsDaten;
	showAusgabe.close();

	verbindungsSock.close();
	wznip = WZNIP;
	schreibeLog("\n2612: Next IP\n", INFO, "2612");
	wznip = 0;

	return GUT;
}


// cliVerbindung:
//***************
// Das ist die Funktion, wenn CLI als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::cliVerbindung(std::string adresse, uint status, uint modus, dqstring conf, int idx)
{
	// Initialisierung START
	// ist viel dabei, was bei HTTP nicht benötigt wird; Ist aber egal, wenn es trotzdem gemacht wird

	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - cliVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird

	std::string empfangsDaten = "";

	while (!config.empty())
	{
		// GET mit User defined URL senden
		std::string cfg = "";
		if (!config.empty())
		{
			cfg = config.front();
			config.pop_front();
		}

		std::size_t varPos = cfg.find("$$ip$$");
		if (varPos != cfg.npos)
		{
			cfg.replace(varPos, 6, adresse);
		}
		
		varPos = cfg.find("$$port$$");
		if (varPos != cfg.npos)
		{
			cfg.replace(varPos, 8, vPort);
		}
		
		std::string cUser = username[0].substr(0, username[0].find_first_of("\r\n"));
		std::string cLopw = loginpass[0].substr(0, loginpass[0].find_first_of("\r\n"));
		std::string cEnpw = enablepass[0].substr(0, enablepass[0].find_first_of("\r\n"));
		varPos = cfg.find("$$user$$");
		if (varPos != cfg.npos)
		{
			cfg.replace(varPos, 8, cUser);
		}
		varPos = cfg.find("$$lopw$$");
		if (varPos != cfg.npos)
		{
			cfg.replace(varPos, 8, cLopw);
		}
		varPos = cfg.find("$$enpw$$");
		if (varPos != cfg.npos)
		{
			cfg.replace(varPos, 8, cEnpw);
		}

		if (debugSenden)
		{
			std::string bla10 = boost::lexical_cast<std::string>(cfg.length());
			std::string bla12 = "2714: <Sent Data Bytes: " + bla10;
			bla12 += " / Send = <";
			bla12 += cfg;
			bla12 += ">";
			schreibeLog(bla12, DEBUGFEHLER);
		}
		
		std::string eBuf = "";					// Empfangsdaten in c++-std::string-Form
#ifdef _WINDOWS_
		char psBuffer[256];
		FILE *chkdsk;
		if((chkdsk = _popen(cfg.c_str(), "rt")) == NULL )
		{
			std::string fehlermeldung = "5312: FAIL: " + cfg;
		}
		else
		{
			while(!feof(chkdsk))
			{
				if( fgets(psBuffer, 256, chkdsk) != NULL)
					eBuf.append(psBuffer);
			}
			// Close pipe and print return value of CHKDSK.
			_pclose( chkdsk );
		}
#else
		char psBuffer[256];
		FILE *chkdsk;

		if((chkdsk = popen(cfg.c_str(), "r")) == NULL )
		{
			std::string fehlermeldung = "5312: FAIL: " + cfg;
		}
		else
		{
			while (!feof(chkdsk))
			{
				if (fgets(psBuffer, 256, chkdsk) != NULL) 
					eBuf.append(psBuffer);
			}
			pclose(chkdsk);
		}
#endif

		if (debugEmpfangen)
		{
			std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
			std::string bla12 = "2713: <Received Data Bytes: " + bla10;
			bla12 += " / Received = <";
			bla12 += eBuf;
			bla12 += ">";
			schreibeLog(bla12, DEBUGFEHLER);
		}

		schreibeLog(eBuf, INFO);
		empfangsDaten += eBuf;
	}
	// Ausgabe der Daten im Debugfenster
	std::string datname = ausgabePfad + ip + ".txt";
	showAusgabe.rdbuf()->open(datname.c_str(), std::ios_base::out | std::ios_base::binary);
	showAusgabe	<< empfangsDaten;
	showAusgabe.close();

	wznip = WZNIP;
	schreibeLog("\n2612: Next IP\n", INFO, "2612");
	wznip = 0;

	return GUT;
}


// httpsVerbindung:
//******************
// Das ist die Funktion, wenn HTTPS als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
uint Verbinder::httpsVerbindung(std::string adresse, uint status, uint modus, dqstring conf, int idx)
{
	// Initialisierung START
	// ist viel dabei, was bei HTTPS nicht benötigt wird; Ist aber egal, wenn es trotzdem gemacht wird

	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - httpsVerbindung>\n", DEBUGFEHLER);
	}

	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
	config = conf;				// Die Konfiguration die eingespielt werden soll
	uint httpsPort = 0;			// Portnummer
	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird
	if (vPort == "")
	{
		httpsPort = 443;
	}
	else
	{
		try
		{
			httpsPort = boost::lexical_cast<int>(vPort);
		}
		catch (boost::bad_lexical_cast &)
		{
			httpsPort = 443;			
		}
	}
	initVerbinderVars();
	std::string empfangsDaten;				// Gesammelte Empfangsdaten

	std::string httpUser = username[0].substr(0, username[0].find_first_of("\r\n"));
	std::string httpPass = loginpass[0].substr(0, loginpass[0].find_first_of("\r\n"));



	// GET mit User defined URL senden
	std::string httpAusgabe = "";
	while (!config.empty())
	{
		int retKey = 0;
		// Create the session
		retKey = cryptCreateSession(&cryptSession, CRYPT_UNUSED, CRYPT_SESSION_SSL);
		// Add the server name, user name, and password
		retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_USERNAME, httpUser.c_str(), httpUser.length());
		retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_PASSWORD, httpPass.c_str(), httpPass.length());
		retKey = cryptSetAttributeString(cryptSession, CRYPT_SESSINFO_SERVER_NAME, adresse.c_str(), adresse.size());
		retKey = cryptSetAttribute(CRYPT_UNUSED, CRYPT_OPTION_NET_READTIMEOUT, 30);
		//retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_VERSION, 0);
		retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_SERVER_PORT, httpsPort);
		retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_SSL_OPTIONS, CRYPT_SSLOPTION_DISABLE_NAMEVERIFY);

		// Proxy:
		// TODO: Funktioniert nicht - laut Wireshark kein Traffic
		//retKey = cryptSetAttributeString(CRYPT_UNUSED, CRYPT_OPTION_NET_SOCKS_SERVER, "10.43.50.226:1080", 17);
		//retKey = cryptSetAttributeString(CRYPT_UNUSED, CRYPT_OPTION_NET_HTTP_PROXY, "10.43.50.226:3128", 17);

		// Activate the session
		retKey = cryptSetAttribute(cryptSession, CRYPT_SESSINFO_ACTIVE, 1);

		// ReadTimeout auf 30 Sekunden setzen
		//	retKey = cryptSetAttribute(cryptSession, CRYPT_OPTION_NET_READTIMEOUT, 30);

		if (retKey != CRYPT_OK)
		{
			std::string errStr = "";
			int errorCode, errorStringLength;
			char *errorString = new char[1000];
			cryptGetAttribute(cryptSession, CRYPT_ATTRIBUTE_ERRORTYPE,	&errorCode );
			cryptGetAttributeString(cryptSession, CRYPT_ATTRIBUTE_ERRORMESSAGE, errorString, &errorStringLength);
			try
			{
				errStr.assign(errorString, errorStringLength);
			}
			catch (const std::exception& ex)
			{
				errStr = "Unknown HTTPS error! - Error Length: ";
			}
			delete errorString;

			std::string bla10 = boost::lexical_cast<std::string>(retKey);
			std::string bla12 = boost::lexical_cast<std::string>(sshVersion);

			std::string errMessage1 = "2411: HTTPS error <" + bla10 + "><" + bla12 + ">" + errStr;
			schreibeLog(errMessage1, FEHLER, "2411");
				
			cryptDestroySession(cryptSession);

			std::string errMessage = "2204: Connection error with " + adresse;
			schreibeLog(errMessage, SYSTEMFEHLER, "2204");
			cryptDestroySession(cryptSession);
			return SOCKETERROR;
		}
		else
		{
			erfolgreich = true;
		}
		vne = false;
		
		// GET mit User defined URL senden
		std::string cfg = "";
		if (!config.empty())
		{
			cfg = config.front();
			config.pop_front();
		}
		
		std::string upwAscii =  httpUser + ":" + httpPass;
		std::string upwBase64 = base64_encode(reinterpret_cast<const unsigned char*>(upwAscii.c_str()), upwAscii.length());
		
		std::string sendeDaten = "GET /" + cfg;
		sendeDaten += " HTTP/1.1\r\n";
		sendeDaten += "Host: " + adresse + "\r\n";
		sendeDaten += "User-Agent: wktools3\r\n";
		sendeDaten += "Accept: text/plain, text/html\r\n";
		sendeDaten += "Authorization: Basic " + upwBase64 + "\r\n";
		sendeDaten += "\r\n";
		int sentBytes = 0;

		retKey = cryptPushData(cryptSession, sendeDaten.c_str(), sendeDaten.length(), &sentBytes);

		if (retKey != CRYPT_OK)
		{
			std::string errMessage = "2205: No connection with " + adresse;
			schreibeLog(errMessage, SYSTEMFEHLER, "2205");
			cryptDestroySession(cryptSession);
			return NOCONN;
		}
		cryptFlushData(cryptSession);

		if (sentBytes != sendeDaten.length())
		{
			std::string errMessage = "2205: No connection with " + adresse;
			schreibeLog(errMessage, SYSTEMFEHLER, "2205");
			return NOCONN;
		}

		if (debugSenden)
		{
			std::string bla10 = boost::lexical_cast<std::string>(sendeDaten.length());
			std::string bla12 = "2714: <Sent Data Bytes: " + bla10;
			bla12 += " / Send = <";
			bla12 += sendeDaten;
			bla12 += ">";
			schreibeLog(bla12, DEBUGFEHLER);
		}

		// Daten EMPFANGEN
		bool httpCheck = true;			// HTTP Status Check
		int bytes = 1;					// Anzahl der empfangenen Bytes
		std::string eBuf = "";			// Empfangsdaten in c++-std::string-Form


		while (1)
		{
			char *buf = new char[32768];	// Empfangsdaten, die dann in einem c++-String abgespeichert werden
			retKey = cryptPopData(cryptSession, buf, 32768, &bytes);
			if (retKey == CRYPT_ERROR_COMPLETE)
			{
				// OK - Fertig
				delete[] buf;		// buf löschen
				break;
			}
			else if (retKey != CRYPT_OK)
			{
				std::string errMessage = "2205: No connection with " + adresse;
				schreibeLog(errMessage, SYSTEMFEHLER, "2205");
				delete[] buf;		// buf löschen
				return NOCONN;
			}

			buf[bytes] = 0x00;
			eBuf.assign(buf);
			delete[] buf;		// buf löschen
			schreibeLog(eBuf, INFO);


			if (debugEmpfangen)
			{
				std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
				std::string bla12 = "2713: <Received Data Bytes: " + bla10;
				bla12 += " / Received = <";
				bla12 += eBuf;
				bla12 += ">";
				schreibeLog(bla12, DEBUGFEHLER);
			}


			if (httpCheck)		// Soll während einer GET Abfrage nur einmal gemacht werden
			{
				std::string http_version = eBuf.substr(0, eBuf.find(" "));
				int pos1 = http_version.length();
				std::string status_code = eBuf.substr(pos1+1, 3);

				if (http_version.substr(0, 5) != "HTTP/")
				{
					schreibeLog("2305: Invalid response!", FEHLER, "2305");
					break;
				}
				if (status_code != "200" && f2301)
				{
					std::string scerr = "2306: Response returned with status code " + status_code;
					schreibeLog(scerr, FEHLER, "2306");
					break;
				}
				httpCheck = false;
				size_t httpHeaderEnde = eBuf.find("\r\n\r\n") + 4;
				eBuf.erase(0, httpHeaderEnde);
			}

//			eBuf.erase(0, eBuf.find("<")-1);
			httpAusgabe += eBuf;

			if (eBuf.find("</html>") != eBuf.npos || eBuf.find("</HTML>") != eBuf.npos)
			{
				break;
			}
		}
		cryptDestroySession(cryptSession);
	}

	// Ausgabe der Daten im Debugfenster
	std::string datname = ausgabePfad + ip + ".htm";
	showAusgabe.rdbuf()->open(datname.c_str(), std::ios_base::out | std::ios_base::binary);
	showAusgabe	<< httpAusgabe;
	showAusgabe.close();

	wznip = WZNIP;
	schreibeLog("\n2612: Next IP\n", INFO, "2612");
	wznip = 0;

	return GUT;
}


// httpsVerbindungAsio
//********************
// Das ist die Funktion, wenn HTTPS als Verbindungstype gewählt wird. Mit ihr werden Daten empfangen und gesendet 
// und die einzelnen Unterfunktionen, wie z.B.: die empfangen Daten auswerten aufgerufen.
//uint Verbinder::httpsVerbindungAsio(std::string adresse, uint status, uint modus, dqstring conf, int idx)
//{
//	// Initialisierung START
//	// ist viel dabei, was bei HTTP nicht benötigt wird; Ist aber egal, wenn es trotzdem gemacht wird
//
//	if (debugFunctionCall)
//	{
//		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - httpVerbindung>\n", DEBUGFEHLER);
//	}
//
//	MODUS = modus;				// Welcher Modus wird verwendet? -> global NEU oder WEITER
//	STATUS = status;			// Welchen Status hat MODUS? -> NEU oder WEITER
//	ip = adresse;				// Die IP Adresse des zu bearbeitenden Hosts
//	config = conf;				// Die Konfiguration die eingespielt werden soll
//	uint telnetPort = 0;		// Portnummer
//	index = idx;				// Index der IP Adresse, der für die Fehlerausgab im Log mitgeschickt wird
//	if (vPort == "")
//	{
//		vPort = "443";
//	}
//	else
//	{
//		try
//		{
//			boost::lexical_cast<int>(vPort);
//		}
//		catch (boost::bad_lexical_cast &)
//		{
//			vPort = "443";			
//		}
//	}
//
//	std::string empfangsDaten = "";
//	while (!config.empty())
//	{
//		boost::asio::ssl::context ctx(ioserv, boost::asio::ssl::context::sslv23);
//		ctx.set_verify_mode(boost::asio::ssl::context::verify_none);
//		sslSock.reset(new boost::asio::ssl::stream<tcp::socket>(ioserv, ctx));
//
//		// Setzen der Socketeinstellungen, mit Namensauflösung
//		if (!setSockEinstellungen(ip, vPort)) 
//		{
//			return SCHLECHT;
//		}
//		
//		boost::system::error_code errorcode = boost::asio::error::host_not_found;
//		tcp::resolver::iterator end;
//
//
//		// Herstellen einer Verbindung
//		while (errorcode && endpoint_iterator != end)
//		{
//			sslSock->lowest_layer().close();
//			sslSock->lowest_layer().connect(*endpoint_iterator++, errorcode);
//		}
//		if (errorcode)
//		{
//			std::string error = "2202: Could not connect to " + ip + ": " + errorcode.message();
//			schreibeLog(error, SYSTEMFEHLER, "2202");
//			sslSock->lowest_layer().close();
//
//			ERR_print_errors_fp(stderr);
//			return SOCKETERROR;
//		}
//		else
//		{
//			erfolgreich = true;
//			sslSock->handshake(boost::asio::ssl::stream_base::client, errorcode);
//			if (errorcode)
//			{
//				std::string error = "2202: Could not connect to " + ip + ": " + errorcode.message();
//				schreibeLog(error, SYSTEMFEHLER, "2202");
//				sslSock->lowest_layer().close();
//
//				ERR_print_errors_fp(stderr);
//				return SOCKETERROR;
//			}
//		}
//
//		// GET mit User defined URL senden
//		std::string cfg = "";
//		if (!config.empty())
//		{
//			cfg = config.front();
//			config.pop_front();
//		}
//
//		//typedef base64_from_binary<transform_width<const char *,6,8>> base64_text;
//
//		std::string httpUser = username[0].substr(0, username[0].find_first_of("\r\n"));
//		std::string httpPass = loginpass[0].substr(0, loginpass[0].find_first_of("\r\n"));
//		
//		std::string upwAscii =  httpUser + ":" + httpPass;
//
//		//std::stringstream os;
//		//std::copy(base64_text(upwAscii.c_str()),base64_text(upwAscii.c_str() + upwAscii.size()),ostream_iterator<char>(os));
//		//std::string upwBase64 = os.str();
//		
//		std::string upwBase64 = base64_encode(reinterpret_cast<const unsigned char*>(upwAscii.c_str()), upwAscii.length());
//		
//		std::string sendeDaten = "GET /" + cfg;
//		sendeDaten += " HTTP/1.1\r\n";
//		sendeDaten += "Host: " + adresse + "\r\n";
//		sendeDaten += "User-Agent: wktools3\r\n";
//		sendeDaten += "Accept: text/plain, text/html\r\n";
//		sendeDaten += "Authorization: Basic " + upwBase64 + "\r\n";
//		sendeDaten += "\r\n";
//
//		size_t sentBytes = 0;
//
//		std::string werr = "";
//		try
//		{
//			sentBytes = boost::asio::write(*sslSock, boost::asio::buffer(sendeDaten, sendeDaten.length()));
//		}
//		catch (std::exception& e)
//		{
//			werr = e.what();
//		}
//
//
//		if (debugSenden)
//		{
//			std::string bla10 = boost::lexical_cast<std::string>(sendeDaten.length());
//			std::string bla12 = "2714: <Sent Data Bytes: " + bla10;
//			bla12 += " / Send = <";
//			bla12 += sendeDaten;
//			bla12 += ">";
//			schreibeLog(bla12, DEBUGFEHLER);
//		}
//
//		if (sentBytes != sendeDaten.length())
//		{
//			schreibeLog("2304: Could not send all data! - " + werr, SYSTEMFEHLER, "2304");
//			sslSock->lowest_layer().close();
//			return SCHLECHT;
//		}
//
//		// Daten EMPFANGEN
//		size_t bytes = 0;					// Anzahl der empfangenen Bytes
//		std::string eBuf = "";					// Empfangsdaten in c++-std::string-Form
//		boost::asio::streambuf iSBuf;
//		std::istream iStream(&iSBuf);
//
//		bytes = boost::asio::read_until(*sslSock, iSBuf, "\r\n");
//
//
//		// Check that response is OK.
//		std::string http_version;
//		iStream >> http_version;
//		uint status_code;
//		iStream >> status_code;
//		std::string status_message;
//		getline(iStream, status_message);
//
//		if (debugEmpfangen)
//		{
//			eBuf = http_version + status_code + status_message;
//			std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
//			std::string bla12 = "2713: <Received Data Bytes: " + bla10;
//			bla12 += " / Received = <";
//			bla12 += eBuf;
//			bla12 += ">";
//			schreibeLog(bla12, DEBUGFEHLER);
//		}
//
//		if (!iStream || http_version.substr(0, 5) != "HTTP/")
//		{
//			schreibeLog("2305: Invalid response!", SYSTEMFEHLER, "2305");
//			sslSock->lowest_layer().close();
//			return SCHLECHT;
//		}
//		
//		//if (status_code == 401)
//		//{
//		//	wxMessageBox("Authe!");
//		//}
//		if (status_code != 200  && f2301)
//		{
//			std::string scerr = "2306: Response returned with status code " + status_code;
//			schreibeLog(scerr, FEHLER, "2306");
//			//sslSock->lowest_layer().close();
//			//return SCHLECHT;
//		}
//
//		// Read the response headers, which are terminated by a blank line.
//		bytes = boost::asio::read_until(*sslSock, iSBuf, "\r\n\r\n");
//
//		char *iBuf = new char[bytes+1];
//		iStream.read(iBuf, bytes);
//		iBuf[bytes] = 0x00;
//
//		eBuf.assign(iBuf);
//		delete[] iBuf;		// iBuf löschen
//
//		if (debugEmpfangen)
//		{
//			std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
//			std::string bla12 = "2713: <Received Data Bytes: " + bla10;
//			bla12 += " / Received = <";
//			bla12 += eBuf;
//			bla12 += ">";
//			schreibeLog(bla12, DEBUGFEHLER);
//		}
//
//		// Read until EOF, writing data to output as we go.
//		boost::system::error_code error = boost::asio::error::host_not_found;
//		
//		while (bytes = boost::asio::read(*sslSock, iSBuf, boost::asio::transfer_at_least(1), error))
//		{
//#ifdef _WINDOWS_
//			Sleep(100);
//#else
//			usleep(100000);
//#endif				
//			std::size_t g = iSBuf.size();
//
//			iBuf = new char[g+1];
//			iStream.read(iBuf, g);
//			iBuf[g] = 0x00;
//
//			eBuf.assign(iBuf);
//			delete[] iBuf;		// iBuf löschen
//
//			schreibeLog(eBuf, INFO);
//			empfangsDaten += eBuf;
//
//			// Debug Ausgabe
//			if (debugEmpfangen)
//			{
//				std::string bla10 = boost::lexical_cast<std::string>(eBuf.length());
//				std::string bla12 = "2713: <Received Data Bytes: " + bla10;
//				bla12 += " / Received = <";
//				bla12 += eBuf;
//				bla12 += ">";
//				schreibeLog(bla12, DEBUGFEHLER);
//			}
//			
//			boost::asio::socket_base::bytes_readable command(true);
//			sslSock->lowest_layer().io_control(command);
//			size_t bytes_readable = command.get();
//
//			if (!bytes_readable)
//			{
//				if (eBuf.find("</html>") != eBuf.npos || eBuf.find("</HTML>") != eBuf.npos  || error == boost::asio::error::eof)
//				{
//					error = boost::asio::error::eof;
//					break;
//				}
//			}
//
//		}
//		if (error != boost::asio::error::eof && empfangsDaten == "")
//		{
//			std::string err = "2208: Connection error with " + ip + ": Problems with reading data from socket: " + error.message();
//			schreibeLog(err, SYSTEMFEHLER, "2208");
//			return SCHLECHT;
//		}
//	}
//	// Ausgabe der Daten im Debugfenster
//	std::string datname = ausgabePfad + ip + ".htm";
//	showAusgabe.rdbuf()->open(datname.c_str(), std::ios_base::out | std::ios_base::binary);
//	showAusgabe	<< empfangsDaten;
//	showAusgabe.close();
//
//	sslSock->lowest_layer().close();
//	wznip = WZNIP;
//	schreibeLog("\n2612: Next IP\n", INFO, "2612");
//	wznip = 0;
//
//	return GUT;
//}


// getDate:
//*********
// zum Auslesen des Datums
void Verbinder::getDate(char *datum)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - getDate>\n", DEBUGFEHLER);
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
std::string Verbinder::strfind(std::string zuFinden, std::string basisString, int stellenAnzahl)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - strfind>\n", DEBUGFEHLER);
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
			std::string gefunden = basisString.substr(pos1, groesse);
			return gefunden;
		}
	}

	return "ERROR";
}


// nullEater:
//***********
// Zum Entfernen von NULL aus CatOS Empfangsdaten
std::string Verbinder::nullEater(std::string catOSdaten)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - nullEater>\n", DEBUGFEHLER);
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


// getShow:
//*********
// Funktion zum Auslesen der show Ausgabe
std::string Verbinder::getShow()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - getShow>\n", DEBUGFEHLER);
	}

	return shString;
}


// schreibeLog:
//*************
// Logausgabe; Sendet einen Event mit den Logs an den Parent
void Verbinder::schreibeLog(std::string log, uint dringlichkeit, std::string log2)
{
	WkLog::evtData evtDat;

	std::string *lg = new std::string;
	lg->assign(log);

	switch (dringlichkeit)
	{
	case INFO:
		evtDat.farbe = WkLog::WkLog_SCHWARZ;
		evtDat.format = WkLog::WkLog_NORMALFORMAT;
		evtDat.logEintrag = log;
		evtDat.logEintrag2 = log2;
		evtDat.logEintrag3 = descr;
		evtDat.type = WkLog::WkLog_NORMALTYPE;
		evtDat.groesse = 10;
		evtDat.ipa = ip;
		evtDat.severity = WkLog::WkLog_INFO;
		evtDat.toolSpecific = index;
		evtDat.toolSpecific2 = wznip;
		break;
	case FEHLER:
		evtDat.farbe = WkLog::WkLog_ROT;
		evtDat.format =  WkLog::WkLog_NORMALFORMAT;
		evtDat.logEintrag = log;
		evtDat.logEintrag2 = log2;
		evtDat.logEintrag3 = descr;
		evtDat.type = WkLog::WkLog_ZEIT;
		evtDat.groesse = 10;
		evtDat.ipa = ip;
		evtDat.severity = WkLog::WkLog_FEHLER;
		evtDat.toolSpecific = index;
		evtDat.toolSpecific2 = wznip;
		break;
	case SYSTEMFEHLER:
		evtDat.farbe = WkLog::WkLog_ROT;
		evtDat.format = WkLog::WkLog_FETT;
		evtDat.logEintrag = log;
		evtDat.logEintrag2 = log2;
		evtDat.logEintrag3 = descr;
		evtDat.type = WkLog::WkLog_ZEIT;
		evtDat.groesse = 10;
		evtDat.ipa = ip;
		evtDat.severity = WkLog::WkLog_SYSTEMFEHLER;
		evtDat.toolSpecific = index;
		evtDat.toolSpecific2 = wznip;
		break;
	case DEBUGFEHLER:
		evtDat.farbe = WkLog::WkLog_ROT;
		evtDat.format = WkLog::WkLog_NORMALFORMAT;
		evtDat.logEintrag = log;
		evtDat.logEintrag2 = log2;
		evtDat.logEintrag3 = descr;
		evtDat.type = WkLog::WkLog_NORMALTYPE;
		evtDat.groesse = 10;
		evtDat.ipa = ip;
		evtDat.severity = WkLog::WkLog_DEBUGFEHLER;
		evtDat.toolSpecific = index;
		evtDat.toolSpecific2 = wznip;
		break;
	default:
		break;
	}	

	LogEvent evt(EVT_LOG_MELDUNG);
	evt.SetData(evtDat);
	wxPostEvent(wkVerbinder, evt);
}


// skipVerbindungsTest:
//*********************
// Funktion zum einstellen, dass der Test, ob die Verbindung erfolgreich war, übersprungen werden soll
void Verbinder::skipVerbindungsTest()
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - skipVerbindungsTest>\n", DEBUGFEHLER);
	}

	teste = false;
}


// setDebug:
//**********
// Debug Einstellungen setzen; Wenn eine Option true, dann wird der entsprechende Debug angezeigt
void Verbinder::setDebug(bool dbgSend, bool dbgReceive, bool dbgFunction, bool dbgBufTester, bool dbgHostname, bool dbgBufTestDet, 
						 bool dbgShowAusgabe, bool dbgRgx, bool dbgHex, bool dbgSp1, bool dbgSp2)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setDebug>\n", DEBUGFEHLER);
	}
	debugBufTester = dbgBufTester;
	debugFunctionCall = dbgFunction;
	debugSenden = dbgSend;
	debugEmpfangen = dbgReceive;
	debugHostname = dbgHostname;
	debugbufTestDetail = dbgBufTestDet;
	debugShowAusgabe = dbgShowAusgabe;
	debugRegex = dbgRgx;
	debugHex = dbgHex;
	debugSpecial1 = dbgSp1;
	debugSpecial2 = dbgSp2;
}


// setDesc:
//*********
// Zum Setzen der Description (letzte Spalte im DevGroup File; Für die Logausgabe relevant
void Verbinder::setDesc(std::string description)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - setDesc>\n", DEBUGFEHLER);
	}
	descr = description;
}


// steuerzeichenEater:
//********************
// Steuerzeichen löschen; Interessant für PIX/ASA, da bei langen Eingaben viele Backspaces geschickt werden, die die Ausgabe verhundsen; 
// Diese 0x08er werden aus den Empfangsdaten gelöscht, bevor sie weiterverarbeitet werden.
std::string Verbinder::steuerzeichenEater(std::string pixOSdaten)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - steuerzeichenEater>\n", DEBUGFEHLER);
	}
	// pixOSdaten auf Steuerzeichen überprüfen, vor allem wenn nach links verschoben wird
	// Es werden dann 0x08 und ein Teil der bereits angezeigten Daten und einige Leerzeichen gesendet
	// Diese Daten solle nrausgefiltert werden, damit es bei der Logausgabe lesbarer wird
	if (pixOSdaten.find(0x08, 0) != pixOSdaten.npos)
	{
		steuerzeichenGefunden = true;
	}

	if (steuerzeichenGefunden)
	{
		size_t szPos = pixOSdaten.find(0x08);
		if (dollarGefunden)
		{
			pixOSdaten.erase(0, szPos);
			dollarGefunden = false;
			szPos = 0;
			
		}

		// Am Ende wird der Zeilenanfang noch einmal ausgegeben. Das wird hier erkannt und gelöscht.
		size_t dszPos = pixOSdaten.find("$\x08");
		if (dszPos != pixOSdaten.npos)
		{
			size_t dszPos1 = pixOSdaten.rfind("\x08", dszPos);
			pixOSdaten.erase(dszPos1, dszPos-dszPos1+1);
		}

		// Alle Steuerzeichen Abschnitte suchen und löschen
		for (;szPos < pixOSdaten.length(); szPos++)
		{
			while (pixOSdaten[szPos] == 0x08)
			{
				pixOSdaten.erase(szPos, 1);
			}
			if (pixOSdaten[szPos] == '$')		// Anfang der verschobenen Daten
			{
				// Suchen nach Leerzeichen mit anschließenden Steuerzeichen
				size_t szEndePos = pixOSdaten.find("   \x08", szPos);
				
				if (szEndePos != pixOSdaten.npos)
				{
					// Wenn ein Zeichen falsch, zu wenig, oder zu viel ist, dann ist lastChar falsch; War im Original szPos+38, jetzt szPos+30;
					std::string lastChar = "";
					lastChar += pixOSdaten[szPos+30];
					pixOSdaten.erase(szPos, szEndePos-szPos+4);
					pixOSdaten.insert(szPos, lastChar);
					szPos--;
				}
				else
				{
					dollarGefunden = true;
				}
			}
			//else
			//{
			//	steuerzeichenGefunden = false;
			//	dollarGefunden = false;
			//	break;
			//}
		}
	}
	return pixOSdaten;
}


// antwortAuswerten:
//******************
// Antwort von einem Gerät auswerten -> schauen ob der Ausdruck unter rgx in der Antwort vorkommt; 
// Wenn ja, dann true
// Wenn nein, dann false
bool Verbinder::antwortAuswerten(std::string antwort, std::string rgx)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - antwortAuswerten>\n", DEBUGFEHLER);
	}
	bool ret = false;
	
	boost::regex re;
	boost::smatch what;
	
	try
	{
		re.assign(rgx, boost::regex_constants::perl);
	}
	catch(const std::exception& e)
	{
		std::string err = "2316: Regex Error: ";
		err += e.what();
		schreibeLog(err, FEHLER, "2316");
		return false;
	}

	if (debugRegex)
	{
		schreibeLog("\r\n2715: Regex: <" + re.str() + ">\r\n", DEBUGFEHLER);
		schreibeLog("\r\n2716: Return Value: " + antwort + "\r\n", DEBUGFEHLER);
	}

	std::string line;
	std::istringstream iss;
	iss.str(antwort);
	
	while (std::getline(iss, line))
	{
		bool result = boost::regex_search(line, what, re);
		if (result)
		{
			ret = true;
			if (debugRegex)
			{
				schreibeLog("\r\n2717: Match found in line: " + line, DEBUGFEHLER);
			}
		}
	}

	return ret; 
}


// commandBauen:
//**************
// Command zusammenbauen; Interessant dann, wenn sich ein oder mehrere Regex darin befinden
// Rückgabe ist der Command mit den ersetzten regex's
// Die Regex strings werden aus dem command extrahiert; Sie werden mit [rgx][/rgx]
std::string Verbinder::commandBauen(std::string antwort, std::string command)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - commandBauen>\n", DEBUGFEHLER);
	}
	size_t rgxPos = command.find("[rgx]");
	
	while (rgxPos != command.npos)
	{
		std::string::const_iterator start, end; 
		start = antwort.begin(); 
		end = antwort.end(); 

		rgxPos += 5;
		std::size_t rgxPos2 = command.find("[/rgx]", rgxPos);
		std::string rgx = command.substr(rgxPos, rgxPos2-rgxPos); 	

		// regex auswerten
		boost::regex_constants::syntax_option_type flags = boost::regex_constants::perl; 
		boost::regex re;
		boost::smatch what;
		
		try
		{
			re.assign(rgx, flags);
		}
		catch(const std::exception& e)
		{
			std::string err = "2316: Regex Error: ";
			err += e.what();
			schreibeLog(err, FEHLER, "2316");
			return command;
		}

		if (debugRegex)
		{
			schreibeLog("\r\n2715: Regex: <" + re.str() + ">\r\n", DEBUGFEHLER);
			schreibeLog("\r\n2716: Return Value: " + antwort + "\r\n", DEBUGFEHLER);
		}

		bool firstMatch = false;	// Zum Prüfen, ob es der erste Durchlauf ist
		std::string fms = "";		// erster Match der Regex

		while(boost::regex_search(start, end, what, re)) 
		{
			start = what[0].second; 
			flags |= boost::match_prev_avail; 
			flags |= boost::match_not_bob; 
			
			if (debugRegex)
			{
				schreibeLog("\r\n2717: Match found: " + what[0], DEBUGFEHLER);
			}

			if (!firstMatch)
			{
				firstMatch = true;
				fms = what[0];
			}
		}

		command.replace(rgxPos-5, rgxPos2-rgxPos+11, fms);
		rgxStrings.push_back(fms);
		rgxPos = command.find("[rgx]");
	}

	size_t rgxMemPos = command.find("[rgxMem]");
	while (rgxMemPos != command.npos)
	{
		rgxMemPos += 8;
		std::size_t rgxMemPos2 = command.find("[/rgxMem]", rgxMemPos);
		std::string rgxMemIndex = command.substr(rgxMemPos, rgxMemPos2-rgxMemPos); 

		int rgxMemIdx = 0;
		try
		{
			rgxMemIdx = boost::lexical_cast<int>(rgxMemIndex);
		}
		catch (boost::bad_lexical_cast &)
		{
			schreibeLog("2319: Regex Index Error - the specified Index is not a number!", FEHLER, "2319");			
			return command;
		}

		if (rgxMemIdx >= rgxStrings.size())
		{
			std::string err = "2320: Regex Index Error: Index out of scope!";
			schreibeLog(err, FEHLER, "2320");
			return command;
		}

		command.replace(rgxMemPos-8, rgxMemPos2-rgxMemPos+17, rgxStrings[rgxMemIdx]);
		rgxMemPos = command.find("[rgxMem]");
	}
	
	return command;
}


// commandBauenML:
//****************
// Command zusammenbauen; Anhand der Anzahl der Rückgabewerte auf die Regex
// Rückgabe ist eine Queue mit den neuen Commands
dqstring Verbinder::commandBauenML(std::string antwort, std::vector<std::string> commands, std::string rgxString)
{
	if (debugFunctionCall)
	{
		schreibeLog("\n2701: <DEBUG Function Call: Verbinder - commandBauenML>\n", DEBUGFEHLER);
	}

	// regex auswerten
	boost::regex_constants::syntax_option_type flags = boost::regex_constants::perl; 
	boost::regex re;
	boost::smatch what;

	dqstring newCommands;		// Queue mit den aktualisierten Commands

	try
	{
		re.assign(rgxString, flags);
	}
	catch(const std::exception& e)
	{
		std::string err = "2316: Regex Error: ";
		err += e.what();
		schreibeLog(err, FEHLER, "2316");
		return newCommands;
	}

	if (debugRegex)
	{
		schreibeLog("\r\n2715: Regex: <" + re.str() + ">\r\n", DEBUGFEHLER);
		schreibeLog("\r\n2716: Return Value: " + antwort + "\r\n", DEBUGFEHLER);
	}

	std::string::const_iterator start, end; 
	start = antwort.begin(); 
	end = antwort.end(); 

	while(boost::regex_search(start, end, what, re)) 
	{
		start = what[0].second; 
		flags |= boost::match_prev_avail; 
		flags |= boost::match_not_bob; 

		if (debugRegex)
		{
			schreibeLog("\r\n2717: Match found: " + what[0], DEBUGFEHLER);
		}

		// Jedes Command nach [regex] durchsuchen und [regex] im Fall durch die Regex Rückgabe ersetzen
		for (int i = 0; i < commands.size(); i++)
		{
			size_t rgxPos = commands[i].find("[regex]");
			if (rgxPos != commands[i].npos)
			{
				std::string command = commands[i];
				command.replace(rgxPos, 7, what[0]);
				newCommands.push_back(command);
			}
			else
			{
				newCommands.push_back(commands[i]);
			}
		}
	}


	return newCommands;
}


// verzeichnisErstellen
//*********************
// Checken ob ein Verzeichnis vorhanden ist. Wenn nicht, dann erstellen
void Verbinder::verzeichnisErstellen(std::string pfad)
{
	pfad = pfad.erase(pfad.find_last_of("\\/"));
	wxFileName fn;
	if (!fn.DirExists(pfad))
	{
		fn.Mkdir(pfad, 0777, wxPATH_MKDIR_FULL);
	}
}


// setDummyCommand
//****************
// Dummy Command setzen; Wird bei SingleHop gemacht, um am Schluss immer einen validen Command zu haben
void Verbinder::setDummyCommand(bool dc)
{
	dummyCommand = dc;
}

