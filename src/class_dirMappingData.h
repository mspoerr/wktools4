#pragma once
#include "stdwx.h"

#include <map>
#include <string>


class CDirMappingData
{
private:
	typedef std::pair <std::string, std::string> pss;
	TiXmlDocument *doc;

public:
	CDirMappingData(TiXmlDocument *document);
	~CDirMappingData();
	void dirMapsEinlesen();
	void loeschen();

	bool dirMapsVorhanden;
	std::map <std::string, std::string> dirMapsWin;
	std::map <std::string, std::string> dirMapsLin;
	std::map <std::string, std::string>::iterator werteIter;
};
