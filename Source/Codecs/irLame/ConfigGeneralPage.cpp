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
#include "ConfigGeneralPage.h"

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

bool CConfigGeneralPage::OnApply()
{
	m_pConfig->m_bMono = IsDlgButtonChecked(IDC_MONOCHECK) == TRUE;
	m_pConfig->m_bBitrateMode = IsDlgButtonChecked(IDC_BITRATERADIO) == TRUE;
	m_pConfig->m_bConstantBitrate = IsDlgButtonChecked(IDC_BITRATECHECK) == TRUE;
	m_pConfig->m_bFastVBR = m_VBRModeComboBox.GetCurSel() == 1;

	m_pConfig->m_iPreset = m_PresetComboBox.GetCurSel();
	m_pConfig->m_iEncodeQuality = m_EncQualityComboBox.GetCurSel();
	m_pConfig->m_iBitrate = m_BitrateTrackBar.GetPos();
	m_pConfig->m_iQuality = m_QualityTrackBar.GetPos();

	return true;
}

LRESULT CConfigGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Fill the preset combo box.
	m_PresetComboBox = GetDlgItem(IDC_PRESETCOMBO);
	m_PresetComboBox.AddString(_T("Custom"));
	m_PresetComboBox.AddString(_T("Medium (VBR 150-180 kbps)"));
	m_PresetComboBox.AddString(_T("Standard (VBR 170-210 kbps)"));
	m_PresetComboBox.AddString(_T("Extreme (VBR 200-240 kbps)"));
	m_PresetComboBox.AddString(_T("Insane (CBR 320 kbps)"));
	m_PresetComboBox.SetCurSel(m_pConfig->m_iPreset);

	// Fill the encode quality combo box.
	m_EncQualityComboBox = GetDlgItem(IDC_ENCODEQUALITYCOMBO);
	m_EncQualityComboBox.AddString(_T("Fast"));
	m_EncQualityComboBox.AddString(_T("Standard"));
	m_EncQualityComboBox.AddString(_T("High"));
	m_EncQualityComboBox.SetCurSel(m_pConfig->m_iEncodeQuality);

	// Fill the VBR mode combo box.
	m_VBRModeComboBox = GetDlgItem(IDC_VBRMODECOMBO);
	m_VBRModeComboBox.AddString(_T("Standard"));
	m_VBRModeComboBox.AddString(_T("Fast"));
	m_VBRModeComboBox.SetCurSel((int)m_pConfig->m_bFastVBR);

	// Perform a Windows XP theme check here to move the radio button.
	/*CStatic BitrateStatic = GetDlgItem(IDC_BITRATESTATIC);

	RECT rcWindow;
	BitrateStatic.GetWindowRect(&rcWindow);
	ScreenToClient(&rcWindow);

	rcWindow.left -= 2;
	BitrateStatic.MoveWindow(&rcWindow);*/

	// Setup the bitrate slider.
	m_BitrateTrackBar = GetDlgItem(IDC_BITRATESLIDER);
	m_BitrateTrackBar.SetRange(8,320);
	m_BitrateTrackBar.SetPageSize(8);
	m_BitrateTrackBar.SetLineSize(8);
	m_BitrateTrackBar.SetTic(16);
	m_BitrateTrackBar.SetTic(24);
	m_BitrateTrackBar.SetTic(32);
	m_BitrateTrackBar.SetTic(40);
	m_BitrateTrackBar.SetTic(48);
	m_BitrateTrackBar.SetTic(56);
	m_BitrateTrackBar.SetTic(64);
	m_BitrateTrackBar.SetTic(80);
	m_BitrateTrackBar.SetTic(96);
	m_BitrateTrackBar.SetTic(112);
	m_BitrateTrackBar.SetTic(128);
	m_BitrateTrackBar.SetTic(144);
	m_BitrateTrackBar.SetTic(160);
	m_BitrateTrackBar.SetTic(192);
	m_BitrateTrackBar.SetTic(224);
	m_BitrateTrackBar.SetTic(256);
	m_BitrateTrackBar.SetPos(m_pConfig->m_iBitrate);

	// Setup the quality slider.
	m_QualityTrackBar = GetDlgItem(IDC_QUALITYSLIDER);
	m_QualityTrackBar.SetRange(0,9);
	m_QualityTrackBar.SetPos(m_pConfig->m_iQuality);

	// We select the bitrate mode by default.
	if (m_pConfig->m_bBitrateMode)
		CheckDlgButton(IDC_BITRATERADIO,BST_CHECKED);
	else
		CheckDlgButton(IDC_QUALITYRADIO,BST_CHECKED);

	SelectBitrateMode(m_pConfig->m_bBitrateMode);

	// Select the default (standard) preset.
	SelectPreset(m_pConfig->m_iPreset);

	return TRUE;
}

LRESULT CConfigGeneralPage::OnHScroll(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (IsDlgButtonChecked(IDC_BITRATERADIO) && LOWORD(wParam) == TB_THUMBTRACK)
	{
		unsigned int uiPos = HIWORD(wParam);

		// Jump to the closest position that have a tic.
		if (uiPos < 71)
		{
			uiPos >>= 3;
			uiPos <<= 3;
		}
		else if (uiPos < 175)
		{
			uiPos >>= 4;
			uiPos <<= 4;
		}
		else if (uiPos < 287)
		{
			uiPos >>= 5;
			uiPos <<= 5;
		}
		else
		{
			uiPos = 320;
		}

		m_BitrateTrackBar.SetPos(uiPos);
	}

	bHandled = false;
	return TRUE;
}

void CConfigGeneralPage::SelectBitrateMode(BOOL bSelect)
{
	::EnableWindow(GetDlgItem(IDC_QUALITYSTATIC),!bSelect);
	::EnableWindow(GetDlgItem(IDC_QUALITYSLIDER),!bSelect);
	::EnableWindow(GetDlgItem(IDC_QUALITYHIGHSTATIC),!bSelect);
	::EnableWindow(GetDlgItem(IDC_QUALITYLOWSTATIC),!bSelect);
	::EnableWindow(GetDlgItem(IDC_VBRMODESTATIC),!bSelect);
	::EnableWindow(GetDlgItem(IDC_VBRMODECOMBO),!bSelect);

	::EnableWindow(GetDlgItem(IDC_BITRATESTATIC),bSelect);
	::EnableWindow(GetDlgItem(IDC_BITRATESLIDER),bSelect);
	::EnableWindow(GetDlgItem(IDC_BITRATE8STATIC),bSelect);
	::EnableWindow(GetDlgItem(IDC_BITRATE64STATIC),bSelect);
	::EnableWindow(GetDlgItem(IDC_BITRATE128STATIC),bSelect);
	::EnableWindow(GetDlgItem(IDC_BITRATE192STATIC),bSelect);
	::EnableWindow(GetDlgItem(IDC_BITRATE320STATIC),bSelect);
	::EnableWindow(GetDlgItem(IDC_BITRATECHECK),bSelect);
}

void CConfigGeneralPage::SelectPreset(int iPreset)
{
	if (iPreset != CONFIG_PRESET_CUSTOM)
	{
		::EnableWindow(GetDlgItem(IDC_QUALITYSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_QUALITYSLIDER),FALSE);
		::EnableWindow(GetDlgItem(IDC_QUALITYHIGHSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_QUALITYLOWSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_VBRMODESTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_VBRMODECOMBO),FALSE);

		::EnableWindow(GetDlgItem(IDC_BITRATESTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BITRATESLIDER),FALSE);
		::EnableWindow(GetDlgItem(IDC_BITRATE8STATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BITRATE64STATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BITRATE128STATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BITRATE192STATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BITRATE320STATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BITRATECHECK),FALSE);

		// Controls not covered by the SelectBitrateMode function.
		::EnableWindow(GetDlgItem(IDC_BITRATERADIO),FALSE);
		::EnableWindow(GetDlgItem(IDC_QUALITYRADIO),FALSE);
		::EnableWindow(GetDlgItem(IDC_ENCODEQUALITYSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_ENCODEQUALITYCOMBO),FALSE);
		::EnableWindow(GetDlgItem(IDC_MONOCHECK),FALSE);
	}
	else
	{
		SelectBitrateMode(IsDlgButtonChecked(IDC_BITRATERADIO));

		// Controls not covered by the SelectBitrateMode function.
		::EnableWindow(GetDlgItem(IDC_BITRATERADIO),TRUE);
		::EnableWindow(GetDlgItem(IDC_QUALITYRADIO),TRUE);
		::EnableWindow(GetDlgItem(IDC_ENCODEQUALITYSTATIC),TRUE);
		::EnableWindow(GetDlgItem(IDC_ENCODEQUALITYCOMBO),TRUE);
		::EnableWindow(GetDlgItem(IDC_MONOCHECK),TRUE);
	}
}

LRESULT CConfigGeneralPage::OnBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CheckDlgButton(IDC_QUALITYRADIO,BST_UNCHECKED);
	SelectBitrateMode(TRUE);

	bHandled = false;
	return TRUE;
}

LRESULT CConfigGeneralPage::OnQualityCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CheckDlgButton(IDC_BITRATERADIO,BST_UNCHECKED);
	SelectBitrateMode(FALSE);

	bHandled = false;
	return TRUE;
}

LRESULT CConfigGeneralPage::OnPresetChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	SelectPreset(m_PresetComboBox.GetCurSel());

	return 0;
}
