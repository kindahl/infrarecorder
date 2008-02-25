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
#include <vector>
#include "TreeManager.h"

#define CDTEXT_SIGNATURE			0x2201
#define CDTEXT_MAXFIELDSIZE			160
//#define CDTEXT_CRCPOLYNOMIAL		0x8408
#define CDTEXT_CRCPOLYNOMIAL		0x1021

// Define if a 4 byte header should be stored in the beginning of each cd-text binary file.
//#define CDTEXT_SAVESIGNATURE

class CCDText
{
private:
	unsigned short m_usCRCTable[256];

	char m_szBuffer[CDTEXT_MAXFIELDSIZE];
	unsigned int m_uiBufferPos;

	unsigned int m_uiBlockCount;
	unsigned int m_uiPrevPID2;

	void Reset();
	unsigned int FindBufferEOS(unsigned int uiStart,unsigned int uiEnd);
	unsigned int ReadPacket(HANDLE hFile,unsigned long ulPID,unsigned long ulBlockInfo);

	// CRC routines.
	void InitCRC();
	unsigned short CalcCRC(unsigned char *pBuffer,unsigned int uiLength);

	// Write routines.
	unsigned int WriteText(HANDLE hFile,unsigned char ucType,unsigned char ucPID2,
		const char *szText,unsigned int uiCharPos);
	void FlushText(HANDLE hFile,unsigned char ucType,unsigned int uiCharPos);

	enum ePTI
	{
		PTI_NAMETITLE = 0x80,
		PTI_NAMEPERFORMER = 0x81,
		PTI_NAMEWRITER = 0x82,
		PTI_NAMESONGWRITER = 0x83,
		PTI_NAMEARRANGER = 0x84,
		PTI_MESSAGEARTIST = 0x85,
		PTI_DISCID = 0x86,
		PTI_GENREID = 0x87,
		PTI_TOCINFO = 0x88,
		PTI_TOCINFO2 = 0x89,
		PTI_RESERVED1 = 0x8A,
		PTI_RESERVED2 = 0x8B,
		PTI_RESERVED3 = 0x8C,
		PTI_RESERVED4 = 0x8D,
		PTI_UPCEAN = 0x8E,
		PTI_SIZEINFO = 0x8F
	};

public:
	CCDText();
	~CCDText();

	char m_szAlbumName[CDTEXT_MAXFIELDSIZE];
	char m_szArtistName[CDTEXT_MAXFIELDSIZE];
	std::vector<std::string> m_TrackNames;
	std::vector<std::string> m_ArtistNames;

	bool ReadFile(const TCHAR *szFileName);
	bool WriteFile(const TCHAR *szFileName);
	bool WriteFileEx(const TCHAR *szFileName,const TCHAR *szAlbumName,
		const TCHAR *szArtistName,std::vector<CItemData *> &Tracks);
};
