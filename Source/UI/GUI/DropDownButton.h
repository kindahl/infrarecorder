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

#include <atlcrack.h>	// BEGIN_MSG_MAP_EX
#include "VisualStyles.h"

#define DROPDOWNBUTTON_MAX_TEXT_SIZE			64

class CDropDownButton : public CWindowImpl<CDropDownButton,CButton>,
	public COwnerDraw<CDropDownButton>,
	public CCustomDraw<CDropDownButton>
{
private:
	bool m_bDrawArrow;
	HMENU m_hMenu;
	HFONT m_hWebdingsFont;
	HTHEME m_hTheme;

public:
	CDropDownButton(unsigned int uiMenuID,bool bDrawArrow);
	~CDropDownButton();

	HMENU GetMenu();
	BOOL SubclassWindow(HWND hWnd);
	void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	DWORD OnPrePaint(int idCtrl,LPNMCUSTOMDRAW lpNMCD);
	DWORD OnPostPaint(int idCtrl,LPNMCUSTOMDRAW lpNMCD);

#if _ATL_VER <= 0x0300
	BEGIN_MSG_MAP_EX(CDropDownButton)
#else
	BEGIN_MSG_MAP(CDropDownButton)
#endif
		REFLECTED_COMMAND_CODE_HANDLER(BN_CLICKED,OnClicked)

		 CHAIN_MSG_MAP_ALT(COwnerDraw<CDropDownButton>,1)
		 CHAIN_MSG_MAP_ALT(CCustomDraw<CDropDownButton>,1)
	END_MSG_MAP()

	LRESULT OnClicked(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
