#ifndef __VERBINDER__
#define __VERBINDER__

#include <iostream>
#include <string>
#include <deque>
#include <fstream>
#include <vector>
using namespace std;

#include <cryptlib.h>

#include "stdwx.h"
#include "class_wkLog.h"
#include "class_wke.h"

using asio::ip::tcp;


typedef unsigned short int uint;
typedef deque<string> dqstring;

class Wke;

class Verbinder
{
private:	
	WkLog *asgbe;						// Zeiger auf das Fenster, das die Nachrichten mit den Lgos entgegennimmt
	Wke *wkVerbinder;					// Zeiger auf Wke Parent, um die Events richtig zu senden
	string fehler;						// der Fehler in Text
	tcp::socket verbindungsSock;		// Socket für Telnet
	asio::io_service &ioserv;			// IO Service für ASIO Sockets
	asio::serial_port serPort;			// Serielles Port
	tcp::resolver::iterator endpoint_iterator;		// Iterator für Hostnamen

	static const uint BUFFER_SIZE;		// Buffergroeße für Sende/Empfangsbuffer

	CRYPT_SESSION cryptSession;			// SSH Session
	string hostname;					// Hostname des gerade zu bearbeitenden Gerätes
	string hostnameAlt;					// Hostname des vorigen Gerätes
	string ip;							// IP Adresse
	string vPort;						// Portnummer
	string defaultDatei;				// Default Datei
	vector <string> username;			// Username
	vector <string> loginpass;			// Login Passwort
	vector <string> enablepass;			// Enable Passwort
	string enable;						// "Was tun bei enable?"
	string erweiterung;					// Dateierweiterung für dynamische Configs
	string pfad;						// Pfad für dynamische Configs
	string ausgabePfad;					// Ausgabeverzeichnis für die Show Ausgabe
	string shString;					// String, in dem die Show Ausgabe zwischengespeichert wird
	string multiHop;					// Was soll bei MultiHop für die weiteren Verbindungen gemacht werden
	string multiHopPost;				// Was wird bei MultiHop hinter der IP Adresse noch hinzugefügt?
	string nichtsString;				// Wenn buftest = NICHTS, dann werden die Empfangsdaten hier gespeichert
	uint MODUS;							// Welcher Modus wird verwendet -> global
	uint STATUS;						// Status vom Modus -> für die jetzige Verbindung
	uint logOpt;						// Optionen für den Dateinamen für die Ausgabe der show Rückgabe
	uint iosAnfangsSettings;			// sollen bei IOS Geräten "logging sync" und/oder "term len 0" gesendet werden?
	uint enPWf;							// was tun, wenn enable PW falsch oder nicht vorhanden
	uint ulez;							// user, lopw, enpw Zähler: Wenn falsch, wird er nach oben gezählt
	uint ulezmax;						// ulez max Zähler: wie viele user/passworte stehen maximal zur Verfügung
	uint ulezmaxEn;						// ulexmax zum runterzählen für enable Passwort
	uint sshVersion;					// SSH Version
	bool NICHTSaendertBufTest;			// wird auf TRUE gesetzt, wenn bei outBufZusammensteller (NICHTS) der bufTester geändert werden soll.
	bool bufTestAenderung;				// zeigt die tatsächliche Änderung des Buftesters an
	bool schonAuthe;					// wurde schon authentifiziert? Überwachung des Authentifizierungsstatus
	bool schonEnableAuthe;				// wurde für den enable Mode schon authentifiziert?
	bool execKontrolle;					// Zum Feststellen, ob schon einmal versucht wurde, in den enable Modus zu wechseln	
	bool hostnameAuslesen;				// Hostnamen auslesen?
	bool keineDatei;					// Falls bei dynamischern Konfigurationen die spezifische Datei nicht gefunden werden kann
	bool ersterDurchlauf;				// Zum Feststellen, ob die default Konfig eingelesen werden muss
	bool showGesendet;					// Zum Festhalten, ob ein "show" Befehl abgesetzt wurde
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
	bool debugEmpfangen;				// Debug Flag: Empfangsdaten werden detailiiert ausgegeben
	bool debugSenden;					// Debug Flag: Sendedaten werden ausgegeben
	bool debugFunctionCall;				// Debug Flag: Jeder Funktionsaufruf wird ausgegeben
	bool debugBufTester;				// Debug Flag: Buftester Infos werden ausgegeben
	bool debugHostname;					// Debug Flag: NameAuslesen wird ausgegeben
	bool debugbufTestDetail;			// Debug Flag: detaillierte Buftest Info wird ausgegeben
	bool debugShowAusgabe;				// Debug Flag: detaillierte Infos zur Show Ausgabe werden ausgegeben
	dqstring defaultConfig;				// die defaultConfig wird darin gespeichert
	dqstring config;						// die Config
//	HANDLE konsole;						// für die Erstellung der Konsole I/O
	size_t outBufZaehler;				// Zum Feststellen der Göße des Sendebuffers
	uint empfangsOffset;				// Ab welcher Position müssen die empfangenen Daten überprüft werden
	char *outBuf;						// Die zu sendenden Daten
	string zuletztGesendet;				// Die zuletzt gesendeten Daten
	string zuletztEmpfangen;			// Die zuletzt empfangenen Daten
	ofstream showAusgabe;				// Die Datei, in der die Rückgabe eines "show" Kommandos ausgegeben werden soll
// 	ofstream troubleshooting;			// Ausgabe in eine Datei zu Troubleshootingzwecken
// 	uint trcounter;						// Zaehler für Troubleshooting Zwecke
	bool nullEaterEnable;				// NullEater Funktion einschalten
	struct evtData {					// Daten, die mit dem Event an das Logfenster gegeben werden
		int type;
		string logEintrag;
		int farbe;
		int format;
		int groesse;						
		string ipa;							// derzeitige IP Adresse
	};

	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		string logEintrag,					// Text für den Logeintrag
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);
	string nullEater(					// zum Entfernen von NULL in CatOS Empfangsdaten
		string catOSdaten					// Empfangsdaten
		);
	bool setSockEinstellungen(			// zum Setzen von IP Adresse und TCP Port
		string ip,							// IP Adresse
		string port							// TCP Port
		);		
	uint telnetOptionen(				// zum Auswerten der Telnetoptionen beim Verbindungsaufbau
		const char *buf						// Daten, die ausgewertet werden sollen
		);					
	uint telnetOptionenFinder(			// zum Auffinden von Telnet Optionen mitten im Eingangspuffer
		char *buf,							// Daten, in denen nach Telnet Optionen gesucht werden soll
		size_t bufLaenge					// Länge der Daten
		);
	uint IOSbufTester(					// zum Auswerten des Empfangsbuffers bei Verbindungen mit Cisco IOS Boxen
		string buf, 						// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint PIXbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit Cisco PIXen
		string buf, 						// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint CATbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit Cisco Catalyst Switches (CatOS)
		string buf, 						// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint UTAbufTester(					// zum Auswerten des Empfangspuffers bei Verbindungen mit der UTA Managment Konsole
		string buf,							// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint loginBufTester(				// zum Auswerten des Empfangspuffers, solange das Login noch nicht abgeschlossen ist
		string buf, 						// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint loginTest(						// zum Auswerten, ob User/Pass im Puffer vorkommt
		string buf, 						// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint loginModeTest(					// zum Auswerten des Puffers, wenn sich das Gerät im login Modus befindet
		string buf, 						// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint enableModeTest(				// zum Auswerten des Puffers, wenn sich das Gerät im enable Modus befindet
		string buf, 						// Daten, die ausgewertet werden sollen
		uint bufStat						// Code, der festlegt, was gesendet werden soll
		);
	uint upFinder(						// Funktion zum Username/Passwort aus der config auslesen
		string up,							// String, der nach User und PW durchsucht werden soll
		uint upwZaehler						// Zähler, damit festgestellt werden kann wie viele User/PW schon gefunden wurden
		);				
	uint outBufZusammensteller(			// Funktion zum Füllen des Sendebuffers
		uint bufStat,						// Code, der festlegt, was gesendet werden soll
		string buf							// Eingangspuffer, der z.B.: für die "show" Ausgabe verwendet wird
		);	
	void nameauslesen(					// Funktion zum Auslesen des Hostnames
		string buf							// Puffer, in dem der Hostname vorkommt
		);
	void IOSnameauslesen(				// Funktion zum Auslesen des Hostnames
		string buf							// Puffer, in dem der Hostname vorkommt
		);
	void PIXnameauslesen(				// Funktion zum Auslesen des Hostnames
		string buf							// Puffer, in dem der Hostname vorkommt
		);
	void CATnameauslesen(				// Funktion zum Auslesen des Hostnames
		string buf							// Puffer, in dem der Hostname vorkommt
		);
	void UTAnameauslesen(				// Funktion zum Auslesen des Hostnames
		string buf							// Puffer
		);
	void schreibeLog(					// Funktion zum Schreiben von Logs
		string log,							// String der geschrieben werden soll
		uint dringlichkeit					// Welche Dringlichkeit hat die Meldung?
		);
	void initVerbinderVars();			// zum Initialisieren der mehrfach genutzten Variablen

public:
	Verbinder(							// Konstruktor
		WkLog *logAusgabe,					// Zeiger auf das Debug Fenster
		Wke *wkV,							// Zeiger auf Wkn für die Events
		asio::io_service& io_service		// IO_Service für ASIO Socket Initialisierung
		);
	~Verbinder();

	enum logopts { ipN = 0,				// Logoptionen; Dateiname = IP Adresse
				   hoN,					// Dateiname = Hostname
				   siN,					// Dateiname für alle Hosts gleich
				   poi					// Daten werden nicht in Datei sondern einem Zeiger gespeichert
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
		CTELNET,						// MultiHop zur nächsten IP Adresse; Als Protokoll wird das in string multiHop gespeicherte verwendet
		CSSH1,							// SSHv1 zur nächsten IP Adresse
		CSSH2,							// SSHv2 zur nächsten IP Adresse
		EXIT,							// "exit" senden
		KEINEOPTIONEN,					// keine Telnet Optionen gefunden
		OPTIONENGEF,					// Telnet Optionen gefunden
		WZNIP,							// Weiter zur nächsten IP Adresse
		FEHLERMELDUNGALT,				// Übernahme der Fehlermeldung
		PAGERLINE,						// "pager line 0" senden
		SETLENGTH,						// "set length 0" senden
		YES,							// "yes" senden
		NO								// "no" senden
		};
	volatile bool cancelVerbindung;		// alle Aktivitäten abbrechen und zurückkehren
	void setEinstellungen(				// Funktion zum Setzen diverser Einstellungen
		string enable,						// "Was tun bei enable" setzen
		int logo,							// Setzen der Log Optionen
		string ausgabeVz,					// Ausgabeverzeichnis der Show Ausgabe
		string mh,							// Was tun bei MultiHop
		int enaPWf, 						// setzen von "Was tun wenn enable PW falsch"
		int raute							// auf Raute warten?
		);
	void setMultiHop(					// Funktion zum Setzen des Multihop Commands
		string mh							// Was tun bei Mutlihop
		);
	void setShowAppend(					// Funktion zum Setzen der showAppend Variable
		bool showApp						// Soll die Ausgabe bei show an ein bestehendes File angehängt werden?
		);
	void setModus(						// Funktion zum Modus setzen
		uint modus							// Modus
		);				
	void setUserPass(					// Funktion zum Setzen des Usernamen, Login Passwortes und Enable Passwortes
		string user,						// Username
		string lopw,						// Login Passwort
		string enpw							// enable Passwort
		);	
	void setPort(						// Funktion zum Port setzen
		string verbPort,					// Portnummer
		string tscm							// Terminal Server Connection Mode
		);
	void setDefaultDatei(				// Funktion zum Setzen der Default Datei und der Statischen Datei
		string datei						// Dateiname
		);		
	void setLogsy(						// soll "logging synchronous" konfiguriert werden?
		uint logsy							// logsy Einstellungen
		);
	dqstring statConfig(				// Funktion für die Rückgabe einer statischen Config, die für den Caller dynamisch ist
		string dateiName,					// Dateiname
		bool dynPWstat						// keine Bedeutung
		);
	dqstring inventoryConf(				// Funktion zum Einlesen der richtigen Config für Inventory
		string dummyString,					// keine Bedeutung
		bool dummyBool						// keine Bedeutung
		);
	dqstring intfConf(					// Funktion zum Einlesen der richtigen Config für Inventory
		string dummyString,					// keine Bedeutung
		bool dummyBool						// keine Bedeutung
		);
	void telnetInit();					// Zum initialisieren aller nötigen Telnet Parameter
	void consoleInit();					// Zum Initialisieren aller nötigen COM Paramater
	void sshInit();						// Zum Initialisieren aller nötigen SSH Paramenter
	void httpInit();					// Zum Initialisieren aller nötigen HTTP Parameter
	uint telnetVerbindung(				// Zum Telnet Verbindungsaufbau
		string adresse,						// IP Adresse oder Name
		uint status,						// Status -> neu oder weiter
		uint modus,							// Modus -> neu oder weiter
		dqstring config						// Konfiguration
		);				
	uint consoleVerbindung(				// Zum Console Verbindungsaufbau
		string adresse,						// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config						// Konfiguration
		);			
	uint sshVerbindung(					// Zum SSH Verbindungsaufbau
		string adresse,						// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config						// Konfiguration
		);
	uint httpVerbindung(				// Zum HTTP Verbindungsaufbau
		string adresse,						// IP Adresse oder Name
		uint status,						// Status
		uint modus,							// Modus
		dqstring config						// HTTP Befehl
		);
	void getDate(						// Funktion zum Auslesen des Datums
		char *datum							// Zeiger auf das Datum
		);
	string strfind(					// Den Wert in einem belieben String anhand eines Tags finden
		string zuFinden,				// Tag
		string basisString,				// gesamter String, in dem gesucht werden soll
		int stellenAnzahl = 7			// wie viele Stellen sollen zu pos1 dazuaddiert werden; Defaultwert = 7
		);	
	string getShow();				// Funktion zum Auslesen der show Ausgabe
	void skipVerbindungsTest();		// Funktion zum einstellen, dass der Test, ob die Verbindung erfolgreich war, übersprungen werden soll
	void setDebug(					// Funktion zum Setzen der Debug Einstellungen
		bool dbgSend,
		bool dbgReceive,
		bool dbgFunction,
		bool dbgBufTester,
		bool dbgHostname,
		bool dbgBufTestDet,
		bool dbgShowAusgabe
		);
};


#endif
