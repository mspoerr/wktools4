#include "class_dirMappingData.h"


CDirMappingData::CDirMappingData(TiXmlDocument *document)
{
	doc = document;
	dirMapsVorhanden = false;
}

CDirMappingData::~CDirMappingData()
{

}


void CDirMappingData::dirMapsEinlesen()
{
	// Einträge aus wktools.xml auslesen und in die Maps eintragen
	TiXmlHandle hDoc(doc);
	TiXmlElement *pElem;		// ein Element
	TiXmlElement *pElemSets;	// das Element, das auf Settings zeig
	TiXmlElement *pElemDirMaps;	// das Element, das auf die DirMapping Einträge zeigt

	// Settings im wktools.xml finden
	pElem = hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (pElem)
	{
		pElemSets = pElem->FirstChildElement("Settings");
		pElemDirMaps = pElemSets->FirstChildElement("DirMapping");

		for (int i = 0; pElemDirMaps; i++)
		{
			std::string winDir;
			std::string linDir;

			if (pElemDirMaps->Attribute("Win"))
			{
				winDir = pElemDirMaps->Attribute("Win");
				linDir = pElemDirMaps->Attribute("Linux");
				// Ausgelesene Einträge in die Maps einlesen.
				dirMapsLin.insert(pss(linDir, winDir));
				dirMapsWin.insert(pss(winDir, linDir));

				dirMapsVorhanden = true;
			}

			pElemDirMaps = pElemDirMaps->NextSiblingElement();
		}
	}
}


void CDirMappingData::loeschen()
{
	dirMapsLin.clear();
	dirMapsWin.clear();
}