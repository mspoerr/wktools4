#include "TaskBarIcon.h"
#include "wktools4Frame.h"

#ifdef __WXDEBUG__
#define new WXDEBUG_NEW
#endif

BEGIN_EVENT_TABLE(CTaskBarIcon, wxTaskBarIcon)
	EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLeftButtonDClick)
END_EVENT_TABLE()

CTaskBarIcon::CTaskBarIcon(wxWindow *pParent)
: wxTaskBarIcon()
, m_pParent(pParent)
{
}

CTaskBarIcon::~CTaskBarIcon(void)
{
}

void CTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent& WXUNUSED(event))
{
	((Cwktools4Frame *)m_pParent)->ShowTaskIcon(m_pParent->IsShown());
}

/**
 * \fn wxMenu* CTaskBarIcon::CreatePopupMenu()
 * \brief Creates the pop up menu for the task bar icon.
 * \return The pop up menu pointer.
 */
wxMenu* CTaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu;
	return menu;
}
