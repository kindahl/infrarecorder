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

#include "stdafx.h"
#include "LabelContainer.h"
#include <base/GraphUtil.h>
#include "CtrlMessages.h"
#include "WinVer.h"
#include "VisualStyles.h"
#include "Resource.h"
#include "CtrlMessages.h"

CLabelContainer::CLabelContainer(bool bClosable)
{
	m_iHeaderHeight = 0;

	m_hBorderBrush = ::CreateSolidBrush(LABELCONTAINER_COLOR_BORDER);
	m_szLabelText[0] = '\0';

	m_hWndCustomDraw = NULL;
	m_iControlID = -1;

	// Button releated.
	m_hCloseImageList = NULL;
	m_hWndCloseHost = NULL;
	if (bClosable)
		InitializeImageList();

	m_iButtonState = PANE_BUTTON_NORMAL;
	m_bButtonDown = false;
}

CLabelContainer::~CLabelContainer()
{
	if (m_hBorderBrush != NULL)
		::DeleteObject(m_hBorderBrush);

	// Destroy the image list.
	if (m_hCloseImageList != NULL)
		ImageList_Destroy(m_hCloseImageList);
}

void CLabelContainer::InitializeImageList()
{
	HBITMAP hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_PANECLOSEBITMAP));
	m_hCloseImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,4);
	ImageList_AddMasked(m_hCloseImageList,hBitmap,RGB(255,0,255));

	DeleteObject(hBitmap);
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

void CLabelContainer::SetCloseHost(HWND hWndCloseHost)
{
	m_hWndCloseHost = hWndCloseHost;
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
		DrawButton(hDC,&rcHeader);

		ReleaseDC(hDC);
	}
	else
	{
		CPaintDC dc(m_hWnd);

		DrawBackground(dc.m_hDC,&rcHeader);
		DrawText(dc.m_hDC,&rcHeader);
		DrawButton(dc.m_hDC,&rcHeader);

		ReleaseDC(dc);
	}

	return 0;
}

LRESULT CLabelContainer::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Only deal with this event if the panel is closable.
	if (m_hCloseImageList == NULL)
		return 0;

	POINT ptMouse = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

	RECT rcButton,rcClient;
	GetClientRect(&rcClient);

	rcButton.top = rcClient.top + LABELCONTAINER_BUTTON_TOPSPACING;
	rcButton.right = rcClient.right - LABELCONTAINER_BUTTON_RIGHTSPACING;
	rcButton.bottom = rcButton.top + 16;
	rcButton.left = rcButton.right - 16;

	int iNewState;
	if (::PtInRect(&rcButton,ptMouse))
	{
		SetCapture();
		if (m_iButtonState != PANE_BUTTON_DOWN)
			iNewState = PANE_BUTTON_HOVER;
		else
			iNewState = PANE_BUTTON_DOWN;
	}
	else
	{
		ReleaseCapture();
		iNewState = PANE_BUTTON_NORMAL;
	}

	if (iNewState != m_iButtonState)
	{
		m_iButtonState = iNewState;
		InvalidateRect(&rcButton);
	}

	return 0;
}

LRESULT CLabelContainer::OnMouseDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Only deal with this event if the panel is closable.
	if (m_hCloseImageList == NULL)
		return 0;

	POINT ptMouse = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

	RECT rcButton,rcClient;
	GetClientRect(&rcClient);

	rcButton.top = rcClient.top + LABELCONTAINER_BUTTON_TOPSPACING;
	rcButton.right = rcClient.right - LABELCONTAINER_BUTTON_RIGHTSPACING;
	rcButton.bottom = rcButton.top + 16;
	rcButton.left = rcButton.right - 16;

	if (::PtInRect(&rcButton,ptMouse))
	{
		m_iButtonState = PANE_BUTTON_DOWN;
		InvalidateRect(&rcButton);
	}

	return 0;
}

LRESULT CLabelContainer::OnMouseUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Only deal with this event if the panel is closable.
	if (m_hCloseImageList == NULL)
		return 0;

	POINT ptMouse = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

	RECT rcButton,rcClient;
	GetClientRect(&rcClient);

	rcButton.top = rcClient.top + LABELCONTAINER_BUTTON_TOPSPACING;
	rcButton.right = rcClient.right - LABELCONTAINER_BUTTON_RIGHTSPACING;
	rcButton.bottom = rcButton.top + 16;
	rcButton.left = rcButton.right - 16;

	if (::PtInRect(&rcButton,ptMouse))
	{
		if (m_iButtonState == PANE_BUTTON_DOWN && m_hWndCloseHost != NULL)
			::PostMessage(m_hWndCloseHost,WM_LABELCONTAINER_CLOSE,0,0);

		m_iButtonState = PANE_BUTTON_HOVER;
		InvalidateRect(&rcButton);
	}
	else
	{
		m_iButtonState = PANE_BUTTON_NORMAL;
		InvalidateRect(&rcButton);
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
	HFONT hOldFont = NULL;

	if (g_WinVer.m_ulMajorVersion == MAJOR_WINVISTA &&
		g_WinVer.m_ulMinorVersion == MINOR_WINVISTA &&
		g_VisualStyles.IsThemeActive())
	{
		hOldFont = (HFONT)SelectObject(dc,AtlGetDefaultGuiFont());

		::SetBkMode(dc,TRANSPARENT);
		::SetTextColor(dc,RGB(139,139,139));
	}
	else
	{
		HFONT hFont = AtlCreateBoldFont(AtlGetDefaultGuiFont());
		hOldFont = (HFONT)SelectObject(dc,hFont);

		::SetBkMode(dc,TRANSPARENT);
		::SetTextColor(dc,::GetSysColor(COLOR_BTNFACE));
	}

	RECT rcText;
	rcText.left = pHeaderRect->left + 3;
	rcText.right = pHeaderRect->right;
	rcText.top = pHeaderRect->top + LABELCONTAINER_BORDER_HEIGHT + 1;
	rcText.bottom = pHeaderRect->bottom;

	::DrawText(dc,m_szLabelText,lstrlen(m_szLabelText),&rcText,
		DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	if (hOldFont != NULL)
		SelectObject(dc,hOldFont);
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
	else if (true)
	{
		// Gradient background.
		rcMisc.top = pHeaderRect->top;
		rcMisc.bottom = pHeaderRect->bottom - LABELCONTAINER_BOTTOMBORDER_HEIGHT;
		DrawHorGradientRect(dc.m_hDC,&rcMisc,LABELCONTAINER_COLOR_BACKGROUNDALT,GetSysColor(COLOR_BTNFACE));
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

void CLabelContainer::DrawButton(CDCHandle dc,RECT *pHeaderRect)
{
	if (m_hCloseImageList != NULL)
	{
		ImageList_Draw(m_hCloseImageList,m_iButtonState,dc,
			pHeaderRect->right - 16 - LABELCONTAINER_BUTTON_RIGHTSPACING,
			LABELCONTAINER_BUTTON_TOPSPACING,ILD_TRANSPARENT);
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
