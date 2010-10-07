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
#include "double_buffered_static.hh"

CDoubleBufferedStatic::CDoubleBufferedStatic()
{
	m_Text.reserve(DOUBLEBUFFEREDSTATIC_RESERVE_LENGTH);
}

CDoubleBufferedStatic::~CDoubleBufferedStatic()
{
}

void CDoubleBufferedStatic::SetWindowText(const TCHAR *szWindowText)
{
	m_Text = szWindowText;

	RedrawWindow();
}

LRESULT CDoubleBufferedStatic::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CPaintDC dc(m_hWnd);

	RECT rcClient;
	GetClientRect(&rcClient);

	// Setup double buffering.
	HDC hMemDC = CreateCompatibleDC(dc);
	HBITMAP hMemBitmap = CreateCompatibleBitmap(dc,rcClient.right,rcClient.bottom);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC,hMemBitmap);
 
	// Draw the background.
	FillRect(hMemDC,&rcClient,GetSysColorBrush(COLOR_BTNFACE));

	// Draw the text.
	HFONT hOldFont = (HFONT)SelectObject(hMemDC,AtlGetDefaultGuiFont());

	::SetTextColor(hMemDC,GetSysColor(COLOR_BTNTEXT));
	::SetBkColor(hMemDC,GetSysColor(COLOR_BTNFACE));

	DrawText(hMemDC,m_Text.c_str(),(int)m_Text.length(),&rcClient,
		DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	SelectObject(hMemDC,hOldFont);

	BitBlt(dc,0,0,rcClient.right,rcClient.bottom,hMemDC,0,0,SRCCOPY);

	SelectObject(hMemDC,hOldBitmap);
	DeleteDC(hMemDC);
	DeleteObject(hMemBitmap);

	bHandled = true;
	return 0;
}

LRESULT CDoubleBufferedStatic::OnEraseBkGnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	bHandled = true;
	return 0;
}
