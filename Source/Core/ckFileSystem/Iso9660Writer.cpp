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
#include "../../Common/FileStream.h"
#include "../../Common/FileManager.h"
#include "StringTable.h"
#include "Const.h"
#include "Iso9660Writer.h"

namespace ckFileSystem
{
	CIso9660Writer::CIso9660Writer(CLog *pLog,CSectorOutStream *pOutStream,
		CSectorManager *pSectorManager,CIso9660 *pIso9660,CJoliet *pJoliet,
		CElTorito *pElTorito,bool bUseFileTimes,bool bUseJoliet) :
		m_pLog(pLog),m_pOutStream(pOutStream),m_pSectorManager(pSectorManager),
		m_pIso9660(pIso9660),m_pJoliet(pJoliet),m_pElTorito(pElTorito),
		m_bUseFileTimes(bUseFileTimes),m_bUseJoliet(bUseJoliet),
		m_uiPathTableSizeNormal(0),m_uiPathTableSizeJoliet(0)
	{
		// Get system time.
		GetLocalTime(&m_stImageCreate);
	}

	CIso9660Writer::~CIso9660Writer()
	{
	}

	bool CIso9660Writer::CalcPathTableSize(CFileSet &Files,bool bJolietTable,
		unsigned __int64 &uiPathTableSize,CProgressEx &Progress)
	{
		// Root record + 1 padding byte since the root record size is odd.
		uiPathTableSize = sizeof(tPathTableRecord) + 1;

		// Write all other path table records.
		std::set<tstring> PathDirList;		// To help keep track of which records that have already been counted.
		tstring PathBuffer,CurDirName,InternalPath;

		// Set to true of we have found that the directory structure is to deep.
		// This variable is needed so that the warning message will only be printed
		// once.
		bool bFoundDeep = false;

		CFileSet::const_iterator itFile;
		for (itFile = Files.begin(); itFile != Files.end(); itFile++)
		{
			// We're do not have to add the root record once again.
			int iLevel = CFileComparator::Level(*itFile);
			if (iLevel <= 1)
				continue;

			if (iLevel > m_pIso9660->GetMaxDirLevel())
			{
				// Print the message only once.
				if (!bFoundDeep)
				{
					m_pLog->AddLine(_T("  Warning: The directory structure is deeper than %d levels. Deep files and folders will be ignored."),
						m_pIso9660->GetMaxDirLevel());
					Progress.AddLogEntry(CProgressEx::LT_WARNING,g_StringTable.GetString(WARNING_FSDIRLEVEL),m_pIso9660->GetMaxDirLevel());
					bFoundDeep = true;
				}

				m_pLog->AddLine(_T("  Skipping: %s."),itFile->m_InternalPath.c_str());
				Progress.AddLogEntry(CProgressEx::LT_WARNING,g_StringTable.GetString(WARNING_SKIPFILE),
					itFile->m_InternalPath.c_str());
				continue;
			}

			// We're only interested in directories.
			if (!(itFile->m_ucFlags & CFileDescriptor::FLAG_DIRECTORY))
				continue;

			// Make sure that the path to the current file or folder exists.
			size_t iPrevDelim = 0;

			// If the file is a directory, append a slash so it will be parsed as a
			// directory in the loop below.
			InternalPath = itFile->m_InternalPath;
			InternalPath.push_back('/');

			PathBuffer.erase();
			PathBuffer = _T("/");

			for (size_t i = 0; i < InternalPath.length(); i++)
			{
				if (InternalPath[i] == '/')
				{
					if (i > (iPrevDelim + 1))
					{
						// Obtain the name of the current directory.
						CurDirName.erase();
						for (size_t j = iPrevDelim + 1; j < i; j++)
							CurDirName.push_back(InternalPath[j]);

						PathBuffer += CurDirName;
						PathBuffer += _T("/");

						// The path does not exist, create it.
						if (PathDirList.find(PathBuffer) == PathDirList.end())
						{
							unsigned char ucNameLen = bJolietTable ?
								m_pJoliet->CalcFileNameLen(CurDirName.c_str(),true) << 1 : 
								m_pIso9660->CalcFileNameLen(CurDirName.c_str(),true);

							unsigned char ucPathTableRecLen = sizeof(tPathTableRecord) + ucNameLen - 1;

							// If the record length is not even padd it with a 0 byte.
							if (ucPathTableRecLen % 2 == 1)
								ucPathTableRecLen++;

							PathDirList.insert(PathBuffer);

							// Update the path table length.
							uiPathTableSize += ucPathTableRecLen;
						}
					}

					iPrevDelim = i;
				}
			}
		}

		return true;
	}

	/*
		Calculates the size of a directory entry in sectors. This size does not include the size
		of the extend data of the directory contents.
	*/
	bool CIso9660Writer::CalcLocalDirEntryLength(CFileTreeNode *pLocalNode,bool bJoliet,
		int iLevel,unsigned long &ulDirLength)
	{
		ulDirLength = 0;

		// The number of bytes of data in the current sector.
		// Each directory always includes '.' and '..'.
		unsigned long ulDirSecData = sizeof(tDirRecord) << 1;

		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				// Validate directory level.
				if (iLevel >= m_pIso9660->GetMaxDirLevel())
					continue;
			}

			// Validate file size.
			if ((*itFile)->m_uiFileSize > ISO9660_MAX_EXTENT_SIZE && !m_pIso9660->AllowsFragmentation())
				continue;

			// Calculate the number of times this record will be written. It
			// will be larger than one when using multi-extent.
			unsigned long ulFactor = 1;
			unsigned __int64 uiExtentRemain = (*itFile)->m_uiFileSize;
			while (uiExtentRemain > ISO9660_MAX_EXTENT_SIZE)
			{
				uiExtentRemain -= ISO9660_MAX_EXTENT_SIZE;
				ulFactor++;
			}

			bool bIsFolder = (*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY;

			unsigned char ucNameLen;
			if (bJoliet)
				ucNameLen = m_pJoliet->CalcFileNameLen((*itFile)->m_FileName.c_str(),bIsFolder) << 1;
			else
				ucNameLen = m_pIso9660->CalcFileNameLen((*itFile)->m_FileName.c_str(),bIsFolder);

			unsigned char ucCurRecSize = sizeof(tDirRecord) + ucNameLen - 1;

			// If the record length is not even padd it with a 0 byte.
			if (ucCurRecSize % 2 == 1)
				ucCurRecSize++;

			if (ulDirSecData + (ucCurRecSize * ulFactor) > ISO9660_SECTOR_SIZE)
			{
				ulDirSecData = ucCurRecSize * ulFactor;
				ulDirLength++;
			}
			else
			{
				ulDirSecData += ucCurRecSize * ulFactor;
			}
		}

		if (ulDirSecData != 0)
			ulDirLength++;

		return true;
	}

	bool CIso9660Writer::CalcLocalDirEntriesLength(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
		CFileTreeNode *pLocalNode,int iLevel,unsigned __int64 &uiSecOffset,CProgressEx &Progress)
	{
		unsigned long ulDirLenNormal = 0;
		if (!CalcLocalDirEntryLength(pLocalNode,false,iLevel,ulDirLenNormal))
			return false;

		unsigned long ulDirLenJoliet = 0;
		if (m_bUseJoliet && !CalcLocalDirEntryLength(pLocalNode,true,iLevel,ulDirLenJoliet))
			return false;

		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				// Validate directory level.
				if (iLevel >= m_pIso9660->GetMaxDirLevel())
					continue;
				else
					DirNodeStack.push_back(std::make_pair(*itFile,iLevel + 1));
			}
		}

		// We now know how much space is needed to store the current directory record.
		pLocalNode->m_uiDataLenNormal = ulDirLenNormal * ISO9660_SECTOR_SIZE;
		pLocalNode->m_uiDataLenJoliet = ulDirLenJoliet * ISO9660_SECTOR_SIZE;

		pLocalNode->m_uiDataPosNormal = uiSecOffset;
		uiSecOffset += ulDirLenNormal;
		pLocalNode->m_uiDataPosJoliet = uiSecOffset;
		uiSecOffset += ulDirLenJoliet;

		return true;
	}

	bool CIso9660Writer::CalcDirEntriesLength(CFileTree &FileTree,CProgressEx &Progress,
		unsigned __int64 uiStartSector,unsigned __int64 &uiLength)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();
		unsigned __int64 uiSecOffset = uiStartSector;

		std::vector<std::pair<CFileTreeNode *,int> > DirNodeStack;
		if (!CalcLocalDirEntriesLength(DirNodeStack,pCurNode,0,uiSecOffset,Progress))
			return false;

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			int iLevel = DirNodeStack[DirNodeStack.size() - 1].second;
			DirNodeStack.pop_back();

			if (!CalcLocalDirEntriesLength(DirNodeStack,pCurNode,iLevel,uiSecOffset,Progress))
				return false;
		}

		uiLength = uiSecOffset - uiStartSector;
		return true;
	}

	bool CIso9660Writer::WritePathTable(CFileSet &Files,CFileTree &FileTree,
		bool bJolietTable,bool bMSBF,CProgressEx &Progress)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();

		tPathTableRecord RootRecord,PathRecord;
		memset(&RootRecord,0,sizeof(RootRecord));

		RootRecord.ucDirIdentifierLen = 1;					// Always 1 for the root record.
		RootRecord.ucExtAttrRecordLen = 0;

		if (bJolietTable)
			Write73(RootRecord.ucExtentLocation,(unsigned long)pCurNode->m_uiDataPosJoliet,bMSBF);	// Start sector of extent data.
		else
			Write73(RootRecord.ucExtentLocation,(unsigned long)pCurNode->m_uiDataPosNormal,bMSBF);	// Start sector of extent data.

		Write72(RootRecord.ucParentDirNumber,0x01,bMSBF);	// The root has itself as parent.
		RootRecord.ucDirIdentifier[0] = 0;					// The file name is set to zero.

		// Write the root record.
		unsigned long ulProcessedSize = 0;
		if (m_pOutStream->Write(&RootRecord,sizeof(RootRecord),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(RootRecord))
			return false;

		// We need to pad the root record since it's size is otherwise odd.
		if (m_pOutStream->Write(RootRecord.ucDirIdentifier,1,&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != 1)
			return false;

		// Write all other path table records.
		std::map<tstring,unsigned short> PathDirNumMap;
		tstring PathBuffer,CurDirName,InternalPath;

		// Counters for all records.
		unsigned short usRecordNumber = 2;	// Root has number 1.

		CFileSet::const_iterator itFile;
		for (itFile = Files.begin(); itFile != Files.end(); itFile++)
		{
			// We're do not have to add the root record once again.
			int iLevel = CFileComparator::Level(*itFile);
			if (iLevel <= 1)
				continue;

			if (iLevel > m_pIso9660->GetMaxDirLevel())
				continue;

			// We're only interested in directories.
			if (!(itFile->m_ucFlags & CFileDescriptor::FLAG_DIRECTORY))
				continue;

			// Locate the node in the file tree.
			pCurNode = FileTree.GetNodeFromPath(*itFile);

			// Make sure that the path to the current file or folder exists.
			size_t iPrevDelim = 0;

			// If the file is a directory, append a slash so it will be parsed as a
			// directory in the loop below.
			InternalPath = itFile->m_InternalPath;
			InternalPath.push_back('/');

			PathBuffer.erase();
			PathBuffer = _T("/");

			unsigned short usParent = 1;	// Start from root.

			for (size_t i = 0; i < InternalPath.length(); i++)
			{
				if (InternalPath[i] == '/')
				{
					if (i > (iPrevDelim + 1))
					{
						// Obtain the name of the current directory.
						CurDirName.erase();
						for (size_t j = iPrevDelim + 1; j < i; j++)
							CurDirName.push_back(InternalPath[j]);

						PathBuffer += CurDirName;
						PathBuffer += _T("/");

						// The path does not exist, create it.
						if (PathDirNumMap.find(PathBuffer) == PathDirNumMap.end())
						{
							memset(&PathRecord,0,sizeof(PathRecord));

							unsigned char ucNameLen;
							unsigned char szFileName[ISO9660WRITER_FILENAME_BUFFER_SIZE];	// Large enough for both level 1, 2 and even Joliet.
							if (bJolietTable)
								ucNameLen = m_pJoliet->WriteFileName(szFileName,CurDirName.c_str(),true) << 1;
							else
								ucNameLen = m_pIso9660->WriteFileName(szFileName,CurDirName.c_str(),true);

							// If the record length is not even padd it with a 0 byte.
							bool bPadByte = false;
							if (ucNameLen % 2 == 1)
								bPadByte = true;

							PathRecord.ucDirIdentifierLen = ucNameLen;
							PathRecord.ucExtAttrRecordLen = 0;

							if (bJolietTable)
								Write73(PathRecord.ucExtentLocation,(unsigned long)pCurNode->m_uiDataPosJoliet,bMSBF);
							else
								Write73(PathRecord.ucExtentLocation,(unsigned long)pCurNode->m_uiDataPosNormal,bMSBF);

							Write72(PathRecord.ucParentDirNumber,usParent,bMSBF);
							PathRecord.ucDirIdentifier[0] = 0;

							PathDirNumMap[PathBuffer] = usParent = usRecordNumber++;

							// Write the record.
							if (m_pOutStream->Write(&PathRecord,sizeof(PathRecord) - 1,&ulProcessedSize) != STREAM_OK)
								return false;
							if (ulProcessedSize != sizeof(PathRecord) - 1)
								return false;

							if (m_pOutStream->Write(szFileName,ucNameLen,&ulProcessedSize) != STREAM_OK)
								return false;
							if (ulProcessedSize != ucNameLen)
								return false;

							// Pad if necessary.
							if (bPadByte)
							{
								char szTemp[1] = { 0 };
								if (m_pOutStream->Write(szTemp,1,&ulProcessedSize) != STREAM_OK)
									return false;
								if (ulProcessedSize != 1)
									return false;
							}
						}
						else
						{
							usParent = PathDirNumMap[PathBuffer];
						}
					}

					iPrevDelim = i;
				}
			}
		}

		if (m_pOutStream->GetAllocated() != 0)
			m_pOutStream->PadSector();

		return true;
	}

	bool CIso9660Writer::WriteSysDirectory(CFileTreeNode *pParent,eSysDirType Type,
		unsigned long ulDataPos)
	{
		tDirRecord DirRecord;
		memset(&DirRecord,0,sizeof(DirRecord));

		DirRecord.ucDirRecordLen = 0x22;
		Write733(DirRecord.ucExtentLocation,ulDataPos);
		Write733(DirRecord.ucDataLen,ISO9660_SECTOR_SIZE);
		MakeDateTime(m_stImageCreate,DirRecord.RecDateTime);
		DirRecord.ucFileFlags = DIRRECORD_FILEFLAG_DIRECTORY;
		Write723(DirRecord.ucVolSeqNumber,0x01);	// The directory is on the first volume set.
		DirRecord.ucFileIdentifierLen = 1;
		DirRecord.ucFileIdentifier[0] = Type == TYPE_CURRENT ? 0 : 1;

		unsigned long ulProcessedSize;
		if (m_pOutStream->Write(&DirRecord,sizeof(DirRecord),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(DirRecord))
			return false;

		return true;
	}

	bool CIso9660Writer::ValidateTreeNode(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
		CFileTreeNode *pNode,int iLevel)
	{
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pNode->m_Children.begin(); itFile !=
			pNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				if (iLevel >= m_pIso9660->GetMaxDirLevel())
					return false;
				else
					DirNodeStack.push_back(std::make_pair(*itFile,iLevel + 1));
			}
		}

		return true;
	}

	/*
		Returns true if the specified file tree conforms to the configuration
		used to create the disc image.
	*/
	bool CIso9660Writer::ValidateTree(CFileTree &FileTree)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();

		std::vector<std::pair<CFileTreeNode *,int> > DirNodeStack;
		if (!ValidateTreeNode(DirNodeStack,pCurNode,1))
			return false;

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			int iLevel = DirNodeStack[DirNodeStack.size() - 1].second;
			DirNodeStack.pop_back();

			if (!ValidateTreeNode(DirNodeStack,pCurNode,iLevel))
				return false;
		}

		return true;
	}

	int CIso9660Writer::AllocateHeader()
	{
		// Allocate volume descriptor.
		unsigned long ulVolDescSize = sizeof(tVolDescPrimary) + sizeof(tVolDescSetTerm);
		if (m_pElTorito->GetBootImageCount() > 0)
			ulVolDescSize += sizeof(tVolDescElToritoRecord);
		if (m_pIso9660->HasVolDescSuppl())
			ulVolDescSize += sizeof(tVolDescSuppl);
		if (m_bUseJoliet)
			ulVolDescSize += sizeof(tVolDescSuppl);

		m_pSectorManager->AllocateBytes(this,SR_DESCRIPTORS,ulVolDescSize);

		// Allocate boot catalog and data.
		unsigned __int64 uiBootCatSize = 0;
		if (m_pElTorito->GetBootImageCount() > 0)
		{
			m_pSectorManager->AllocateBytes(this,SR_BOOTCATALOG,m_pElTorito->GetBootCatSize());
			m_pSectorManager->AllocateBytes(this,SR_BOOTDATA,m_pElTorito->GetBootDataSize());
		}

		return RESULT_OK;
	}

	int CIso9660Writer::AllocatePathTables(CProgressEx &Progress,CFileSet &Files)
	{
		// Calculate path table sizes.
		m_uiPathTableSizeNormal = 0;
		if (!CalcPathTableSize(Files,false,m_uiPathTableSizeNormal,Progress))
		{
			m_pLog->AddLine(_T("  Error: Unable to calculate path table size."));
			return RESULT_FAIL;
		}

		m_uiPathTableSizeJoliet = 0;
		if (m_bUseJoliet && !CalcPathTableSize(Files,true,m_uiPathTableSizeJoliet,Progress))
		{
			m_pLog->AddLine(_T("  Error: Unable to calculate Joliet path table size."));
			return RESULT_FAIL;
		}

		if (m_uiPathTableSizeNormal > 0xFFFFFFFF || m_uiPathTableSizeJoliet > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: The path table is too large, %I64d and %I64d bytes."),
				m_uiPathTableSizeNormal,m_uiPathTableSizeJoliet);
			Progress.AddLogEntry(CProgressEx::LT_ERROR,g_StringTable.GetString(ERROR_PATHTABLESIZE));
			return RESULT_FAIL;
		}

		m_pSectorManager->AllocateBytes(this,SR_PATHTABLE_NORMAL_L,m_uiPathTableSizeNormal);
		m_pSectorManager->AllocateBytes(this,SR_PATHTABLE_NORMAL_M,m_uiPathTableSizeNormal);
		m_pSectorManager->AllocateBytes(this,SR_PATHTABLE_JOLIET_L,m_uiPathTableSizeJoliet);
		m_pSectorManager->AllocateBytes(this,SR_PATHTABLE_JOLIET_M,m_uiPathTableSizeJoliet);

		return RESULT_OK;
	}

	int CIso9660Writer::AllocateDirEntries(CFileTree &FileTree,CProgressEx &Progress)
	{
		unsigned __int64 uiDirEntriesLen = 0;
		if (!CalcDirEntriesLength(FileTree,Progress,m_pSectorManager->GetNextFree(),uiDirEntriesLen))
			return RESULT_FAIL;

		m_pSectorManager->AllocateSectors(this,SR_DIRENTRIES,uiDirEntriesLen);

		m_pLog->AddLine(_T("  Allocated directory entries %I64d sectors."),uiDirEntriesLen);
		return RESULT_OK;
	}

	int CIso9660Writer::WriteHeader(CFileSet &Files,CFileTree &FileTree,CProgressEx &Progress)
	{
		// Make sure that everything has been allocated.
		if (m_uiPathTableSizeNormal == 0)
		{
			m_pLog->AddLine(_T("  Error: Memory for ISO9660 path table has not been allocated."));
			return RESULT_FAIL;
		}

		// Calculate boot catalog and image data.
		if (m_pElTorito->GetBootImageCount() > 0)
		{
			unsigned __int64 uiBootDataSector = m_pSectorManager->GetStart(this,SR_BOOTDATA);
			unsigned __int64 uiBootDataLength = BytesToSector64(m_pElTorito->GetBootDataSize());
			unsigned __int64 uiBootDataEnd = uiBootDataSector + uiBootDataLength;

			if (uiBootDataSector > 0xFFFFFFFF || uiBootDataEnd > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Error: Invalid boot data sector range (%I64d to %I64d)."),
					uiBootDataSector,uiBootDataEnd);
				return RESULT_FAIL;
			}

			if (!m_pElTorito->CalculateFileSysData(uiBootDataSector,uiBootDataEnd))
				return RESULT_FAIL;
		}

		unsigned long ulPosPathTableNormalL = (unsigned long)m_pSectorManager->GetStart(this,SR_PATHTABLE_NORMAL_L);
		unsigned long ulPosPathTableNormalM = (unsigned long)m_pSectorManager->GetStart(this,SR_PATHTABLE_NORMAL_M);
		unsigned long ulPosPathTableJolietL = (unsigned long)m_pSectorManager->GetStart(this,SR_PATHTABLE_JOLIET_L);
		unsigned long ulPosPathTableJolietM = (unsigned long)m_pSectorManager->GetStart(this,SR_PATHTABLE_JOLIET_M);

		// Print the sizes.
		/*m_pLog->AddLine(_T("  Sizes: %I64d, %I64d, %I64d, %d."),uiPathTableSizeNormal,uiPathTableSizeJoliet,
			uiBootCatSize,ulVolDescSize);
		m_pLog->AddLine(_T("  Locations: %I64d, %d, %d, %d, %d, %d."),uiPosBootCat,ulPosPathTableNormalL,
			ulPosPathTableNormalM,ulPosPathTableJolietL,ulPosPathTableJolietM,ulRootExtentLoc);*/

		unsigned __int64 uiDirEntriesSector = m_pSectorManager->GetStart(this,SR_DIRENTRIES);
		unsigned __int64 uiFileDataEndSector = m_pSectorManager->GetDataStart() + m_pSectorManager->GetDataLength();

		if (uiDirEntriesSector > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: Invalid start sector of directory entries (%I64d)."),uiDirEntriesSector);
			return RESULT_FAIL;
		}

		if (!m_pIso9660->WriteVolDescPrimary(m_pOutStream,m_stImageCreate,(unsigned long)uiFileDataEndSector,
				(unsigned long)m_uiPathTableSizeNormal,ulPosPathTableNormalL,ulPosPathTableNormalM,
				(unsigned long)uiDirEntriesSector,(unsigned long)FileTree.GetRoot()->m_uiDataLenNormal))
		{
			return RESULT_FAIL;
		}

		// Write the El Torito boot record at sector 17 if necessary.
		if (m_pElTorito->GetBootImageCount() > 0)
		{
			unsigned __int64 uiBootCatSector = m_pSectorManager->GetStart(this,SR_BOOTCATALOG);
			if (!m_pElTorito->WriteBootRecord(m_pOutStream,(unsigned long)uiBootCatSector))
			{
				m_pLog->AddLine(_T("  Error: Failed to write boot record at sector %I64d."),uiBootCatSector);
				return RESULT_FAIL;
			}

			m_pLog->AddLine(_T("  Wrote El Torito boot record at sector %d."),uiBootCatSector);
		}

		// Write ISO9660 descriptor.
		if (m_pIso9660->HasVolDescSuppl())
		{
			if (!m_pIso9660->WriteVolDescSuppl(m_pOutStream,m_stImageCreate,(unsigned long)uiFileDataEndSector,
				(unsigned long)m_uiPathTableSizeNormal,ulPosPathTableNormalL,ulPosPathTableNormalM,
				(unsigned long)uiDirEntriesSector,(unsigned long)FileTree.GetRoot()->m_uiDataLenNormal))
			{
				m_pLog->AddLine(_T("  Error: Failed to write supplementary volume descriptor."));
				return RESULT_FAIL;
			}
		}

		// Write the Joliet header.
		if (m_bUseJoliet)
		{
			if (FileTree.GetRoot()->m_uiDataLenNormal > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Error: Root folder is larger (%I64d) than the ISO9660 file system can handle."),
					FileTree.GetRoot()->m_uiDataLenNormal);
				return RESULT_FAIL;
			}

			unsigned long ulRootExtentLocJoliet = (unsigned long)uiDirEntriesSector +
				BytesToSector((unsigned long)FileTree.GetRoot()->m_uiDataLenNormal);

			if (!m_pJoliet->WriteVolDesc(m_pOutStream,m_stImageCreate,(unsigned long)uiFileDataEndSector,
				(unsigned long)m_uiPathTableSizeJoliet,ulPosPathTableJolietL,ulPosPathTableJolietM,
				ulRootExtentLocJoliet,(unsigned long)FileTree.GetRoot()->m_uiDataLenJoliet))
			{
				return false;
			}
		}

		if (!m_pIso9660->WriteVolDescSetTerm(m_pOutStream))
			return false;

		// Write the El Torito boot catalog and boot image data.
		if (m_pElTorito->GetBootImageCount() > 0)
		{
			if (!m_pElTorito->WriteBootCatalog(m_pOutStream))
			{
				m_pLog->AddLine(_T("  Error: Failed to write boot catalog."));
				return false;
			}

			if (!m_pElTorito->WriteBootImages(m_pOutStream))
			{
				m_pLog->AddLine(_T("  Error: Failed to write images."));
				return false;
			}
		}

		return RESULT_OK;
	}

	int CIso9660Writer::WritePathTables(CFileSet &Files,CFileTree &FileTree,CProgressEx &Progress)
	{
		// Write the path tables.
		if (!WritePathTable(Files,FileTree,false,false,Progress))
		{
			m_pLog->AddLine(_T("  Error: Failed to write path table (LSBF)."));
			return RESULT_FAIL;
		}
		if (!WritePathTable(Files,FileTree,false,true,Progress))
		{
			m_pLog->AddLine(_T("  Error: Failed to write path table (MSBF)."));
			return RESULT_FAIL;
		}

		if (m_bUseJoliet)
		{
			if (!WritePathTable(Files,FileTree,true,false,Progress))
			{
				m_pLog->AddLine(_T("  Error: Failed to write Joliet path table (LSBF)."));
				return RESULT_FAIL;
			}
			if (!WritePathTable(Files,FileTree,true,true,Progress))
			{
				m_pLog->AddLine(_T("  Error: Failed to write Joliet path table (MSBF)."));
				return RESULT_FAIL;
			}
		}

		return RESULT_OK;
	}

	int CIso9660Writer::WriteLocalDirEntry(CProgressEx &Progress,CFileTreeNode *pLocalNode,
		bool bJoliet,int iLevel)
	{
		tDirRecord DirRecord;

		// Write the '.' and '..' directories.
		CFileTreeNode *pParentNode = pLocalNode->GetParent();
		if (pParentNode == NULL)
			pParentNode = pLocalNode;

		if (bJoliet)
		{
			WriteSysDirectory(pLocalNode,TYPE_CURRENT,(unsigned long)pLocalNode->m_uiDataPosJoliet);
			WriteSysDirectory(pLocalNode,TYPE_PARENT,(unsigned long)pParentNode->m_uiDataPosJoliet);
		}
		else
		{
			WriteSysDirectory(pLocalNode,TYPE_CURRENT,(unsigned long)pLocalNode->m_uiDataPosNormal);
			WriteSysDirectory(pLocalNode,TYPE_PARENT,(unsigned long)pParentNode->m_uiDataPosNormal);
		}

		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			// Check if we should abort.
			if (Progress.IsCanceled())
				return RESULT_CANCEL;

			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				// Validate directory level.
				if (iLevel >= m_pIso9660->GetMaxDirLevel())
					continue;
			}

			// Validate file size.
			if ((*itFile)->m_uiFileSize > ISO9660_MAX_EXTENT_SIZE && !m_pIso9660->AllowsFragmentation())
				continue;

			// This loop is necessary for multi-extent support.
			unsigned __int64 uiFileRemain = bJoliet ? (*itFile)->m_uiDataLenJoliet : (*itFile)->m_uiDataLenNormal;
			unsigned __int64 uiExtentLoc = bJoliet ? (*itFile)->m_uiDataPosJoliet : (*itFile)->m_uiDataPosNormal;

			do
			{
				// We can't actually use 0xFFFFFFFF bytes since that will not fit perfectly withing a sector range.
				unsigned long ulExtentSize = uiFileRemain > ISO9660_MAX_EXTENT_SIZE ?
					ISO9660_MAX_EXTENT_SIZE : (unsigned long)uiFileRemain;

				memset(&DirRecord,0,sizeof(DirRecord));

				// Make a valid file name.
				unsigned char ucNameLen;
				unsigned char szFileName[ISO9660WRITER_FILENAME_BUFFER_SIZE + 4]; // Large enough for level 1, 2 and even Joliet.

				bool bIsFolder = (*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY;
				if (bJoliet)
					ucNameLen = m_pJoliet->WriteFileName(szFileName,(*itFile)->m_FileName.c_str(),bIsFolder) << 1;
				else
					ucNameLen = m_pIso9660->WriteFileName(szFileName,(*itFile)->m_FileName.c_str(),bIsFolder);

				// If the record length is not even padd it with a 0 byte.
				bool bPadByte = false;
				unsigned char ucDirRecLen = sizeof(DirRecord) + ucNameLen - 1;
				if (ucDirRecLen % 2 == 1)
				{
					bPadByte = true;
					ucDirRecLen++;
				}

				DirRecord.ucDirRecordLen = ucDirRecLen;
				DirRecord.ucExtAttrRecordLen = 0;

				Write733(DirRecord.ucExtentLocation,(unsigned long)uiExtentLoc);
				Write733(DirRecord.ucDataLen,(unsigned long)ulExtentSize);

				// Date time.
				if (m_bUseFileTimes)
				{
					unsigned short usFileDate = 0;
					unsigned short usFileTime = 0;
					bool bResult = true;

					if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
						bResult = fs_getdirmodtime((*itFile)->m_FileFullPath.c_str(),usFileDate,usFileTime);
					else
						bResult = fs_getmodtime((*itFile)->m_FileFullPath.c_str(),usFileDate,usFileTime);

					if (bResult)
						MakeDateTime(usFileDate,usFileTime,DirRecord.RecDateTime);
					else
						MakeDateTime(m_stImageCreate,DirRecord.RecDateTime);
				}
				else
				{
					// The time when the disc image creation was initialized.
					MakeDateTime(m_stImageCreate,DirRecord.RecDateTime);
				}

				// File flags.
				DirRecord.ucFileFlags = 0;
				if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
					DirRecord.ucFileFlags |= DIRRECORD_FILEFLAG_DIRECTORY;

				unsigned long ulFileAttr = fs_getfileattributes((*itFile)->m_FileFullPath.c_str());
				if (ulFileAttr != INVALID_FILE_ATTRIBUTES)
				{
					if (ulFileAttr & FILE_ATTRIBUTE_HIDDEN)
						DirRecord.ucFileFlags |= DIRRECORD_FILEFLAG_HIDDEN;
				}

				// Remaining bytes, before checking if we're dealing with the last segment.
				uiFileRemain -= ulExtentSize;
				if ((*itFile)->m_uiFileSize > ISO9660_MAX_EXTENT_SIZE && uiFileRemain > 0)
					DirRecord.ucFileFlags |= DIRRECORD_FILEFLAG_MULTIEXTENT;

				DirRecord.ucFileUnitSize = 0;
				DirRecord.ucInterleaveGapSize = 0;
				Write723(DirRecord.ucVolSeqNumber,0x01);
				DirRecord.ucFileIdentifierLen = ucNameLen;

				// Write the record.
				unsigned long ulProcessedSize;
				if (m_pOutStream->Write(&DirRecord,sizeof(DirRecord) - 1,&ulProcessedSize) != STREAM_OK)
					return RESULT_FAIL;
				if (ulProcessedSize != sizeof(DirRecord) - 1)
					return RESULT_FAIL;

				if (m_pOutStream->Write(szFileName,ucNameLen,&ulProcessedSize) != STREAM_OK)
					return RESULT_FAIL;
				if (ulProcessedSize != ucNameLen)
					return RESULT_FAIL;

				// Pad if necessary.
				if (bPadByte)
				{
					char szTemp[1] = { 0 };
					if (m_pOutStream->Write(szTemp,1,&ulProcessedSize) != STREAM_OK)
						return RESULT_FAIL;
					if (ulProcessedSize != 1)
						return RESULT_FAIL;
				}

				// Update location of the next extent.
				uiExtentLoc += BytesToSector(ulExtentSize);
			}
			while (uiFileRemain > 0);
		}

		if (m_pOutStream->GetAllocated() != 0)
			m_pOutStream->PadSector();

		return RESULT_OK;
	}

	int CIso9660Writer::WriteLocalDirEntries(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
		CProgressEx &Progress,CFileTreeNode *pLocalNode,int iLevel)
	{
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				// Validate directory level.
				if (iLevel >= m_pIso9660->GetMaxDirLevel())
					continue;
				else
					DirNodeStack.push_back(std::make_pair(*itFile,iLevel + 1));
			}
		}

		int iResult = WriteLocalDirEntry(Progress,pLocalNode,false,iLevel);
		if (iResult != RESULT_OK)
			return iResult;

		if (m_bUseJoliet)
		{
			iResult = WriteLocalDirEntry(Progress,pLocalNode,true,iLevel);
			if (iResult != RESULT_OK)
				return iResult;
		}

		return RESULT_OK;
	}

	int CIso9660Writer::WriteDirEntries(CFileTree &FileTree,CProgressEx &Progress)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();

		std::vector<std::pair<CFileTreeNode *,int> > DirNodeStack;
		if (!WriteLocalDirEntries(DirNodeStack,Progress,pCurNode,1))
			return RESULT_FAIL;

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			int iLevel = DirNodeStack[DirNodeStack.size() - 1].second;
			DirNodeStack.pop_back();

			int iResult = WriteLocalDirEntries(DirNodeStack,Progress,pCurNode,iLevel);
			if (iResult != RESULT_OK)
				return iResult;
		}

		return RESULT_OK;
	}
};
