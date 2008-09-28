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
#include "CustomPageCtrl.h"
#include "../../Common/GraphUtil.h"

CCustomPageCtrl::CCustomPageCtrl()
{
	m_uiPageIndex = 0;
	m_iHoverPage = -1;

	m_hBlackBrush = ::CreateSolidBrush(RGB(0,0,0));
}

CCustomPageCtrl::~CCustomPageCtrl()
{
	::DeleteObject(m_hBlackBrush);
}

void CCustomPageCtrl::AddPage(CCustomPage *pPage)
{
	m_Pages.push_back(pPage);
}

void CCustomPageCtrl::DrawHeader(HDC hDC,RECT *pClientRect)
{
	FillRect(hDC,pClientRect,m_hBlackBrush);
	// Draw the buttons.
	RECT rcButton = {
		CUSTOMPAGECTRL_HEADER_BUTTONSPACING,
		CUSTOMPAGECTRL_HEADER_BUTTONSPACING,
		CUSTOMPAGECTRL_HEADER_BUTTONSPACING + CUSTOMPAGE_BUTTON_WIDTH,
		CUSTOMPAGECTRL_HEADER_BUTTONSPACING + CUSTOMPAGE_BUTTON_HEIGHT };

	for (unsigned int i = 0; i < m_Pages.size(); i++)
	{
		m_Pages[i]->DrawButton(hDC,&rcButton,m_iHoverPage == i,m_uiPageIndex == i);

		rcButton.left += CUSTOMPAGE_BUTTON_WIDTH + CUSTOMPAGECTRL_HEADER_BUTTONSPACING;
		rcButton.right += CUSTOMPAGE_BUTTON_WIDTH + CUSTOMPAGECTRL_HEADER_BUTTONSPACING;
	}

	// Draw the rest of the header background.
	rcButton.right = pClientRect->right;
	DrawVertGradientRect(hDC,&rcButton,CUSTOMPAGE_BUTTON_TOPCOLOR,CUSTOMPAGE_BUTTON_BOTTOMCOLOR);

	// Draw the shaddow.
	RECT rcShadow = *pClientRect;
	rcShadow.top = rcButton.bottom + CUSTOMPAGECTRL_HEADER_BUTTONSPACING;
	rcShadow.bottom = rcShadow.top + CUSTOMPAGECTRL_HEADER_SHADOWHEIGHT;

	DrawVertGradientRect(hDC,&rcShadow,RGB(67,67,67),RGB(255,255,255));
}

void CCustomPageCtrl::DrawPage(HDC hDC,RECT *pClientRect)
{
	RECT rcPage = *pClientRect;
	rcPage.top = CUSTOMPAGECTRL_HEADER_BUTTONSPACING * 2 + CUSTOMPAGE_BUTTON_HEIGHT +
		CUSTOMPAGECTRL_HEADER_SHADOWHEIGHT;

	m_Pages[m_uiPageIndex]->DrawContents(hDC,&rcPage);
}

LRESULT CCustomPageCtrl::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CPaintDC dc(m_hWnd);

	RECT rcClient;
	GetClientRect(&rcClient);

	HDC hMemDC;
	HBITMAP hMemBitmap;

	hMemDC = CreateCompatibleDC(dc);
	hMemBitmap = CreateCompatibleBitmap(dc,rcClient.right,rcClient.bottom);

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC,hMemBitmap);

	// Draw the header.
	DrawHeader(hMemDC,&rcClient);

	// Draw the active page.
	DrawPage(hMemDC,&rcClient);

	BitBlt(dc,0,0,rcClient.right,rcClient.bottom,hMemDC,0,0,SRCCOPY);

	SelectObject(hMemDC,hOldBitmap);

	ReleaseDC(dc);
	DeleteDC(hMemDC);

	DeleteObject(hMemBitmap);

	return 0;
}

LRESULT CCustomPageCtrl::OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	return true;
}

LRESULT CCustomPageCtrl::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	int iX = LOWORD(lParam);
	int iY = HIWORD(lParam);

	int iHoverPage = -1;

	// Check if the cursor is on the header.
	if (iY <= CUSTOMPAGE_BUTTON_HEIGHT)
	{
		int iPage = iX / (CUSTOMPAGECTRL_HEADER_BUTTONSPACING + CUSTOMPAGE_BUTTON_WIDTH);

		if (iPage < (int)m_Pages.size())
			iHoverPage = iPage;
		else
			iHoverPage = -1;
	}
	else
	{
		iHoverPage = -1;

		RECT rcClient;
		GetClientRect(&rcClient);
		rcClient.top = CUSTOMPAGECTRL_HEADER_BUTTONSPACING * 2 + CUSTOMPAGE_BUTTON_HEIGHT +
			CUSTOMPAGECTRL_HEADER_SHADOWHEIGHT;

		iY -= rcClient.top;

		m_Pages[m_uiPageIndex]->OnMouseMove(iX,iY,m_hWnd,&rcClient);
	}

	if (m_iHoverPage != iHoverPage)
	{
		m_iHoverPage = iHoverPage;

		// Redraw.
		RECT rcClient;
		GetClientRect(&rcClient);
		rcClient.bottom = CUSTOMPAGE_BUTTON_HEIGHT;

		InvalidateRect(&rcClient);
	}

	return 0;
}

LRESULT CCustomPageCtrl::OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_iHoverPage != -1)
	{
		m_uiPageIndex = m_iHoverPage;

		// Redraw.
		RECT rcClient;
		GetClientRect(&rcClient);
		//rcClient.top = CUSTOMPAGE_BUTTON_HEIGHT + 1;

		InvalidateRect(&rcClient);
	}
	else
	{
		m_Pages[m_uiPageIndex]->OnLButtonDown();
	}

	return 0;
}
