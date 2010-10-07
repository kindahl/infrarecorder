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
#include "burn_advanced_page.hh"
#include "ctrl_messages.hh"
#include "cdrtools_parse_strings.hh"
#include "settings.hh"
#include "string_table.hh"
#include "lang_util.hh"

CBurnAdvancedPage::CBurnAdvancedPage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_ADVANCED,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CBurnAdvancedPage::~CBurnAdvancedPage()
{
	Detach();
}

bool CBurnAdvancedPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a burn translation section.
	if (!pLng->EnterSection(_T("burn")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDC_OVERBURNCHECK,szStrValue))
		SetDlgItemText(IDC_OVERBURNCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_SWABCHECK,szStrValue))
		SetDlgItemText(IDC_SWABCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_IGNORESIZECHECK,szStrValue))
		SetDlgItemText(IDC_IGNORESIZECHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_IMMEDCHECK,szStrValue))
		SetDlgItemText(IDC_IMMEDCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_AUDIOMASTERCHECK,szStrValue))
		SetDlgItemText(IDC_AUDIOMASTERCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_FORCESPEEDCHECK,szStrValue))
		SetDlgItemText(IDC_FORCESPEEDCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_VARIRECCHECK,szStrValue))
		SetDlgItemText(IDC_VARIRECCHECK,szStrValue);

	return true;
}

bool CBurnAdvancedPage::OnApply()
{
	// Remember the configuration.
	g_BurnAdvancedSettings.m_bOverburn = IsDlgButtonChecked(IDC_OVERBURNCHECK) == TRUE;
	g_BurnAdvancedSettings.m_bSwab = IsDlgButtonChecked(IDC_SWABCHECK) == TRUE;
	g_BurnAdvancedSettings.m_bIgnoreSize = IsDlgButtonChecked(IDC_IGNORESIZECHECK) == TRUE;
	g_BurnAdvancedSettings.m_bImmed = IsDlgButtonChecked(IDC_IMMEDCHECK) == TRUE;
	g_BurnAdvancedSettings.m_bAudioMaster = IsDlgButtonChecked(IDC_AUDIOMASTERCHECK) == TRUE;
	g_BurnAdvancedSettings.m_bForceSpeed = IsDlgButtonChecked(IDC_FORCESPEEDCHECK) == TRUE;
	g_BurnAdvancedSettings.m_bVariRec = IsDlgButtonChecked(IDC_VARIRECCHECK) == TRUE;
	g_BurnAdvancedSettings.m_iVariRec = m_VariRecTrackBar.GetPos();

	return true;
}

void CBurnAdvancedPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/burn_options.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CBurnAdvancedPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// VariRec.
	m_VariRecTrackBar = GetDlgItem(IDC_VARIRECSLIDER);
	m_VariRecTrackBar.SetRange(-2,2);
	m_VariRecTrackBar.SetTipSide(TBTS_BOTTOM);

	// Setup the default settings.
	CheckDlgButton(IDC_OVERBURNCHECK,g_BurnAdvancedSettings.m_bOverburn);
	CheckDlgButton(IDC_SWABCHECK,g_BurnAdvancedSettings.m_bSwab);
	CheckDlgButton(IDC_IGNORESIZECHECK,g_BurnAdvancedSettings.m_bIgnoreSize);
	CheckDlgButton(IDC_IMMEDCHECK,g_BurnAdvancedSettings.m_bImmed);
	CheckDlgButton(IDC_AUDIOMASTERCHECK,g_BurnAdvancedSettings.m_bAudioMaster);
	CheckDlgButton(IDC_FORCESPEEDCHECK,g_BurnAdvancedSettings.m_bForceSpeed);
	CheckDlgButton(IDC_VARIRECCHECK,g_BurnAdvancedSettings.m_bVariRec);

	m_VariRecTrackBar.SetPos(g_BurnAdvancedSettings.m_iVariRec);
	TCHAR szVariRec[16];
	lsprintf(szVariRec,_T("%d"),m_VariRecTrackBar.GetPos());
	SetDlgItemText(IDC_VARIRECEDIT,szVariRec);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CBurnAdvancedPage::OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if ((BOOL)wParam == TRUE)
	{
		ckmmc::Device *pDevice =
			reinterpret_cast<ckmmc::Device *>(::SendMessage(GetParent(),WM_GETDEVICE,0,0));

		::EnableWindow(GetDlgItem(IDC_SWABCHECK),TRUE);
		::EnableWindow(GetDlgItem(IDC_AUDIOMASTERCHECK),
					   pDevice->support(ckmmc::Device::ckDEVICE_AUDIO_MASTER));

		::EnableWindow(GetDlgItem(IDC_FORCESPEEDCHECK),
					   pDevice->support(ckmmc::Device::ckDEVICE_FORCE_SPEED));

		bool bVariRec = pDevice->support(ckmmc::Device::ckDEVICE_VARIREC);
		::EnableWindow(GetDlgItem(IDC_VARIRECCHECK),bVariRec);
		::EnableWindow(GetDlgItem(IDC_VARIRECSLIDER),bVariRec);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CBurnAdvancedPage::OnHScroll(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (wParam == TB_ENDTRACK)
	{
		TCHAR szPos[16];
		lsprintf(szPos,_T("%d"),m_VariRecTrackBar.GetPos());
		SetDlgItemText(IDC_VARIRECEDIT,szPos);
	}

	bHandled = FALSE;
	return 0;
}
