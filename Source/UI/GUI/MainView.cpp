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
#include "MainView.h"
#include "VisualStyles.h"

CMainView::CMainView()
{
	// Use the themed border size by default
	m_uiBorderSize = MAINVIEW_THEMEDBORDER_SIZE;
}

CMainView::~CMainView()
{
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

	return 0;
}

LRESULT CMainView::OnNCPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	RECT rcClient;

	// Get the window rect and convert it's origin to 0,0 instead of the top
	// left of the screen.
	GetWindowRect(&rcClient);
	rcClient.bottom -= rcClient.top;
	rcClient.top = 0;
	rcClient.right -= rcClient.left;
	rcClient.left = 0;

	HDC hDC = GetWindowDC();
		// Since FillRect is used, the center needs to be excluded.
		// Otherwise there will be some flicker and drawing issues.
		ExcludeClipRect(hDC,rcClient.left + m_uiBorderSize,
			rcClient.top + m_uiBorderSize,
			rcClient.right - m_uiBorderSize,
			rcClient.bottom);
		FillRect(hDC,&rcClient,GetSysColorBrush(COLOR_BTNFACE));
	ReleaseDC(hDC);

	bHandled = false;
	return 0;
}
