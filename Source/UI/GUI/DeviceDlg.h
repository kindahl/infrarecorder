/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "DeviceGeneralPage.h"
#include "DeviceAdvancedPage.h"

class CDeviceDlg : public CPropertySheetImpl<CDeviceDlg>
{
private:
	bool m_bCentered;

	CDeviceGeneralPage m_GeneralPage;
	CDeviceAdvancedPage m_AdvancedPage;

public:
	CDeviceDlg(UINT_PTR uiDeviceIndex,const TCHAR *szTitle);
	~CDeviceDlg();

	BEGIN_MSG_MAP(CDeviceDlg)
		MESSAGE_HANDLER(WM_SHOWWINDOW,OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheetImpl<CDeviceDlg>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
