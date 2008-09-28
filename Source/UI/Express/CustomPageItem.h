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

#pragma once

#define CUSTOMPAGEITEM_TEXT_LEFTINDENT		8
#define CUSTOMPAGEITEM_TEXT_COLOR			RGB(0,0,0)
#define CUSTOMPAGEITEM_TEXT_MAXTITLE		48
#define CUSTOMPAGEITEM_TEXT_MAXDESC			128

class CCustomPageItem
{
private:
	CImageList *m_pImageList;
	unsigned int m_uiImageIndex;

	HFONT m_hBoldFont;
	HFONT m_hUnderlineFont;
	HFONT m_hUnderlineBoldFont;

	TCHAR m_szTitle[CUSTOMPAGEITEM_TEXT_MAXTITLE];
	unsigned int m_uiTitleLength;
	TCHAR m_szDesc[CUSTOMPAGEITEM_TEXT_MAXDESC];
	unsigned int m_uiDescLength;

	unsigned int m_uiCommandID;

public:
	CCustomPageItem(unsigned int uiImageIndex,unsigned int uiCommandID);
	~CCustomPageItem();

	void SetText(const TCHAR *szTitle,const TCHAR *szDesc);
	void SetImageList(CImageList *pImageList);

	void Draw(HDC hDC,RECT *pRect,bool bHover);

	unsigned int GetCommandID();
};
