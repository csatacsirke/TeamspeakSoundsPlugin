
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

//#define _AFXDLL

#define NO_WARN_MBCS_MFC_DEPRECATION

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS
//#define _AFXDLL

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

//


#include "resource.h"


#include <vector>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <stack>

#include <concurrent_queue.h>

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <memory>
#include <future>
//using namespace std;


#include <pluginsdk\include\ts3_functions.h>



#define NEW_SOUND_PROCESSING_VERSION 1

//enum Messages {
//	WM_PIPE_MESSAGE = WM_USER+1
//
//};

#define USE_WINDOWS_MEDIA_PACK_FEATURES FALSE



#include <iostream>
#include <fstream>


#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


#include <Gui\WindowUtil.h>
#include <afxcontrolbars.h>



namespace TSPlugin {
	using namespace std;
}






#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

