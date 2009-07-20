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
#include <ckmmc/device.hh>
#include "resource.h"

class CDeviceGeneralPage : public CPropertyPageImpl<CDeviceGeneralPage>
{
private:
	HICON m_hIcon;
	ckmmc::Device &m_Device;

	bool Translate();
	void PrintDeviceType(ckmmc::Device &Device);

public:
	enum { IDD = IDD_PROPPAGE_DEVICEGENERAL };

	CDeviceGeneralPage(ckmmc::Device &Device);
	~CDeviceGeneralPage();

	BEGIN_MSG_MAP(CDeviceGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		CHAIN_MSG_MAP(CPropertyPageImpl<CDeviceGeneralPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnWriteSpeedSpin(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
};
