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

#include "stdafx.h"
#include "../../Common/GraphUtil.h"
#include "Resource.h"
#include "InfraRecorder.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "SplashWindow.h"

CSplashWindow::CSplashWindow()
{
	m_hTextBkBrush = ::CreateSolidBrush(SPLASHWINDOW_TEXTBKCOLOR);

	// Load the function dynamically.
	HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
	m_pUpdateLayeredWindow = (tUpdateLayeredWindow)GetProcAddress(hUser32,"UpdateLayeredWindow");

	// Load a 32-bit transparent bitmap for Windows 2000 and newer systems.
	if (m_pUpdateLayeredWindow != NULL)
		LoadBitmap();
}

CSplashWindow::~CSplashWindow()
{
	::DeleteObject(m_hTextBkBrush);
}

LRESULT CSplashWindow::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	SIZE sSplashBitmap;
	m_SplashBitmap.GetSize(sSplashBitmap);
	SetWindowPos(HWND_TOPMOST,0,0,sSplashBitmap.cx,sSplashBitmap.cy,SWP_NOMOVE);

	CenterWindow(HWND_DESKTOP);

	// For per-pixel alpha transparency the window needs to be layered.
	if (m_pUpdateLayeredWindow != NULL)
		ModifyStyleEx(0,WS_EX_LAYERED);

	return 0;
}

LRESULT CSplashWindow::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CPaintDC dc(m_hWnd);

	if (m_pUpdateLayeredWindow == NULL)
		return 0;

	RECT rcClient;
	GetClientRect(&rcClient);

	HDC hMemDC = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_SplashBitmap);

	DrawText(hMemDC);
	DrawBitmap(dc,hMemDC);

	SelectObject(hMemDC,hOldBitmap);

	ReleaseDC(dc);
	ReleaseDC(hMemDC);

	return 0;
}

void CSplashWindow::DrawBitmap(HDC hScreenDC,HDC hMemDC)
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
	GetTextExtentPoint32(hDC,m_InfoText.c_str(),m_InfoText.size(),&sTextSize);

	RECT rcText = { 33,125,280,125 + sTextSize.cy };

	// Do the actual drawing.
	FillRect(hDC,&rcText,m_hTextBkBrush);

	::SetBkColor(hDC,SPLASHWINDOW_TEXTBKCOLOR);
	::SetTextColor(hDC,::GetSysColor(COLOR_WINDOWTEXT));

	::DrawText(hDC,m_InfoText.c_str(),m_InfoText.size(),&rcText,
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

void CSplashWindow::SetInfoText(const TCHAR *szInfoText)
{
	m_InfoText = szInfoText;

	// Force redraw.
	InvalidateRect(NULL);

	// Process the message queue.
	ProcessMessages();
}

void CSplashWindow::LoadBitmap()
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

void CSplashWindow::event_status(ckmmc::DeviceManager::ScanCallback::Status Status)
{
	if (Status == ckmmc::DeviceManager::ScanCallback::ckEVENT_DEV_SCAN)
		SetInfoText(lngGetString(INIT_SCANBUS));
	else if (Status == ckmmc::DeviceManager::ScanCallback::ckEVENT_DEV_CAP)
		SetInfoText(lngGetString(INIT_LOADCAPABILITIES));
}

bool CSplashWindow::event_device(ckmmc::Device::Address &Addr)
{
	return true;
}

void CSplashWindow::SafeCreate()
{
	Create(HWND_DESKTOP,CWindow::rcDefault);
}

void CSplashWindow::SafeDestroy()
{
	DestroyWindow();
}