/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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

#include "stdafx.hh"
#include "resource.h"
#include "main_dlg.hh"

CAppModule _Module;

int Run(LPTSTR lpstrCmdLine = NULL,int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop MainLoop;
	_Module.AddMessageLoop(&MainLoop);

	CMainDlg MainDlg;

	if (MainDlg.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	MainDlg.ShowWindow(nCmdShow);

	int nRet = MainLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpstrCmdLine,int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// This resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used.
	::DefWindowProc(NULL,0,0,0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// Add flags to support other controls.

	hRes = _Module.Init(NULL,hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine,nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
