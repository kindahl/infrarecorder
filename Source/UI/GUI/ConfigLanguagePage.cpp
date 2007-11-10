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
#include "ConfigLanguagePage.h"
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"

CConfigLanguagePage::CConfigLanguagePage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_LANGUAGE,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CConfigLanguagePage::~CConfigLanguagePage()
{
	// Clear the file list vector.
	m_LangFileList.clear();
}

bool CConfigLanguagePage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a config translation section.
	if (!pLNG->EnterSection(_T("config")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDC_LANGUAGESTATIC,szStrValue))
		SetDlgItemText(IDC_LANGUAGESTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_LANGUAGEINFOSTATIC,szStrValue))
		SetDlgItemText(IDC_LANGUAGEINFOSTATIC,szStrValue);

	return true;
}

bool CConfigLanguagePage::OnApply()
{
	// Remember the configuration.
	int iLangIndex = m_LangCombo.GetCurSel();

	if (iLangIndex == 0)
	{
		g_LanguageSettings.m_szLanguageFile[0] = '\0';
	}
	else
	{
		::GetModuleFileName(NULL,g_LanguageSettings.m_szLanguageFile,MAX_PATH - 1);
		ExtractFilePath(g_LanguageSettings.m_szLanguageFile);
		lstrcat(g_LanguageSettings.m_szLanguageFile,_T("Languages\\"));
		lstrcat(g_LanguageSettings.m_szLanguageFile,m_LangFileList[iLangIndex - 1].szFileName);
	}

	return true;
}

void CConfigLanguagePage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/configuration.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

void CConfigLanguagePage::FillLangCombo()
{
	// Always add the english language item.
	m_LangCombo.AddString(_T("English (default)"));
	m_LangCombo.SetCurSel(0);

	// Add file translations.
	WIN32_FIND_DATA FileData;
	HANDLE hFile;

	// Setup paths.
	TCHAR szFolderPath[MAX_PATH];
	::GetModuleFileName(NULL,szFolderPath,MAX_PATH - 1);
	ExtractFilePath(szFolderPath);
	lstrcat(szFolderPath,_T("Languages\\"));

	TCHAR szFilePath[MAX_PATH];
	lstrcpy(szFilePath,szFolderPath);
	lstrcat(szFilePath,_T("*.irl"));

	// Get the handle of the first file.
	hFile = FindFirstFile(szFilePath,&FileData);

	// Clear the file list vector.
	m_LangFileList.clear();

	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			// We want to ingore the "." and ".." records.
			if (!lstrcmp(FileData.cFileName,TEXT(".")) || !lstrcmp(FileData.cFileName,TEXT("..")))
				continue;

			// Add the file data to the vector.
			tLangFileData LangFileData;
			lstrcpy(LangFileData.szFileName,FileData.cFileName);
			m_LangFileList.push_back(LangFileData);

			// Remove the file extension and add the item.
			lstrcpy(szFilePath,FileData.cFileName);
			ChangeFileExt(szFilePath,_T(""));
			m_LangCombo.AddString(szFilePath);

			lstrcpy(szFilePath,szFolderPath);
			lstrcat(szFilePath,FileData.cFileName);

			if (!lstrcmp(szFilePath,g_LanguageSettings.m_szLanguageFile))
				m_LangCombo.SetCurSel(m_LangCombo.GetCount() - 1);
		}
		while (FindNextFile(hFile,&FileData));

		FindClose(hFile);
	}
}

LRESULT CConfigLanguagePage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Setup language combo box.
	m_LangCombo = GetDlgItem(IDC_LANGUAGECOMBO);

	// Load configuration.
	FillLangCombo();

	// Translate the window.
	Translate();

	return TRUE;
}
