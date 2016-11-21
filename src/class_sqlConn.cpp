#include "class_sqlConn.h"
#include <boost/lexical_cast.hpp>



BEGIN_EVENT_TABLE(CSqlConn, wxEvtHandler)
EVT_LOG(wxID_ANY, CSqlConn::OnLog)
END_EVENT_TABLE()



CSqlConn::CSqlConn() : wxEvtHandler()
{
	nocon = true;
	logAusgabe = NULL;
	sqlStatement = "";
	descr = "";
	retErr = "";
}


CSqlConn::~CSqlConn()
{

}


void CSqlConn::OnLog(LogEvent &event)
{
#ifdef USESQL
	if (sqlUse)
	{
		boost::any data = event.GetData();
		if (data.type() == typeid(WkLog::evtData))
		{
			WkLog::evtData evtDat = event.GetData<WkLog::evtData>();
			int ret = schreibeEintrag(evtDat.logEintrag3, evtDat.logEintrag2);
			if (ret)
			{
				std::string fehlermeldung = "";
				switch (ret)
				{
				case 1:
					// Kein Error Code
					fehlermeldung += "0303: SQL Error: No wktools Error Code specified!";
					break;
				case 2:
					// Verbindungsfehler
					fehlermeldung += "0304: SQL Error: Database Connection Error!";
					break;
				default:
					// MySQL Fehlercode
					fehlermeldung += "0305: SQL Error: MySQL Error Code: " + boost::lexical_cast<std::string>(ret);
					fehlermeldung += "\n";
					fehlermeldung += retErr;
					break;
				}
				if (logAusgabe != NULL)
				{
					logAusgabe->schreibeLog(WkLog::WkLog_ABSATZ, fehlermeldung, "", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				}
			}
		}
	}
#endif
}



int CSqlConn::sets(int useit, std::string host, std::string user, std::string password, std::string desc)
{
	sqlUse = useit;
	if (useit)
	{
		sqlSrv = host;
		sqlUser = user;
		sqlPass = password;
		if (sqlSrv == "" || sqlUser == "" || sqlPass == "")
		{
			return 1;
		}

		std::string connStr = "tcp://" + host + ":3306";

	#ifdef USESQL
		try 
		{
			/* Create a connection */
			driver = get_driver_instance();
			con = driver->connect(connStr, user, password);
			con->setSchema("NTSinventory");
			nocon = false;
		}
		catch (sql::SQLException &e) 
		{
			nocon = true;
			return 2;
		}

		// Host ID von der Datenbank auslesen
		if (desc == "")
		{
			descr = "0";
		}
		else
		{
			setDesc(desc);
		}


	#endif
	}

	return 0;
}


int CSqlConn::schreibeEintrag(std::string hostid, std::string error)
{
	if (hostid == "")
	{
		hostid = descr;
	}

	if (error == "")
	{
		return 1;
	}
	
	// Nur einfügen, wenn Verbindung besteht
	if (!nocon)
	{
		// INSERT INTO wktools_device_log SET device_host_id = 58, wktools_codes_id = 2308, wktools_source_id = 2, wktools_level_id = 3, time = now();
		sqlStatement = "INSERT INTO wktools_device_log SET devices_host_id = " + hostid;
		sqlStatement += ", wktools_codes_id = ";
		sqlStatement += error;
		sqlStatement += ", wktools_source_id = ";
		sqlStatement += error[0];
		sqlStatement += ", wktools_level_id = ";
		sqlStatement += error[1];
		sqlStatement += ", time = now();";

	#ifdef USESQL
		try 
		{
			sql::Statement *stmt;
			sql::ResultSet *res;

			stmt = con->createStatement();
			res = stmt->executeQuery(sqlStatement);
		}
		catch (sql::SQLException &e) 
		{
			retErr = e.what();
			if (retErr != "")
			{
				std::string old = sqlStatement;
				
				if (retErr.find("devices_host_id") != retErr.npos)
				{
					// Host ID nicht gefunden
					sqlStatement = "INSERT INTO wktools_device_log SET devices_host_id = " + descr;
					sqlStatement += ", wktools_codes_id = ";
					sqlStatement += error;
					sqlStatement += ", wktools_source_id = ";
					sqlStatement += error[0];
					sqlStatement += ", wktools_level_id = ";
					sqlStatement += error[1];
					sqlStatement += ", description = 'Host ID not found: ";
					sqlStatement += hostid;
					sqlStatement += "', time = now();";
				}
				
				if (retErr.find("wktools_codes_id") != retErr.npos)
				{
					// Error Code nicht gefunden
					sqlStatement = "INSERT INTO wktools_device_log SET devices_host_id = " + hostid;
					sqlStatement += ", wktools_codes_id = 0";
					sqlStatement += ", wktools_source_id = 0";
					sqlStatement += ", wktools_level_id = 0";
					sqlStatement += ", description = 'wktools Error Code not found: ";
					sqlStatement += error;
					sqlStatement += "', time = now();";
				}

				retErr += "\n";
				retErr += sqlStatement;

				int origErr = e.getErrorCode();
				try
				{

					sql::Statement *stmt;
					sql::ResultSet *res;

					stmt = con->createStatement();
					res = stmt->executeQuery(sqlStatement);
				}
				catch (sql::SQLException &e1)
				{
					retErr += "\n";
					retErr = e1.what();
					retErr += "\n";
					retErr += sqlStatement;
				}
				return origErr;
			}

		}

	#endif
		return 0;

	}

	return 2;
}


void CSqlConn::setDesc(std::string desc)
{
#ifdef USESQL
	if (sqlUse)
	{
		if (!nocon)
		{
			try
			{
				sql::Statement *stmt;
				sql::ResultSet *res;

				sqlStatement = "SELECT monarch_host_id FROM devices WHERE sysName like '";
				sqlStatement += desc;
				sqlStatement += "'";
				stmt = con->createStatement();
				res = stmt->executeQuery(sqlStatement);
				res->next();
				int ret = res->getInt("monarch_host_id");
				descr = boost::lexical_cast<std::string>(ret);
			}

			catch (sql::SQLException &e) 
			{
				descr = "0";
				std::string fehlermeldung = "0305: SQL Error: MySQL Error Code: " + boost::lexical_cast<std::string>(e.getErrorCode());
				fehlermeldung += "\n";
				fehlermeldung += e.what();
				fehlermeldung += "\n";
				fehlermeldung += sqlStatement;
				if (logAusgabe != NULL)
				{
					logAusgabe->schreibeLog(WkLog::WkLog_ABSATZ, fehlermeldung, "0305", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
				}
			}
		}
	}


#endif
}