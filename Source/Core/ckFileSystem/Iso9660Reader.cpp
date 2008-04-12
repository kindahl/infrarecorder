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
#include "Iso9660Writer.h"
#include "Iso9660Reader.h"
#include "Iso9660.h"

namespace ckFileSystem
{
	CIso9660Reader::CIso9660Reader(CLog *pLog) : m_pLog(pLog),
		m_pRootNode(NULL)
	{
	}

	CIso9660Reader::~CIso9660Reader()
	{
		if (m_pRootNode != NULL)
		{
			delete m_pRootNode;
			m_pRootNode = NULL;
		}
	}

	/**
		Reads an entire directory entry.
	*/
	bool CIso9660Reader::ReadDirEntry(CSeekInStream &InStream,
		std::vector<CIso9660TreeNode *> &DirEntries,
		CIso9660TreeNode *pParentNode,bool bJoliet)
	{
		// Search to the extent location.
		InStream.Seek(pParentNode->m_ulExtentLocation * ISO9660_SECTOR_SIZE,CSeekInStream::SM_BEGIN);

		unsigned char ucFileNameBuffer[ISO9660WRITER_FILENAME_BUFFER_SIZE];
		TCHAR szFileName[(ISO9660WRITER_FILENAME_BUFFER_SIZE / sizeof(TCHAR)) + 1];

		// Skip the '.' and '..' entries.
		unsigned long ulRead = 0/*sizeof(tDirRecord) << 1*/,ulProcessedSize;
		//InStream.Seek(ulRead,CSeekInStream::SM_CURRENT);

		for (unsigned int i = 0; i < 2; i++)
		{
			unsigned char ulSysDirRecLen = 0;
			if (InStream.Read(&ulSysDirRecLen,1,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("Error: Unable to read system directory record length."));
				return false;
			}
			if (ulProcessedSize != 1)
			{
				m_pLog->AddLine(_T("Error: Unable to read system directory record length (size mismatch)."));
				return false;
			}

			InStream.Seek(ulSysDirRecLen - 1,CSeekInStream::SM_CURRENT);
			ulRead += ulSysDirRecLen;
		}

		tDirRecord CurDirRecord;

		// Read all other records.
		while (ulRead < pParentNode->m_ulExtentLength)
		{
			// Check if we can read further.
			if (ulRead + sizeof(tDirRecord) > pParentNode->m_ulExtentLength)
				break;

			unsigned long ulDirRecProcessed = 0;

			memset(&CurDirRecord,0,sizeof(tDirRecord));
			if (InStream.Read(&CurDirRecord,sizeof(tDirRecord) - 1,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("Error: Unable to read directory record."));
				return false;
			}
			if (ulProcessedSize != sizeof(tDirRecord) - 1)
			{
				m_pLog->AddLine(_T("Error: Unable to read directory record (size mismatch: %u vs. %u)."),
					ulProcessedSize,sizeof(tDirRecord) - 1);
				return false;
			}
			ulDirRecProcessed += ulProcessedSize;

			if (CurDirRecord.ucFileIdentifierLen > ISO9660WRITER_FILENAME_BUFFER_SIZE)
			{
				m_pLog->AddLine(_T("Error: Directory record file identifier is too large: %u bytes."),
					CurDirRecord.ucFileIdentifierLen);
				return false;
			}

			// Read identifier.
			if (InStream.Read(ucFileNameBuffer,CurDirRecord.ucFileIdentifierLen,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("Error: Unable to read directory record file identifier."));
				return false;
			}
			if (ulProcessedSize != CurDirRecord.ucFileIdentifierLen)
			{
				m_pLog->AddLine(_T("Error: Unable to read directory record file identifier (size mismatch)."));
				return false;
			}
			ulDirRecProcessed += ulProcessedSize;

			// Convert identifier.
			if (bJoliet)
			{
#ifdef UNICODE
				wchar_t *szWideFileName = szFileName;
#else
				wchar_t szWideFileName[(ISO9660WRITER_FILENAME_BUFFER_SIZE >> 1) + 1];
#endif
				unsigned char ucFileNameLen = CurDirRecord.ucFileIdentifierLen >> 1;
				unsigned char ucFileNamePos = 0;

				for (unsigned int i = 0; i < ucFileNameLen; i++)
				{
					szWideFileName[i]  = ucFileNameBuffer[ucFileNamePos++] << 8;
					szWideFileName[i] |= ucFileNameBuffer[ucFileNamePos++];
				}

				szWideFileName[ucFileNameLen] = '\0';

#ifndef UNICODE
				UnicodeToAnsi(szFileName,szWideFileName,sizeof(szFileName));
#endif
			}
			else
			{
#ifdef UNICODE
				AnsiToUnicode(szFileName,(const char *)ucFileNameBuffer,sizeof(szFileName) / sizeof(TCHAR));
#else
				memcpy(szFileName,ucFileNameBuffer,CurDirRecord.ucFileIdentifierLen);
#endif
				szFileName[CurDirRecord.ucFileIdentifierLen] = '\0';
			}

			// Remove any version information.
			unsigned char ucLength = bJoliet ? CurDirRecord.ucFileIdentifierLen >> 1 :
				CurDirRecord.ucFileIdentifierLen;
			if (szFileName[ucLength - 2] == ';')
				szFileName[ucLength - 2] = '\0';

			//m_pLog->AddLine(_T("  %s: %u"),szFileName,Read733(CurDirRecord.ucExtentLocation));

			CIso9660TreeNode *pNewNode = new CIso9660TreeNode(pParentNode,szFileName,
					Read733(CurDirRecord.ucExtentLocation),Read733(CurDirRecord.ucDataLen),
					Read723(CurDirRecord.ucVolSeqNumber),CurDirRecord.ucFileFlags,
					CurDirRecord.ucFileUnitSize,CurDirRecord.ucInterleaveGapSize,CurDirRecord.RecDateTime);

			if (CurDirRecord.ucFileFlags & DIRRECORD_FILEFLAG_DIRECTORY)
			{
				DirEntries.push_back(pNewNode);
			}

			pParentNode->m_Children.push_back(pNewNode);

			// Skip any extra data.
			if (CurDirRecord.ucDirRecordLen - ulDirRecProcessed > 0)
			{
				InStream.Seek(CurDirRecord.ucDirRecordLen - ulDirRecProcessed,CSeekInStream::SM_CURRENT);
				ulDirRecProcessed = CurDirRecord.ucDirRecordLen;
			}

			ulRead += ulDirRecProcessed;

			// Skip any extended attribute record.
			InStream.Seek(CurDirRecord.ucExtAttrRecordLen,CSeekInStream::SM_CURRENT);
			ulRead += CurDirRecord.ucExtAttrRecordLen;

			// Ignore any zeroes.
			unsigned char ucNext;
			if (InStream.Read(&ucNext,1,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("Error: Unable to read through zeroes."));
				return false;
			}
			if (ulProcessedSize != 1)
			{
				m_pLog->AddLine(_T("Error: Unable to read through zeroes (size mismatch)."));
				return false;
			}

			while (ucNext == 0)
			{
				ulRead++;

				if (InStream.Read(&ucNext,1,&ulProcessedSize) != STREAM_OK)
				{
					m_pLog->AddLine(_T("Error: Unable to read through zeroes."));
					return false;
				}
				if (ulProcessedSize != 1)
				{
					m_pLog->AddLine(_T("Error: Unable to read through zeroes (size mismatch)."));
					return false;
				}
			}

			InStream.Seek(pParentNode->m_ulExtentLocation * ISO9660_SECTOR_SIZE + ulRead,CSeekInStream::SM_BEGIN);
		}

		return true;
	}

	bool CIso9660Reader::Read(CSeekInStream &InStream,unsigned long ulStartSector)
	{
		m_pLog->AddLine(_T("CIso9660Reader::Read"));

		// Seek to the start sector.
		//InStream.Seek(ulStartSector * ISO9660_SECTOR_SIZE,CSeekInStream::SM_BEGIN);

		// Seek to sector 16.
		InStream.Seek(ISO9660_SECTOR_SIZE/* << 4*/ * (16 + ulStartSector),CSeekInStream::SM_BEGIN);
		if (InStream.EOS())
		{
			m_pLog->AddLine(_T("  Error: Invalid ISO9660 file system."));
			return false;
		}

		unsigned long ulProcessedSize = 0;

		// Read the primary volume descriptor.
		tVolDescPrimary VolDescPrim;
		memset(&VolDescPrim,0,sizeof(tVolDescPrimary));
		if (InStream.Read(&VolDescPrim,sizeof(tVolDescPrimary),&ulProcessedSize) != STREAM_OK)
		{
			m_pLog->AddLine(_T("  Error: Unable to read ISO9660 file system."));
			return false;
		}
		if (ulProcessedSize != sizeof(tVolDescPrimary))
		{
			m_pLog->AddLine(_T("  Error: Unable to read ISO9660 primary volume descriptor."));
			return false;
		}

		// Validate primary volume descriptor.
		if (VolDescPrim.ucType != VOLDESCTYPE_PRIM_VOL_DESC)
		{
			m_pLog->AddLine(_T("  Error: Primary volume decsriptor not found at sector 16."));
			return false;
		}

		if (VolDescPrim.ucVersion != 1)
		{
			m_pLog->AddLine(_T("  Error: Bad primary volume descriptor version."));
			return false;
		}

		if (VolDescPrim.ucFileStructVer != 1)
		{
			m_pLog->AddLine(_T("  Error: Bad primary volume descriptor structure version."));
			return false;
		}

		if (memcmp(VolDescPrim.ucIdentifier,g_IdentCD,sizeof(VolDescPrim.ucIdentifier)))
		{
			m_pLog->AddLine(_T("  Error: Bad primary volume descriptor identifer."));
			return false;
		}

		// Search all other volume descriptors for a Joliet descriptor (we only search 99 maximum).
		tVolDescSuppl VolDescSuppl;
		bool bJoliet = false;
		for (unsigned int i = 0; i < 99; i++)
		{
			memset(&VolDescSuppl,0,sizeof(tVolDescSuppl));
			if (InStream.Read(&VolDescSuppl,sizeof(tVolDescSuppl),&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("  Error: Unable to read ISO9660 file system."));
				return false;
			}
			if (ulProcessedSize != sizeof(tVolDescSuppl))
			{
				m_pLog->AddLine(_T("  Error: Unable to read additional ISO9660 volume descriptor."));
				return false;
			}

			// Check if we found the terminator.
			if (VolDescSuppl.ucType == VOLDESCTYPE_VOL_DESC_SET_TERM)
				break;

			// Check if we found a supplementary volume descriptor.
			if (VolDescSuppl.ucType == VOLDESCTYPE_SUPPL_VOL_DESC)
			{
				// Check if Joliet.
				if (VolDescSuppl.ucEscapeSeq[0] == 0x25 &&
					VolDescSuppl.ucEscapeSeq[1] == 0x2F)
				{
					if (VolDescSuppl.ucEscapeSeq[2] == 0x45 ||
						VolDescSuppl.ucEscapeSeq[2] == 0x43 ||
						VolDescSuppl.ucEscapeSeq[2] == 0x40)
					{
						m_pLog->AddLine(_T("  Found Joliet file system extension."));
						bJoliet = true;
						break;
					}
				}
			}
		}

		// Obtain positions of interest.
		unsigned long ulRootExtentLoc;
		unsigned long ulRootExtentLen;

		if (bJoliet)
		{
			ulRootExtentLoc = Read733(VolDescSuppl.RootDirRecord.ucExtentLocation);
			ulRootExtentLen = Read733(VolDescSuppl.RootDirRecord.ucDataLen);
		}
		else
		{
			ulRootExtentLoc = Read733(VolDescPrim.RootDirRecord.ucExtentLocation);
			ulRootExtentLen = Read733(VolDescPrim.RootDirRecord.ucDataLen);
		}

		m_pLog->AddLine(_T("  Location of root directory extent: %u."),ulRootExtentLoc);
		m_pLog->AddLine(_T("  Length of root directory extent: %u."),ulRootExtentLen);

		if (m_pRootNode != NULL)
			delete m_pRootNode;

		m_pRootNode = new CIso9660TreeNode(NULL,NULL,ulRootExtentLoc,ulRootExtentLen,
			Read723(VolDescSuppl.RootDirRecord.ucVolSeqNumber),VolDescSuppl.RootDirRecord.ucFileFlags,
			VolDescSuppl.RootDirRecord.ucFileUnitSize,VolDescSuppl.RootDirRecord.ucInterleaveGapSize,
			VolDescSuppl.RootDirRecord.RecDateTime);

		std::vector<CIso9660TreeNode *> DirEntries;
		if (!ReadDirEntry(InStream,DirEntries,m_pRootNode,bJoliet))
		{
			m_pLog->AddLine(_T("  Error: Failed to read directory entry at sector: %u."),ulRootExtentLoc);
			return false;
		}

		while (DirEntries.size() > 0)
		{
			CIso9660TreeNode *pParentNode = DirEntries.back();
			DirEntries.pop_back();

			if (!ReadDirEntry(InStream,DirEntries,pParentNode,bJoliet))
			{
				m_pLog->AddLine(_T("  Error: Failed to read directory entry at sector: %u."),pParentNode->m_ulExtentLocation);
				return false;
			}
		}

		return true;
	}

	#ifdef _DEBUG
	void CIso9660Reader::PrintLocalTree(std::vector<std::pair<CIso9660TreeNode *,int> > &DirNodeStack,
		CIso9660TreeNode *pLocalNode,int iIndent)
	{
		std::vector<CIso9660TreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & DIRRECORD_FILEFLAG_DIRECTORY)
			{
				DirNodeStack.push_back(std::make_pair(*itFile,iIndent));
			}
			else
			{
				for (int i = 0; i < iIndent; i++)
					m_pLog->AddString(_T(" "));

				m_pLog->AddString(_T("<f>"));
				m_pLog->AddString((*itFile)->m_FileName.c_str());
				m_pLog->AddLine(_T(" (%u:%u)"),(*itFile)->m_ulExtentLocation,(*itFile)->m_ulExtentLength);
			}
		}
	}

	void CIso9660Reader::PrintTree()
	{
		if (m_pRootNode == NULL)
			return;

		CIso9660TreeNode *pCurNode = m_pRootNode;
		int iIndent = 0;

		m_pLog->AddLine(_T("CIso9660Reader::PrintTree"));
		m_pLog->AddLine(_T("  <root> (%I64d:%I64d)"),pCurNode->m_ulExtentLocation,pCurNode->m_ulExtentLength);

		std::vector<std::pair<CIso9660TreeNode *,int> > DirNodeStack;
		PrintLocalTree(DirNodeStack,pCurNode,4);

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			iIndent = DirNodeStack[DirNodeStack.size() - 1].second;

			DirNodeStack.pop_back();

			// Print the directory name.
			for (int i = 0; i < iIndent; i++)
				m_pLog->AddString(_T(" "));

			m_pLog->AddString(_T("<d>"));
			m_pLog->AddString(pCurNode->m_FileName.c_str());
			m_pLog->AddLine(_T(" (%I64d:%I64d)"),pCurNode->m_ulExtentLocation,pCurNode->m_ulExtentLength);

			PrintLocalTree(DirNodeStack,pCurNode,iIndent + 2);
		}
	}
#endif
};
