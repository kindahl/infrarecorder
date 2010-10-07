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

#pragma once
#include "png_file.hh"

class CCustomButton : public CWindowImpl<CCustomButton,CButton>,
	public COwnerDraw<CCustomButton>
{
private:
	enum eState
	{
		STATE_NORMAL,
		STATE_HOT,
		STATE_DOWN
	};

	eState m_State;
	int m_iCoverLeft;
	int m_iCoverTop;

	CPngFile m_CoverImage;
	CPngFile m_NormalImage;
	CPngFile m_FocusImage;
	CPngFile m_HoverImage;
	CPngFile m_HoverFocusImage;

public:
	DECLARE_WND_CLASS(_T("ckButton"));

	CCustomButton(unsigned short usCoverPng,int iCoverLeft,int iCoverRight);
	~CCustomButton();

	BEGIN_MSG_MAP(CCustomButton)
		MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE,OnMouseLeave)
		//MESSAGE_HANDLER(WM_LBUTTONDOWN,OnLButtonDown)
		//MESSAGE_HANDLER(WM_LBUTTONUP,OnLButtonUp)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)

		CHAIN_MSG_MAP_ALT(COwnerDraw<CCustomButton>,1)
	END_MSG_MAP()

	LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseLeave(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnLButtonUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	// For ownerdraw.
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};
