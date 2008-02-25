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
#include "ProgressDlg.h"
#include "../../Common/StringUtil.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Settings.h"
#include "InfraRecorder.h"

CProgressDlg g_ProgressDlg;

CProgressDlg::CProgressDlg()
{
	// Load the icons.
	m_hListImageList = ImageList_Create(16,16,ILC_COLOR32,0,4);

	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_INFORMATION));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_WARNING));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_ERROR));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_WINLOGO));

	m_pConsolePipe = NULL;
	m_bAppMode = false;
	m_bRealMode = false;
	m_bCanceled = false;

	m_hWndHost = NULL;

	m_iPercent = -1;

	SMOKE_INIT
}

CProgressDlg::~CProgressDlg()
{
	if (m_hListImageList != NULL)
		ImageList_Destroy(m_hListImageList);
}

bool CProgressDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a progress translation section.
	if (!pLNG->EnterSection(_T("progress")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLNG->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLNG->GetValuePtr(IDC_RELOADBUTTON,szStrValue))
		SetDlgItemText(IDC_RELOADBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_BUFFERSTATIC,szStrValue))
		SetDlgItemText(IDC_BUFFERSTATIC,szStrValue);

	return true;
}

void CProgressDlg::AttachConsolePipe(CConsolePipe *pConsolePipe)
{
	m_pConsolePipe = pConsolePipe;
}

void CProgressDlg::AttachHost(HWND hWndHost)
{
	m_hWndHost = hWndHost;
}

void CProgressDlg::AddLogEntry(eLogType Type,const TCHAR *szMessage,...)
{
	int iItemIndex = m_ListView.GetItemCount();

	// Time.
	SYSTEMTIME st;
	GetLocalTime(&st);

	TCHAR szTime[9];	// xx:yy:zz
	lsprintf(szTime,_T("%.2d:%.2d:%.2d"),st.wHour,st.wMinute,st.wSecond);
	szTime[8] = '\0';

	// Convert the log type to an index.
	int iImageIndex = 0;
	switch (Type)
	{
		case LT_INFORMATION:
			iImageIndex = 0;
			break;
		case LT_WARNING:
			iImageIndex = 1;
			break;
		case LT_ERROR:
			iImageIndex = 2;
			break;
		case LT_WINLOGO:
			iImageIndex = 3;
			break;
	}

	m_ListView.AddItem(iItemIndex,0,szTime,iImageIndex);

	// Parse the variable argument list.
	va_list args;
	va_start(args,szMessage);

#ifdef UNICODE
	_vsnwprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szMessage,args);
#else
	_vsnprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szMessage,args);
#endif
	m_ListView.AddItem(iItemIndex,1,m_szStringBuffer);

	m_ListView.SetColumnWidth(1,LVSCW_AUTOSIZE);
	m_ListView.EnsureVisible(iItemIndex,false);
}

void CProgressDlg::SetStatus(const TCHAR *szStatus,...)
{
	// Prepare the string.
	TCHAR szStatusStr[256];
	lstrcpy(szStatusStr,lngGetString(PROGRESS_STATUS));

	unsigned int uiFreeSpace = sizeof(szStatusStr)/sizeof(TCHAR) - lstrlen(szStatusStr) - 1;
	if ((unsigned int)lstrlen(szStatus) > uiFreeSpace)
		lstrncat(szStatusStr,szStatus,uiFreeSpace);
	else
		lstrcat(szStatusStr,szStatus);

	// Parse the variable argument list.
	va_list args;
	va_start(args,szStatus);

#ifdef UNICODE
	_vsnwprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szStatusStr,args);
#else
	_vsnprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szStatusStr,args);
#endif

	m_StatusStatic.SetWindowText(m_szStringBuffer);
}

void CProgressDlg::SetDevice(const TCHAR *szDevice)
{
	TCHAR szDeviceStr[128];
	lstrcpy(szDeviceStr,lngGetString(PROGRESS_DEVICE));

	unsigned int uiFreeSpace = sizeof(szDeviceStr)/sizeof(TCHAR) - lstrlen(szDeviceStr) - 1;
	if ((unsigned int)lstrlen(szDevice) > uiFreeSpace)
		lstrncat(szDeviceStr,szDevice,uiFreeSpace);
	else
		lstrcat(szDeviceStr,szDevice);

	SetDlgItemText(IDC_DEVICESTATIC,szDeviceStr);
}

void CProgressDlg::NotifyComplteted()
{
	::EnableWindow(GetDlgItem(IDOK),true);
	::EnableWindow(GetDlgItem(IDCANCEL),false);

	if (m_bCanceled)
	{
		SetStatus(lngGetString(PROGRESS_CANCELED));
		AddLogEntry(LT_WARNING,lngGetString(PROGRESS_CANCELED));
	}

	SMOKE_STOP
}

void CProgressDlg::SetAppMode(bool bAppMode)
{
	m_bAppMode = bAppMode;
}

void CProgressDlg::SetRealMode(bool bRealMode)
{
	m_bRealMode = bRealMode;
}

void CProgressDlg::SetProgress(int iPercent)
{
	if (iPercent < 0)
		iPercent = 0;
	else if (iPercent > 100)
		iPercent = 100;

	// Only redraw when we have to.
	if (m_iPercent != iPercent)
	{
		SendDlgItemMessage(IDC_TOTALPROGRESS,PBM_SETPOS,(WPARAM)iPercent,0);

		TCHAR szProgress[32];
		lsnprintf_s(szProgress,32,lngGetString(PROGRESS_TOTAL),iPercent);
		m_TotalStatic.SetWindowText(szProgress);

		ProcessMessages();
	}
}

void CProgressDlg::SetBuffer(int iPercent)
{
	SendDlgItemMessage(IDC_BUFFERPROGRESS,PBM_SETPOS,(WPARAM)iPercent,0);
}

void CProgressDlg::AllowReload()
{
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_SHOW);
}

void CProgressDlg::AllowCancel(bool bAllow)
{
	::EnableWindow(GetDlgItem(IDCANCEL),bAllow);
}

bool CProgressDlg::IsCanceled()
{
	return m_bCanceled;
}

void CProgressDlg::Reset()
{
	m_bRealMode = false;
	m_bCanceled = false;
}

void CProgressDlg::StartSmoke()
{
	SMOKE_START
}

LRESULT CProgressDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// If we're in application mode, add a minimize button to the window.
	if (m_bAppMode)
		ModifyStyle(0,WS_MINIMIZEBOX,0);

	SendDlgItemMessage(IDC_TOTALPROGRESS,PBM_SETRANGE32,0,100);
	SendDlgItemMessage(IDC_BUFFERPROGRESS,PBM_SETRANGE32,0,100);

	// Make the static controls double buffered.
	m_TotalStatic.SubclassWindow(GetDlgItem(IDC_TOTALSTATIC));
	m_StatusStatic.SubclassWindow(GetDlgItem(IDC_STATUSSTATIC));

	// Initialize the list view.
	m_ListView = GetDlgItem(IDC_MESSAGELIST);
	m_ListView.SetImageList(m_hListImageList,LVSIL_NORMAL);
	m_ListView.SetImageList(m_hListImageList,LVSIL_SMALL);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	m_ListView.AddColumn(lngGetString(COLUMN_TIME),0);
	m_ListView.SetColumnWidth(0,70);
	m_ListView.AddColumn(lngGetString(COLUMN_EVENT),1);
	m_ListView.SetColumnWidth(1,150);

	// Disable the OK button.
	::EnableWindow(GetDlgItem(IDOK),false);

	SetProgress(0);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CProgressDlg::OnReload(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Send a CR-LF message to inform CD-tools that the drive has been reloaded.
	if (m_pConsolePipe != NULL)
		m_pConsolePipe->WriteInput("\r\n",2);

	// Hide the reload button.
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_HIDE);

	return FALSE;
}

LRESULT CProgressDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Re-enable the main window.
	//g_MainFrame.EnableWindow(true);
	if (::IsWindow(m_hWndHost))
		::EnableWindow(m_hWndHost,true);

	DestroyWindow();

	// If we're in application mode we post a quit message.
	if (m_bAppMode)
		::PostQuitMessage(wID);

	return FALSE;
}

LRESULT CProgressDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Make sure that we're allowed to cancel.
	if (!::IsWindowEnabled(GetDlgItem(IDCANCEL)))
		return TRUE;

	// If we're in real mode we dispay a warning message informing the user that
	// aborting might permanently damage the CD.
	if (m_bRealMode)
	{
		if (lngMessageBox(m_hWnd,CONFIRM_WRITECANCEL,GENERAL_WARNING,MB_YESNO | MB_ICONWARNING) == IDNO)
			return TRUE;
	}

	m_bCanceled = true;

	if (m_pConsolePipe != NULL)
		m_pConsolePipe->Kill();

	// Hide the reload button.
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_HIDE);

	return TRUE;
}
