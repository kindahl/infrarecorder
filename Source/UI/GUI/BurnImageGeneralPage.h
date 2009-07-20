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
#include "Core2.h"

class CBurnImageGeneralPage : public CPropertyPageImpl<CBurnImageGeneralPage>
{
private:
	bool m_bImageHasTOC;
	bool m_bEnableOnFly;
	bool m_bEnableVerify;
	unsigned int m_uiParentTitleLen;
	HICON m_hIcon;
	HICON m_hRefreshIcon;
	HIMAGELIST m_hRefreshImageList;

	CComboBox m_RecorderCombo;
	CComboBox m_WriteSpeedCombo;
	CComboBox m_WriteMethodCombo;
	CComboBox m_NumCopiesCombo;

	enum
	{
		TIMER_ID = 42,
		TIMER_INTERVAL = 1000
	};

	bool Translate();
	bool InitRecorderMedia();
	void SuggestWriteMethod();
	void InitRefreshButton();
	void CheckRecorderMedia();

public:
	enum { IDD = IDD_PROPPAGE_BURNIMAGEGENERAL };

	CBurnImageGeneralPage(bool bImageHasTOC,bool bEnableOnFly,bool bEnableVerify);
	~CBurnImageGeneralPage();

	bool OnApply();
	void OnHelp();

	BEGIN_MSG_MAP(CBurnImageGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER,OnTimer)
		COMMAND_HANDLER(IDC_RECORDERCOMBO,CBN_SELCHANGE,OnRecorderChange)
		COMMAND_HANDLER(IDC_REFRESHBUTTON,BN_CLICKED,OnRefresh)
		COMMAND_HANDLER(IDC_FIXATECHECK,BN_CLICKED,OnFixateCheck)

		CHAIN_MSG_MAP(CPropertyPageImpl<CBurnImageGeneralPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnRecorderChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnRefresh(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFixateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
