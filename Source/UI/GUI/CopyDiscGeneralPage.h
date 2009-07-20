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
#include <vector>
#include "Resource.h"
#include "Core2.h"

class CCopyDiscGeneralPage : public CPropertyPageImpl<CCopyDiscGeneralPage>
{
private:
	unsigned int m_uiParentTitleLen;
	HICON m_hRefreshIcon;
	HIMAGELIST m_hRefreshImageList;

	CComboBox m_SourceCombo;
	CComboBox m_TargetCombo;
	CComboBox m_WriteSpeedCombo;
	CComboBox m_WriteMethodCombo;

	// This vectors holds the IDs of all controls located below the burn on the fly
	// warning label. These controls will be moved to make the translated text fit
	// if necessary.
	std::vector<int> m_iCtrlsBelowOnFly;

	enum
	{
		TIMER_ID = 42,
		TIMER_INTERVAL = 1000
	};

	bool Translate();
	bool InitRecorderMedia();
	bool AnalyzeDriveMedia(ckmmc::Device &Device);
	void InitRefreshButton();
	void CheckRecorderMedia();

	unsigned long MSFToLBA(unsigned long ulMin,unsigned long ulSec,unsigned long ulFrame);

public:
	enum { IDD = IDD_PROPPAGE_COPYDISCGENERAL };

	CCopyDiscGeneralPage();
	~CCopyDiscGeneralPage();

	bool OnApply();
	void OnHelp();

	BEGIN_MSG_MAP(CCopyDiscGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER,OnTimer)
		COMMAND_HANDLER(IDC_SOURCECOMBO,CBN_SELCHANGE,OnSourceChange)
		COMMAND_HANDLER(IDC_TARGETCOMBO,CBN_SELCHANGE,OnTargetChange)
		COMMAND_HANDLER(IDC_REFRESHBUTTON,BN_CLICKED,OnRefresh)
		COMMAND_HANDLER(IDC_FIXATECHECK,BN_CLICKED,OnFixateCheck)
		COMMAND_ID_HANDLER(IDC_ONFLYCHECK,OnFlyCheck)
		COMMAND_ID_HANDLER(IDC_CLONECHECK,OnCloneCheck)

		CHAIN_MSG_MAP(CPropertyPageImpl<CCopyDiscGeneralPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnSourceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnTargetChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnRefresh(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFlyCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCloneCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFixateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
