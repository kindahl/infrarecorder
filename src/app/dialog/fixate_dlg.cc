/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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
#include "infrarecorder.hh"
#include "string_table.hh"
#include "device_util.hh"
#include "settings.hh"
#include "lang_util.hh"
#include "fixate_dlg.hh"

CFixateDlg::CFixateDlg(bool bAppMode)
{
    m_bAppMode = bAppMode;
}

CFixateDlg::~CFixateDlg()
{
}

bool CFixateDlg::Translate()
{
    if (g_LanguageSettings.m_pLngProcessor == NULL)
        return false;

    CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
    
    // This dialog shares many strings with the erase dialog so we begin
    // borrow strings from the erase section.
    if (!pLng->EnterSection(_T("erase")))
        return false;

    // Translate.
    TCHAR *szStrValue;

    if (pLng->GetValuePtr(IDOK,szStrValue))
        SetDlgItemText(IDOK,szStrValue);
    if (pLng->GetValuePtr(IDCANCEL,szStrValue))
        SetDlgItemText(IDCANCEL,szStrValue);
    if (pLng->GetValuePtr(IDC_HELPBUTTON,szStrValue))
        SetDlgItemText(IDC_HELPBUTTON,szStrValue);
    if (pLng->GetValuePtr(IDC_RECORDERSTATIC,szStrValue))
        SetDlgItemText(IDC_RECORDERSTATIC,szStrValue);
    if (pLng->GetValuePtr(IDC_SIMULATECHECK,szStrValue))
        SetDlgItemText(IDC_SIMULATECHECK,szStrValue);

    if (!pLng->EnterSection(_T("fixate")))
        return false;

    if (pLng->GetValuePtr(IDD_FIXATEDLG,szStrValue))			// Title.
        SetWindowText(szStrValue);
    if (pLng->GetValuePtr(IDC_EJECTCHECK,szStrValue))
        SetDlgItemText(IDC_EJECTCHECK,szStrValue);

    return true;
}

LRESULT CFixateDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    CenterWindow(GetParent());

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
    std::vector<ckmmc::Device *>::const_iterator it;
    for (it = g_DeviceManager.devices().begin(); it !=
        g_DeviceManager.devices().end(); it++)
    {
        ckmmc::Device *pDevice = *it;

        // We only want to add recorder to the list.
        if (!pDevice->recorder())
            continue;

        m_RecorderCombo.AddString(NDeviceUtil::GetDeviceName(*pDevice).c_str());
        m_RecorderCombo.SetItemData(m_RecorderCombo.GetCount() - 1,
                                    reinterpret_cast<DWORD_PTR>(pDevice));
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
    ckmmc::Device *pDevice =
        reinterpret_cast<ckmmc::Device *>(m_RecorderCombo.GetItemData(0));
    ::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),
                   pDevice->support(ckmmc::Device::ckDEVICE_TEST_WRITE));

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
    ckmmc::Device *pDevice =
        reinterpret_cast<ckmmc::Device *>(m_RecorderCombo.GetItemData(
                                          m_RecorderCombo.GetCurSel()));

    ::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),
                   pDevice->support(ckmmc::Device::ckDEVICE_TEST_WRITE));

    bHandled = false;
    return 0;
}

LRESULT CFixateDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
    // Remember the configuration.
    g_FixateSettings.m_bEject = IsDlgButtonChecked(IDC_EJECTCHECK) == 1;
    g_FixateSettings.m_bSimulate = IsDlgButtonChecked(IDC_SIMULATECHECK) == 1;

    // For internal use only.
    g_FixateSettings.m_pRecorder =
        reinterpret_cast<ckmmc::Device *>(m_RecorderCombo.GetItemData(
                                          m_RecorderCombo.GetCurSel()));

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
