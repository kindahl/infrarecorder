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
#include "ProjectPropDlg.h"
#include "ProjectManager.h"
#include "StringTable.h"
#include "LangUtil.h"

CProjectPropDlg::CProjectPropDlg() : CPropertySheetImpl<CProjectPropDlg>(lngGetString(PROJECTPROP_TITLE),0,NULL)
{
	m_bCentered = false;

	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	m_psh.dwFlags |= PSH_HASHELP;

	AddPage(m_GeneralPage);

	switch (g_ProjectManager.GetProjectType())
	{
		case PROJECTTYPE_DATA:
		case PROJECTTYPE_DVDVIDEO:
			AddPage(m_ISOPage);
			AddPage(m_FieldsPage);
			AddPage(m_BootPage);
			break;

		case PROJECTTYPE_AUDIO:
			AddPage(m_AudioPage);
			break;

		case PROJECTTYPE_MIXED:
			AddPage(m_ISOPage);
			AddPage(m_FieldsPage);
			AddPage(m_AudioPage);
			break;
	};
}

CProjectPropDlg::~CProjectPropDlg()
{
}

LRESULT CProjectPropDlg::OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
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
