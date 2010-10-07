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
#include "resource.h"

class CProjectPropIsoPage : public CPropertyPageImpl<CProjectPropIsoPage>
{
private:
	CComboBox m_LevelCombo;
	CComboBox m_FormatCombo;

	bool Translate();

public:
	enum { IDD = IDD_PROPPAGE_PROJECTPROPISO };

	CProjectPropIsoPage();
	~CProjectPropIsoPage();

	bool OnApply();
	void OnHelp();

	BEGIN_MSG_MAP(CProjectPropIsoPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_COMMAND,OnCommand)

		CHAIN_MSG_MAP(CPropertyPageImpl<CProjectPropIsoPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
