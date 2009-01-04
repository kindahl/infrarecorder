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
#include "DriveLetterDlg.h"
#include "Settings.h"
#include "TransUtil.h"
#include "LangUtil.h"
#include "StringTable.h"
#include "DeviceManager.h"

CDriveLetterDlg::CDriveLetterDlg()
{
	m_cDriveLetter = 0;
}

CDriveLetterDlg::~CDriveLetterDlg()
{
}

bool CDriveLetterDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is an erase translation section.
	if (!pLNG->EnterSection(_T("driveletter")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLNG->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLNG->GetValuePtr(IDC_INFOSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_INFOSTATIC,szStrValue);

		int iDiff = UpdateStaticHeight(m_hWnd,IDC_INFOSTATIC,szStrValue);
		if (iDiff > 0)
		{
			// Move the combo box.
			RECT rcControl = { 0 };
			::GetWindowRect(GetDlgItem(IDC_DRIVECOMBO),&rcControl);
			ScreenToClient(&rcControl);

			::MoveWindow(GetDlgItem(IDC_DRIVECOMBO),rcControl.left,
				rcControl.top + iDiff,rcControl.right - rcControl.left,
				rcControl.bottom - rcControl.top,TRUE);

			// Resize the dialog window.
			GetWindowRect(&rcControl);

			MoveWindow(rcControl.left,
				rcControl.top,rcControl.right - rcControl.left,
				rcControl.bottom - rcControl.top + iDiff,TRUE);
		}
	}

	return true;
}

void CDriveLetterDlg::FillDriveCombo()
{
	// Get information about the drives connected to the system.
	unsigned long ulDrives = GetLogicalDrives();
	unsigned long ulMask = 0x00000001;

	for (int i = 0; i < 32; i++,ulMask <<= 1)
	{
		if ((ulDrives & ulMask) == 0)
			continue;

		// Get the driveletter on the form x:/
		TCHAR szDriveLetter[4];
		szDriveLetter[0] = i + _T('A');
		szDriveLetter[1] = _T(':');
		szDriveLetter[2] = _T('\\');
		szDriveLetter[3] = _T('\0');

		if (GetDriveType(szDriveLetter) != DRIVE_CDROM)
			continue;

		// Make sure that this letter is not already taken by another drive.
		tDeviceInfo *pDeviceInfo;
		bool bLetterTaken = false;
		for (unsigned int j = 0; j < g_DeviceManager.GetDeviceCount(); j++)
		{
			pDeviceInfo = g_DeviceManager.GetDeviceInfo(j);
			if (pDeviceInfo->Address.m_cDriveLetter == NULL)
				continue;

			if (pDeviceInfo->Address.m_cDriveLetter == szDriveLetter[0])
			{
				bLetterTaken = true;
				break;
			}
		}

		if (bLetterTaken)
			continue;

		m_DriveCombo.AddString(szDriveLetter);
		m_DriveCombo.SetItemData(m_DriveCombo.GetCount() - 1,(DWORD_PTR)szDriveLetter[0]);
	}

	if (m_DriveCombo.GetCount() == 0)
	{
		m_DriveCombo.AddString(lngGetString(FAILURE_NODEVICES));
		m_DriveCombo.EnableWindow(FALSE);

		::EnableWindow(GetDlgItem(IDOK),FALSE);
	}

	m_DriveCombo.SetCurSel(0);
}

LRESULT CDriveLetterDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());
	SetWindowText((TCHAR *)lParam);

	// Setup drive combo.
	m_DriveCombo = GetDlgItem(IDC_DRIVECOMBO);
	FillDriveCombo();

	// Translate the window.
	Translate();

	// Move the window down (below the splash screen).
	RECT rcWindow;
	GetWindowRect(&rcWindow);
	MoveWindow(rcWindow.left,rcWindow.top + 180,rcWindow.right - rcWindow.left,rcWindow.bottom - rcWindow.top);

	return TRUE;
}

LRESULT CDriveLetterDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	m_cDriveLetter = (TCHAR)m_DriveCombo.GetItemData(m_DriveCombo.GetCurSel());

	EndDialog(wID);
	return FALSE;
}

LRESULT CDriveLetterDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}

TCHAR CDriveLetterDlg::GetDriveLetter()
{
	return m_cDriveLetter;
}
