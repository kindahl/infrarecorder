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

#pragma once
#include "resource.h"

#define CONFIG_PRESET_CUSTOM		0
#define CONFIG_PRESET_MEDIUM		1
#define CONFIG_PRESET_STANDARD		2
#define CONFIG_PRESET_EXTREME		3
#define CONFIG_PRESET_INSANE		4

#define CONFIG_EQ_FAST				0
#define CONFIG_EQ_STANDARD			1
#define CONFIG_EQ_HIGH				2

class CEncoderConfig
{
public:
	CEncoderConfig()
	{
		m_bMono = false;
		m_bBitrateMode = true;
		m_bConstantBitrate = false;
		m_bFastVBR = false;
		m_iPreset = CONFIG_PRESET_STANDARD;
		m_iEncodeQuality = CONFIG_EQ_STANDARD;
		m_iBitrate = 192;
		m_iQuality = 4;
	}

	bool m_bMono;
	bool m_bBitrateMode;
	bool m_bConstantBitrate;
	bool m_bFastVBR;
	int m_iPreset;
	int m_iEncodeQuality;
	int m_iBitrate;
	int m_iQuality;
};

class CConfigGeneralPage : public CPropertyPageImpl<CConfigGeneralPage>
{
private:
	CComboBox m_PresetComboBox;
	CComboBox m_EncQualityComboBox;
	CComboBox m_VBRModeComboBox;
	CTrackBarCtrl m_BitrateTrackBar;
	CTrackBarCtrl m_QualityTrackBar;

	void SelectBitrateMode(BOOL bSelect);
	void SelectPreset(int iPreset);

	CEncoderConfig *m_pConfig;

public:
	enum { IDD = IDD_PROPPAGE_CONFIGGENERAL };

	CConfigGeneralPage();
	~CConfigGeneralPage();

	bool SetConfig(CEncoderConfig *pConfig);
	bool OnApply();

	BEGIN_MSG_MAP(CConfigGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL,OnHScroll)

		COMMAND_ID_HANDLER(IDC_BITRATERADIO,OnBitrateCheck)
		COMMAND_ID_HANDLER(IDC_QUALITYRADIO,OnQualityCheck)
		COMMAND_HANDLER(IDC_PRESETCOMBO,CBN_SELCHANGE,OnPresetChange)

		CHAIN_MSG_MAP(CPropertyPageImpl<CConfigGeneralPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnHScroll(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnQualityCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnPresetChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
