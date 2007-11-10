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
#include "LabelContainer.h"
#include "../../Common/GraphUtil.h"
#include "CtrlMessages.h"
#include "WinVer.h"
#include "VisualStyles.h"

CLabelContainer::CLabelContainer()
{
	m_iHeaderHeight = 0;

	m_hBorderBrush = ::CreateSolidBrush(LABELCONTAINER_COLOR_BORDER);
	m_szLabelText[0] = '\0';

	m_hWndCustomDraw = NULL;
	m_iControlID = -1;
}

CLabelContainer::~CLabelContainer()
{
	::DeleteObject(m_hBorderBrush);
}

void CLabelContainer::SetCustomDrawHandler(HWND hWndCustomDraw,int iID)
{
	m_hWndCustomDraw = hWndCustomDraw;
	m_iControlID = iID;
}

void CLabelContainer::SetClient(HWND hWndClient)
{
	m_ClientWindow = hWndClient;

	UpdateLayout();
}

void CLabelContainer::UpdateLayout()
{
	RECT rcClient;
	GetClientRect(&rcClient);

	UpdateLayout(rcClient.right,rcClient.bottom);
}

void CLabelContainer::UpdateLayout(int iWidth,int iHeight)
{
	RECT rcHeader = { 0,0,iWidth,m_iHeaderHeight };

	if (m_ClientWindow.m_hWnd != NULL)
		m_ClientWindow.SetWindowPos(NULL,0,m_iHeaderHeight,iWidth,iHeight - m_iHeaderHeight,SWP_NOZORDER);
	else
		rcHeader.bottom = iHeight;

	InvalidateRect(&rcHeader);
}

LRESULT CLabelContainer::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	UpdateLayout(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
	return 0;
}

LRESULT CLabelContainer::OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_ClientWindow.m_hWnd != NULL)
		m_ClientWindow.SetFocus();

	return 0;
}

LRESULT CLabelContainer::OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	return TRUE;
}

LRESULT CLabelContainer::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	RECT rcHeader;
	GetClientRect(&rcHeader);

	rcHeader.bottom = rcHeader.top + m_iHeaderHeight;

	if (wParam != NULL)
	{
		HDC hDC = (HDC)wParam;

		DrawBackground(hDC,&rcHeader);
		DrawText(hDC,&rcHeader);

		ReleaseDC(hDC);
	}
	else
	{
		CPaintDC dc(m_hWnd);

		DrawBackground(dc.m_hDC,&rcHeader);
		DrawText(dc.m_hDC,&rcHeader);

		ReleaseDC(dc);
	}

	return 0;
}

LRESULT CLabelContainer::OnCustomDraw(int idCtrl,LPNMHDR pnmh,BOOL &bHandled)
{
	if (m_hWndCustomDraw != NULL && idCtrl == m_iControlID)
		return ::SendMessage(m_hWndCustomDraw,WM_CONTROLCUSTOMDRAW,0,(LPARAM)pnmh);
		
	bHandled = false;
	return CDRF_DODEFAULT;
}

void CLabelContainer::DrawText(CDCHandle dc,RECT *pHeaderRect)
{
	if (g_WinVer.m_ulMajorVersion == MAJOR_WINVISTA &&
		g_WinVer.m_ulMinorVersion == MINOR_WINVISTA &&
		g_VisualStyles.IsThemeActive())
	{
		HFONT hOldFont = (HFONT)SelectObject(dc,AtlGetDefaultGuiFont());

		::SetBkMode(dc,TRANSPARENT);
		::SetTextColor(dc,RGB(139,139,139));

		RECT rcText;
		rcText.left = pHeaderRect->left + 3;
		rcText.right = pHeaderRect->right;
		rcText.top = pHeaderRect->top + LABELCONTAINER_BORDER_HEIGHT + 1;
		rcText.bottom = pHeaderRect->bottom;

		::DrawText(dc,m_szLabelText,lstrlen(m_szLabelText),&rcText,
			DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

		SelectObject(dc,hOldFont);
	}
	else
	{
		HFONT hFont = AtlCreateBoldFont(AtlGetDefaultGuiFont());
		HFONT hOldFont = (HFONT)SelectObject(dc,hFont);

		::SetBkMode(dc,TRANSPARENT);
		::SetTextColor(dc,LABELCONTAINER_COLOR_BORDER);

		RECT rcText;
		rcText.left = pHeaderRect->left + 3;
		rcText.right = pHeaderRect->right;
		rcText.top = pHeaderRect->top + LABELCONTAINER_BORDER_HEIGHT + 1;
		rcText.bottom = pHeaderRect->bottom;

		::DrawText(dc,m_szLabelText,lstrlen(m_szLabelText),&rcText,
			DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

		SelectObject(dc,hOldFont);
		DeleteObject(hFont);
	}
}

void CLabelContainer::DrawBackground(CDCHandle dc,RECT *pHeaderRect)
{
	RECT rcMisc;
	rcMisc.left = pHeaderRect->left;
	rcMisc.right = pHeaderRect->right;

	// Bottom border.
	rcMisc.top = pHeaderRect->bottom - LABELCONTAINER_BOTTOMBORDER_HEIGHT;
	rcMisc.bottom = pHeaderRect->bottom;
	FillRect(dc,&rcMisc,GetSysColorBrush(COLOR_BTNFACE));

	if (g_WinVer.m_ulMajorVersion == MAJOR_WINVISTA &&
		g_WinVer.m_ulMinorVersion == MINOR_WINVISTA &&
		g_VisualStyles.IsThemeActive())
	{
		// Gradient background.
		rcMisc.top = pHeaderRect->top;
		rcMisc.bottom = pHeaderRect->bottom - LABELCONTAINER_BOTTOMBORDER_HEIGHT;
		DrawHorGradientRect(dc.m_hDC,&rcMisc,LABELCONTAINER_COLOR_BACKGROUNDVISTA,GetSysColor(COLOR_BTNFACE));
	}
	else
	{
		// Gradient background.
		rcMisc.left = LABELCONTAINER_BORDER_HEIGHT;
		rcMisc.top = pHeaderRect->top + LABELCONTAINER_BORDER_HEIGHT;
		rcMisc.bottom = pHeaderRect->bottom - LABELCONTAINER_BOTTOMBORDER_HEIGHT - LABELCONTAINER_BORDER_HEIGHT;
		DrawHorGradientRect(dc.m_hDC,&rcMisc,LABELCONTAINER_COLOR_BACKGROUND,GetSysColor(COLOR_BTNFACE));

		// Border lines.
		rcMisc.left = pHeaderRect->left;

		rcMisc.top = pHeaderRect->top;
		rcMisc.bottom = pHeaderRect->top + LABELCONTAINER_BORDER_HEIGHT;
		DrawHorGradientRect(dc.m_hDC,&rcMisc,LABELCONTAINER_COLOR_BORDER,GetSysColor(COLOR_BTNFACE));

		rcMisc.top = pHeaderRect->bottom - LABELCONTAINER_BORDER_HEIGHT - LABELCONTAINER_BOTTOMBORDER_HEIGHT;
		rcMisc.bottom = pHeaderRect->bottom - LABELCONTAINER_BOTTOMBORDER_HEIGHT;
		DrawHorGradientRect(dc.m_hDC,&rcMisc,LABELCONTAINER_COLOR_BORDER,GetSysColor(COLOR_BTNFACE));

		rcMisc.top = pHeaderRect->top;
		rcMisc.bottom = pHeaderRect->bottom - LABELCONTAINER_BOTTOMBORDER_HEIGHT;
		rcMisc.right = pHeaderRect->left + LABELCONTAINER_BORDER_HEIGHT;
		FillRect(dc,&rcMisc,m_hBorderBrush);
	}
}

void CLabelContainer::SetHeaderHeight(int iHeight)
{
	m_iHeaderHeight = iHeight;
}

void CLabelContainer::SetLabelText(const TCHAR *szText)
{
	lstrcpy(m_szLabelText,szText);
}
