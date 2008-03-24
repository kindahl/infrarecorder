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
#include "../../Common/FileManager.h"
#include "../../Common/FileStream.h"
#include "ElTorito.h"
#include "Iso9660.h"

namespace ckFileSystem
{
	CElTorito::CElTorito(CLog *pLog) : m_pLog(pLog)
	{
	}

	CElTorito::~CElTorito()
	{
		// Free the children.
		std::vector<CElToritoImage *>::iterator itImage;
		for (itImage = m_BootImages.begin(); itImage != m_BootImages.end(); itImage++)
			delete *itImage;

		m_BootImages.clear();
	}

	bool CElTorito::ReadSysTypeMBR(const TCHAR *szFullPath,unsigned char &ucSysType)
	{
		// Find the system type in the path table located in the MBR.
		CInFileStream InFile;
		if (!InFile.Open(szFullPath))
		{
			m_pLog->AddLine(_T("  Error: Unable to obtain file handle to \"%s\"."),szFullPath);
			return false;
		}

		tMasterBootRec MBR;

		unsigned long ulProcessedSize;
		if (InFile.Read(&MBR,sizeof(tMasterBootRec),&ulProcessedSize) != STREAM_OK)
			return false;

		if (ulProcessedSize != sizeof(tMasterBootRec) ||
			(MBR.ucSignature1 != 0x55 || MBR.ucSignature2 != 0xAA))
		{
			m_pLog->AddLine(_T("  Error: Unable to locate MBR in default boot image."));
			return false;
		}

		size_t iUsedPartition = -1;
		for (size_t i = 0; i < MBR_PARTITION_COUNT; i++)
		{
			// Look for the first used partition.
			if (MBR.Partitions[i].ucPartType != 0)
			{
				if (iUsedPartition != -1)
				{
					m_pLog->AddLine(_T("  Error: Invalid boot image, it contains more than one partition."));
					return false;
				}
				else
				{
					iUsedPartition = i;
				}
			}
		}

		ucSysType = MBR.Partitions[iUsedPartition].ucPartType;
		return true;
	}

	bool CElTorito::WriteBootRecord(CSectorOutStream *pOutStream,unsigned long ulBootCatSecPos)
	{
		tVolDescElToritoRecord BootRecord;
		memset(&BootRecord,0,sizeof(tVolDescElToritoRecord));

		BootRecord.ucType = 0;
		memcpy(BootRecord.ucIdentifier,g_IdentCD,sizeof(BootRecord.ucIdentifier));
		BootRecord.ucVersion = 1;
		memcpy(BootRecord.ucBootSysIdentifier,g_IdentElTorito,strlen(g_IdentElTorito));
		BootRecord.uiBootCatalogPtr = ulBootCatSecPos;

		unsigned long ulProcessedSize;
		if (pOutStream->Write(&BootRecord,sizeof(BootRecord),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(BootRecord))
			return false;

		return true;
	}

	bool CElTorito::WriteBootCatalog(CSectorOutStream *pOutStream)
	{
		char szManufacturer[] = { 0x49,0x4E,0x46,0x52,0x41,0x52,0x45,0x43,0x4F,0x52,0x44,0x45,0x52 };
		tElToritoValiEntry ValidationEntry;
		memset(&ValidationEntry,0,sizeof(tElToritoValiEntry));

		ValidationEntry.ucHeader = 0x01;
		ValidationEntry.ucPlatform = ELTORITO_PLATFORM_80X86;
		memcpy(ValidationEntry.ucManufacturer,szManufacturer,13);
		ValidationEntry.ucKeyByte1 = 0x55;
		ValidationEntry.ucKeyByte2 = 0xAA;

		// Calculate check sum.
		int iCheckSum = 0;
		unsigned char *pEntryPtr = (unsigned char *)&ValidationEntry;
		for (size_t i = 0; i < sizeof(tElToritoValiEntry); i += 2) {
			iCheckSum += (unsigned int)pEntryPtr[i];
			iCheckSum += ((unsigned int)pEntryPtr[i + 1]) << 8;
		}

		ValidationEntry.usCheckSum = -iCheckSum;

		unsigned long ulProcessedSize;
		if (pOutStream->Write(&ValidationEntry,sizeof(ValidationEntry),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(ValidationEntry))
			return false;

		// Write the default boot entry.
		if (m_BootImages.size() < 1)
			return false;

		CElToritoImage *pDefImage = m_BootImages[0];

		tElToritoDefEntry DefBootEntry;
		memset(&DefBootEntry,0,sizeof(tElToritoDefEntry));

		DefBootEntry.ucBootIndicator = pDefImage->m_bBootable ?
			ELTORITO_BOOTINDICATOR_BOOTABLE : ELTORITO_BOOTINDICATOR_NONBOOTABLE;
		DefBootEntry.usLoadSegment = pDefImage->m_usLoadSegment;
		DefBootEntry.usSectorCount = pDefImage->m_usSectorCount;
		DefBootEntry.ulLoadSecAddr = pDefImage->m_ulDataSecPos;

		switch (pDefImage->m_Emulation)
		{
			case CElToritoImage::EMULATION_NONE:
				DefBootEntry.ucEmulation = ELTORITO_EMULATION_NONE;
				DefBootEntry.ucSysType = 0;
				break;

			case CElToritoImage::EMULATION_FLOPPY:
				switch (fs_filesize(pDefImage->m_FullPath.c_str()))
				{
					case 1200 * 1024:
						DefBootEntry.ucEmulation = ELTORITO_EMULATION_DISKETTE12;
						break;
					case 1440 * 1024:
						DefBootEntry.ucEmulation = ELTORITO_EMULATION_DISKETTE144;
						break;
					case 2880 * 1024:
						DefBootEntry.ucEmulation = ELTORITO_EMULATION_DISKETTE288;
						break;
					default:
						return false;
				}

				DefBootEntry.ucSysType = 0;
				break;

			case CElToritoImage::EMULATION_HARDDISK:
				DefBootEntry.ucEmulation = ELTORITO_EMULATION_HARDDISK;

				if (!ReadSysTypeMBR(pDefImage->m_FullPath.c_str(),DefBootEntry.ucSysType))
					return false;
				break;
		}

		if (pOutStream->Write(&DefBootEntry,sizeof(DefBootEntry),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(DefBootEntry))
			return false;

		// Write the rest of the boot images.
		tElToritoSecHeader SecHeader;
		tElToritoSecEntry SecEntry;

		unsigned short usNumImages = (unsigned short)m_BootImages.size();
		for (unsigned short i = 1; i < usNumImages; i++)
		{
			CElToritoImage *pCurImage = m_BootImages[i];

			// Write section header.
			memset(&SecHeader,0,sizeof(tElToritoSecHeader));

			SecHeader.ucHeader = i == (usNumImages - 1) ?
				ELTORITO_HEADER_FINAL : ELTORITO_HEADER_NORMAL;
			SecHeader.ucPlatform = ELTORITO_PLATFORM_80X86;
			SecHeader.usNumSecEntries = 1;

			char szIdentifier[16];
			sprintf(szIdentifier,"IMAGE%u",i + 1);
			memcpy(SecHeader.ucIndentifier,szIdentifier,strlen(szIdentifier));

			if (pOutStream->Write(&SecHeader,sizeof(SecHeader),&ulProcessedSize) != STREAM_OK)
				return false;
			if (ulProcessedSize != sizeof(SecHeader))
				return false;

			// Write the section entry.
			memset(&SecEntry,0,sizeof(tElToritoSecEntry));

			SecEntry.ucBootIndicator = pCurImage->m_bBootable ?
				ELTORITO_BOOTINDICATOR_BOOTABLE : ELTORITO_BOOTINDICATOR_NONBOOTABLE;
			SecEntry.usLoadSegment = pCurImage->m_usLoadSegment;
			SecEntry.usSectorCount = pCurImage->m_usSectorCount;
			SecEntry.ulLoadSecAddr = pCurImage->m_ulDataSecPos;

			switch (pCurImage->m_Emulation)
			{
				case CElToritoImage::EMULATION_NONE:
					SecEntry.ucEmulation = ELTORITO_EMULATION_NONE;
					SecEntry.ucSysType = 0;
					break;

				case CElToritoImage::EMULATION_FLOPPY:
					switch (fs_filesize(pCurImage->m_FullPath.c_str()))
					{
						case 1200 * 1024:
							SecEntry.ucEmulation = ELTORITO_EMULATION_DISKETTE12;
							break;
						case 1440 * 1024:
							SecEntry.ucEmulation = ELTORITO_EMULATION_DISKETTE144;
							break;
						case 2880 * 1024:
							SecEntry.ucEmulation = ELTORITO_EMULATION_DISKETTE288;
							break;
						default:
							return false;
					}

					SecEntry.ucSysType = 0;
					break;

				case CElToritoImage::EMULATION_HARDDISK:
					SecEntry.ucEmulation = ELTORITO_EMULATION_HARDDISK;

					if (!ReadSysTypeMBR(pCurImage->m_FullPath.c_str(),SecEntry.ucSysType))
						return false;
					break;
			}

			if (pOutStream->Write(&SecEntry,sizeof(SecEntry),&ulProcessedSize) != STREAM_OK)
				return false;
			if (ulProcessedSize != sizeof(SecEntry))
				return false;
		}

		if (pOutStream->GetAllocated() != 0)
			pOutStream->PadSector();

		return true;
	}

	bool CElTorito::WriteBootImage(CSectorOutStream *pOutStream,const TCHAR *szFileName)
	{
		CInFileStream FileStream;
		if (!FileStream.Open(szFileName))
		{
			m_pLog->AddLine(_T("  Error: Unable to obtain file handle to \"%s\"."),szFileName);
			return false;
		}

		char szBuffer[ELTORITO_IO_BUFFER_SIZE];
		unsigned long ulProcessedSize = 0;

		while (!FileStream.EOS())
		{
			if (FileStream.Read(szBuffer,ELTORITO_IO_BUFFER_SIZE,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("  Error: Unable read file: %s."),szFileName);
				return false;
			}

			if (pOutStream->Write(szBuffer,ulProcessedSize,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("  Error: Unable write to disc image."));
				return false;
			}
		}

		// Pad the sector.
		if (pOutStream->GetAllocated() != 0)
			pOutStream->PadSector();

		return true;
	}

	bool CElTorito::WriteBootImages(CSectorOutStream *pOutStream)
	{
		std::vector<CElToritoImage *>::iterator itImage;
		for (itImage = m_BootImages.begin(); itImage != m_BootImages.end(); itImage++)
		{
			if (!WriteBootImage(pOutStream,(*itImage)->m_FullPath.c_str()))
				return false;
		}

		return true;
	}

	bool CElTorito::AddBootImageNoEmu(const TCHAR *szFullPath,bool bBootable,
		unsigned short usLoadSegment,unsigned short usSectorCount)
	{
		if (m_BootImages.size() >= ELTORITO_MAX_BOOTIMAGE_COUNT)
			return false;

		if (!fs_fileexists(szFullPath))
			return false;

		m_BootImages.push_back(new CElToritoImage(
			szFullPath,bBootable,CElToritoImage::EMULATION_NONE,usLoadSegment,usSectorCount));
		return true;
	}

	bool CElTorito::AddBootImageFloppy(const TCHAR *szFullPath,bool bBootable)
	{
		if (m_BootImages.size() >= ELTORITO_MAX_BOOTIMAGE_COUNT)
			return false;

		if (!fs_fileexists(szFullPath))
			return false;

		__int64 iFileSize = fs_filesize(szFullPath);
		if (iFileSize != 1200 * 1024 && iFileSize != 1440 * 1024 && iFileSize != 2880 * 1024)
		{
			m_pLog->AddLine(_T("  Error: Invalid file size for floppy emulated boot image."));
			return false;
		}

		// usSectorCount = 1, only load one sector for floppies.
		m_BootImages.push_back(new CElToritoImage(
			szFullPath,bBootable,CElToritoImage::EMULATION_FLOPPY,0,1));
		return true;
	}

	bool CElTorito::AddBootImageHardDisk(const TCHAR *szFullPath,bool bBootable)
	{
		if (m_BootImages.size() >= ELTORITO_MAX_BOOTIMAGE_COUNT)
			return false;

		if (!fs_fileexists(szFullPath))
			return false;

		unsigned char ucDummy;
		if (!ReadSysTypeMBR(szFullPath,ucDummy))
			return false;

		// usSectorCount = 1, Only load the MBR.
		m_BootImages.push_back(new CElToritoImage(
			szFullPath,bBootable,CElToritoImage::EMULATION_HARDDISK,0,1));
		return true;
	}

	bool CElTorito::CalculateFileSysData(unsigned __int64 uiStartSec,
		unsigned __int64 &uiLastSec)
	{
		std::vector<CElToritoImage *>::iterator itImage;
		for (itImage = m_BootImages.begin(); itImage != m_BootImages.end(); itImage++)
		{
			if (uiStartSec > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Error: Sector offset overflow (%I64d), can not include boot image: %s."),
					uiStartSec,(*itImage)->m_FullPath.c_str());
				return false;
			}

			(*itImage)->m_ulDataSecPos = (unsigned long)uiStartSec;

			uiStartSec += BytesToSector64((unsigned __int64)
				fs_filesize((*itImage)->m_FullPath.c_str()));
		}

		uiLastSec = uiStartSec;
		return true;
	}

	unsigned __int64 CElTorito::GetBootCatSize()
	{
		// The validator and default boot image allocates 64 bytes, the remaining
		// boot images allocates 64 bytes a piece.
		return m_BootImages.size() << 6;
	}

	unsigned __int64 CElTorito::GetBootDataSize()
	{
		unsigned __int64 uiSize = 0;
		std::vector<CElToritoImage *>::iterator itImage;
		for (itImage = m_BootImages.begin(); itImage != m_BootImages.end(); itImage++)
			uiSize += BytesToSector64((unsigned __int64)fs_filesize((*itImage)->m_FullPath.c_str()));

		return uiSize;
	}

	unsigned __int64 CElTorito::GetBootImageCount()
	{
		return m_BootImages.size();
	}
};
