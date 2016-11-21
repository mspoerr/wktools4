#ifndef __ENHANCED_STRING_PROPRETY__
#define __ENHANCED_STRING_PROPRETY__

#include "stdwx.h"
#include <wx/propgrid/propdev.h>

class eStringProperty : public wxLongStringProperty
{
public:

	eStringProperty();
	// Normal property constructor.
	eStringProperty(const wxString& label,
		const wxString& name = wxPG_LABEL,
		const wxString& value = wxEmptyString);

	// Do something special when button is clicked.
	bool OnButtonClick( wxPropertyGrid* propgrid, wxString& value );
	

private:
	WX_PG_DECLARE_PROPERTY_CLASS(eStringProperty)

};


 WX_PG_IMPLEMENT_PROPERTY_CLASS(eStringProperty, wxLongStringProperty, wxString, const wxString&, TextCtrlAndButton)

eStringProperty::eStringProperty(const wxString& label, const wxString& name, const wxString& value)
: wxLongStringProperty(label,name,value)
{

}	

 eStringProperty::eStringProperty()
 {

 }

// Do something special when button is clicked.
bool eStringProperty::OnButtonClick( wxPropertyGrid* propgrid, wxString& value )
{
	wxCommandEvent event(wkEVT_SCHED_NEU);
	event.SetInt(1);
	wxPostEvent(GetGrid(), event);
	
	return true;
}

#endif