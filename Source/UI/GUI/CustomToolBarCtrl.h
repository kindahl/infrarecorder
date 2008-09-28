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

class CCustomToolBarCtrl : public CWindowImpl<CCustomToolBarCtrl,CToolBarCtrl>
{
public:
	DECLARE_WND_CLASS(_T("ckToolBar"));

	CCustomToolBarCtrl();
	~CCustomToolBarCtrl();

	BEGIN_MSG_MAP(CCustomToolBarCtrl)
		MESSAGE_HANDLER(WM_RBUTTONDOWN,OnRButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK,OnLButtonDblClk)
	END_MSG_MAP()

	LRESULT OnRButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnLButtonDblClk(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
