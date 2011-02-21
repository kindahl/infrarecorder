/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#include <base/string_util.hh>

#define DOUBLEBUFFEREDSTATIC_RESERVE_LENGTH				256

class CDoubleBufferedStatic : public CWindowImpl<CDoubleBufferedStatic,CStatic>
{
private:
	tstring m_Text;

public:
	CDoubleBufferedStatic();
	~CDoubleBufferedStatic();

	void SetWindowText(const TCHAR *szWindowText);

	BEGIN_MSG_MAP(CDoubleBufferedStatic)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkGnd)
	END_MSG_MAP()

	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnEraseBkGnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
