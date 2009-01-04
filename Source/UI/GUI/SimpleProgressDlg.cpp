/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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
#include "SimpleProgressDlg.h"
#include "../../Common/StringUtil.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Settings.h"

CSimpleProgressDlg g_SimpleProgressDlg;

CSimpleProgressDlg::CSimpleProgressDlg() : m_pProcess(NULL),m_bAppMode(false),
	m_bRealMode(false),m_bCancelled(false),m_hWndHost(NULL)
{
	// Load the icons.
	m_hListImageList = ImageList_Create(16,16,ILC_COLOR32,0,4);

	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_INFORMATION));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_WARNING));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_ERROR));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_WINLOGO));

	SMOKE_INIT
}

CSimpleProgressDlg::~CSimpleProgressDlg()
{
	if (m_hListImageList != NULL)
		ImageList_Destroy(m_hListImageList);
}

BOOL CSimpleProgressDlg::PreTranslateMessage(MSG *pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CSimpleProgressDlg::OnIdle()
{
	return FALSE;
}

bool CSimpleProgressDlg::Translate()
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

	return true;
}

void CSimpleProgressDlg::AttachProcess(ckcore::Process *pProcess)
{
	m_pProcess = pProcess;
}

void CSimpleProgressDlg::AttachHost(HWND hWndHost)
{
	m_hWndHost = hWndHost;
}

void CSimpleProgressDlg::SetAppMode(bool bAppMode)
{
	m_bAppMode = bAppMode;
}

void CSimpleProgressDlg::SetRealMode(bool bRealMode)
{
	m_bRealMode = bRealMode;
}

void CSimpleProgressDlg::set_status(const TCHAR *szStatus,...)
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

	SetDlgItemText(IDC_STATUSSTATIC,m_szStringBuffer);
}

void CSimpleProgressDlg::notify(ckcore::Progress::MessageType Type,const TCHAR *szMessage,...)
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
		case ckcore::Progress::ckINFORMATION:
			iImageIndex = 0;
			break;
		case ckcore::Progress::ckWARNING:
			iImageIndex = 1;
			break;
		case ckcore::Progress::ckERROR:
			iImageIndex = 2;
			break;
		case ckcore::Progress::ckEXTERNAL:
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

bool CSimpleProgressDlg::cancelled()
{
	return m_bCancelled;
}

void CSimpleProgressDlg::SetDevice(const TCHAR *szDevice)
{
	TCHAR szDeviceStr[128];
	lstrcpy(szDeviceStr,lngGetString(PROGRESS_DEVICE));
	lstrcat(szDeviceStr,szDevice);

	SetDlgItemText(IDC_DEVICESTATIC,szDeviceStr);
}

void CSimpleProgressDlg::NotifyCompleted()
{
	::EnableWindow(GetDlgItem(IDOK),true);
	SendMessage(WM_NEXTDLGCTL,(WPARAM)GetDlgItem(IDOK).m_hWnd,0);	// Change default focus to the OK button.
	::EnableWindow(GetDlgItem(IDCANCEL),false);

	if (m_bCancelled)
	{
		set_status(lngGetString(PROGRESS_CANCELED));
		notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
	}

	SMOKE_STOP
}

void CSimpleProgressDlg::AllowReload()
{
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_SHOW);
}

void CSimpleProgressDlg::AllowCancel(bool bAllow)
{
	::EnableWindow(GetDlgItem(IDCANCEL),bAllow);
}

void CSimpleProgressDlg::Reset()
{
	m_bRealMode = false;
	m_bCancelled = false;
}

bool CSimpleProgressDlg::RequestNextDisc()
{
	return lngMessageBox(m_hWnd,INFO_NEXTCOPY,GENERAL_INFORMATION,MB_OKCANCEL | MB_ICONINFORMATION) == IDOK;
}

void CSimpleProgressDlg::StartSmoke()
{
	SMOKE_START
}

LRESULT CSimpleProgressDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// If we're in application mode, add a minimize button to the window.
	// UPDATE: This does not work, nor is it a good idea regarding the smoke effect.
	/*if (m_bAppMode)
		ModifyStyle(0,WS_MINIMIZEBOX,0);*/

	// Initialize the list view.
	m_ListView = GetDlgItem(IDC_MESSAGELIST);
	m_ListView.SetImageList(m_hListImageList,LVSIL_NORMAL);
	m_ListView.SetImageList(m_hListImageList,LVSIL_SMALL);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	m_ListView.AddColumn(lngGetString(COLUMN_TIME),0);
	m_ListView.SetColumnWidth(0,70);
	m_ListView.AddColumn(lngGetString(COLUMN_EVENT),1);
	m_ListView.SetColumnWidth(1,350);

	// Disable the OK button.
	::EnableWindow(GetDlgItem(IDOK),false);
	::EnableWindow(GetDlgItem(IDCANCEL),true);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CSimpleProgressDlg::OnReload(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Send a CR-LF message to inform CD-tools that the drive has been reloaded.
	if (m_pProcess != NULL)
		m_pProcess->write("\r\n",2);

	// Hide the reload button.
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_HIDE);

	return FALSE;
}

LRESULT CSimpleProgressDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
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

LRESULT CSimpleProgressDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
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

	m_bCancelled = true;

	if (m_pProcess != NULL)
		m_pProcess->kill();

	// Hide the reload button.
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_HIDE);

	return TRUE;
}
