/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#include <base/string_util.hh>
#include "edit_track_dlg.hh"
#include "string_table.hh"
#include "settings.hh"

CEditTrackDlg::CEditTrackDlg(CItemData *pItemData)
{
	m_pItemData = pItemData;
}

CEditTrackDlg::~CEditTrackDlg()
{
}

bool CEditTrackDlg::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a edittrack translation section.
	if (!pLng->EnterSection(_T("edittrack")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLng->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLng->GetValuePtr(IDC_TITLESTATIC,szStrValue))
		SetDlgItemText(IDC_TITLESTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_ARTISTSTATIC,szStrValue))
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
