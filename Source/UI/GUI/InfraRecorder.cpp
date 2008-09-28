/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "resource.h"
#include "MainFrm.h"
#include "SplashWindow.h"
#include "DeviceManager.h"
#include "LogDlg.h"
#include "Settings.h"
#include "SettingsManager.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "ActionManager.h"
#include "TempManager.h"
#include "InfraRecorder.h"
#include "InfoDlg.h"

CAppModule _Module;

// Enable splash screen.
#define SHOW_SPLASH

// Global codec manager object.
CCodecManager g_CodecManager;

void PerformDeviceScan()
{
#ifdef SHOW_SPLASH
	// Display the slash screen.
	CSplashWindow *pSplashWindow = new CSplashWindow();

	RECT rcDefault = { 0,0,200,200 };
	pSplashWindow->Create(NULL,rcDefault);
	pSplashWindow->SetMaxProgress(3);

	// Scan SCSI/IDE busses.
	pSplashWindow->SetInfoText(lngGetString(INIT_SCANBUS));
#endif
	bool bScanBusRes = g_DeviceManager.ScanBus();
#ifdef SHOW_SPLASH
	pSplashWindow->SetProgress(1);

	// Load device capabilities.
	pSplashWindow->SetInfoText(lngGetString(INIT_LOADCAPABILITIES));
#endif
	bool bLoadCap = g_DeviceManager.LoadCapabilities();
#ifdef SHOW_SPLASH
	pSplashWindow->SetProgress(2);

	// Load extended device infomation.
	pSplashWindow->SetInfoText(lngGetString(INIT_LOADINFOEX));
#endif
	bool bLoadInfoEx = g_DeviceManager.LoadExInfo();
#ifdef SHOW_SPLASH
	pSplashWindow->SetProgress(3);

	pSplashWindow->DestroyWindow();
	delete pSplashWindow;
#endif

	// Check of we failed to scan the bus.
	if (!bScanBusRes)
		lngMessageBox(HWND_DESKTOP,FAILURE_SCANBUS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
	else if (!bLoadCap)
		lngMessageBox(HWND_DESKTOP,FAILURE_LOADCAP,GENERAL_ERROR,MB_OK | MB_ICONERROR);
	else if (!bLoadInfoEx)
		lngMessageBox(HWND_DESKTOP,FAILURE_LOADINFOEX,GENERAL_ERROR,MB_OK | MB_ICONERROR);
}

int Run(LPTSTR lpstrCmdLine = NULL,int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop MainLoop;
	_Module.AddMessageLoop(&MainLoop);

	if (g_MainFrame.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	// Load the last used window position and size.
	if (g_DynamicSettings.m_rcWindow.left != -1 &&
		g_DynamicSettings.m_rcWindow.right != -1 &&
		g_DynamicSettings.m_rcWindow.top != -1 &&
		g_DynamicSettings.m_rcWindow.bottom != -1)
	{
		g_MainFrame.SetWindowPos(HWND_TOP,&g_DynamicSettings.m_rcWindow,0);
	}
	else
	{
		g_MainFrame.CenterWindow();
	}

	g_MainFrame.ShowWindow(g_DynamicSettings.m_bWinMaximized ? SW_SHOWMAXIMIZED : nCmdShow);

	int nRet = MainLoop.Run();
		_Module.RemoveMessageLoop();
	return nRet;
}

INT_PTR ParseAndRun(LPTSTR lpstrCmdLine,int nCmdShow = SW_SHOWDEFAULT)
{
	// From the express application.
	if (!lstrcmp(lpstrCmdLine,_T("-datadvdproject")))
	{
		g_MainFrame.m_bDefaultProjDataDVD = true;
	}
	if (!lstrcmp(lpstrCmdLine,_T("-audioproject")))
	{
		g_MainFrame.m_iDefaultProjType = PROJECTTYPE_AUDIO;
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-mixedproject")))
	{
		g_MainFrame.m_iDefaultProjType = PROJECTTYPE_MIXED;
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-dvdvideoproject")))
	{
		g_MainFrame.m_bDefaultProjDataDVD = true;
		g_MainFrame.m_bDefaultProjDVDVideo = true;
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-burnimage")))
	{
		return g_ActionManager.BurnImage(NULL,true);
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-copyimage")))
	{
		return g_ActionManager.CopyImage(NULL,true);
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-tracks")))
	{
		return g_ActionManager.ManageTracks(true);
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-erase")))
	{
		return g_ActionManager.Erase(NULL,true);
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-fixate")))
	{
		return g_ActionManager.Fixate(NULL,true);
	}
	else if (!lstrcmp(lpstrCmdLine,_T("-copydisc")))
	{
		return g_ActionManager.CopyDisc(NULL,true);
	}
	// From the shell extension.
	else if (!lstrncmp(lpstrCmdLine,_T("-burnimage "),11))
	{
		return g_ActionManager.BurnImageEx(NULL,true,lpstrCmdLine + 11);
	}
	else if (!lstrncmp(lpstrCmdLine,_T("-burnproject "),13))
	{
		if (g_ProjectManager.LoadProject(lpstrCmdLine + 13))
			return g_ActionManager.BurnCompilation(NULL,true);

		return false;
	}
	// General, open file.
	else if (lpstrCmdLine[0] != '\0')
	{
		// Strip quotes.
		/*if (lpstrCmdLine[0] == '\"')
		{
			lstrcpy(g_MainFrame.m_szProjectFile,lpstrCmdLine + 1);
			g_MainFrame.m_szProjectFile[lstrlen(g_MainFrame.m_szProjectFile) - 1] = '\0';
		}
		else
		{
			lstrcpy(g_MainFrame.m_szProjectFile,lpstrCmdLine);
		}*/

		TCHAR *szFullPath = new TCHAR[lstrlen(lpstrCmdLine) + 1];

		// Strip quotes.
		if (lpstrCmdLine[0] == '\"')
		{
			lstrcpy(szFullPath,lpstrCmdLine + 1);
			szFullPath[lstrlen(szFullPath) - 1] = '\0';
		}
		else
		{
			lstrcpy(szFullPath,lpstrCmdLine);
		}

		// Check what type of file that was specified.
		int iDelim = LastDelimiter(lpstrCmdLine,'.');
		if (iDelim != -1)
		{
			// FIXME: Move this check to a common class for disc image management?
			if (!lstrcmpi(szFullPath + iDelim,_T(".iso")) ||
				!lstrcmpi(szFullPath + iDelim,_T(".img")) ||
				!lstrcmpi(szFullPath + iDelim,_T(".cue")) ||
				!lstrcmpi(szFullPath + iDelim,_T(".bin")) ||
				!lstrcmpi(szFullPath + iDelim,_T(".raw")))
				return g_ActionManager.BurnImageEx(NULL,true,szFullPath);
		}

		lstrcpy(g_MainFrame.m_szProjectFile,szFullPath);
		delete [] szFullPath;
	}

	return Run(lpstrCmdLine,nCmdShow);
}

/*INT_PTR*/int WINAPI _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpstrCmdLine,int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// This resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used.
	::DefWindowProc(NULL,0,0,0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

	hRes = _Module.Init(NULL,hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	// Load the configuration.
	g_SettingsManager.Load();

	// Create the log dialog.
	g_LogDlg.Create(HWND_DESKTOP);

	// Translate some of the string tables.
	lngTranslateTables();

	// Initialize, SCSI buses etc.
	if (g_DeviceManager.LoadConfiguration())
	{
		if (g_GlobalSettings.m_bAutoCheckBus)
		{
			if (!g_DeviceManager.VerifyConfiguration())
			{
				if (lngMessageBox(HWND_DESKTOP,INIT_FOUNDDEVICES,GENERAL_QUESTION,MB_YESNO | MB_ICONQUESTION) == IDYES)
					PerformDeviceScan();
			}
		}
	}
	else
	{
		// If we failed to load drive configuration we scan the busses while
		// displaying the splash screen.
		PerformDeviceScan();
	}

	// Load the codecs.
	TCHAR szCodecPath[MAX_PATH];
    GetModuleFileName(NULL,szCodecPath,MAX_PATH - 1);

	ExtractFilePath(szCodecPath);
	lstrcat(szCodecPath,_T("Codecs\\"));

	if (!g_CodecManager.LoadCodecs(szCodecPath))
	{
		if (g_GlobalSettings.m_bCodecWarning)
		{
			CInfoDlg InfoDlg(&g_GlobalSettings.m_bCodecWarning,lngGetString(ERROR_LOADCODECS),INFODLG_NOCANCEL | INFODLG_ICONWARNING);
			InfoDlg.DoModal();
		}
	}

	// Display the main window.
	INT_PTR nRet = ParseAndRun(lpstrCmdLine,nCmdShow);

	// Save the devices configuration.
	g_DeviceManager.SaveConfiguration();

	// Remove any temporary files.
	g_TempManager.CleanUp();

	// Destroy the log dialog.
	if (g_LogDlg.IsWindow())
		g_LogDlg.DestroyWindow();

	_Module.Term();
	::CoUninitialize();

	return (int)nRet;
}

void ProcessMessages()
{
	MSG Msg;
	while (::PeekMessage(&Msg,NULL,0,0,PM_REMOVE)) 
	{ 
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}
