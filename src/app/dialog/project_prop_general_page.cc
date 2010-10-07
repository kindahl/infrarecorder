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
#include <base/StringUtil.h>
#include "project_prop_general_page.hh"
#include "project_manager.hh"
#include "string_table.hh"
#include "settings.hh"
#include "lang_util.hh"

CProjectPropGeneralPage::CProjectPropGeneralPage()
{
	m_hIcon = NULL;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_GENERAL,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CProjectPropGeneralPage::~CProjectPropGeneralPage()
{
	if (m_hIcon != NULL)
		DestroyIcon(m_hIcon);
}

bool CProjectPropGeneralPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a projectprop translation section.
	if (!pLng->EnterSection(_T("projectprop")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDC_TYPELABELSTATIC,szStrValue))
		SetDlgItemText(IDC_TYPELABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_SIZELABELSTATIC,szStrValue))
		SetDlgItemText(IDC_SIZELABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_CONTAINSLABELSTATIC,szStrValue))
		SetDlgItemText(IDC_CONTAINSLABELSTATIC,szStrValue);

	return true;
}

bool CProjectPropGeneralPage::OnApply()
{
	GetDlgItemText(IDC_NAMEEDIT,g_ProjectSettings.m_szLabel,127);

	return true;
}

void CProjectPropGeneralPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/project_settings.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

void CProjectPropGeneralPage::SetupDataProject()
{
	TCHAR szBuffer[64];

	// Create the icon.
	m_hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_DATAICON),
		IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	SendMessage(GetDlgItem(IDC_ICONSTATIC),STM_SETICON,(WPARAM)m_hIcon,0L);

	// Label name.
	SetDlgItemText(IDC_NAMEEDIT,g_ProjectSettings.m_szLabel);

	// Type.
	SetDlgItemText(IDC_TYPESTATIC,lngGetString(PROJECT_DATA));

	// Size.
	unsigned __int64 uiSize = g_ProjectManager.GetProjectSize();
	TCHAR szSizeText[32];
	FormatBytes(szBuffer,uiSize);
	lsprintf(szSizeText,_T(" (%I64d Bytes)"),uiSize);
	lstrcat(szBuffer,szSizeText);
	SetDlgItemText(IDC_SIZESTATIC,szBuffer);

	// Contents.
	unsigned __int64 uiFileCount,uiFolderCount,uiTrackCount;
	g_ProjectManager.GetProjectContents(uiFileCount,uiFolderCount,uiTrackCount);
	lsnprintf_s(szBuffer,64,lngGetString(PROJECT_CONTENTS),uiFileCount,uiFolderCount,uiTrackCount);
	SetDlgItemText(IDC_CONTAINSSTATIC,szBuffer);
}

void CProjectPropGeneralPage::SetupAudioProject()
{
	TCHAR szBuffer[64];

	// Create the icon.
	m_hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_AUDIOICON),
		IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	SendMessage(GetDlgItem(IDC_ICONSTATIC),STM_SETICON,(WPARAM)m_hIcon,0L);

	// Label name.
	SetDlgItemText(IDC_NAMEEDIT,lngGetString(MISC_NOTAVAILABLE));
	::EnableWindow(GetDlgItem(IDC_NAMEEDIT),false);

	// Type.
	SetDlgItemText(IDC_TYPESTATIC,lngGetString(PROJECT_AUDIO));

	// Size.
	lsnprintf_s(szBuffer,64,lngGetString(MISC_MINUTES),g_ProjectManager.GetProjectSize()/(1000 * 60));
	SetDlgItemText(IDC_SIZESTATIC,szBuffer);

	// Contents.
	unsigned __int64 uiFileCount,uiFolderCount,uiTrackCount;
	g_ProjectManager.GetProjectContents(uiFileCount,uiFolderCount,uiTrackCount);
	lsnprintf_s(szBuffer,64,lngGetString(PROJECT_CONTENTS),uiFileCount,uiFolderCount,uiTrackCount);
	SetDlgItemText(IDC_CONTAINSSTATIC,szBuffer);
}

void CProjectPropGeneralPage::SetupMixedProject()
{
	TCHAR szBuffer[64];

	// Create the icon.
	m_hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_MIXEDICON),
		IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	SendMessage(GetDlgItem(IDC_ICONSTATIC),STM_SETICON,(WPARAM)m_hIcon,0L);

	// Label name.
	SetDlgItemText(IDC_NAMEEDIT,g_ProjectSettings.m_szLabel);

	// Type.
	SetDlgItemText(IDC_TYPESTATIC,lngGetString(PROJECT_MIXED));

	// Size.
	unsigned __int64 uiSize = g_ProjectManager.GetProjectSize();
	TCHAR szSizeText[32];
	FormatBytes(szBuffer,uiSize);
	lsprintf(szSizeText,_T(" (%I64d Bytes)"),uiSize);
	lstrcat(szBuffer,szSizeText);
	SetDlgItemText(IDC_SIZESTATIC,szBuffer);

	// Contents.
	unsigned __int64 uiFileCount,uiFolderCount,uiTrackCount;
	g_ProjectManager.GetProjectContents(uiFileCount,uiFolderCount,uiTrackCount);
	lsnprintf_s(szBuffer,64,lngGetString(PROJECT_CONTENTS),uiFileCount,uiFolderCount,uiTrackCount);
	SetDlgItemText(IDC_CONTAINSSTATIC,szBuffer);
}

LRESULT CProjectPropGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Set edit field length limit.
	::SendMessage(GetDlgItem(IDC_NAMEEDIT),EM_SETLIMITTEXT,127,0);

	switch (g_ProjectManager.GetProjectType())
	{
		case PROJECTTYPE_AUDIO:
			SetupAudioProject();
			break;

		case PROJECTTYPE_MIXED:
			SetupMixedProject();
			break;

		case PROJECTTYPE_DATA:
			SetupDataProject();
			break;
	};

	// Translate the window.
	Translate();

	return TRUE;
}
