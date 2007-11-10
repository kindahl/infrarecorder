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

#include "stdafx.h"
#include "ConfigDlg.h"

CConfigDlg::CConfigDlg(CEncoderConfig *pConfig) : CPropertySheetImpl<CConfigDlg>(_T("Configuration"),0,NULL)
{
	m_bCentered = false;

	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	m_GeneralPage.SetConfig(pConfig);
	AddPage(m_GeneralPage);
}

CConfigDlg::~CConfigDlg()
{
}

LRESULT CConfigDlg::OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
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
