#ifndef __SQLCONN__
#define __SQLCONN__

#include "stdwx.h"
#include "class_logEvent.h"
#include "class_wkLog.h"
#include "class_wkLogFile.h"


#ifdef USESQL
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
//#pragma(lib, "mysqlcppconn.lib")
#endif // USESQL


#include <string>



class CSqlConn : public wxEvtHandler
{
private:
	int sqlUse;
	std::string sqlSrv;
	std::string sqlUser;
	std::string sqlPass;
	std::string sqlStatement;
	std::string descr;
	std::string retErr;
	bool nocon;
	void OnLog(						// Funktion, die auf LogEvent ausgeführt wird und das Logfile beschreibt
		LogEvent &event					// Event Daten
		);
	int schreibeEintrag(			// Eintrag in SQL DB schreiben
		std::string hostid,					// HostID 
		std::string error					// Errorcode
		);

#ifdef USESQL
	sql::Driver *driver;
	sql::Connection *con;
#endif // USESQL

	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

public:
	CSqlConn();						// Konstruktor
	~CSqlConn();					// Destruktor

	WkLog *logAusgabe;				// Zeiger auf das Ausgabefenster
	WkLogFile *logFile;				// Zeiger auf das Logfile

	int sets(						// Einstellungsverbindungen
		int useit,						// Connect -> JA/Nein
		std::string host,					// IP/Hostname des DB Servers
		std::string user,					// User
		std::string password,				// Passwort
		std::string desc						// Description (Unter WKE für die HostID verwendet)
		);
	void setDesc(					// Description/HostID setzen
		std::string dessc
		);

};

#endif // __SQLCONN__
