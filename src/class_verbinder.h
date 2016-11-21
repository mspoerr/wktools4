#ifndef __VERBINDER__
#define __VERBINDER__

#include <iostream>
#include <string>
#include <deque>
#include <fstream>
#include <vector>
#include <memory>

//#include <boost/asio/ssl.hpp>

#include <cryptlib.h>

#include "stdwx.h"
#include "class_wkLog.h"
#include "class_wke.h"

using boost::asio::ip::tcp;


typedef unsigned int uint;
typedef std::deque<std::string> dqstring;

class Wke;

class Verbinder
{
private:	
	WkLog *asgbe;						// Zeiger auf das Fenster, das die Nachrichten mit den Lgos entgegennimmt
	Wke *wkVerbinder;					// Zeiger auf Wke Parent, um die Events richtig zu senden
	std::string fehler;					// der Fehler in Text
	tcp::socket verbindungsSock;		// Socket für Telnet
	boost::asio::io_service &ioserv;			// IO Service für ASIO Sockets
	boost::asio::serial_port serPort;			// Serielles Port
//	std::auto_ptr<boost::asio::ssl::stream<tcp::socket> > sslSock;
	
	tcp::resolver::iterator endpoint_iterator;		// Iterator für Hostnamen

	static const uint BUFFER_SIZE;		// Buffergroeße für Sende/Empfangsbuffer

	CRYPT_SESSION cryptSession;			// SSH Session
	std::string hostname;					// Hostname des gerade zu bearbeitenden Gerätes
	std::string hostnameAlt;				// Hostname des vorigen Gerätes
	std::string hostnameShowOutput;			// Hostname der für den show Output verwendet werden soll
	std::string showPattern;				// Filename Pattern bei show Ausgabe
	std::string customShowName;				// Filename bei Custom Ausgabe (wenn Ausgabename im Configfile steht)
	std::string ip;							// IP Adresse
	std::string descr;						// Beschreibung (Letzte Spalte im DevGroup File)
	std::string vPort;						// Portnummer
	std::string defaultDatei;				// Default Datei
	std::vector <std::string> username;		// Username
	std::vector <std::string> loginpass;	// Login Passwort
	std::vector <std::string> enablepass;	// Enable Passwort
	std::vector <std::string> rgxStrings;	// Regex Strings
	std::string user;						// Schlussendlich verwendeter Benutzer
	std::string enable;						// "Was tun bei enable?"
	std::string erweiterung;				// Dateierweiterung für dynamische Configs
	std::string pfad;						// Pfad für dynamische Configs
	std::string ausgabePfad;				// Ausgabeverzeichnis für die Show Ausgabe
	std::string shString;					// String, in dem die Show Ausgabe zwischengespeichert wird
	std::string multiHop;					// Was soll bei MultiHop für die weiteren Verbindungen gemacht werden
	std::string multiHopPost;				// Was wird bei MultiHop hinter der IP Adresse noch hinzugefügt?
	std::string nichtsString;				// Wenn buftest = NICHTS, dann werden die Empfangsdaten hier gespeichert
	std::string antwort;					// komplette Antwort eines Befehls; Wird zurückgesetzt, wenn der nächste Befehl gesendet wird
	std::string showPrefix;				// Prefix vor die show Ausgabe
	int index;							// Index: Gibt die Nummer der gerade verarbeiteten IP Adresse an
	int wznip;							// Weiter zur nächsten IP Adresse
	uint MODUS;							// Welcher Modus wird verwendet -> global
	uint STATUS;						// Status vom Modus -> für die jetzige Verbindung
	uint logOpt;						// Optionen für den Dateinamen für die Ausgabe der show Rückgabe
	uint iosAnfangsSettings;			// sollen bei IOS Geräten "logging sync" und/oder "term len 0" gesendet werden?
	uint enPWf;							// was tun, wenn enable PW falsch oder nicht vorhanden
	uint ulez;							// user, lopw, enpw Zähler: Wenn falsch, wird er nach oben gezählt
	uint ulezmax;						// ulez max Zähler: wie viele user/passworte stehen maximal zur Verfügung
	uint ulezmaxEn;						// ulexmax zum runterzählen für enable Passwort
	uint sshVersion;					// SSH Version
	uint timeoutCounter;				// Zähler, um zu schauen, wie oft bereits ein Timeout erkannt wurde.
	uint termLenCounter;				// Zähler für "terminal length 0" zum Schauen, ob wktools in einer "terminal length 0" Schleife festhängt
	bool steuerzeichenGefunden;			// Steuerzeichen in den Empfangsdaten von der ASA/PIX gefunden
	bool dollarGefunden;				// $ in dem Steuerzeichenkonstrukt gefunden
	bool NICHTSaendertBufTest;			// wird auf TRUE gesetzt, wenn bei outBufZusammensteller (NICHTS) der bufTester geändert werden soll.
	bool bufTestAenderung;				// zeigt die tatsächliche Änderung des Buftesters an
	bool schonAuthe;					// wurde schon authentifiziert? Überwachung des Authentifizierungsstatus
	bool schonEnableAuthe;				// wurde für den enable Mode schon authentifiziert?
	bool execKontrolle;					// Zum Feststellen, ob schon einmal versucht wurde, in den enable Modus zu wechseln	
	bool hostnameAuslesen;				// Hostnamen auslesen?
	bool keineDatei;					// Falls bei dynamischern Konfigurationen die spezifische Datei nicht gefunden werden kann
	bool ersterDurchlauf;				// Zum Feststellen, ob die default Konfig eingelesen werden muss
	bool showGesendet;					// Zum Festhalten, ob ein "show" Befehl abgesetzt wurde
	bool ifreturn;						// Zum Festhalten, ob IFRETURN erfolgreich behandelt wurde
	bool elsereturn;					// Zum Festhalten, ob ELSERETURN erfolgreich behandelt wurde
	bool showDat;						// Zum Festhalten, ob eine Datei für die Show Ausgabe geöffnet wurde
	bool erfolgreich;					// War der Verbindungsaufbau erfolgreich?
	bool debug;							// Debug Modus oder nicht
	bool manuell;						// Manueller Modus oder nicht
	bool configMode;					// Ist das Gerät im Konfigurationsmodus?
	bool warteRaute;					// auf Raute warten?
	bool teste;							// Verbindungstest durchführen?
	bool vne;							// vorher nicht erreichbar
	bool tscNoAuthe;					// Terminal Server Verbindung ohne Authentifizierung
	bool tscAuthe;						// Terminal Server Verbindung mit Authentifizierung
	bool fertig;						// Zeigt an, wenn alle Commands gesendet wurden
	bool showAppend;					// bei der show Ausgabe den Output an ein bestehendes File anhängen?
	bool showAppendDate;				// beim Ausgabe File das aktuelle Datum anhängen?
	bool showAppendTemp;				// bei der show Ausgabe den Output an ein bestehendes File anhängen? Verwendung bei custom showAusgabe File
	bool debugEmpfangen;				// Debug Flag: Empfangsdaten werden detailiiert ausgegeben
	bool debugSenden;					// Debug Flag: Sendedaten werden ausgegeben
	bool debugFunctionCall;				// Debug Flag: Jeder Funktionsaufruf wird ausgegeben
	bool debugBufTester;				// Debug Flag: Buftester Infos werden ausgegeben
	bool debugHostname;					// Debug Flag: NameAuslesen wird ausgegeben
	bool debugbufTestDetail;			// Debug Flag: detaillierte Buftest Info wird ausgegeben
	bool debugShowAusgabe;				// Debug Flag: detaillierte Infos zur Show Ausgabe werden ausgegeben
	bool debugRegex;					// Debug Flag: detaillierte Infos zu den Regex's ausgeben
	bool debugHex;						// Debug Flag: detaillierte Infos zu den HEX
	bool debugSpecial1;					// Debug Flag: Platzhalter
	bool debugSpecial2;					// Debug Flag: Platzhalter
	bool rmCR;							// !remove-newline gesetzt
	bool f2301;							// Soll der Fehler 2301 geloggt werden, ja/nein
	bool advancedRauteCheck;			// Soll zusätzlich zur Raute noch geprüft werden, ob der Hostname im Prompt vorkommt?
	bool dummyCommand;					// DummyCommand setzen -> JA/NEIN
	bool templateConfig;				// Config Tempalte in Verwendung?
	bool ucs_nxos;						// connect nxos auf UCS war erfolgreich
	bool keepWLC;						// WLC BufTester behalten
	dqstring defaultConfig;				// die defaultConfig wird darin gespeichert
	dqstring config;					// die Config
	dqstring ifconfig;					// Sub-Config bei Verwendung von !IFRETURN
	dqstring elseconfig;				// Sub-Config bei Verwendung von !ELSERETURN
//	HANDLE konsole;						// für die Erstellung der Konsole I/O
	size_t outBufZaehler;				// Zum Feststellen der Göße des Sendebuffers
	uint empfangsOffset;				// Ab welcher Position müssen die empfangenen Daten überprüft werden
	char *outBuf;						// Die zu sendenden Daten
	std::string zuletztGesendet;		// Die zuletzt gesendeten Daten
	std::string zuletztEmpfangen;		// Die zuletzt empfangenen Daten
	std::ofstream showAusgabe;				// Die Datei, in der die Rückgabe eines "show" Kommandos ausgegeben werden soll
// 	ofstream troubleshooting;			// Ausgabe in eine Datei zu Troubleshootingzwecken
// 	uint trcounter;						// Zaehler für Troubleshooting Zwecke
	bool nullEaterEnable;				// NullEater Funktion einschalten

	void verzeichnisErstellen(			// Wird aufgerufen bei !showOutput, zum Prüfen, ob es den Pfad gibt und im Fall anlegen
		std::string pfad					// Pfad
		);
	bool antwortAuswerten(				// Antwort, die vom letzten Command zurückgekommen ist, auswerten
		std::string antwort,				// String, der die Antwort beinhaltet
		std::string rgx						// regex, die auf die Antwort angewendet werden soll
		);
	std::string commandBauen(			// Command zusammenbauen; Interessant dann, wenn sich ein oder mehrere Regex darin befinden
		std::string antwort,				// std::string, auf die sich die Regex beziehen
		std::string command					// kompletter Comamnd mit (oder ohne) den regex
		);
	dqstring commandBauenML(			// Command zusammenbauen; Interessant dann, wenn sich ein oder mehrere Regex darin befinden; MultiLine Version
		std::string antwort,				// std::string, auf die sich die Regex beziehen
		std::vector<std::string> commands,	// Commands, ohne Regex
		std::string rgxString				// Regex String der zur Anwendung gebracht werden soll
		);
	std::string steuerzeichenEater(		// zum Entfernen von Steuerzeichen in PIX OS Empfangsdaten
		std::string pixOSdaten				// Empfangsdaten
		);	
	std::string nullEater(				// zum Entfernen von NULL in CatOS Empfangsdaten
		std::string catOSdaten				// Empfangsdaten
		);
	bool setSockEinstellungen(			// zum Setzen von IP Adresse und TCP Port
		std::string ip,						// IP Adresse
		std::string port					// TCP Port
		);		
	uint telnetOptionen(				// zum Auswerten der Telnetoptionen beim Verbindungsaufbau
		const char *buf						// Daten, die ausgewertet werden sollen
		);					
	uint telnetOptionenFinder(			// zum Auffinden von Telnet Optionen mitten im Eingangspuffer
		char *buf,							// Daten, in denen nach Telnet Optionen gesucht werden soll
		size_t bufLaenge					// Länge der Daten
		);
	uint IOSbufTester(					// zum Auswerten des Empfangsbuffers bei Verbindungen mit Cisco IOS Boxen
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint PIXbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit Cisco PIXen
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint CATbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit Cisco Catalyst Switches (CatOS)
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint NXOSbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit Cisco NX OS Boxen
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint WLCbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit Cisco WLC Boxen
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint UTAbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit der UTA Managment Konsole
		std::string buf,					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint loginBufTester(				// zum Auswerten des Empfangspuffers, solange das Login noch nicht abgeschlossen ist
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint loginTest(						// zum Auswerten, ob User/Pass im Puffer vorkommt
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint loginModeTest(					// zum Auswerten des Puffers, wenn sich das Gerät im login Modus befindet
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint enableModeTest(				// zum Auswerten des Puffers, wenn sich das Gerät im enable Modus befindet
		std::string buf, 					// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint upFinder(						// Funktion zum Username/Passwort aus der config auslesen
		std::string up,						// String, der nach User und PW durchsucht werden soll
		uint upwZaehler						// Zähler, damit festgestellt werden kann wie viele User/PW schon gefunden wurden
		);				
	uint outBufZusammensteller(			// Funktion zum Füllen des Sendebuffers
		uint bufStat,						// Code, der festlegt, was gesendet werden soll
		std::string buf						// Eingangspuffer, der z.B.: für die "show" Ausgabe verwendet wird
		);	
	void nameauslesen(					// Funktion zum Auslesen des Hostnames
		std::string buf						// Puffer, in dem der Hostname vorkommt
		);
	void IOSnameauslesen(				// Funktion zum Auslesen des Hostnames
		std::string buf						// Puffer, in dem der Hostname vorkommt
		);
	void PIXnameauslesen(				// Funktion zum Auslesen des Hostnames
		std::string buf						// Puffer, in dem der Hostname vorkommt
		);
	void NXOSnameauslesen(				// Funktion zum Auslesen des Hostnames
		std::string buf						// Puffer, in dem der Hostname vorkommt
		);
	void CATnameauslesen(				// Funktion zum Auslesen des Hostnames
		std::string buf						// Puffer, in dem der Hostname vorkommt
		);
	void UTAnameauslesen(				// Funktion zum Auslesen des Hostnames
		std::string buf						// Puffer
		);
	void WLCnameauslesen(				// Funktion zum Auslesen des Hostnames (WLC)
		std::string buf						// Puffer
		);
	void schreibeLog(					// Funktion zum Schreiben von Logs
		std::string log,					// String der geschrieben werden soll
		uint dringlichkeit,					// Welche Dringlichkeit hat die Meldung?
		std::string log2=""					// Erweiterter Log Eintrag
		);
	void initVerbinderVars();			// zum Initialisieren der mehrfach genutzten Variablen

public:
	Verbinder(							// Konstruktor
		WkLog *logAusgabe,					// Zeiger auf das Debug Fenster
		Wke *wkV,							// Zeiger auf Wkn für die Events
		boost::asio::io_service &io_service			// IO_Service für ASIO Socket Initialisierung
//		boost::asio::ssl::context &ctx				// Context für SSL für die ASIO HTTPS Variante
		);
	~Verbinder();

	enum logopts { ipN = 0,				// Logoptionen; Dateiname = IP Adresse
				   hoN,					// Dateiname = Hostname
				   siN,					// Dateiname für alle Hosts gleich
				   poi,					// Daten werden nicht in Datei sondern einem Zeiger gespeichert
				   cuN					// Custom Name -> wenn im Config File gesetzt
				 };
	enum stats { SCHLECHT = 100,		// Rückgabewerte für xxVerbindung und Übergabewerte für "unit status" und uint modus
				 GUT,
				 NOCONN,
				 HTTPERROR,
				 NEU = 110,
				 WEITER,
				 WEITERVERWEDNEN,
				 COM1,
				 COMx,
				 SSH,
				 SSH1,
				 SSH2,
				 TELNET,
				 HTTP,
				 ENDE,
				 SOCKETERROR,
				 FEHLER,
				 SYSTEMFEHLER,
				 DEBUGFEHLER,
				 INFO
			   };
	enum bufTestRueckgabe {				// Rückgabewerte für alle bufTester.
		UNDEFINIERT,					//Undefiniert, kein Ergebnis, oder Fehler
		NICHTS,							// nichts senden
		ENTER,							// CR/LF soll gesendet werden
		ENABLEMODUS,					// Zum Enable Modus wechseln
		USERNAME,						// Senden des Usernamen
		LOGINPASS,						// Login Passwort senden
		ENABLEPASS,						// Enable Passwort senden
		ENABLEPASSAGAIN,				// Enable Passwort wurde schon einmal gesendet, noch einmal
		COMMAND,						// Senden der nächsten Konfigurtionszeile
		TLULS,							// "terminal length 0" & "logging synchronous" senden
		TERMLEN,						// "terminal length 0" senden
		LOGGSYNC,						// "logging synchronous" senden
		FEHLERMELDUNG,					// bufTester hat eine Fehlermldung erkannt und diese ausgegeben
		SHOW,							// bufTester hat einen show Command erkannt
		CTELNET,						// MultiHop zur nächsten IP Adresse; Als Protokoll wird das in std::string multiHop gespeicherte verwendet
		CSSH1,							// SSHv1 zur nächsten IP Adresse
		CSSH2,							// SSHv2 zur nächsten IP Adresse
		EXIT,							// "exit" senden
		KEINEOPTIONEN,					// keine Telnet Optionen gefunden
		OPTIONENGEF,					// Telnet Optionen gefunden
		WZNIP,							// Weiter zur nächsten IP Adresse
		FEHLERMELDUNGALT,				// Übernahme der Fehlermeldung
		PAGERLINE,						// "pager line 0" senden
		SETLENGTH,						// "set length 0" senden
		CONFPAGE,						// "configure paging disable
		YES,							// "yes" senden
		YESY,							// "y" senden
		NO,								// "no" senden
		NXTERMLEN						// NXOS Term Len senden (und im Fall connect nxos -> bei UCS)
		};
	volatile bool cancelVerbindung;		// alle Aktivitäten abbrechen und zurückkehren
	void setEinstellungen(				// Funktion zum Setzen diverser Einstellungen
		std::string enable,					// "Was tun bei enable" setzen
		int logo,							// Setzen der Log Optionen
		std::string ausgabeVz,				// Ausgabeverzeichnis der Show Ausgabe
		std::string fnp,					// Filename Pattern -> wird beim show Output im Filenamen hinzugefügt
		std::string mh,						// Was tun bei MultiHop
		int enaPWf, 						// setzen von "Was tun wenn enable PW falsch"
		int raute,							// auf Raute warten?
		int f2301							// soll der Fehler 2301 geloggt werden
		);
	void setMultiHop(					// Funktion zum Setzen des Multihop Commands
		std::string mh						// Was tun bei Mutlihop
		);
	void setDesc(						// Zum Setzen der Description (letzte Spalte im DevGroup File
		std::string description				// Description
		);
	void setShowAppend(					// Funktion zum Setzen der showAppend Variable
		bool showApp,						// Soll die Ausgabe bei show an ein bestehendes File angehängt werden?
		bool showAppDate					// Soll das aktuelle Datum im Filenamen angehängt werden?
		);
	void setModus(						// Funktion zum Modus setzen
		uint modus							// Modus
		);				
	void setUserPass(					// Funktion zum Setzen des Usernamen, Login Passwortes und Enable Passwortes
		std::string user,					// Username
		std::string lopw,					// Login Passwort
		std::string enpw					// enable Passwort
		);	
	void setPort(						// Funktion zum Port setzen
		std::string verbPort,				// Portnummer
		std::string tscm					// Terminal Server Connection Mode
		);
	void setDefaultDatei(				// Funktion zum Setzen der Default Datei und der Statischen Datei
		std::string datei					// Dateiname
		);		
	void setLogsy(						// soll "logging synchronous" konfiguriert werden?
		uint logsy							// logsy Einstellungen
		);
	dqstring statConfig(				// Funktion für die Rückgabe einer statischen Config, die für den Caller dynamisch ist
		std::string dateiName,				// Dateiname
		bool dynPWstat						// keine Bedeutung
		);
	dqstring inventoryConf(				// Funktion zum Einlesen der richtigen Config für Inventory
		std::string dummyString,			// keine Bedeutung
		bool dummyBool						// keine Bedeutung
		);
	dqstring intfConf(					// Funktion zum Einlesen der richtigen Config für Interface
		std::string dummyString,			// keine Bedeutung
		bool dummyBool						// keine Bedeutung
		);
	dqstring mapperConf(				// Funktion zum Einlesen der richtigen Config für Mapper
		std::string dummyString,			// keine Bedeutung
		bool dummyBool						// keine Bedeutung
		);
	dqstring mapperConfIPT(				// Funktion zum Einlesen der richtigen Config für Mapper für IP Telefone
		std::string dummyString,			// keine Bedeutung
		bool dummyBool						// keine Bedeutung
		);
	dqstring leereConf(				// Funktion zum Einlesen einer leeren Config
		std::string dummyString,			// keine Bedeutung
		bool dummyBool						// keine Bedeutung
		);
	bool telnetInit();					// Zum initialisieren aller nötigen Telnet Parameter
	bool consoleInit();					// Zum Initialisieren aller nötigen COM Paramater
	bool sshInit();						// Zum Initialisieren aller nötigen SSH Paramenter
	bool httpInit();					// Zum Initialisieren aller nötigen HTTP Parameter
	bool httpsInit();					// Zum Initialisieren aller nötigen HTTPS Parameter
	bool cliInit();						// Zum Initialisieren aller nötigen CLI Parameter
//	bool httpsInitAsio();				// Zum Initialisieren aller nötigen HTTPS Parameter - ASIO Variante
	uint telnetVerbindung(				// Zum Telnet Verbindungsaufbau
		std::string adresse,				// IP Adresse oder Name
		uint status,						// Status -> neu oder weiter
		uint modus,							// Modus -> neu oder weiter
		dqstring config,					// Konfiguration
		int index							// Index der IP Adresse
		);				
	uint consoleVerbindung(				// Zum Console Verbindungsaufbau
		std::string adresse,				// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config,					// Konfiguration
		int index							// Index der IP Adresse
		);			
	uint sshVerbindung(					// Zum SSH Verbindungsaufbau
		std::string adresse,				// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config,					// Konfiguration
		int index							// Index der IP Adresse
		);
	uint httpVerbindung(				// Zum HTTP Verbindungsaufbau
		std::string adresse,				// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config,					// HTTP Befehl
		int index							// Index der IP Adresse
		);
	uint httpVerbindungAsync(			// Zum HTTP Verbindungsaufbau (Asio Async)
		std::string adresse,				// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config,					// HTTP Befehl
		int index							// Index der IP Adresse
		);
	uint httpsVerbindung(				// Zum HTTPS Verbindungsaufbau mit cryptlib
		std::string adresse,				// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config,					// HTTP Befehl
		int index							// Index der IP Adresse
		);
	uint cliVerbindung(					// Aufruf eines Befehls in der Shell (DOS oder Linux Shell) und Sammeln der Rückgabe
		std::string adresse,				// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config,					// Befehle
		int index							// Index der IP Adresse
		);
	//uint httpsVerbindungAsio(			// Zum HTTPS Verbindungsaufbau mit asio
	//	std::string adresse,				// IP Adresse oder Name
	//	uint status,						// Status
	//	uint modus,							// Modus
	//	dqstring config,					// HTTP Befehl
	//	int index							// Index der IP Adresse
	//	);
	void getDate(						// Funktion zum Auslesen des Datums
		char *datum							// Zeiger auf das Datum
		);
	std::string strfind(				// Den Wert in einem belieben String anhand eines Tags finden
		std::string zuFinden,				// Tag
		std::string basisString,			// gesamter String, in dem gesucht werden soll
		int stellenAnzahl = 7				// wie viele Stellen sollen zu pos1 dazuaddiert werden; Defaultwert = 7
		);	
	std::string getShow();				// Funktion zum Auslesen der show Ausgabe
	void skipVerbindungsTest();			// Funktion zum einstellen, dass der Test, ob die Verbindung erfolgreich war, übersprungen werden soll
	void setDebug(						// Funktion zum Setzen der Debug Einstellungen
		bool dbgSend,
		bool dbgReceive,
		bool dbgFunction,
		bool dbgBufTester,
		bool dbgHostname,
		bool dbgBufTestDet,
		bool dbgShowAusgabe,
		bool dbgRgx,
		bool dbgHex,
		bool dbgSp1,
		bool dbgSp2
		);
	void setDummyCommand(				// Dummy Command setzen; Wird bei SingleHop gemacht, um am Schluss immer einen validen Command zu haben
		bool dc								// Set/Unset
		);				
};


#endif
