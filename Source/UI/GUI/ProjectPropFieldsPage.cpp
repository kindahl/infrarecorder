/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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
#include "ProjectPropFieldsPage.h"
#include "Settings.h"
#include "LangUtil.h"
#include "StringTable.h"
#include "TransUtil.h"

CProjectPropFieldsPage::CProjectPropFieldsPage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_FIELDS,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CProjectPropFieldsPage::~CProjectPropFieldsPage()
{
}

bool CProjectPropFieldsPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a projectprop translation section.
	if (!pLNG->EnterSection(_T("projectprop")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	int iMaxStaticRight = 0;

	if (pLNG->GetValuePtr(IDC_PUBLISHERSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_PUBLISHERSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_PUBLISHERSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_PREPARERSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_PREPARERSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_PREPARERSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_SYSTEMSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_SYSTEMSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_SYSTEMSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_VOLUMESTATIC,szStrValue))
	{
		SetDlgItemText(IDC_VOLUMESTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_VOLUMESTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_COPYRIGHTSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_COPYRIGHTSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_COPYRIGHTSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_ABSTRACTSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_ABSTRACTSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_ABSTRACTSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_BIBLIOGRAPHICSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_BIBLIOGRAPHICSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_BIBLIOGRAPHICSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}

	// Make sure that the edit controls are not in the way of the statics.
	if (iMaxStaticRight > 75)
	{
		UpdateEditPos(m_hWnd,IDC_PUBLISHEREDIT,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_PREPAREREDIT,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_SYSTEMEDIT,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_VOLUMEEDIT,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_COPYRIGHTEDIT,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_ABSTRACTEDIT,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_BIBLIOGRAPHICEDIT,iMaxStaticRight);
	}

	return true;
}

bool CProjectPropFieldsPage::OnApply()
{
	GetDlgItemText(IDC_PUBLISHEREDIT,g_ProjectSettings.m_szPublisher,127);
	GetDlgItemText(IDC_PREPAREREDIT,g_ProjectSettings.m_szPreparer,127);
	GetDlgItemText(IDC_SYSTEMEDIT,g_ProjectSettings.m_szSystem,127);
	GetDlgItemText(IDC_VOLUMEEDIT,g_ProjectSettings.m_szVolumeSet,127);

	GetDlgItemText(IDC_COPYRIGHTEDIT,g_ProjectSettings.m_szCopyright,36);
	GetDlgItemText(IDC_ABSTRACTEDIT,g_ProjectSettings.m_szAbstract,36);
	GetDlgItemText(IDC_BIBLIOGRAPHICEDIT,g_ProjectSettings.m_szBibliographic,36);

	return true;
}

void CProjectPropFieldsPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/project_settings.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CProjectPropFieldsPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Set field length limits.
	::SendMessage(GetDlgItem(IDC_PUBLISHEREDIT),EM_SETLIMITTEXT,127,0);
	::SendMessage(GetDlgItem(IDC_PREPAREREDIT),EM_SETLIMITTEXT,127,0);
	::SendMessage(GetDlgItem(IDC_SYSTEMEDIT),EM_SETLIMITTEXT,127,0);
	::SendMessage(GetDlgItem(IDC_VOLUMEEDIT),EM_SETLIMITTEXT,127,0);

	::SendMessage(GetDlgItem(IDC_COPYRIGHTEDIT),EM_SETLIMITTEXT,36,0);
	::SendMessage(GetDlgItem(IDC_ABSTRACTEDIT),EM_SETLIMITTEXT,36,0);
	::SendMessage(GetDlgItem(IDC_BIBLIOGRAPHICEDIT),EM_SETLIMITTEXT,36,0);

	// Load the default settings.
	SetDlgItemText(IDC_PUBLISHEREDIT,g_ProjectSettings.m_szPublisher);
	SetDlgItemText(IDC_PREPAREREDIT,g_ProjectSettings.m_szPreparer);
	SetDlgItemText(IDC_SYSTEMEDIT,g_ProjectSettings.m_szSystem);
	SetDlgItemText(IDC_VOLUMEEDIT,g_ProjectSettings.m_szVolumeSet);

	SetDlgItemText(IDC_COPYRIGHTEDIT,g_ProjectSettings.m_szCopyright);
	SetDlgItemText(IDC_ABSTRACTEDIT,g_ProjectSettings.m_szAbstract);
	SetDlgItemText(IDC_BIBLIOGRAPHICEDIT,g_ProjectSettings.m_szBibliographic);

	// Translate the window.
	Translate();

	return true;
}
