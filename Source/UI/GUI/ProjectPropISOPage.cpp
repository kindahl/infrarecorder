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
#include "ProjectPropISOPage.h"
#include "Settings.h"
#include "System.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "TransUtil.h"

CProjectPropISOPage::CProjectPropISOPage()
{
	m_psp.dwFlags |= PSP_HASHELP;
}

CProjectPropISOPage::~CProjectPropISOPage()
{
}

bool CProjectPropISOPage::Translate()
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

	if (pLNG->GetValuePtr(IDC_LEVELSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_LEVELSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_LEVELSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_CHARSETSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_CHARSETSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_CHARSETSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_FORMATSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_FORMATSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_FORMATSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_JOLIETCHECK,szStrValue))
		SetDlgItemText(IDC_JOLIETCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_JOLIETLONGNAMESCHECK,szStrValue))
		SetDlgItemText(IDC_JOLIETLONGNAMESCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_UDFCHECK,szStrValue))
		SetDlgItemText(IDC_UDFCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_ROCKRIDGECHECK,szStrValue))
		SetDlgItemText(IDC_ROCKRIDGECHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_OMITVNCHECK,szStrValue))
		SetDlgItemText(IDC_OMITVNCHECK,szStrValue);

	// Make sure that the edit controls are not in the way of the statics.
	if (iMaxStaticRight > 75)
	{
		UpdateEditPos(m_hWnd,IDC_LEVELCOMBO,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_CHARSETCOMBO,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_FORMATCOMBO,iMaxStaticRight);
	}

	return true;
}

bool CProjectPropISOPage::OnApply()
{
	g_ProjectSettings.m_iISOLevel = m_LevelCombo.GetCurSel();
	g_ProjectSettings.m_iISOCharSet = m_CharSetCombo.GetCurSel();
	g_ProjectSettings.m_iISOFormat = m_FormatCombo.GetCurSel();
	g_ProjectSettings.m_bJoliet = IsDlgButtonChecked(IDC_JOLIETCHECK) == TRUE;
	g_ProjectSettings.m_bJolietLongNames = IsDlgButtonChecked(IDC_JOLIETLONGNAMESCHECK) == TRUE;
	g_ProjectSettings.m_bUDF = IsDlgButtonChecked(IDC_UDFCHECK) == TRUE;
	g_ProjectSettings.m_bRockRidge = IsDlgButtonChecked(IDC_ROCKRIDGECHECK) == TRUE;
	g_ProjectSettings.m_bOmitVN = IsDlgButtonChecked(IDC_OMITVNCHECK) == TRUE;

	return true;
}

void CProjectPropISOPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/project_settings.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CProjectPropISOPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Setup the level combo box.
	m_LevelCombo = GetDlgItem(IDC_LEVELCOMBO);

	m_LevelCombo.AddString(lngGetString(PROJECTPROP_ISOLEVEL1));
	m_LevelCombo.AddString(lngGetString(PROJECTPROP_ISOLEVEL2));
	m_LevelCombo.AddString(lngGetString(PROJECTPROP_ISOLEVEL3));
	m_LevelCombo.AddString(lngGetString(PROJECTPROP_ISOLEVEL4));
	m_LevelCombo.SetCurSel(g_ProjectSettings.m_iISOLevel);

	// Setup the character set combo box.
	m_CharSetCombo = GetDlgItem(IDC_CHARSETCOMBO);

	int iCurCharSet = CodePageToCharacterSet(GetACP());
	TCHAR szBuffer[64];

	for (unsigned int i = 0; i < 37; i++)
	{
		if (i == iCurCharSet)
		{
			lstrcpy(szBuffer,g_szCharacterSets[i]);
			lstrcat(szBuffer,lngGetString(MISC_AUTODETECT));

			m_CharSetCombo.AddString(szBuffer);
		}
		else
		{
			m_CharSetCombo.AddString(g_szCharacterSets[i]);
		}
	}

	m_CharSetCombo.SetCurSel(g_ProjectSettings.m_iISOCharSet);

	// Format combo box.
	m_FormatCombo = GetDlgItem(IDC_FORMATCOMBO);

	m_FormatCombo.AddString(lngGetString(PROJECTPROP_MODE1));
	m_FormatCombo.AddString(lngGetString(PROJECTPROP_MODE2));
	m_FormatCombo.SetCurSel(g_ProjectSettings.m_iISOFormat);

	// Joliet.
	CheckDlgButton(IDC_JOLIETCHECK,g_ProjectSettings.m_bJoliet);
	CheckDlgButton(IDC_JOLIETLONGNAMESCHECK,g_ProjectSettings.m_bJolietLongNames);
	
	if (g_ProjectSettings.m_bJoliet)
	{
		::EnableWindow(GetDlgItem(IDC_JOLIETLONGNAMESCHECK),true);
		::EnableWindow(GetDlgItem(IDC_ROCKRIDGECHECK),false);
		CheckDlgButton(IDC_ROCKRIDGECHECK,true);
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_JOLIETLONGNAMESCHECK),false);
		CheckDlgButton(IDC_ROCKRIDGECHECK,g_ProjectSettings.m_bRockRidge);
	}

	// Miscellaneous.
	CheckDlgButton(IDC_UDFCHECK,g_ProjectSettings.m_bUDF);
	CheckDlgButton(IDC_OMITVNCHECK,g_ProjectSettings.m_bOmitVN);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CProjectPropISOPage::OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	LRESULT lResult = DefWindowProc();

	if (HIWORD(wParam) == BN_CLICKED)
	{
		// IDC_JOLIETCHECK clicked?
		if (LOWORD(wParam) == IDC_JOLIETCHECK)
		{
			bool bJoliet = IsDlgButtonChecked(IDC_JOLIETCHECK) == TRUE;

			::EnableWindow(GetDlgItem(IDC_JOLIETLONGNAMESCHECK),bJoliet);

			::EnableWindow(GetDlgItem(IDC_ROCKRIDGECHECK),!bJoliet);
			if (bJoliet)
				CheckDlgButton(IDC_ROCKRIDGECHECK,true);
		}
	}

	bHandled = false;
	return lResult;
}
