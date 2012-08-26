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
#include <base/string_util.hh>
#include "infrarecorder.hh"
#include "device_dlg.hh"
#include "string_table.hh"
#include "settings.hh"
#include "lang_util.hh"
#include "device_util.hh"
#include "devices_dlg.hh"

void CDevicesDlg::ScanCallback::event_status(ckmmc::DeviceManager::ScanCallback::Status Status)
{
    if (Status == ckmmc::DeviceManager::ScanCallback::ckEVENT_DEV_SCAN)
    {
        m_WaitDlg.SetMessage(lngGetString(INIT_SCANBUS));
    }
    else if (Status == ckmmc::DeviceManager::ScanCallback::ckEVENT_DEV_CAP)
    {
        m_WaitDlg.SetMessage(lngGetString(INIT_LOADCAPABILITIES));
    }
}

bool CDevicesDlg::ScanCallback::event_device(ckmmc::Device::Address &Addr)
{
    return true;
}

CDevicesDlg::CDevicesDlg()
{
    m_hListImageList = NULL;
}

CDevicesDlg::~CDevicesDlg()
{
    if (m_hListImageList)
        ImageList_Destroy(m_hListImageList);
}

bool CDevicesDlg::Translate()
{
    if (g_LanguageSettings.m_pLngProcessor == NULL)
        return false;

    CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
    
    // Make sure that there is a devices translation section.
    if (!pLng->EnterSection(_T("devices")))
        return false;

    // Translate.
    TCHAR *szStrValue;

    if (pLng->GetValuePtr(IDD_DEVICESDLG,szStrValue))			// Title.
        SetWindowText(szStrValue);
    if (pLng->GetValuePtr(IDOK,szStrValue))
        SetDlgItemText(IDOK,szStrValue);
    if (pLng->GetValuePtr(IDC_HELPBUTTON,szStrValue))
        SetDlgItemText(IDC_HELPBUTTON,szStrValue);
    if (pLng->GetValuePtr(IDC_RESCANBUTTON,szStrValue))
        SetDlgItemText(IDC_RESCANBUTTON,szStrValue);
    if (pLng->GetValuePtr(IDC_INFOSTATIC,szStrValue))
        SetDlgItemText(IDC_INFOSTATIC,szStrValue);

    return true;
}

void CDevicesDlg::FillListView()
{
    int iItemCount = 0;

    std::vector<ckmmc::Device *>::const_iterator it;
    for (it = g_DeviceManager.devices().begin(); it !=
        g_DeviceManager.devices().end(); it++)
    {
        ckmmc::Device *pDevice = *it;

        TCHAR szBuffer[64];

        if (pDevice->address().bus_ == -1 || pDevice->address().target_ == -1 ||
            pDevice->address().lun_ == -1)
        {
            m_ListView.AddItem(iItemCount,0,_T("[?,?,?]"),0);
        }
        else
        {
            lsprintf(szBuffer,_T("[%d,%d,%d]"),pDevice->address().bus_,
                     pDevice->address().target_,pDevice->address().lun_);
            m_ListView.AddItem(iItemCount,0,szBuffer,0);
        }

        lsprintf(szBuffer,_T("%c:"),pDevice->address().device_[0]);

        m_ListView.AddItem(iItemCount,1,szBuffer,0);
        m_ListView.AddItem(iItemCount,2,pDevice->vendor(),0);
        m_ListView.AddItem(iItemCount,3,pDevice->identifier(),0);
        m_ListView.AddItem(iItemCount,4,pDevice->revision(),0);
        m_ListView.SetItemData(iItemCount,reinterpret_cast<DWORD_PTR>(pDevice));

        iItemCount++;
    }
}

void CDevicesDlg::InitializeListView()
{
    // Create the image list.
    HINSTANCE hInstance = LoadLibrary(_T("shell32.dll"));
        HICON hIcon = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(12),IMAGE_ICON,16,16,/*LR_DEFAULTCOLOR*/LR_LOADTRANSPARENT);
    FreeLibrary(hInstance);

    m_hListImageList = ImageList_Create(16,16,ILC_COLOR32,0,1);
    ImageList_AddIcon(m_hListImageList,hIcon);

    DestroyIcon(hIcon);

    // Setup the list view.
    m_ListView.SubclassWindow(GetDlgItem(IDC_DEVICELIST));
    m_ListView.SetImageList(m_hListImageList,LVSIL_NORMAL);
    m_ListView.SetImageList(m_hListImageList,LVSIL_SMALL);
    m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

    // Add the columns.
    m_ListView.AddColumn(lngGetString(COLUMN_ID),0);
    m_ListView.SetColumnWidth(0,60);
    m_ListView.AddColumn(lngGetString(COLUMN_DRIVE),1);
    m_ListView.SetColumnWidth(1,40);  // 25 is not enough for wide letters like "W:"
    m_ListView.AddColumn(lngGetString(COLUMN_VENDOR),2);
    m_ListView.SetColumnWidth(2,65);
    m_ListView.AddColumn(lngGetString(COLUMN_IDENTIFICATION),3);
    m_ListView.SetColumnWidth(3,125);
    m_ListView.AddColumn(lngGetString(COLUMN_REVISION),4);
    m_ListView.SetColumnWidth(4,55);
}

LRESULT CDevicesDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    CenterWindow(GetParent());

    // Initialize the list view.
    InitializeListView();

    // Fill the list view.
    FillListView();

    // Translate the window.
    Translate();

    return TRUE;
}

LRESULT CDevicesDlg::OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    // Detach the internal list view control.
    m_ListView.UnsubclassWindow();

    bHandled = false;
    return 0;
}

LRESULT CDevicesDlg::OnListDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
    if (m_ListView.GetSelectedCount() > 0)
    {
        ckmmc::Device *pDevice =
            reinterpret_cast<ckmmc::Device *>(m_ListView.GetItemData(
                                              m_ListView.GetSelectedIndex()));

        ckcore::tstring Title = lngGetString(PROPERTIES_TITLE);
        Title += NDeviceUtil::GetDeviceName(*pDevice);

        CDeviceDlg DeviceDlg(*pDevice,Title.c_str());
        DeviceDlg.DoModal();
    }

    bHandled = false;
    return 0;
}

LRESULT CDevicesDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
    EndDialog(wID);
    return FALSE;
}

LRESULT CDevicesDlg::OnRescan(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
    // Empty the list view.
    m_ListView.DeleteAllItems();

    // Rescan the bus.
    CWaitDlg WaitDlg;
    WaitDlg.Create(m_hWnd);
    WaitDlg.ShowWindow(SW_SHOW);
        ScanCallback Callback(WaitDlg);
        g_DeviceManager.scan(&Callback);
    WaitDlg.DestroyWindow();

    // Fill the list view.
    FillListView();

    return FALSE;
}

LRESULT CDevicesDlg::OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

    ExtractFilePath(szFileName);
    lstrcat(szFileName,lngGetManual());
    lstrcat(szFileName,_T("::/how_to_use/device_configuration.html"));

    HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
    return 0;
}