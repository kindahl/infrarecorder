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

#include "stdafx.hh"
#include "Resource.h"
#include "custom_button.hh"

CCustomButton::CCustomButton(unsigned short usCoverPng,int iCoverLeft,int iCoverRight) :
	m_State(STATE_NORMAL),m_iCoverLeft(iCoverLeft),m_iCoverTop(iCoverRight)
{
	// Load the images.
	m_CoverImage.Open(usCoverPng);
	m_NormalImage.Open(IDR_BUTTONNPNG);
	m_FocusImage.Open(IDR_BUTTONFPNG);
	m_HoverImage.Open(IDR_BUTTONHPNG);
	m_HoverFocusImage.Open(IDR_BUTTONHFPNG);
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

	FillRect(dc,&rcClient,GetSysColorBrush(COLOR_WINDOW));

	switch (m_State)
	{
		case STATE_NORMAL:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				m_FocusImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			else
				m_NormalImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			break;

		case STATE_HOT:
		case STATE_DOWN:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				m_HoverFocusImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			else
				m_HoverImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			break;
	}

	m_CoverImage.Draw(dc,m_iCoverLeft,m_iCoverTop,rcClient.right,rcClient.bottom);

	ReleaseDC(dc);
}
