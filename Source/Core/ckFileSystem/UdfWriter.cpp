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
#include "Const.h"
#include "UdfWriter.h"
#include "Iso9660.h"

namespace ckFileSystem
{
	CUdfWriter::CUdfWriter(CLog *pLog,CSectorOutStream *pOutStream,
		CSectorManager *pSectorManager,CUdf *pUdf,bool bUseFileTimes) :
		m_pLog(pLog),m_pOutStream(pOutStream),m_pSectorManager(pSectorManager),
		m_pUdf(pUdf),m_bUseFileTimes(bUseFileTimes),m_uiPartLength(0)
	{
		memset(&m_MainVolDescSeqExtent,0,sizeof(tUdfExtentAd));
		memset(&m_ReserveVolDescSeqExtent,0,sizeof(tUdfExtentAd));

		// Get system time.
		GetLocalTime(&m_stImageCreate);
	}

	CUdfWriter::~CUdfWriter()
	{
	}

	void CUdfWriter::CalcLocalNodeLengths(std::vector<CFileTreeNode *> &DirNodeStack,
		CFileTreeNode *pLocalNode)
	{
		pLocalNode->m_uiUdfSize = 0;
		pLocalNode->m_uiUdfSize += BytesToSector(m_pUdf->CalcFileEntrySize());
		pLocalNode->m_uiUdfSize += BytesToSector(CalcIdentSize(pLocalNode));
		pLocalNode->m_uiUdfSizeTot = pLocalNode->m_uiUdfSize;

		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
				DirNodeStack.push_back(*itFile);
			else
			{
				(*itFile)->m_uiUdfSize = BytesToSector(m_pUdf->CalcFileEntrySize());
				(*itFile)->m_uiUdfSizeTot = (*itFile)->m_uiUdfSize;
			}
		}
	}

	void CUdfWriter::CalcNodeLengths(CFileTree &FileTree)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();

		std::vector<CFileTreeNode *> DirNodeStack;
		CalcLocalNodeLengths(DirNodeStack,pCurNode);

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1];
			DirNodeStack.pop_back();

			CalcLocalNodeLengths(DirNodeStack,pCurNode);
		}
	}

	unsigned __int64 CUdfWriter::CalcIdentSize(CFileTreeNode *pLocalNode)
	{
		unsigned __int64 uiTotalIdentSize = 0;
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			uiTotalIdentSize += m_pUdf->CalcFileIdentSize((*itFile)->m_FileName.c_str());
		}

		// Don't forget to add the '..' item to the total.
		uiTotalIdentSize += m_pUdf->CalcFileIdentParentSize();

		return uiTotalIdentSize;
	}

	/*
		FIXME: This function uses recursion, it should be converted to an
		iterative function to avoid the risk of running out of memory.
	*/
	unsigned __int64 CUdfWriter::CalcNodeSizeTotal(CFileTreeNode *pLocalNode)
	{
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			pLocalNode->m_uiUdfSizeTot += CalcNodeSizeTotal(*itFile);
		}

		return pLocalNode->m_uiUdfSizeTot;
	}

	/*
		FIXME: This function uses recursion, it should be converted to an
		iterative function to avoid the risk of running out of memory.
	*/
	unsigned __int64 CUdfWriter::CalcNodeLinksTotal(CFileTreeNode *pLocalNode)
	{
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			pLocalNode->m_uiUdfLinkTot += CalcNodeLinksTotal(*itFile);
		}

		return (pLocalNode->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY) ? 1 : 0;
	}

	/**
		Calculates the number of bytes needed to store the UDF partition.
	*/
	unsigned __int64 CUdfWriter::CalcParitionLength(CFileTree &FileTree)
	{
		// First wee need to calculate the individual sizes of each records.
		CalcNodeLengths(FileTree);

		// Calculate the number of directory links associated with each directory node.
		CalcNodeLinksTotal(FileTree.GetRoot());

		// The update the compelte tree (unfortunately recursively).
		return CalcNodeSizeTotal(FileTree.GetRoot());
	}

	bool CUdfWriter::WriteLocalParitionDir(std::deque<CFileTreeNode *> &DirNodeQueue,
		CFileTreeNode *pLocalNode,unsigned long &ulCurPartSec,unsigned __int64 &uiUniqueIdent)
	{
		unsigned long ulEntrySec = ulCurPartSec++;
		unsigned long ulIdentSec = ulCurPartSec;	// On folders the identifiers will follow immediately.

		// Calculate the size of all identifiers.
		unsigned __int64 uiTotalIdentSize = 0;
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			uiTotalIdentSize += m_pUdf->CalcFileIdentSize((*itFile)->m_FileName.c_str());
		}

		// Don't forget to add the '..' item to the total.
		uiTotalIdentSize += m_pUdf->CalcFileIdentParentSize();

		unsigned long ulNextEntrySec = ulIdentSec + BytesToSector(uiTotalIdentSize);

		// Get file modified dates.
		SYSTEMTIME stCreateTime,stAccessTime,stWriteTime;
		if (!fs_getdirtime(pLocalNode->m_FileFullPath.c_str(),stCreateTime,stAccessTime,stWriteTime))
			stCreateTime = stAccessTime = stWriteTime = m_stImageCreate;

		// The current folder entry.
		if (!m_pUdf->WriteFileEntry(m_pOutStream,ulEntrySec,true,(unsigned short)pLocalNode->m_uiUdfLinkTot + 1,
			uiUniqueIdent,ulIdentSec,uiTotalIdentSize,stAccessTime,stWriteTime,stCreateTime))
		{
			return false;
		}

		// Unique identifiers 0-15 are reserved for Macintosh implementations.
		if (uiUniqueIdent == 0)
			uiUniqueIdent = 16;
		else
			uiUniqueIdent++;

		// The '..' item.
		unsigned long ulParentEntrySec = pLocalNode->GetParent() == NULL ? ulEntrySec : pLocalNode->GetParent()->m_ulUdfPartLoc;
		if (!m_pUdf->WriteFileIdentParent(m_pOutStream,ulCurPartSec,ulParentEntrySec))
			return false;

		// Keep track on how many bytes we have in our sector.
		unsigned long ulSectorBytes = m_pUdf->CalcFileIdentParentSize();

		std::vector<CFileTreeNode *> TempStack;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			// Push the item to the temporary stack.
			TempStack.push_back(*itFile);

			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				if (!m_pUdf->WriteFileIdent(m_pOutStream,ulCurPartSec,ulNextEntrySec,true,(*itFile)->m_FileName.c_str()))
					return false;

				(*itFile)->m_ulUdfPartLoc = ulNextEntrySec;	// Remember where this entry was stored.

				ulNextEntrySec += (unsigned long)(*itFile)->m_uiUdfSizeTot;
			}
			else
			{
				if (!m_pUdf->WriteFileIdent(m_pOutStream,ulCurPartSec,ulNextEntrySec,false,(*itFile)->m_FileName.c_str()))
					return false;

				(*itFile)->m_ulUdfPartLoc = ulNextEntrySec;	// Remember where this entry was stored.

				ulNextEntrySec += (unsigned long)(*itFile)->m_uiUdfSizeTot;
			}

			ulSectorBytes += m_pUdf->CalcFileIdentSize((*itFile)->m_FileName.c_str());
			if (ulSectorBytes >= UDF_SECTOR_SIZE)
			{
				ulCurPartSec++;
				ulSectorBytes -= UDF_SECTOR_SIZE;
			}
		}

		// Insert the elements from the temporary stack into the queue.
		std::vector<CFileTreeNode *>::const_reverse_iterator itStackNode;
		for (itStackNode = TempStack.rbegin() ; itStackNode != TempStack.rend(); itStackNode++)
			DirNodeQueue.push_front(*itStackNode);

		// Pad to the next sector.
		m_pOutStream->PadSector();

		if (ulSectorBytes > 0)
			ulCurPartSec++;

		return true;
	}

	bool CUdfWriter::WritePartitionEntries(CFileTree &FileTree)
	{
		// We start at partition sector 1, sector 0 is the parition anchor descriptor.
		unsigned long ulCurPartSec = 1;

		// We start with unique identifier 0 (which is reserved for root) and
		// increase it for every file or folder added.
		unsigned __int64 uiUniqueIdent = 0;

		CFileTreeNode *pCurNode = FileTree.GetRoot();
		pCurNode->m_ulUdfPartLoc = ulCurPartSec;

		std::deque<CFileTreeNode *> DirNodeStack;
		if (!WriteLocalParitionDir(DirNodeStack,pCurNode,ulCurPartSec,uiUniqueIdent))
			return false;

		unsigned long ulTemp = 0;

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack.front();
			DirNodeStack.pop_front();

#ifdef _DEBUG
			if (pCurNode->m_ulUdfPartLoc != ulCurPartSec)
			{
				m_pLog->AddLine(_T("Invalid location for \"%s\" in UDF file system. Proposed position %u verus actual position %u."),
					pCurNode->m_FileFullPath.c_str(),pCurNode->m_ulUdfPartLoc,ulCurPartSec);
			}
#endif

			if (pCurNode->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				if (!WriteLocalParitionDir(DirNodeStack,pCurNode,ulCurPartSec,/*ulParentEntrySec,*/uiUniqueIdent))
					return false;
			}
			else
			{
				ulTemp++;

				// Get file modified dates.
				SYSTEMTIME stCreateTime,stAccessTime,stWriteTime;
				if (m_bUseFileTimes && !fs_gettime(pCurNode->m_FileFullPath.c_str(),stCreateTime,stAccessTime,stWriteTime))
					stCreateTime = stAccessTime = stWriteTime = m_stImageCreate;

				if (!m_pUdf->WriteFileEntry(m_pOutStream,ulCurPartSec++,false,1,
					uiUniqueIdent,(unsigned long)pCurNode->m_uiDataPosNormal - 257,pCurNode->m_uiFileSize,
					stAccessTime,stWriteTime,stCreateTime))
				{
					return false;
				}

				// Unique identifiers 0-15 are reserved for Macintosh implementations.
				if (uiUniqueIdent == 0)
					uiUniqueIdent = UDF_UNIQUEIDENT_MIN;
				else
					uiUniqueIdent++;
			}
		}

		return true;
	}

	int CUdfWriter::AllocateHeader()
	{
		m_pSectorManager->AllocateBytes(this,SR_INITIALDESCRIPTORS,m_pUdf->GetVolDescInitialSize());
		return RESULT_OK;
	}

	int CUdfWriter::AllocatePartition(CFileTree &FileTree)
	{
		// Allocate everything up to sector 258.
		m_pSectorManager->AllocateSectors(this,SR_MAINDESCRIPTORS,258 - m_pSectorManager->GetNextFree());

		// Allocate memory for the file set contents.
		m_uiPartLength = CalcParitionLength(FileTree);

		m_pSectorManager->AllocateSectors(this,SR_FILESETCONTENTS,m_uiPartLength);

		return RESULT_OK;
	}

	int CUdfWriter::WriteHeader()
	{
		if (!m_pUdf->WriteVolDescInitial(m_pOutStream))
		{
			m_pLog->AddLine(_T("  Error: Failed to write initial UDF volume descriptors."));
			return RESULT_FAIL;
		}

		return RESULT_OK;
	}

	int CUdfWriter::WritePartition(CFileTree &FileTree)
	{
		if (m_uiPartLength == 0)
		{
			m_pLog->AddLine(_T("  Error: Cannot write UDF parition since no space has been reserved for it."));
			return RESULT_FAIL;
		}

		if (m_uiPartLength > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: UDF partition is too large (%I64d sectors)."),m_uiPartLength);
			return RESULT_FAIL;
		}

		if (m_pSectorManager->GetStart(this,SR_MAINDESCRIPTORS) > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: Error during sector space allocation. Start of UDF main descriptors %I64d."),
				m_pSectorManager->GetStart(this,SR_MAINDESCRIPTORS));
			return RESULT_FAIL;
		}

		if (m_pSectorManager->GetDataLength() > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: File data too large (%I64d sectors) for UDF."),m_pSectorManager->GetDataLength());
			return RESULT_FAIL;
		}

		// Used for padding.
		char szTemp[1] = { 0 };
		unsigned long ulProcessedSize = 0;

		// Parition size = the partition size calculated above + the set descriptor + the data length.
		unsigned long ulUdfCurSec = (unsigned long)m_pSectorManager->GetStart(this,SR_MAINDESCRIPTORS);
		unsigned long ulPartLength = (unsigned long)m_uiPartLength + 1 + (unsigned long)m_pSectorManager->GetDataLength();

		/*TCHAR szTemp2[64];
		lsprintf(szTemp2,_T("%u %u %u"),ulUdfCurSec,m_uiPartLength,ulUdfCurSec + m_uiPartLength);
		MessageBox(NULL,szTemp2,_T(""),MB_OK);*/

		// Assign a unique identifier that's larger than any unique identifier of a
		// file entry + 16 for the reserved numbers.
		unsigned __int64 uiUniqueIdent = FileTree.GetDirCount() + FileTree.GetFileCount() + 1 + UDF_UNIQUEIDENT_MIN;

		tUdfExtentAd IntegritySeqExtent;
		IntegritySeqExtent.ulExtentLen = UDF_SECTOR_SIZE;
		IntegritySeqExtent.ulExtentLoc = ulUdfCurSec;

		if (!m_pUdf->WriteVolDescLogIntegrity(m_pOutStream,ulUdfCurSec,FileTree.GetFileCount(),
			FileTree.GetDirCount() + 1,ulPartLength,uiUniqueIdent,m_stImageCreate))
		{
			m_pLog->AddLine(_T("  Error: Failed to write UDF logical integrity descriptor."));
			return RESULT_FAIL;
		}
		ulUdfCurSec++;			

		// 6 volume descriptors. But minimum length is 16 so we pad with empty sectors.
		m_MainVolDescSeqExtent.ulExtentLen = m_ReserveVolDescSeqExtent.ulExtentLen = 16 * ISO9660_SECTOR_SIZE;

		// We should write the set of volume descriptors twice.
		for (unsigned int i = 0; i < 2; i++)
		{
			// Remember the start of the volume descriptors.
			if (i == 0)
				m_MainVolDescSeqExtent.ulExtentLoc = ulUdfCurSec;
			else
				m_ReserveVolDescSeqExtent.ulExtentLoc = ulUdfCurSec;

			unsigned long ulVolDescSeqNum = 0;
			if (!m_pUdf->WriteVolDescPrimary(m_pOutStream,ulVolDescSeqNum++,ulUdfCurSec,m_stImageCreate))
			{
				m_pLog->AddLine(_T("  Error: Failed to write UDF primary volume descriptor."));
				return RESULT_FAIL;
			}
			ulUdfCurSec++;

			if (!m_pUdf->WriteVolDescImplUse(m_pOutStream,ulVolDescSeqNum++,ulUdfCurSec))
			{
				m_pLog->AddLine(_T("  Error: Failed to write UDF implementation use descriptor."));
				return RESULT_FAIL;
			}
			ulUdfCurSec++;

			if (!m_pUdf->WriteVolDescPartition(m_pOutStream,ulVolDescSeqNum++,ulUdfCurSec,257,ulPartLength))
			{
				m_pLog->AddLine(_T("  Error: Failed to write UDF partition descriptor."));
				return RESULT_FAIL;
			}
			ulUdfCurSec++;

			if (!m_pUdf->WriteVolDescLogical(m_pOutStream,ulVolDescSeqNum++,ulUdfCurSec,IntegritySeqExtent))
			{
				m_pLog->AddLine(_T("  Error: Failed to write UDF logical partition descriptor."));
				return RESULT_FAIL;
			}
			ulUdfCurSec++;

			if (!m_pUdf->WriteVolDescUnalloc(m_pOutStream,ulVolDescSeqNum++,ulUdfCurSec))
			{
				m_pLog->AddLine(_T("  Error: Failed to write UDF unallocated space descriptor."));
				return RESULT_FAIL;
			}
			ulUdfCurSec++;

			if (!m_pUdf->WriteVolDescTerm(m_pOutStream,ulUdfCurSec))
			{
				m_pLog->AddLine(_T("  Error: Failed to write UDF descriptor terminator."));
				return RESULT_FAIL;
			}
			ulUdfCurSec++;

			// According to UDF 1.02 standard each volume descriptor
			// sequence extent must contain atleast 16 sectors. Because of
			// this we need to add 10 empty sectors.
			for (int j = 0; j < 10; j++)
			{
				for (unsigned long i = 0; i < ISO9660_SECTOR_SIZE; i++)
					m_pOutStream->Write(szTemp,1,&ulProcessedSize);
				ulUdfCurSec++;
			}
		}

		// Allocate everything until sector 256 with empty sectors.
		while (ulUdfCurSec < 256)
		{
			for (unsigned long i = 0; i < ISO9660_SECTOR_SIZE; i++)
				m_pOutStream->Write(szTemp,1,&ulProcessedSize);
			ulUdfCurSec++;
		}

		// At sector 256 write the first anchor volume descriptor pointer.
		if (!m_pUdf->WriteAnchorVolDescPtr(m_pOutStream,ulUdfCurSec,m_MainVolDescSeqExtent,m_ReserveVolDescSeqExtent))
		{
			m_pLog->AddLine(_T("  Error: Failed to write anchor volume descriptor pointer."));
			return RESULT_FAIL;
		}
		ulUdfCurSec++;

		// The file set descriptor is the first entry in the partition, hence the logical block address 0.
		// The root is located directly after this descriptor, hence the location 1.
		if (!m_pUdf->WriteFileSetDesc(m_pOutStream,0,1,m_stImageCreate))
		{
			m_pLog->AddLine(_T("  Error: Failed to write file set descriptor."));
			return RESULT_FAIL;
		}

		if (!WritePartitionEntries(FileTree))
		{
			m_pLog->AddLine(_T("  Error: Failed to write file UDF partition."));
			return RESULT_FAIL;
		}

		return RESULT_OK;
	}

	int CUdfWriter::WriteTail()
	{
		unsigned __int64 uiLastDataSector = m_pSectorManager->GetDataStart() +
			m_pSectorManager->GetDataLength();
		if (uiLastDataSector > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: File data too large, last data sector is %I64d."),uiLastDataSector);
			return RESULT_FAIL;
		}

		// Finally write the 2nd and last anchor volume descriptor pointer.
		if (!m_pUdf->WriteAnchorVolDescPtr(m_pOutStream,(unsigned long)uiLastDataSector,
			m_MainVolDescSeqExtent,m_ReserveVolDescSeqExtent))
		{
			m_pLog->AddLine(_T("  Error: Failed to write anchor volume descriptor pointer."));
			return RESULT_FAIL;
		}

		m_pOutStream->PadSector();
		return RESULT_OK;
	}
};
