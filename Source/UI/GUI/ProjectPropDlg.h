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
#include "ProjectPropGeneralPage.h"
#include "ProjectPropFileSysPage.h"
#include "ProjectPropISOPage.h"
#include "ProjectPropFieldsPage.h"
#include "ProjectPropAudioPage.h"
#include "ProjectPropBootPage.h"
#include "CtrlMessages.h"

class CProjectPropDlg : public CPropertySheetImpl<CProjectPropDlg>
{
private:
	bool m_bCentered;

	CProjectPropGeneralPage m_GeneralPage;
	CProjectPropFileSysPage m_FileSysPage;
	CProjectPropISOPage m_IsoPage;
	CProjectPropFieldsPage m_FieldsPage;
	CProjectPropAudioPage m_AudioPage;
	CProjectPropBootPage m_BootPage;

	HPROPSHEETPAGE m_hGeneralPage;
	HPROPSHEETPAGE m_hFileSysPage;
	HPROPSHEETPAGE m_hIsoPage;
	HPROPSHEETPAGE m_hFieldsPage;
	HPROPSHEETPAGE m_hAudioPage;
	HPROPSHEETPAGE m_hBootPage;

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
