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

#include "stdafx.h"
#include "LogDlg.h"
#include "../../Common/FileManager.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Settings.h"
#include "Diagnostics.h"
#include "WinVer.h"

CLogDlg g_LogDlg;

// FIXME: No arrow is used, not much space.
CLogDlg::CLogDlg() : m_DiagButton(IDR_DIAGNOSTICSMENU,false)
{
	m_hLogFile = NULL;
}

CLogDlg::~CLogDlg()
{
	if (m_hLogFile != NULL)
		fs_close(m_hLogFile);
}

void CLogDlg::InitializeLogFile()
{
	// Get system date.
	SYSTEMTIME st;
	GetLocalTime(&st);

	TCHAR szDate[7];
	GetDateFormat(LOCALE_USER_DEFAULT,0,&st,_T("yyMMdd"),szDate,7);

	TCHAR szFileName[MAX_PATH];
#ifdef UNICODE
	if (SUCCEEDED(SHGetFolderPath(m_hWnd,CSIDL_APPDATA | CSIDL_FLAG_CREATE,NULL,
		SHGFP_TYPE_CURRENT,szFileName)))
#else	// Win 9x.
	if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd,szFileName,CSIDL_APPDATA,true)))
#endif
	{
		IncludeTrailingBackslash(szFileName);
		lstrcat(szFileName,_T("InfraRecorder\\Logs\\"));

		// Create the file path if it doesn't exist.
		fs_createpath(szFileName);

		lstrcat(szFileName,szDate);

#ifdef UNICODE
		lstrcat(szFileName,_T("_u"));
#else
		lstrcat(szFileName,_T("_a"));
#endif
		lstrcat(szFileName,_T(".log"));

		if (fs_fileexists(szFileName))
		{
			m_hLogFile = fs_open(szFileName,_T("a"));
			if (m_hLogFile != NULL)
			{
				fs_seek(m_hLogFile,0,FILE_END);
				fs_write(_T("\r\n"),sizeof(TCHAR) << 1,m_hLogFile);
			}
		}
		else
		{
			m_hLogFile = fs_open(szFileName,_T("w"));

#ifdef UNICODE
			// Write byte order mark.
			if (m_hLogFile != NULL)
			{
				unsigned short usBOM = BOM_UTF32BE;
				fs_write(&usBOM,2,m_hLogFile);
			}
#endif
		}
	}
}

bool CLogDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a log translation section.
	if (!pLNG->EnterSection(_T("log")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDD_LOGDLG,szStrValue))			// Title.
		SetWindowText(szStrValue);
	if (pLNG->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLNG->GetValuePtr(ID_SAVEASBUTTON,szStrValue))
		SetDlgItemText(ID_SAVEASBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_FILESBUTTON,szStrValue))
		SetDlgItemText(IDC_FILESBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_DIAGNOSTICSBUTTON,szStrValue))
		SetDlgItemText(IDC_DIAGNOSTICSBUTTON,szStrValue);

	// Modify the diagnostics popup menu.
	if (pLNG->GetValuePtr(ID_DIAGNOSTICS_DEVICESCAN,szStrValue))
		ModifyMenu(m_DiagButton.GetMenu(),ID_DIAGNOSTICS_DEVICESCAN,MF_BYCOMMAND | MF_STRING,
			ID_DIAGNOSTICS_DEVICESCAN,(LPCTSTR)szStrValue);

	return true;
}

/*
	CLogDlg::Show
	-------------
	Should be called when the log should be displayed. This function automaticly
	scrolls to the bottom of the log.
*/
void CLogDlg::Show()
{
	g_LogDlg.ShowWindow(SW_SHOW);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount());
}

void CLogDlg::AddString(const TCHAR *szString,...)
{
	if (g_GlobalSettings.m_bLog && IsWindow())
	{
		va_list args;
		va_start(args,szString);

#ifdef UNICODE
		_vsnwprintf(m_szLineBuffer,LOG_LINEBUFFER_SIZE - 1,szString,args);
#else
		_vsnprintf(m_szLineBuffer,LOG_LINEBUFFER_SIZE - 1,szString,args);
#endif

		m_LogEdit.AppendText(m_szLineBuffer);

		// Write to the log file.
		if (m_hLogFile != NULL)
			fs_write(m_szLineBuffer,lstrlen(m_szLineBuffer) * sizeof(TCHAR),m_hLogFile);
	}
}

void CLogDlg::AddLine(const TCHAR *szLine,...)
{
	if (g_GlobalSettings.m_bLog && IsWindow())
	{
		va_list args;
		va_start(args,szLine);

#ifdef UNICODE
		_vsnwprintf(m_szLineBuffer,LOG_LINEBUFFER_SIZE - 1,szLine,args);
#else
		_vsnprintf(m_szLineBuffer,LOG_LINEBUFFER_SIZE - 1,szLine,args);
#endif

		m_LogEdit.AppendText(m_szLineBuffer);
		m_LogEdit.AppendText(_T("\r\n"));

		// Write to the log file.
		if (m_hLogFile != NULL)
		{
			fs_write(m_szLineBuffer,lstrlen(m_szLineBuffer) * sizeof(TCHAR),m_hLogFile);
			fs_write(_T("\r\n"),2 * sizeof(TCHAR),m_hLogFile);
		}
	}
}

bool CLogDlg::SaveLog(const TCHAR *szFileName)
{
	HANDLE hFile = fs_open(szFileName,_T("w"));
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// Obtain buffer handle.
	HLOCAL hBuffer = m_LogEdit.GetHandle();
	if (hBuffer == NULL)
		return false;

	// A special unicode hack is needed here since the edit might be in unicode
	// while the application is not.
	unsigned char *pBuffer = (unsigned char *)LocalLock(hBuffer);
	unsigned int uiTextLength = m_LogEdit.GetWindowTextLength();

	bool bIsUnicode = ::IsWindowUnicode(m_LogEdit) == TRUE;

#ifdef LOG_SAVE_BOM
	if (bIsUnicode)
	{
		unsigned short usBOM = BOM_UTF32BE;
		fs_write(&usBOM,2,hFile);
	}
#endif

	// Do the actual writing.
	unsigned int uiRemaining = uiTextLength * (bIsUnicode ? 2 : 1);

	while (uiRemaining > LOG_WRITEBUFFER_SIZE)
	{
		fs_write(pBuffer,LOG_WRITEBUFFER_SIZE,hFile);
		uiRemaining -= LOG_WRITEBUFFER_SIZE;
		pBuffer += LOG_WRITEBUFFER_SIZE;
	}

	fs_write(pBuffer,uiRemaining,hFile);

	LocalUnlock(hBuffer);

	fs_close(hFile);
	return true;
}

LRESULT CLogDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	DlgResize_Init(true,true,WS_CLIPCHILDREN);

	m_LogEdit = GetDlgItem(IDC_LOGEDIT);
	m_DiagButton.SubclassWindow(GetDlgItem(IDC_DIAGNOSTICSBUTTON));

	// Initialize the log.
	if (g_GlobalSettings.m_bLog)
	{
		// Initialize the log file.
		InitializeLogFile();

		// Print version information.
		TCHAR szFileName[MAX_PATH];
		GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

		unsigned long ulDummy;
		unsigned long ulDataSize = GetFileVersionInfoSize(szFileName,&ulDummy);

		if (ulDataSize != 0)
		{
			unsigned char *pBlock = new unsigned char[ulDataSize];

			struct LANGANDCODEPAGE
			{
				unsigned short usLanguage;
				unsigned short usCodePage;
			} *pTranslate;

			if (GetFileVersionInfo(szFileName,NULL,ulDataSize,pBlock) > 0)
			{
				// Get language information (only one language should be present).
				VerQueryValue(pBlock,_T("\\VarFileInfo\\Translation"),
					(LPVOID *)&pTranslate,(unsigned int *)&ulDummy);

				// Calculate the FileVersion sub block path.
				TCHAR szStrBuffer[128];
				lsprintf(szStrBuffer,_T("\\StringFileInfo\\%04x%04x\\FileVersion"),
					pTranslate[0].usLanguage,pTranslate[0].usCodePage);

				unsigned char *pBuffer;
				VerQueryValue(pBlock,szStrBuffer,(LPVOID *)&pBuffer,(unsigned int *)&ulDataSize);

				AddString(_T("InfraRecorder version %s"),(TCHAR *)pBuffer);
			}

			delete [] pBlock;
		}

#ifdef _M_IA64
		AddLine(_T(" (IA64)"));
#elif defined _M_X64
		AddLine(_T(" (x64)"));
#else
		AddLine(_T(" (x86)"));
#endif

		// Date and time.
		SYSTEMTIME st;
		GetLocalTime(&st);
		TCHAR szDate[64];
		GetDateFormat(LOCALE_USER_DEFAULT,0,&st,_T("dddd, MMMM dd yyyy "),szDate,64);
		TCHAR szDateTime[128];
		lsprintf(szDateTime,_T("Started: %s%.2d:%.2d:%.2d."),szDate,st.wHour,st.wMinute,st.wSecond);
		AddLine(szDateTime);

		// Windows version.
		AddLine(_T("Versions: MSW = %d.%d, IE = %d.%d, CC = %d.%d."),
			g_WinVer.m_ulMajorVersion,g_WinVer.m_ulMinorVersion,
			g_WinVer.m_ulMajorIEVersion,g_WinVer.m_ulMinorIEVersion,
			g_WinVer.m_ulMajorCCVersion,g_WinVer.m_ulMinorCCVersion);
	}

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CLogDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	ShowWindow(SW_HIDE);
	return FALSE;
}

LRESULT CLogDlg::OnSaveAs(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	WTL::CFileDialog FileDialog(false,_T("txt"),_T("Untitled"),OFN_EXPLORER | OFN_HIDEREADONLY,
		_T("Text Documents (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
	{
		if (!SaveLog(FileDialog.m_szFileName))
			lngMessageBox(m_hWnd,ERROR_FILEWRITE,GENERAL_ERROR,MB_OK | MB_ICONERROR);
	}

	return FALSE;
}

LRESULT CLogDlg::OnFiles(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	TCHAR szPathName[MAX_PATH];
#ifdef UNICODE
	if (SUCCEEDED(SHGetFolderPath(m_hWnd,CSIDL_APPDATA | CSIDL_FLAG_CREATE,NULL,
		SHGFP_TYPE_CURRENT,szPathName)))
#else
	if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd,szPathName,CSIDL_APPDATA,true)))
#endif
	{
		IncludeTrailingBackslash(szPathName);
		lstrcat(szPathName,_T("InfraRecorder\\Logs\\"));
		ShellExecute(HWND_DESKTOP,_T("open"),_T("explorer.exe"),szPathName,NULL,SW_SHOWDEFAULT);
	}

	return 0;
}

LRESULT CLogDlg::OnDiagDeviceScan(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_Diagnostics.DeviceScan();
	return 0;
}
