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
#include <ckmmc/util.hh>
#include "InfraRecorder.h"
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"
#include "CtrlMessages.h"
#include "cdrtools_parse_strings.hh"
#include "WaitDlg.h"
#include "scsi.hh"
#include "TransUtil.h"
#include "DeviceUtil.h"
#include "core2_util.hh"
#include "Version.h"
#include "VisualStyles.h"
#include "InfoDlg.h"
#include "CopyDiscGeneralPage.h"

CCopyDiscGeneralPage::CCopyDiscGeneralPage()
{
	m_uiParentTitleLen = 0;
	m_hRefreshIcon = NULL;
	m_hRefreshImageList = NULL;

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

	// Specify which controls that are located beloe the burn on the fly warning
	// label.
	m_iCtrlsBelowOnFly.push_back(IDC_CLONECHECK);
	m_iCtrlsBelowOnFly.push_back(IDC_BEVELSTATIC5);
	m_iCtrlsBelowOnFly.push_back(IDC_WRITESPEEDSTATIC);
	m_iCtrlsBelowOnFly.push_back(IDC_WRITESPEEDCOMBO);
	m_iCtrlsBelowOnFly.push_back(IDC_WRITEMETHODSTATIC);
	m_iCtrlsBelowOnFly.push_back(IDC_WRITEMETHODCOMBO);
	m_iCtrlsBelowOnFly.push_back(IDC_BEVELSTATIC4);
	m_iCtrlsBelowOnFly.push_back(IDC_EJECTCHECK);
	m_iCtrlsBelowOnFly.push_back(IDC_SIMULATECHECK);
	m_iCtrlsBelowOnFly.push_back(IDC_BUPCHECK);
	m_iCtrlsBelowOnFly.push_back(IDC_PADCHECK);
	m_iCtrlsBelowOnFly.push_back(IDC_FIXATECHECK);
}

CCopyDiscGeneralPage::~CCopyDiscGeneralPage()
{
	if (m_hRefreshImageList != NULL)
		ImageList_Destroy(m_hRefreshImageList);

	if (m_hRefreshIcon != NULL)
		DestroyIcon(m_hRefreshIcon);
}

bool CCopyDiscGeneralPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a burn translation section.
	if (!pLng->EnterSection(_T("burn")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	int iMaxStaticRight = 0;

	if (pLng->GetValuePtr(IDC_WRITESPEEDSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_WRITESPEEDSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_WRITESPEEDSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLng->GetValuePtr(IDC_WRITEMETHODSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_WRITEMETHODSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_WRITEMETHODSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLng->GetValuePtr(IDC_EJECTCHECK,szStrValue))
		SetDlgItemText(IDC_EJECTCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_SIMULATECHECK,szStrValue))
		SetDlgItemText(IDC_SIMULATECHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_BUPCHECK,szStrValue))
		SetDlgItemText(IDC_BUPCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_PADCHECK,szStrValue))
		SetDlgItemText(IDC_PADCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_FIXATECHECK,szStrValue))
		SetDlgItemText(IDC_FIXATECHECK,szStrValue);

	// Make sure that there is a copy translation section.
	if (!pLng->EnterSection(_T("copy")))
		return false;

	if (pLng->GetValuePtr(IDC_SOURCESTATIC,szStrValue))
		SetDlgItemText(IDC_SOURCESTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_TARGETSTATIC,szStrValue))
		SetDlgItemText(IDC_TARGETSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_ONFLYCHECK,szStrValue))
		SetDlgItemText(IDC_ONFLYCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_ONFLYSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_ONFLYSTATIC,szStrValue);

		int iDiff = UpdateStaticHeight(m_hWnd,IDC_ONFLYSTATIC,szStrValue);
		if (iDiff > 0)
		{
			for (unsigned int i = 0; i < m_iCtrlsBelowOnFly.size(); i++)
			{
				RECT rcControl = { 0 };
				::GetWindowRect(GetDlgItem(m_iCtrlsBelowOnFly[i]),&rcControl);
				ScreenToClient(&rcControl);

				::MoveWindow(GetDlgItem(m_iCtrlsBelowOnFly[i]),rcControl.left,
					rcControl.top + iDiff,rcControl.right - rcControl.left,
					rcControl.bottom - rcControl.top,TRUE);
			}

			/*RECT rcWindow = { 0 };
			GetWindowRect(&rcWindow);

			MoveWindow(rcWindow.left,
				rcWindow.top,rcWindow.right - rcWindow.left,
				rcWindow.bottom - rcWindow.top + iDiff,TRUE);*/
		}
	}
	if (pLng->GetValuePtr(IDC_CLONECHECK,szStrValue))
		SetDlgItemText(IDC_CLONECHECK,szStrValue);

	// Make sure that the edit controls are not in the way of the statics.
	if (iMaxStaticRight > 75)
	{
		UpdateEditPos(m_hWnd,IDC_WRITESPEEDCOMBO,iMaxStaticRight,true);
		UpdateEditPos(m_hWnd,IDC_WRITEMETHODCOMBO,iMaxStaticRight,true);
	}


	return true;
}

bool CCopyDiscGeneralPage::InitRecorderMedia()
{
	if (m_uiParentTitleLen == 0)
		m_uiParentTitleLen = GetParentWindow(this).GetWindowTextLength();

	TCHAR szBuffer[256];
	GetParentWindow(this).GetWindowText(szBuffer,m_uiParentTitleLen + 1);

	ckmmc::Device &Device =
		*reinterpret_cast<ckmmc::Device *>(m_TargetCombo.GetItemData(
										   m_TargetCombo.GetCurSel()));

	Device.refresh();

	// Get current profile.
	ckmmc::Device::Profile Profile = Device.profile();

	// Note: On CD-R/RW media the Test Write bit is valid only for Write Type
	// 1 or 2 (Track at Once or Session at Once). On DVD-R media, the Test
	// Write bit is valid only for Write Type 0 or 2 (Incremental or
	// Disc-at-once). This is completely disregarded at the moment.
	bool bSupportedProfile = false;
	switch (Profile)
	{
		case ckmmc::Device::ckPROFILE_DVDRAM:
		case ckmmc::Device::ckPROFILE_DVDPLUSRW:
		case ckmmc::Device::ckPROFILE_DVDPLUSRW_DL:
		case ckmmc::Device::ckPROFILE_DVDMINUSRW_RESTOV:
		case ckmmc::Device::ckPROFILE_DVDMINUSRW_SEQ:
		case ckmmc::Device::ckPROFILE_DVDPLUSR:
		case ckmmc::Device::ckPROFILE_DVDPLUSR_DL:
			// Not sure about these:
		case ckmmc::Device::ckPROFILE_BDROM:
		case ckmmc::Device::ckPROFILE_BDR_SRM:
		case ckmmc::Device::ckPROFILE_BDR_RRM:
		case ckmmc::Device::ckPROFILE_BDRE:
		case ckmmc::Device::ckPROFILE_HDDVDROM:
		case ckmmc::Device::ckPROFILE_HDDVDR:
		case ckmmc::Device::ckPROFILE_HDDVDRAM:
			bSupportedProfile = true;

			// Disable simulation (not supported for DVD+RW media).
			::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),FALSE);
			break;

		case ckmmc::Device::ckPROFILE_CDR:
		case ckmmc::Device::ckPROFILE_CDRW:
		case ckmmc::Device::ckPROFILE_DVDMINUSR_SEQ:
		case ckmmc::Device::ckPROFILE_DVDMINUSR_DL_SEQ:
		case ckmmc::Device::ckPROFILE_DVDMINUSR_DL_JUMP:
			bSupportedProfile = true;

			// Enable simulation if supported by recorder.
			::EnableWindow(GetDlgItem(IDC_SIMULATECHECK),
						   Device.support(ckmmc::Device::ckDEVICE_TEST_WRITE));
			break;

		case ckmmc::Device::ckPROFILE_NONE:
			lstrcat(szBuffer,_T(" "));
			lstrcat(szBuffer,lngGetString(MEDIA_INSERTBLANK));
			break;

		default:
			lstrcat(szBuffer,_T(" "));
			lstrcat(szBuffer,lngGetString(MEDIA_UNSUPPORTED));
			break;
	}

	GetParentWindow(this).SetWindowText(szBuffer);

	if (bSupportedProfile)
	{
		// Maximum write speed.
		m_WriteSpeedCombo.ResetContent();
		m_WriteSpeedCombo.AddString(lngGetString(MISC_MAXIMUM));
		m_WriteSpeedCombo.SetItemData(0,static_cast<DWORD_PTR>(-1));
		m_WriteSpeedCombo.SetCurSel(0);

		// Other supported write speeds.
		const std::vector<ckcore::tuint32> &WriteSpeeds = Device.write_speeds();
		std::vector<ckcore::tuint32>::const_iterator it;
		for (it = WriteSpeeds.begin(); it != WriteSpeeds.end(); it++)
		{
			m_WriteSpeedCombo.AddString(ckmmc::util::sec_to_disp_speed(*it,Profile).c_str());
			m_WriteSpeedCombo.SetItemData(m_WriteSpeedCombo.GetCount() - 1,
										  static_cast<DWORD_PTR>(ckmmc::util::sec_to_human_speed(*it,
																 ckmmc::Device::ckPROFILE_CDR)));
		}

		// Write modes.
		m_WriteMethodCombo.ResetContent();

		if (Device.support(ckmmc::Device::ckWM_SAO))
			m_WriteMethodCombo.AddString(lngGetString(WRITEMODE_SAO));
		if (Device.support(ckmmc::Device::ckWM_TAO))
		{
			m_WriteMethodCombo.AddString(lngGetString(WRITEMODE_TAO));
			m_WriteMethodCombo.AddString(lngGetString(WRITEMODE_TAONOPREGAP));
		}
		if (Device.support(ckmmc::Device::ckWM_RAW96R))
			m_WriteMethodCombo.AddString(lngGetString(WRITEMODE_RAW96R));
		if (Device.support(ckmmc::Device::ckWM_RAW16))
			m_WriteMethodCombo.AddString(lngGetString(WRITEMODE_RAW16));
		if (Device.support(ckmmc::Device::ckWM_RAW96P))
			m_WriteMethodCombo.AddString(lngGetString(WRITEMODE_RAW96P));

		m_WriteMethodCombo.SetCurSel(0);
	}
	else
	{
		return false;
	}

	return true;
}

void CCopyDiscGeneralPage::InitRefreshButton()
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

void CCopyDiscGeneralPage::CheckRecorderMedia()
{
	bool bMediaInit = InitRecorderMedia();
	if (!bMediaInit)
	{
		// Write speed.
		m_WriteSpeedCombo.ResetContent();
		m_WriteSpeedCombo.AddString(lngGetString(MISC_MAXIMUM));
		m_WriteSpeedCombo.SetCurSel(0);

		// Write method.
		m_WriteMethodCombo.ResetContent();
		m_WriteMethodCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
		m_WriteMethodCombo.SetCurSel(0);
	}

	m_WriteSpeedCombo.EnableWindow(bMediaInit);
	m_WriteMethodCombo.EnableWindow(bMediaInit);
	::EnableWindow(::GetDlgItem(GetParent(),IDOK),bMediaInit);
	::EnableWindow(GetDlgItem(IDC_WRITESPEEDSTATIC),bMediaInit);
	::EnableWindow(GetDlgItem(IDC_WRITEMETHODSTATIC),bMediaInit);
}

bool CCopyDiscGeneralPage::OnApply()
{
	// Remember the configuration.
	g_CopyDiscSettings.m_bOnFly = IsDlgButtonChecked(IDC_ONFLYCHECK) == TRUE;
	g_CopyDiscSettings.m_bClone = IsDlgButtonChecked(IDC_CLONECHECK) == TRUE;
	g_BurnImageSettings.m_bEject = IsDlgButtonChecked(IDC_EJECTCHECK) == TRUE;
	g_BurnImageSettings.m_bSimulate = IsDlgButtonChecked(IDC_SIMULATECHECK) == TRUE;
	g_BurnImageSettings.m_bBUP = IsDlgButtonChecked(IDC_BUPCHECK) == TRUE;
	g_BurnImageSettings.m_bPadTracks = IsDlgButtonChecked(IDC_PADCHECK) == TRUE;
	g_BurnImageSettings.m_bFixate = IsDlgButtonChecked(IDC_FIXATECHECK) == TRUE;

	ckmmc::Device *pSrcDevice =
		reinterpret_cast<ckmmc::Device *>(m_SourceCombo.GetItemData(
										  m_SourceCombo.GetCurSel()));

	if (g_CopyDiscSettings.m_bOnFly && !AnalyzeDriveMedia(*pSrcDevice))
	{
		MessageBox(_T("Unable to get source drive media information. Can not continue."),lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
		return false;
	}

	// For internal use only.
	g_CopyDiscSettings.m_pSource =
		reinterpret_cast<ckmmc::Device *>(m_SourceCombo.GetItemData(
										  m_SourceCombo.GetCurSel()));
	g_CopyDiscSettings.m_pTarget =
		reinterpret_cast<ckmmc::Device *>(m_TargetCombo.GetItemData(
										  m_TargetCombo.GetCurSel()));

	g_BurnImageSettings.m_uiWriteSpeed = m_WriteSpeedCombo.GetItemData(m_WriteSpeedCombo.GetCurSel());
	
	TCHAR szBuffer[32];
	GetDlgItemText(IDC_WRITEMETHODCOMBO,szBuffer,32);

	if (!lstrcmp(szBuffer,lngGetString(WRITEMODE_SAO)))
		g_BurnImageSettings.m_iWriteMethod = WRITEMETHOD_SAO;
	else if (!lstrcmp(szBuffer,lngGetString(WRITEMODE_TAO)))
		g_BurnImageSettings.m_iWriteMethod = WRITEMETHOD_TAO;
	else if (!lstrcmp(szBuffer,lngGetString(WRITEMODE_TAONOPREGAP)))
		g_BurnImageSettings.m_iWriteMethod = WRITEMETHOD_TAONOPREGAP;
	else if (!lstrcmp(szBuffer,lngGetString(WRITEMODE_RAW96R)))
		g_BurnImageSettings.m_iWriteMethod = WRITEMETHOD_RAW96R;
	else if (!lstrcmp(szBuffer,lngGetString(WRITEMODE_RAW16)))
		g_BurnImageSettings.m_iWriteMethod = WRITEMETHOD_RAW16;
	else if (!lstrcmp(szBuffer,lngGetString(WRITEMODE_RAW96P)))
		g_BurnImageSettings.m_iWriteMethod = WRITEMETHOD_RAW96P;

	g_BurnImageSettings.m_lNumCopies = 1;
	return true;
}

void CCopyDiscGeneralPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/copy_data_disc.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CCopyDiscGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_SourceCombo = GetDlgItem(IDC_SOURCECOMBO);
	m_TargetCombo = GetDlgItem(IDC_TARGETCOMBO);
	m_WriteSpeedCombo = GetDlgItem(IDC_WRITESPEEDCOMBO);
	m_WriteMethodCombo = GetDlgItem(IDC_WRITEMETHODCOMBO);

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
		m_SourceCombo.EnableWindow(false);
	}

	m_SourceCombo.SetCurSel(0);

	// Target combo box.
	for (it = g_DeviceManager.devices().begin(); it !=
		g_DeviceManager.devices().end(); it++)
	{
		ckmmc::Device *pDevice = *it;

		// We only want to add recorder to the list.
		if (!pDevice->recorder())
			continue;

		m_TargetCombo.AddString(NDeviceUtil::GetDeviceName(*pDevice).c_str());
		m_TargetCombo.SetItemData(m_TargetCombo.GetCount() - 1,
								  reinterpret_cast<DWORD_PTR>(pDevice));
	}

	if (m_TargetCombo.GetCount() == 0)
	{
		m_TargetCombo.AddString(lngGetString(FAILURE_NORECORDERS));
		m_TargetCombo.EnableWindow(false);
		m_TargetCombo.SetCurSel(0);

		m_WriteSpeedCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
		m_WriteSpeedCombo.EnableWindow(false);
		m_WriteSpeedCombo.SetCurSel(0);

		m_WriteMethodCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
		m_WriteMethodCombo.EnableWindow(false);
		m_WriteMethodCombo.SetCurSel(0);

		// Disable the OK button.
		::EnableWindow(::GetDlgItem(GetParent(),IDOK),false);
	}
	else
	{
		m_TargetCombo.SetCurSel(0);

		BOOL bDummy;
		OnTargetChange(NULL,NULL,NULL,bDummy);
	}

	// Setup the default settings.
	CheckDlgButton(IDC_ONFLYCHECK,g_CopyDiscSettings.m_bOnFly);
	CheckDlgButton(IDC_CLONECHECK,g_CopyDiscSettings.m_bClone);
		::SendMessage(GetParent(),WM_SETCLONEMODE,g_CopyDiscSettings.m_bClone,0);
	CheckDlgButton(IDC_EJECTCHECK,g_BurnImageSettings.m_bEject);
	CheckDlgButton(IDC_SIMULATECHECK,g_BurnImageSettings.m_bSimulate);
	CheckDlgButton(IDC_BUPCHECK,g_BurnImageSettings.m_bBUP);
	CheckDlgButton(IDC_PADCHECK,g_BurnImageSettings.m_bPadTracks);
	CheckDlgButton(IDC_FIXATECHECK,g_BurnImageSettings.m_bFixate);

	BOOL bDummy;
	OnFlyCheck(NULL,NULL,NULL,bDummy);

	if (::IsWindowEnabled(::GetDlgItem(GetParent(),IDOK)))
		OnCloneCheck(NULL,NULL,NULL,bDummy);

	// Make sure that the same device is not selected as both target and source.
	OnSourceChange(NULL,NULL,NULL,bDummy);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CCopyDiscGeneralPage::OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	ckmmc::Device *pDstDevice =
		reinterpret_cast<ckmmc::Device *>(m_TargetCombo.GetItemData(
										  m_TargetCombo.GetCurSel()));

	// Check for media change.
	if (g_Core2.CheckMediaChange(*pDstDevice))
		CheckRecorderMedia();

	return TRUE;
}

unsigned long CCopyDiscGeneralPage::MSFToLBA(unsigned long ulMin,unsigned long ulSec,
											 unsigned long ulFrame)
{
	return ((ulMin * 60 * 75) + (ulSec * 75) + ulFrame - 150);
}

bool CCopyDiscGeneralPage::AnalyzeDriveMedia(ckmmc::Device &Device)
{
	// Rescan the bus.
	CWaitDlg WaitDlg;
	WaitDlg.Create(m_hWnd);
	WaitDlg.ShowWindow(SW_SHOW);
		// Initialize device (detect drive letter, open handle, count tracks).
		WaitDlg.SetMessage(lngGetString(INIT_DEVICECD));

		TCHAR szDriveLetter[3];
		szDriveLetter[0] = Device.address().device_[0];
		szDriveLetter[1] = ':';
		szDriveLetter[2] = '\0';

		// Get the allocated space on the source drive.
		ULARGE_INTEGER uiAvailable;
		ULARGE_INTEGER uiTotal;
		ULARGE_INTEGER uiFree;
		uiAvailable.QuadPart = 0;
		uiTotal.QuadPart = 0;
		uiFree.QuadPart = 0;

		if (GetDiskFreeSpaceEx(szDriveLetter,&uiAvailable,&uiTotal,&uiFree))
			g_CopyDiscSettings.m_uiSourceSize = uiTotal.QuadPart;
	WaitDlg.DestroyWindow();

	return true;
}

LRESULT CCopyDiscGeneralPage::OnSourceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	ckmmc::Device *pSrcDevice =
		reinterpret_cast<ckmmc::Device *>(m_SourceCombo.GetItemData(
										  m_SourceCombo.GetCurSel()));
	ckmmc::Device *pDstDevice =
		reinterpret_cast<ckmmc::Device *>(m_TargetCombo.GetItemData(
										  m_TargetCombo.GetCurSel()));

	::SendMessage(GetParent(),WM_SETDEVICE,1,reinterpret_cast<LPARAM>(pSrcDevice));

	// Disable/enable the record on the fly button.
	::EnableWindow(GetDlgItem(IDC_ONFLYCHECK),pSrcDevice != pDstDevice);
	if (pSrcDevice == pDstDevice)
		CheckDlgButton(IDC_ONFLYCHECK,false);

	BOOL bDummy;
	OnFlyCheck(NULL,NULL,NULL,bDummy);

	bHandled = false;
	return 0;
}

LRESULT CCopyDiscGeneralPage::OnTargetChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// If you insert a CD and immediately open this dialog, the application
	// will be unresponsive for a few seconds while the device is being opened
	// [see m_CurDevice.Open() below]. It would be best to avoid that pause
	// altogether, but at least we display the hourglass cursor here, to let
	// the user know he should be patient, otherwise he might try to click all
	// over the place and wonder why nothing happens.
	CWaitCursor WaitCursor;		// This displays the hourglass cursor.

	ckmmc::Device *pSrcDevice =
		reinterpret_cast<ckmmc::Device *>(m_SourceCombo.GetItemData(
										  m_SourceCombo.GetCurSel()));
	ckmmc::Device *pDstDevice =
		reinterpret_cast<ckmmc::Device *>(m_TargetCombo.GetItemData(
										  m_TargetCombo.GetCurSel()));

	::SendMessage(GetParent(),WM_SETDEVICE,0,reinterpret_cast<LPARAM>(pDstDevice));

	// Kill any already running timers.
	::KillTimer(m_hWnd,TIMER_ID);
	::SetTimer(m_hWnd,TIMER_ID,TIMER_INTERVAL,NULL);

	// Initialize the drive media.
	CheckRecorderMedia();

	// General.
	::EnableWindow(GetDlgItem(IDC_BUPCHECK),
				   pDstDevice->support(ckmmc::Device::ckDEVICE_BUP));

	// Suggest an appropriate write method if necessary.
	BOOL bDummy;
	if (IsDlgButtonChecked(IDC_ONFLYCHECK) == FALSE)
		OnCloneCheck(0,0,NULL,bDummy);
	else
		m_WriteMethodCombo.SetCurSel(0);

	// Disable/enable the record on the fly button.
	::EnableWindow(GetDlgItem(IDC_ONFLYCHECK),pSrcDevice != pDstDevice);
	if (pSrcDevice == pDstDevice)
		CheckDlgButton(IDC_ONFLYCHECK,false);

	OnFlyCheck(NULL,NULL,NULL,bDummy);

	bHandled = false;
	return 0;
}

LRESULT CCopyDiscGeneralPage::OnRefresh(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Initialize the drive media.
	CheckRecorderMedia();

	return 0;
}

LRESULT CCopyDiscGeneralPage::OnFlyCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Enable/disable the clone disc check box.
	if (IsDlgButtonChecked(IDC_ONFLYCHECK) == FALSE)
	{
		::EnableWindow(GetDlgItem(IDC_CLONECHECK),TRUE);
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_CLONECHECK),FALSE);

		// Select the top write method (usually SAO).
		m_WriteMethodCombo.SetCurSel(0);

		// Disable and uncheck the clone check box.
		CheckDlgButton(IDC_CLONECHECK,FALSE);
		::SendMessage(GetParent(),WM_SETCLONEMODE,FALSE,0);
	}

	bHandled = false;
	return 0;
}

LRESULT CCopyDiscGeneralPage::OnCloneCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Try to select a recommended write method.
	if (IsDlgButtonChecked(IDC_CLONECHECK))
	{
		::SendMessage(GetParent(),WM_SETCLONEMODE,TRUE,0);

		ckmmc::Device &DstDevice =
			*reinterpret_cast<ckmmc::Device *>(m_TargetCombo.GetItemData(
											   m_TargetCombo.GetCurSel()));

		// Check for supported write modes. Raw96r has the highest priority, if
		// that's not support the raw16 mode is recommended, if none of the mentioned
		// write modes are supported a warning message is displayed.
		int uiStrID = -1;

		if (DstDevice.support(ckmmc::Device::ckWM_RAW96R))
			uiStrID = WRITEMODE_RAW96R;
		else if (DstDevice.support(ckmmc::Device::ckWM_RAW16))
			uiStrID = WRITEMODE_RAW16;

		const TCHAR *szStr = lngGetString(uiStrID);

		if (uiStrID != -1)
		{
			TCHAR szItemText[128];

			for (int i = 0; i < m_WriteMethodCombo.GetCount(); i++)
			{
				if (m_WriteMethodCombo.GetLBTextLen(i) >= (sizeof(szItemText) / sizeof(TCHAR)))
					continue;

				m_WriteMethodCombo.GetLBText(i,szItemText);
				if (!lstrcmp(szItemText,szStr))
				{
					m_WriteMethodCombo.SetCurSel(i);
					break;
				}
			}
		}
		else
		{
			// No good raw writing mode found.
			MessageBox(lngGetString(WARNING_CLONEWRITEMETHOD),lngGetString(GENERAL_WARNING),
				MB_OK | MB_ICONWARNING);
		}
	}
	else
	{
		::SendMessage(GetParent(),WM_SETCLONEMODE,FALSE,0);

		m_WriteMethodCombo.SetCurSel(0);
	}

	bHandled = false;
	return 0;
}

LRESULT CCopyDiscGeneralPage::OnFixateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!IsDlgButtonChecked(IDC_FIXATECHECK))
	{
		// Display warning message.
		if (g_GlobalSettings.m_bFixateWarning)
		{
			CInfoDlg InfoDlg(&g_GlobalSettings.m_bFixateWarning,lngGetString(WARNING_NOFIXATION),INFODLG_ICONWARNING);
			if (InfoDlg.DoModal() == IDCANCEL)
				CheckDlgButton(IDC_FIXATECHECK,true);
		}
	}

	return 0;
}