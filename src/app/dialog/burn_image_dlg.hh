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
#include "burn_image_general_page.hh"
#include "burn_advanced_page.hh"
#include "CtrlMessages.h"

class CBurnImageDlg : public CPropertySheetImpl<CBurnImageDlg>
{
private:
	bool m_bCentered;
	bool m_bAppMode;
	ckmmc::Device *m_pDevice;

	CBurnImageGeneralPage m_GeneralPage;
	CBurnAdvancedPage m_AdvancedPage;

public:
	CBurnImageDlg(const TCHAR *szTitle,bool bImageHasTOC,
		bool bEnableOnFly,bool bEnableVerify,bool bAppMode);
	~CBurnImageDlg();

	BEGIN_MSG_MAP(CBurnImageDlg)
		MESSAGE_HANDLER(WM_SHOWWINDOW,OnShowWindow)
		MESSAGE_HANDLER(WM_SETDEVICE,OnSetDevice)
		MESSAGE_HANDLER(WM_GETDEVICE,OnGetDevice)

		CHAIN_MSG_MAP(CPropertySheetImpl<CBurnImageDlg>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetDevice(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnGetDevice(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
