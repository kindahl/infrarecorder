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
#include <math.h>
#include <base/string_util.hh>
#include "cd_text.hh"

void CCdText::InitCRC()
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

unsigned short CCdText::CalcCRC(unsigned char *pBuffer,unsigned int uiLength)
{
	unsigned short usCRC = 0;

	for (int i = 0; i < (int)uiLength; i++)
		usCRC = (usCRC << 8) ^ m_usCRCTable[(usCRC >> 8) ^ pBuffer[i]];
 
	return usCRC;
}

CCdText::CCdText()
{
	Reset();

	// Initialize the CRC table.
	InitCRC();
}

CCdText::~CCdText()
{
	m_TrackNames.clear();
	m_ArtistNames.clear();
}

void CCdText::Reset()
{
	m_szAlbumName[0] = '\0';
	m_szArtistName[0] = '\0';

	m_uiBufferPos = 0;
	m_szBuffer[0] = '\0';

	m_TrackNames.clear();
	m_ArtistNames.clear();
}

unsigned int CCdText::FindBufferEOS(unsigned int uiStart,unsigned int uiEnd)
{
	for (unsigned int i = uiStart; i < uiEnd; i++)
	{
		if (m_szBuffer[i] == '\0')
			return i;
	}

	return 0;
}

unsigned int CCdText::ReadPacket(ckcore::File &File,unsigned long ulPID,
								 unsigned long ulBlockInfo)
{
	// If the buffer can't hold the data we abort.
	if (m_uiBufferPos + 12 >= CDTEXT_MAXFIELDSIZE)
		return 0;

	// Not currently used.
	/*unsigned char ucDBCC = (unsigned char)(ulBlockInfo & 0x80) >> 7;
	unsigned char ucBlockNumber = (unsigned char)(ulBlockInfo & 0x70) >> 4;
	unsigned char ucCharPos = (unsigned char)(ulBlockInfo & 0x0F);*/

	if (File.read(m_szBuffer + m_uiBufferPos,12) == -1)
		return false;

	// Debug information.
	char szTemp[25];
	sprintf(szTemp,"%d, %d, %d",(int)(ulPID & 0xFF0000) >> 16,(int)(ulPID & 0xFF00) >> 8,(int)ulPID & 0xFF);

	char szTemp2[13];
	memcpy(szTemp2,m_szBuffer + m_uiBufferPos,12);
	szTemp2[12] = '\0';

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

bool CCdText::ReadFile(const TCHAR *szFileName)
{
	// Clear any previous data.
	Reset();

	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_READ))
		return false;

	// Make sure that the file is not too large.
	ckcore::tint64 iFileSize = File.size();
	if (iFileSize > 0xFFFFFFFF)
		return false;

	unsigned int uiDataSize = (unsigned int)File.size();

	// Try to find a signature.
	unsigned long ulSignature = 0;
	if (File.read(&ulSignature,4) == -1)
		return false;

	if (ulSignature == CDTEXT_SIGNATURE)
		uiDataSize -= 4;
	else
		File.seek(0,ckcore::File::ckFILE_BEGIN);

	unsigned int uiNumPackets = uiDataSize/18;

	for (unsigned int i = 0; i < uiNumPackets; i++)
	{
		// Read header.
		unsigned long ulHeader = 0;
		if (File.read(&ulHeader,sizeof(unsigned long)) == -1)
			return false;

		// The first three bytes of the header is the Pack Type Indicator (PID),
		// the last byte contain block information.
		ReadPacket(File,ulHeader & 0xFFFFFF,(ulHeader & 0xFF000000) >> 24);

		// Skip the CRC-field.
		if (File.seek(2,ckcore::File::ckFILE_CURRENT) == -1)
			return false;
	}

	return true;
}

unsigned int CCdText::WriteText(ckcore::File &File,unsigned char ucType,unsigned char ucPID2,
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
		ucBuffer[1] = m_ucPrevPID2;
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
			ucBuffer[2] = m_ucBlockCount++;
			ucBuffer[3] = uiCharPos + uiCurrentPos;

			// CRC.
			unsigned short usCRC = CalcCRC(ucBuffer,16);
			usCRC ^= 0xFFFF;
			ucBuffer[16] = static_cast<unsigned char>((usCRC & 0xFF00) >> 8);
			ucBuffer[17] = static_cast<unsigned char>(usCRC & 0xFF);

			// Write.
			File.write(ucBuffer,18);

			m_uiBufferPos = 0;

			uiCurrentPos += iByteCount;

			// If we have just written a block associated with the previous we can now
			// safeley set the second PID to match the current string.
			if (ucBuffer[1] == m_ucPrevPID2)
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

void CCdText::FlushText(ckcore::File &File,unsigned char ucType,unsigned int uiCharPos)
{
	// Is there anything to flush?
	if (m_uiBufferPos > 0)
	{
		unsigned char ucBuffer[18];
		ucBuffer[0] = ucType;

		ucBuffer[1] = m_ucPrevPID2;
		ucBuffer[2] = m_ucBlockCount++;
		ucBuffer[3] = uiCharPos;

		memcpy(ucBuffer + 4,m_szBuffer,m_uiBufferPos);
		memset(ucBuffer + 4 + m_uiBufferPos,'\0',12 - m_uiBufferPos);

		// CRC.
		unsigned short usCRC = CalcCRC(ucBuffer,16);
		usCRC ^= 0xFFFF;
		ucBuffer[16] = static_cast<unsigned char>((usCRC & 0xFF00) >> 8);
		ucBuffer[17] = static_cast<unsigned char>(usCRC & 0xFF);

		// Flush.
		File.write(ucBuffer,18);

		m_uiBufferPos = 0;
	}
}

bool CCdText::WriteFile(const TCHAR *szFileName)
{
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_WRITE))
		return false;

#ifdef CDTEXT_SAVESIGNATURE
	// Write a signature.
	unsigned long ulSignature = CDTEXT_SIGNATURE;
	fs_write(&ulSignature,4,hFile);
#endif

	// Reset the internal counters.
	m_uiBufferPos = 0;
	m_ucBlockCount = 0;

	unsigned int uiCharPos = 0;

	// Track title information.
	if (m_szAlbumName[0] != '\0')
	{
		uiCharPos = WriteText(File,PTI_NAMETITLE,0,m_szAlbumName,uiCharPos);
		m_ucPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < m_TrackNames.size(); i++)
	{
		uiCharPos = WriteText(File,PTI_NAMETITLE,(unsigned char)i + 1,m_TrackNames[i].c_str(),uiCharPos);
		m_ucPrevPID2 = i + 1;
	}

	FlushText(File,PTI_NAMETITLE,uiCharPos);
	uiCharPos = 0;

	// Track artist information.
	if (m_szArtistName[0] != '\0')
	{
		uiCharPos = WriteText(File,PTI_NAMEPERFORMER,0,m_szArtistName,uiCharPos);
		m_ucPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < m_ArtistNames.size(); i++)
	{
		uiCharPos = WriteText(File,PTI_NAMEPERFORMER,(unsigned char)i + 1,m_ArtistNames[i].c_str(),uiCharPos);
		m_ucPrevPID2 = i + 1;
	}

	FlushText(File,PTI_NAMEPERFORMER,uiCharPos);
	uiCharPos = 0;

	return true;
}

bool CCdText::WriteFileEx(const TCHAR *szFileName,const TCHAR *szAlbumName,
						  const TCHAR *szArtistName,std::vector<CItemData *> &Tracks)
{
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_WRITE))
		return false;

#ifdef CDTEXT_SAVESIGNATURE
	// Write a signature.
	unsigned long ulSignature = CDTEXT_SIGNATURE;
	if (File.write(&ulSignature,4) == -1)
		return false;
#endif

	// Reset the internal counters.
	m_uiBufferPos = 0;
	m_ucBlockCount = 0;

	unsigned int uiCharPos = 0;
	char szBuffer[CDTEXT_MAXFIELDSIZE];

	// Track title information.
	if (szAlbumName[0] != '\0')
	{
#ifdef UNICODE
		UnicodeToAnsi(szBuffer,szAlbumName,sizeof(szBuffer));
#else
		strcpy(szBuffer,szAlbumName);
#endif

		uiCharPos = WriteText(File,PTI_NAMETITLE,0,szBuffer,uiCharPos);
		m_ucPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < Tracks.size(); i++)
	{
#ifdef UNICODE
		UnicodeToAnsi(szBuffer,Tracks[i]->GetAudioData()->szTrackTitle,sizeof(szBuffer));
#else
		strcpy(szBuffer,Tracks[i]->GetAudioData()->szTrackTitle);
#endif

		uiCharPos = WriteText(File,PTI_NAMETITLE,(unsigned char)i + 1,szBuffer,uiCharPos);
		m_ucPrevPID2 = i + 1;
	}

	FlushText(File,PTI_NAMETITLE,uiCharPos);
	uiCharPos = 0;

	// Track artist information.
	if (szArtistName[0] != '\0')
	{
#ifdef UNICODE
		UnicodeToAnsi(szBuffer,szArtistName,sizeof(szBuffer));
#else
		strcpy(szBuffer,szArtistName);
#endif

		uiCharPos = WriteText(File,PTI_NAMEPERFORMER,0,szBuffer,uiCharPos);
		m_ucPrevPID2 = 0;
	}

	for (unsigned int i = 0; i < Tracks.size(); i++)
	{
#ifdef UNICODE
		UnicodeToAnsi(szBuffer,Tracks[i]->GetAudioData()->szTrackArtist,sizeof(szBuffer));
#else
		strcpy(szBuffer,Tracks[i]->GetAudioData()->szTrackArtist);
#endif

		uiCharPos = WriteText(File,PTI_NAMEPERFORMER,(unsigned char)i + 1,szBuffer,uiCharPos);
		m_ucPrevPID2 = i + 1;
	}

	FlushText(File,PTI_NAMEPERFORMER,uiCharPos);
	uiCharPos = 0;

	return true;
}
