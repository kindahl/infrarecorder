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
#include "visual_styles.hh"

CVisualStyles g_VisualStyles;

CVisualStyles::CVisualStyles()
{
	m_hInstance = LoadLibrary(_T("uxtheme.dll"));
}

CVisualStyles::~CVisualStyles()
{
	if (m_hInstance != NULL)
		FreeLibrary(m_hInstance);
                
	m_hInstance = NULL;
}

bool CVisualStyles::Supported()
{
	return (m_hInstance != NULL);
}

bool CVisualStyles::IsThemeActive()
{
	if (!Supported())
		return false;
                
	PFNISTHEMEACTIVE pfn = (PFNISTHEMEACTIVE)GetProcAddress(m_hInstance,"IsThemeActive");

	if (!pfn)
		return false;
                
	return (*pfn)();
}

bool CVisualStyles::IsAppThemed()
{
	if (!Supported())
		return false;
                
	PFNISAPPTHEMED pfnIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(m_hInstance,"IsAppThemed");

	if (!pfnIsAppThemed)
		return false;

	return (*pfnIsAppThemed)();
}

HTHEME CVisualStyles::OpenThemeData(HWND hwnd,LPCWSTR pszClassList)
{
	if (!Supported())
		return NULL;

	PFNOPENTHEMEDATA pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(m_hInstance,"OpenThemeData");

	if (!pfnOpenThemeData)
		return NULL;

	return (*pfnOpenThemeData)(hwnd,pszClassList);
}

HRESULT CVisualStyles::CloseThemeData(HTHEME hTheme)
{
	if (!Supported())
		return false;

	PFNCLOSETHEMEDATA pfnCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(m_hInstance,"CloseThemeData");

	if (!pfnCloseThemeData)
		return false;

	return (*pfnCloseThemeData)(hTheme);
}

HRESULT CVisualStyles::DrawThemeBackground(HTHEME hTheme,HDC hdc,
	int iPartId,int iStateId,const RECT *pRect,const RECT *pClipRect)
{
	if (!Supported())
		return false;
                
	PFNDRAWTHEMEBACKGROUND pfnDrawThemeBackground =
		(PFNDRAWTHEMEBACKGROUND)GetProcAddress(m_hInstance,"DrawThemeBackground");

	if (!pfnDrawThemeBackground)
		return false;

	return (*pfnDrawThemeBackground)(hTheme,hdc,iPartId,iStateId,pRect,pClipRect);
}

HRESULT CVisualStyles::GetThemeColor(HTHEME hTheme,int iPartId,int iStateId,
	int iPropId,COLORREF *pColor)
{
	if (!Supported())
		return false;

	PFNGETTHEMECOLOR pfnGetThemeColor =
		(PFNGETTHEMECOLOR)GetProcAddress(m_hInstance,"GetThemeColor");

	if (!pfnGetThemeColor)
		return false;

	return (*pfnGetThemeColor)(hTheme,iPartId,iStateId,iPropId,pColor);
}
