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

#pragma once
#include "resource.h"
#include "CtrlMessages.h"

class CReadOptionsPage : public CPropertyPageImpl<CReadOptionsPage>
{
private:
	bool m_bEnableClone;
	bool m_bEnableSpeed;
	bool m_bCloneCheck;		// The initial state of the read subchannel data.
	HICON m_hRefreshIcon;
	HIMAGELIST m_hRefreshImageList;
	CComboBox m_ReadSpeedCombo;

	bool Translate();
	void UpdateSpeeds();
	void CheckMedia();

public:
	enum { IDD = IDD_PROPPAGE_READOPTIONS };

	CReadOptionsPage(bool bEnableClone,bool bEnableSpeed);
	~CReadOptionsPage();

	bool OnApply();
	void OnHelp();

	BEGIN_MSG_MAP(CReadOptionsPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_SHOWWINDOW,OnShowWindow)
		MESSAGE_HANDLER(WM_CHECKMEDIA,OnCheckMedia)

		CHAIN_MSG_MAP(CPropertyPageImpl<CReadOptionsPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnCheckMedia(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	void SetCloneMode(bool bEnable);
};
