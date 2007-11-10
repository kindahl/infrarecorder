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

#include "stdafx.h"
#include "CustomPage.h"
#include "../../Common/GraphUtil.h"
#include "WinVer.h"

CCustomPage::CCustomPage()
{
	m_iHoverItem = -1;

	m_hBackgroundBrush = ::CreateSolidBrush(RGB(255,255,255));
	m_hButtonFont = AtlCreateBoldFont(AtlGetDefaultGuiFont());

	m_hWndReceiver = NULL;
}

CCustomPage::~CCustomPage()
{
	::DeleteObject(m_hBackgroundBrush);
	::DeleteObject(m_hButtonFont);
}

void CCustomPage::SetTitle(const TCHAR *szTitle)
{
	lstrcpy(m_szTitle,szTitle);
	uiTitleLength = lstrlen(m_szTitle);
}

void CCustomPage::SetReceiver(HWND hWndReceiver)
{
	m_hWndReceiver = hWndReceiver;
}

void CCustomPage::AddItem(CCustomPageItem *pItem)
{
	m_Items.push_back(pItem);
}

void CCustomPage::DrawButton(HDC hDC,RECT *pRect,bool bHover,bool bActive)
{
	if (bActive)
		DrawVertGradientRect(hDC,pRect,RGB(19,21,62),RGB(33,33,51));
	else
		DrawVertGradientRect(hDC,pRect,CUSTOMPAGE_BUTTON_TOPCOLOR,CUSTOMPAGE_BUTTON_BOTTOMCOLOR);

	// Draw the text.
	HFONT hOldFont = (HFONT)SelectObject(hDC,m_hButtonFont);

	::SetBkMode(hDC,TRANSPARENT);

	if (bHover)
		::SetTextColor(hDC,CUSTOMPAGE_BUTTON_TEXTHOVERCOLOR);
	else
		::SetTextColor(hDC,CUSTOMPAGE_BUTTON_TEXTCOLOR);

	DrawText(hDC,m_szTitle,uiTitleLength,pRect,DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);

	// Clean up.
	SelectObject(hDC,hOldFont);
}

void CCustomPage::DrawContents(HDC hDC,RECT *pRect)
{
	FillRect(hDC,pRect,m_hBackgroundBrush);

	// Draw the items.
	RECT rcItem;
	rcItem.left = pRect->left + CUSTOMPAGE_ITEM_SPACING;
	rcItem.top = pRect->top + CUSTOMPAGE_ITEM_SPACING;
	rcItem.right = pRect->right;
	rcItem.bottom = rcItem.top + CUSTOMPAGE_ITEM_HEIGHT;

	for (unsigned int i = 0; i < m_Items.size(); i++)
	{
		m_Items[i]->Draw(hDC,&rcItem,m_iHoverItem == i);

		rcItem.top += CUSTOMPAGE_ITEM_SPACING + CUSTOMPAGE_ITEM_HEIGHT;
		rcItem.bottom = rcItem.top + CUSTOMPAGE_ITEM_HEIGHT;
	}
}

void CCustomPage::OnMouseMove(int iX,int iY,HWND hWnd,RECT *pClientRect)
{
	int iItemTop = CUSTOMPAGE_ITEM_SPACING;
	int iItemBottom = iItemTop + CUSTOMPAGE_ITEM_HEIGHT;

	for (unsigned int i = 0; i < m_Items.size(); i++)
	{
		if (iY >= iItemTop && iY <= iItemBottom)
		{
			m_iHoverItem = i;

			InvalidateRect(hWnd,pClientRect,true);

			// Change cursor in Windows 2000 and newer systems only.
#ifdef VERSION_COMPATIBILITY_CHECK
			if (g_WinVer.m_ulMajorVersion >= MAJOR_WIN2000)
#endif
				SetCursor(LoadCursor(NULL,IDC_HAND));

			return;
		}

		iItemTop += CUSTOMPAGE_ITEM_SPACING + CUSTOMPAGE_ITEM_HEIGHT;
		iItemBottom = iItemTop + CUSTOMPAGE_ITEM_HEIGHT;
	}

	m_iHoverItem = -1;

	InvalidateRect(hWnd,pClientRect,true);

	// Change cursor in Windows 2000 and newer systems only.
#ifdef VERSION_COMPATIBILITY_CHECK
	if (g_WinVer.m_ulMajorVersion >= MAJOR_WIN2000)
#endif
		::SetCursor(LoadCursor(NULL,IDC_ARROW));
}

void CCustomPage::OnLButtonDown()
{
	if (m_iHoverItem != -1 && m_hWndReceiver != NULL)
		::SendMessage(m_hWndReceiver,WM_COMMAND,(WPARAM)m_Items[m_iHoverItem]->GetCommandID(),NULL);
}
