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
#include "../../Common/Stream.h"

#define ISO9660_SECTOR_SIZE						2048
#define ISO9660_MAX_NAMELEN_1999				 207
#define ISO9660_MAX_DIRLEVEL_NORMAL				   8		// Maximum is 8 for ISO9660:1988.
#define ISO9660_MAX_DIRLEVEL_1999				 255		// Maximum is 255 for ISO9660:1999.
#define ISO9660_MAX_EXTENT_SIZE					0xFFFFF800

#define DIRRECORD_FILEFLAG_HIDDEN				1 << 0
#define DIRRECORD_FILEFLAG_DIRECTORY			1 << 1
#define DIRRECORD_FILEFLAG_ASSOCIATEDFILE		1 << 2
#define DIRRECORD_FILEFLAG_RECORD				1 << 3
#define DIRRECORD_FILEFLAG_PROTECTION			1 << 4
#define DIRRECORD_FILEFLAG_MULTIEXTENT			1 << 7

#define VOLDESCTYPE_BOOT_CATALOG				0
#define VOLDESCTYPE_PRIM_VOL_DESC				1
#define VOLDESCTYPE_SUPPL_VOL_DESC				2
#define VOLDESCTYPE_VOL_PARTITION_DESC			3
#define VOLDESCTYPE_VOL_DESC_SET_TERM			255

namespace ckFileSystem
{
	/*
		Identifiers.
	*/
	extern const char *g_IdentCD;
	extern const char *g_IdentElTorito;

	/*
		File and Directory Descriptors.
	*/
#pragma pack(1)	// Force byte alignment.

	typedef struct
	{
		unsigned char ucYear;		// Number of years since 1900.
		unsigned char ucMonth;		// Month of the year from 1 to 12.
		unsigned char ucDay;		// Day of the month from 1 to 31.
		unsigned char ucHour;		// Hour of the day from 0 to 23.
		unsigned char ucMinute;		// Minute of the hour from 0 to 59.
		unsigned char ucSecond;		// Second of the minute from 0 to 59.
		unsigned char ucZone;		// Offset from Greenwich Mean Time in number of
									// 15 min intervals from -48 (West) to + 52 (East)
									// recorded according to 7.1.2.
	} tDirRecordDateTime;

	typedef struct
	{
		unsigned char ucDirRecordLen;
		unsigned char ucExtAttrRecordLen;
		unsigned char ucExtentLocation[8];	// 7.3.3.
		unsigned char ucDataLen[8];			// 7.3.3.
		tDirRecordDateTime RecDateTime;
		unsigned char ucFileFlags;
		unsigned char ucFileUnitSize;
		unsigned char ucInterleaveGapSize;
		unsigned char ucVolSeqNumber[4];	// 7.2.3.
		unsigned char ucFileIdentifierLen;
		unsigned char ucFileIdentifier[1];	// Actually of size ucFileIdentifierLen.
	} tDirRecord;

	typedef struct
	{
		unsigned char ucDirIdentifierLen;
		unsigned char ucExtAttrRecordLen;
		unsigned char ucExtentLocation[4];	// 7.3.?.
		unsigned char ucParentDirNumber[2];	// 7.2.?.
		unsigned char ucDirIdentifier[1];	// Actually consumes the rest of the
											// available path table record size.
	} tPathTableRecord;

	typedef struct
	{
		unsigned char ucOwnerIdentification[4];	// 7.2.3.
		unsigned char ucGroupIdentification[4];	// 7.2.3.
		unsigned short usPermissions;
		tDirRecordDateTime CreateDateTime;
		tDirRecordDateTime ModDateTime;
		tDirRecordDateTime ExpDateTime;
		tDirRecordDateTime EffectiveDateTime;
		unsigned char ucRecFormat;
		unsigned char ucRecAttr;
		unsigned char ucRecLen[4];				// 7.2.3.
		unsigned char ucSysIndetifier[32];
		unsigned char ucSysData[64];
		unsigned char ucExtAttrRecordVersion;
		unsigned char ucEscLen;
		unsigned char ucReserved[64];
		unsigned char ucAppDataLen[4];			// 7.2.3.
		unsigned char ucAppDate[1];				// Actually of size uiAppDataLen.
	} tExtAttrRecord;

	/*
		Volume Descriptors.
	*/
	typedef struct
	{
		unsigned int uiYear;			// Year from I to 9999.
		unsigned short usMonth;			// Month of the year from 1 to 12.
		unsigned short usDay;			// Day of the month from 1 to 31.
		unsigned short usHour;			// Hour of the day from 0 to 23.
		unsigned short usMinute;		// Minute of the hour from 0 to 59.
		unsigned short usSecond;		// Second of the minute from 0 to 59.
		unsigned short usHundreds;		// Hundredths of a second.
		unsigned char ucZone;			// Offset from Greenwich Mean Time in number of
										// 15 min intervals from -48 (West) to +52 (East)
										// recorded according to 7.1.2.
	} tVolDescDateTime;

	typedef struct
	{
		unsigned char ucType;						// 0.
		unsigned char ucIdentifier[5];				// "CD001".
		unsigned char ucVersion;
		unsigned char ucBootSysIdentifier[32];
		unsigned char ucBootIdentifier[32];
		unsigned char ucBootSysData[1977];
	} tVolDescBootRecord;		// Must be 2048 bytes in size.

	typedef struct
	{
		unsigned char ucType;						// 0.
		unsigned char ucIdentifier[5];				// "CD001".
		unsigned char ucVersion;					// Must be 1.
		unsigned char ucBootSysIdentifier[32];		// Must be "EL TORITO SPECIFICATION" padded with 0s.
		unsigned char Unused1[32];					// Must be 0.
		unsigned int uiBootCatalogPtr;				// Absolute pointer to first sector of Boot Catalog.
		unsigned char ucBootSysData[1973];
	} tVolDescElToritoRecord;	// Must be 2048 bytes in size.

	typedef struct
	{
		unsigned char ucType;
		unsigned char ucIdentifier[5];					// "CD001".
		unsigned char ucVersion;
		unsigned char ucUnused1;
		unsigned char ucSysIdentifier[32];
		unsigned char ucVolIdentifier[32];
		unsigned char ucUnused2[8];
		unsigned char ucVolSpaceSize[8];				// 7.3.3.
		unsigned char ucUnused3[32];
		unsigned char ucVolSetSize[4];					// 7.2.3.
		unsigned char ucVolSeqNumber[4];				// 7.2.3.
		unsigned char ucLogicalBlockSize[4];			// 7.2.3.
		unsigned char ucPathTableSize[8];				// 7.3.3.
		unsigned char ucPathTableTypeL[4];				// 7.3.1.
		unsigned char ucOptPathTableTypeL[4];			// 7.3.1.
		unsigned char ucPathTableTypeM[4];				// 7.3.2.
		unsigned char ucOptPathTableTypeM[4];			// 7.3.2.
		tDirRecord RootDirRecord;
		unsigned char ucVolSetIdentifier[128];
		unsigned char ucPublIdentifier[128];
		unsigned char ucPrepIdentifier[128];
		unsigned char ucAppIdentifier[128];
		unsigned char ucCopyFileIdentifier[37];
		unsigned char ucAbstFileIdentifier[37];
		unsigned char ucBiblFileIdentifier[37];
		tVolDescDateTime CreateDateTime;
		tVolDescDateTime ModDateTime;
		tVolDescDateTime ExpDateTime;
		tVolDescDateTime EffectiveDateTime;
		unsigned char ucFileStructVer;
		unsigned char ucUnused4;
		unsigned char ucAppData[512];
		unsigned char ucUnused5[653];
	} tVolDescPrimary;		// Must be 2048 bytes in size.

	typedef struct
	{
		unsigned char ucType;
		unsigned char ucIdentifier[5];					// "CD001".
		unsigned char ucVersion;
		unsigned char ucVolFlags;
		unsigned char ucSysIdentifier[32];
		unsigned char ucVolIdentifier[32];
		unsigned char ucUnused1[8];
		unsigned char ucVolSpaceSize[8];				// 7.3.3.
		unsigned char ucEscapeSeq[32];
		unsigned char ucVolSetSize[4];					// 7.2.3.
		unsigned char ucVolSeqNumber[4];				// 7.2.3.
		unsigned char ucLogicalBlockSize[4];			// 7.2.3.
		unsigned char ucPathTableSize[8];				// 7.3.3.
		unsigned char ucPathTableTypeL[4];				// 7.3.1.
		unsigned char ucOptPathTableTypeL[4];			// 7.3.1.
		unsigned char ucPathTableTypeM[4];				// 7.3.2.
		unsigned char ucOptPathTableTypeM[4];			// 7.3.2.
		tDirRecord RootDirRecord;
		unsigned char ucVolSetIdentifier[128];
		unsigned char ucPublIdentifier[128];
		unsigned char ucPrepIdentifier[128];
		unsigned char ucAppIdentifier[128];
		unsigned char ucCopyFileIdentifier[37];
		unsigned char ucAbstFileIdentifier[37];
		unsigned char ucBiblFileIdentifier[37];
		tVolDescDateTime CreateDateTime;
		tVolDescDateTime ModDateTime;
		tVolDescDateTime ExpDateTime;
		tVolDescDateTime EffectiveDateTime;
		unsigned char ucFileStructVer;
		unsigned char ucUnused2;
		unsigned char ucAppData[512];
		unsigned char ucUnused3[653];
	} tVolDescSuppl;		// Must be 2048 bytes in size.

	typedef struct
	{
		unsigned char ucType;						// 3.
		unsigned char ucIdentifier[5];				// "CD001".
		unsigned char ucVersion;
		unsigned char ucUnused1;
		unsigned char ucSysIdentifier[32];
		unsigned char ucPartitionIdentifier[32];
		unsigned char ucPartitionLocation[8];		// 7.3.3.
		unsigned char ucPartitionSize[8];			// 7.3.3.
		unsigned char ucSysData[1960];
	} tVolDescPartition;	// Must be 2048 bytes in size.

	typedef struct
	{
		unsigned char ucType;						// 255.
		unsigned char ucIdentifier[5];				// "CD001".
		unsigned char ucVersion;
		unsigned char ucReserved[2041];
	} tVolDescSetTerm;		// Must be 2048 bytes in size.

#pragma pack()	// Switch back to normal alignment.

	/// Class for handling ISO9660 file systems.
	/**
		Implements functionallity for creating parts of ISO9660 file systems.
		For example writing certain descriptors and for generating ISO9660
		compatible file names.
	*/
	class CIso9660
	{
	public:
		enum eInterLevel
		{
			LEVEL_1,
			LEVEL_2,
			LEVEL_3,
			ISO9660_1999
		};

	private:
		bool m_bRelaxMaxDirLevel;
		bool m_bIncFileVerInfo;

		eInterLevel m_InterLevel;

		tVolDescPrimary m_VolDescPrimary;
		tVolDescSetTerm m_VolDescSetTerm;

		char MakeCharA(char c);
		char MakeCharD(char c);
		void MemStrCopyA(unsigned char *szTarget,const char *szSource,size_t iLength);
		void MemStrCopyD(unsigned char *szTarget,const char *szSource,size_t iLength);

		unsigned char WriteFileNameL1(unsigned char *pOutBuffer,const TCHAR *szFileName);
		unsigned char WriteFileNameGeneric(unsigned char *pOutBuffer,const TCHAR *szFileName,int iMaxLen);
		unsigned char WriteFileNameL2(unsigned char *pOutBuffer,const TCHAR *szFileName);
		unsigned char WriteFileName1999(unsigned char *pOutBuffer,const TCHAR *szFileName);
		unsigned char WriteDirNameL1(unsigned char *pOutBuffer,const TCHAR *szDirName);
		unsigned char WriteDirNameGeneric(unsigned char *pOutBuffer,const TCHAR *szDirName,int iMaxLen);
		unsigned char WriteDirNameL2(unsigned char *pOutBuffer,const TCHAR *szDirName);
		unsigned char WriteDirName1999(unsigned char *pOutBuffer,const TCHAR *szDirName);
		unsigned char CalcFileNameLenL1(const TCHAR *szFileName);
		unsigned char CalcFileNameLenL2(const TCHAR *szFileName);
		unsigned char CalcFileNameLen1999(const TCHAR *szFileName);
		unsigned char CalcDirNameLenL1(const TCHAR *szFileName);
		unsigned char CalcDirNameLenL2(const TCHAR *szFileName);
		unsigned char CalcDirNameLen1999(const TCHAR *szFileName);

		void InitVolDescPrimary();
		void InitVolDescSetTerm();

	public:
		CIso9660();
		~CIso9660();

		// Change of internal state functions.
		void SetVolumeLabel(const TCHAR *szLabel);
		void SetTextFields(const TCHAR *szSystem,const TCHAR *szVolSetIdent,
			const TCHAR *szPublIdent,const TCHAR *szPrepIdent);
		void SetFileFields(const TCHAR *ucCopyFileIdent,const TCHAR *ucAbstFileIdent,
			const TCHAR *ucBiblIdent);
		void SetInterchangeLevel(eInterLevel eInterLevel);
		void SetRelaxMaxDirLevel(bool bRelaxRestriction);
		void SetIncludeFileVerInfo(bool bIncludeInfo);

		// Write functions.
		bool WriteVolDescPrimary(COutStream *pOutStream,SYSTEMTIME &stImageCreate,
				unsigned long ulVolSpaceSize,unsigned long ulPathTableSize,
				unsigned long ulPosPathTableL,unsigned long ulPosPathTableM,
				unsigned long ulRootExtentLoc,unsigned long ulDataLen);
		bool WriteVolDescSuppl(COutStream *pOutStream,SYSTEMTIME &stImageCreate,
				unsigned long ulVolSpaceSize,unsigned long ulPathTableSize,
				unsigned long ulPosPathTableL,unsigned long ulPosPathTableM,
				unsigned long ulRootExtentLoc,unsigned long ulDataLen);
		bool WriteVolDescSetTerm(COutStream *pOutStream);

		// Helper functions.
		unsigned char WriteFileName(unsigned char *pOutBuffer,const TCHAR *szFileName,bool bIsDir);
		unsigned char CalcFileNameLen(const TCHAR *szFileName,bool bIsDir);
		unsigned char GetMaxDirLevel();
		bool HasVolDescSuppl();
		bool AllowsFragmentation();
		bool IncludesFileVerInfo();
	};

	/*
		Helper Functions.
	*/
	void Write721(unsigned char *pOut,unsigned short usValue);				// Least significant byte first.
	void Write722(unsigned char *pOut,unsigned short usValue);				// Most significant byte first.
	void Write723(unsigned char *pOut,unsigned short usValue);				// Both-byte orders.
	void Write72(unsigned char *pOut,unsigned short usValue,bool bMSBF);	// 7.2.1 or 7.2.2 is decided by parameter.
	void Write731(unsigned char *pOut,unsigned long ulValue);				// Least significant byte first.
	void Write732(unsigned char *pOut,unsigned long ulValue);				// Most significant byte first.
	void Write733(unsigned char *pOut,unsigned long ulValue);				// Both-byte orders.
	void Write73(unsigned char *pOut,unsigned long ulValue,bool bMSBF);		// 7.3.1 or 7.3.2 is decided by parameter.
	unsigned short Read721(unsigned char *pOut);
	unsigned short Read722(unsigned char *pOut);
	unsigned short Read723(unsigned char *pOut);
	unsigned long Read731(unsigned char *pOut);
	unsigned long Read732(unsigned char *pOut);
	unsigned long Read733(unsigned char *pOut);

	unsigned long BytesToSector(unsigned long ulBytes);
	unsigned long BytesToSector(unsigned __int64 uiBytes);
	unsigned __int64 BytesToSector64(unsigned __int64 uiBytes);

	void MakeDateTime(SYSTEMTIME &st,tVolDescDateTime &DateTime);
	void MakeDateTime(SYSTEMTIME &st,tDirRecordDateTime &DateTime);
	void MakeDateTime(unsigned short usDate,unsigned short usTime,tDirRecordDateTime &DateTime);
};
