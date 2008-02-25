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
#include "CRC32.h"
#include "FileManager.h"

CCRC32::CCRC32(unsigned int uiPolynomial)
{
	m_ulValue = 0xFFFFFFFF;

	// Calculate the table entries.
	unsigned long ulCRC;

	for (int i = 0; i < 256; i++)
	{
		ulCRC = i;

		for (int j = 0; j < 8; j++)
		{
			if (ulCRC & 1)
				ulCRC = (ulCRC >> 1) ^ uiPolynomial;
			else
				ulCRC = (ulCRC >> 1);
		}

		m_ulTable[i] = ulCRC;
	}

	Reset();
}

/*
	CCRC32::Reset
	-------------
	Resets the internal CRC32 value.
*/
void CCRC32::Reset()
{
	m_ulValue = 0xFFFFFFFF;
}

/*
	CCRC32::Calculate
	-----------------
	Calculates the CRC32 value of the specified buffer.
*/
void CCRC32::Update(unsigned char *pBuffer,unsigned int uiLength)
{
	unsigned long ulCRC = m_ulValue;

	for (unsigned int i = 0; i < uiLength; i++)
		ulCRC = (ulCRC >> 8) ^ m_ulTable[(unsigned char)ulCRC ^ pBuffer[i]];
 
	m_ulValue = ulCRC;
}

/*
	CCRC32::GetValue
	----------------
	Returns the current calculated CRC32 value.
*/
unsigned long CCRC32::GetValue()
{
	return m_ulValue ^ 0xFFFFFFFF;
}

CCRC32File::CCRC32File(CProgress *pProgress,CFilesProgress *pFilesProgress)
{
	m_pProgress = pProgress;
	m_pFilesProgress = pFilesProgress;
}

/*
	CCRC32File::Calculate
	---------------------
	Calculates the CRC32 checksum of the specified file.
*/
unsigned long CCRC32File::Calculate(const TCHAR *szFileName)
{
	CCRC32 CRC32(CRC32_FILE_POLYNOMIAL);

	HANDLE hFile = fs_open(szFileName,_T("rb"));
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	unsigned char ucBuffer[CRC32_FILE_BUFFERSIZE];
	unsigned long ulRead = fs_read(ucBuffer,CRC32_FILE_BUFFERSIZE,hFile);
	while (ulRead > 0)
	{
		if (m_pProgress->IsCanceled())
			break;

		CRC32.Update(ucBuffer,ulRead);
		m_pProgress->SetProgress(m_pFilesProgress->UpdateProcessed(ulRead));

		ulRead = fs_read(ucBuffer,CRC32_FILE_BUFFERSIZE,hFile);
	}

	fs_close(hFile);
	return CRC32.GetValue();
}
