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
#include "CustomPageItem.h"

CCustomPageItem::CCustomPageItem(unsigned int uiImageIndex,unsigned int uiCommandID)
{
	m_uiImageIndex = uiImageIndex;
	m_pImageList = NULL;

	m_hBoldFont = AtlCreateBoldFont(AtlGetDefaultGuiFont());

	m_uiCommandID = uiCommandID;

	// Create underlined font.
	m_hUnderlineFont = NULL;
	LOGFONT lf;
	memset(&lf,0,sizeof(LOGFONT));

	if (::GetObject(AtlGetDefaultGuiFont(),sizeof(LOGFONT),&lf) == sizeof(LOGFONT))
	{
		lf.lfUnderline = true;
		m_hUnderlineFont = ::CreateFontIndirect(&lf);
	}

	// Create underlines bold font.
	m_hUnderlineBoldFont = NULL;
	memset(&lf,0,sizeof(LOGFONT));

	if (::GetObject(m_hBoldFont,sizeof(LOGFONT),&lf) == sizeof(LOGFONT))
	{
		lf.lfUnderline = true;
		m_hUnderlineBoldFont = ::CreateFontIndirect(&lf);
	}	
}

CCustomPageItem::~CCustomPageItem()
{
	::DeleteObject(m_hBoldFont);
	::DeleteObject(m_hUnderlineFont);
	::DeleteObject(m_hUnderlineBoldFont);
}

void CCustomPageItem::SetText(const TCHAR *szTitle,const TCHAR *szDesc)
{
	lstrcpy(m_szTitle,szTitle);
	m_uiTitleLength = lstrlen(m_szTitle);
	lstrcpy(m_szDesc,szDesc);
	m_uiDescLength = lstrlen(m_szDesc);
}

void CCustomPageItem::SetImageList(CImageList *pImageList)
{
	m_pImageList = pImageList;
}

void CCustomPageItem::Draw(HDC hDC,RECT *pRect,bool bHover)
{
	RECT rcIcon;
	rcIcon.left = pRect->left;
	rcIcon.top = pRect->top;
	rcIcon.right = rcIcon.left + 32;
	rcIcon.bottom = rcIcon.top + 32;

	if (m_pImageList != NULL)
	{
		POINT ptImage = { pRect->left,pRect->top };
		m_pImageList->Draw(hDC,m_uiImageIndex,ptImage,ILS_NORMAL);
	}

	// Draw the text.
	HFONT hTitleFont = bHover ? m_hUnderlineBoldFont : m_hBoldFont;
	HFONT hOldFont = (HFONT)SelectObject(hDC,hTitleFont);

	RECT rcText = rcIcon;
	rcText.left = rcIcon.right + CUSTOMPAGEITEM_TEXT_LEFTINDENT;
	rcText.right = pRect->right;

	::SetBkMode(hDC,TRANSPARENT);
	::SetTextColor(hDC,CUSTOMPAGEITEM_TEXT_COLOR);

	int iHeight = DrawText(hDC,m_szTitle,m_uiTitleLength,&rcText,DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	// Draw the description.
	if (bHover)
		(HFONT)SelectObject(hDC,m_hUnderlineFont);
	else
		(HFONT)SelectObject(hDC,AtlGetDefaultGuiFont());

	rcText.left = rcIcon.right + CUSTOMPAGEITEM_TEXT_LEFTINDENT;
	rcText.right = pRect->right;
	rcText.top += iHeight;

	DrawText(hDC,m_szDesc,m_uiDescLength,&rcText,DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	// Clean up.
	SelectObject(hDC,hOldFont);
}

unsigned int CCustomPageItem::GetCommandID()
{
	return m_uiCommandID;
}
