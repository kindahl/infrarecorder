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
#include <ckcore/directory.hh>
#include "LogDlg.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Settings.h"
#include "Diagnostics.h"
#include "WinVer.h"

CLogDlg g_LogDlg;

// FIXME: No arrow is used, not enough space.
CLogDlg::CLogDlg() : m_DiagButton(IDR_DIAGNOSTICSMENU,false),
	m_LogFile(GetLogFullPath())
{
}

CLogDlg::~CLogDlg()
{
}

ckcore::Path CLogDlg::GetLogFullPath()
{
	// Get system date.
	SYSTEMTIME st;
	GetLocalTime(&st);

	TCHAR szFileName[13];
	GetDateFormat(LOCALE_USER_DEFAULT,0,&st,_T("yyMMdd"),szFileName,7);

	// Get the log dir path.
	ckcore::Path DirPath = GetLogDirPath();
	
	// Create the file path if it doesn't exist.
	ckcore::Directory::create(DirPath);

	// Construct the file name.
#ifdef UNICODE
	lstrcat(szFileName,_T("_u"));
#else
	lstrcat(szFileName,_T("_a"));
#endif
	lstrcat(szFileName,_T(".log"));

	// Append the file name and return.
	return DirPath + szFileName;
}

ckcore::Path CLogDlg::GetLogDirPath()
{
	TCHAR szDirPath[MAX_PATH];

#ifdef PORTABLE
	GetModuleFileName(NULL,szDirPath,MAX_PATH - 1);
	ExtractFilePath(szDirPath);

	lstrcat(szDirPath,_T("Logs\\"));
#else
#ifdef UNICODE
	if (SUCCEEDED(SHGetFolderPath(m_hWnd,CSIDL_APPDATA | CSIDL_FLAG_CREATE,NULL,
		SHGFP_TYPE_CURRENT,szDirPath)))
#else	// Win 9x.
	if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd,szDirPath,CSIDL_APPDATA,true)))
#endif
	{
		IncludeTrailingBackslash(szDirPath);
		lstrcat(szDirPath,_T("InfraRecorder\\Logs\\"));
	}
	else
	{
		GetModuleFileName(NULL,szDirPath,MAX_PATH - 1);
		ExtractFilePath(szDirPath);
	}
#endif

	return ckcore::Path(szDirPath);
}

void CLogDlg::InitializeLogFile()
{
	if (m_LogFile.exist())
	{
		if (m_LogFile.open(ckcore::FileBase::ckOPEN_READWRITE))
		{
			m_LogFile.seek(0,ckcore::FileBase::ckFILE_END);
			m_LogFile.write(ckT("\r\n"),sizeof(ckcore::tchar) << 1);
		}
	}
	else
	{
		if (m_LogFile.open(ckcore::FileBase::ckOPEN_WRITE))
		{
#ifdef UNICODE
			// Write byte order mark.
			unsigned short usBOM = BOM_UTF32BE;
			m_LogFile.write(&usBOM,2);
		}
#endif
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

void CLogDlg::print(const TCHAR *szString,...)
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
		if (m_LogFile.test())
			m_LogFile.write(m_szLineBuffer,lstrlen(m_szLineBuffer) * sizeof(TCHAR));
	}
}

void CLogDlg::print_line(const TCHAR *szLine,...)
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
		if (m_LogFile.test())
		{
			m_LogFile.write(m_szLineBuffer,lstrlen(m_szLineBuffer) * sizeof(TCHAR));
			m_LogFile.write(ckT("\r\n"),2 * sizeof(ckcore::tchar));
		}
	}
}

bool CLogDlg::SaveLog(const TCHAR *szFileName)
{
	ckcore::File File(szFileName);
	if (!File.open(ckcore::FileBase::ckOPEN_WRITE))
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
		if (File.write(&usBOM,2) == -1)
		{
			File.remove();
			return false;
		}
	}
#endif

	// Do the actual writing.
	unsigned int uiRemaining = uiTextLength * (bIsUnicode ? 2 : 1);

	while (uiRemaining > LOG_WRITEBUFFER_SIZE)
	{
		if (File.write(pBuffer,LOG_WRITEBUFFER_SIZE) == -1)
		{
			File.remove();
			return false;
		}

		uiRemaining -= LOG_WRITEBUFFER_SIZE;
		pBuffer += LOG_WRITEBUFFER_SIZE;
	}

	if (File.write(pBuffer,uiRemaining) == -1)
	{
		File.remove();
		return false;
	}

	LocalUnlock(hBuffer);
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

				print(_T("InfraRecorder version %s"),(TCHAR *)pBuffer);
			}

			delete [] pBlock;
		}

#ifdef _M_IA64
		print_line(_T(" (IA64)"));
#elif defined _M_X64
		print_line(_T(" (x64)"));
#else
		print_line(_T(" (x86)"));
#endif

		// Date and time.
		SYSTEMTIME st;
		GetLocalTime(&st);
		TCHAR szDate[64];
		GetDateFormat(LOCALE_USER_DEFAULT,0,&st,_T("dddd, MMMM dd yyyy "),szDate,64);
		TCHAR szDateTime[128];
		lsprintf(szDateTime,_T("Started: %s%.2d:%.2d:%.2d."),szDate,st.wHour,st.wMinute,st.wSecond);
		print_line(szDateTime);

		// Windows version.
		print_line(_T("Versions: MSW = %d.%d, IE = %d.%d, CC = %d.%d."),
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
	ckcore::Path Path = GetLogDirPath();

	ShellExecute(HWND_DESKTOP,_T("open"),_T("explorer.exe"),Path.name().c_str(),NULL,SW_SHOWDEFAULT);
	return 0;
}

LRESULT CLogDlg::OnDiagDeviceScan(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_Diagnostics.DeviceScan();
	return 0;
}
