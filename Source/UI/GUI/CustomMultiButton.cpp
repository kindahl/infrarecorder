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
#include "CustomMultiButton.h"

CCustomMultiButton::CCustomMultiButton(long lCtrlMainId,long lCtrlSub1Id,long lCtrlSub2Id,
									   ATL::_U_STRINGorID NormalBitmap,
									   ATL::_U_STRINGorID HoverMainBitmap,
									   ATL::_U_STRINGorID HoverSub1Bitmap,
									   ATL::_U_STRINGorID HoverSub2Bitmap,
								       ATL::_U_STRINGorID FocusBitmap) :
	m_lCtrlMainId(lCtrlMainId),m_lCtrlSub1Id(lCtrlSub1Id),m_lCtrlSub2Id(lCtrlSub2Id),
	m_State(STATE_NORMAL)
{
	// Load the bitmaps.
	m_NormalBitmap.LoadBitmap(NormalBitmap);
	m_HoverMainBitmap.LoadBitmap(HoverMainBitmap);
	m_HoverSub1Bitmap.LoadBitmap(HoverSub1Bitmap);
	m_HoverSub2Bitmap.LoadBitmap(HoverSub2Bitmap);
	m_FocusBitmap.LoadBitmap(FocusBitmap);
}

CCustomMultiButton::~CCustomMultiButton()
{
}

LRESULT CCustomMultiButton::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	TRACKMOUSEEVENT	tme = { 0 };

	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.hwndTrack = m_hWnd;
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime	= 10000;
	TrackMouseEvent(&tme);

	int iPosX = GET_X_LPARAM(lParam); 
	int iPosY = GET_Y_LPARAM(lParam);

	if (iPosX < SPLITTER_X)
		m_State = STATE_HOTMAIN;
	else if (iPosY < SPLITTER_Y)
		m_State = STATE_HOTSUB1;
	else
		m_State = STATE_HOTSUB2;

	RECT rcClient;
	GetClientRect(&rcClient);
	InvalidateRect(&rcClient);

	return 0;
}

LRESULT CCustomMultiButton::OnMouseLeave(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_State = STATE_NORMAL;

	RECT rcClient;
	GetClientRect(&rcClient);
	InvalidateRect(&rcClient);

	return 0;
}

LRESULT CCustomMultiButton::OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_State == STATE_HOTMAIN ||
		m_State == STATE_HOTSUB1 ||
		m_State == STATE_HOTSUB2)
	{
		m_State = STATE_DOWN;

		RECT rcClient;
		GetClientRect(&rcClient);
		InvalidateRect(&rcClient);
	}

	return 0;
}

LRESULT CCustomMultiButton::OnLButtonUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_State == STATE_DOWN)
	{
		m_State = STATE_HOTMAIN;

		RECT rcClient;
		GetClientRect(&rcClient);
		InvalidateRect(&rcClient);
	}

	// Take action.
	int iPosX = GET_X_LPARAM(lParam); 
	int iPosY = GET_Y_LPARAM(lParam);

	if (iPosX < SPLITTER_X)
		::PostMessage(GetParent(),WM_COMMAND,m_lCtrlMainId,0);
	else if (iPosY < SPLITTER_Y)
		::PostMessage(GetParent(),WM_COMMAND,m_lCtrlSub1Id,0);
	else
		::PostMessage(GetParent(),WM_COMMAND,m_lCtrlSub2Id,0);

	return 0;
}

LRESULT CCustomMultiButton::OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	return 0;
}

void CCustomMultiButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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

		case STATE_HOTMAIN:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_FocusBitmap);
			else
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_HoverMainBitmap);
			break;

		case STATE_HOTSUB1:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_FocusBitmap);
			else
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_HoverSub1Bitmap);
			break;

		case STATE_HOTSUB2:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_FocusBitmap);
			else
				hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_HoverSub2Bitmap);
			break;
	}

	BitBlt(dc,0,0,rcClient.right,rcClient.bottom,hMemDC,0,0,SRCCOPY);

	SelectObject(hMemDC,hOldBitmap);

	ReleaseDC(dc);
	ReleaseDC(hMemDC);
}
