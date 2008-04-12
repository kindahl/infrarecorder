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
#include "ProjectPropFileSysPage.h"
#include "Settings.h"
#include "LangUtil.h"
#include "StringTable.h"
#include "TransUtil.h"
#include "CtrlMessages.h"

CProjectPropFileSysPage::CProjectPropFileSysPage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_FILESYSTEM,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CProjectPropFileSysPage::~CProjectPropFileSysPage()
{
}

bool CProjectPropFileSysPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a projectprop translation section.
	if (!pLNG->EnterSection(_T("projectprop")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	if (pLNG->GetValuePtr(IDC_FILESYSSTATIC,szStrValue))
		SetDlgItemText(IDC_FILESYSSTATIC,szStrValue);

	return true;
}

bool CProjectPropFileSysPage::OnApply()
{
	g_ProjectSettings.m_iFileSystem = (int)m_FileSysCombo.GetItemData(m_FileSysCombo.GetCurSel());

	return true;
}

void CProjectPropFileSysPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/project_settings.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CProjectPropFileSysPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Setup the file system combo box.
	m_FileSysCombo = GetDlgItem(IDC_FILESYSCOMBO);

	int iIndex = 0;
	m_FileSysCombo.AddString(_T("ISO9660"));
	m_FileSysCombo.SetItemData(iIndex++,FILESYSTEM_ISO9660);
	m_FileSysCombo.AddString(_T("ISO9660 + UDF"));
	m_FileSysCombo.SetItemData(iIndex++,FILESYSTEM_ISO9660_UDF);
	m_FileSysCombo.AddString(_T("ISO9660 + UDF (DVD-Video)"));
	m_FileSysCombo.SetItemData(iIndex++,FILESYSTEM_DVDVIDEO);
	m_FileSysCombo.AddString(_T("UDF"));
	m_FileSysCombo.SetItemData(iIndex++,FILESYSTEM_UDF);	

	bool bFoundItem = false;
	for (int i = 0; i < m_FileSysCombo.GetCount(); i++)
	{
		if (m_FileSysCombo.GetItemData(i) == g_ProjectSettings.m_iFileSystem)
		{
			m_FileSysCombo.SetCurSel(i);
			bFoundItem = true;
			break;
		}
	}

	if (!bFoundItem)
		m_FileSysCombo.SetCurSel(0);

	m_iSelFileSystem = g_ProjectSettings.m_iFileSystem;

	if (g_ProjectSettings.m_bMultiSession)
		m_FileSysCombo.EnableWindow(FALSE);

	// Translate the window.
	Translate();

	return true;
}

LRESULT CProjectPropFileSysPage::OnFileSysChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	::SendMessage(GetParent(),WM_SETFILESYSTEM,m_iSelFileSystem,m_FileSysCombo.GetItemData(m_FileSysCombo.GetCurSel()));

	m_iSelFileSystem = (int)m_FileSysCombo.GetItemData(m_FileSysCombo.GetCurSel());

	bHandled = false;
	return 0;
}
