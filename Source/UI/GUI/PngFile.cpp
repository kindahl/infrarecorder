/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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
#include "Resource.h"
#include "PngFile.h"

CPngFile::CPngFile() : m_ulWidth(0),m_ulHeight(0),m_pRowData(NULL)
{
}

CPngFile::~CPngFile()
{
	Close();
}

void CPngFile::ReadMemFile(png_structp pPng,png_bytep pData,png_size_t iSize)
{
	CMemFile *pMemFile = (CMemFile *)pPng->io_ptr;
	if (pMemFile->m_iRemainBytes >= iSize)
	{
		memcpy(pData,pMemFile->m_pData,iSize);
		pMemFile->m_pData += iSize;
		pMemFile->m_iRemainBytes -= iSize;
	}
	else
	{
		png_error(pPng,"io error");
	}
}

bool CPngFile::Open(unsigned short usResourceId)
{
	HRSRC hPngResource = FindResource(NULL,MAKEINTRESOURCE(usResourceId),_T("PNG"));
	if (hPngResource == NULL)
		return false;

	unsigned long ulSize = SizeofResource(NULL,hPngResource);
	HGLOBAL hPngData = LoadResource(NULL,hPngResource);
	unsigned char *pData = (unsigned char *)LockResource(hPngData);

	if (pData == NULL)
		return false;

	// Read the data.
	CMemFile MemFile(pData,ulSize);

	// Create data structures.
	png_structp pRead = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
	png_infop pInfo = png_create_info_struct(pRead);

	png_set_read_fn(pRead,&MemFile,ReadMemFile);

	// Read entire image.
	png_read_png(pRead,pInfo,PNG_TRANSFORM_EXPAND,0);
	
	if (pInfo->valid & PNG_INFO_IDAT && pInfo->pixel_depth == 32)
	{
		m_ulWidth = pInfo->width;
		m_ulHeight = pInfo->height;
		m_pRowData = pInfo->row_pointers;
		pInfo->row_pointers = NULL;

		for (unsigned long y = 0; y < m_ulHeight; y++)
		{
			unsigned char *pRowData = m_pRowData[y];
			for (unsigned long x = 0; x < m_ulWidth; x++)
			{
				pRowData[0] = (pRowData[0] * pRowData[3]) / 255;
				pRowData[1] = (pRowData[1] * pRowData[3]) / 255;
				pRowData[2] = (pRowData[2] * pRowData[3]) / 255;
				pRowData[3] = ~pRowData[3];
				pRowData += 4;
			}
		}

		png_destroy_read_struct(&pRead,&pInfo,0);

		GlobalUnlock(hPngData);
		return true;
	}

	png_destroy_read_struct(&pRead,&pInfo,0);

	GlobalUnlock(hPngData);
	return true;
}

bool CPngFile::Open(const TCHAR *szFullPath)
{
#ifdef _UNICODE
	FILE *pFile = _wfopen(szFullPath,_T("rb"));
#else
	FILE *pFile = fopen(szFullPath,"rb");
#endif
	if (pFile == NULL)
		return false;

	// Create data structures.
	png_structp pRead = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
	png_infop pInfo = png_create_info_struct(pRead);

	png_init_io(pRead,pFile);

	// Read entire image.
	png_read_png(pRead,pInfo,PNG_TRANSFORM_EXPAND,0);
	fclose(pFile);

	if (pInfo->valid & PNG_INFO_IDAT && pInfo->pixel_depth == 32)
	{
		m_ulWidth = pInfo->width;
		m_ulHeight = pInfo->height;
		m_pRowData = pInfo->row_pointers;
		pInfo->row_pointers = NULL;

		for (unsigned long y = 0; y < m_ulHeight; y++)
		{
			unsigned char *pData = m_pRowData[y];
			for (unsigned long x = 0; x < m_ulWidth; x++)
			{
				pData[0] = (pData[0] * pData[3]) / 255;
				pData[1] = (pData[1] * pData[3]) / 255;
				pData[2] = (pData[2] * pData[3]) / 255;
				pData[3] = ~pData[3];
				pData += 4;
			}
		}

		png_destroy_read_struct(&pRead,&pInfo,0);
		return true;
	}

	png_destroy_read_struct(&pRead,&pInfo,0);
	return false;
}

bool CPngFile::Close()
{
	if (m_pRowData == NULL)
		return false;

	// FIXME: This does not work if libpng is linked dynamically. Maybe look into
	//		  saving the pInfo structure and free it with png_destroy_read_struct
	//		  here.
	for (unsigned long y = 0; y < m_ulHeight; y++)
		free(m_pRowData[y]);

	free(m_pRowData);
	return true;
}

bool CPngFile::Draw(HDC hDC,int iX,int iY,int iWidth,int iHeight)
{
	if (iWidth > (int)m_ulWidth)
		iWidth = m_ulWidth;

	if (iHeight > (int)m_ulHeight)
		iHeight = m_ulHeight;

	if (iWidth == 0 || iHeight == 0)
		return false;

	HDC hMemDC = NULL;
	HBITMAP hDestBitmap = NULL,hOldBitmap = NULL;

	unsigned char *pDestBits = NULL;
	unsigned char **pRowData = m_pRowData + m_ulHeight - 1 - (m_ulHeight - iHeight);

	//Do alpla blend
	int iLineTailDest = (((24 * iWidth) + 31) / 32 * 4) - 3 * iWidth;
	
	BITMAPINFO bmi;
	ZeroMemory(&bmi,sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = iHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	// Create device context.
	hMemDC = CreateCompatibleDC(hDC);

	hDestBitmap = CreateDIBSection(hDC,&bmi,DIB_RGB_COLORS,(void **)&pDestBits,NULL,0);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC,hDestBitmap);

	BitBlt(hMemDC,0,0,iWidth,iHeight,hDC,iX,iY,SRCCOPY);

	BYTE *p1 = pDestBits;
	for (int y = 0; y < iHeight; y++)
	{
		BYTE *p2 = *(pRowData--);
		for (int x = 0; x < iWidth; x++)
		{
			*p1++ = ((p2[3] * (*p1)) >> 8) + p2[2];	// B
			*p1++ = ((p2[3] * (*p1)) >> 8) + p2[1];	// G
			*p1++ = ((p2[3] * (*p1)) >> 8) + p2[0];	// R

			p2 += 4;
		}
		p1 += iLineTailDest;
	}

	// Render.
	BitBlt(hDC,iX,iY,iWidth,iHeight,hMemDC,0,0,SRCCOPY);
	SelectObject(hMemDC,hOldBitmap);

	//Free memory.
	DeleteObject(hDestBitmap);
	DeleteDC(hMemDC);

	return true;
}
