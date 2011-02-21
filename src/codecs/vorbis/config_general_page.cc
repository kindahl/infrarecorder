/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#ifndef UNICODE
#include <stdio.h>
#endif
#include "config_general_page.hh"
#include <base/string_util.hh>

CConfigGeneralPage::CConfigGeneralPage()
{
	m_pConfig = NULL;
}

CConfigGeneralPage::~CConfigGeneralPage()
{
}

bool CConfigGeneralPage::SetConfig(CEncoderConfig *pConfig)
{
	if (pConfig == NULL)
		return false;

	m_pConfig = pConfig;
	return true;
}

bool CConfigGeneralPage::ValidateBitrateValue(int iControl,int &iValue)
{
	TCHAR szBuffer[4];

	GetDlgItemText(iControl,szBuffer,4);
#ifdef UNICODE
	iValue = _wtoi(szBuffer);
#else
	iValue = atoi(szBuffer);
#endif

	return iValue > 47 && iValue < 501;
}

bool CConfigGeneralPage::OnApply()
{
	// Bitrates.
	int iValue;

	if (!ValidateBitrateValue(IDC_BITRATEEDIT,iValue))
	{
		MessageBox(_T("Invalid constant bitrate value. The value must be between 48 and 500 (inclusive)."),_T("Error"),MB_OK | MB_ICONERROR);
		return false;
	}
	m_pConfig->m_iBitrate = iValue;

	if (!ValidateBitrateValue(IDC_MINBITRATEEDIT,iValue))
	{
		MessageBox(_T("Invalid minimum bitrate value. The value must be between 48 and 500 (inclusive)."),_T("Error"),MB_OK | MB_ICONERROR);
		return false;
	}
	m_pConfig->m_iMinBitrate = iValue;

	if (!ValidateBitrateValue(IDC_MAXBITRATEEDIT,iValue))
	{
		MessageBox(_T("Invalid maximum bitrate value. The value must be between 48 and 500 (inclusive)."),_T("Error"),MB_OK | MB_ICONERROR);
		return false;
	}
	m_pConfig->m_iMaxBitrate = iValue;

	if (!ValidateBitrateValue(IDC_AVBITRATEEDIT,iValue))
	{
		MessageBox(_T("Invalid average bitrate value. The value must be between 48 and 500 (inclusive)."),_T("Error"),MB_OK | MB_ICONERROR);
		return false;
	}
	m_pConfig->m_iAvBitrate = iValue;

	// Mode.
	if (IsDlgButtonChecked(IDC_QUALITYRADIO))
		m_pConfig->m_iMode = CONFIG_MODE_QUALITY;
	else if (IsDlgButtonChecked(IDC_BITRATERADIO))
		m_pConfig->m_iMode = CONFIG_MODE_BITRATE;
	else if (IsDlgButtonChecked(IDC_VARBITRATERADIO))
		m_pConfig->m_iMode = CONFIG_MODE_VARBITRATE;
	else
		m_pConfig->m_iMode = CONFIG_MODE_AVBITRATE;

	// Quality.
	m_pConfig->m_iQuality = m_QualityTrackBar.GetPos();

	return true;
}

LRESULT CConfigGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Setup the quality slider.
	m_QualityTrackBar = GetDlgItem(IDC_QUALITYSLIDER);
	for (unsigned int i = 0; i <= 100; i += 10)
		m_QualityTrackBar.SetTic(i);

	m_QualityTrackBar.SetLineSize(10);
	m_QualityTrackBar.SetPageSize(20);
	m_QualityTrackBar.SetRange(0,100);
	m_QualityTrackBar.SetPos(m_pConfig->m_iQuality);

	// Initialize the edit controls.
	TCHAR szBuffer[4];
	lsprintf(szBuffer,_T("%d"),m_pConfig->m_iBitrate);
	::SendMessage(GetDlgItem(IDC_BITRATEEDIT),EM_SETLIMITTEXT,3,0);
	SetDlgItemText(IDC_BITRATEEDIT,szBuffer);

	lsprintf(szBuffer,_T("%d"),m_pConfig->m_iMinBitrate);
	::SendMessage(GetDlgItem(IDC_MINBITRATEEDIT),EM_SETLIMITTEXT,3,0);
	SetDlgItemText(IDC_MINBITRATEEDIT,szBuffer);

	lsprintf(szBuffer,_T("%d"),m_pConfig->m_iMaxBitrate);
	::SendMessage(GetDlgItem(IDC_MAXBITRATEEDIT),EM_SETLIMITTEXT,3,0);
	SetDlgItemText(IDC_MAXBITRATEEDIT,szBuffer);

	lsprintf(szBuffer,_T("%d"),m_pConfig->m_iAvBitrate);
	::SendMessage(GetDlgItem(IDC_AVBITRATEEDIT),EM_SETLIMITTEXT,3,0);
	SetDlgItemText(IDC_AVBITRATEEDIT,szBuffer);

	// Select the default mode.
	switch (m_pConfig->m_iMode)
	{
		case CONFIG_MODE_QUALITY:
			CheckDlgButton(IDC_QUALITYRADIO,BST_CHECKED);
			break;

		case CONFIG_MODE_BITRATE:
			CheckDlgButton(IDC_BITRATERADIO,BST_CHECKED);
			break;

		case CONFIG_MODE_VARBITRATE:
			CheckDlgButton(IDC_VARBITRATERADIO,BST_CHECKED);
			break;

		case CONFIG_MODE_AVBITRATE:
			CheckDlgButton(IDC_AVBITRATERADIO,BST_CHECKED);
			break;
	}

	SelectMode(m_pConfig->m_iMode);

	return TRUE;
}

void CConfigGeneralPage::SelectMode(int iMode)
{
	// Quality mode.
	::EnableWindow(GetDlgItem(IDC_QUALITYSTATIC),iMode == CONFIG_MODE_QUALITY);
	::EnableWindow(GetDlgItem(IDC_QUALITYSLIDER),iMode == CONFIG_MODE_QUALITY);
	::EnableWindow(GetDlgItem(IDC_QUALITYHIGHSTATIC),iMode == CONFIG_MODE_QUALITY);
	::EnableWindow(GetDlgItem(IDC_QUALITYLOWSTATIC),iMode == CONFIG_MODE_QUALITY);

	// Constant bitrate mode.
	::EnableWindow(GetDlgItem(IDC_BITRATESTATIC),iMode == CONFIG_MODE_BITRATE);
	::EnableWindow(GetDlgItem(IDC_BITRATEEDIT),iMode == CONFIG_MODE_BITRATE);
	::EnableWindow(GetDlgItem(IDC_BITRATESPIN),iMode == CONFIG_MODE_BITRATE);

	// Variable bitrate mode.
	::EnableWindow(GetDlgItem(IDC_MINBITRATESTATIC),iMode == CONFIG_MODE_VARBITRATE);
	::EnableWindow(GetDlgItem(IDC_MINBITRATEEDIT),iMode == CONFIG_MODE_VARBITRATE);
	::EnableWindow(GetDlgItem(IDC_MINBITRATESPIN),iMode == CONFIG_MODE_VARBITRATE);
	::EnableWindow(GetDlgItem(IDC_MAXBITRATESTATIC),iMode == CONFIG_MODE_VARBITRATE);
	::EnableWindow(GetDlgItem(IDC_MAXBITRATEEDIT),iMode == CONFIG_MODE_VARBITRATE);
	::EnableWindow(GetDlgItem(IDC_MAXBITRATESPIN),iMode == CONFIG_MODE_VARBITRATE);

	// Average bitrate mode.
	::EnableWindow(GetDlgItem(IDC_AVBITRATESTATIC),iMode == CONFIG_MODE_AVBITRATE);
	::EnableWindow(GetDlgItem(IDC_AVBITRATEEDIT),iMode == CONFIG_MODE_AVBITRATE);
	::EnableWindow(GetDlgItem(IDC_AVBITRATESPIN),iMode == CONFIG_MODE_AVBITRATE);
}

LRESULT CConfigGeneralPage::OnQualityCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CheckDlgButton(IDC_BITRATERADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_VARBITRATERADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_AVBITRATERADIO,BST_UNCHECKED);
	SelectMode(CONFIG_MODE_QUALITY);

	bHandled = false;
	return TRUE;
}

LRESULT CConfigGeneralPage::OnBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CheckDlgButton(IDC_QUALITYRADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_VARBITRATERADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_AVBITRATERADIO,BST_UNCHECKED);
	SelectMode(CONFIG_MODE_BITRATE);

	bHandled = false;
	return TRUE;
}

LRESULT CConfigGeneralPage::OnVarBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CheckDlgButton(IDC_QUALITYRADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_BITRATERADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_AVBITRATERADIO,BST_UNCHECKED);
	SelectMode(CONFIG_MODE_VARBITRATE);

	bHandled = false;
	return TRUE;
}

LRESULT CConfigGeneralPage::OnAvBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CheckDlgButton(IDC_QUALITYRADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_BITRATERADIO,BST_UNCHECKED);
	CheckDlgButton(IDC_VARBITRATERADIO,BST_UNCHECKED);
	SelectMode(CONFIG_MODE_AVBITRATE);

	bHandled = false;
	return TRUE;
}

LRESULT CConfigGeneralPage::OnBitrateSpin(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	NMUPDOWN *lpnmud = (NMUPDOWN *)pNMH;

	TCHAR szBuffer[4];

	// Get the control value.
	switch (idCtrl)
	{
		case IDC_BITRATESPIN:
			GetDlgItemText(IDC_BITRATEEDIT,szBuffer,4);
			break;

		case IDC_MINBITRATESPIN:
			GetDlgItemText(IDC_MINBITRATEEDIT,szBuffer,4);
			break;

		case IDC_MAXBITRATESPIN:
			GetDlgItemText(IDC_MAXBITRATEEDIT,szBuffer,4);
			break;

		case IDC_AVBITRATESPIN:
			GetDlgItemText(IDC_AVBITRATEEDIT,szBuffer,4);
			break;
	}

#ifdef UNICODE
	int iValue = _wtoi(szBuffer);
#else
	int iValue = atoi(szBuffer);
#endif

	// Calculate the new value.
	if (lpnmud->iDelta < 0)
	{
		if (iValue < 48)
			iValue = 48;
		else if (iValue < 71)
		{
			iValue >>= 3;
			iValue <<= 3;
			iValue += 8;
		}
		else if (iValue < 175)
		{
			iValue >>= 4;
			iValue <<= 4;
			iValue += 16;
		}
		else
		{
			iValue >>= 5;
			iValue <<= 5;
			iValue += 32;
		}

		if (iValue > 500)
			iValue = 500;
	}
	else
	{
		if (iValue > 500)
			iValue = 500;
		else if (iValue < 71)
		{
			iValue >>= 3;
			iValue <<= 3;
			iValue -= 8;
		}
		else if (iValue < 175)
		{
			iValue >>= 4;
			iValue <<= 4;
			iValue -= 16;
		}
		else
		{
			iValue >>= 5;
			iValue <<= 5;
			iValue -= 32;
		}

		if (iValue < 48)
			iValue = 48;
	}

	// Set the new value.
	lsprintf(szBuffer,_T("%d"),iValue);
	switch (idCtrl)
	{
		case IDC_BITRATESPIN:
			SetDlgItemText(IDC_BITRATEEDIT,szBuffer);
			break;

		case IDC_MINBITRATESPIN:
			SetDlgItemText(IDC_MINBITRATEEDIT,szBuffer);
			break;

		case IDC_MAXBITRATESPIN:
			SetDlgItemText(IDC_MAXBITRATEEDIT,szBuffer);
			break;

		case IDC_AVBITRATESPIN:
			SetDlgItemText(IDC_AVBITRATEEDIT,szBuffer);
			break;
	}

	return TRUE;
}
