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
#include <vector>
#include "CustomPageItem.h"

#define CUSTOMPAGE_BUTTON_TOPCOLOR			RGB(25,28,52)
#define CUSTOMPAGE_BUTTON_BOTTOMCOLOR		RGB(41,41,41)
#define CUSTOMPAGE_BUTTON_TEXTCOLOR			RGB(255,255,255)
#define CUSTOMPAGE_BUTTON_TEXTHOVERCOLOR	RGB(183,165,217)
#define CUSTOMPAGE_BUTTON_WIDTH				80
#define CUSTOMPAGE_BUTTON_HEIGHT			40
#define CUSTOMPAGE_BUTTON_MAXTEXT			32
#define CUSTOMPAGE_ITEM_SPACING				12
#define CUSTOMPAGE_ITEM_HEIGHT				32

// HACK: Enable hand cursor on Windows 2000 an newer systems.
#if(WINVER < 0x0500)
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

class CCustomPage
{
private:
	int m_iHoverItem;

	HFONT m_hButtonFont;
	HBRUSH m_hBackgroundBrush;

	std::vector<CCustomPageItem *> m_Items;

	TCHAR m_szTitle[CUSTOMPAGE_BUTTON_MAXTEXT];
	unsigned int uiTitleLength;

	HWND m_hWndReceiver;

public:
	CCustomPage();
	~CCustomPage();

	void SetTitle(const TCHAR *szTitle);
	void SetReceiver(HWND hWndReceiver);

	void AddItem(CCustomPageItem *pItem);

	void DrawButton(HDC hDC,RECT *pRect,bool bHover,bool bActive);
	void DrawContents(HDC hDC,RECT *pRect);

	void OnMouseMove(int iX,int iY,HWND hWnd,RECT *pClientRect);
	void OnLButtonDown();
};
