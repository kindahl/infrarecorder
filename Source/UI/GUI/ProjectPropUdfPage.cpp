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
#include "ProjectPropUdfPage.h"
#include "Settings.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "TransUtil.h"

CProjectPropUdfPage::CProjectPropUdfPage()
{
	m_psp.dwFlags |= PSP_HASHELP;
}

CProjectPropUdfPage::~CProjectPropUdfPage()
{
}

bool CProjectPropUdfPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a projectprop translation section.
	if (!pLNG->EnterSection(_T("projectprop")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	if (pLNG->GetValuePtr(IDC_VERSIONSTATIC,szStrValue))
		SetDlgItemText(IDC_VERSIONSTATIC,szStrValue);

	return true;
}

bool CProjectPropUdfPage::OnApply()
{
	return true;
}

void CProjectPropUdfPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/project_settings.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CProjectPropUdfPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Setup the version combo box.
	CComboBox VersionCombo = GetDlgItem(IDC_VERSIONCOMBO);
	VersionCombo.AddString(_T("1.02"));
	VersionCombo.SetCurSel(0);

	// Translate the window.
	Translate();

	return TRUE;
}
