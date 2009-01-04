/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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
#include "ConfigGeneralPage.h"

class CConfigDlg : public CPropertySheetImpl<CConfigDlg>
{
private:
	bool m_bCentered;

	CConfigGeneralPage m_GeneralPage;

public:
	CConfigDlg(CEncoderConfig *pConfig);
	~CConfigDlg();

	BEGIN_MSG_MAP(CConfigDlg)
		MESSAGE_HANDLER(WM_SHOWWINDOW,OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheetImpl<CConfigDlg>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
