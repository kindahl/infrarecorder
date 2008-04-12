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
#include "Iso9660Writer.h"
#include "DiscImageHelper.h"

namespace ckFileSystem
{
	CDiscImageHelper::CDiscImageHelper(CDiscImageWriter::eFileSystem FileSystem,
		bool bIncludeInfo,bool bLongJolietNames,CIso9660::eInterLevel InterLevel) :
		m_FileSystem(FileSystem)
	{
		// Joliet.
		m_Joliet.SetIncludeFileVerInfo(bIncludeInfo);
		m_Joliet.SetRelaxMaxNameLen(bLongJolietNames);

		// ISO9660.
		m_Iso9660.SetIncludeFileVerInfo(bIncludeInfo);
		m_Iso9660.SetInterchangeLevel(InterLevel);
	}

	CDiscImageHelper::~CDiscImageHelper()
	{
	}

	// Warning: This function duplicates some functionality in CUdf.
	// szFileName is assumed to be at least as long as szReqFileName.
	void CDiscImageHelper::CalcFileName(const TCHAR *szReqFileName,TCHAR *szFileName,bool bIsDir)
	{
		bool bUseIso = m_FileSystem != CDiscImageWriter::FS_UDF;
		bool bUseUdf = m_FileSystem == CDiscImageWriter::FS_ISO9660_UDF ||
			m_FileSystem == CDiscImageWriter::FS_ISO9660_UDF_JOLIET ||
			m_FileSystem == CDiscImageWriter::FS_UDF ||
			m_FileSystem == CDiscImageWriter::FS_DVDVIDEO;
		bool bUseJoliet = m_FileSystem == CDiscImageWriter::FS_ISO9660_JOLIET ||
			m_FileSystem == CDiscImageWriter::FS_ISO9660_UDF_JOLIET;

		if (bUseUdf)
		{
			size_t iNameLen = lstrlen(szFileName);
			lstrncpy(szFileName,szReqFileName,iNameLen < (254 >> 1) ? iNameLen : (254 >> 1));		// One byte is reserved for compression descriptor.
		}
		else if (bUseJoliet)
		{
			unsigned char ucFileName[ISO9660WRITER_FILENAME_BUFFER_SIZE + 1];
			unsigned char ucLen = m_Joliet.WriteFileName((unsigned char *)ucFileName,szReqFileName,bIsDir);

#ifdef UNICODE
			unsigned char ucFileNamePos = 0;
			for (unsigned char i = 0; i < ucLen; i++)
			{
				szFileName[i]  = ucFileName[ucFileNamePos++] << 8;
				szFileName[i] |= ucFileName[ucFileNamePos++];
			}

			szFileName[ucLen] = '\0';
#else
			wchar_t szWideFileName[ISO9660WRITER_FILENAME_BUFFER_SIZE + 1];
			unsigned char ucFileNamePos = 0;
			for (unsigned char i = 0; i < ucLen; i++)
			{
				szWideFileName[i]  = ucFileName[ucFileNamePos++] << 8;
				szWideFileName[i] |= ucFileName[ucFileNamePos++];
			}

			szWideFileName[ucLen] = '\0';

			UnicodeToAnsi(szFileName,szWideFileName,ucLen);
#endif
		}
		else if (bUseIso)
		{
			//CalcFileNameLen
#ifdef UNICODE
			char szMultiFileName[ISO9660WRITER_FILENAME_BUFFER_SIZE + 1];
			unsigned char ucLen = m_Iso9660.WriteFileName((unsigned char *)szMultiFileName,szReqFileName,bIsDir);
			szMultiFileName[ucLen] = '\0';

			AnsiToUnicode(szFileName,szMultiFileName,ucLen + 1);
#else
			unsigned char ucLen = m_Iso9660.WriteFileName((unsigned char *)szFileName,szReqFileName,bIsDir);
			szFileName[ucLen] = '\0';
#endif
		}
		else
		{
			lstrcpy(szFileName,szReqFileName);
		}
	}

	void CDiscImageHelper::CalcFilePath(const TCHAR *szReqFilePath,tstring &FilePath)
	{
		size_t iDirPathLen = lstrlen(szReqFilePath),iPrevDelim = 0,iPos = 0;
		tstring CurDirName;
		TCHAR szFileNameBuffer[ISO9660WRITER_FILENAME_BUFFER_SIZE + 1];

		// Locate the first delimiter (so we can safely skip any driveletter).
		for (size_t i = 0; i < iDirPathLen; i++)
		{
			if (szReqFilePath[i] == '/' || szReqFilePath[i] == '\\')
			{
				iPos = i;
				break;
			}
		}

		// Make sure that we don't delete any thing before the first delimiter that exists in FilePath.
		if (iPos > 0)
		{
			iPrevDelim = iPos;
			
			if (FilePath.length() > iPos)
				FilePath.erase(iPos);
		}

		for (; iPos < iDirPathLen; iPos++)
		{
			if (szReqFilePath[iPos] == '/' || szReqFilePath[iPos] == '\\')	// I don't like supporting two delimiters.
			{
				if (iPos > (iPrevDelim + 1))
				{
					// Obtain the name of the current directory.
					CurDirName.erase();
					for (size_t j = iPrevDelim + 1; j < iPos; j++)
						CurDirName.push_back(szReqFilePath[j]);

					CalcFileName(CurDirName.c_str(),szFileNameBuffer,true);
					FilePath.append(_T("/"));
					FilePath.append(szFileNameBuffer);
				}

				iPrevDelim = iPos;
			}
		}

		CalcFileName(szReqFilePath + iPrevDelim + 1,szFileNameBuffer,false);

		size_t iFileNameBufferLen = lstrlen(szFileNameBuffer);
		if (szFileNameBuffer[iFileNameBufferLen - 2] = ';')
			szFileNameBuffer[iFileNameBufferLen - 2] = '\0';

		FilePath.append(_T("/"));
		FilePath.append(szFileNameBuffer);
	}
};
