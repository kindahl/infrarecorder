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

#pragma once

#define MAINVIEW_NORMALBORDER_SIZE				2
#define MAINVIEW_THEMEDBORDER_SIZE				4

class CMainView : public CSplitterWindowImpl<CMainView,/*false*/true>
{
private:
	unsigned int m_uiBorderSize;

public:
	CMainView();
	~CMainView();

	typedef CSplitterWindowImpl<CMainView,/*false*/true> _baseClass;

	BEGIN_MSG_MAP(CMainView)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_NCCALCSIZE,OnNCCalcSize)
		MESSAGE_HANDLER(WM_NCPAINT,OnNCPaint)

		CHAIN_MSG_MAP(_baseClass);
    END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCCalcSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
