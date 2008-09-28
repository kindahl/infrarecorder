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

#pragma once
#include <vector>
#include "CustomPage.h"

#define CUSTOMPAGECTRL_HEADER_BUTTONSPACING			1
#define CUSTOMPAGECTRL_HEADER_SHADOWHEIGHT			10

class CCustomPageCtrl : public CWindowImpl<CCustomPageCtrl,CWindow>
{
private:
	unsigned int m_uiPageIndex;
	int m_iHoverPage;
	HBRUSH m_hBlackBrush;

	std::vector<CCustomPage *> m_Pages;

	void DrawHeader(HDC hDC,RECT *pClientRect);
	void DrawPage(HDC hDC,RECT *pClientRect);

public:
	CCustomPageCtrl();
	~CCustomPageCtrl();

	void AddPage(CCustomPage *pPage);

	DECLARE_WND_CLASS(_T("ckCustomPageCtrl"));

	BEGIN_MSG_MAP(CCustomPageCtrl)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
		MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN,OnLButtonDown)
    END_MSG_MAP()

	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
