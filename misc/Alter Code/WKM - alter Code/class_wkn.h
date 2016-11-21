/***************************************************************
 * Name:      class_wkn.h
 * Purpose:   Defines ConfigMaker I/O Class
 * Created:   2007-11-06
 **************************************************************/

#ifndef __WKN__
#define __WKN__


#include "stdwx.h"
#include "class_wknPropGrid.h"
#include "wktools4Frame.h"
#include "class_ersetzer.h"
#include "class_wkLogFile.h"
#include "class_tabellePanel.h"


class Cwktools4Frame;			// Vorwärtsdekleration
class Ersetzer;

class Wkn : public wxEvtHandler
{
    private:
        // Werte Variablen: Settings, die aus der wktools.xml Datei gelesen werden,
		//                  oder im GUI definiert werden
		enum STRS {							// Index für die String Variablen
			WERTEDATEI,							// Wertedatei
			VORLAGE,							// Vorlagendatei
			AVZ,								// Ausgabeverzeichnis
			DATVAR,								// Dateinamenvariable
			SEPERATOR,							// Zusätzlicher Seperator
			ERSATZZEICHEN,						// Durch das werden Zeilenumbrüche in Zellen ersetzt
			LOGDATEI							// Log Datei
		};
		enum INTS {							// Index für die Integer und Bool Variablen
			NONSTANDARD,						// wird verwendet, wenn ein Excel generiertes DataFile mit line Breaks verwendet wird.
			FLEXIBLE,							// Flexible Data File: Die durch Zeilenumbruch getrennten Variablen in einer Zelle
												// werden als eigenständig betrachtet
			USETAGS,							// dann wird in den Variablen nach Tags geschaut (\n usw.)
			CYCLE,								// Wie oft soll der Durchlauf wiederholt werden
			USESEPEND,							// true, wenn der Seperator die Endemarkierung für die Variablen sein soll
			MODUS,								// Modus
			APPEND								// Soll bei einer Datei, die schon besteht, die Ausgabe dazugehängt werden?
												// oder soll sie überschrieben werden.
		};		
		std::string vars[7];						// Array für die String Variablen
		int vari[7];						// Array für die Int Variablen

		// Allgemeine Einstellungen:
		bool guiStatus;						// Silent oder GUI Mode: false: GUI; true: silent - kein GUI
		TiXmlDocument *doc;					// Zeiger auf das TiXMLDoc von wktools.xml
		TiXmlElement *pElemProf;			// das Element, das auf das WKN Profil zeigt
		CWknPropGrid *wknProps;				// Das Settings Fenster
		int profilIndex;					// Index des Profils in wktools.xml
		WkLog *logAusgabe;					// Zeiger auf das Ausgabefenster
		WkLogFile *logfile;					// Zeiger auf das LogFile
		Cwktools4Frame *wkMain;				// Mainframe
		Ersetzer *derErsetzer;				// der Ersetzer
		TabellePanel *datAusgabe;			// Datenfile Ausgabe
		char varkz;							// Variablen Kennzeichner

        // Funktionen:
		int testDataFile(					// zum Testen, ob das Daten File konsistent ist
            std::string datfile						// Daten File Name
            );
		void testDataFileAdvanced(			// Darstellen des Datenfiles inklusive Fehler hervorheben
			std::string datfile						// Daten File Name
			);
        bool testNonStandardDataFile(		// NonStandard DataFiles anschauen und ändern
            std::string datfile						// DatenFile Name
            );
		bool einstellungenInit();			// Werte zum Profil werden geladen
		bool doTest(						// teste Eingaben (nur xml File)
			std::string profilName
			);
		bool doTestGui(						// teste Eingaben (GUI)
			std::string profilName
			);
		void startErsetzer();				// Thread Start
	
		enum testRet {						// Return Values für testDataFile
            WKE_NO_ERROR,						// Kein Fehler
            WKE_SEP_ERROR,						// Specified Seperator not found
            WKE_COLUMN_ERROR,					// Not all rows have the same number of columns
            WKE_PLACEHOLDER_ERROR,				// Forbidden characters used for placeholders
            WKE_PLACEHOLDER_ERROR_2				// Not all placeholders have the same beginning character
        };
		
		void OnErstellerFertig(wxCommandEvent &event);
		// Any class wishing to process wxWindows events must use this macro
		DECLARE_EVENT_TABLE()

    public:
        Wkn();								// Konstruktor
        ~Wkn();								// Destruktor
        
		boost::thread thrd;					// Worker Thread, damit das GUI nicht blockiert wird
		bool doTestDataFileOnly();			// teste Data File Eingabe
		void doIt();						// starte Tool
		void cancelIt();					// cancel Tool
		void kilIt();						// kill Tool
        bool doLoad(						// lade Profil
            std::string profilName
            );
        bool doSave(						// sichere Profil
            std::string profilName
            );
		bool doRemove(						// lösche Profil
			std::string profilName
			);
		bool xmlInit(						// Initialisieren der XML Settings
			std::string profilName,				// Profilname, der gesucht werden soll
			int speichern = 0					// Falls der Profilname nicht gefunden wird, dann soll er angelegt werden; Wenn auf 2, dann löschen
			);
		void sets(							// wichtige allgemeine Grundeinstellungen
			TiXmlDocument *document,			// XML Doc von wktools.xml
			bool guiStats,						// Silent oder GUI Mode; Wichtig
			WkLogFile *lf						// Zeiger auf das Logfile Objekt; !=NULL, wenn in das Logfile geschrieben werden soll
			);
		void guiSets(						// wichtige Grundeinstellungen
			CWknPropGrid *wknProp,				// XML Doc von wktools.xml
			WkLog *ausgabe,						// Pointer auf das Ausgabefenster
			TabellePanel *table					// Pointer auf die Datenfile Ausgabe
			);
		wxArrayString ladeProfile();		// zum Laden der Profile
		void schreibeLog(					// Funktion für die LogAusgabe
			int type,							// Log Type
			std::string logEintrag,				// Text für den Logeintrag
			std::string log2,					// Fehlercode
			int farbe = WkLog::WkLog_SCHWARZ,	// Textfarbe
			int format = WkLog::WkLog_NORMALFORMAT,	// Format
			int groesse = 10
			);
		void setMainFrame(						// zum Setzen vom MainFrame
			Cwktools4Frame *fp						// MainFrame
			);	
};



#endif
