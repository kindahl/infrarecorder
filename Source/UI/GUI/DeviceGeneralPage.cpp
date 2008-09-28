/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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
#include "DeviceGeneralPage.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Settings.h"
#include "InfoDlg.h"

CDeviceGeneralPage::CDeviceGeneralPage()
{
	m_uiDeviceIndex = 0;

	m_hIcon = NULL;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_GENERAL,szStrValue))
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
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a device translation section.
	if (!pLNG->EnterSection(_T("device")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDC_TYPELABELSTATIC,szStrValue))
		SetDlgItemText(IDC_TYPELABELSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_LOCATIONLABELSTATIC,szStrValue))
		SetDlgItemText(IDC_LOCATIONLABELSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_BUFFERLABELSTATIC,szStrValue))
		SetDlgItemText(IDC_BUFFERLABELSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_MAXREADLABELSTATIC,szStrValue))
		SetDlgItemText(IDC_MAXREADLABELSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_MAXWRITELABELSTATIC,szStrValue))
		SetDlgItemText(IDC_MAXWRITELABELSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_READSTATIC,szStrValue))
		SetDlgItemText(IDC_READSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_WRITESTATIC,szStrValue))
		SetDlgItemText(IDC_WRITESTATIC,szStrValue);

	return true;
}

void CDeviceGeneralPage::SetDeviceIndex(UINT_PTR uiDeviceIndex)
{
	m_uiDeviceIndex = uiDeviceIndex;
}

void CDeviceGeneralPage::PrintDeviceType(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap)
{
	if ((pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITECDR) ||
		(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITECDRW))
	{
		if ((pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDR) ||
			(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDRAM))
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
		if ((pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READDVDR) ||
			(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READDVDROM) ||
			(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READDVDRAM))
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
	// Create the icon.
	HINSTANCE hInstance = LoadLibrary(_T("shell32.dll"));
		m_hIcon = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(12),IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	FreeLibrary(hInstance);

	SendMessage(GetDlgItem(IDC_ICONSTATIC),STM_SETICON,(WPARAM)m_hIcon,0L);

	// Display the information.
	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(m_uiDeviceIndex);
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(m_uiDeviceIndex);
	TCHAR szBuffer[128];

	g_DeviceManager.GetDeviceName(pDeviceInfo,szBuffer);
	TCHAR szDriveLetter[8];
	lsprintf(szDriveLetter,_T(" (%C:)"),pDeviceInfo->Address.m_cDriveLetter);
	lstrcat(szBuffer,szDriveLetter);
	SetDlgItemText(IDC_NAMESTATIC,szBuffer);

	// Type.
	PrintDeviceType(pDeviceInfo,pDeviceCap);

	// Location.
	lsnprintf_s(szBuffer,128,lngGetString(PROPERTIES_DEVICELOC),pDeviceInfo->Address.m_iBus,
		pDeviceInfo->Address.m_iTarget,pDeviceInfo->Address.m_iLun);
	SetDlgItemText(IDC_LOCATIONSTATIC,szBuffer);

	// Buffer size.
	lsprintf(szBuffer,_T("%d kB"),pDeviceCap->uiBufferSize);
	SetDlgItemText(IDC_BUFFERSTATIC,szBuffer);

	// Max read speed.
	lsprintf(szBuffer,_T("%d kB/s (CD: %dx, DVD: %dx)"),pDeviceCap->uiMaxSpeeds[3],pDeviceCap->uiMaxSpeeds[4],pDeviceCap->uiMaxSpeeds[5]);
	SetDlgItemText(IDC_MAXREADSTATIC,szBuffer);

	// Max write speed.
	lsprintf(szBuffer,_T("%d kB/s (CD: %dx, DVD: %dx)"),pDeviceCap->uiMaxSpeeds[0],pDeviceCap->uiMaxSpeeds[1],pDeviceCap->uiMaxSpeeds[2]);
	SetDlgItemText(IDC_MAXWRITESTATIC,szBuffer);

	if (pDeviceCap->uiMaxSpeeds[0] == 0)
		::EnableWindow(GetDlgItem(IDC_WRITESPEEDSPIN),FALSE);

	// Read media.
	::SendMessage(GetDlgItem(IDC_READCDRCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READCDR) ? BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READCDRWCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READCDRW) ? BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDRCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READDVDR) ? BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDRAMCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READDVDRAM) ? BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_READDVDROMCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_READDVDROM) ? BST_INDETERMINATE : BST_UNCHECKED,0);

	// Write media.
	::SendMessage(GetDlgItem(IDC_WRITECDRCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITECDR) ? BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITECDRWCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITECDRW) ? BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDRCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDR) ? BST_INDETERMINATE : BST_UNCHECKED,0);
	::SendMessage(GetDlgItem(IDC_WRITEDVDRAMCHECK),BM_SETCHECK,(pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDRAM) ? BST_INDETERMINATE : BST_UNCHECKED,0);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CDeviceGeneralPage::OnWriteSpeedSpin(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	NMUPDOWN *lpnmud = (NMUPDOWN *)pNMH;

	// Display a warning message.
	if (g_GlobalSettings.m_bWriteSpeedWarning)
	{
		CInfoDlg InfoDlg(&g_GlobalSettings.m_bWriteSpeedWarning,lngGetString(INFO_WRITESPEED),INFODLG_ICONWARNING);
		if (InfoDlg.DoModal() == IDCANCEL)
			return TRUE;
	}

	TCHAR szBuffer[128];
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(m_uiDeviceIndex);

	float f1CD = (float)pDeviceCap->uiMaxSpeeds[0]/pDeviceCap->uiMaxSpeeds[1];

	if (lpnmud->iDelta < 0)
	{
		if (pDeviceCap->uiMaxSpeeds[1] < 2147483647)	// cdrecord uses signed integers.
		{
			pDeviceCap->uiMaxSpeeds[0] += (unsigned int)f1CD;
			pDeviceCap->uiMaxSpeeds[1] += 1;
			pDeviceCap->uiMaxSpeeds[2] = pDeviceCap->uiMaxSpeeds[0]/1385;
		}
	}
	else
	{
		if (pDeviceCap->uiMaxSpeeds[1] > 1)
		{
			pDeviceCap->uiMaxSpeeds[0] -= (unsigned int)f1CD;
			pDeviceCap->uiMaxSpeeds[1] -= 1;
			pDeviceCap->uiMaxSpeeds[2] = pDeviceCap->uiMaxSpeeds[0]/1385;
		}
	}

	lsprintf(szBuffer,_T("%d kB/s (CD: %dx, DVD: %dx)"),pDeviceCap->uiMaxSpeeds[0],pDeviceCap->uiMaxSpeeds[1],pDeviceCap->uiMaxSpeeds[2]);
	SetDlgItemText(IDC_MAXWRITESTATIC,szBuffer);

	return TRUE;
}
