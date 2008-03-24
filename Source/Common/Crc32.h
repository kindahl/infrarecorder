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

#pragma once
#include "Progress.h"

#define CRC32_FILE_BUFFERSIZE			4096
#define CRC32_FILE_POLYNOMIAL			0xEDB88320

class CCrc32
{
private:
	unsigned long m_ulValue;
	unsigned long m_ulTable[256];

public:
	CCrc32(unsigned int uiPolynomial);

	void Reset();
	void Update(unsigned char *pBuffer,unsigned int uiLength);
	unsigned long GetValue();
};

class CCrc32File
{
private:
	CProgress *m_pProgress;
	CFilesProgress *m_pFilesProgress;
	unsigned __int64 m_uiTotalBytes;

public:
	CCrc32File(CProgress *pProgress,CFilesProgress *pFilesProgress);

	unsigned long Calculate(const TCHAR *szFileName);
};
