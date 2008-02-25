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

#include "stdafx.h"
#include "SpaceMeter.h"
#include "../../Common/StringUtil.h"
#include "../../Common/GraphUtil.h"
#include "LangUtil.h"
#include "StringTable.h"
#include "WinVer.h"

CSpaceMeter::CSpaceMeter()
{
	m_iMeterPosition = 0;
	m_iMeterSegmentSpacing = 0;

	m_hProgressTheme = NULL;
	m_iHorIndent = SPACEMETER_BARINDENT_THEMED;

	// By default we use the size display mode.
	m_iDisplayMode = SPACEMETER_DMSIZE;

	// The default draw state is ofcourse normal.
	m_iDrawState = SPACEMETER_DRAWSTATE_NORMAL;

	m_uiAllocatedSize = 1;
	m_uiDiscSize = 1;
	m_uiMeterSize = 1;

	for (unsigned int i = 0; i < SPACEMETER_METER_COUNT; i++)
		lstrcpy(m_uiMeterSegments[i],_T("0 B"));

	m_hBarBorderBrush = ::CreateSolidBrush(SPACEMETER_BORDERCOLOR);
	m_hWarnBarBorderBrush = ::CreateSolidBrush(SPACEMETER_WARNBORDERCOLOR);
	m_hFullBarBorderBrush = ::CreateSolidBrush(SPACEMETER_FULLBORDERCOLOR);

	m_szToolTip[0] = '\0';

	// Create and fill the popup menu.
	m_hPopupMenu = CreatePopupMenu();
	FillPopupMenu();
}

CSpaceMeter::~CSpaceMeter()
{
	::DeleteObject(m_hBarBorderBrush);
	::DeleteObject(m_hWarnBarBorderBrush);
	::DeleteObject(m_hFullBarBorderBrush);

	// Destroy the popup menu.
	if (m_hPopupMenu != NULL)
		DestroyMenu(m_hPopupMenu);

	// Destroy the tooltip control.
	if (m_ToolTip.IsWindow())
		m_ToolTip.DestroyWindow();

	// Close the progress theme data.
	if (m_hProgressTheme != NULL)
		g_VisualStyles.CloseThemeData(m_hProgressTheme);
}

void CSpaceMeter::FillPopupMenu()
{
	// Remove the old items.
	int iMenuItemCount = GetMenuItemCount(m_hPopupMenu);
	int iMenuIndex = 0;
	for (int i = iMenuItemCount - 1; i >= 0; i--)
		RemoveMenu(m_hPopupMenu,i,MF_BYPOSITION);

	// Add new items (and on the same time convert them to radio items).
	unsigned int uiMenuIndex = 0;
	unsigned int uiCommand = SPACEMETER_POPUPMENU_IDBASE;

	CMenuItemInfo mii;
	mii.fMask = 0x100;
	mii.fType = MFT_RADIOCHECK;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("DVD 7.96 GiB"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("DVD 4.38 GiB"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("99 min (870 MiB)"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("90 min (791 MiB)"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("80 min (703 MiB)"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("74 min (650 MiB)"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("24 min (210 MiB)"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;

	InsertMenu(m_hPopupMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiCommand++,_T("21 min (185 MiB)"));
	SetMenuItemInfo(m_hPopupMenu,uiMenuIndex,TRUE,&mii);
	uiMenuIndex++;
}

void CSpaceMeter::DrawBar(HDC hDC,RECT *pClientRect)
{
	int iBottom = pClientRect->bottom - SPACEMETER_BARINDENT_BOTTOM;

	RECT rcBar = {
		pClientRect->left + m_iHorIndent,
		iBottom - SPACEMETER_BAR_HEIGHT,
		pClientRect->right - m_iHorIndent,
		iBottom
	};

	// Draw the meter.
	DrawMeter(hDC,pClientRect,&rcBar);

	// Draw a different meter on Windows Vista systems.
	if (m_hProgressTheme == NULL)
	{
		switch (m_iDrawState)
		{
			case SPACEMETER_DRAWSTATE_NORMAL:
				FillRect(hDC,&rcBar,m_hBarBorderBrush);
				break;

			case SPACEMETER_DRAWSTATE_WARN:
				FillRect(hDC,&rcBar,m_hWarnBarBorderBrush);
				break;

			case SPACEMETER_DRAWSTATE_FULL:
				FillRect(hDC,&rcBar,m_hFullBarBorderBrush);
				break;
		}		

		ContractRect(&rcBar,1);

		// First we draw the free space.
		rcBar.left += m_iMeterPosition;

		switch (m_iDrawState)
		{
			case SPACEMETER_DRAWSTATE_NORMAL:
				DrawVertGradientRect(hDC,&rcBar,SPACEMETER_BARCOLOR_FREETOP,SPACEMETER_BARCOLOR_FREEBOTTOM);
				break;

			case SPACEMETER_DRAWSTATE_WARN:
				DrawVertGradientRect(hDC,&rcBar,SPACEMETER_WARNBARCOLOR_FREETOP,SPACEMETER_WARNBARCOLOR_FREEBOTTOM);
				break;

			case SPACEMETER_DRAWSTATE_FULL:
				DrawVertGradientRect(hDC,&rcBar,SPACEMETER_FULLBARCOLOR_FREETOP,SPACEMETER_FULLBARCOLOR_FREEBOTTOM);
				break;
		}

		// Next we draw the allocated space.
		rcBar.left -= m_iMeterPosition;
		rcBar.right = m_iMeterPosition + m_iHorIndent + 1;

		switch (m_iDrawState)
		{
			case SPACEMETER_DRAWSTATE_NORMAL:
				DrawVertGradientRect(hDC,&rcBar,SPACEMETER_BARCOLOR_TOP,SPACEMETER_BARCOLOR_BOTTOM);
				break;

			case SPACEMETER_DRAWSTATE_WARN:
				DrawVertGradientRect(hDC,&rcBar,SPACEMETER_WARNBARCOLOR_TOP,SPACEMETER_WARNBARCOLOR_BOTTOM);
				break;

			case SPACEMETER_DRAWSTATE_FULL:
				DrawVertGradientRect(hDC,&rcBar,SPACEMETER_FULLBARCOLOR_TOP,SPACEMETER_FULLBARCOLOR_BOTTOM);
				break;
		}
	}
	else
	{
		g_VisualStyles.DrawThemeBackground(m_hProgressTheme,hDC,11,2,&rcBar,NULL);

		// Draw the allocated space.
		//rcBar.left++;
		rcBar.right = m_iMeterPosition + m_iHorIndent + 1;

		switch (m_iDrawState)
		{
			case SPACEMETER_DRAWSTATE_NORMAL:
				g_VisualStyles.DrawThemeBackground(m_hProgressTheme,hDC,5,4,&rcBar,NULL);
				break;

			case SPACEMETER_DRAWSTATE_WARN:
				g_VisualStyles.DrawThemeBackground(m_hProgressTheme,hDC,5,3,&rcBar,NULL);
				break;

			case SPACEMETER_DRAWSTATE_FULL:
				g_VisualStyles.DrawThemeBackground(m_hProgressTheme,hDC,5,2,&rcBar,NULL);
				break;
		}
	}
}

void CSpaceMeter::DrawFullBar(HDC hDC,RECT *pClientRect)
{
	int iBottom = pClientRect->bottom - SPACEMETER_BARINDENT_BOTTOM;

	RECT rcBar = {
		pClientRect->left + m_iHorIndent,
		iBottom - SPACEMETER_BAR_HEIGHT,
		pClientRect->right - m_iHorIndent,
		iBottom
	};

	// Draw the meter.
	DrawMeter(hDC,pClientRect,&rcBar);

	// Draw a different meter on Windows Vista systems.
	if (m_hProgressTheme == NULL)
	{
		FillRect(hDC,&rcBar,m_hFullBarBorderBrush);

		ContractRect(&rcBar,1);

		// Draw the allocated space.
		DrawVertGradientRect(hDC,&rcBar,SPACEMETER_FULLBARCOLOR_TOP,SPACEMETER_FULLBARCOLOR_BOTTOM);
	}
	else
	{
		g_VisualStyles.DrawThemeBackground(m_hProgressTheme,hDC,11,2,&rcBar,NULL);

		// Draw the allocated space.
		g_VisualStyles.DrawThemeBackground(m_hProgressTheme,hDC,5,2,&rcBar,NULL);
	}
}

void CSpaceMeter::DrawMeter(HDC hDC,RECT *pClientRect,RECT *pBarRect)
{
	int iLineTop = pBarRect->bottom + SPACEMETER_METER_SPACING;

	RECT rcLine = { 0, iLineTop, 0, iLineTop + SPACEMETER_METER_HEIGHT };
	RECT rcText = { 0, iLineTop, pBarRect->right, pClientRect->bottom };

	HFONT hOldFont = (HFONT)SelectObject(hDC,AtlGetDefaultGuiFont());

	for (unsigned int i = 0; i < SPACEMETER_METER_COUNT; i++)
	{
		// Draw the line.
		rcLine.left = pBarRect->left + i * m_iMeterSegmentSpacing;
		rcLine.right = rcLine.left + 1;

		::FillRect(hDC,&rcLine,::GetSysColorBrush(COLOR_WINDOWTEXT));

		// Draw the text.
		::SetBkMode(hDC,TRANSPARENT);
		::SetTextColor(hDC,::GetSysColor(COLOR_WINDOWTEXT));

		rcText.left = rcLine.left + SPACEMETER_METERTEXT_INDENT_LEFT;
		DrawText(hDC,m_uiMeterSegments[i],lstrlen(m_uiMeterSegments[i]),&rcText,
			DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);
	}

	SelectObject(hDC,hOldFont);
}

void CSpaceMeter::UpdateMeter(int iClientWidth)
{
	int iMeterWidth = iClientWidth - m_iHorIndent - m_iHorIndent - 2;
	m_iMeterSegmentSpacing = iMeterWidth / SPACEMETER_METER_COUNT;

	unsigned __int64 uiAllocatedSize = min(m_uiAllocatedSize,m_uiMeterSize);
	m_iMeterPosition = (int)(((float)uiAllocatedSize / m_uiMeterSize) * iMeterWidth);
}

void CSpaceMeter::UpdateToolTip()
{
	TCHAR szBuffer[64];
	const TCHAR *szUsed = lngGetString(SPACEMETER_USED);
	const TCHAR *szFree = lngGetString(SPACEMETER_FREE);
	unsigned __int64 uiFree = m_uiAllocatedSize > m_uiDiscSize ? 0 : m_uiDiscSize -  m_uiAllocatedSize;

	// Make sure that there will be no buffer overruns.
	if (lstrlen(szUsed) > 16)
		szUsed = g_szStringTable[SPACEMETER_USED];

	if (lstrlen(szFree) > 16)
		szFree = g_szStringTable[SPACEMETER_FREE];

	if (m_iDisplayMode == SPACEMETER_DMSIZE)
	{
		// Used.
		lstrcpy(m_szToolTip,szUsed);
		FormatBytes(szBuffer,m_uiAllocatedSize);
		lstrcat(m_szToolTip,szBuffer);

		lsnprintf_s(szBuffer,64,_T(" (%I64d Bytes)\r\n"),m_uiAllocatedSize);
		lstrcat(m_szToolTip,szBuffer);

		// Free.
		lstrcat(m_szToolTip,szFree);
		FormatBytes(szBuffer,uiFree);
		lstrcat(m_szToolTip,szBuffer);

		lsnprintf_s(szBuffer,64,_T(" (%I64d Bytes)"),uiFree);
		lstrcat(m_szToolTip,szBuffer);
	}
	else
	{
		// Used.
		lstrcpy(m_szToolTip,szUsed);

		lsnprintf_s(szBuffer,64,lngGetString(MISC_MINUTES),m_uiAllocatedSize/(1000 * 60));
		lstrcat(m_szToolTip,szBuffer);
		lstrcat(m_szToolTip,_T("\r\n"));

		// Free.
		lstrcat(m_szToolTip,szFree);

		lsnprintf_s(szBuffer,64,lngGetString(MISC_MINUTES),uiFree/(1000 * 60));
		lstrcat(m_szToolTip,szBuffer);
	}
}

void CSpaceMeter::SetDiscSize(unsigned int uiDiscSize)
{
	for (unsigned int i = 0; i < SPACEMETER_POPUPMENU_COUNT; i++)
		::CheckMenuItem(m_hPopupMenu,i,MF_BYPOSITION | MF_UNCHECKED);

	switch (uiDiscSize)
	{
		case SPACEMETER_SIZE_DLDVD:
			m_uiDiscSize = 8547991552;
			::CheckMenuItem(m_hPopupMenu,0,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_DVD:
			m_uiDiscSize = 4702989189;
			::CheckMenuItem(m_hPopupMenu,1,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_870MB:
			m_uiDiscSize = 912261120;
			::CheckMenuItem(m_hPopupMenu,2,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_791MB:
			m_uiDiscSize = 829423616;
			::CheckMenuItem(m_hPopupMenu,3,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_703MB:
			m_uiDiscSize = 737148928;
			::CheckMenuItem(m_hPopupMenu,4,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_650MB:
			m_uiDiscSize = 681574400;
			::CheckMenuItem(m_hPopupMenu,5,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_210MB:
			m_uiDiscSize = 220200960;
			::CheckMenuItem(m_hPopupMenu,6,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_185MB:
			m_uiDiscSize = 193986560;
			::CheckMenuItem(m_hPopupMenu,7,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_1020MIN:		// This is incorrect, I have not been able to find how many sectors a dual layer DVD can contain.
			m_uiDiscSize = 1020 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,0,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_510MIN:
			m_uiDiscSize = 510 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,1,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_99MIN:
			m_uiDiscSize = 99 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,2,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_90MIN:
			m_uiDiscSize = 90 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,3,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_80MIN:
			m_uiDiscSize = 80 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,4,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_74MIN:
			m_uiDiscSize = 74 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,5,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_24MIN:
			m_uiDiscSize = 24 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,6,MF_BYPOSITION | MF_CHECKED);
			break;

		case SPACEMETER_SIZE_21MIN:
			m_uiDiscSize = 21 * 1000 * 60;
			::CheckMenuItem(m_hPopupMenu,7,MF_BYPOSITION | MF_CHECKED);
			break;
	}

	unsigned __int64 uiSegmentSize = m_uiDiscSize / (SPACEMETER_METER_COUNT - 1);
	unsigned __int64 uiCurSegment = 0;

	if (m_iDisplayMode == SPACEMETER_DMSIZE)
	{
		for (unsigned int i = 0; i < SPACEMETER_METER_COUNT; i++)
		{
			// For segments larger than 1GB we allow two decimals (by using the standard FormatBytes function).
			if (m_uiMeterSegments[i],uiCurSegment > 1024 * 1024 * 1024)
				FormatBytes(m_uiMeterSegments[i],uiCurSegment);
			else
				FormatBytesEx(m_uiMeterSegments[i],uiCurSegment);

			uiCurSegment += uiSegmentSize;
		}
	}
	else
	{
		for (unsigned int i = 0; i < SPACEMETER_METER_COUNT; i++)
		{
			// Minutes.
			lsnprintf_s(m_uiMeterSegments[i],SPACEMETER_METERTEXT_SIZE,_T("%d min"),uiCurSegment/(1000 * 60));

			uiCurSegment += uiSegmentSize;
		}
	}

	// The meter hold the disc size + one segment.
	m_uiMeterSize = m_uiDiscSize + uiSegmentSize;
}


void CSpaceMeter::SetAllocatedSize(unsigned __int64 uiAllocatedSize)
{
	m_uiAllocatedSize = uiAllocatedSize;

	if (m_uiAllocatedSize > m_uiMeterSize)
		m_iDrawState = SPACEMETER_DRAWSTATE_OUTOFSCOPE;
	else if (m_uiAllocatedSize > m_uiDiscSize)
	{
		unsigned __int64 uiOverBurnAmount = m_iDisplayMode == SPACEMETER_DMSIZE ?
			SPACEMETER_OVERBURNSIZE : SPACEMETER_OVERBURNLENGTH;

		if (m_uiAllocatedSize < m_uiDiscSize + uiOverBurnAmount)
			m_iDrawState = SPACEMETER_DRAWSTATE_WARN;
		else
			m_iDrawState = SPACEMETER_DRAWSTATE_FULL;
	}
	else
	{
		m_iDrawState = SPACEMETER_DRAWSTATE_NORMAL;
	}

	// Update the meter.
	RECT rcClient;
	GetClientRect(&rcClient);

	UpdateMeter(rcClient.right);

	// Finally update the tooltip text buffer.
	UpdateToolTip();
}

void CSpaceMeter::IncreaseAllocatedSize(unsigned __int64 uiSize)
{
	SetAllocatedSize(m_uiAllocatedSize + uiSize);
}

void CSpaceMeter::DecreaseAllocatedSize(unsigned __int64 uiSize)
{
	SetAllocatedSize(m_uiAllocatedSize - uiSize);
}

unsigned __int64 CSpaceMeter::GetAllocatedSize()
{
	return m_uiAllocatedSize;
}

void CSpaceMeter::SetDisplayMode(int iDisplayMode)
{
	m_iDisplayMode = iDisplayMode;
}

void CSpaceMeter::ForceRedraw()
{
	RECT rcSpaceMeter;
	GetClientRect(&rcSpaceMeter);

	InvalidateRect(&rcSpaceMeter,true);
}

void CSpaceMeter::Initialize()
{
	// Create the tool tip.
	m_ToolTip.Create(m_hWnd);
	m_ToolTip.SetMaxTipWidth(200);

	CToolInfo ti(0,m_hWnd,SPACEMETER_TOOLTIP_ID,NULL,LPSTR_TEXTCALLBACK);

	m_ToolTip.AddTool(&ti);
	m_ToolTip.Activate(true);

	// Initialize the progress theme data.
	if (g_WinVer.m_ulMajorVersion == MAJOR_WINVISTA &&
		g_WinVer.m_ulMinorVersion == MINOR_WINVISTA)
	{
		m_hProgressTheme = g_VisualStyles.OpenThemeData(m_hWnd,L"PROGRESS");
	}
}

LRESULT CSpaceMeter::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// If the application is themed, we use a different horizontal indentation.
	if (!g_VisualStyles.IsThemeActive())
		m_iHorIndent = SPACEMETER_BARINDENT_NORMAL;

	return 0;
}

LRESULT CSpaceMeter::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	LRESULT lResult = DefWindowProc(uMsg,wParam,lParam);
	unsigned long ulNewSize = LOWORD(lParam);

	unsigned int uiWidth = LOWORD(lParam);
	UpdateMeter(uiWidth);

	// Update the tooltip region.
	if (m_ToolTip.IsWindow())
	{
		RECT rcClient = { 0,0,uiWidth,HIWORD(lParam) };
		m_ToolTip.SetToolRect(m_hWnd,SPACEMETER_TOOLTIP_ID,&rcClient);
	}

	return lResult;
}

LRESULT CSpaceMeter::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CPaintDC dc(m_hWnd);

	RECT rcClient;
	GetClientRect(&rcClient);

	HDC hMemDC;
	HBITMAP hMemBitmap;

	hMemDC = CreateCompatibleDC(dc);
	hMemBitmap = CreateCompatibleBitmap(dc,rcClient.right,rcClient.bottom);

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC,hMemBitmap);
 
	// Draw the background.
	FillRect(hMemDC,&rcClient,GetSysColorBrush(COLOR_BTNFACE));

	// Draw the bar.
	if (m_iDrawState == SPACEMETER_DRAWSTATE_OUTOFSCOPE)
		DrawFullBar(hMemDC,&rcClient);		// Just a faster draw method.
	else
		DrawBar(hMemDC,&rcClient);

	BitBlt(dc,0,0,rcClient.right,rcClient.bottom,hMemDC,0,0,SRCCOPY);

	SelectObject(hMemDC,hOldBitmap);

	ReleaseDC(dc);
	DeleteDC(hMemDC);

	DeleteObject(hMemBitmap);

	return 0;
}

LRESULT CSpaceMeter::OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	return true;
}

LRESULT CSpaceMeter::OnRButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	POINT CursorPos;
	GetCursorPos(&CursorPos);

	TrackPopupMenuEx(m_hPopupMenu,0,CursorPos.x,CursorPos.y,m_hWnd,NULL);

	bHandled = false;
	return 0;
}

LRESULT CSpaceMeter::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_ToolTip.IsWindow())
	{
		MSG Msg = { m_hWnd,uMsg,wParam,lParam };
		m_ToolTip.RelayEvent(&Msg);
	}

	bHandled = false;
	return 0;
}

LRESULT CSpaceMeter::OnGetDispInfo(int idCtrl,LPNMHDR pnmh,BOOL &bHandled)
{
	// Make sure that the notification is about the control we care about.
	if (pnmh->hwndFrom != m_ToolTip)
	{
		bHandled = FALSE;
		return 0;
	}

	LPNMTTDISPINFO pNMTDI = (LPNMTTDISPINFO)pnmh;
	pNMTDI->lpszText = m_szToolTip;

	return 0;
}

LRESULT CSpaceMeter::OnPopupMenuClick(UINT uNotifyCode,int nID,CWindow wnd)
{
	nID -= SPACEMETER_POPUPMENU_IDBASE;

	if (m_iDisplayMode == SPACEMETER_DMSIZE)
		SetDiscSize(nID);
	else
		SetDiscSize(nID + SPACEMETER_POPUPMENU_COUNT);
	
	SetAllocatedSize(m_uiAllocatedSize);
	ForceRedraw();

	return 0;
}
