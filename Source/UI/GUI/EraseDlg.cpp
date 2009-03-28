/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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
#include "EraseDlg.h"
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"
#include "Core2Util.h"
#include "WinVer.h"
#include "VisualStyles.h"

CEraseDlg::CEraseDlg(bool bAppMode)
{
	m_bAppMode = bAppMode;

	m_hRefreshIcon = NULL;
	m_hRefreshImageList = NULL;

	m_uiRecorderTextLen = 0;
}

CEraseDlg::~CEraseDlg()
{
	if (m_hRefreshImageList != NULL)
		ImageList_Destroy(m_hRefreshImageList);

	if (m_hRefreshIcon != NULL)
		DestroyIcon(m_hRefreshIcon);
}

bool CEraseDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is an erase translation section.
	if (!pLNG->EnterSection(_T("erase")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDD_ERASEDLG,szStrValue))				// Title.
		SetWindowText(szStrValue);
	if (pLNG->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLNG->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLNG->GetValuePtr(IDC_HELPBUTTON,szStrValue))
		SetDlgItemText(IDC_HELPBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_RECORDERSTATIC,szStrValue))
		SetDlgItemText(IDC_RECORDERSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_METHODSTATIC,szStrValue))
		SetDlgItemText(IDC_METHODSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_FORCECHECK,szStrValue))
		SetDlgItemText(IDC_FORCECHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_EJECTCHECK,szStrValue))
		SetDlgItemText(IDC_EJECTCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_SIMULATECHECK,szStrValue))
		SetDlgItemText(IDC_SIMULATECHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_SPEEDSTATIC,szStrValue))
		SetDlgItemText(IDC_SPEEDSTATIC,szStrValue);

	return true;
}

bool CEraseDlg::InitRecorderMedia()
{
	// Make sure that the current device has been successfully opened.
	if (!m_CurDevice.IsOpen())
		return false;

	if (m_uiRecorderTextLen == 0)
		m_uiRecorderTextLen = ::GetWindowTextLength(GetDlgItem(IDC_RECORDERSTATIC));

	TCHAR szBuffer[256];
	::GetWindowText(GetDlgItem(IDC_RECORDERSTATIC),szBuffer,m_uiRecorderTextLen + 1);

	// Empty the method combo box.
	m_MethodCombo.ResetContent();

	// Open the device.
	UINT_PTR uiDeviceIndex = m_RecorderCombo.GetItemData(m_RecorderCombo.GetCurSel());
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(uiDeviceIndex);

	// Get current profile.
	unsigned short usProfile = PROFILE_NONE;
	if (!g_Core2.GetProfile(&m_CurDevice,usProfile))
		return false;

	//g_LogDlg.AddLine(_T("  Current profile: 0x%.2X"),usProfile);

	bool bSupportedProfile = false;
	switch (usProfile)
	{
		case PROFILE_CDRW:
			m_MethodCombo.AddString(lngGetString(BLANKMODE_FULL));
			m_MethodCombo.AddString(lngGetString(BLANKMODE_MINIMAL));
			m_MethodCombo.AddString(lngGetString(BLANKMODE_UNCLOSE));
			m_MethodCombo.AddString(lngGetString(BLANKMODE_SESSION));
			m_MethodCombo.SetItemData(0,CCore2::ERASE_BLANK_FULL);
			m_MethodCombo.SetItemData(1,CCore2::ERASE_BLANK_MINIMAL);
			m_MethodCombo.SetItemData(2,CCore2::ERASE_BLANK_UNCLOSE);
			m_MethodCombo.SetItemData(3,CCore2::ERASE_BLANK_SESSION);

			m_MethodCombo.SetCurSel(0);
			bSupportedProfile = true;

			// Enable simulation if supported by the recorder.
			::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING);

			// Enable force erase.
			::EnableWindow(GetDlgItem(IDC_FORCECHECK),FALSE);
			break;

		case PROFILE_DVDPLUSRW:
		case PROFILE_DVDPLUSRW_DL:
			m_MethodCombo.AddString(lngGetString(FORMATMODE_QUICK));
			m_MethodCombo.AddString(lngGetString(FORMATMODE_FULL));
			m_MethodCombo.SetItemData(0,CCore2::ERASE_FORMAT_QUICK);
			m_MethodCombo.SetItemData(1,CCore2::ERASE_FORMAT_FULL);

			m_MethodCombo.SetCurSel(1);
			bSupportedProfile = true;

			// Disable simulation (not supported for DVD+RW media).
			::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),FALSE);

			// Disable force erase.
			::EnableWindow(GetDlgItem(IDC_FORCECHECK),FALSE);
			break;

		case PROFILE_DVDRAM:
			m_MethodCombo.AddString(lngGetString(FORMATMODE_FULL));
			m_MethodCombo.SetItemData(0,CCore2::ERASE_FORMAT_FULL);

			m_MethodCombo.SetCurSel(0);
			bSupportedProfile = true;

			// Disable simulation (not supported for DVD-RAM media).
			::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),FALSE);

			// Disable force erase.
			::EnableWindow(GetDlgItem(IDC_FORCECHECK),FALSE);
			break;

		case PROFILE_DVDMINUSRW_RESTOV:
		case PROFILE_DVDMINUSRW_SEQ:
			m_MethodCombo.AddString(lngGetString(FORMATMODE_QUICK));
			m_MethodCombo.AddString(lngGetString(FORMATMODE_FULL));
			m_MethodCombo.AddString(lngGetString(BLANKMODE_FULL));
			m_MethodCombo.AddString(lngGetString(BLANKMODE_MINIMAL));
			m_MethodCombo.AddString(lngGetString(BLANKMODE_UNCLOSE));
			m_MethodCombo.AddString(lngGetString(BLANKMODE_SESSION));

			m_MethodCombo.SetItemData(0,CCore2::ERASE_FORMAT_QUICK);
			m_MethodCombo.SetItemData(1,CCore2::ERASE_FORMAT_FULL);
			m_MethodCombo.SetItemData(2,CCore2::ERASE_BLANK_FULL);
			m_MethodCombo.SetItemData(3,CCore2::ERASE_BLANK_MINIMAL);
			m_MethodCombo.SetItemData(4,CCore2::ERASE_BLANK_UNCLOSE);
			m_MethodCombo.SetItemData(5,CCore2::ERASE_BLANK_SESSION);

			m_MethodCombo.SetCurSel(2);
			bSupportedProfile = true;

			// Disable simulation (not supported for DVD-RW media).
			::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),FALSE);

			// Disable force erase.
			::EnableWindow(GetDlgItem(IDC_FORCECHECK),FALSE);
			break;

		case PROFILE_NONE:
			m_MethodCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
			m_MethodCombo.SetCurSel(0);

			lstrcat(szBuffer,_T(" "));
			lstrcat(szBuffer,lngGetString(MEDIA_INSERT));
			break;

		default:
			m_MethodCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
			m_MethodCombo.SetCurSel(0);

			lstrcat(szBuffer,_T(" "));
			lstrcat(szBuffer,lngGetString(MEDIA_UNSUPPORTED));
			break;
	}

	::SetWindowText(GetDlgItem(IDC_RECORDERSTATIC),szBuffer);

	// Maximum write speed.
	m_SpeedCombo.ResetContent();
	m_SpeedCombo.AddString(lngGetString(MISC_MAXIMUM));
	m_SpeedCombo.SetItemData(0,0xFFFFFFFF);
	m_SpeedCombo.SetCurSel(0);

	// General
	if (bSupportedProfile)
	{
		m_MethodCombo.EnableWindow(TRUE);
		m_SpeedCombo.EnableWindow(TRUE);
		::EnableWindow(GetDlgItem(IDOK),TRUE);
		::EnableWindow(GetDlgItem(IDC_METHODSTATIC),TRUE);
		::EnableWindow(GetDlgItem(IDC_SPEEDSTATIC),TRUE);

		// Write speeds.
		std::vector<unsigned int> Speeds;
		if (!g_Core2.GetMediaWriteSpeeds(&m_CurDevice,Speeds))
			return false;

		for (unsigned int i = 0; i < Speeds.size(); i++)
		{
			lsprintf(szBuffer,_T("%gx"),GetDispSpeed(usProfile,Speeds[i]));
			m_SpeedCombo.AddString(szBuffer);
			m_SpeedCombo.SetItemData(m_SpeedCombo.GetCount() - 1,Speeds[i]);
		}
	}
	else
	{
		m_MethodCombo.EnableWindow(FALSE);
		m_SpeedCombo.EnableWindow(FALSE);
		::EnableWindow(GetDlgItem(IDOK),FALSE);
		::EnableWindow(GetDlgItem(IDC_METHODSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_SPEEDSTATIC),FALSE);
	}

	return true;
}

void CEraseDlg::InitRefreshButton()
{
	m_hRefreshIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_REFRESHICON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

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

void CEraseDlg::CheckRecorderMedia()
{
	if (!InitRecorderMedia())
	{
		// Method.
		m_MethodCombo.ResetContent();
		m_MethodCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
		m_MethodCombo.SetCurSel(0);

		// Write speed.
		m_SpeedCombo.ResetContent();
		m_SpeedCombo.AddString(lngGetString(MISC_MAXIMUM));
		m_SpeedCombo.SetCurSel(0);

		m_MethodCombo.EnableWindow(FALSE);
		m_SpeedCombo.EnableWindow(FALSE);
		::EnableWindow(GetDlgItem(IDOK),FALSE);
		::EnableWindow(GetDlgItem(IDC_METHODSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_SPEEDSTATIC),FALSE);
	}
}

LRESULT CEraseDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
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
	m_MethodCombo = GetDlgItem(IDC_METHODCOMBO);
	m_SpeedCombo = GetDlgItem(IDC_SPEEDCOMBO);

	// Set the refresh button icon.
	InitRefreshButton();

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
		m_RecorderCombo.SetCurSel(0);

		// Disable the OK button.
		::EnableWindow(GetDlgItem(IDOK),false);
	}
	else
	{
		m_RecorderCombo.SetCurSel(0);

		BOOL bDummy;
		OnRecorderChange(NULL,NULL,NULL,bDummy);
	}

	// Setup the default settings.
	m_MethodCombo.SetCurSel(g_EraseSettings.m_iMode);
	CheckDlgButton(IDC_FORCECHECK,g_EraseSettings.m_bForce);
	CheckDlgButton(IDC_EJECTCHECK,g_EraseSettings.m_bEject);
	CheckDlgButton(IDC_SIMULATECHECK,g_EraseSettings.m_bSimulate);

	// Translate the window.
	Translate();

	// Initialize the drive media.
	CheckRecorderMedia();

	return TRUE;
}

LRESULT CEraseDlg::OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_CurDevice.Close();
	return TRUE;
}

/*LRESULT CEraseDlg::OnDeviceChange(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam; 

	if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)
	{
		// See if we are dealing with a CD-unit.
		if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;

			if (lpdbv->dbcv_flags & DBTF_MEDIA)
			{
				// Find the drive letter from the unit mask.
				unsigned int uiMask = lpdbv->dbcv_unitmask;
				char c;

				for (c = 0; c < 26; c++)
				{
					if (uiMask & 0x01)
						break;

					uiMask = uiMask >> 1;
				} 

				// Check if the current drive has changed.
				tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(
					m_RecorderCombo.GetItemData(m_RecorderCombo.GetCurSel()));

				if (pDeviceInfo->Address.m_cDriveLetter == (c + 'A'))
					CheckRecorderMedia();
			}
		}
	}

	return TRUE;
}*/

LRESULT CEraseDlg::OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_CurDevice.IsOpen())
	{
		// Check for media change.
		if (g_Core2.CheckMediaChange(&m_CurDevice))
			CheckRecorderMedia();
	}

	return TRUE;
}

LRESULT CEraseDlg::OnRecorderChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Enable/disable the simulation checkbox depending on if the selected recorder
	// supports that operation.
	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(
		m_RecorderCombo.GetItemData(m_RecorderCombo.GetCurSel()));

	// Open the new device.
	m_CurDevice.Close();
	if (!m_CurDevice.Open(&pDeviceInfo->Address))
		return 0;

	// Kill any already running timers.
	::KillTimer(m_hWnd,TIMER_ID);
	::SetTimer(m_hWnd,TIMER_ID,TIMER_INTERVAL,NULL);

	// Initialize the drive media.
	CheckRecorderMedia();

	bHandled = false;
	return 0;
}

LRESULT CEraseDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Remember the configuration.
	g_EraseSettings.m_iMode = (int)m_MethodCombo.GetItemData(m_MethodCombo.GetCurSel());
	g_EraseSettings.m_bForce = IsDlgButtonChecked(IDC_FORCECHECK) == TRUE;
	g_EraseSettings.m_bEject = IsDlgButtonChecked(IDC_EJECTCHECK) == TRUE;
	g_EraseSettings.m_bSimulate = IsDlgButtonChecked(IDC_SIMULATECHECK) == TRUE;

	// For internal use only.
	g_EraseSettings.m_iRecorder = m_RecorderCombo.GetItemData(m_RecorderCombo.GetCurSel());
	g_EraseSettings.m_uiSpeed = (unsigned int)m_SpeedCombo.GetItemData(m_SpeedCombo.GetCurSel());

	EndDialog(wID);
	return FALSE;
}

LRESULT CEraseDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}

LRESULT CEraseDlg::OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/erase_disc.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
	return 0;
}

LRESULT CEraseDlg::OnRefresh(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Initialize the drive media.
	CheckRecorderMedia();

	return 0;
}
