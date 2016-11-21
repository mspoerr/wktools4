#pragma once
#include "stdwx.h" 
#include <wx/taskbar.h>

class CTaskBarIcon : public wxTaskBarIcon
{
private:
	/**
	 * \var wxWindow *m_pParent;
	 * \breif Pointer to the main application frame/window.
	 */
	wxWindow *m_pParent;

public:
	CTaskBarIcon(wxWindow *pParent);
	virtual ~CTaskBarIcon(void);

private:
	virtual wxMenu *CreatePopupMenu();

private:
	void OnLeftButtonDClick(wxTaskBarIconEvent& WXUNUSED(event));

private:
	DECLARE_EVENT_TABLE()
};
