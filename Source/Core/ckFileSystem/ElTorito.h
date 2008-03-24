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
#include <string>
#include <vector>
#include "../../Common/StringUtil.h"
#include "../../Common/Log.h"
#include "SectorStream.h"

// Maximum values of unsigned short + 1 + the default boot image.
#define ELTORITO_MAX_BOOTIMAGE_COUNT			0xFFFF + 2

#define ELTORITO_IO_BUFFER_SIZE					0x10000

#pragma pack(1)	// Force byte alignment.

/*
	Boot Catalog Entries.
*/
#define ELTORITO_PLATFORM_80X86					0
#define ELTORITO_PLATFORM_POWERPC				1
#define ELTORITO_PLATFORM_MAC					2

typedef struct
{
	unsigned char ucHeader;
	unsigned char ucPlatform;
	unsigned short usReserved1;
	unsigned char ucManufacturer[24];
	unsigned short usCheckSum;
	unsigned char ucKeyByte1;		// Must be 0x55.
	unsigned char ucKeyByte2;		// Must be 0xAA.
} tElToritoValiEntry;

#define ELTORITO_BOOTINDICATOR_BOOTABLE			0x88
#define ELTORITO_BOOTINDICATOR_NONBOOTABLE		0x00

#define ELTORITO_EMULATION_NONE					0
#define ELTORITO_EMULATION_DISKETTE12			1
#define ELTORITO_EMULATION_DISKETTE144			2
#define ELTORITO_EMULATION_DISKETTE288			3
#define ELTORITO_EMULATION_HARDDISK				4

typedef struct
{
	unsigned char ucBootIndicator;
	unsigned char ucEmulation;
	unsigned short usLoadSegment;
	unsigned char ucSysType;		// Must be a copy of byte 5 (System Type) from boot image partition table.
	unsigned char ucUnused1;
	unsigned short usSectorCount;
	unsigned long ulLoadSecAddr;
	unsigned char ucUnused2[20];
} tElToritoDefEntry;

#define ELTORITO_HEADER_NORMAL					0x90
#define ELTORITO_HEADER_FINAL					0x91

typedef struct
{
	unsigned char ucHeader;
	unsigned char ucPlatform;
	unsigned short usNumSecEntries;
	unsigned char ucIndentifier[28];
} tElToritoSecHeader;

typedef struct
{
	unsigned char ucBootIndicator;
	unsigned char ucEmulation;
	unsigned short usLoadSegment;
	unsigned char ucSysType;		// Must be a copy of byte 5 (System Type) from boot image partition table.
	unsigned char ucUnused1;
	unsigned short usSectorCount;
	unsigned long ulLoadSecAddr;
	unsigned char ucSelCriteria;
	unsigned char ucUnused2[19];
} tElToritoSecEntry;

/*
	Structures for reading the master boot record of a boot image.
*/
#define MBR_PARTITION_COUNT						4

typedef struct
{
	unsigned char ucBootIndicator;
	unsigned char ucPartStartCHS[3];
	unsigned char ucPartType;
	unsigned char ucPartEndCHS[3];
	unsigned long ulStartLBA;
	unsigned long ulSecCount;
} tMasterBootRecPart;

typedef struct
{
	unsigned char ucCodeArea[440];
	unsigned long ulOptDiscSig;
	unsigned short usPad;
	tMasterBootRecPart Partitions[MBR_PARTITION_COUNT];
	unsigned char ucSignature1;
	unsigned char ucSignature2;
} tMasterBootRec;

#pragma pack()	// Switch back to normal alignment.

namespace ckFileSystem
{
	class CElToritoImage
	{
	public:
		enum eEmulation
		{
			EMULATION_NONE,
			EMULATION_FLOPPY,
			EMULATION_HARDDISK
		};

		tstring m_FullPath;
		bool m_bBootable;
		eEmulation m_Emulation;
		unsigned short m_usLoadSegment;
		unsigned short m_usSectorCount;

		// Needs to be calculated in a separate pass.
		unsigned long m_ulDataSecPos;	// Sector number of first sector containing data.

		CElToritoImage(const TCHAR *szFullPath,bool bBootable,eEmulation Emulation,
			unsigned short usLoadSegment,unsigned short usSectorCount)
		{
			m_FullPath = szFullPath;
			m_bBootable = bBootable;
			m_Emulation = Emulation;
			m_usLoadSegment = usLoadSegment;
			m_usSectorCount = usSectorCount;

			m_ulDataSecPos = 0;
		}
	};

	class CElTorito
	{
	private:
		CLog *m_pLog;

		std::vector<CElToritoImage *> m_BootImages;

		bool ReadSysTypeMBR(const TCHAR *szFullPath,unsigned char &ucSysType);

		bool WriteBootImage(CSectorOutStream *pOutStream,const TCHAR *szFileName);

	public:
		CElTorito(CLog *pLog);
		~CElTorito();

		bool WriteBootRecord(CSectorOutStream *pOutStream,unsigned long ulBootCatSecPos);
		bool WriteBootCatalog(CSectorOutStream *pOutStream);
		bool WriteBootImages(CSectorOutStream *pOutStream);

		bool AddBootImageNoEmu(const TCHAR *szFullPath,bool bBootable,
			unsigned short usLoadSegment,unsigned short usSectorCount);
		bool AddBootImageFloppy(const TCHAR *szFullPath,bool bBootable);
		bool AddBootImageHardDisk(const TCHAR *szFullPath,bool bBootable);

		bool CalculateFileSysData(unsigned __int64 uiStartSec,
			unsigned __int64 &uiLastSec);

		unsigned __int64 GetBootCatSize();
		unsigned __int64 GetBootDataSize();
		unsigned __int64 GetBootImageCount();
	};
};
