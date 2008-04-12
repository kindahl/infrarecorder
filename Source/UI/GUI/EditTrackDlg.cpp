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
#include "EditTrackDlg.h"
#include "../../Common/StringUtil.h"
#include "StringTable.h"
#include "Settings.h"

CEditTrackDlg::CEditTrackDlg(CItemData *pItemData)
{
	m_pItemData = pItemData;
}

CEditTrackDlg::~CEditTrackDlg()
{
}

bool CEditTrackDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a edittrack translation section.
	if (!pLNG->EnterSection(_T("edittrack")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLNG->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLNG->GetValuePtr(IDC_TITLESTATIC,szStrValue))
		SetDlgItemText(IDC_TITLESTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_ARTISTSTATIC,szStrValue))
		SetDlgItemText(IDC_ARTISTSTATIC,szStrValue);

	return true;
}

LRESULT CEditTrackDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());
	SetWindowText((TCHAR *)lParam);

	SetDlgItemText(IDC_TITLEEDIT,m_pItemData->GetAudioData()->szTrackTitle);
	SetDlgItemText(IDC_ARTISTEDIT,m_pItemData->GetAudioData()->szTrackArtist);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CEditTrackDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	GetDlgItemText(IDC_TITLEEDIT,m_pItemData->GetAudioData()->szTrackTitle,159);
	GetDlgItemText(IDC_ARTISTEDIT,m_pItemData->GetAudioData()->szTrackArtist,159);

	EndDialog(wID);
	return FALSE;
}

LRESULT CEditTrackDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}
