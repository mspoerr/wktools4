#pragma once
#include "stdwx.h"

#ifdef WKTOOLS_MAPPER

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>
#include <iostream>

class WkmDB
{
private:
	sqlite3 *wkmdb;						// Die Datenbank
	std::string filename;				// DB File Name
	bool inmem;							// Load DB in Memory?

public:
	WkmDB(								// Konstruktor
		std::string filename,				// DB Filename
		bool im								// Load DB in-Memory? -> True: Ja; False: Nein
		);	
	~WkmDB();							// Destruktor

	bool open(							// DB ˆffnen
		bool im								// Load DB in Memory? -> True: Ja; False: Nein
		);
	std::vector<std::vector<std::string> > query(		// DB Abfrage
		const char* query
		);
	bool close();						// DB schlieﬂen
	std::string error;					// DB Error Code
};



#endif