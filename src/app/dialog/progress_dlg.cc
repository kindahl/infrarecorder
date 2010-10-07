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
#include <comutil.h>
#include <base/StringUtil.h>
#include "string_table.hh"
#include "lang_util.hh"
#include "settings.hh"
#include "infra_recorder.hh"
#include "device_util.hh"
#include "version.hh"
#include "progress_dlg.hh"

static const int SUBITEM_TEXT = 1;

CProgressDlg::CProgressDlg() : m_pProcess(NULL),m_bAppMode(false),
	m_bRealMode(false),m_bCancelled(false),m_hWndHost(NULL),m_ucPercent(0),
	m_szHostTitle(NULL),m_pFlashWindowEx(NULL),m_bNeedToFocusOkButton(false)
{
	// Load the icons.
	m_hListImageList = ImageList_Create(16,16,ILC_COLOR32,0,4);

	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_INFORMATION));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_WARNING));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_ERROR));
	ImageList_AddIcon(m_hListImageList,LoadIcon(NULL,IDI_WINLOGO));


	// The MSDN documentation states that FlashWindowEx() is only
	// available in Windows 2000 and later. However, MS KB article ID 254339
	// states that you can use this API in Windows 98 too.
	const HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
	if (hUser32 == NULL)
	{
		ATLASSERT(false);  // This should never happen at this point.
	}
	else
	{
		m_pFlashWindowEx = (PointerToFlashWindowEx)GetProcAddress(hUser32,"FlashWindowEx");

		// This should never happen at this point.
		ATLASSERT(m_pFlashWindowEx != NULL);
	}

	SMOKE_INIT
}

CProgressDlg::~CProgressDlg()
{
	if (m_hListImageList != NULL)
		ImageList_Destroy(m_hListImageList);

	if (m_szHostTitle != NULL)
	{
		delete m_szHostTitle;
		m_szHostTitle = NULL;
	}
}

bool CProgressDlg::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a progress translation section.
	if (!pLng->EnterSection(_T("progress")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLng->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLng->GetValuePtr(IDC_RELOADBUTTON,szStrValue))
		SetDlgItemText(IDC_RELOADBUTTON,szStrValue);
	if (pLng->GetValuePtr(IDC_BUFFERSTATIC,szStrValue))
		SetDlgItemText(IDC_BUFFERSTATIC,szStrValue);

	return true;
}

void CProgressDlg::AttachProcess(ckcore::Process *pProcess)
{
	m_pProcess = pProcess;
}

void CProgressDlg::AttachHost(HWND hWndHost)
{
	m_hWndHost = hWndHost;
}

void CProgressDlg::SetAppMode(bool bAppMode)
{
	m_bAppMode = bAppMode;
}

void CProgressDlg::SetRealMode(bool bRealMode)
{
	m_bRealMode = bRealMode;
}

void CProgressDlg::set_progress(unsigned char ucPercent)
{
	if (ucPercent < 0)
		ucPercent = 0;
	else if (ucPercent > 100)
		ucPercent = 100;

	// Make sure that the progress does not go in the wrong direction.
	if (ucPercent < m_ucPercent && ucPercent != 0)
		return;

	// Only redraw when we have to.
	if (m_ucPercent != ucPercent)
	{
		m_ucPercent = ucPercent;

		SendDlgItemMessage(IDC_TOTALPROGRESS,PBM_SETPOS,(WPARAM)ucPercent,0);

		TCHAR szProgress[32];
		lsnprintf_s(szProgress,32,lngGetString(PROGRESS_TOTAL),ucPercent);
		m_TotalStatic.SetWindowText(szProgress);

		// Update parent title if necessary.
		if (!m_bAppMode && m_szHostTitle != NULL)
		{
			TCHAR szTitle[64];
			GetWindowText(szTitle,sizeof(szTitle)/sizeof(TCHAR));

			TCHAR szHostTitle[32 + 64];
			lsnprintf_s(szHostTitle,sizeof(szHostTitle)/sizeof(TCHAR),_T("%d%% - %s"),ucPercent,szTitle);

			GetParentWindow(this).SetWindowText(szHostTitle);
		}

		ProcessMessages();
	}
}

void CProgressDlg::set_marquee(bool bEnable)
{
	// Only supported in Windows XP and newer (common controls version 6).
	if (g_WinVer.m_ulMajorCCVersion >= 6)
	{
		HWND hWndCtrl = GetDlgItem(IDC_TOTALPROGRESS);

		if (bEnable)
		{
			unsigned long ulStyle = ::GetWindowLong(hWndCtrl,GWL_STYLE);
			::SetWindowLong(hWndCtrl,GWL_STYLE,ulStyle | PBS_MARQUEE);
			::SendMessage(hWndCtrl,PBM_SETMARQUEE,TRUE,50);
		}
		else
		{
			unsigned long ulStyle = ::GetWindowLong(hWndCtrl,GWL_STYLE);
			::SetWindowLong(hWndCtrl,GWL_STYLE,ulStyle & ~PBS_MARQUEE);
			::SendMessage(hWndCtrl,PBM_SETMARQUEE,FALSE,50);
		}
	}
}

void CProgressDlg::set_status(const TCHAR *szStatus,...)
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

void CProgressDlg::notify(ckcore::Progress::MessageType Type,const TCHAR *szMessage,...)
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
	m_ListView.AddItem(iItemIndex,SUBITEM_TEXT,m_szStringBuffer);

	m_ListView.SetColumnWidth(SUBITEM_TEXT,LVSCW_AUTOSIZE);
	m_ListView.EnsureVisible(iItemIndex,false);
}

bool CProgressDlg::cancelled()
{
	return m_bCancelled;
}

void CProgressDlg::SetDevice(const TCHAR *szDevice)
{
	ckcore::tstring DeviceStr = lngGetString(PROGRESS_DEVICE);
	DeviceStr += szDevice;

	SetDlgItemText(IDC_DEVICESTATIC,DeviceStr.c_str());
}

void CProgressDlg::SetDevice(ckmmc::Device &Device)
{
	ckcore::tstring DeviceStr = lngGetString(PROGRESS_DEVICE);
	DeviceStr += NDeviceUtil::GetDeviceName(Device);

	SetDlgItemText(IDC_DEVICESTATIC,DeviceStr.c_str());
}

void CProgressDlg::NotifyCompleted()
{
	::EnableWindow(GetDlgItem(IDOK),true);
	if ( GetForegroundWindow() == m_hWnd )
	{
		// Change default focus to the OK button. We can only do this
		// if this window is active. Otherwise, this call will not
		// make it active, but will still trigger a WM_ACTIVATE message
		// to this window.
		// Later on, when this window does get active, it will not
		// get the corresponding WM_ACTIVATE and we won't be able
		// to stop the flashing if necessary, see OnActivate() below.
		SendMessage(WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDOK),0);
	}
	else
		m_bNeedToFocusOkButton = true;

	::EnableWindow(GetDlgItem(IDCANCEL),false);

	if (m_bCancelled)
	{
		set_status(lngGetString(PROGRESS_CANCELED));
		notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
	}

	// Restore the window title.
	if (!m_bAppMode && m_szHostTitle != NULL)
	{
		GetParentWindow(this).SetWindowText(m_szHostTitle);

		delete m_szHostTitle;
		m_szHostTitle = NULL;
	}

	SMOKE_STOP

	// Recording a CD can take some time, and the user may switch to
	// another application meanwhile and not notice when the CD is finished.
	// Therefore, at the end of the process, we prompt for
	// the user's attention by making the window and taskbar icon flash.
	//
	// Calling SetForegroundWindow() or SetActiveWindow()
	// should have an equivalent effect according to the documentation,
	// but it does not seem to do the trick, we need to call
	// FlashWindow().
	// We don't want to start flashing if this window is the active one
	// at the moment. We also need to stop the flashing if the window
	// wasn't active and the user reacts to the flashing and clicks on it.
	//
	// Rather than flashing, it would be better to display a balloon notification
	// on the task bar, but that's only available from Windows XP onwards.

	if ( m_pFlashWindowEx != NULL && GetForegroundWindow() != m_hWnd )
	{
		FLASHWINFO fi1;
		ZeroMemory(&fi1, sizeof(fi1));

		fi1.cbSize = sizeof(fi1);
		fi1.hwnd = m_hWnd;
		fi1.dwFlags = 1; // FLASHW_CAPTION. The taskbar icon belongs to the main window.
		fi1.uCount = 5;
		fi1.dwTimeout = 0;

		m_pFlashWindowEx( &fi1 );

		if ( !m_bAppMode )
		{
			FLASHWINFO fi2;
			ZeroMemory(&fi2, sizeof(fi2));

			fi2.cbSize = sizeof(fi2);
			fi2.hwnd = GetParentWindow(this).m_hWnd;
			fi2.dwFlags = 2; // FLASHW_TRAY
			fi2.uCount = 5;
			fi2.dwTimeout = 0;

			m_pFlashWindowEx( &fi2 );
		}
	}
}

LRESULT CProgressDlg::OnActivate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	const UINT nState = LOWORD(wParam);

	// If this window is getting activated...
	if (nState != WA_INACTIVE)
	{
		// Stop the window flashing. The window may never have flashed,
		// or may have flashed but may have stopped flashing after the given
		// flash count, but that doesn't matter.
		if (m_pFlashWindowEx != NULL)
		{
			FLASHWINFO fi1;
			ZeroMemory(&fi1,sizeof(fi1));

			fi1.cbSize = sizeof(fi1);
			fi1.hwnd = m_hWnd;
			fi1.dwFlags = 0; // FLASHW_STOP
			fi1.uCount = 0;
			fi1.dwTimeout = 0;

			m_pFlashWindowEx(&fi1);

			if (!m_bAppMode)
			{
				FLASHWINFO fi2;
				ZeroMemory(&fi2,sizeof(fi2));

				fi2.cbSize = sizeof(fi2);
				fi2.hwnd = GetParentWindow(this).m_hWnd;
				fi2.dwFlags = 0; // FLASHW_STOP
				fi2.uCount = 0;
				fi2.dwTimeout = 0;

				m_pFlashWindowEx(&fi2);
			}
		}

		if (m_bNeedToFocusOkButton)
		{
			SendMessage(WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDOK),0);	// Change default focus to the OK button.
			m_bNeedToFocusOkButton = false;
		}
	}

	return 0;
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

void CProgressDlg::Reset()
{
	m_bRealMode = false;
	m_bCancelled = false;
}

bool CProgressDlg::RequestNextDisc()
{
	return lngMessageBox(m_hWnd,INFO_NEXTCOPY,GENERAL_INFORMATION,MB_OKCANCEL | MB_ICONINFORMATION) == IDOK;
}

void CProgressDlg::StartSmoke()
{
	SMOKE_START
}

LRESULT CProgressDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	DlgResize_Init();

	CenterWindow(GetParent());

	// If we're in application mode, add a minimize button to the window.
	// UPDATE: This does not work, nor is it a good idea regarding the smoke effect.
	/*if (m_bAppMode)
		ModifyStyle(0,WS_MINIMIZEBOX,0);*/

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

	set_progress(0);

	// Translate the window.
	Translate();

	if (!m_bAppMode)
	{
		if (m_szHostTitle != NULL)
			delete [] m_szHostTitle;

		int iSize = GetParentWindow(this).GetWindowTextLength() + 1;
		m_szHostTitle = new TCHAR[iSize];
		GetParentWindow(this).GetWindowText(m_szHostTitle,iSize);
	}

	return TRUE;
}

LRESULT CProgressDlg::OnReload(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Send a CR-LF message to inform CD-tools that the drive has been reloaded.
	if (m_pProcess != NULL)
		m_pProcess->write("\r\n",2);

	// Hide the reload button.
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_HIDE);

	return FALSE;
}

LRESULT CProgressDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Re-enable the main window.
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

	m_bCancelled = true;

	if (m_pProcess != NULL)
		m_pProcess->kill();

	// Hide the reload button.
	::ShowWindow(GetDlgItem(IDC_RELOADBUTTON),SW_HIDE);

	return TRUE;
}

LRESULT CProgressDlg::OnListViewDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	ATLASSERT(iCtrlID == IDC_MESSAGELIST);
	LPNMITEMACTIVATE pItemActivate = (LPNMITEMACTIVATE)pNMH;

	_bstr_t LineText;
	m_ListView.GetItemText(pItemActivate->iItem,SUBITEM_TEXT,LineText.GetBSTR());

	MessageBox(LineText,_T("Log line"),MB_OK | MB_ICONINFORMATION);

	bHandled = TRUE;
	return 0;
}