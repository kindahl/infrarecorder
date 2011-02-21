/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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

#include "stdafx.hh"
#include "device_dlg.hh"

CDeviceDlg::CDeviceDlg(ckmmc::Device &Device,const TCHAR *szTitle) :
	CPropertySheetImpl<CDeviceDlg>(szTitle,0,NULL),
	m_GeneralPage(Device),m_AdvancedPage(Device)
{
	m_bCentered = false;

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;

	AddPage(m_GeneralPage);
	AddPage(m_AdvancedPage);
}

CDeviceDlg::~CDeviceDlg()
{
}

LRESULT CDeviceDlg::OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Center the window, once.
	if (wParam == TRUE && !m_bCentered)
	{
		CenterWindow();
		m_bCentered = true;
	}

	bHandled = FALSE;
	return 0;
}
