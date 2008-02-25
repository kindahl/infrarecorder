/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once
#include "BurnImageGeneralPage.h"
#include "BurnAdvancedPage.h"
#include "CtrlMessages.h"

class CBurnImageDlg : public CPropertySheetImpl<CBurnImageDlg>
{
private:
	bool m_bCentered;
	unsigned int m_uiDeviceIndex;

	CBurnImageGeneralPage m_GeneralPage;
	CBurnAdvancedPage m_AdvancedPage;

public:
	CBurnImageDlg(const TCHAR *szTitle,bool bImageHasTOC,
		bool bEnableOnFly,bool bEnableVerify);
	~CBurnImageDlg();

	BEGIN_MSG_MAP(CBurnImageDlg)
		MESSAGE_HANDLER(WM_SHOWWINDOW,OnShowWindow)
		MESSAGE_HANDLER(WM_SETDEVICEINDEX,OnSetDeviceIndex)
		MESSAGE_HANDLER(WM_GETDEVICEINDEX,OnGetDeviceIndex)

		CHAIN_MSG_MAP(CPropertySheetImpl<CBurnImageDlg>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetDeviceIndex(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnGetDeviceIndex(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
