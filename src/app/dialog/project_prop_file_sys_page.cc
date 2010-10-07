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
#include "project_prop_file_sys_page.hh"
#include "settings.hh"
#include "string_table.hh"
#include "lang_util.hh"
#include "trans_util.hh"
#include "ctrl_messages.hh"

CProjectPropFileSysPage::CProjectPropFileSysPage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_FILESYSTEM,szStrValue))
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
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a projectprop translation section.
	if (!pLng->EnterSection(_T("projectprop")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	if (pLng->GetValuePtr(IDC_FILESYSSTATIC,szStrValue))
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
		if (static_cast<int>(m_FileSysCombo.GetItemData(i)) == g_ProjectSettings.m_iFileSystem)
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
