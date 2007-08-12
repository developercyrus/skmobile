// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change this value to use different versions
#define WINVER 0x0420

#define _WIN32_WCE_AYGSHELL 1

#include <windows.h>

#include <atlbase.h>
#include <atlstr.h>
#include <atlsafe.h>
#include <atlutil.h>

#include <atlapp.h>

extern CAppModule _Module;
#include <atlwin.h>

#include <aygshell.h>
#pragma comment(lib, "aygshell.lib")

extern CAtlString _RootPath;

#define CALD2MOBILE_VERSION _T("0.5")