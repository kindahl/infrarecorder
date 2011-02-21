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

#pragma once
#include "project_prop_general_page.hh"
#include "project_prop_file_sys_page.hh"
#include "project_prop_iso_page.hh"
#include "project_prop_fields_page.hh"
#include "project_prop_audio_page.hh"
#include "project_prop_boot_page.hh"
#include "project_prop_udf_page.hh"
#include "ctrl_messages.hh"

class CProjectPropDlg : public CPropertySheetImpl<CProjectPropDlg>
{
private:
	bool m_bCentered;

	CProjectPropGeneralPage m_GeneralPage;
	CProjectPropFileSysPage m_FileSysPage;
	CProjectPropIsoPage m_IsoPage;
	CProjectPropFieldsPage m_FieldsPage;
	CProjectPropBootPage m_BootPage;
	CProjectPropUdfPage m_UdfPage;
	CProjectPropAudioPage m_AudioPage;

	HPROPSHEETPAGE m_hGeneralPage;
	HPROPSHEETPAGE m_hFileSysPage;
	HPROPSHEETPAGE m_hIsoPage;
	HPROPSHEETPAGE m_hFieldsPage;
	HPROPSHEETPAGE m_hBootPage;
	HPROPSHEETPAGE m_hUdfPage;
	HPROPSHEETPAGE m_hAudioPage;

public:
	CProjectPropDlg();
	~CProjectPropDlg();

	BEGIN_MSG_MAP(CProjectPropDlg)
		MESSAGE_HANDLER(WM_SHOWWINDOW,OnShowWindow)
		MESSAGE_HANDLER(WM_SETFILESYSTEM,OnSetFileSystem);

		CHAIN_MSG_MAP(CPropertySheetImpl<CProjectPropDlg>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetFileSystem(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
