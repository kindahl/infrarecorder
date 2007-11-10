/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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

#define SPLASHWINDOW_TEXT_SPACING				2
#define SPLASHWINDOW_TEXT_MAXLENGTH				64

#define SPLASHWINDOW_BAR_HEIGHT					10
#define SPLASHWINDOW_BARINDENT_LEFT				4
#define SPLASHWINDOW_BARINDENT_RIGHT			4
#define SPLASHWINDOW_BARINDENT_BOTTOM			4

#define SPLASHWINDOW_BORDERCOLOR				RGB(94,97,158)
#define SPLASHWINDOW_BARCOLOR_TOP				RGB(212,212,243)
#define SPLASHWINDOW_BARCOLOR_BOTTOM			RGB(255,255,255)
#define SPLASHWINDOW_BARCOLOR_PROGRESSTOP		RGB(194,194,232)
#define SPLASHWINDOW_BARCOLOR_PROGRESSBOTTOM	RGB(145,145,209)

#define SPLASHWINDOW_TEXTBKCOLOR				RGB(255,255,255)

#if (_WIN32_WINNT < 0x0500)
#define WS_EX_LAYERED							0x00080000
#define ULW_ALPHA								0x00000002
#endif

typedef BOOL (WINAPI *tUpdateLayeredWindow)(HWND hWnd,HDC hdcDst,POINT *pptDst,SIZE *psize,HDC hdcSrc,POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend,DWORD dwFlags);

class CSplashWindow : public CWindowImpl<CSplashWindow,CWindow,CWinTraits<WS_POPUP | WS_VISIBLE,WS_EX_TOOLWINDOW> >
{
private:
	CBitmap m_SplashBitmap;
	RECT m_rcProgressBar;

	HBRUSH m_hBarBorderBrush;
	HBRUSH n_hTextBkBrush;

	int m_iProgressBarPos;
	int m_iProgressBarSegSize;

	TCHAR m_szInfoText[SPLASHWINDOW_TEXT_MAXLENGTH];
	unsigned int m_uiInfoTextLen;

	tUpdateLayeredWindow m_pUpdateLayeredWindow;
	void DrawTransparentBitmap(HDC hScreenDC,HDC hMemDC);

	void DrawText(HDC hDC);
	void DrawTransparentText(HDC hDC);

	void DrawProgressBar(HDC hDC,RECT *pClientRect);

	void LoadBitmap();
	void LoadTransparentBitmap();

public:
	CSplashWindow();
	~CSplashWindow();

	BEGIN_MSG_MAP(CSplashWindow)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	void SetInfoText(const TCHAR *szInfoText);
	void SetMaxProgress(unsigned int uiMaxProgress);
	void SetProgress(unsigned int uiProgress);
};
