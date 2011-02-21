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
#include <ckmmc/util.hh>
#include "read_options_page.hh"
#include "ctrl_messages.hh"
#include "string_table.hh"
#include "lang_util.hh"
#include "settings.hh"
#include "trans_util.hh"
#include "core2.hh"
#include "core2_util.hh"
#include "version.hh"
#include "infrarecorder.hh"
#include "visual_styles.hh"

CReadOptionsPage::CReadOptionsPage(bool bEnableClone,bool bEnableSpeed)
{
	m_bEnableClone = bEnableClone;
	m_bEnableSpeed = bEnableSpeed;
	m_hRefreshIcon = NULL;
	m_hRefreshImageList = NULL;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_READ,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CReadOptionsPage::~CReadOptionsPage()
{
	if (m_hRefreshImageList != NULL)
		ImageList_Destroy(m_hRefreshImageList);

	if (m_hRefreshIcon != NULL)
		DestroyIcon(m_hRefreshIcon);
}

bool CReadOptionsPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a burn translation section.
	if (!pLng->EnterSection(_T("read")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	int iMaxStaticRight = 0;

	if (pLng->GetValuePtr(IDC_NOREADERRCHECK,szStrValue))
		SetDlgItemText(IDC_NOREADERRCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_READSUBCHANNELCHECK,szStrValue))
		SetDlgItemText(IDC_READSUBCHANNELCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_READSPEEDSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_READSPEEDSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_READSPEEDSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}

	// Make sure that the edit/combo controls are not in the way of the statics.
	if (iMaxStaticRight > 75)
		UpdateEditPos(m_hWnd,IDC_READSPEEDCOMBO,iMaxStaticRight,true);

	return true;
}

bool CReadOptionsPage::OnApply()
{
	// Remember the configuration.
	g_ReadSettings.m_bIgnoreErr = IsDlgButtonChecked(IDC_NOREADERRCHECK) == TRUE;
	g_ReadSettings.m_bClone = IsDlgButtonChecked(IDC_READSUBCHANNELCHECK) == TRUE;
	g_ReadSettings.m_iReadSpeed = m_ReadSpeedCombo.GetItemData(m_ReadSpeedCombo.GetCurSel());

	return true;
}

void CReadOptionsPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/read_options.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

void CReadOptionsPage::UpdateSpeeds()
{
	ckmmc::Device &Device =
		*reinterpret_cast<ckmmc::Device *>(::SendMessage(GetParent(),WM_GETDEVICE,1,0));

	// Maximum read speed.
	m_ReadSpeedCombo.ResetContent();
	m_ReadSpeedCombo.AddString(lngGetString(MISC_MAXIMUM));
	m_ReadSpeedCombo.SetItemData(0,0xFFFFFFFF);
	m_ReadSpeedCombo.SetCurSel(0);

	// Get current profile.
	ckmmc::Device::Profile Profile = Device.profile();
	if (Profile != ckmmc::Device::ckPROFILE_NONE)
	{
		const std::vector<ckcore::tuint32> &ReadSpeeds = Device.read_speeds();

		std::vector<ckcore::tuint32>::const_iterator it;
		for (it = ReadSpeeds.begin(); it != ReadSpeeds.end(); it++)
		{
			m_ReadSpeedCombo.AddString(ckmmc::util::sec_to_disp_speed(*it,Profile).c_str());
			m_ReadSpeedCombo.SetItemData(m_ReadSpeedCombo.GetCount() - 1,
				static_cast<DWORD_PTR>(ckmmc::util::sec_to_human_speed(*it,
									   ckmmc::Device::ckPROFILE_CDR)));
		}
	}
}

void CReadOptionsPage::CheckMedia()
{
	if (m_bEnableSpeed)
	{
		UpdateSpeeds();
	}
	else
	{
		m_ReadSpeedCombo.ResetContent();
		m_ReadSpeedCombo.AddString(lngGetString(MISC_AUTO));
		m_ReadSpeedCombo.SetCurSel(0);
	}
}

LRESULT CReadOptionsPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_ReadSpeedCombo = GetDlgItem(IDC_READSPEEDCOMBO);

	CheckDlgButton(IDC_NOREADERRCHECK,g_ReadSettings.m_bIgnoreErr);

	// Enable/disable options.
	if (!m_bEnableClone)
	{
		::EnableWindow(GetDlgItem(IDC_READSUBCHANNELCHECK),FALSE);
		CheckDlgButton(IDC_READSUBCHANNELCHECK,m_bCloneCheck);
	}
	else
	{
		CheckDlgButton(IDC_READSUBCHANNELCHECK,g_ReadSettings.m_bClone);
	}

	if (!m_bEnableSpeed)
	{
		::EnableWindow(GetDlgItem(IDC_READSPEEDSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_READSPEEDCOMBO),FALSE);
	}

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CReadOptionsPage::OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if ((BOOL)wParam == TRUE)
		CheckMedia();

	bHandled = FALSE;
	return 0;
}

LRESULT CReadOptionsPage::OnCheckMedia(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CheckMedia();

	bHandled = FALSE;
	return 0;
}

void CReadOptionsPage::SetCloneMode(bool bEnable)
{
	m_bCloneCheck = bEnable;

	if (IsWindow())
		CheckDlgButton(IDC_READSUBCHANNELCHECK,bEnable);
}
