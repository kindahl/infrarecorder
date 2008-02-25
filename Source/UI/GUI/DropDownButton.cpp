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

#include "stdafx.h"
#include "DropDownButton.h"

CDropDownButton::CDropDownButton(unsigned int uiMenuID,bool bDrawArrow)
{
	m_bDrawArrow = bDrawArrow;

	// Menu.
	m_hMenu = LoadMenu(_Module.GetResourceInstance(),MAKEINTRESOURCE(uiMenuID));

	// Font used for the arrow (if unthemed only).
	CFont apa;
	CLogFont lf = AtlGetDefaultGuiFont();
	lstrcpy(lf.lfFaceName,_T("Webdings"));
	lf.lfCharSet = SYMBOL_CHARSET;
	lf.lfHeight = 18;

	m_hWebdingsFont = CreateFontIndirect(&lf);

	// Theme data.
	m_hTheme = NULL;
}

CDropDownButton::~CDropDownButton()
{
	// Menu.
	DestroyMenu(m_hMenu);

	// Arrow font.
	DeleteObject(m_hWebdingsFont);

	// Unload theme data.
	if (m_hTheme != NULL)
		g_VisualStyles.CloseThemeData(m_hTheme);
}

HMENU CDropDownButton::GetMenu()
{
	return m_hMenu;
}

BOOL CDropDownButton::SubclassWindow(HWND hWnd)
{
	BOOL bResult = CWindowImpl<CDropDownButton,CButton>::SubclassWindow(hWnd);

	if (m_bDrawArrow)
	{
		SetButtonStyle(g_VisualStyles.IsAppThemed() ? BS_PUSHBUTTON : BS_OWNERDRAW);

		// Load theme data.
		if (g_VisualStyles.IsAppThemed())
			m_hTheme = g_VisualStyles.OpenThemeData(m_hWnd,L"TOOLBAR");
	}

	return bResult;
}

void CDropDownButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDCHandle dc = lpDIS->hDC;
	RECT rcItem = lpDIS->rcItem;
	bool bEnabled = (lpDIS->itemState & ODS_DISABLED) == 0;
	bool bFocused = (lpDIS->itemState & ODS_FOCUS) != 0;

	// Draw the button.
	RECT rcButton = rcItem;
	if (bFocused)
	{
		dc.Draw3dRect(&rcButton,::GetSysColor(COLOR_3DDKSHADOW),::GetSysColor(COLOR_3DDKSHADOW));
		::InflateRect(&rcButton,-1, -1);
	}

	unsigned int uiState = 0;
	if (lpDIS->itemState & ODS_SELECTED)
	{
		uiState |= DFCS_PUSHED;

		::OffsetRect(&rcItem,1,1);
	}

	if (!bEnabled)
		uiState |= DFCS_INACTIVE;

	::DrawFrameControl(dc,&rcButton,DFC_BUTTON,DFCS_BUTTONPUSH | uiState);

	// Draw the text.
	TCHAR szText[DROPDOWNBUTTON_MAX_TEXT_SIZE];
	GetWindowText(szText,DROPDOWNBUTTON_MAX_TEXT_SIZE - 1);

	HFONT hOldFont = dc.SelectFont(GetFont());
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(::GetSysColor(bEnabled ? COLOR_BTNTEXT : COLOR_GRAYTEXT));

	if (!bEnabled)
	{
		RECT rcText = rcItem;
		RECT rcBound = rcText;
		dc.DrawText(szText,-1,&rcBound,DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT);
		::OffsetRect(&rcText,(rcText.right - rcBound.right) / 2, (rcText.bottom - rcBound.bottom) / 2);
		::DrawState(dc,NULL,NULL,(LPARAM)szText,0,rcText.left,rcText.top,rcText.right,rcText.bottom,DST_PREFIXTEXT | DSS_DISABLED);
	}
	else
	{
		RECT rcText = rcItem;
		dc.DrawText(szText,-1,&rcText,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	// Draw the separator.
	RECT rcSeparator = { rcItem.right - 22,rcItem.top + 5,rcItem.right - 20,rcItem.bottom - 5 };
	dc.Draw3dRect(&rcSeparator,::GetSysColor(COLOR_BTNSHADOW),::GetSysColor(COLOR_BTNHIGHLIGHT));
	dc.SelectFont(m_hWebdingsFont);

	RECT rcArrow = { rcItem.right - 18,rcItem.top,rcItem.right,rcItem.bottom };
	dc.DrawText(_T("\x36"),1,&rcArrow,DT_SINGLELINE | DT_LEFT | DT_VCENTER);

	HBRUSH hOldBrush = dc.SelectBrush((HBRUSH)::GetStockObject(HOLLOW_BRUSH));
	::InflateRect(&rcButton,-3,-3);

	// Draw the focus rectangle.
	if (bFocused)
		dc.DrawFocusRect(&rcButton);

	dc.SelectBrush(hOldBrush);
	dc.SelectFont(hOldFont);
}

DWORD CDropDownButton::OnPrePaint(int idCtrl,LPNMCUSTOMDRAW lpNMCD)
{
	return CDRF_NOTIFYPOSTPAINT;
}

DWORD CDropDownButton::OnPostPaint(int idCtrl,LPNMCUSTOMDRAW lpNMCD)
{
	CDCHandle dc = lpNMCD->hdc;
	RECT rcButton = lpNMCD->rc;
	bool bEnabled = (lpNMCD->uItemState & (CDIS_DISABLED | CDIS_GRAYED)) == 0;

	// Draw the separator
	RECT rcSeparator = { rcButton.right - 21,rcButton.top + 4,rcButton.right - 19,rcButton.bottom - 4 };
	g_VisualStyles.DrawThemeBackground(m_hTheme,dc,TP_SEPARATOR,TS_NORMAL,&rcSeparator,NULL);

	// Draw the arrow.
	RECT rcArrow = { rcButton.right - 22,rcButton.top,rcButton.right,rcButton.bottom };
	g_VisualStyles.DrawThemeBackground(m_hTheme,dc,	TP_SPLITBUTTONDROPDOWN,bEnabled ? TS_NORMAL : TS_DISABLED,&rcArrow,NULL);
	return CDRF_DODEFAULT;
}

LRESULT CDropDownButton::OnClicked(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	RECT rcButton;
	GetWindowRect(&rcButton);

	int iID = TrackPopupMenuEx(GetSubMenu(m_hMenu,0),TPM_NONOTIFY | TPM_RETURNCMD,
		rcButton.left,rcButton.bottom,m_hWnd,NULL);
	::PostMessage(GetParent(),WM_COMMAND,iID,0);

	bHandled = false;
	return 0;
}
