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
#include "resource.h"
#include "TitleTipListViewCtrl.h"

class CDeviceAdvancedPage : public CPropertyPageImpl<CDeviceAdvancedPage>
{
private:
	UINT_PTR m_uiDeviceIndex;
	bool m_bLockAdvList;
	CTitleTipListViewCtrl m_ListView;

	bool Translate();

public:
	enum { IDD = IDD_PROPPAGE_DEVICEADVANCED };

	CDeviceAdvancedPage();
	~CDeviceAdvancedPage();

	void SetDeviceIndex(UINT_PTR uiDeviceIndex);

	BEGIN_MSG_MAP(CDeviceAdvancedPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		
		NOTIFY_HANDLER(IDC_ADVLIST,LVN_ITEMCHANGING,OnItemChanging)

		CHAIN_MSG_MAP(CPropertyPageImpl<CDeviceAdvancedPage>)
		CHAIN_MSG_MAP_MEMBER(m_ListView)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnItemChanging(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
};
