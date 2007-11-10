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
#include "ConfigAdvancedPage.h"
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"

CConfigAdvancedPage::CConfigAdvancedPage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_ADVANCED,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CConfigAdvancedPage::~CConfigAdvancedPage()
{
}

bool CConfigAdvancedPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a config translation section.
	if (!pLNG->EnterSection(_T("config")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDC_LOGCHECK,szStrValue))
		SetDlgItemText(IDC_LOGCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_SMOKECHECK,szStrValue))
		SetDlgItemText(IDC_SMOKECHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_FIFOGROUPSTATIC,szStrValue))
		SetDlgItemText(IDC_FIFOGROUPSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_FIFOINFOSTATIC,szStrValue))
		SetDlgItemText(IDC_FIFOINFOSTATIC,szStrValue);

	return true;
}

bool CConfigAdvancedPage::OnApply()
{
	TCHAR szFIFO[4];
	GetDlgItemText(IDC_FIFOEDIT,szFIFO,4);
#ifdef UNICODE
	int iFIFOSize = _wtoi(szFIFO);
#else
	int iFIFOSize = atoi(szFIFO);
#endif

	if (iFIFOSize < FIFO_MIN || iFIFOSize > FIFO_MAX)
	{
		TCHAR szMessage[128];
		lsnprintf_s(szMessage,128,lngGetString(ERROR_FIFOSIZE),FIFO_MIN,FIFO_MAX);
		MessageBox(szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
		return false;
	}

	// Remember the configuration.
	g_GlobalSettings.m_bLog = IsDlgButtonChecked(IDC_LOGCHECK) == TRUE;
	g_GlobalSettings.m_bSmoke = IsDlgButtonChecked(IDC_SMOKECHECK) == TRUE;
	g_GlobalSettings.m_iFIFOSize = iFIFOSize;

	return true;
}

void CConfigAdvancedPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/configuration.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CConfigAdvancedPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Load configuration.
	CheckDlgButton(IDC_LOGCHECK,g_GlobalSettings.m_bLog);
	CheckDlgButton(IDC_SMOKECHECK,g_GlobalSettings.m_bSmoke);

	::SendMessage(GetDlgItem(IDC_FIFOEDIT),EM_SETLIMITTEXT,3,0);
	TCHAR szFIFO[4];
#ifdef UNICODE
	_itow(g_GlobalSettings.m_iFIFOSize,szFIFO,10);
#else
	_itoa(g_GlobalSettings.m_iFIFOSize,szFIFO,10);
#endif
	SetDlgItemText(IDC_FIFOEDIT,szFIFO);

	// Translate the window.
	Translate();

	return TRUE;
}
