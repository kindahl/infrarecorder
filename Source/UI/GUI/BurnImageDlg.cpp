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

#include "stdafx.h"
#include "BurnImageDlg.h"

CBurnImageDlg::CBurnImageDlg(const TCHAR *szTitle,bool bImageHasTOC,
							 bool bEnableOnFly,bool bEnableVerify,
							 bool bAppMode) :
	CPropertySheetImpl<CBurnImageDlg>(szTitle,0,NULL),
	m_GeneralPage(bImageHasTOC,bEnableOnFly,bEnableVerify)
{
	m_bCentered = false;
	m_bAppMode = bAppMode;

	m_uiDeviceIndex = 0;

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_HASHELP | PSH_NOCONTEXTHELP;

	AddPage(m_GeneralPage);
	AddPage(m_AdvancedPage);
}

CBurnImageDlg::~CBurnImageDlg()
{
}

LRESULT CBurnImageDlg::OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Center the window, once.
	if (wParam == TRUE && !m_bCentered)
	{
		CenterWindow();
		m_bCentered = true;
	}

	// Add the dialog to the task bar and enable it to be minimized.
	if (m_bAppMode)
	{
		ModifyStyle(0,WS_MINIMIZEBOX | WS_SYSMENU);
		ModifyStyleEx(0,WS_EX_APPWINDOW);

		HMENU hSysMenu = GetSystemMenu(FALSE);
		::InsertMenu(hSysMenu,0,MF_BYPOSITION,SC_RESTORE,_T("&Restore"));
		::InsertMenu(hSysMenu,2,MF_BYPOSITION,SC_MINIMIZE,_T("Mi&nimize"));
		::InsertMenu(hSysMenu,3,MF_BYPOSITION | MF_SEPARATOR,0,_T(""));
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CBurnImageDlg::OnSetDeviceIndex(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_uiDeviceIndex = (unsigned int)lParam;

	bHandled = TRUE;
	return 0;
}

LRESULT CBurnImageDlg::OnGetDeviceIndex(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	bHandled = TRUE;
	return m_uiDeviceIndex;
}
