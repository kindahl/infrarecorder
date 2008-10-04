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
#include "resource.h"
#include "../../Common/GraphUtil.h"
#include "InfraRecorder.h"
#include "SplashWindow.h"

CSplashWindow::CSplashWindow()
{
	m_szInfoText[0] = '\0';
	m_uiInfoTextLen = 0;

	m_hBarBorderBrush = ::CreateSolidBrush(SPLASHWINDOW_BORDERCOLOR);
	n_hTextBkBrush = ::CreateSolidBrush(SPLASHWINDOW_TEXTBKCOLOR);

	m_iProgressBarPos = 0;
	m_iProgressBarSegSize = 0;

	// Load the function dynamically.
	HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
	m_pUpdateLayeredWindow = (tUpdateLayeredWindow)GetProcAddress(hUser32,"UpdateLayeredWindow");

	// Load a 32-bit transparent bitmap for Windows 2000 and newer systems.
	if (m_pUpdateLayeredWindow != NULL)
		LoadTransparentBitmap();
	else
		LoadBitmap();
}

CSplashWindow::~CSplashWindow()
{
	::DeleteObject(m_hBarBorderBrush);
	::DeleteObject(n_hTextBkBrush);
}

LRESULT CSplashWindow::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	SIZE sSplashBitmap;
	m_SplashBitmap.GetSize(sSplashBitmap);
	SetWindowPos(HWND_TOPMOST,0,0,sSplashBitmap.cx,sSplashBitmap.cy,SWP_NOMOVE);

	CenterWindow(HWND_DESKTOP);

	int iBottom = sSplashBitmap.cy - SPLASHWINDOW_BARINDENT_BOTTOM;
	m_rcProgressBar.left = SPLASHWINDOW_BARINDENT_LEFT;
	m_rcProgressBar.top = iBottom - SPLASHWINDOW_BAR_HEIGHT;
	m_rcProgressBar.right = sSplashBitmap.cx - SPLASHWINDOW_BARINDENT_RIGHT;
	m_rcProgressBar.bottom = iBottom;

	// For per-pixel alpha transparency the window needs to be layered.
	if (m_pUpdateLayeredWindow != NULL)
		ModifyStyleEx(0,WS_EX_LAYERED);

	return 0;
}

LRESULT CSplashWindow::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CPaintDC dc(m_hWnd);

	RECT rcClient;
	GetClientRect(&rcClient);

	HDC hMemDC = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_SplashBitmap);

	if (m_pUpdateLayeredWindow != NULL)
	{
		// Draw the text.
		DrawTransparentText(hMemDC);

		DrawTransparentBitmap(dc,hMemDC);
	}
	else
	{
		// Draw the progresbar.
		DrawProgressBar(hMemDC,&rcClient);

		// Draw the text.
		DrawText(hMemDC);

		BitBlt(dc,0,0,rcClient.right,rcClient.bottom,hMemDC,0,0,SRCCOPY);
	}

	SelectObject(hMemDC,hOldBitmap);

	ReleaseDC(dc);
	ReleaseDC(hMemDC);

	return 0;
}

void CSplashWindow::DrawTransparentBitmap(HDC hScreenDC,HDC hMemDC)
{
	SIZE sSplashBitmap;
	m_SplashBitmap.GetSize(sSplashBitmap);

	// Calculate window dimensions.
	RECT rcWindow;
	GetWindowRect(&rcWindow);

	POINT ptWindowPos;
	ptWindowPos.x = rcWindow.left;
	ptWindowPos.y = rcWindow.top;

	BLENDFUNCTION bfPixelFunction = { AC_SRC_OVER,0,255,AC_SRC_ALPHA };
	POINT ptSource = { 0,0 };

	m_pUpdateLayeredWindow(m_hWnd,hScreenDC,&ptWindowPos,&sSplashBitmap,hMemDC,&ptSource,0,&bfPixelFunction,ULW_ALPHA);
}

void CSplashWindow::DrawText(HDC hDC)
{
	HFONT hOldFont = (HFONT)SelectObject(hDC,AtlGetDefaultGuiFont());

	// Calculate the text height.
	SIZE sTextSize;
	GetTextExtentPoint32(hDC,m_szInfoText,m_uiInfoTextLen,&sTextSize);

	RECT rcText = m_rcProgressBar;
	rcText.top -= sTextSize.cy + SPLASHWINDOW_TEXT_SPACING;
	rcText.bottom = m_rcProgressBar.top;

	// Do the actual drawing.
	FillRect(hDC,&rcText,n_hTextBkBrush);

	::SetBkColor(hDC,SPLASHWINDOW_TEXTBKCOLOR);
	::SetTextColor(hDC,::GetSysColor(COLOR_WINDOWTEXT));

	::DrawText(hDC,m_szInfoText,m_uiInfoTextLen,&rcText,
		DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	SelectObject(hDC,hOldFont);
}

void CSplashWindow::DrawTransparentText(HDC hDC)
{
	HFONT hOldFont = (HFONT)SelectObject(hDC,AtlGetDefaultGuiFont());

	// Calculate the text height.
	SIZE sTextSize;
	GetTextExtentPoint32(hDC,m_szInfoText,m_uiInfoTextLen,&sTextSize);

	/*RECT rcText = m_rcProgressBar;
	rcText.top -= sTextSize.cy + SPLASHWINDOW_TEXT_SPACING;
	rcText.bottom = m_rcProgressBar.top;*/

	RECT rcText = { 33,125,280,125 + sTextSize.cy };

	// Do the actual drawing.
	FillRect(hDC,&rcText,n_hTextBkBrush);

	::SetBkColor(hDC,SPLASHWINDOW_TEXTBKCOLOR);
	::SetTextColor(hDC,::GetSysColor(COLOR_WINDOWTEXT));

	::DrawText(hDC,m_szInfoText,m_uiInfoTextLen,&rcText,
		DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	SelectObject(hDC,hOldFont);
	
	// Get bitmap information.
	BITMAP bmpInfo;
	m_SplashBitmap.GetBitmap(&bmpInfo);

	// Since the regular GDI functions (with a few exceptions) clear the alpha bit
	// when they are used we need to set it, since we don't want to draw
	// transparent text.
	unsigned char *pDataBits = (unsigned char *)bmpInfo.bmBits;

	int iStart = bmpInfo.bmHeight - rcText.bottom;
	int iEnd = bmpInfo.bmHeight - rcText.top;

	for (int y = iStart; y < iEnd; y++)
	{
		unsigned char *pPixel = pDataBits + bmpInfo.bmWidth * 4 * y;

		pPixel += 4 * rcText.left;

		for (int x = rcText.left; x < rcText.right; x++)
		{
			pPixel[3] = 0xFF;
			pPixel += 4;
		}
	}
}

void CSplashWindow::DrawProgressBar(HDC hDC,RECT *pClientRect)
{
	RECT rcBar = m_rcProgressBar;

	FillRect(hDC,&rcBar,m_hBarBorderBrush);

	ContractRect(&rcBar,1);

	// First we draw the background.
	rcBar.left += m_iProgressBarPos;
	DrawVertGradientRect(hDC,&rcBar,SPLASHWINDOW_BARCOLOR_TOP,SPLASHWINDOW_BARCOLOR_BOTTOM);

	// Next we draw progress.
	rcBar.left -= m_iProgressBarPos;
	rcBar.right = m_iProgressBarPos + SPLASHWINDOW_BARINDENT_LEFT + 1;
	DrawVertGradientRect(hDC,&rcBar,SPLASHWINDOW_BARCOLOR_PROGRESSTOP,SPLASHWINDOW_BARCOLOR_PROGRESSBOTTOM);
}

void CSplashWindow::SetInfoText(const TCHAR *szInfoText)
{
	lstrcpy(m_szInfoText,szInfoText);
	m_uiInfoTextLen = lstrlen(szInfoText);

	// The text should most probably not be larger than 42 pixels.
	RECT rcText = m_rcProgressBar;
	rcText.top -= 42 + SPLASHWINDOW_TEXT_SPACING;
	InvalidateRect(&rcText);

	// Process the message queue.
	ProcessMessages();
}

void CSplashWindow::SetMaxProgress(unsigned int uiMaxProgress)
{
	RECT rcClient;
	GetClientRect(&rcClient);

	int iProgressBarWidth = rcClient.right - SPLASHWINDOW_BARINDENT_LEFT - SPLASHWINDOW_BARINDENT_RIGHT - 2;
	m_iProgressBarSegSize = iProgressBarWidth/uiMaxProgress;
}

void CSplashWindow::SetProgress(unsigned int uiProgress)
{
	m_iProgressBarPos = uiProgress * m_iProgressBarSegSize;

	InvalidateRect(&m_rcProgressBar);

	// Process the message queue.
	ProcessMessages();
}

void CSplashWindow::LoadBitmap()
{
	// Load the bitmap.
	m_SplashBitmap.LoadBitmap(IDB_SPLASHBITMAP_);
}

void CSplashWindow::LoadTransparentBitmap()
{
	// Load the bitmap.
	HBITMAP hBitmap = (HBITMAP)LoadImage(_Module.GetModuleInstance(),MAKEINTRESOURCE(IDB_SPLASHBITMAP),
		IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
	m_SplashBitmap.Attach(hBitmap);

	// Precaclulate multiply the transparency.
	BITMAP bmpInfo;
	m_SplashBitmap.GetBitmap(&bmpInfo);

	unsigned char *pDataBits = (unsigned char *)bmpInfo.bmBits;

	for (int y = 0; y < bmpInfo.bmHeight; y++)
	{
		unsigned char *pPixel = pDataBits + bmpInfo.bmWidth * 4 * y;

		for (int x = 0; x < bmpInfo.bmWidth; x++)
		{
			pPixel[0] = pPixel[0] * pPixel[3] / 255;
			pPixel[1] = pPixel[1] * pPixel[3] / 255;
			pPixel[2] = pPixel[2] * pPixel[3] / 255;

			pPixel += 4;
		}
	}
}
