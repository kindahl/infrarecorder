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

#include "stdafx.h"
#include "project_prop_dlg.hh"
#include "project_manager.hh"
#include "string_table.hh"
#include "lang_util.hh"
#include "settings.hh"

CProjectPropDlg::CProjectPropDlg() : CPropertySheetImpl<CProjectPropDlg>(lngGetString(PROJECTPROP_TITLE),0,NULL)
{
	m_bCentered = false;

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_HASHELP | PSH_NOCONTEXTHELP;

	m_hGeneralPage = ::CreatePropertySheetPage(m_GeneralPage);
	m_hFileSysPage = ::CreatePropertySheetPage(m_FileSysPage);
	m_hIsoPage = ::CreatePropertySheetPage(m_IsoPage);
	m_hFieldsPage = ::CreatePropertySheetPage(m_FieldsPage);
	m_hBootPage = ::CreatePropertySheetPage(m_BootPage);
	m_hUdfPage = ::CreatePropertySheetPage(m_UdfPage);
	m_hAudioPage = ::CreatePropertySheetPage(m_AudioPage);

	AddPage(m_hGeneralPage);

	switch (g_ProjectManager.GetProjectType())
	{
		case PROJECTTYPE_DATA:
			AddPage(m_hFileSysPage);

			if (g_ProjectSettings.m_iFileSystem == FILESYSTEM_ISO9660_UDF ||
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_UDF ||
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO)
			{
				AddPage(m_hUdfPage);
			}

			if (g_ProjectSettings.m_iFileSystem == FILESYSTEM_ISO9660 || 
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_ISO9660_UDF ||
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO)
			{
				AddPage(m_hIsoPage);
				AddPage(m_hFieldsPage);
				AddPage(m_hBootPage);
			}
			break;

		case PROJECTTYPE_AUDIO:
			AddPage(m_hAudioPage);
			break;

		case PROJECTTYPE_MIXED:
			AddPage(m_hAudioPage);
			AddPage(m_hFileSysPage);

			if (g_ProjectSettings.m_iFileSystem == FILESYSTEM_ISO9660_UDF ||
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_UDF ||
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO)
			{
				AddPage(m_hUdfPage);
			}

			if (g_ProjectSettings.m_iFileSystem == FILESYSTEM_ISO9660 || 
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_ISO9660_UDF ||
				g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO)
			{
				AddPage(m_hIsoPage);
				AddPage(m_hFieldsPage);
				AddPage(m_hBootPage);
			}
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

	bHandled = false;
	return 0;
}

LRESULT CProjectPropDlg::OnSetFileSystem(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	switch (lParam)
	{
		case FILESYSTEM_ISO9660:
			if (wParam == FILESYSTEM_UDF)	// Check previous selection.
			{
				m_hIsoPage = ::CreatePropertySheetPage(m_IsoPage);
				m_hFieldsPage = ::CreatePropertySheetPage(m_FieldsPage);
				m_hBootPage = ::CreatePropertySheetPage(m_BootPage);

				AddPage(m_hIsoPage);
				AddPage(m_hFieldsPage);
				AddPage(m_hBootPage);
			}
			
			if (wParam != FILESYSTEM_ISO9660)
			{
				RemovePage(m_hUdfPage);
			}
			break;

		case FILESYSTEM_UDF:
			if (wParam != FILESYSTEM_UDF)	// Check previous selection.
			{
				RemovePage(m_hIsoPage);
				RemovePage(m_hFieldsPage);
				RemovePage(m_hBootPage);
			}

			if (wParam == FILESYSTEM_ISO9660)
			{
				m_hUdfPage = ::CreatePropertySheetPage(m_UdfPage);

				AddPage(m_hUdfPage);
			}
			break;

		case FILESYSTEM_ISO9660_UDF:
		case FILESYSTEM_DVDVIDEO:
			if (wParam == FILESYSTEM_UDF)	// Check previous selection.
			{
				m_hIsoPage = ::CreatePropertySheetPage(m_IsoPage);
				m_hFieldsPage = ::CreatePropertySheetPage(m_FieldsPage);
				m_hBootPage = ::CreatePropertySheetPage(m_BootPage);

				AddPage(m_hIsoPage);
				AddPage(m_hFieldsPage);
				AddPage(m_hBootPage);
			}
			else if (wParam == FILESYSTEM_ISO9660)
			{
				m_hUdfPage = ::CreatePropertySheetPage(m_UdfPage);

				InsertPage(2,m_hUdfPage);
			}
			break;
	}

	bHandled = true;
	return 0;
}
