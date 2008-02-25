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
#include "GraphUtil.h"

void DrawVertGradientRect(HDC hDC,RECT *pRect,COLORREF cTop,COLORREF cBottom)
{
	unsigned int uiClientHeight = pRect->bottom - pRect->top;

	RECT rcBand;
	rcBand.left = pRect->left;
	rcBand.right = pRect->right;

	int iBeginRGB[3];
	int iRGBDiff[3];
	int iR,iG,iB;

	iBeginRGB[0] = GetRValue(cTop);
	iBeginRGB[1] = GetGValue(cTop);
	iBeginRGB[2] = GetBValue(cTop);

	iRGBDiff[0] = GetRValue(cBottom) - iBeginRGB[0];
	iRGBDiff[1] = GetGValue(cBottom) - iBeginRGB[1];
	iRGBDiff[2] = GetBValue(cBottom) - iBeginRGB[2];

	for (int i = pRect->top,j = 0; i < pRect->bottom; i++,j++)
	{
		iR = iBeginRGB[0] + MulDiv(j,iRGBDiff[0],uiClientHeight);
		iG = iBeginRGB[1] + MulDiv(j,iRGBDiff[1],uiClientHeight);
		iB = iBeginRGB[2] + MulDiv(j,iRGBDiff[2],uiClientHeight);

		rcBand.top = (int)(i);
		rcBand.bottom = (int)((i + 1));

		HBRUSH hBrush = CreateSolidBrush(RGB(iR,iG,iB));
			FillRect(hDC,&rcBand,hBrush);
		DeleteObject(hBrush);
	}
}

void DrawHorGradientRect(HDC hDC,RECT *pRect,COLORREF cLeft,COLORREF cRight)
{
	unsigned int uiClientWidth = pRect->right - pRect->left;

	RECT rcBand;
	rcBand.top = pRect->top;
	rcBand.bottom = pRect->bottom;

	int iBeginRGB[3];
	int iRGBDiff[3];
	int iR,iG,iB;

	iBeginRGB[0] = GetRValue(cLeft);
	iBeginRGB[1] = GetGValue(cLeft);
	iBeginRGB[2] = GetBValue(cLeft);

	iRGBDiff[0] = GetRValue(cRight) - iBeginRGB[0];
	iRGBDiff[1] = GetGValue(cRight) - iBeginRGB[1];
	iRGBDiff[2] = GetBValue(cRight) - iBeginRGB[2];

	for (int i = pRect->left,j = 0; i < pRect->right; i++,j++)
	{
		iR = iBeginRGB[0] + MulDiv(j,iRGBDiff[0],uiClientWidth);
		iG = iBeginRGB[1] + MulDiv(j,iRGBDiff[1],uiClientWidth);
		iB = iBeginRGB[2] + MulDiv(j,iRGBDiff[2],uiClientWidth);

		rcBand.left = (int)(i);
		rcBand.right = (int)((i + 1));

		HBRUSH hBrush = CreateSolidBrush(RGB(iR,iG,iB));
			FillRect(hDC,&rcBand,hBrush);
		DeleteObject(hBrush);
	}
}

void ContractRect(RECT *pRect,int iSize)
{
	pRect->left += iSize;
	pRect->right -= iSize;
	pRect->top += iSize;
	pRect->bottom -= iSize;
}
