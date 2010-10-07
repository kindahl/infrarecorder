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

#pragma once
#include <png.h>

class CPngFile
{
private:
	class CMemFile
	{
	public:
		unsigned char *m_pData;
		size_t m_iRemainBytes;

		CMemFile(unsigned char *pData,size_t iRemainBytes) :
			m_pData(pData),m_iRemainBytes(iRemainBytes)
		{
		}
	};

	unsigned long m_ulWidth;
	unsigned long m_ulHeight;

	unsigned char **m_pRowData;

	static void ReadMemFile(png_structp pPng,png_bytep pData,png_size_t iSize);

public:
	CPngFile();
	~CPngFile();

	bool Open(unsigned short usResourceId);
	bool Open(const TCHAR *szFullPath);
	bool Close();

	bool Draw(HDC hDC,int iX,int iY,int iWidth,int iHeight);
};
