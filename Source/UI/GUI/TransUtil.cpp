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
#include "TransUtil.h"

/*
	UpdateStaticWidth
	-----------------
	Updates the width of the specified static control so it fits the specified
	text. The function return the right end position of the static control.
*/
int UpdateStaticWidth(HWND hParentWnd,HWND hCtrlWnd,const TCHAR *szText)
{
	// Calculate text width using the default system font.
	HDC hDC = GetDC(hParentWnd);
	HFONT hOldFont = (HFONT)::SelectObject(hDC,AtlGetDefaultGuiFont());

	RECT rcText = { 0 };
	::DrawText(hDC,szText,lstrlen(szText),&rcText,DT_LEFT | DT_CALCRECT | DT_SINGLELINE);

	::SelectObject(hDC,hOldFont);
	::ReleaseDC(hParentWnd,hDC);

	if (rcText.right < 75)
		return -1;

	// Get static rectangle.
	RECT rcStatic = { 0 };
	::GetWindowRect(hCtrlWnd,&rcStatic);

	::ScreenToClient(hParentWnd,(LPPOINT)&rcStatic);
	::ScreenToClient(hParentWnd,((LPPOINT)&rcStatic) + 1);

	// Update static rectangle.
	rcStatic.right = rcStatic.left + rcText.right + TRANSUTIL_STATICEDIT_SPACING;
	::MoveWindow(hCtrlWnd,rcStatic.left,rcStatic.top,rcStatic.right - rcStatic.left,rcStatic.bottom - rcStatic.top,TRUE);

	return rcStatic.right;
}

int UpdateStaticWidth(HWND hParentWnd,int iStaticID,const TCHAR *szText)
{
	return UpdateStaticWidth(hParentWnd,GetDlgItem(hParentWnd,iStaticID),szText);
}

/*
	UpdateEditPos
	-------------
	Updates the left position of the specified edit control. If bMove is set to
	true the edit control will be moved (the right position will be updated as
	well).
*/
void UpdateEditPos(HWND hParentWnd,int iEditID,int iLeft,bool bMove)
{
	RECT rcEdit = { 0 };
	::GetWindowRect(::GetDlgItem(hParentWnd,iEditID),&rcEdit);
	::ScreenToClient(hParentWnd,(LPPOINT)&rcEdit);
	::ScreenToClient(hParentWnd,((LPPOINT)&rcEdit) + 1);

	int iDiff = rcEdit.left - iLeft;
	rcEdit.left = iLeft;

	::MoveWindow(::GetDlgItem(hParentWnd,iEditID),rcEdit.left,
		rcEdit.top,rcEdit.right - rcEdit.left - (bMove ? iDiff : 0),
		rcEdit.bottom - rcEdit.top,TRUE);
}

int UpdateStaticHeight(HWND hParentWnd,int iStaticID,const TCHAR *szText)
{
	RECT rcStatic = { 0 };
	::GetWindowRect(::GetDlgItem(hParentWnd,iStaticID),&rcStatic);
	::ScreenToClient(hParentWnd,(LPPOINT)&rcStatic);
	::ScreenToClient(hParentWnd,((LPPOINT)&rcStatic) + 1);

	HDC hDC = GetDC(hParentWnd);
	HFONT hOldFont = (HFONT)::SelectObject(hDC,AtlGetDefaultGuiFont());

	RECT rcText = { 0 };
	rcText.right = rcStatic.right - rcStatic.left;
	int iTextHeight = DrawText(hDC,szText,lstrlen(szText),&rcText,DT_LEFT | DT_CALCRECT | DT_WORDBREAK);
	int iDiff = iTextHeight - (rcStatic.bottom - rcStatic.top);

	if (iDiff > 0)
		::MoveWindow(::GetDlgItem(hParentWnd,iStaticID),rcStatic.left,rcStatic.top,rcStatic.right - rcStatic.left,iTextHeight,TRUE);

	::SelectObject(hDC,hOldFont);
	::ReleaseDC(hParentWnd,hDC);
	return iDiff;
}
