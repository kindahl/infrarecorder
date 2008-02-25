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
#include "CustomToolBarCtrl.h"
#include "MainFrm.h"

CCustomToolBarCtrl::CCustomToolBarCtrl()
{
}

CCustomToolBarCtrl::~CCustomToolBarCtrl()
{
}

LRESULT CCustomToolBarCtrl::OnRButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	POINT ptCursor;
	GetCursorPos(&ptCursor);

	HMENU hToolBarsMenu = g_MainFrame.GetToolBarsMenu();

	// Force the popup menu items to update. This is needed since WTL seems to be lazy
	// and only want to refresh the corresponding main menu items when needed.
	CMainFrame::_AtlUpdateUIData UIData;
	UIData.m_lpData = NULL;

	UIData.m_wState = (WORD)g_MainFrame.UIGetState(ID_VIEW_STANDARDTOOLBAR);
	g_MainFrame.UIUpdateMenuBarElement(ID_VIEW_STANDARDTOOLBAR,&UIData,hToolBarsMenu);

	// Show the popup menu.
	TrackPopupMenuEx(hToolBarsMenu,0,ptCursor.x,ptCursor.y,m_hWnd,NULL);

	bHandled = false;
	return 0;
}

LRESULT CCustomToolBarCtrl::OnLButtonDblClk(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// We don't want to display the original customize toolbar dialog when the
	// toolbar is double-clicked on.
	bHandled = true;
	return 0;
}
