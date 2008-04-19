/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "FixateDlg.h"
#include "../../Common/StringUtil.h"
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"

CFixateDlg::CFixateDlg(bool bAppMode)
{
	m_bAppMode = bAppMode;
}

CFixateDlg::~CFixateDlg()
{
}

bool CFixateDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// This dialog shares many strings with the erase dialog so we begin
	// borrow strings from the erase section.
	if (!pLNG->EnterSection(_T("erase")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLNG->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLNG->GetValuePtr(IDC_HELPBUTTON,szStrValue))
		SetDlgItemText(IDC_HELPBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_RECORDERSTATIC,szStrValue))
		SetDlgItemText(IDC_RECORDERSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_SIMULATECHECK,szStrValue))
		SetDlgItemText(IDC_SIMULATECHECK,szStrValue);

	if (!pLNG->EnterSection(_T("fixate")))
		return false;

	if (pLNG->GetValuePtr(IDD_FIXATEDLG,szStrValue))			// Title.
		SetWindowText(szStrValue);
	if (pLNG->GetValuePtr(IDC_EJECTCHECK,szStrValue))
		SetDlgItemText(IDC_EJECTCHECK,szStrValue);

	return true;
}

LRESULT CFixateDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// Add the window to the task bar and add a minimize button to the dialog if
	// the windows is in application mode.
	/*if (m_bAppMode)
	{
		ModifyStyle(WS_POPUPWINDOW | WS_DLGFRAME | DS_MODALFRAME,(WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) | WS_OVERLAPPED,0);
		ModifyStyleEx(WS_EX_DLGMODALFRAME,WS_EX_APPWINDOW,0);

		// Set icons.
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
		SetIcon(hIcon,TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON,::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
		SetIcon(hIconSmall,FALSE);
	}*/
	if (m_bAppMode)
	{
		ModifyStyle(0,WS_MINIMIZEBOX | WS_SYSMENU);
		ModifyStyleEx(0,WS_EX_APPWINDOW);

		HMENU hSysMenu = GetSystemMenu(FALSE);
		::InsertMenu(hSysMenu,0,MF_BYPOSITION,SC_RESTORE,_T("&Restore"));
		::InsertMenu(hSysMenu,2,MF_BYPOSITION,SC_MINIMIZE,_T("Mi&nimize"));
		::InsertMenu(hSysMenu,3,MF_BYPOSITION | MF_SEPARATOR,0,_T(""));
	}

	m_RecorderCombo = GetDlgItem(IDC_RECORDERCOMBO);

	// Recorder combo box.
	for (unsigned int i = 0; i < g_DeviceManager.GetDeviceCount(); i++)
	{
		// We only want to add recorder to the list.
		if (!g_DeviceManager.IsDeviceRecorder(i))
			continue;

		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(i);

		TCHAR szDeviceName[128];
		g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

		m_RecorderCombo.AddString(szDeviceName);
		m_RecorderCombo.SetItemData(m_RecorderCombo.GetCount() - 1,i);
	}

	if (m_RecorderCombo.GetCount() == 0)
	{
		m_RecorderCombo.AddString(lngGetString(FAILURE_NORECORDERS));
		m_RecorderCombo.EnableWindow(false);

		// Disable the OK button.
		::EnableWindow(GetDlgItem(IDOK),false);
	}

	m_RecorderCombo.SetCurSel(0);

	// Enable/disable the simulation checkbox depending on if the selected recorder
	// supports that operation.
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(m_RecorderCombo.GetItemData(0));
	::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING);

	// Setup the default settings.
	CheckDlgButton(IDC_EJECTCHECK,g_FixateSettings.m_bEject);
	CheckDlgButton(IDC_SIMULATECHECK,g_FixateSettings.m_bSimulate);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CFixateDlg::OnRecorderChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Enable/disable the simulation checkbox depending on if the selected recorder
	// supports that operation.
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(
		m_RecorderCombo.GetItemData(m_RecorderCombo.GetCurSel()));

	::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING);

	bHandled = false;
	return 0;
}

LRESULT CFixateDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Remember the configuration.
	g_FixateSettings.m_bEject = IsDlgButtonChecked(IDC_EJECTCHECK) == 1;
	g_FixateSettings.m_bSimulate = IsDlgButtonChecked(IDC_SIMULATECHECK) == 1;

	// For internal use only.
	g_FixateSettings.m_iRecorder = m_RecorderCombo.GetItemData(m_RecorderCombo.GetCurSel());

	EndDialog(wID);
	return FALSE;
}

LRESULT CFixateDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}

LRESULT CFixateDlg::OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/fixate_disc.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
	return 0;
}
