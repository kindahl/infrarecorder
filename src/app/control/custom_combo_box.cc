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

#include "stdafx.hh"
#include "custom_combo_box.hh"

/*
	A custom combo box control that provides item icons to the left of the text.
	The data property determines the icon index of an combo box item.

	To get proper dimensions:
	  m_TrackCombo.SetItemHeight( 0,16);
	  m_TrackCombo.SetItemHeight(-1,16);	// Static item.
*/
CCustomComboBox::CCustomComboBox() : m_hImageList(NULL),m_iImageList(0)
{
}

CCustomComboBox::~CCustomComboBox()
{
}

void CCustomComboBox::SetImageList(HIMAGELIST hImageList,int iImageList)
{
	m_hImageList = hImageList;
	m_iImageList = iImageList;
}

void CCustomComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDCHandle dc = lpDrawItemStruct->hDC;
	RECT rcItem = lpDrawItemStruct->rcItem;
	
	bool bSelected = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
	bool bFocused = (lpDrawItemStruct->itemState & ODS_FOCUS) != 0;
	bool bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;

	int iBackgroundColor;
	if (bDisabled)
		iBackgroundColor = COLOR_BTNFACE;
	else if (bSelected)
		iBackgroundColor = COLOR_HIGHLIGHT;
	else
		iBackgroundColor = COLOR_WINDOW;

	// Draw the background.
	HBRUSH hBackground = ::GetSysColorBrush(iBackgroundColor);
	dc.FillRect(&rcItem,hBackground);

	// Draw icon.
	if (!bDisabled && m_hImageList != NULL)
	{
		ImageList_Draw(m_hImageList,(int)lpDrawItemStruct->itemData,dc,
			rcItem.left,
			rcItem.top,ILD_TRANSPARENT);

		rcItem.left += 16 + CUSTOMCOMBO_ICONSPACING;
	}

	// Draw the text.
	HFONT hOldFont = (HFONT)dc.SelectFont(AtlGetDefaultGuiFont());

	int iTextColor;
	if (bDisabled)
		iTextColor = COLOR_GRAYTEXT;
	else if (bSelected)
		iTextColor = COLOR_HIGHLIGHTTEXT;
	else
		iTextColor = COLOR_WINDOWTEXT;

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(::GetSysColor(iTextColor));

	TCHAR szText[128];
	if (GetLBTextLen(lpDrawItemStruct->itemID) >= (sizeof(szText) / sizeof(TCHAR) - 1))
	{
		TCHAR *szVarText = new TCHAR[GetLBTextLen(lpDrawItemStruct->itemID) + 1];
		GetLBText(lpDrawItemStruct->itemID,szVarText);

		dc.DrawText(szVarText,lstrlen(szVarText),&rcItem,DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);
		delete [] szVarText;
	}
	else
	{
		GetLBText(lpDrawItemStruct->itemID,szText);

		dc.DrawText(szText,lstrlen(szText),&rcItem,DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);
	}

	if (bFocused)
		dc.DrawFocusRect(&rcItem);

	dc.SelectFont(hOldFont);
}
