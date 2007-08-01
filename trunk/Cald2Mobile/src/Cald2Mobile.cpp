// SKMobile.cpp : main source file for SKMobile.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include <atlmisc.h>
#include <atlctrlx.h>
#include <atlscrl.h>
#include <atlwince.h>
#include <atlddx.h>
#include <atlcrack.h>

#include "resource.h"

#include <fstream>
#include <iomanip>
#include "unicode.h"
#include <atlfile.h>
using namespace std;
using namespace mysk;

#include "DictionaryService.h"
#include "Script.h"
#include "Controller.h"

#include "PickButton.h"
#include "Cald2MobileView.h"
#include "aboutdlg.h"
#include "MainFrm.h"

// #include "ComAdapter.h"
#include "BasicController.h"
#include "Cald2Controller.h"


CAppModule _Module;
CAtlString _RootPath;

char const* ControllerNames[] = { "Cald2" };

void ControllerFactory::listAvailableControllerNames(char const* names[], unsigned * count)
{
	names = ControllerNames;
	*count = sizeof ControllerNames / sizeof (char const*);
}



Controller * ControllerFactory::create(char const* name)
{
	return new Cald2Controller();
}



int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = CMainFrame::ActivatePreviousInstance(hInstance, lpstrCmdLine);
	if(FAILED(hRes) || S_FALSE == hRes)
	{
		return hRes;
	}

	// initial root path
	TCHAR modulePath[MAX_PATH] = {0};
	GetModuleFileName(_Module.GetModuleInstance(), modulePath, sizeof modulePath / sizeof TCHAR);
	_RootPath = modulePath;
	int lastSepPos = _RootPath.ReverseFind('\\');
	_RootPath = _RootPath.Left(lastSepPos);

	hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// Calling AtlInitCommonControls is not necessary to utilize picture,
	// static text, edit box, group box, button, check box, radio button, 
	// combo box, list box, or the horizontal and vertical scroll bars.
	// Calling AtlInitCommonControls with 0 is required to utilize the spin, 
	// progress, slider, list, tree, and tab controls.
	// Adding the ICC_DATE_CLASSES flag is required to initialize the 
	// date time picker and month calendar controls.
	// Add additional flags to support additoinal controls not mentioned above.
	AtlInitCommonControls(ICC_DATE_CLASSES);

	// initial HTML control
	InitHTMLControl(hInstance);

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = 0;
	try
	{
		nRet = CMainFrame::AppRun(lpstrCmdLine, nCmdShow);
	}
	catch (truntime_error& e)
	{
		SK_TRACE(SK_LOG_INFO, _T("Catch an exception. error=%s"), 
			e.errorMsg().c_str());
	}

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
