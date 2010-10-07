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
#include "Resource.h"
#include "custom_multi_button.hh"

CCustomMultiButton::CCustomMultiButton(long lCtrlMainId,long lCtrlSub1Id,long lCtrlSub2Id,
									   unsigned short usCoverPng,int iCoverLeft,int iCoverRight) :
	m_lCtrlMainId(lCtrlMainId),m_lCtrlSub1Id(lCtrlSub1Id),m_lCtrlSub2Id(lCtrlSub2Id),
	m_State(STATE_NORMAL),m_iCoverLeft(iCoverLeft),m_iCoverTop(iCoverRight)
{
	// Load the images.
	m_CoverImage.Open(usCoverPng);
	m_NormalImage.Open(IDR_MBUTTONNPNG);
	m_FocusImage.Open(IDR_MBUTTONFPNG);
	m_HoverImage.Open(IDR_MBUTTONHPNG);
	m_HoverSub1Image.Open(IDR_MBUTTONHS1PNG);
	m_HoverSub2Image.Open(IDR_MBUTTONHS2PNG);
	m_HoverFocusImage.Open(IDR_MBUTTONFPNG);
	m_HoverFocusSub1Image.Open(IDR_MBUTTONHFS1PNG);
	m_HoverFocusSub2Image.Open(IDR_MBUTTONHFS2PNG);
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

	eState NewState;
	if (iPosX < SPLITTER_X)
		NewState = STATE_HOTMAIN;
	else if (iPosY < SPLITTER_Y)
		NewState = STATE_HOTSUB1;
	else
		NewState = STATE_HOTSUB2;

	if (NewState != m_State)
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		InvalidateRect(&rcClient);
	}

	m_State = NewState;
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
	FillRect(dc,&rcClient,GetSysColorBrush(COLOR_WINDOW));

	switch (m_State)
	{
		case STATE_NORMAL:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				m_FocusImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			else
				m_NormalImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			break;

		case STATE_HOTMAIN:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				m_HoverFocusImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			else
				m_HoverImage.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			break;

		case STATE_HOTSUB1:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				m_HoverFocusSub1Image.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			else
				m_HoverSub1Image.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			break;

		case STATE_HOTSUB2:
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
				m_HoverFocusSub2Image.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			else
				m_HoverSub2Image.Draw(dc,0,0,rcClient.right,rcClient.bottom);
			break;
	}

	m_CoverImage.Draw(dc,m_iCoverLeft,m_iCoverTop,rcClient.right,rcClient.bottom);

	ReleaseDC(dc);
}
