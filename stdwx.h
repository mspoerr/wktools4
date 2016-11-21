
#ifndef _stdwx_h_
#define _stdwx_h_

//Subtool Defines
#define WKTOOLS_MAPPER
//#define USESQL

// Boost ASIO
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501
#include <boost/asio.hpp>

// Boost Thread
#include <boost/thread/thread.hpp>

// SYSTEM INCLUDES
#include <wx/wxprec.h>
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// APPLICATION INCLUDES
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/string.h>

// TinyXML
#include "tinyxml.h"
// Eventing
#include "wkEvents.h"
#include "class_wkLog.h"


#ifdef __VISUALC__
#ifdef __WXDEBUG__
#include <crtdbg.h>
#undef WXDEBUG_NEW
#define WXDEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#undef WXDEBUG_NEW
#define WXDEBUG_NEW new
#endif
#endif

#if _MSC_VER>=1400
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#endif


