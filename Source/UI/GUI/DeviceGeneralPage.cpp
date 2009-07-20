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
#include <ckmmc/util.hh>
#include "DeviceUtil.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Settings.h"
#include "InfoDlg.h"
#include "DeviceGeneralPage.h"

CDeviceGeneralPage::CDeviceGeneralPage(ckmmc::Device &Device) :
	m_hIcon(NULL),m_Device(Device)
{
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
}

CDeviceGeneralPage::~CDeviceGeneralPage()
{
	if (m_hIcon != NULL)
		DestroyIcon(m_hIcon);
}

bool CDeviceGeneralPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a device translation section.
	if (!pLng->EnterSection(_T("device")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDC_TYPELABELSTATIC,szStrValue))
		SetDlgItemText(IDC_TYPELABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_LOCATIONLABELSTATIC,szStrValue))
		SetDlgItemText(IDC_LOCATIONLABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_BUFFERLABELSTATIC,szStrValue))
		SetDlgItemText(IDC_BUFFERLABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_MAXREADLABELSTATIC,szStrValue))
		SetDlgItemText(IDC_MAXREADLABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_MAXWRITELABELSTATIC,szStrValue))
		SetDlgItemText(IDC_MAXWRITELABELSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_READSTATIC,szStrValue))
		SetDlgItemText(IDC_READSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_WRITESTATIC,szStrValue))
		SetDlgItemText(IDC_WRITESTATIC,szStrValue);

	return true;
}

void CDeviceGeneralPage::PrintDeviceType(ckmmc::Device &Device)
{
	if (Device.support(ckmmc::Device::ckDEVICE_WRITE_CDR) ||
		Device.support(ckmmc::Device::ckDEVICE_WRITE_CDRW))
	{
		if (Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDR) ||
			Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDRAM))
		{
			SetDlgItemText(IDC_TYPESTATIC,lngGetString(DEVICETYPE_DVDRECORDER));
		}
		else
		{
			SetDlgItemText(IDC_TYPESTATIC,lngGetString(DEVICETYPE_CDRECORDER));
		}
	}
	else
	{
		if (Device.support(ckmmc::Device::ckDEVICE_READ_DVDROM) ||
			Device.support(ckmmc::Device::ckDEVICE_READ_DVDR) ||
			Device.support(ckmmc::Device::ckDEVICE_READ_DVDRAM))
		{
			SetDlgItemText(IDC_TYPESTATIC,lngGetString(DEVICETYPE_DVDREADER));
		}
		else
		{
			SetDlgItemText(IDC_TYPESTATIC,lngGetString(DEVICETYPE_CDREADER));
		}
	}
}

LRESULT CDeviceGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	ATLASSERT(m_pDevice != 0);

	// Create the icon.
	HINSTANCE hInstance = LoadLibrary(_T("shell32.dll"));
		m_hIcon = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(12),IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	FreeLibrary(hInstance);

	SendMessage(GetDlgItem(IDC_ICONSTATIC),STM_SETICON,(WPARAM)m_hIcon,0L);

	// Display the information.
	SetDlgItemText(IDC_NAMESTATIC,NDeviceUtil::GetDeviceName(m_Device).c_str());

	// Type.
	PrintDeviceType(m_Device);

	// Location.
	TCHAR szBuffer[128];
	lsnprintf_s(szBuffer,128,lngGetString(PROPERTIES_DEVICELOC),m_Device.address().bus_,
				m_Device.address().target_,m_Device.address().lun_);
	SetDlgItemText(IDC_LOCATIONSTATIC,szBuffer);

	// Buffer size.
	lsprintf(szBuffer,_T("%d kB"),m_Device.property(ckmmc::Device::ckPROP_BUFFER_SIZE));
	SetDlgItemText(IDC_BUFFERSTATIC,szBuffer);

	// Max read speed.
	ckcore::tuint32 max_read_speed = m_Device.property(ckmmc::Device::ckPROP_MAX_READ_SPD);
	lsprintf(szBuffer,_T("%d sectors/s (CD: %sx, DVD: %sx)"),
			 max_read_speed,
			 ckmmc::util::sec_to_disp_speed(max_read_speed,
											ckmmc::Device::ckPROFILE_CDROM).c_str(),
			 ckmmc::util::sec_to_disp_speed(max_read_speed,
											ckmmc::Device::ckPROFILE_DVDROM).c_str());
	SetDlgItemText(IDC_MAXREADSTATIC,szBuffer);

	// Max write speed.
	ckcore::tuint32 max_write_speed = m_Device.property(ckmmc::Device::ckPROP_MAX_WRITE_SPD);
	lsprintf(szBuffer,_T("%d sectors/s (CD: %sx, DVD: %sx)"),
			 max_write_speed,
			 ckmmc::util::sec_to_disp_speed(max_write_speed,
											ckmmc::Device::ckPROFILE_CDROM).c_str(),
			 ckmmc::util::sec_to_disp_speed(max_write_speed,
											ckmmc::Device::ckPROFILE_DVDROM).c_str());
	SetDlgItemText(IDC_MAXWRITESTATIC,szBuffer);

	// Read media.
	::SendMessage(GetDlgItem(IDC_READCDRCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_CDR) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READCDRWCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_CDRW) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDRCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_DVDR) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDRAMCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_DVDRAM) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDROMCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_DVDROM) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDPLUSRCHECK),BM_SETCHECK,
		m_Device.support(ckmmc::Device::ckDEVICE_READ_DVDPLUSR) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDPLUSRWCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_DVDPLUSRW) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDPLUSRDLCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_DVDPLUSR_DL) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDPLUSRWDLCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_DVDPLUSRW_DL) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READBDCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_BD) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READHDDVDCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_READ_HDDVD) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);

	// Write media.
	::SendMessage(GetDlgItem(IDC_WRITECDRCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_CDR) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITECDRWCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_CDRW) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDRCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDR) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDRAMCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDRAM) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDPLUSRCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDPLUSR) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDPLUSRWCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDPLUSRW) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDPLUSRDLCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDPLUSR_DL) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDPLUSRWDLCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_DVDPLUSRW_DL) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEBDCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_BD) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEHDDVDCHECK),BM_SETCHECK,
				  m_Device.support(ckmmc::Device::ckDEVICE_WRITE_HDDVD) ?
				  BST_INDETERMINATE : BST_UNCHECKED,0);

	// Translate the window.
	Translate();

	return TRUE;
}
