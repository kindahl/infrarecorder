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
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"
#include "WaitDlg.h"
#include "scsi.hh"
#include "DeviceUtil.h"
#include "core2_info.hh"
#include "LogDlg.h"
#include "InfraRecorder.h"
#include "ImportSessionDlg.h"

CImportSessionDlg::CImportSessionDlg() : m_pSelDevice(NULL),m_pSelTrackData(NULL)
{
}

CImportSessionDlg::~CImportSessionDlg()
{
	// Delete any track data.
	ResetDiscInfo();
}

bool CImportSessionDlg::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is an erase translation section.
	if (!pLng->EnterSection(_T("importsession")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDD_IMPORTSESSIONDLG,szStrValue))			// Title.
		SetWindowText(szStrValue);
	if (pLng->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLng->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLng->GetValuePtr(IDC_DRIVESTATIC,szStrValue))
		SetDlgItemText(IDC_DRIVESTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_USEDSPACELABELSTATIC,szStrValue))
		SetDlgItemText(IDC_USEDSPACELABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_TRACKSTATIC,szStrValue))
		SetDlgItemText(IDC_TRACKSTATIC,szStrValue);

	return true;
}

bool CImportSessionDlg::UpdateDiscInfo(ckmmc::Device &Device)
{
	g_pLogDlg->print_line(_T("CImportSessionDlg::UpdateDiscInfo"));

	unsigned char ucFirstTrackNumber = 0,ucLastTrackNumber = 0;
	std::vector<CCore2TOCTrackDesc> Tracks;

	CCore2Info Info;
	if (Info.ReadTOC(Device,ucFirstTrackNumber,ucLastTrackNumber,Tracks))
	{
		g_pLogDlg->print_line(_T("  First and last disc track number: %d, %d."),
			ucFirstTrackNumber,ucLastTrackNumber);

		// Iterate through all tracks.
		CCore2TrackInfo TrackInfo;

		std::vector<CCore2TOCTrackDesc>::const_iterator itTrack;
		for (itTrack = Tracks.begin(); itTrack != Tracks.end(); itTrack++)
		{
			if (!Info.ReadTrackInformation(Device,CCore2Info::TIT_LBA,itTrack->m_ulTrackAddr,&TrackInfo))
			{
				g_pLogDlg->print_line(_T("  Error: Unable to read information about track %d."),itTrack->m_ucTrackNumber);
				return false;
			}

			// Always assume a post-gap of 150 sectors. This may be a bad idea.
			unsigned long ulTrackSize = TrackInfo.m_ulTrackSize - 150;

			m_TrackData.push_back(new CTrackData(TrackInfo.m_ucDataMode,TrackInfo.m_usSessionNumber,
				TrackInfo.m_usTrackNumber,itTrack->m_ulTrackAddr,ulTrackSize));
		}
	}
	else
	{
		g_pLogDlg->print_line(_T("  Error: Unable to read TOC information to validate selected track."));
		return false;
	}

	return true;
}

void CImportSessionDlg::ResetDiscInfo()
{
	std::vector<CTrackData *>::iterator itTrack;
	for (itTrack = m_TrackData.begin(); itTrack != m_TrackData.end(); itTrack++)
		delete *itTrack;

	m_TrackData.clear();
}

LRESULT CImportSessionDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	m_DeviceCombo = GetDlgItem(IDC_DEVICECOMBO);
	m_TrackCombo = GetDlgItem(IDC_TRACKCOMBO);

	// Device combo box.
	std::vector<ckmmc::Device *>::const_iterator it;
	for (it = g_DeviceManager.devices().begin(); it !=
		g_DeviceManager.devices().end(); it++)
	{
		const ckmmc::Device *pDevice = *it;

		m_DeviceCombo.AddString(NDeviceUtil::GetDeviceName(*pDevice).c_str());
		m_DeviceCombo.SetItemData(m_DeviceCombo.GetCount() - 1,
								  reinterpret_cast<DWORD_PTR>(pDevice));
	}

	if (m_DeviceCombo.GetCount() == 0)
	{
		m_DeviceCombo.AddString(lngGetString(FAILURE_NORECORDERS));
		m_DeviceCombo.EnableWindow(false);

		// Disable the OK button.
		::EnableWindow(GetDlgItem(IDOK),false);
	}

	m_DeviceCombo.SetCurSel(0);

	// Translate the window.
	Translate();

	// Update the information.
	if (m_DeviceCombo.GetCount() > 0)
	{
		BOOL bDymmy;
		OnDeviceChange(NULL,NULL,NULL,bDymmy);
	}

	return TRUE;
}

LRESULT CImportSessionDlg::OnDeviceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	bHandled = false;

	ckmmc::Device *pDevice =
		reinterpret_cast<ckmmc::Device *>(m_DeviceCombo.GetItemData(
										  m_DeviceCombo.GetCurSel()));

	// Rescan the bus.
	CWaitDlg WaitDlg;
	WaitDlg.Create(m_hWnd);
	WaitDlg.ShowWindow(SW_SHOW);
		// Initialize device.
		WaitDlg.SetMessage(lngGetString(INIT_DEVICECD));

		CCore2Info Info;
		CCore2DiscInfo DiscInfo;

		if (Info.ReadDiscInformation(*pDevice,&DiscInfo))
		{
			// UPDATE: This does not seem to work on DVD+RW discs.
			//::EnableWindow(GetDlgItem(IDOK),DiscInfo.m_ucDiscStatus == CCore2DiscInfo::DS_INCOMPLETE);
			::EnableWindow(GetDlgItem(IDOK),TRUE);

			// Space.
			unsigned __int64 uiUsedBytes = 0;
			unsigned __int64 uiFreeBytes = 0;

			if (Info.GetTotalDiscCapacity(*pDevice,uiUsedBytes,uiFreeBytes))
			{
				m_uiAllocatedSize = uiUsedBytes;

				TCHAR szBuffer[64];
				FormatBytes(szBuffer,m_uiAllocatedSize);
				lsprintf(szBuffer + lstrlen(szBuffer),_T(" (%I64d Bytes)"),m_uiAllocatedSize);
				SetDlgItemText(IDC_USEDSPACESTATIC,szBuffer);
			}
			else
			{
				SetDlgItemText(IDC_USEDSPACESTATIC,lngGetString(DISC_UNKNOWN));
			}

			// Delete any  previous track data.
			ResetDiscInfo();
			UpdateDiscInfo(*pDevice);

			m_TrackCombo.ResetContent();
			if (m_TrackData.size() == 0)
			{
				m_TrackCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
				m_TrackCombo.EnableWindow(FALSE);

				::EnableWindow(GetDlgItem(IDOK),FALSE);
			}
			else
			{
				m_TrackCombo.EnableWindow(TRUE);

				TCHAR szBuffer[128];

				std::vector<CTrackData *>::iterator itTrack;
				for (itTrack = m_TrackData.begin(); itTrack != m_TrackData.end(); itTrack++)
				{
					lsnprintf_s(szBuffer,sizeof(szBuffer)/sizeof(TCHAR),_T("%s %u %s %u: %u-%u (%s %u)"),
						lngGetString(MISC_SESSION),(*itTrack)->m_usSessionNumber,
						lngGetString(MISC_TRACK),(*itTrack)->m_usTrackNumber,
						(*itTrack)->m_ulTrackAddr,(*itTrack)->m_ulTrackAddr + (*itTrack)->m_ulTrackLen,
						lngGetString(MISC_MODE),(*itTrack)->m_ucMode);
					m_TrackCombo.AddString(szBuffer);
					m_TrackCombo.SetItemData(m_TrackCombo.GetCount() - 1,(DWORD_PTR)*itTrack);
				}

				::EnableWindow(GetDlgItem(IDOK),TRUE);
			}

			m_TrackCombo.SetCurSel(m_TrackCombo.GetCount() - 1);
		}
		else
		{
			ResetDiscInfo();
			m_TrackCombo.ResetContent();
			m_TrackCombo.AddString(lngGetString(MISC_NOTAVAILABLE));
			m_TrackCombo.SetCurSel(0);
			m_TrackCombo.EnableWindow(FALSE);

			::EnableWindow(GetDlgItem(IDOK),false);
			SetDlgItemText(IDC_USEDSPACESTATIC,lngGetString(DISC_UNKNOWN));
		}

	WaitDlg.DestroyWindow();
	return 0;
}

LRESULT CImportSessionDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	m_pSelDevice =
		reinterpret_cast<ckmmc::Device *>(m_DeviceCombo.GetItemData(
										  m_DeviceCombo.GetCurSel()));
	m_pSelTrackData = (CTrackData *)m_TrackCombo.GetItemData(m_TrackCombo.GetCurSel());

	EndDialog(wID);
	return FALSE;
}

LRESULT CImportSessionDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}