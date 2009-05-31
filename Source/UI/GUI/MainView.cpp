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

#include "stdafx.h"
#include "VisualStyles.h"
#include "resource.h"
#include "MainView.h"

CMainView::CMainView() : m_uiBorderSize(MAINVIEW_THEMEDBORDER_SIZE),
	m_bHintBar(false),m_HintType(HT_INFORMATION),
	m_ButtonState(BS_NORMAL),
	m_hIconImageList(NULL),m_hCloseImageList(NULL)
{
	// Fill the icon image list.
	m_hIconImageList = ImageList_Create(16,16,ILC_COLOR32,0,4);

	ImageList_AddIcon(m_hIconImageList,LoadIcon(NULL,IDI_INFORMATION));
	ImageList_AddIcon(m_hIconImageList,LoadIcon(NULL,IDI_WARNING));
	ImageList_AddIcon(m_hIconImageList,LoadIcon(NULL,IDI_ERROR));
	ImageList_AddIcon(m_hIconImageList,LoadIcon(NULL,IDI_WINLOGO));

	// Fill the close image list.
	HBITMAP hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_PANECLOSEBITMAP));

	m_hCloseImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,4);
	ImageList_AddMasked(m_hCloseImageList,hBitmap,RGB(255,0,255));

	DeleteObject(hBitmap);
}

CMainView::~CMainView()
{
	if (m_hIconImageList != NULL)
		ImageList_Destroy(m_hIconImageList);

	if (m_hCloseImageList != NULL)
		ImageList_Destroy(m_hCloseImageList);
}

LRESULT CMainView::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// If the application is themed, we use a different border size.
	if (!g_VisualStyles.IsThemeActive())
		m_uiBorderSize = MAINVIEW_NORMALBORDER_SIZE;

	return 0;
}

LRESULT CMainView::OnNCCalcSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	RECT *pRect = (RECT *)lParam;

	// Contract all size of the rectangle except for the bottom. The reason for
	// this is that the space meter and this view is separated by a splitter.
	pRect->left += m_uiBorderSize;
	pRect->right -= m_uiBorderSize;
	pRect->top += m_uiBorderSize;

	// Let the default window proc adjust the rect further since there might be
	// a scrollbar visible.
	//DefWindowProc(uMsg,wParam,lParam);

	if (m_bHintBar)
		pRect->top += MAINVIEW_HINTBAR_SIZE;

	return 0;
}

LRESULT CMainView::OnNCPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	RECT rcClient;

	// Get the window rect and convert it's origin to 0,0 instead of the top
	// left of the screen.
	GetWindowRect(&rcClient);

	HDC hDC = GetWindowDC();

		rcClient.bottom -= rcClient.top;
		rcClient.top = 0;
		rcClient.right -= rcClient.left;
		rcClient.left = 0;
	
		// Since FillRect is used, the center needs to be excluded.
		// Otherwise there will be some flicker and drawing issues.
		ExcludeClipRect(hDC,rcClient.left + m_uiBorderSize,
			rcClient.top + m_uiBorderSize + (m_bHintBar ? MAINVIEW_HINTBAR_SIZE : 0),
			rcClient.right - m_uiBorderSize,
			rcClient.bottom);
		FillRect(hDC,&rcClient,GetSysColorBrush(COLOR_BTNFACE));

		if (m_bHintBar)
		{
			RECT rcHintBar = rcClient;
			rcHintBar.bottom = rcHintBar.top + MAINVIEW_HINTBAR_SIZE;

			DrawHintBar(hDC,rcHintBar);
		}

	ReleaseDC(hDC);

	bHandled = false;
	return 0;
}

LRESULT CMainView::OnNCMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Only deal with this event if the hint bar is visible.
	if (!m_bHintBar)
		return 0;

	POINT ptMouse = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

	RECT rcButton,rcWindow;
	GetWindowRect(&rcWindow);

	RECT rcHintBar = rcWindow;
	rcHintBar.bottom = rcHintBar.top + MAINVIEW_HINTBAR_SIZE;

	rcButton.top = rcHintBar.top + MAINVIEW_HINTBAR_SIZE/2 - 8;
	rcButton.right = rcHintBar.right - (MAINVIEW_HINTBAR_SIZE/2 - 8);
	rcButton.bottom = rcButton.top + 16;
	rcButton.left = rcButton.right - 16;

	eButtonState NewState;
	if (::PtInRect(&rcButton,ptMouse))
	{
		if (m_ButtonState != BS_DOWN)
			NewState = BS_HOVER;
		else
			NewState = BS_DOWN;
	}
	else
	{
		NewState = BS_NORMAL;
	}

	if (NewState != m_ButtonState)
	{
		m_ButtonState = NewState;
		InvalidateRect(&rcButton);

		SetWindowPos(NULL,0,0,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE |
					 SWP_DRAWFRAME);
	}

	return 0;
}

LRESULT CMainView::OnNCMouseDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Only deal with this event if the hint bar is visible.
	if (!m_bHintBar)
		return 0;

	POINT ptMouse = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

	RECT rcButton,rcWindow;
	GetWindowRect(&rcWindow);

	RECT rcHintBar = rcWindow;
	rcHintBar.bottom = rcHintBar.top + MAINVIEW_HINTBAR_SIZE;

	rcButton.top = rcHintBar.top + MAINVIEW_HINTBAR_SIZE/2 - 8;
	rcButton.right = rcHintBar.right - (MAINVIEW_HINTBAR_SIZE/2 - 8);
	rcButton.bottom = rcButton.top + 16;
	rcButton.left = rcButton.right - 16;

	if (::PtInRect(&rcButton,ptMouse))
	{
		m_ButtonState = BS_DOWN;
		InvalidateRect(&rcButton);

		SetWindowPos(NULL,0,0,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE |
					 SWP_DRAWFRAME);
	}

	return 0;
}

LRESULT CMainView::OnNCMouseUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Only deal with this event if the hint bar is visible.
	if (!m_bHintBar)
		return 0;

	POINT ptMouse = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

	RECT rcButton,rcWindow;
	GetWindowRect(&rcWindow);

	RECT rcHintBar = rcWindow;
	rcHintBar.bottom = rcHintBar.top + MAINVIEW_HINTBAR_SIZE;

	rcButton.top = rcHintBar.top + MAINVIEW_HINTBAR_SIZE/2 - 8;
	rcButton.right = rcHintBar.right - (MAINVIEW_HINTBAR_SIZE/2 - 8);
	rcButton.bottom = rcButton.top + 16;
	rcButton.left = rcButton.right - 16;

	if (::PtInRect(&rcButton,ptMouse))
	{
		if (m_ButtonState == BS_DOWN)
			HideHintMsg();

		m_ButtonState = BS_HOVER;
		InvalidateRect(&rcButton);

		SetWindowPos(NULL,0,0,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE |
					 SWP_DRAWFRAME);
	}
	else
	{
		m_ButtonState = BS_NORMAL;
		InvalidateRect(&rcButton);

		SetWindowPos(NULL,0,0,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE |
					 SWP_DRAWFRAME);
	}

	return 0;
}

LRESULT CMainView::OnNCHitTest(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	return HTBORDER;
}

void CMainView::DrawHintBar(HDC hDC,RECT &rcHintBar)
{
	// Draw the background.
	FillRect(hDC,&rcHintBar,GetSysColorBrush(COLOR_INFOBK));

	// Draw background highlight.
	RECT rcHighlight = rcHintBar;
	rcHighlight.bottom = rcHighlight.top + 1;
	FillRect(hDC,&rcHighlight,GetSysColorBrush(COLOR_WINDOW));

	rcHighlight.bottom = rcHintBar.bottom;
	rcHighlight.right = rcHighlight.left + 1;
	FillRect(hDC,&rcHighlight,GetSysColorBrush(COLOR_WINDOW));

	// Draw background shadow.
	RECT rcShadow = rcHintBar;
	rcShadow.top = rcShadow.bottom - 1;
	FillRect(hDC,&rcShadow,GetSysColorBrush(COLOR_3DLIGHT));

	rcShadow.top = rcHintBar.top;
	rcShadow.left = rcShadow.right - 1;
	FillRect(hDC,&rcShadow,GetSysColorBrush(COLOR_3DLIGHT));

	// Draw icon.
	ImageList_Draw(m_hIconImageList,static_cast<int>(m_HintType),hDC,
				   MAINVIEW_HINTBAR_SIZE/2 - 8,
				   MAINVIEW_HINTBAR_SIZE/2 - 8,ILD_TRANSPARENT);

	// Draw the close button.
	ImageList_Draw(m_hCloseImageList,static_cast<int>(m_ButtonState),hDC,
			rcHintBar.right - MAINVIEW_HINTBAR_SIZE/2 - 8,
			MAINVIEW_HINTBAR_SIZE/2 - 8,ILD_TRANSPARENT);

	// Draw text.
	RECT rcText;
	rcText.left = rcHintBar.left + 16 + (MAINVIEW_HINTBAR_SIZE/2 - 8) * 2;
	rcText.right = rcHintBar.right;
	rcText.top = rcHintBar.top;
	rcText.bottom = rcHintBar.bottom;

	SetBkColor(hDC,GetSysColor(COLOR_INFOBK));
	SetTextColor(hDC,GetSysColor(COLOR_INFOTEXT));

	HFONT hOldFont = (HFONT)SelectObject(hDC,AtlGetDefaultGuiFont());

	DrawText(hDC,m_HintMsg.c_str(),m_HintMsg.size(),&rcText,
			 DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);

	if (hOldFont != NULL)
		SelectObject(hDC,hOldFont);
}

void CMainView::ShowHintMsg(eHintType HintType,const TCHAR *szHintMsg)
{
	m_bHintBar = true;

	m_HintType = HintType;
	m_HintMsg = szHintMsg;

	m_ButtonState = BS_NORMAL;

	SetWindowPos(NULL,0,0,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE |
				 SWP_FRAMECHANGED);

	PlaySound(_T("SystemNotification"),0,SND_ASYNC | SND_ALIAS | SND_NOWAIT);
}

void CMainView::HideHintMsg()
{
	m_bHintBar = false;

	SetWindowPos(NULL,0,0,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE |
				 SWP_FRAMECHANGED);
}
