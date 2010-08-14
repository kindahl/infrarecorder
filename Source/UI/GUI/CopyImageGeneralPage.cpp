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

#include "stdafx.h"
#include "InfraRecorder.h"
#include "Settings.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "CtrlMessages.h"
#include "TransUtil.h"
#include "WinVer.h"
#include "VisualStyles.h"
#include "DeviceUtil.h"
#include "CopyImageGeneralPage.h"

CCopyImageGeneralPage::CCopyImageGeneralPage()
{
	m_uiParentTitleLen = 0;
	m_hRefreshIcon = NULL;
	m_hRefreshImageList = NULL;

	m_szFileName[0] = '\0';

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

CCopyImageGeneralPage::~CCopyImageGeneralPage()
{
	if (m_hRefreshImageList != NULL)
		ImageList_Destroy(m_hRefreshImageList);

	if (m_hRefreshIcon != NULL)
		DestroyIcon(m_hRefreshIcon);
}

bool CCopyImageGeneralPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a copy translation section.
	if (!pLng->EnterSection(_T("copy")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	int iMaxStaticRight = 0;

	if (pLng->GetValuePtr(IDC_SOURCESTATIC,szStrValue))
	{
		SetDlgItemText(IDC_SOURCESTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_SOURCESTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLng->GetValuePtr(IDC_IMAGESTATIC,szStrValue))
	{
		SetDlgItemText(IDC_IMAGESTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_IMAGESTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}

	// Make sure that the edit/combo controls are not in the way of the statics.
	if (iMaxStaticRight > 75)
	{
		UpdateEditPos(m_hWnd,IDC_SOURCECOMBO,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_IMAGEEDIT,iMaxStaticRight);
	}

	return true;
}

void CCopyImageGeneralPage::InitRefreshButton()
{
	m_hRefreshIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_REFRESHICON),IMAGE_ICON,/*GetSystemMetrics(SM_CXICON)*/16,/*GetSystemMetrics(SM_CYICON)*/16,LR_DEFAULTCOLOR);

	// In Windows XP there is a bug causing the button to loose it's themed style if
	// assigned an icon. The solution to this is to assign an image list instead.
	if (g_WinVer.m_ulMajorCCVersion >= 6)
	{
		// Get color depth (minimum requirement is 32-bits for alpha blended images).
		int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP),BITSPIXEL);

		m_hRefreshImageList = ImageList_Create(16,16,ILC_COLOR32 | (iBitsPixel < 32 ? ILC_MASK : 0),0,1);
		ImageList_AddIcon(m_hRefreshImageList,m_hRefreshIcon);

		BUTTON_IMAGELIST bImageList;
		bImageList.himl = m_hRefreshImageList;
		bImageList.margin.left = 0;
		bImageList.margin.top = 0;
		bImageList.margin.right = 0;
		bImageList.margin.bottom = 0;
		bImageList.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
		SendMessage(GetDlgItem(IDC_REFRESHBUTTON),BCM_SETIMAGELIST,0,(LPARAM)&bImageList);
	}
	else
	{
		SendMessage(GetDlgItem(IDC_REFRESHBUTTON),BM_SETIMAGE,IMAGE_ICON,(LPARAM)m_hRefreshIcon);
	}

	// If the application is themed, then adjust the size of the button.
	if (g_VisualStyles.IsThemeActive())
	{
		RECT rcButton;
		::GetWindowRect(GetDlgItem(IDC_REFRESHBUTTON),&rcButton);
		ScreenToClient(&rcButton);
		::SetWindowPos(GetDlgItem(IDC_REFRESHBUTTON),NULL,rcButton.left - 1,rcButton.top - 1,
			rcButton.right - rcButton.left + 2,rcButton.bottom - rcButton.top + 2,0);
	}
}

bool CCopyImageGeneralPage::OnApply()
{
	// For internal use only.
	g_CopyDiscSettings.m_pSource =
		reinterpret_cast<ckmmc::Device *>(m_SourceCombo.GetItemData(
										  m_SourceCombo.GetCurSel()));

	GetDlgItemText(IDC_IMAGEEDIT,m_szFileName,MAX_PATH - 1);

	return true;
}

void CCopyImageGeneralPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/copy_data_disc.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CCopyImageGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_SourceCombo = GetDlgItem(IDC_SOURCECOMBO);

	// Set the refresh button icon.
	InitRefreshButton();

	// Source combo box.
	std::vector<ckmmc::Device *>::const_iterator it;
	for (it = g_DeviceManager.devices().begin(); it !=
		g_DeviceManager.devices().end(); it++)
	{
		ckmmc::Device *pDevice = *it;

		m_SourceCombo.AddString(NDeviceUtil::GetDeviceName(*pDevice).c_str());
		m_SourceCombo.SetItemData(m_SourceCombo.GetCount() - 1,
								  reinterpret_cast<DWORD_PTR>(pDevice));
	}

	if (m_SourceCombo.GetCount() == 0)
	{
		m_SourceCombo.AddString(lngGetString(FAILURE_NODEVICES));
		m_SourceCombo.SetItemData(0,0);

		m_SourceCombo.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDC_BROWSEBUTTON),false);
	}

	m_SourceCombo.SetCurSel(0);

	// Disable the OK button.
	::EnableWindow(::GetDlgItem(GetParent(),IDOK),false);

	// Let the parent know which source drive that's selected.
	BOOL bDummy;
	OnSourceChange(NULL,NULL,NULL,bDummy);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CCopyImageGeneralPage::OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	ckmmc::Device *pDevice =
		reinterpret_cast<ckmmc::Device *>(m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel()));

	ATLASSERT(sizeof(ckmmc::Device *) == sizeof(LPARAM));

	// Check for media change.
	if (g_Core2.CheckMediaChange(*pDevice))
	{
		GetParentWindow(this).SendMessage(WM_CHECKMEDIA_BROADCAST,0,
										  reinterpret_cast<LPARAM>(pDevice));
	}

	return TRUE;
}

LRESULT CCopyImageGeneralPage::OnSourceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	ckmmc::Device *pSrcDevice =
		reinterpret_cast<ckmmc::Device *>(m_SourceCombo.GetItemData(
										  m_SourceCombo.GetCurSel()));

	::SendMessage(GetParent(),WM_SETDEVICE,1,reinterpret_cast<LPARAM>(pSrcDevice));

	// Kill any already running timers.
	::KillTimer(m_hWnd,TIMER_ID);
	::SetTimer(m_hWnd,TIMER_ID,TIMER_INTERVAL,NULL);

	// Initialize the drive media.
	GetParentWindow(this).SendMessage(WM_CHECKMEDIA_BROADCAST,0,
		m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel()));

	bHandled = false;
	return 0;
}

LRESULT CCopyImageGeneralPage::OnRefresh(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Initialize the drive media.
	GetParentWindow(this).SendMessage(WM_CHECKMEDIA_BROADCAST,0,
		m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel()));

	return 0;
}

LRESULT CCopyImageGeneralPage::OnBrowse(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	WTL::CFileDialog FileDialog(false,_T("iso"),_T("Untitled"),OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("Disc Images (*.iso)\0*.iso\0\0"),m_hWnd);
	
	if (FileDialog.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_IMAGEEDIT,FileDialog.m_szFileName);
		::EnableWindow(::GetDlgItem(GetParent(),IDOK),true);
	}

	return 0;
}

TCHAR *CCopyImageGeneralPage::GetFileName()
{
	return m_szFileName;
}