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

#include "stdafx.h"
#include "info_dlg.hh"
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"

CInfoDlg::CInfoDlg(bool *pRemember,const TCHAR *szMessage,int iFlags)
{
	m_pRemember = pRemember;
	m_iFlags = iFlags;
	n_szMessage = szMessage;
}

CInfoDlg::~CInfoDlg()
{
}

bool CInfoDlg::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;

	TCHAR *szStrValue;

	// Translate the title.
	if (!pLng->EnterSection(_T("strings")))
		return false;
	
	// Translate the rest.
	if (!pLng->EnterSection(_T("info")))
		return false;

	if (pLng->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLng->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLng->GetValuePtr(IDC_DISPLAYMSGCHECK,szStrValue))
		SetDlgItemText(IDC_DISPLAYMSGCHECK,szStrValue);

	return true;
}

/*
	CInfoDlg::InitializeMessage
	---------------------------
	Sets the text of the static control to szMessage. Resizes the static to fit
	all of the text, and if necessary also moves the check box and resizes the
	main window.
*/
void CInfoDlg::InitializeMessage(const TCHAR *szMessage)
{
	// Set the message text.
	SetDlgItemText(IDC_INFOSTATIC,szMessage);

	// Get handles.
	HWND hMessageCtrl = GetDlgItem(IDC_INFOSTATIC);
	HWND hCheckCtrl = GetDlgItem(IDC_DISPLAYMSGCHECK);
	HDC hMessageDC = ::GetDC(hMessageCtrl);

	// Perpare font.
	HFONT hMessageFont = (HFONT)::SendMessage(hMessageCtrl,WM_GETFONT,0,0);
	HFONT hOldFont = (HFONT)::SelectObject(hMessageDC,hMessageFont);

	// Calculate and set the message size.
	RECT rcMessage;
	::GetWindowRect(GetDlgItem(IDC_INFOSTATIC),&rcMessage);
	ScreenToClient(&rcMessage);

	RECT rcText;
	rcText.left = 0;
	rcText.right = rcMessage.right - rcMessage.left;
	int iMessageHeight = DrawText(hMessageDC,szMessage,lstrlen(szMessage),&rcText,DT_LEFT | DT_CALCRECT | DT_WORDBREAK);

	::MoveWindow(GetDlgItem(IDC_INFOSTATIC),rcMessage.left,rcMessage.top,rcMessage.right - rcMessage.left,iMessageHeight,TRUE);

	// Restore old font.
	::SelectObject(hMessageDC,hOldFont);

	// Move the check box and resize the window if necessary.
	RECT rcCheck;
	::GetWindowRect(hCheckCtrl,&rcCheck);
	ScreenToClient(&rcCheck);
	int iNewCheckTop = rcMessage.top + iMessageHeight + 8;

	if (iNewCheckTop > rcCheck.top)
	{
		::SetWindowPos(hCheckCtrl,0,
			rcCheck.left,iNewCheckTop,rcCheck.right - rcCheck.left,rcCheck.bottom - rcCheck.top,0);

		int iDiff = iNewCheckTop - rcCheck.top;

		// Resize the main window.
		RECT rcWindow;
		GetWindowRect(&rcWindow);
		MoveWindow(rcWindow.left,rcWindow.top,
			rcWindow.right - rcWindow.left,rcWindow.bottom - rcWindow.top + iDiff);
	}
}

LRESULT CInfoDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// Set the icon and window text.
	HANDLE hIcon;

	if (m_iFlags & INFODLG_ICONERROR)
	{
		hIcon = LoadIcon(NULL,IDI_HAND);
		SetWindowText(lngGetString(GENERAL_ERROR));
	}
	else if (m_iFlags & INFODLG_ICONWARNING)
	{
		hIcon = LoadIcon(NULL,IDI_EXCLAMATION);
		SetWindowText(lngGetString(GENERAL_WARNING));
	}
	else
	{
		hIcon = LoadIcon(NULL,IDI_ASTERISK);
		SetWindowText(lngGetString(GENERAL_INFORMATION));
	}

	::SendMessage(GetDlgItem(IDC_ICONSTATIC),STM_SETICON,(WPARAM)hIcon,0L);

	// Translate the window.
	Translate();

	// Display message and caculate correct messsage and window widths.
	InitializeMessage(n_szMessage);

	// Disable the cancel button if necessary.
	if (m_iFlags & INFODLG_NOCANCEL)
		::EnableWindow(GetDlgItem(IDCANCEL),FALSE);

	return TRUE;
}

LRESULT CInfoDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	*m_pRemember = !IsDlgButtonChecked(IDC_DISPLAYMSGCHECK) == TRUE;

	EndDialog(wID);
	return FALSE;
}

LRESULT CInfoDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}
