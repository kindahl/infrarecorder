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
#include "Core2Device.h"
#include "Core2Info.h"

class CDiscGeneralPage : public CPropertyPageImpl<CDiscGeneralPage>
{
private:
	HICON m_hIcon;

	CCore2DeviceAddress *m_pDeviceAddress;
	TCHAR m_szDiscLabel[64];

	bool Translate();

	void DisplayDiscType(unsigned short usProfile);
	void DisplayBookType(unsigned char ucBookType,unsigned char ucBookRev);
	void DisplayStatus(CCore2DiscInfo *pDiscInfo);

public:
	enum { IDD = IDD_PROPPAGE_DISCGENERAL };

	CDiscGeneralPage(const TCHAR *szDiscLabel,CCore2DeviceAddress *pDeviceAddress);
	~CDiscGeneralPage();

	BEGIN_MSG_MAP(CDiscGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		CHAIN_MSG_MAP(CPropertyPageImpl<CDiscGeneralPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
