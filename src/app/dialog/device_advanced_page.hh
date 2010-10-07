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
#include <ckmmc/device.hh>
#include "Resource.h"
#include "title_tip_list_view_ctrl.hh"

class CDeviceAdvancedPage : public CPropertyPageImpl<CDeviceAdvancedPage>
{
private:
	ckmmc::Device &m_Device;
	bool m_bLockAdvList;
	CTitleTipListViewCtrl m_ListView;

	bool Translate();

public:
	enum { IDD = IDD_PROPPAGE_DEVICEADVANCED };

	CDeviceAdvancedPage(ckmmc::Device &Device);
	~CDeviceAdvancedPage();

	BEGIN_MSG_MAP(CDeviceAdvancedPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		
		NOTIFY_HANDLER(IDC_ADVLIST,LVN_ITEMCHANGING,OnItemChanging)

		CHAIN_MSG_MAP(CPropertyPageImpl<CDeviceAdvancedPage>)
		CHAIN_MSG_MAP_MEMBER(m_ListView)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnItemChanging(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
};
