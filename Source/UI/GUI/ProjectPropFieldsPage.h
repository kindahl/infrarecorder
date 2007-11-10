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
#include "resource.h"

class CProjectPropFieldsPage : public CPropertyPageImpl<CProjectPropFieldsPage>
{
private:
	bool Translate();

public:
	enum { IDD = IDD_PROPPAGE_PROJECTPROPFIELDS };

	CProjectPropFieldsPage();
	~CProjectPropFieldsPage();

	bool OnApply();
	void OnHelp();

	BEGIN_MSG_MAP(CProjectPropFieldsPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		CHAIN_MSG_MAP(CPropertyPageImpl<CProjectPropFieldsPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
