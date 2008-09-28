/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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
#include "TitleTipListViewCtrl.h"
#include "../../Common/StringUtil.h"

/*
	IMPORTANT: Requires host to chain the message loop.
*/

CTitleTipListViewCtrl::CTitleTipListViewCtrl()
{
	m_iCurrentItem = -1;
	m_bTrackingMouseLeave = false;
}

CTitleTipListViewCtrl::~CTitleTipListViewCtrl()
{
	if (m_ToolTip.IsWindow())
		m_ToolTip.DestroyWindow();
}

bool CTitleTipListViewCtrl::Initialize()
{
	m_ToolTip.Create(NULL,CWindow::rcDefault,NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,WS_EX_TOPMOST);

	if (!m_ToolTip.IsWindow())
		return false;

	// Fill the tool information structure.
	::ZeroMemory(&m_ti,sizeof(m_ti));
	m_ti.cbSize = sizeof(TOOLINFO);
	m_ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	m_ti.hwnd = GetParent();
	m_ti.hinst = _Module.GetResourceInstance();
	m_ti.uId = (unsigned int)m_hWnd;
	m_ti.lpszText = LPSTR_TEXTCALLBACK;

	m_ToolTip.AddTool(&m_ti);
	m_ToolTip.TrackActivate(&m_ti,FALSE);

	return true;
}

LRESULT CTitleTipListViewCtrl::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	bHandled = false;

	// Make sure that the control has been initialized.
	if (!m_ToolTip.IsWindowEnabled())
		return 0;

	// Track the mouse to see when it leaves the list view.
	if (!m_bTrackingMouseLeave)
	{
         TRACKMOUSEEVENT tme = { 0 };
         tme.cbSize = sizeof(TRACKMOUSEEVENT);
         tme.dwFlags = TME_LEAVE;
         tme.hwndTrack = m_hWnd;

		 TrackMouseEvent(&tme);
         m_bTrackingMouseLeave = true;
      }

	POINT ptCursor = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

	unsigned int uiFlags = 0;
	int iItem = HitTest(ptCursor,&uiFlags);

	// Check if whe're hovering above an item.
	if (iItem != -1)
	{
		RECT rcItem;
		GetItemRect(iItem,&rcItem,LVIR_SELECTBOUNDS);

		RECT rcListView;
		GetClientRect(&rcListView);

		// Check if the item text is to long to be fully displayed.
		if (rcItem.right >= rcListView.right)
		{
			// Is the tool tip already visible?
			if (m_ToolTip.GetCurrentTool(NULL) != 0)
			{
				if (m_iCurrentItem != iItem)
				{
					m_iCurrentItem = iItem;
					m_ToolTip.TrackActivate(&m_ti,TRUE);
				}

				ClientToScreen(&rcItem);
				ClientToScreen(&ptCursor);
				m_ToolTip.TrackPosition(rcItem.left,rcItem.top);
			}
			else
			{
				m_ToolTip.TrackActivate(&m_ti,TRUE);
				m_iCurrentItem = iItem;
			}
		}
		else
		{
			// Do not display the tool tip.
			m_ToolTip.TrackActivate(&m_ti,FALSE);
		}
	}
	else
	{
		// Do not display the tool tip.
		m_ToolTip.TrackActivate(&m_ti,FALSE);
	}

	return 0;
}

LRESULT CTitleTipListViewCtrl::OnMouseLeave(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_bTrackingMouseLeave = false;

	RECT rcListView;
	GetClientRect(&rcListView);
	ClientToScreen(&rcListView);

	POINT ptCursor;
	GetCursorPos(&ptCursor);

	// Check if the cursor has left the list view control.
	if (!PtInRect(&rcListView,ptCursor))
	{
		if (m_ToolTip.IsWindow())
			m_ToolTip.TrackActivate(&m_ti,FALSE);
	}

	bHandled = false;
	return 0;
}

LRESULT CTitleTipListViewCtrl::OnGetDispInfo(int idCtrl,LPNMHDR pnmh,BOOL &bHandled)
{
	// Make sure that the notification is about the control we care about.
	if (pnmh->hwndFrom != m_ToolTip)
	{
		bHandled = FALSE;
		return 0;
	}

	// Copy the item title to the hint string buffer.
	LPNMTTDISPINFO pNMTDI = (LPNMTTDISPINFO)pnmh;
	GetItemText(m_iCurrentItem,0,pNMTDI->lpszText,80);

	return 0;
}
