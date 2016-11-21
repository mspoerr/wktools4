#ifndef __LOGEVENT__
#define __LOGEVENT__
//
#include <boost/any.hpp>
#include <wx/event.h>

DECLARE_EVENT_TYPE( EVT_LOG_MELDUNG, -1 )

// A custom event that transports a whole wxString.
class LogEvent: public wxEvent
{
	boost::any data;

public:
	LogEvent () : wxEvent(0, EVT_LOG_MELDUNG) { }
	LogEvent (const boost::any& data) : wxEvent(0, EVT_LOG_MELDUNG), data(data) {  }
	~LogEvent () { }

	wxEvent* Clone () const { return new LogEvent(*this); }

	void SetData (const boost::any& data) { this->data = data; }
	boost::any GetData () const { return data; }

	template<typename T> T GetData () const { return boost::any_cast<T>(data); }

};


typedef void (wxEvtHandler::*LogEventFunction)(LogEvent &);

// This #define simplifies the one below, and makes the syntax less
// ugly if you want to use Connect() instead of an event table.
#define LogEventHandler(func)                                         \
	(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)\
	wxStaticCastEvent(LogEventFunction, &func)                    

// Define the event table entry. Yes, it really *does* end in a comma.
#define EVT_LOG(id, fn)                                            \
	DECLARE_EVENT_TABLE_ENTRY( EVT_LOG_MELDUNG, id, wxID_ANY,  \
	(wxObjectEventFunction)(wxEventFunction)                     \
	(wxCommandEventFunction) wxStaticCastEvent(                  \
	LogEventFunction, &fn ), (wxObject*) NULL ),

#endif
