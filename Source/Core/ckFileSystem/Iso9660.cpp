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
#include "Iso9660.h"
#include "../../Common/StringUtil.h"

namespace ckFileSystem
{
	/*
		Identifiers.
	*/
	const char *g_IdentCD = "CD001";
	const char *g_IdentElTorito = "EL TORITO SPECIFICATION";

	/*
		Helper Functions.
	*/
	void Write721(unsigned char *pOut,unsigned short usValue)		// Least significant byte first.
	{
		pOut[0] = usValue & 0xFF;
		pOut[1] = (usValue >> 8) & 0xFF;
	}

	void Write722(unsigned char *pOut,unsigned short usValue)		// Most significant byte first.
	{
		pOut[0] = (usValue >> 8) & 0xFF;
		pOut[1] = usValue & 0xFF;
	}

	void Write723(unsigned char *pOut,unsigned short usValue)		// Both-byte orders.
	{
		pOut[3] = pOut[0] = usValue & 0xFF;
		pOut[2] = pOut[1] = (usValue >> 8) & 0xFF;
	}

	void Write731(unsigned char *pOut,unsigned long ulValue)		// Least significant byte first.
	{
		pOut[0] = (unsigned char)(ulValue & 0xFF);
		pOut[1] = (unsigned char)((ulValue >> 8) & 0xFF);
		pOut[2] = (unsigned char)((ulValue >> 16) & 0xFF);
		pOut[3] = (unsigned char)((ulValue >> 24) & 0xFF);
	}

	void Write732(unsigned char *pOut,unsigned long ulValue)		// Most significant byte first.
	{
		pOut[0] = (unsigned char)((ulValue >> 24) & 0xFF);
		pOut[1] = (unsigned char)((ulValue >> 16) & 0xFF);
		pOut[2] = (unsigned char)((ulValue >> 8) & 0xFF);
		pOut[3] = (unsigned char)(ulValue & 0xFF);
	}

	void Write733(unsigned char *pOut,unsigned long ulValue)		// Both-byte orders.
	{
		pOut[7] = pOut[0] = (unsigned char)(ulValue & 0xFF);
		pOut[6] = pOut[1] = (unsigned char)((ulValue >> 8) & 0xFF);
		pOut[5] = pOut[2] = (unsigned char)((ulValue >> 16) & 0xFF);
		pOut[4] = pOut[3] = (unsigned char)((ulValue >> 24) & 0xFF);
	}

	void Write72(unsigned char *pOut,unsigned short usValue,bool bMSBF)
	{
		if (bMSBF)
			Write722(pOut,usValue);
		else
			Write721(pOut,usValue);
	}

	void Write73(unsigned char *pOut,unsigned long ulValue,bool bMSBF)
	{
		if (bMSBF)
			Write732(pOut,ulValue);
		else
			Write731(pOut,ulValue);
	}

	unsigned short Read721(unsigned char *pOut)		// Least significant byte first.
	{
		return ((unsigned short)pOut[1] << 8) | pOut[0];
	}

	unsigned short Read722(unsigned char *pOut)		// Most significant byte first.
	{
		return ((unsigned short)pOut[0] << 8) | pOut[1];
	}

	unsigned short Read723(unsigned char *pOut)		// Both-byte orders.
	{
		return Read721(pOut);
	}

	unsigned long Read731(unsigned char *pOut)			// Least significant byte first.
	{
		return ((unsigned long)pOut[3] << 24) | ((unsigned long)pOut[2] << 16) |
			((unsigned long)pOut[1] << 8) | pOut[0];
	}

	unsigned long Read732(unsigned char *pOut)			// Most significant byte first.
	{
		return ((unsigned long)pOut[0] << 24) | ((unsigned long)pOut[1] << 16) |
			((unsigned long)pOut[2] << 8) | pOut[3];
	}

	unsigned long Read733(unsigned char *pOut)			// Both-byte orders.
	{
		return Read731(pOut);
	}

	unsigned long BytesToSector(unsigned long ulBytes)
	{
		if (ulBytes == 0)
			return 0;

		unsigned long ulSectors = 1;

		while (ulBytes > ISO9660_SECTOR_SIZE)
		{
			ulBytes -= ISO9660_SECTOR_SIZE;
			ulSectors++;
		}

		return ulSectors;
	}

	unsigned long BytesToSector(unsigned __int64 uiBytes)
	{
		if (uiBytes == 0)
			return 0;

		unsigned long ulSectors = 1;

		while (uiBytes > ISO9660_SECTOR_SIZE)
		{
			uiBytes -= ISO9660_SECTOR_SIZE;
			ulSectors++;
		}

		return ulSectors;
	}

	unsigned __int64 BytesToSector64(unsigned __int64 uiBytes)
	{
		if (uiBytes == 0)
			return 0;

		unsigned __int64 uiSectors = 1;

		while (uiBytes > ISO9660_SECTOR_SIZE)
		{
			uiBytes -= ISO9660_SECTOR_SIZE;
			uiSectors++;
		}

		return uiSectors;
	}

	void MakeDateTime(SYSTEMTIME &st,tVolDescDateTime &DateTime)
	{
		char szBuffer[5];
		sprintf(szBuffer,"%.4u",st.wYear);
		memcpy(&DateTime.uiYear,szBuffer,4);

		sprintf(szBuffer,"%.2u",st.wMonth);
		memcpy(&DateTime.usMonth,szBuffer,2);

		sprintf(szBuffer,"%.2u",st.wDay);
		memcpy(&DateTime.usDay,szBuffer,2);

		sprintf(szBuffer,"%.2u",st.wHour);
		memcpy(&DateTime.usHour,szBuffer,2);

		sprintf(szBuffer,"%.2u",st.wMinute);
		memcpy(&DateTime.usMinute,szBuffer,2);

		sprintf(szBuffer,"%.2u",st.wSecond);
		memcpy(&DateTime.usSecond,szBuffer,2);

		sprintf(szBuffer,"%.2u",st.wMilliseconds * 10);
		memcpy(&DateTime.usHundreds,szBuffer,2);

		TIME_ZONE_INFORMATION tzi;
		GetTimeZoneInformation(&tzi);
		DateTime.ucZone = -(unsigned char)(tzi.Bias/15);
	}

	void MakeDateTime(SYSTEMTIME &st,tDirRecordDateTime &DateTime)
	{
		DateTime.ucYear = (unsigned char)(st.wYear - 1900);
		DateTime.ucMonth = (unsigned char)st.wMonth;
		DateTime.ucDay = (unsigned char)st.wDay;
		DateTime.ucHour = (unsigned char)st.wHour;
		DateTime.ucMinute = (unsigned char)st.wMinute;
		DateTime.ucSecond = (unsigned char)st.wSecond;

		TIME_ZONE_INFORMATION tzi;
		GetTimeZoneInformation(&tzi);
		DateTime.ucZone = -(unsigned char)(tzi.Bias/15);
	}

	void MakeDateTime(unsigned short usDate,unsigned short usTime,tDirRecordDateTime &DateTime)
	{
		DateTime.ucYear = ((usDate >> 9) & 0x7F) + 80;
		DateTime.ucMonth = (usDate >> 5) & 0x0F;
		DateTime.ucDay = usDate & 0x1F;
		DateTime.ucHour = (usTime >> 11) & 0x1F;
		DateTime.ucMinute = (usTime >> 5) & 0x3F;
		DateTime.ucSecond = (usTime & 0x1F) << 1;

		TIME_ZONE_INFORMATION tzi;
		GetTimeZoneInformation(&tzi);
		DateTime.ucZone = -(unsigned char)(tzi.Bias/15);
	}

	CIso9660::CIso9660()
	{
		m_InterLevel = LEVEL_1;
		m_bRelaxMaxDirLevel = false;
		m_bIncFileVerInfo = true;	// Include ";1" file version information.

		InitVolDescPrimary();
		InitVolDescSetTerm();
	}

	CIso9660::~CIso9660()
	{
	}

	/*
		Convert the specified character to an a-character (appendix A).
	*/
	char CIso9660::MakeCharA(char c)
	{
		char cResult = toupper(c);

		// Make sure that it's a valid character, otherwise return '_'.
		if ((cResult >= 0x20 && cResult <= 0x22) ||
			(cResult >= 0x25 && cResult <= 0x39) ||
			(cResult >= 0x41 && cResult <= 0x5A) || cResult == 0x5F)
			return cResult;

		return '_';
	}

	/*
		Convert the specified character to a d-character (appendix A).
	*/
	char CIso9660::MakeCharD(char c)
	{
		char cResult = toupper(c);

		// Make sure that it's a valid character, otherwise return '_'.
		if ((cResult >= 0x30 && cResult <= 0x39) ||
			(cResult >= 0x41 && cResult <= 0x5A) || cResult == 0x5F)
			return cResult;

		return '_';
	}

	/*
		Performs a memory copy from szSource to szTarget, all characters
		in szTarget will be A-characters.
	*/
	void CIso9660::MemStrCopyA(unsigned char *szTarget,const char *szSource,size_t iLength)
	{
		for (size_t i = 0; i < iLength; i++)
			szTarget[i] = MakeCharA(szSource[i]);
	}

	/*
		Performs a memory copy from szSource to szTarget, all characters
		in szTarget will be D-characters.
	*/
	void CIso9660::MemStrCopyD(unsigned char *szTarget,const char *szSource,size_t iLength)
	{
		for (size_t i = 0; i < iLength; i++)
			szTarget[i] = MakeCharD(szSource[i]);
	}

	/*
		Converts the input file name to a valid ISO level 1 file name. This means:
		 - A maximum of 12 characters.
		 - A file extension of at most 3 characters.
		 - A file name of at most 8 characters.
	*/
	unsigned char CIso9660::WriteFileNameL1(unsigned char *pOutBuffer,const TCHAR *szFileName)
	{
		int iFileNameLen = (int)lstrlen(szFileName);
		unsigned char ucLength = 0;

		char *szMultiFileName;
	#ifdef UNICODE
		szMultiFileName = new char [iFileNameLen + 1];
		UnicodeToAnsi(szMultiFileName,szFileName,iFileNameLen + 1);
	#else
		szMultiFileName = szFileName;
	#endif

		int iExtDelimiter = LastDelimiterA(szMultiFileName,'.');
		if (iExtDelimiter == -1)
		{
			size_t iMax = iFileNameLen < 8 ? iFileNameLen : 8;
			for (size_t i = 0; i < iMax; i++)
				pOutBuffer[i] = MakeCharD(szMultiFileName[i]);
			
			ucLength = (unsigned char)iMax;
			pOutBuffer[iMax] = '\0';
		}
		else
		{
			int iExtLen = (int)iFileNameLen - iExtDelimiter - 1;
			if (iExtLen > 3)	// Level one support a maxmimum file extension of length 3.
				iExtLen = 3;

			size_t iMax = iExtDelimiter < 8 ? iExtDelimiter : 8;
			for (size_t i = 0; i < iMax; i++)
				pOutBuffer[i] = MakeCharD(szMultiFileName[i]);

			pOutBuffer[iMax] = '.';

			// Copy the extension.
			for (size_t i = iMax + 1; i < iMax + iExtLen + 1; i++)
				pOutBuffer[i] = MakeCharD(szMultiFileName[++iExtDelimiter]);

			ucLength = (unsigned char)iMax + (unsigned char)iExtLen + 1;
			pOutBuffer[ucLength] = '\0';
		}

	#ifdef UNICODE
		delete [] szMultiFileName;
	#endif

		return ucLength;
	}

	unsigned char CIso9660::WriteFileNameGeneric(unsigned char *pOutBuffer,const TCHAR *szFileName,
												 int iMaxLen)
	{
		int iFileNameLen = (int)lstrlen(szFileName);
		unsigned char ucLength = 0;

	#ifdef UNICODE
		char *szMultiFileName = new char [iFileNameLen + 1];
		UnicodeToAnsi(szMultiFileName,szFileName,iFileNameLen + 1);
	#else
		char *szMultiFileName = szFileName;
	#endif

		int iExtDelimiter = LastDelimiterA(szMultiFileName,'.');
		if (iExtDelimiter == -1)
		{
			size_t iMax = iFileNameLen < iMaxLen ? iFileNameLen : iMaxLen;
			for (size_t i = 0; i < iMax; i++)
				pOutBuffer[i] = MakeCharD(szMultiFileName[i]);
			
			ucLength = (unsigned char)iMax;
			pOutBuffer[iMax] = '\0';
		}
		else
		{
			int iExtLen = (int)iFileNameLen - iExtDelimiter - 1;
			if (iExtLen > iMaxLen - 1)	// The file can at most contain an extension of length iMaxLen - 1 characters.
				iExtLen = iMaxLen - 1;

			size_t iMax = iExtDelimiter < (iMaxLen - iExtLen) ? iExtDelimiter : (iMaxLen - 1 - iExtLen);
			for (size_t i = 0; i < iMax; i++)
				pOutBuffer[i] = MakeCharD(szMultiFileName[i]);

			pOutBuffer[iMax] = '.';

			// Copy the extension.
			for (size_t i = iMax + 1; i < iMax + iExtLen + 1; i++)
				pOutBuffer[i] = MakeCharD(szMultiFileName[++iExtDelimiter]);

			ucLength = (unsigned char)iMax + (unsigned char)iExtLen + 1;
			pOutBuffer[ucLength] = '\0';
		}

	#ifdef UNICODE
		delete [] szMultiFileName;
	#endif

		return ucLength;
	}

	/*
		Converts the input file name to a valid ISO level 2 and above file name. This means:
		 - A maximum of 31 characters.
	*/
	unsigned char CIso9660::WriteFileNameL2(unsigned char *pOutBuffer,const TCHAR *szFileName)
	{
		return WriteFileNameGeneric(pOutBuffer,szFileName,31);
	}

	unsigned char CIso9660::WriteFileName1999(unsigned char *pOutBuffer,const TCHAR *szFileName)
	{
		return WriteFileNameGeneric(pOutBuffer,szFileName,ISO9660_MAX_NAMELEN_1999);
	}

	unsigned char CIso9660::WriteDirNameL1(unsigned char *pOutBuffer,const TCHAR *szDirName)
	{
		int iDirNameLen = (int)lstrlen(szDirName);
		int iMax = iDirNameLen < 8 ? iDirNameLen : 8;

	#ifdef UNICODE
		char *szMultiDirName = new char [iDirNameLen + 1];
		UnicodeToAnsi(szMultiDirName,szDirName,iDirNameLen + 1);
	#else
		char *szMultiDirName = szDirName;
	#endif

		for (size_t i = 0; i < iMax; i++)
			pOutBuffer[i] = MakeCharD(szMultiDirName[i]);
			
		pOutBuffer[iMax] = '\0';

	#ifdef UNICODE
		delete [] szMultiDirName;
	#endif

		return iMax;
	}

	unsigned char CIso9660::WriteDirNameGeneric(unsigned char *pOutBuffer,const TCHAR *szDirName,
												int iMaxLen)
	{
		int iDirNameLen = (int)lstrlen(szDirName);
		int iMax = iDirNameLen < iMaxLen ? iDirNameLen : iMaxLen;

	#ifdef UNICODE
		char *szMultiDirName = new char [iDirNameLen + 1];
		UnicodeToAnsi(szMultiDirName,szDirName,iDirNameLen + 1);
	#else
		char *szMultiDirName = szDirName;
	#endif

		for (size_t i = 0; i < iMax; i++)
			pOutBuffer[i] = MakeCharD(szMultiDirName[i]);
			
		pOutBuffer[iMax] = '\0';

	#ifdef UNICODE
		delete [] szMultiDirName;
	#endif

		return iMax;
	}

	unsigned char CIso9660::WriteDirNameL2(unsigned char *pOutBuffer,const TCHAR *szDirName)
	{
		return WriteDirNameGeneric(pOutBuffer,szDirName,31);
	}

	unsigned char CIso9660::WriteDirName1999(unsigned char *pOutBuffer,const TCHAR *szDirName)
	{
		return WriteDirNameGeneric(pOutBuffer,szDirName,ISO9660_MAX_NAMELEN_1999);
	}

	unsigned char CIso9660::CalcFileNameLenL1(const TCHAR *szFileName)
	{
		unsigned char szTempBuffer[13];
		return WriteFileNameL1(szTempBuffer,szFileName);
	}

	unsigned char CIso9660::CalcFileNameLenL2(const TCHAR *szFileName)
	{
		size_t iFileNameLen = lstrlen(szFileName);
		if (iFileNameLen < 31)
			return (unsigned char)iFileNameLen;

		return 31;
	}

	unsigned char CIso9660::CalcFileNameLen1999(const TCHAR *szFileName)
	{
		size_t iFileNameLen = lstrlen(szFileName);
		if (iFileNameLen < ISO9660_MAX_NAMELEN_1999)
			return (unsigned char)iFileNameLen;

		return ISO9660_MAX_NAMELEN_1999;
	}

	unsigned char CIso9660::CalcDirNameLenL1(const TCHAR *szDirName)
	{
		size_t iDirNameLen = lstrlen(szDirName);
		if (iDirNameLen < 8)
			return (unsigned char)iDirNameLen;

		return 8;
	}

	unsigned char CIso9660::CalcDirNameLenL2(const TCHAR *szDirName)
	{
		size_t iDirNameLen = lstrlen(szDirName);
		if (iDirNameLen < 31)
			return (unsigned char)iDirNameLen;

		return 31;
	}

	unsigned char CIso9660::CalcDirNameLen1999(const TCHAR *szDirName)
	{
		size_t iDirNameLen = lstrlen(szDirName);
		if (iDirNameLen < ISO9660_MAX_NAMELEN_1999)
			return (unsigned char)iDirNameLen;

		return ISO9660_MAX_NAMELEN_1999;
	}

	void CIso9660::InitVolDescPrimary()
	{
		// Clear memory.
		memset(&m_VolDescPrimary,0,sizeof(m_VolDescPrimary));
		memset(m_VolDescPrimary.ucSysIdentifier,0x20,sizeof(m_VolDescPrimary.ucSysIdentifier));
		memset(m_VolDescPrimary.ucVolIdentifier,0x20,sizeof(m_VolDescPrimary.ucVolIdentifier));
		memset(m_VolDescPrimary.ucVolSetIdentifier,0x20,sizeof(m_VolDescPrimary.ucVolSetIdentifier));
		memset(m_VolDescPrimary.ucPublIdentifier,0x20,sizeof(m_VolDescPrimary.ucPublIdentifier));
		memset(m_VolDescPrimary.ucPrepIdentifier,0x20,sizeof(m_VolDescPrimary.ucPrepIdentifier));
		memset(m_VolDescPrimary.ucAppIdentifier,0x20,sizeof(m_VolDescPrimary.ucAppIdentifier));
		memset(m_VolDescPrimary.ucCopyFileIdentifier,0x20,sizeof(m_VolDescPrimary.ucCopyFileIdentifier));
		memset(m_VolDescPrimary.ucAbstFileIdentifier,0x20,sizeof(m_VolDescPrimary.ucAbstFileIdentifier));
		memset(m_VolDescPrimary.ucBiblFileIdentifier,0x20,sizeof(m_VolDescPrimary.ucBiblFileIdentifier));

		// Set primary volume descriptor header.
		m_VolDescPrimary.ucType = VOLDESCTYPE_PRIM_VOL_DESC;
		m_VolDescPrimary.ucVersion = 1;
		m_VolDescPrimary.ucFileStructVer = 1;
		memcpy(m_VolDescPrimary.ucIdentifier,g_IdentCD,sizeof(m_VolDescPrimary.ucIdentifier));	

		// Set the root directory record.
		m_VolDescPrimary.RootDirRecord.ucDirRecordLen = 34;
		m_VolDescPrimary.RootDirRecord.ucFileFlags = DIRRECORD_FILEFLAG_DIRECTORY;
		m_VolDescPrimary.RootDirRecord.ucFileIdentifierLen = 1;	// One byte is always allocated in the tDirRecord structure.

		// Set application identifier.
		memset(m_VolDescPrimary.ucAppData,0x20,sizeof(m_VolDescPrimary.ucAppData));
		char szAppIdentifier[] = { 0x49,0x4E,0x46,0x52,0x41,0x52,0x45,0x43,0x4F,
			0x52,0x44,0x45,0x52,0x20,0x28,0x43,0x29,0x20,0x32,0x30,0x30,0x36,0x2D,
			0x32,0x30,0x30,0x38,0x20,0x43,0x48,0x52,0x49,0x53,0x54,0x49,0x41,0x4E,
			0x20,0x4B,0x49,0x4E,0x44,0x41,0x48,0x4C };
		memcpy(m_VolDescPrimary.ucAppIdentifier,szAppIdentifier,45);
	}

	void CIso9660::InitVolDescSetTerm()
	{
		// Clear memory.
		memset(&m_VolDescSetTerm,0,sizeof(m_VolDescSetTerm));

		// Set volume descriptor set terminator header.
		m_VolDescSetTerm.ucType = VOLDESCTYPE_VOL_DESC_SET_TERM;
		m_VolDescSetTerm.ucVersion = 1;
		memcpy(m_VolDescSetTerm.ucIdentifier,g_IdentCD,sizeof(m_VolDescSetTerm.ucIdentifier));
	}

	void CIso9660::SetVolumeLabel(const TCHAR *szLabel)
	{
		size_t iLabelLen = lstrlen(szLabel);
		size_t iLabelCopyLen = iLabelLen < 32 ? iLabelLen : 32;

		memset(m_VolDescPrimary.ucVolIdentifier,0x20,sizeof(m_VolDescPrimary.ucVolIdentifier));

	#ifdef UNICODE
		char szMultiLabel[33];
		UnicodeToAnsi(szMultiLabel,szLabel,sizeof(szMultiLabel));
		MemStrCopyD(m_VolDescPrimary.ucVolIdentifier,szMultiLabel,iLabelCopyLen);
	#else
		MemStrCopyD(m_VolDescPrimary.ucVolIdentifier,szLabel,iLabelCopyLen);
	#endif
	}

	void CIso9660::SetTextFields(const TCHAR *szSystem,const TCHAR *szVolSetIdent,
								 const TCHAR *szPublIdent,const TCHAR *szPrepIdent)
	{
		size_t iSystemLen = lstrlen(szSystem);
		size_t iVolSetIdentLen = lstrlen(szVolSetIdent);
		size_t iPublIdentLen = lstrlen(szPublIdent);
		size_t iPrepIdentLen = lstrlen(szPrepIdent);

		size_t iSystemCopyLen = iSystemLen < 32 ? iSystemLen : 32;
		size_t iVolSetIdentCopyLen = iVolSetIdentLen < 128 ? iVolSetIdentLen : 128;
		size_t iPublIdentCopyLen = iPublIdentLen < 128 ? iPublIdentLen : 128;
		size_t iPrepIdentCopyLen = iPrepIdentLen < 128 ? iPrepIdentLen : 128;

		memset(m_VolDescPrimary.ucSysIdentifier,0x20,sizeof(m_VolDescPrimary.ucSysIdentifier));
		memset(m_VolDescPrimary.ucVolSetIdentifier,0x20,sizeof(m_VolDescPrimary.ucVolSetIdentifier));
		memset(m_VolDescPrimary.ucPublIdentifier,0x20,sizeof(m_VolDescPrimary.ucPublIdentifier));
		memset(m_VolDescPrimary.ucPrepIdentifier,0x20,sizeof(m_VolDescPrimary.ucPrepIdentifier));

	#ifdef UNICODE
		char szMultiSystem[33];
		char szMultiVolSetIdent[129];
		char szMultiPublIdent[129];
		char szMultiPrepIdent[129];

		UnicodeToAnsi(szMultiSystem,szSystem,sizeof(szMultiSystem));
		UnicodeToAnsi(szMultiVolSetIdent,szVolSetIdent,sizeof(szMultiVolSetIdent));
		UnicodeToAnsi(szMultiPublIdent,szPublIdent,sizeof(szMultiPublIdent));
		UnicodeToAnsi(szMultiPrepIdent,szPrepIdent,sizeof(szMultiPrepIdent));

		MemStrCopyA(m_VolDescPrimary.ucSysIdentifier,szMultiSystem,iSystemCopyLen);
		MemStrCopyD(m_VolDescPrimary.ucVolSetIdentifier,szMultiVolSetIdent,iVolSetIdentCopyLen);
		MemStrCopyA(m_VolDescPrimary.ucPublIdentifier,szMultiPublIdent,iPublIdentCopyLen);
		MemStrCopyA(m_VolDescPrimary.ucPrepIdentifier,szMultiPrepIdent,iPrepIdentCopyLen);
	#else
		MemStrCopyA(m_VolDescPrimary.ucSysIdentifier,szSystem,iSystemCopyLen);
		MemStrCopyD(m_VolDescPrimary.ucVolSetIdentifier,szVolSetIdent,iVolSetIdentCopyLen);
		MemStrCopyA(m_VolDescPrimary.ucPublIdentifier,szPublIdent,iPublIdentCopyLen);
		MemStrCopyA(m_VolDescPrimary.ucPrepIdentifier,szPrepIdent,iPrepIdentCopyLen);
	#endif
	}

	void CIso9660::SetFileFields(const TCHAR *szCopyFileIdent,
								 const TCHAR *szAbstFileIdent,
								 const TCHAR *szBiblFileIdent)
	{
		size_t iCopyFileIdentLen = lstrlen(szCopyFileIdent);
		size_t iAbstFileIdentLen = lstrlen(szAbstFileIdent);
		size_t iBiblFileIdentLen = lstrlen(szBiblFileIdent);

		size_t iCopyFileIdentCopyLen = iCopyFileIdentLen < 37 ? iCopyFileIdentLen : 37;
		size_t iAbstFileIdentCopyLen = iAbstFileIdentLen < 37 ? iAbstFileIdentLen : 37;
		size_t iBiblFileIdentCopyLen = iBiblFileIdentLen < 37 ? iBiblFileIdentLen : 37;

		memset(m_VolDescPrimary.ucCopyFileIdentifier,0x20,sizeof(m_VolDescPrimary.ucCopyFileIdentifier));
		memset(m_VolDescPrimary.ucAbstFileIdentifier,0x20,sizeof(m_VolDescPrimary.ucAbstFileIdentifier));
		memset(m_VolDescPrimary.ucBiblFileIdentifier,0x20,sizeof(m_VolDescPrimary.ucBiblFileIdentifier));

	#ifdef UNICODE
		char szMultiCopyFileIdent[38];
		char szMultiAbstFileIdent[38];
		char szMultiBiblFileIdent[38];

		UnicodeToAnsi(szMultiCopyFileIdent,szCopyFileIdent,sizeof(szMultiCopyFileIdent));
		UnicodeToAnsi(szMultiAbstFileIdent,szAbstFileIdent,sizeof(szMultiAbstFileIdent));
		UnicodeToAnsi(szMultiBiblFileIdent,szBiblFileIdent,sizeof(szMultiBiblFileIdent));

		MemStrCopyD(m_VolDescPrimary.ucCopyFileIdentifier,szMultiCopyFileIdent,iCopyFileIdentCopyLen);
		MemStrCopyD(m_VolDescPrimary.ucAbstFileIdentifier,szMultiAbstFileIdent,iAbstFileIdentCopyLen);
		MemStrCopyD(m_VolDescPrimary.ucBiblFileIdentifier,szMultiBiblFileIdent,iBiblFileIdentCopyLen);
	#else
		MemStrCopyD(m_VolDescPrimary.ucCopyFileIdentifier,szCopyFileIdent,iCopyFileIdentCopyLen);
		MemStrCopyD(m_VolDescPrimary.ucAbstFileIdentifier,szAbstFileIdent,iAbstFileIdentCopyLen);
		MemStrCopyD(m_VolDescPrimary.ucBiblFileIdentifier,szBiblFileIdent,iBiblFileIdentCopyLen);
	#endif
	}

	void CIso9660::SetInterchangeLevel(eInterLevel eInterLevel)
	{
		m_InterLevel = eInterLevel;
	}

	void CIso9660::SetRelaxMaxDirLevel(bool bRelaxRestriction)
	{
		m_bRelaxMaxDirLevel = bRelaxRestriction;
	}

	void CIso9660::SetIncludeFileVerInfo(bool bIncludeInfo)
	{
		m_bIncFileVerInfo = bIncludeInfo;
	}

	bool CIso9660::WriteVolDescPrimary(COutStream *pOutStream,SYSTEMTIME &stImageCreate,
		unsigned long ulVolSpaceSize,unsigned long ulPathTableSize,unsigned long ulPosPathTableL,
		unsigned long ulPosPathTableM,unsigned long ulRootExtentLoc,unsigned long ulDataLen)
	{
		// Initialize the primary volume descriptor.
		Write733(m_VolDescPrimary.ucVolSpaceSize,ulVolSpaceSize);		// Volume size in sectors.
		Write723(m_VolDescPrimary.ucVolSetSize,1);		// Only one disc in the volume set.
		Write723(m_VolDescPrimary.ucVolSeqNumber,1);	// This is the first disc in the volume set.
		Write723(m_VolDescPrimary.ucLogicalBlockSize,ISO9660_SECTOR_SIZE);
		Write733(m_VolDescPrimary.ucPathTableSize,ulPathTableSize);	// Path table size in bytes.
		Write731(m_VolDescPrimary.ucPathTableTypeL,ulPosPathTableL);	// Start sector of LSBF path table.
		Write732(m_VolDescPrimary.ucPathTableTypeM,ulPosPathTableM);	// Start sector of MSBF path table.

		Write733(m_VolDescPrimary.RootDirRecord.ucExtentLocation,ulRootExtentLoc);
		Write733(m_VolDescPrimary.RootDirRecord.ucDataLen,ulDataLen);
		Write723(m_VolDescPrimary.RootDirRecord.ucVolSeqNumber,1);	// The file extent is on the first volume set.

		// Time information.
		MakeDateTime(stImageCreate,m_VolDescPrimary.RootDirRecord.RecDateTime);

		MakeDateTime(stImageCreate,m_VolDescPrimary.CreateDateTime);
		memcpy(&m_VolDescPrimary.ModDateTime,&m_VolDescPrimary.CreateDateTime,sizeof(tVolDescDateTime));

		memset(&m_VolDescPrimary.ExpDateTime,'0',sizeof(tVolDescDateTime));
		m_VolDescPrimary.ExpDateTime.ucZone = 0x00;
		memset(&m_VolDescPrimary.EffectiveDateTime,'0',sizeof(tVolDescDateTime));
		m_VolDescPrimary.EffectiveDateTime.ucZone = 0x00;

		// Write the primary volume descriptor.
		unsigned long ulProcessedSize;
		if (pOutStream->Write(&m_VolDescPrimary,sizeof(m_VolDescPrimary),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(m_VolDescPrimary))
			return false;

		return true;
	}

	bool CIso9660::WriteVolDescSuppl(COutStream *pOutStream,SYSTEMTIME &stImageCreate,
		unsigned long ulVolSpaceSize,unsigned long ulPathTableSize,unsigned long ulPosPathTableL,
		unsigned long ulPosPathTableM,unsigned long ulRootExtentLoc,unsigned long ulDataLen)
	{
		if (m_InterLevel == ISO9660_1999)
		{
			tVolDescSuppl SupplDesc;
			memcpy(&SupplDesc,&m_VolDescPrimary,sizeof(tVolDescSuppl));

			// Update the version information.
			m_VolDescPrimary.ucType = VOLDESCTYPE_SUPPL_VOL_DESC;
			m_VolDescPrimary.ucVersion = 2;			// ISO9660:1999
			m_VolDescPrimary.ucFileStructVer = 2;	// ISO9660:1999

			// Rewrite the values from the primary volume descriptor. We can't guarantee that
			// WriteVolDescPrimary has been called before this function call, even though it
			// should have.
			Write733(SupplDesc.ucVolSpaceSize,ulVolSpaceSize);		// Volume size in sectors.
			Write723(SupplDesc.ucVolSetSize,1);		// Only one disc in the volume set.
			Write723(SupplDesc.ucVolSeqNumber,1);	// This is the first disc in the volume set.
			Write723(SupplDesc.ucLogicalBlockSize,ISO9660_SECTOR_SIZE);
			Write733(SupplDesc.ucPathTableSize,ulPathTableSize);	// Path table size in bytes.
			Write731(SupplDesc.ucPathTableTypeL,ulPosPathTableL);	// Start sector of LSBF path table.
			Write732(SupplDesc.ucPathTableTypeM,ulPosPathTableM);	// Start sector of MSBF path table.

			Write733(SupplDesc.RootDirRecord.ucExtentLocation,ulRootExtentLoc);
			Write733(SupplDesc.RootDirRecord.ucDataLen,ulDataLen);
			Write723(SupplDesc.RootDirRecord.ucVolSeqNumber,1);	// The file extent is on the first volume set.

			// Time information.
			MakeDateTime(stImageCreate,SupplDesc.RootDirRecord.RecDateTime);

			MakeDateTime(stImageCreate,SupplDesc.CreateDateTime);
			memcpy(&SupplDesc.ModDateTime,&SupplDesc.CreateDateTime,sizeof(tVolDescDateTime));

			memset(&SupplDesc.ExpDateTime,'0',sizeof(tVolDescDateTime));
			SupplDesc.ExpDateTime.ucZone = 0x00;
			memset(&SupplDesc.EffectiveDateTime,'0',sizeof(tVolDescDateTime));
			SupplDesc.EffectiveDateTime.ucZone = 0x00;

			// Write the primary volume descriptor.
			unsigned long ulProcessedSize;
			if (pOutStream->Write(&SupplDesc,sizeof(SupplDesc),&ulProcessedSize) != STREAM_OK)
				return false;
			if (ulProcessedSize != sizeof(SupplDesc))
				return false;

			return true;
		}

		return false;
	}

	bool CIso9660::WriteVolDescSetTerm(COutStream *pOutStream)
	{
		// Write volume descriptor set terminator.
		unsigned long ulProcessedSize;
		if (pOutStream->Write(&m_VolDescSetTerm,sizeof(m_VolDescSetTerm),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(m_VolDescSetTerm))
			return false;

		return true;
	}

	unsigned char CIso9660::WriteFileName(unsigned char *pOutBuffer,const TCHAR *szFileName,bool bIsDir)
	{
		switch (m_InterLevel)
		{
			case LEVEL_1:
			default:
				if (bIsDir)
				{
					return WriteDirNameL1(pOutBuffer,szFileName);
				}
				else
				{
					unsigned char ucFileNameLen = WriteFileNameL1(pOutBuffer,szFileName);

					if (m_bIncFileVerInfo)
					{
						pOutBuffer[ucFileNameLen++] = ';';
						pOutBuffer[ucFileNameLen++] = '1';
					}

					return ucFileNameLen;
				}
				break;

			case LEVEL_2:
				if (bIsDir)
				{
					return WriteDirNameL2(pOutBuffer,szFileName);
				}
				else
				{
					unsigned char ucFileNameLen = WriteFileNameL2(pOutBuffer,szFileName);

					if (m_bIncFileVerInfo)
					{
						pOutBuffer[ucFileNameLen++] = ';';
						pOutBuffer[ucFileNameLen++] = '1';
					}

					return ucFileNameLen;
				}
				break;

			case ISO9660_1999:
				if (bIsDir)
					return WriteDirName1999(pOutBuffer,szFileName);
				else
					return WriteFileName1999(pOutBuffer,szFileName);
		}
	}

	unsigned char CIso9660::CalcFileNameLen(const TCHAR *szFileName,bool bIsDir)
	{
		switch (m_InterLevel)
		{
			case LEVEL_1:
			default:
				if (bIsDir)
				{
					return CalcDirNameLenL1(szFileName);
				}
				else
				{
					unsigned char ucFileNameLen = CalcFileNameLenL1(szFileName);

					if (m_bIncFileVerInfo)
						ucFileNameLen += 2;

					return ucFileNameLen;
				}
				break;

			case LEVEL_2:
				if (bIsDir)
				{
					return CalcDirNameLenL2(szFileName);
				}
				else
				{
					unsigned char ucFileNameLen = CalcFileNameLenL2(szFileName);

					if (m_bIncFileVerInfo)
						ucFileNameLen += 2;

					return ucFileNameLen;
				}
				break;

			case ISO9660_1999:
				if (bIsDir)
					return CalcDirNameLen1999(szFileName);
				else
					return CalcFileNameLen1999(szFileName);
		}
	}

	unsigned char CIso9660::GetMaxDirLevel()
	{
		if (m_bRelaxMaxDirLevel)
		{
			return ISO9660_MAX_DIRLEVEL_1999;
		}
		else
		{
			switch (m_InterLevel)
			{
				case LEVEL_1:
				case LEVEL_2:
				default:
					return ISO9660_MAX_DIRLEVEL_NORMAL;

				case ISO9660_1999:
					return ISO9660_MAX_DIRLEVEL_1999;
			}
		}
	}

	/*
		Returns true if the file system has a supplementary volume descriptor.
	*/
	bool CIso9660::HasVolDescSuppl()
	{
		return m_InterLevel == ISO9660_1999;
	}
};
