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
#include "resource.h"
#include "CustomButton.h"

CCustomButton::CCustomButton(ATL::_U_STRINGorID NormalBitmap,
							 ATL::_U_STRINGorID HoverBitmap,
							 ATL::_U_STRINGorID FocusBitmap) :
	m_State(STATE_NORMAL)
{
	// Load the bitmaps.
	m_NormalBitmap.LoadBitmap(NormalBitmap);
	m_HoverBitmap.LoadBitmap(HoverBitmap);
	m_FocusBitmap.LoadBitmap(FocusBitmap);
}

CCustomButton::~CCustomButton()
{
}

LRESULT CCustomButton::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	TRACKMOUSEEVENT	tme = { 0 };

	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.hwndTrack = m_hWnd;
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime	= 10000;
	bool bMouseHover = TrackMouseEvent(&tme) == TRUE;
	if (bMouseHover && m_State == STATE_NORMAL)
	{
		m_State = STATE_HOT;

		RECT rcClient;
		GetClientRect(&rcClient);
		InvalidateRect(&rcClient);
	}

	return 0;
}

LRESULT CCustomButton::OnMouseLeave(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_State = STATE_NORMAL;

	RECT rcClient;
	GetClientRect(&rcClient);
	InvalidateRect(&rcClient);

	return 0;
}

LRESULT CCustomButton::OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_State == STATE_HOT)
	{
		m_State = STATE_DOWN;

		RECT rcClient;
		GetClientRect(&rcClient);
		InvalidateRect(&rcClient);
	}

	return 0;
}

LRESULT CCustomButton::OnLButtonUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_State == STATE_DOWN)
	{
		m_State = STATE_HOT;

		RECT rcClient;
		GetClientRect(&rcClient);
		InvalidateRect(&rcClient);
	}

	return 0;
}

LRESULT CCustomButton::OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	return 0;
}

void CCustomButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDCHandle dc = lpDrawItemStruct->hDC;

	RECT rcClient;
	GetClientRect(&rcClient);

	HDC hMemDC = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap = NULL;
	
	switch (m_State)
	{
		case STATE_NORMAL:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_FocusBitmap);
			else
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_NormalBitmap);
			break;

		case STATE_HOT:
		case STATE_DOWN:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_FocusBitmap);
			else
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_HoverBitmap);
			break;
	}

	BitBlt(dc,0,0,rcClient.right,rcClient.bottom,hMemDC,0,0,SRCCOPY);

	SelectObject(hMemDC,hOldBitmap);

	ReleaseDC(dc);
	ReleaseDC(hMemDC);
}
