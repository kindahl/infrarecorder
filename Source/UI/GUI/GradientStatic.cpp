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
#include "../../Common/GraphUtil.h"
#include "GradientStatic.h"

CGradientStatic::CGradientStatic(COLORREF TopColor) : m_TopColor(TopColor)
{
}

CGradientStatic::~CGradientStatic()
{
}

LRESULT CGradientStatic::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	LRESULT lResult = DefWindowProc(uMsg,wParam,lParam);

	HDC hDC = GetWindowDC();

	RECT rcClient;
	GetClientRect(&rcClient);

	DrawVertGradientRect(hDC,&rcClient,m_TopColor,GetSysColor(COLOR_BTNFACE));

	ReleaseDC(hDC);

	return lResult;
}
