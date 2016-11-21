#pragma once
#include "stdwx.h"
#ifdef WKTOOLS_MAPPER
#include "class_wkmdb.h"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>    

class MacUpdate
{
public:
	MacUpdate(
	std::string dbName
		);
	~MacUpdate();
	int doUpdate();				//Update starten

private:
	WkmDB *dieDB;				// Datenbank für die Einträge
};

#endif // #ifdef WKTOOLS_MAPPER
