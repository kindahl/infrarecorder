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
#include <uxtheme.h>
#include <tmschema.h>

typedef bool (__stdcall *PFNISTHEMEACTIVE)();
typedef bool(__stdcall *PFNISAPPTHEMED)();

typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd,LPCWSTR pszClassList);
typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme,HDC hdc, 
	int iPartId,int iStateId,const RECT *pRect,const RECT *pClipRect);
typedef HRESULT(__stdcall *PFNGETTHEMECOLOR)(HTHEME hTheme,int iPartId, 
    int iStateId,int iPropId,COLORREF *pColor);

class CVisualStyles
{
private:
    HINSTANCE m_hInstance;
        
    bool Supported();

public:
	CVisualStyles();
	~CVisualStyles();

	bool IsThemeActive();
	bool IsAppThemed();

	HTHEME OpenThemeData(HWND hwnd,LPCWSTR pszClassList);
	HRESULT CloseThemeData(HTHEME hTheme);
	HRESULT DrawThemeBackground(HTHEME hTheme,HDC hdc,int iPartId,int iStateId,
		const RECT *pRect,const RECT *pClipRect);
	HRESULT GetThemeColor(HTHEME hTheme,int iPartId,int iStateId,int iPropId,
		COLORREF *pColor);
};

extern CVisualStyles g_VisualStyles;
