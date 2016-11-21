#ifndef __ERSETZER__
#define __ERSETZER__

#include "stdwx.h"
#include "class_wkn.h"

#include <fstream>

typedef unsigned int uint;

class Wkn;

class Ersetzer
{
private:
	std::ofstream logAusgabeDatei;			// Die log-Datei
	uint mods;							// Modus als UINT
	uint modus;							// Modus
	int cycle;							// Anzahl der Durchläufe
	std::string seps;						// Seperatoren
	bool flexible;						// Verwendung von flexiblen Datenfiles
	bool tags;							// Verwendung von Tags?
	int tagStart;						// Position von $$<
	int tagEnde;						// Position von $$>
	WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
	Wkn *wkErsetzer;					// Zeiger auf Wkn Parent, um die Events richtig zu senden
	
	std::string werteDatei;					// Wertedatei
	std::string vorlageDatei;				// Vorlagendatei
	std::string dateiVar;					// Dateivariable
	std::string ausgabeVz;					// Ausgabeverzeichnis
	int append;								// Ausgabe anhängen
	bool useSepEnd;							// Seperator als Variableende benutzen

	void schreibeLog(					// Funktion für die LogAusgabe
		int type,							// Log Type
		std::string logEintrag,				// Text für den Logeintrag
		std::string log2,					// Fehlercode
		int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
		int format = WkLog::WkLog_NORMALFORMAT,	// Format
		int groesse = 10
		);

public:
	Ersetzer(							// Konstruktor
		std::string logfile,				// Name des Logfiles
		WkLog *logAusgabe,					// Zeiger auf das Debug Fenster
		Wkn *wkE							// Zeiger auf Wkn für die Events
		);			
	~Ersetzer();
	
	volatile bool cancel;				// alle Aktivitäten abbrechen und zurückkehren
	void schreibeLogfile(				// zum Schreiben von Einträgen in die Log Datei
		std::string logInput				// String, der in die Log Datei geschrieben werden soll
		);
	uint ersetze();						// zum Erzeugen und Vervielfältigen der Vorlage
	void ersetzeInit(					// zum Initialisieren der wichtigen Einstellungen, um den ersetzer zu starten
		std::string werteDatei,				// Wertedatei: Datei, in der die variablen Daten zu finden sind
		std::string vorlageDatei,			// Vorlagendatei: Vorlage, die zur Vervielfältigung herangezogen wird
		std::string dateiVar,				// Dateinamenvariable: Der Wert für diese Variable wird für den Dateinamen jeder einzelnen Kopie verwendet
		std::string ausgabeVz,				// Verzeichnis, in dem die erstellten Dateien gespeichert werden sollen
		int AppendMenu,						// Ausgabe an Datei anhängen oder Datei überschreiben
		bool useSepEnd						// Den Seperator als Ende Indentifier für die Variablen nutzen
		);
	void wknEinstellungen(				// zum Setzen diverser Einstellungen
		std::string sep,					// Seperatoren
		uint md,							// verwedendeter Modus -> siehe enum mods
		int ck,								// Anzsahl der Durchläufe
		bool flex,							// Verwendung von flexiblen Datenfiles
		bool tg								// Verwendung von Tags
		);
	enum mods {							// verfügbare Modi
		FEHLER,								// Rückgabewert wenn "ersetze()" einen Fehler erkennt
		GUT,								// Rückgabewert wenn "ersetze()" ohne Fehler zurückkehrt
		PZED = 10,							// Pro Wert eine Datei => default Einstellung
		EINEDATEI							// Alle Werte in einer Datei eintragen
	};
};

#endif