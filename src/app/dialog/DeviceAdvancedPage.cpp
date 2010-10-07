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
#include "LangUtil.h"
#include "Settings.h"
#include "cdrtools_parse_strings.hh"
#include "DeviceAdvancedPage.h"

CDeviceAdvancedPage::CDeviceAdvancedPage(ckmmc::Device &Device) :
	m_Device(Device)
{
	// If set to true the list view will not accept any item changes, that
	// includes both selection and checking.
	m_bLockAdvList = false;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_ADVANCED,szStrValue))
				SetTitle(szStrValue);
		}
	}
}

CDeviceAdvancedPage::~CDeviceAdvancedPage()
{
}

bool CDeviceAdvancedPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a device translation section.
	if (!pLng->EnterSection(_T("device")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDC_INFOSTATIC,szStrValue))
		SetDlgItemText(IDC_INFOSTATIC,szStrValue);

	return true;
}

LRESULT CDeviceAdvancedPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Fill the list view.
	m_ListView.SubclassWindow(GetDlgItem(IDC_ADVLIST));
	m_ListView.Initialize();
	m_ListView.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);

	m_ListView.AddColumn(_T(""),0);
	m_ListView.SetColumnWidth(0,370);

	// General.
	unsigned int uiItemCount = 0;
	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_MODE2FORM1));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_MODE_2_FORM_1));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_MODE2FORM2));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_MODE_2_FORM_2));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READDIGAUDIO));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_CDDA_SUPPORTED));

	if (m_Device.support(ckmmc::Device::ckDEVICE_CDDA_SUPPORTED))
	{
		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RESTARTNSDARA));
		m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_CDDA_ACCURATE));
	}

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READMULTSESSION));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_MULTI_SESSION));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READFIXPACKET));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_METHOD_2));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READBARCODE));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_READ_BAR_CODE));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READRWSUBCODE));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_RW_SUPPORTED));

	if (m_Device.support(ckmmc::Device::ckDEVICE_RW_SUPPORTED))
	{
		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_READRAWPWSC));
		m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_RW_DEINT_CORR));
	}

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_SIMULATION));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_TEST_WRITE));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_BUFRECORDING));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_BUP));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_C2EP));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_C2_POINTERS));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_EJECTCDSS));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_EJECT));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_CHANGEDISCSIDE));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_CHANGE_SIDES));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_INDIVIDUALDP));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_CHANGE_DISC_PRSNT));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RETURNCDCN));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_UPC));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RETURNCDISRC));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_ISRC));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DELIVCOMPOSITE));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_COMPOSITE));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_PLAYAUDIOCD));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_AUDIO_PLAY));

	if (m_Device.support(ckmmc::Device::ckDEVICE_AUDIO_PLAY))
	{
		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_INDIVIDUALVC));
		m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_SEP_CHAN_VOL));

		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_INDEPENDENTMUTE));
		m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_SEP_CHAN_MUTE));

		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOPORT1));
		m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_DIGITAL_PORT_1));

		m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOPORT2));
		m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_DIGITAL_PORT_2));

		// Digital output.
		if (m_Device.support(ckmmc::Device::ckDEVICE_DIGITAL_PORT_1) ||
			m_Device.support(ckmmc::Device::ckDEVICE_DIGITAL_PORT_2))
		{
			m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOSENDDIGDAT));
			m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_LSBF));

			m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_DOSETLRCK));
			m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_RCK));

			m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_HASVALIDDATA));
			m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_BCKF));
		}
	}

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_HASLESIC));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_SSS));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_LMOPU));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_PREVENT_JUMPER));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_ALLOWML));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckDEVICE_LOCK));

	// Write methods.
	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_SAO));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckWM_SAO));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_TAO));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckWM_TAO));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RAW96R));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckWM_RAW96R));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RAW16));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckWM_RAW16));

	m_ListView.AddItem(uiItemCount,0,lngGetString(ADVPROP_RAW96P));
	m_ListView.SetCheckState(uiItemCount++,m_Device.support(ckmmc::Device::ckWM_RAW96P));

	m_bLockAdvList = true;

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CDeviceAdvancedPage::OnItemChanging(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	return m_bLockAdvList;
}
