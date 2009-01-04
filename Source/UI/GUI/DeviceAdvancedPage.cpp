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
#include "DeviceAdvancedPage.h"
#include "DeviceManager.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Settings.h"
#include "cdrtoolsParseStrings.h"

CDeviceAdvancedPage::CDeviceAdvancedPage()
{
	m_uiDeviceIndex = 0;

	// If set to true the list view will not accept any item changes, that
	// includes both selection and checking.
	m_bLockAdvList = false;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_ADVANCED,szStrValue))
				SetTitle(szStrValue);
		}
	}
}

CDeviceAdvancedPage::~CDeviceAdvancedPage()
{
}

bool CDeviceAdvancedPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a device translation section.
	if (!pLNG->EnterSection(_T("device")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDC_INFOSTATIC,szStrValue))
		SetDlgItemText(IDC_INFOSTATIC,szStrValue);

	return true;
}

void CDeviceAdvancedPage::SetDeviceIndex(UINT_PTR uiDeviceIndex)
{
	m_uiDeviceIndex = uiDeviceIndex;
}

LRESULT CDeviceAdvancedPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Fill the list view.
	m_ListView.SubclassWindow(GetDlgItem(IDC_ADVLIST));
	m_ListView.Initialize();
	m_ListView.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);

	m_ListView.AddColumn(_T(""),0);
	m_ListView.SetColumnWidth(0,299);

	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(m_uiDeviceIndex);
	tDeviceInfoEx *pDeviceInfoEx = g_DeviceManager.GetDeviceInfoEx(m_uiDeviceIndex);

	// General.
	unsigned int uiItemCount = 0;
	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_MODE2FORM1));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READMODE2FORM1);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_MODE2FORM2));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READMODE2FORM2);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READDIGAUDIO));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READDIGITALAUDIO);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READMULTSESSION));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READMULTISESSION);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READFIXPACKET));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READFIXEDPACKET);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READBARCODE));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READCDBARCODE);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READRWSUBCODE));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READRWSUBCODE);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READRAWPWSC));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READRAWPWSUBCODE);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_SIMULATION));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_BUFRECORDING));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_BUFRECORDING);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_C2EP));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_C2EP);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_EJECTCDSS));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_EJECTCDSS);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_CHANGEDISCSIDE));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_CHANGEDISCSIDE);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_INDIVIDUALDP));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_INDIVIDUALDP);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RETURNCDCN));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_RETURNCDCN);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RETURNCDISRC));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_RETURNCDISRC);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DELIVCOMPOSITE));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_DELIVERCOMPOSITE);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_PLAYAUDIOCD));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_PLAYAUDIOCD);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_HASLESIC));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_HASLESIC);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_LMOPU));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_LMOPU);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_ALLOWML));
	m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_ALLOWML);

	// Digital audio.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READDIGITALAUDIO)
	{
		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RESTARTNSDARA));
		m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiDigitalAudio & DEVICEMANAGER_CAP_RESTARTNSDARA);
	}

	// RW.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READRWSUBCODE)
	{
		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RETURNRWSUBCODE));
		m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiRW & DEVICEMANAGER_CAP_RETURNRWSUBCODE);
	}

	// Audio.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_PLAYAUDIOCD)
	{
		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_INDIVIDUALVC));
		m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiAudio & DEVICEMANAGER_CAP_INDIVIDUALVC);

		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_INDEPENDENTMUTE));
		m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiAudio & DEVICEMANAGER_CAP_INDEPENDENTMUTE);

		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOPORT1));
		m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT1);

		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOPORT2));
		m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT2);

		// Digital output.
		if ((pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT1) ||
			(pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT2))
		{
			m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOSENDDIGDAT));
			m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiDigitalOutput & DEVICEMANAGER_CAP_DOSENDDIGDAT);

			m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOSETLRCK));
			m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiDigitalOutput & DEVICEMANAGER_CAP_DOSETLRCK);

			m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_HASVALIDDATA));
			m_ListView.SetCheckState(uiItemCount++,pDeviceCap->uiDigitalOutput & DEVICEMANAGER_CAP_HASVALIDDATA);
		}
	}

	// Write methods.
	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_SAO));
	m_ListView.SetCheckState(uiItemCount++,strstr(pDeviceInfoEx->szWriteModes,CDRTOOLS_WRITEMODES_SAO) != NULL);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_TAO));
	m_ListView.SetCheckState(uiItemCount++,strstr(pDeviceInfoEx->szWriteModes,CDRTOOLS_WRITEMODES_TAO) != NULL);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RAW96R));
	m_ListView.SetCheckState(uiItemCount++,strstr(pDeviceInfoEx->szWriteModes,CDRTOOLS_WRITEMODES_RAW96R) != NULL);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RAW16));
	m_ListView.SetCheckState(uiItemCount++,strstr(pDeviceInfoEx->szWriteModes,CDRTOOLS_WRITEMODES_RAW16) != NULL);

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RAW96P));
	m_ListView.SetCheckState(uiItemCount++,strstr(pDeviceInfoEx->szWriteModes,CDRTOOLS_WRITEMODES_RAW96P) != NULL);

	m_bLockAdvList = true;

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CDeviceAdvancedPage::OnItemChanging(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	return m_bLockAdvList;
}
