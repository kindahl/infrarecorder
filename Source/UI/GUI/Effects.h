/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

// Configuration.
#define EFFECTWINDOW_HEIGHT				256
#define EFFECTWINDOW_OVERLAP			25

// Messages.
#define WM_CKEFFECTS_INITIALIZED			WM_APP + 0
#define WM_CKEFFECTS_DESTROY				WM_APP + 1
#define WM_CKEFFECTS_STARTSMOKE				WM_APP + 2
#define WM_CKEFFECTS_STOPSMOKE				WM_APP + 3
#define WM_CKEFFECTS_KILLSMOKE				WM_APP + 4

#define SMOKE_INIT\
	m_hWndEffects = NULL;

#define SMOKE_START\
	if (m_hWndEffects == NULL)\
	{\
		if (g_WinVer.m_ulMajorVersion == MAJOR_WINVISTA &&\
			g_WinVer.m_ulMinorVersion == MINOR_WINVISTA &&\
			g_GlobalSettings.m_bSmoke)\
		{\
			bool bHasDependencies = false;\
			HINSTANCE hDummy = LoadLibrary(_T("d3d8thk.dll"));\
			if (hDummy)\
			{\
				FreeLibrary(hDummy);\
				hDummy = LoadLibrary(_T("d3d9.dll"));\
				if (hDummy)\
				{\
					FreeLibrary(hDummy);\
					hDummy = LoadLibrary(_T("d3dx9_32.dll"));\
					if (hDummy)\
					{\
						FreeLibrary(hDummy);\
						hDummy = LoadLibrary(_T("dwmapi.dll"));\
						if (hDummy)\
						{\
							FreeLibrary(hDummy);\
							bHasDependencies = true;\
						}\
					}\
				}\
			}\
			if (bHasDependencies)\
			{\
				RECT rcWindow;\
				GetWindowRect(&rcWindow);\
				TCHAR szParam[128];\
				lsprintf(szParam,_T("-host=%I64x -dim=%d,%d,%d,%d"),(__int64)m_hWnd,rcWindow.left,\
					rcWindow.top - EFFECTWINDOW_HEIGHT + EFFECTWINDOW_OVERLAP,\
					rcWindow.right - rcWindow.left,EFFECTWINDOW_HEIGHT);\
				TCHAR szFileName[MAX_PATH];\
				GetModuleFileName(NULL,szFileName,MAX_PATH - 1);\
				ExtractFilePath(szFileName);\
				IncludeTrailingBackslash(szFileName);\
				lstrcat(szFileName,_T("ckEffects.exe"));\
				ShellExecute(HWND_DESKTOP,_T("open"),szFileName,szParam,NULL,SW_SHOWDEFAULT);\
			}\
		}\
	}

#define SMOKE_STOP\
	if (m_hWndEffects != NULL)\
	{\
		::PostMessage(m_hWndEffects,WM_CKEFFECTS_DESTROY,NULL,NULL);\
		m_hWndEffects = NULL;\
	}

#define SMOKE_EVENTS\
	MESSAGE_HANDLER(WM_WINDOWPOSCHANGED,OnWindowPosChanged)\
	MESSAGE_HANDLER(WM_SIZE,OnSize)\
	MESSAGE_HANDLER(WM_ACTIVATEAPP,OnActivateApp)\
	MESSAGE_HANDLER(WM_CKEFFECTS_INITIALIZED,OnEffectInitialized)

#define SMOKE_IMPL\
	LRESULT OnWindowPosChanged(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)\
	{\
		WINDOWPOS *pWindowPos = (WINDOWPOS *)lParam;\
		if (m_hWndEffects != NULL)\
		{\
			RECT rcWindow;\
			::GetWindowRect(m_hWndEffects,&rcWindow);\
			::MoveWindow(m_hWndEffects,pWindowPos->x,pWindowPos->y - EFFECTWINDOW_HEIGHT + EFFECTWINDOW_OVERLAP,rcWindow.right - rcWindow.left,\
				rcWindow.bottom - rcWindow.top,TRUE);\
		}\
		bHandled = false;\
		return TRUE;\
	}\
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)\
	{\
		switch (wParam)\
		{\
			case SIZE_MINIMIZED:\
				::PostMessage(m_hWndEffects,WM_CKEFFECTS_KILLSMOKE,NULL,NULL);\
				break;\
			case SIZE_RESTORED:\
				::PostMessage(m_hWndEffects,WM_CKEFFECTS_STARTSMOKE,NULL,NULL);\
				break;\
		}\
		bHandled = false;\
		return TRUE;\
	}\
	LRESULT OnActivateApp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)\
	{\
		if (m_hWndEffects)\
		{\
			::SetWindowPos(m_hWndEffects,m_hWnd,0,0,0,0,\
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_ASYNCWINDOWPOS);\
		}\
		bHandled = false;\
		return TRUE;\
	}\
	LRESULT OnEffectInitialized(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)\
	{\
		m_hWndEffects = (HWND)lParam;\
		::PostMessage(m_hWndEffects,WM_CKEFFECTS_STARTSMOKE,NULL,NULL);\
		return TRUE;\
	}\
	private:\
	HWND m_hWndEffects;\
	public:
