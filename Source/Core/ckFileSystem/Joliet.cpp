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
#include "../../Common/StringUtil.h"
#include "Joliet.h"

namespace ckFileSystem
{
	CJoliet::CJoliet()
	{
		m_bIncFileVerInfo = true;	// Include ";1" file version information.
		m_iMaxNameLen = 64;			// According to Joliet specification.

		InitVolDesc();
	}

	CJoliet::~CJoliet()
	{
	}

	/*
		Guaraties that the returned character is allowed by the Joliet file system.
	*/
	wchar_t CJoliet::MakeChar(wchar_t c)
	{
		if (c == '*' || c == '/' || c == ':' || c == ';' || c == '?' || c == '\\')
			return '_';

		return c;
	}

	/*
		Copies the source string to the target buffer assuring that all characters
		in the source string are allowed by the Joliet file system. iLength should
		be the length of the source string in bytes.
	*/
	void CJoliet::MemStrCopy(unsigned char *szTarget,const wchar_t *szSource,size_t iLength)
	{
		size_t iSourcePos = 0;

		for (size_t i = 0; i < iLength; i += 2)
		{
			wchar_t cSafe = MakeChar(szSource[iSourcePos++]);

			szTarget[i    ] = cSafe >> 8;
			szTarget[i + 1] = cSafe & 0xFF;
		}
	}

	void CJoliet::EmptyStrBuffer(unsigned char *szBuffer,size_t iBufferLen)
	{
		for (size_t i = 0; i < iBufferLen; i += 2)
		{
			szBuffer[0] = 0x00;
			szBuffer[1] = 0x20;
		}
	}

	void CJoliet::InitVolDesc()
	{
		// Clear memory.
		memset(&m_VolDescSuppl,0,sizeof(m_VolDescSuppl));
		EmptyStrBuffer(m_VolDescSuppl.ucSysIdentifier,sizeof(m_VolDescSuppl.ucSysIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucVolIdentifier,sizeof(m_VolDescSuppl.ucVolIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucVolSetIdentifier,sizeof(m_VolDescSuppl.ucVolSetIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucPublIdentifier,sizeof(m_VolDescSuppl.ucPublIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucPrepIdentifier,sizeof(m_VolDescSuppl.ucPrepIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucAppIdentifier,sizeof(m_VolDescSuppl.ucAppIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucCopyFileIdentifier,sizeof(m_VolDescSuppl.ucCopyFileIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucAbstFileIdentifier,sizeof(m_VolDescSuppl.ucAbstFileIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucBiblFileIdentifier,sizeof(m_VolDescSuppl.ucBiblFileIdentifier));

		// Set primary volume descriptor header.
		m_VolDescSuppl.ucType = VOLDESCTYPE_SUPPL_VOL_DESC;
		m_VolDescSuppl.ucVersion = 1;
		m_VolDescSuppl.ucFileStructVer = 1;
		memcpy(m_VolDescSuppl.ucIdentifier,g_IdentCD,sizeof(m_VolDescSuppl.ucIdentifier));	

		// Always use Joliet level 3.
		m_VolDescSuppl.ucEscapeSeq[0] = 0x25;
		m_VolDescSuppl.ucEscapeSeq[1] = 0x2F;
		m_VolDescSuppl.ucEscapeSeq[2] = 0x45;

		// Set the root directory record.
		m_VolDescSuppl.RootDirRecord.ucDirRecordLen = 34;
		m_VolDescSuppl.RootDirRecord.ucFileFlags = DIRRECORD_FILEFLAG_DIRECTORY;
		m_VolDescSuppl.RootDirRecord.ucFileIdentifierLen = 1;	// One byte is always allocated in the tDirRecord structure.

		// Set application identifier.
		memset(m_VolDescSuppl.ucAppData,0x20,sizeof(m_VolDescSuppl.ucAppData));
		char szAppIdentifier[] = { 0x00,0x49,0x00,0x6E,0x00,0x66,0x00,0x72,0x00,0x61,
			0x00,0x52,0x00,0x65,0x00,0x63,0x00,0x6F,0x00,0x72,0x00,0x64,0x00,0x65,0x00,0x72,
			0x00,0x20,0x00,0x28,0x00,0x43,0x00,0x29,0x00,0x20,0x00,0x32,0x00,0x30,0x00,0x30,
			0x00,0x36,0x00,0x2D,0x00,0x32,0x00,0x30,0x00,0x30,0x00,0x38,0x00,0x20,0x00,0x43,
			0x00,0x68,0x00,0x72,0x00,0x69,0x00,0x73,0x00,0x74,0x00,0x69,0x00,0x61,0x00,0x6E,
			0x00,0x20,0x00,0x4B,0x00,0x69,0x00,0x6E,0x00,0x64,0x00,0x61,0x00,0x68,0x00,0x6C };
		memcpy(m_VolDescSuppl.ucAppIdentifier,szAppIdentifier,90);
	}

	bool CJoliet::WriteVolDesc(COutStream *pOutStream,SYSTEMTIME &stImageCreate,
		unsigned long ulVolSpaceSize,unsigned long ulPathTableSize,unsigned long ulPosPathTableL,
		unsigned long ulPosPathTableM,unsigned long ulRootExtentLoc,unsigned long ulDataLen)
	{
		// Initialize the supplementary volume descriptor.
		Write733(m_VolDescSuppl.ucVolSpaceSize,ulVolSpaceSize);		// Volume size in sectors.
		Write723(m_VolDescSuppl.ucVolSetSize,1);		// Only one disc in the volume set.
		Write723(m_VolDescSuppl.ucVolSeqNumber,1);		// This is the first disc in the volume set.
		Write723(m_VolDescSuppl.ucLogicalBlockSize,ISO9660_SECTOR_SIZE);
		Write733(m_VolDescSuppl.ucPathTableSize,ulPathTableSize);	// Path table size in bytes.
		Write731(m_VolDescSuppl.ucPathTableTypeL,ulPosPathTableL);	// Start sector of LSBF path table.
		Write732(m_VolDescSuppl.ucPathTableTypeM,ulPosPathTableM);	// Start sector of MSBF path table.

		Write733(m_VolDescSuppl.RootDirRecord.ucExtentLocation,ulRootExtentLoc);
		Write733(m_VolDescSuppl.RootDirRecord.ucDataLen,ulDataLen);
		Write723(m_VolDescSuppl.RootDirRecord.ucVolSeqNumber,1);	// The file extent is on the first volume set.

		// Time information.
		MakeDateTime(stImageCreate,m_VolDescSuppl.RootDirRecord.RecDateTime);

		MakeDateTime(stImageCreate,m_VolDescSuppl.CreateDateTime);
		memcpy(&m_VolDescSuppl.ModDateTime,&m_VolDescSuppl.CreateDateTime,sizeof(tVolDescDateTime));

		memset(&m_VolDescSuppl.ExpDateTime,'0',sizeof(tVolDescDateTime));
		m_VolDescSuppl.ExpDateTime.ucZone = 0x00;
		memset(&m_VolDescSuppl.EffectiveDateTime,'0',sizeof(tVolDescDateTime));
		m_VolDescSuppl.EffectiveDateTime.ucZone = 0x00;

		// Write the supplementary volume descriptor.
		unsigned long ulProcessedSize;
		if (pOutStream->Write(&m_VolDescSuppl,sizeof(m_VolDescSuppl),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(m_VolDescSuppl))
			return false;

		return true;
	}

	void CJoliet::SetVolumeLabel(const TCHAR *szLabel)
	{
		size_t iLabelLen = lstrlen(szLabel);
		size_t iLabelCopyLen = iLabelLen < 16 ? iLabelLen : 16;

		EmptyStrBuffer(m_VolDescSuppl.ucVolIdentifier,sizeof(m_VolDescSuppl.ucVolIdentifier));

	#ifdef UNICODE
		MemStrCopy(m_VolDescSuppl.ucVolIdentifier,szLabel,iLabelCopyLen * sizeof(TCHAR));
	#else
		wchar_t szWideLabel[17];
		AnsiToUnicode(szWideLabel,szLabel,sizeof(szWideLabel) / sizeof(wchar_t));
		MemStrCopy(m_VolDescSuppl.ucVolIdentifier,szWideLabel,iLabelCopyLen * sizeof(wchar_t));
	#endif
	}

	void CJoliet::SetTextFields(const TCHAR *szSystem,const TCHAR *szVolSetIdent,
										 const TCHAR *szPublIdent,const TCHAR *szPrepIdent)
	{
		size_t iSystemLen = lstrlen(szSystem);
		size_t iVolSetIdentLen = lstrlen(szVolSetIdent);
		size_t iPublIdentLen = lstrlen(szPublIdent);
		size_t iPrepIdentLen = lstrlen(szPrepIdent);

		size_t iSystemCopyLen = iSystemLen < 16 ? iSystemLen : 16;
		size_t iVolSetIdentCopyLen = iVolSetIdentLen < 64 ? iVolSetIdentLen : 64;
		size_t iPublIdentCopyLen = iPublIdentLen < 64 ? iPublIdentLen : 64;
		size_t iPrepIdentCopyLen = iPrepIdentLen < 64 ? iPrepIdentLen : 64;

		EmptyStrBuffer(m_VolDescSuppl.ucSysIdentifier,sizeof(m_VolDescSuppl.ucSysIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucVolSetIdentifier,sizeof(m_VolDescSuppl.ucVolSetIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucPublIdentifier,sizeof(m_VolDescSuppl.ucPublIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucPrepIdentifier,sizeof(m_VolDescSuppl.ucPrepIdentifier));

	#ifdef UNICODE
		MemStrCopy(m_VolDescSuppl.ucSysIdentifier,szSystem,iSystemCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucVolSetIdentifier,szVolSetIdent,iVolSetIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucPublIdentifier,szPublIdent,iPublIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucPrepIdentifier,szPrepIdent,iPrepIdentCopyLen * sizeof(TCHAR));
		
	#else
		wchar_t szWideSystem[17];
		wchar_t szWideVolSetIdent[65];
		wchar_t szWidePublIdent[65];
		wchar_t szWidePrepIdent[65];

		AnsiToUnicode(szWideSystem,szSystem,sizeof(szWideSystem) / sizeof(wchar_t));
		AnsiToUnicode(szWideVolSetIdent,szVolSetIdent,sizeof(szWideVolSetIdent) / sizeof(wchar_t));
		AnsiToUnicode(szWidePublIdent,szPublIdent,sizeof(szWidePublIdent) / sizeof(wchar_t));
		AnsiToUnicode(szWidePrepIdent,szPrepIdent,sizeof(szWidePrepIdent) / sizeof(wchar_t));

		MemStrCopy(m_VolDescSuppl.ucSysIdentifier,szWideSystem,iSystemCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucVolSetIdentifier,szWideVolSetIdent,iVolSetIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucPublIdentifier,szWidePublIdent,iPublIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucPrepIdentifier,szWidePrepIdent,iPrepIdentCopyLen * sizeof(TCHAR));
	#endif
	}

	void CJoliet::SetFileFields(const TCHAR *szCopyFileIdent,
										 const TCHAR *szAbstFileIdent,
										 const TCHAR *szBiblFileIdent)
	{
		size_t iCopyFileIdentLen = lstrlen(szCopyFileIdent);
		size_t iAbstFileIdentLen = lstrlen(szAbstFileIdent);
		size_t iBiblFileIdentLen = lstrlen(szBiblFileIdent);

		size_t iCopyFileIdentCopyLen = iCopyFileIdentLen < 18 ? iCopyFileIdentLen : 18;
		size_t iAbstFileIdentCopyLen = iAbstFileIdentLen < 18 ? iAbstFileIdentLen : 18;
		size_t iBiblFileIdentCopyLen = iBiblFileIdentLen < 18 ? iBiblFileIdentLen : 18;

		EmptyStrBuffer(m_VolDescSuppl.ucCopyFileIdentifier,sizeof(m_VolDescSuppl.ucCopyFileIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucAbstFileIdentifier,sizeof(m_VolDescSuppl.ucAbstFileIdentifier));
		EmptyStrBuffer(m_VolDescSuppl.ucBiblFileIdentifier,sizeof(m_VolDescSuppl.ucBiblFileIdentifier));

	#ifdef UNICODE
		MemStrCopy(m_VolDescSuppl.ucCopyFileIdentifier,szCopyFileIdent,iCopyFileIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucAbstFileIdentifier,szAbstFileIdent,iAbstFileIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucBiblFileIdentifier,szBiblFileIdent,iBiblFileIdentCopyLen * sizeof(TCHAR));
	#else
		wchar_t szWideCopyFileIdent[19];
		wchar_t szWideAbstFileIdent[19];
		wchar_t szWideBiblFileIdent[19];

		AnsiToUnicode(szWideCopyFileIdent,szCopyFileIdent,sizeof(szWideCopyFileIdent) / sizeof(wchar_t));
		AnsiToUnicode(szWideAbstFileIdent,szAbstFileIdent,sizeof(szWideAbstFileIdent) / sizeof(wchar_t));
		AnsiToUnicode(szWideBiblFileIdent,szBiblFileIdent,sizeof(szWideBiblFileIdent) / sizeof(wchar_t));

		MemStrCopy(m_VolDescSuppl.ucCopyFileIdentifier,szWideCopyFileIdent,iCopyFileIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucAbstFileIdentifier,szWideAbstFileIdent,iAbstFileIdentCopyLen * sizeof(TCHAR));
		MemStrCopy(m_VolDescSuppl.ucBiblFileIdentifier,szWideBiblFileIdent,iBiblFileIdentCopyLen * sizeof(TCHAR));
	#endif
	}

	void CJoliet::SetIncludeFileVerInfo(bool bIncludeInfo)
	{
		m_bIncFileVerInfo = bIncludeInfo;
	}

	void CJoliet::SetRelaxMaxNameLen(bool bRelaxRestriction)
	{
		if (bRelaxRestriction)
			m_iMaxNameLen = JOLIET_MAX_NAMELEN_RELAXED;
		else
			m_iMaxNameLen = JOLIET_MAX_NAMELEN_NORMAL;
	}

	unsigned char CJoliet::WriteFileName(unsigned char *pOutBuffer,const TCHAR *szFileName,bool bIsDir)
	{
#ifndef UNICODE
		wchar_t szWideFileName[JOLIET_MAX_NAMELEN_RELAXED + 1];
		AnsiToUnicode(szWideFileName,szFileName,sizeof(szWideFileName) / sizeof(wchar_t);
#endif

		int iFileNameLen = (int)lstrlen(szFileName),iMax = 0;

		if (iFileNameLen > m_iMaxNameLen)
		{
			int iExtDelimiter = LastDelimiter(szFileName,'.');
			if (iExtDelimiter != -1)
			{
				int iExtLen = (int)iFileNameLen - iExtDelimiter - 1;
				if (iExtLen > m_iMaxNameLen - 1)	// The file can at most contain an extension of length m_iMaxNameLen - 1 characters.
					iExtLen = m_iMaxNameLen - 1;

				// Copy the file name.
				iMax = iExtDelimiter < (m_iMaxNameLen - iExtLen) ? iExtDelimiter : (m_iMaxNameLen - 1 - iExtLen);
#ifdef UNICODE
				MemStrCopy(pOutBuffer,szFileName,iMax * sizeof(TCHAR));
#else
				MemStrCopy(pOutBuffer,szWideFileName,iMax * sizeof(TCHAR));
#endif

				int iOutPos = iMax << 1;
				pOutBuffer[iOutPos++] = 0x00;
				pOutBuffer[iOutPos++] = '.';

				// Copy the extension.
				MemStrCopy(pOutBuffer + iOutPos,szFileName + iExtDelimiter + 1,iExtLen * sizeof(TCHAR));

				iMax = m_iMaxNameLen;
			}
			else
			{
				iMax = m_iMaxNameLen;

#ifdef UNICODE
				MemStrCopy(pOutBuffer,szFileName,iMax * sizeof(TCHAR));
#else
				MemStrCopy(pOutBuffer,szWideFileName,iMax * sizeof(TCHAR));
#endif
			}
		}
		else
		{
			iMax = iFileNameLen;

#ifdef UNICODE
				MemStrCopy(pOutBuffer,szFileName,iMax * sizeof(TCHAR));
#else
				MemStrCopy(pOutBuffer,szWideFileName,iMax * sizeof(TCHAR));
#endif
		}

		if (!bIsDir && m_bIncFileVerInfo)
		{
			int iOutPos = iMax << 1;
			pOutBuffer[iOutPos + 0] = 0x00;
			pOutBuffer[iOutPos + 1] = ';';
			pOutBuffer[iOutPos + 2] = 0x00;
			pOutBuffer[iOutPos + 3] = '1';

			iMax += 2;
		}

		return iMax;
	}

	unsigned char CJoliet::CalcFileNameLen(const TCHAR *szFileName,bool bIsDir)
	{
		/*size_t iNameLen = lstrlen(szFileName);
		if (iNameLen < m_iMaxNameLen)
			return (unsigned char)iNameLen;

		return m_iMaxNameLen;*/

		size_t iNameLen = lstrlen(szFileName);
		if (iNameLen >= m_iMaxNameLen)
			iNameLen = m_iMaxNameLen;

		if (!bIsDir && m_bIncFileVerInfo)
			iNameLen += 2;

		return (unsigned char)iNameLen;
	}
};
