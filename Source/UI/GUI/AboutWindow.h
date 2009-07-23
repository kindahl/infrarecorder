/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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

#define ABOUTWINDOW_TEXTCOLOR					RGB(50,50,50)
#define ABOUTWINDOW_URLCOLOR					RGB(51,100,163)

#define ABOUTWINDOW_URL_LEFT					37
#define ABOUTWINDOW_URL_TOP						343
#define ABOUTWINDOW_URL_RIGHT					150
#define ABOUTWINDOW_URL_BOTTOM					373

#if (_WIN32_WINNT < 0x0500)
#define WS_EX_LAYERED							0x00080000
#define ULW_ALPHA								0x00000002
#endif

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

typedef BOOL (WINAPI *tUpdateLayeredWindow)(HWND hWnd,HDC hdcDst,POINT *pptDst,
											SIZE *psize,HDC hdcSrc,POINT *pptSrc,
											COLORREF crKey,BLENDFUNCTION *pblend,
											DWORD dwFlags);

class CAboutWindow : public CWindowImpl<CAboutWindow,CWindow,CWinTraits<WS_POPUP | WS_VISIBLE,WS_EX_TOOLWINDOW> >
{
private:
	CBitmap m_SplashTmpBitmap;
	CBitmap m_SplashRefBitmap;

	HFONT m_VerFont;
	HFONT m_UrlFont;
	bool m_bUrlHover;

	// Updated by the UpdateVersionInfo function.
	TCHAR m_szVersion[128];
	ckcore::tstring m_szCdrtoolsVersion;

	HWND m_hWndParent;

	void UpdateVersionInfo();
	void RollbackBitmap();
	void Render();

	tUpdateLayeredWindow m_pUpdateLayeredWindow;
	void DrawBitmap(HDC hScreenDC,HDC hMemDC);
	void DrawText(HDC hDC,HFONT hFont,RECT *pRect,COLORREF Color,
				  const TCHAR *szText,int iTextLen);

	void LoadBitmap(CBitmap &Bitmap);
	void LoadBitmaps();

public:
	CAboutWindow();
	~CAboutWindow();

	BEGIN_MSG_MAP(CAboutWindow)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove);
		MESSAGE_HANDLER(WM_LBUTTONDOWN,OnLButtonDown)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	void CreateAndShow(HWND hWndParent);
};

extern CAboutWindow *g_pAboutWnd;