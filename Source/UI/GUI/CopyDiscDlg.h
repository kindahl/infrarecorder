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
#include "CopyDiscGeneralPage.h"
#include "BurnAdvancedPage.h"
#include "ReadOptionsPage.h"
#include "CtrlMessages.h"

class CCopyDiscDlg : public CPropertySheetImpl<CCopyDiscDlg>
{
private:
	bool m_bCentered;
	bool m_bAppMode;
	unsigned int m_uiSourceDeviceIndex;
	unsigned int m_uiTargetDeviceIndex;

	CCopyDiscGeneralPage m_GeneralPage;
	CBurnAdvancedPage m_AdvancedPage;
	CReadOptionsPage m_ReadPage;

public:
	CCopyDiscDlg(bool bAppMode);
	~CCopyDiscDlg();

	BEGIN_MSG_MAP(CCopyDiscDlg)
		MESSAGE_HANDLER(WM_SHOWWINDOW,OnShowWindow)
		MESSAGE_HANDLER(WM_SETDEVICEINDEX,OnSetDeviceIndex)
		MESSAGE_HANDLER(WM_GETDEVICEINDEX,OnGetDeviceIndex)
		MESSAGE_HANDLER(WM_SETCLONEMODE,OnSetCloneMode)

		CHAIN_MSG_MAP(CPropertySheetImpl<CCopyDiscDlg>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetDeviceIndex(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnGetDeviceIndex(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetCloneMode(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
