#ifndef __WKTOOLSERRMESSAGES__
#define __WKTOOLSERRMESSAGES__

#include <map>
#include <string>

class WKErr
{
public:
	WKErr();
	~WKErr();
	int getFehlerStatus(			// Status für einen Fehlercode auslesen
		std::string fehlerCode			// Fehlercode
		);
	std::string getFehlerDetail(	// Fehlerdetails auslesen
		std::string fehlerCode		// Fehlercode
		);
	typedef std::pair <std::string, std::string> errDetailPss;
	typedef std::pair <std::string, int> errStatPss;
	std::multimap <std::string, std::string> errDetail;
	std::multimap <std::string, int> errStatus;
	std::multimap <std::string, std::string>::iterator errDetailIter, errDetailAnfang, errDetailEnde;
	std::multimap <std::string, int>::iterator errStatIter, errStatAnfang, errStatEnde;

private:
	void fuellen();			// Fehler / Detailbeschreiung zuordnen; Initialisierung vom Status
};


#endif