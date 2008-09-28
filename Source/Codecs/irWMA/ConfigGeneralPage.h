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
		// Profile number 21 should hopefully be 128 kbps.
		m_iProfile = 21;
	}

	int m_iProfile;
};

class CConfigGeneralPage : public CPropertyPageImpl<CConfigGeneralPage>
{
private:
	CListBox m_ProfileList;

	CEncoderConfig *m_pConfig;

	bool FillProfileList();

public:
	enum { IDD = IDD_PROPPAGE_CONFIGGENERAL };

	CConfigGeneralPage();
	~CConfigGeneralPage();

	bool SetConfig(CEncoderConfig *pConfig);
	bool OnApply();

	BEGIN_MSG_MAP(CConfigGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		CHAIN_MSG_MAP(CPropertyPageImpl<CConfigGeneralPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
