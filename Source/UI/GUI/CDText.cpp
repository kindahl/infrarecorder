/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include <math.h>
#include "CDText.h"
#include "../../Common/FileManager.h"
#include "../../Common/StringUtil.h"

void CCDText::InitCRC()
{
	// Compute basis polynomials.
	unsigned short usBasePoly[8];
	usBasePoly[0] = CDTEXT_CRCPOLYNOMIAL;

	for (int i = 1; i < 8; i++)
	{
		usBasePoly[i] = usBasePoly[i - 1] << 1;

		if ((usBasePoly[i - 1] >> 15) & 1)
			usBasePoly[i] ^= usBasePoly[0];

		usBasePoly[i] &= 0x0ffff;
	}

	// Calculate the table entries.
	for (int i = 0; i < 256; i++)
	{
		int iCurrent = i;
		m_usCRCTable[i] = 0;

		for (int j = 0; j < 8; j++)
		{
			if (iCurrent & 1)
				m_usCRCTable[i] ^= usBasePoly[j];

			iCurrent >>= 1;
		}
	}
}

unsigned short CCDText::CalcCRC(unsigned char *pBuffer,unsigned int uiLength)
{
	unsigned short usCRC = 0;

	for (int i = 0; i < (int)uiLength; i++)
		usCRC = (usCRC << 8) ^ m_usCRCTable[(usCRC >> 8) ^ pBuffer[i]];
 
	return usCRC;
}

CCDText::CCDText()
{
	Reset();

	// Initialize the CRC table.
	InitCRC();
}

CCDText::~CCDText()
{
	m_TrackNames.clear();
	m_ArtistNames.clear();
}

void CCDText::Reset()
{
	m_szAlbumName[0] = '\0';
	m_szArtistName[0] = '\0';

	m_uiBufferPos = 0;
	m_szBuffer[0] = '\0';

	m_TrackNames.clear();
	m_ArtistNames.clear();
}

unsigned int CCDText::FindBufferEOS(unsigned int uiStart,unsigned int uiEnd)
{
	for (unsigned int i = uiStart; i < uiEnd; i++)
	{
		if (m_szBuffer[i] == '\0')
			return i;
	}

	return 0;
}

unsigned int CCDText::ReadPacket(HANDLE hFile,unsigned long ulPID,
								  unsigned long ulBlockInfo)
{
	// If the buffer can't hold the data we abort.
	if (m_uiBufferPos + 12 >= CDTEXT_MAXFIELDSIZE)
		return 0;

	// Not currently used.
	unsigned char ucDBCC = (unsigned char)(ulBlockInfo & 0x80) >> 7;
	unsigned char ucBlockNumber = (unsigned char)(ulBlockInfo & 0x70) >> 4;
	unsigned char ucCharPos = (unsigned char)(ulBlockInfo & 0x0F);

	fs_read(m_szBuffer + m_uiBufferPos,12,hFile);

	// CRC check. Not currently used. (ulTemp should contain the complete header)
	/*unsigned char ucTemp[17];
	memcpy(ucTemp,&ulTemp,4);
	memcpy(ucTemp + 4,m_szBuffer + m_uiBufferPos,12);
		
	TCHAR szTemp[128];
	swprintf(szTemp,_T("0x%x"),CalcCRC((unsigned char *)ucTemp,16));
	MessageBox(NULL,szTemp,_T("Calculated CRC"),MB_OK);*/

	// Debug information.
	char szTemp[25];
	sprintf(szTemp,"%d, %d, %d",(int)(ulPID & 0xFF0000) >> 16,(int)(ulPID & 0xFF00) >> 8,(int)ulPID & 0xFF);

	/*char szTemp[25];
	sprintf(szTemp,"%d, %d, %d",(int)ucDBCC,(int)ucBlockNumber,(int)ucCharPos);*/

	char szTemp2[13];
	memcpy(szTemp2,m_szBuffer + m_uiBufferPos,12);
	szTemp2[12] = '\0';
	//MessageBoxA(NULL,szTemp2,szTemp,MB_OK);

	unsigned int uiEOS = FindBufferEOS(m_uiBufferPos,m_uiBufferPos + 12);

	m_uiBufferPos += 12;

	if (uiEOS == 0)
		return 12;

	switch (ulPID & 0xFF)
	{
		case PTI_NAMETITLE:
			// If the second PID bytes is zero it's an album name, otherwise it's a track name.
			if (((ulPID & 0xFF00) >> 8) == 0)
				memcpy(m_szAlbumName,m_szBuffer,uiEOS + 1);
			else
				m_TrackNames.push_back(m_szBuffer);
			break;

		case PTI_NAMEPERFORMER:
			if (((ulPID & 0xFF00) >> 8) == 0)
				memcpy(m_szArtistName,m_szBuffer,uiEOS + 1);
			else
				m_ArtistNames.push_back(m_szBuffer);
			break;

		// We ignore all binary fields.
		//case PTI_DISCID:	// The specification is ambiguous and states that this type contains binary and character data.
		//case PTI_GENREID:	// The specification is ambiguous and states that this type contains binary and character data.
		case PTI_TOCINFO:
		case PTI_TOCINFO2:
		case PTI_SIZEINFO:
			MessageBox(NULL,_T(""),_T(""),MB_OK);
			return 12;

		default:
			/*char szTemp[255];
			memcpy(szTemp,m_szBuffer,uiEOS + 1);
			MessageBoxA(NULL,szTemp,"default",MB_OK);*/
			break;
	};

	// Skip all null characters.
	while (m_szBuffer[uiEOS] == '\0')
		uiEOS++;

	m_uiBufferPos -= uiEOS;

	memcpy(m_szBuffer,m_szBuffer + uiEOS,m_uiBufferPos);

	return 12;
}

bool CCDText::ReadFile(const TCHAR *szFileName)
{
	// Clear any previous data.
	Reset();

	HANDLE hFile = fs_open(szFileName,_T("rb"));
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// I think it's safe to assume that the file is not larger than 4GB.
	unsigned int uiDataSize = (unsigned int)fs_filesize(hFile);

	// Try to find a signature.
	unsigned long ulSignature = 0;
	fs_read(&ulSignature,4,hFile);

	if (ulSignature == CDTEXT_SIGNATURE)
		uiDataSize -= 4;
	else
		fs_seek(hFile,0,FILE_BEGIN);

	unsigned int uiNumPackets = uiDataSize/18;

	for (unsigned int i = 0; i < uiNumPackets; i++)
	{
		// Read header.
		unsigned long ulHeader = 0;
		fs_read(&ulHeader,sizeof(unsigned long),hFile);

		// The first three bytes of the header is the Pack Type Indicator (PID),
		// the last byte contain block information.
		ReadPacket(hFile,ulHeader & 0xFFFFFF,(ulHeader & 0xFF000000) >> 24);

		// Skip the CRC-field.
		fs_seek(hFile,2,FILE_CURRENT);

		// CRC check. Not currently used.
		/*unsigned char ucCRC[2];
		fs_read(ucCRC,2,hFile);
		unsigned long ulReadCRC = (ucCRC[0] & 0xFF) << 8 | (ucCRC[1] & 0xFF);
		ulReadCRC ^= 0xFFFF;

		TCHAR szTemp2[25];
		swprintf(szTemp2,_T("0x%x"),ulReadCRC);
		MessageBox(NULL,szTemp2,_T("Read CRC"),MB_OK);*/
	}

	// Display some information.
	//MessageBoxA(NULL,m_szAlbumName,"Album Name",MB_OK);
	//MessageBoxA(NULL,m_szArtistName,"Artist Name",MB_OK);

	/*for (unsigned int i = 0; i < m_TrackNames.size(); i++)
	{
		MessageBoxA(NULL,m_TrackNames[i].c_str(),"Track Name",MB_OK);
	}

	for (unsigned int i = 0; i < m_ArtistNames.size(); i++)
	{
		MessageBoxA(NULL,m_ArtistNames[i].c_str(),"Artist Name",MB_OK);
	}*/

	fs_close(hFile);
	return true;
}

unsigned int CCDText::WriteText(HANDLE hFile,unsigned char ucType,unsigned char ucPID2,
								const char *szText,unsigned int uiCharPos)
{
	unsigned char ucBuffer[18];
	ucBuffer[0] = ucType;
	ucBuffer[1] = ucPID2;

	unsigned int uiCurrentPos = 0;
	unsigned int uiCurrentLen = (unsigned int)strlen(szText) + 1;		// We want to include the terminating null character.

	// Copy any old data to the write-buffer.
	if (m_uiBufferPos > 0)
	{
		ucBuffer[1] = m_uiPrevPID2;
		memcpy(ucBuffer + 4,m_szBuffer,m_uiBufferPos);
	}

	// Write all full texts.
	while (uiCurrentPos < uiCurrentLen)
	{
		int iByteCount = min(12 - m_uiBufferPos,uiCurrentLen - uiCurrentPos - m_uiBufferPos);

		memcpy(ucBuffer + 4 + m_uiBufferPos,szText + uiCurrentPos,iByteCount);

		// Flush.
		if (iByteCount + m_uiBufferPos == 12)
		{
			ucBuffer[2] = m_uiBlockCount++;
			ucBuffer[3] = uiCharPos + uiCurrentPos;

			// CRC.
			unsigned short usCRC = CalcCRC(ucBuffer,16);
			usCRC ^= 0xFFFF;
			ucBuffer[16] = (usCRC & 0xFF00) >> 8;
			ucBuffer[17] = (usCRC & 0xFF);

			// Write.
			fs_write(ucBuffer,18,hFile);

			m_uiBufferPos = 0;

			uiCurrentPos += iByteCount;

			// If we have just written a block associated with the previous we can now
			// safeley set the second PID to match the current string.
			if (ucBuffer[1] == m_uiPrevPID2)
			{
				ucBuffer[1] = ucPID2;

				uiCharPos = 0;
			}
		}
		else
		{
			memcpy(m_szBuffer + m_uiBufferPos,szText + uiCurrentPos,iByteCount);
			m_uiBufferPos += iByteCount;

			break;
		}
	}

	return uiCurrentPos;
}

void CCDText::FlushText(HANDLE hFile,unsigned char ucType,unsigned int uiCharPos)
{
	// Is there anything to flush?
	if (m_uiBufferPos > 0)
	{
		unsigned char ucBuffer[18];
		ucBuffer[0] = ucType;

		ucBuffer[1] = m_uiPrevPID2;
		ucBuffer[2] = m_uiBlockCount++;
		ucBuffer[3] = uiCharPos;

		memcpy(ucBuffer + 4,m_szBuffer,m_uiBufferPos);
		memset(ucBuffer + 4 + m_uiBufferPos,'\0',12 - m_uiBufferPos);

		// CRC.
		unsigned short usCRC = CalcCRC(ucBuffer,16);
		usCRC ^= 0xFFFF;
		ucBuffer[16] = (usCRC & 0xFF00) >> 8;
		ucBuffer[17] = (usCRC & 0xFF);

		// Flush.
		fs_write(ucBuffer,18,hFile);

		m_uiBufferPos = 0;
	}
}

bool CCDText::WriteFile(const TCHAR *szFileName)
{
	HANDLE hFile = fs_open(szFileName,_T("wb"));
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

#ifdef CDTEXT_SAVESIGNATURE
	// Write a signature.
	unsigned long ulSignature = CDTEXT_SIGNATURE;
	fs_write(&ulSignature,4,hFile);
#endif

	// Reset the internal counters.
	m_uiBufferPos = 0;
	m_uiBlockCount = 0;

	unsigned int uiCharPos = 0;

	// Track title information.
	if (m_szAlbumName[0] != '\0')
	{
		uiCharPos = WriteText(hFile,PTI_NAMETITLE,0,m_szAlbumName,uiCharPos);
		m_uiPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < m_TrackNames.size(); i++)
	{
		uiCharPos = WriteText(hFile,PTI_NAMETITLE,(unsigned char)i + 1,m_TrackNames[i].c_str(),uiCharPos);
		m_uiPrevPID2 = i + 1;
	}

	FlushText(hFile,PTI_NAMETITLE,uiCharPos);
	uiCharPos = 0;

	// Track artist information.
	if (m_szArtistName[0] != '\0')
	{
		uiCharPos = WriteText(hFile,PTI_NAMEPERFORMER,0,m_szArtistName,uiCharPos);
		m_uiPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < m_ArtistNames.size(); i++)
	{
		uiCharPos = WriteText(hFile,PTI_NAMEPERFORMER,(unsigned char)i + 1,m_ArtistNames[i].c_str(),uiCharPos);
		m_uiPrevPID2 = i + 1;
	}

	FlushText(hFile,PTI_NAMEPERFORMER,uiCharPos);
	uiCharPos = 0;

	// Done.
	fs_close(hFile);
	return true;
}

bool CCDText::WriteFileEx(const TCHAR *szFileName,const TCHAR *szAlbumName,
						  const TCHAR *szArtistName,std::vector<CItemData *> &Tracks)
{
	HANDLE hFile = fs_open(szFileName,_T("wb"));
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

#ifdef CDTEXT_SAVESIGNATURE
	// Write a signature.
	unsigned long ulSignature = CDTEXT_SIGNATURE;
	fs_write(&ulSignature,4,hFile);
#endif

	// Reset the internal counters.
	m_uiBufferPos = 0;
	m_uiBlockCount = 0;

	unsigned int uiCharPos = 0;
	char szBuffer[CDTEXT_MAXFIELDSIZE];

	// Track title information.
	if (szAlbumName[0] != '\0')
	{
		TCharToChar(szAlbumName,szBuffer);

		uiCharPos = WriteText(hFile,PTI_NAMETITLE,0,szBuffer,uiCharPos);
		m_uiPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < Tracks.size(); i++)
	{
		TCharToChar(Tracks[i]->szTrackTitle,szBuffer);

		uiCharPos = WriteText(hFile,PTI_NAMETITLE,(unsigned char)i + 1,szBuffer,uiCharPos);
		m_uiPrevPID2 = i + 1;
	}

	FlushText(hFile,PTI_NAMETITLE,uiCharPos);
	uiCharPos = 0;

	// Track artist information.
	if (szArtistName[0] != '\0')
	{
		TCharToChar(szArtistName,szBuffer);

		uiCharPos = WriteText(hFile,PTI_NAMEPERFORMER,0,szBuffer,uiCharPos);
		m_uiPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < Tracks.size(); i++)
	{
		TCharToChar(Tracks[i]->szTrackArtist,szBuffer);

		uiCharPos = WriteText(hFile,PTI_NAMEPERFORMER,(unsigned char)i + 1,szBuffer,uiCharPos);
		m_uiPrevPID2 = i + 1;
	}

	FlushText(hFile,PTI_NAMEPERFORMER,uiCharPos);
	uiCharPos = 0;

	// Done.
	fs_close(hFile);
	return true;
}
