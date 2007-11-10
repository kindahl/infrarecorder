/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "NewFileExtDlg.h"
#include "../../Common/StringUtil.h"
#include "StringTable.h"
#include "Settings.h"

CNewFileExtDlg::CNewFileExtDlg()
{
}

CNewFileExtDlg::~CNewFileExtDlg()
{
}

bool CNewFileExtDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a newfileext translation section.
	if (!pLNG->EnterSection(_T("newfileext")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDD_NEWFILEEXTDLG,szStrValue))	// Title.
		SetWindowText(szStrValue);
	if (pLNG->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLNG->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLNG->GetValuePtr(IDC_DESCSTATIC,szStrValue))
		SetDlgItemText(IDC_DESCSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_EXTSTATIC,szStrValue))
		SetDlgItemText(IDC_EXTSTATIC,szStrValue);

	return true;
}

LRESULT CNewFileExtDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	m_DescEdit.SubclassWindow(GetDlgItem(IDC_DESCEDIT));
	m_ExtEdit.SubclassWindow(GetDlgItem(IDC_EXTEDIT));

	m_DescEdit.SetLimitText(63);
	m_ExtEdit.SetLimitText(63);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CNewFileExtDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	GetDlgItemText(IDC_DESCEDIT,m_szDescBuffer,63);
	GetDlgItemText(IDC_EXTEDIT,m_szExtBuffer,63);

	EndDialog(wID);
	return FALSE;
}

LRESULT CNewFileExtDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}
