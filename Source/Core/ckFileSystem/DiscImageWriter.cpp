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
#include "DiscImageWriter.h"
#include "StringTable.h"
#include "../../Common/FileManager.h"

namespace ckFileSystem
{
	CDiscImageWriter::CDiscImageWriter(CLog *pLog) : m_ElTorito(pLog)
	{
		m_pLog = pLog;

		m_pFileStream = new COutFileStream();
		m_pOutStream = new CSectorOutStream(m_pFileStream,DISCIMAGEWRITER_IO_BUFFER_SIZE,ISO9660_SECTOR_SIZE);

		m_bUseFileTimes = true;
		//m_bUseJoliet = true;
		m_bUseJoliet = false;
	}

	CDiscImageWriter::~CDiscImageWriter()
	{
		// Make sure that the file is closed.
		Close();

		if (m_pOutStream != NULL)
		{
			delete m_pOutStream;
			m_pOutStream = NULL;
		}

		if (m_pFileStream != NULL)
		{
			delete m_pFileStream;
			m_pFileStream = NULL;
		}
	}

	bool CDiscImageWriter::WritePathTable(CFileSet &Files,CFileTree &FileTree,
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

			if (iLevel > m_Iso9660.GetMaxDirLevel())
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

							/*char szFileName[32];	// Large enough for both level 1 and above.
							if (m_InterLevel == LEVEL_1)
								MakeDirNameL1(CurDirName.c_str(),szFileName);
							else
								MakeDirNameL2(CurDirName.c_str(),szFileName);
							unsigned char ucNameLen = (unsigned char)strlen(szFileName);*/

							unsigned char ucNameLen;
							unsigned char szFileName[DISCIMAGEWRITER_FILENAME_BUFFER_LEN];	// Large enough for both level 1, 2 and even Joliet.
							if (bJolietTable)
								ucNameLen = m_Joliet.WriteFileName(szFileName,CurDirName.c_str(),true) << 1;
							else
								ucNameLen = m_Iso9660.WriteFileName(szFileName,CurDirName.c_str(),true);

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

	bool CDiscImageWriter::WriteSysDirectory(CFileTreeNode *pParent,eSysDirType Type,
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

	int CDiscImageWriter::WriteFileNode(CFileTreeNode *pNode,CProgressEx &Progress,
		CFilesProgress &FilesProgress)
	{
		CInFileStream FileStream;
		if (!FileStream.Open(pNode->n_FileFullPath.c_str()))
		{
			m_pLog->AddLine(_T("  Error: Unable to obtain file handle to \"%s\"."),
				pNode->n_FileFullPath.c_str());
			return RESULT_FAIL;
		}

		char szBuffer[DISCIMAGEWRITER_IO_BUFFER_SIZE];
		unsigned long ulProcessedSize = 0;

		while (!FileStream.EOS())
		{
			// Check if we should abort.
			if (Progress.IsCanceled())
				return RESULT_CANCEL;

			if (FileStream.Read(szBuffer,DISCIMAGEWRITER_IO_BUFFER_SIZE,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("  Error: Unable read file: %s."),pNode->n_FileFullPath.c_str());
				return RESULT_FAIL;
			}

			// Check if we should abort.
			if (Progress.IsCanceled())
				return RESULT_CANCEL;

			if (m_pOutStream->Write(szBuffer,ulProcessedSize,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("  Error: Unable write to disc image."));
				return RESULT_FAIL;
			}

			Progress.SetProgress(FilesProgress.UpdateProcessed(ulProcessedSize));
		}

		// Pad the sector.
		if (m_pOutStream->GetAllocated() != 0)
			m_pOutStream->PadSector();

		return RESULT_OK;
	}

	int CDiscImageWriter::WriteLocalDirEntry(CFileTreeNode *pLocalNode,bool bJoliet,int iLevel,
		CProgressEx &Progress,CFilesProgress &FilesProgress)
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
				if (iLevel >= m_Iso9660.GetMaxDirLevel())
					continue;
			}

			// Validate file size.
			if ((*itFile)->m_uiFileSize > 0xFFFFFFFF)
				continue;

			memset(&DirRecord,0,sizeof(DirRecord));

			// Make a valid file name.
			unsigned char ucNameLen;
			unsigned char szFileName[DISCIMAGEWRITER_FILENAME_BUFFER_LEN + 4]; // Large enough for level 1, 2 and even Joliet.

			bool bIsFolder = (*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY;
			if (bJoliet)
				ucNameLen = m_Joliet.WriteFileName(szFileName,(*itFile)->m_FileName.c_str(),bIsFolder) << 1;
			else
				ucNameLen = m_Iso9660.WriteFileName(szFileName,(*itFile)->m_FileName.c_str(),bIsFolder);

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

			if (bJoliet)
			{
				Write733(DirRecord.ucExtentLocation,(unsigned long)(*itFile)->m_uiDataPosJoliet);
				Write733(DirRecord.ucDataLen,(unsigned long)(*itFile)->m_uiDataLenJoliet);
			}
			else
			{
				Write733(DirRecord.ucExtentLocation,(unsigned long)(*itFile)->m_uiDataPosNormal);
				Write733(DirRecord.ucDataLen,(unsigned long)(*itFile)->m_uiDataLenNormal);
			}

			// Date time.
			if (m_bUseFileTimes)
			{
				unsigned short usFileDate = 0;
				unsigned short usFileTime = 0;
				bool bResult = true;

				if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
					bResult = fs_getdirmodtime((*itFile)->n_FileFullPath.c_str(),usFileDate,usFileTime);
				else
					bResult = fs_getmodtime((*itFile)->n_FileFullPath.c_str(),usFileDate,usFileTime);

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

			unsigned long ulFileAttr = fs_getfileattributes((*itFile)->n_FileFullPath.c_str());
			if (ulFileAttr != INVALID_FILE_ATTRIBUTES)
			{
				if (ulFileAttr & FILE_ATTRIBUTE_HIDDEN)
					DirRecord.ucFileFlags |= DIRRECORD_FILEFLAG_HIDDEN;
			}

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

			FilesProgress.UpdateProcessed(ulProcessedSize);

			if (m_pOutStream->Write(szFileName,ucNameLen,&ulProcessedSize) != STREAM_OK)
				return RESULT_FAIL;
			if (ulProcessedSize != ucNameLen)
				return RESULT_FAIL;

			FilesProgress.UpdateProcessed(ulProcessedSize);

			// Pad if necessary.
			if (bPadByte)
			{
				char szTemp[1] = { 0 };
				if (m_pOutStream->Write(szTemp,1,&ulProcessedSize) != STREAM_OK)
					return RESULT_FAIL;
				if (ulProcessedSize != 1)
					return RESULT_FAIL;

				FilesProgress.UpdateProcessed(1);
			}
		}

		if (m_pOutStream->GetAllocated() != 0)
		{
			FilesProgress.UpdateProcessed(m_pOutStream->GetRemaining());
			m_pOutStream->PadSector();
		}

		return RESULT_OK;
	}

	int CDiscImageWriter::WriteLocalDirectory(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
		CFileTreeNode *pLocalNode,int iLevel,CProgressEx &Progress,CFilesProgress &FilesProgress)
	{
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				// Validate directory level.
				if (iLevel >= m_Iso9660.GetMaxDirLevel())
					continue;
				else
					DirNodeStack.push_back(std::make_pair(*itFile,iLevel + 1));
			}
		}

		int iResult = WriteLocalDirEntry(pLocalNode,false,iLevel,Progress,FilesProgress);
		if (iResult != RESULT_OK)
			return iResult;

		if (m_bUseJoliet)
		{
			iResult = WriteLocalDirEntry(pLocalNode,true,iLevel,Progress,FilesProgress);
			if (iResult != RESULT_OK)
				return iResult;
		}

		// Now, write the actual files.
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			// Check if we should abort.
			if (Progress.IsCanceled())
				return RESULT_CANCEL;

			// Skip folders since they will be taken care of later.
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
				continue;

			// Validate file size.
			if ((*itFile)->m_uiFileSize > 0xFFFFFFFF)
				continue;

			switch (WriteFileNode(*itFile,Progress,FilesProgress))
			{
				case RESULT_FAIL:
					m_pLog->AddLine(_T("  Error: Unable to write node \"%s\" to (%I64d,%I64d)."),
						(*itFile)->m_FileName.c_str(),(*itFile)->m_uiDataPosNormal,(*itFile)->m_uiDataLenNormal);
					return RESULT_FAIL;

				case RESULT_CANCEL:
					return RESULT_CANCEL;
			}
		}

		return RESULT_OK;
	}

	int CDiscImageWriter::WriteDirectories(CFileTree &FileTree,
		CProgressEx &Progress,CFilesProgress &FilesProgress)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();

		std::vector<std::pair<CFileTreeNode *,int> > DirNodeStack;
		if (!WriteLocalDirectory(DirNodeStack,pCurNode,1,Progress,FilesProgress))
			return RESULT_FAIL;

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			int iLevel = DirNodeStack[DirNodeStack.size() - 1].second;
			DirNodeStack.pop_back();

			int iResult = WriteLocalDirectory(DirNodeStack,pCurNode,iLevel,Progress,FilesProgress);
			if (iResult != RESULT_OK)
				return iResult;
		}

		return RESULT_OK;
	}

	bool CDiscImageWriter::CalcPathTableSize(CFileSet &Files,bool bJolietTable,
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

			if (iLevel > m_Iso9660.GetMaxDirLevel())
			{
				// Print the message only once.
				if (!bFoundDeep)
				{
					m_pLog->AddLine(_T("  Warning: The directory structure is deeper than %d levels. Deep files and folders will be ignored."),
						m_Iso9660.GetMaxDirLevel());
					Progress.AddLogEntry(CProgressEx::LT_WARNING,g_StringTable.GetString(WARNING_FSDIRLEVEL),m_Iso9660.GetMaxDirLevel());
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
							/*unsigned char ucNameLen;
							if (m_InterLevel == LEVEL_1)
								ucNameLen = CalcDirNameLenL1(CurDirName.c_str());
							else
								ucNameLen = CalcDirNameLenL2(CurDirName.c_str());*/
							unsigned char ucNameLen = bJolietTable ?
								m_Joliet.CalcFileNameLen(CurDirName.c_str(),true) << 1 : 
								m_Iso9660.CalcFileNameLen(CurDirName.c_str(),true);

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
	bool CDiscImageWriter::CalcLocalDirEntrySize(CFileTreeNode *pLocalNode,bool bJoliet,
		int iLevel,unsigned long &ulDirSecSize)
	{
		ulDirSecSize = 0;

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
				if (iLevel >= m_Iso9660.GetMaxDirLevel())
					continue;
			}

			// Validate file size.
			if ((*itFile)->m_uiFileSize > 0xFFFFFFFF)
				continue;

			bool bIsFolder = (*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY;

			unsigned char ucNameLen;
			if (bJoliet)
				ucNameLen = m_Joliet.CalcFileNameLen((*itFile)->m_FileName.c_str(),bIsFolder) << 1;
			else
				ucNameLen = m_Iso9660.CalcFileNameLen((*itFile)->m_FileName.c_str(),bIsFolder);

			unsigned char ucCurRecSize = sizeof(tDirRecord) + ucNameLen - 1;

			// If the record length is not even padd it with a 0 byte.
			if (ucCurRecSize % 2 == 1)
				ucCurRecSize++;

			if (ulDirSecData + ucCurRecSize > ISO9660_SECTOR_SIZE)
			{
				ulDirSecData = 0;
				ulDirSecSize++;
			}
			else
			{
				ulDirSecData += ucCurRecSize;
			}
		}

		if (ulDirSecData != 0)
			ulDirSecSize++;

		return true;
	}

	bool CDiscImageWriter::CalcLocalFileSysData(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
												CFileTreeNode *pLocalNode,int iLevel,
												unsigned __int64 &uiSecOffset,CProgressEx &Progress)
	{
		unsigned long ulDirSecSizeNormal = 0;
		if (!CalcLocalDirEntrySize(pLocalNode,false,iLevel,ulDirSecSizeNormal))
			return false;

		unsigned long ulDirSecSizeJoliet = 0;
		if (m_bUseJoliet && !CalcLocalDirEntrySize(pLocalNode,true,iLevel,ulDirSecSizeJoliet))
			return false;

		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				// Validate directory level.
				if (iLevel >= m_Iso9660.GetMaxDirLevel())
					continue;
				else
					DirNodeStack.push_back(std::make_pair(*itFile,iLevel + 1));
			}

			// Validate file size.
			if ((*itFile)->m_uiFileSize > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Warning: Skipping \"%s\", the file is larger than 4 GiB."),
					(*itFile)->m_FileName.c_str());
				Progress.AddLogEntry(CProgressEx::LT_WARNING,g_StringTable.GetString(WARNING_SKIP4GFILE),
					(*itFile)->m_FileName.c_str());

				continue;
			}
		}

		// We now know how much space is needed to store the current directory record.
		pLocalNode->m_uiDataLenNormal = ulDirSecSizeNormal * ISO9660_SECTOR_SIZE;
		pLocalNode->m_uiDataLenJoliet = ulDirSecSizeJoliet * ISO9660_SECTOR_SIZE;

		pLocalNode->m_uiDataPosNormal = uiSecOffset;
		uiSecOffset += ulDirSecSizeNormal;
		pLocalNode->m_uiDataPosJoliet = uiSecOffset;
		uiSecOffset += ulDirSecSizeJoliet;

		// Calculate the sector offset for the remaining files.
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			// Skip folders since they will be taken care of later.
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
				continue;

			(*itFile)->m_uiDataLenNormal = (*itFile)->m_uiFileSize;
			(*itFile)->m_uiDataLenJoliet = (*itFile)->m_uiFileSize;
			(*itFile)->m_uiDataPosNormal = uiSecOffset;
			(*itFile)->m_uiDataPosJoliet = uiSecOffset;

			uiSecOffset += (*itFile)->m_uiDataLenNormal/ISO9660_SECTOR_SIZE;
			if ((*itFile)->m_uiDataLenNormal % ISO9660_SECTOR_SIZE != 0)
				uiSecOffset++;
		}

		return true;
	}

	bool CDiscImageWriter::CalcFileSysData(CFileTree &FileTree,unsigned __int64 uiStartSec,
										   unsigned __int64 &uiLastSec,CProgressEx &Progress)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();
		unsigned __int64 uiSecOffset = uiStartSec;

		std::vector<std::pair<CFileTreeNode *,int> > DirNodeStack;
		if (!CalcLocalFileSysData(DirNodeStack,pCurNode,0,uiSecOffset,Progress))
			return false;

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			int iLevel = DirNodeStack[DirNodeStack.size() - 1].second;
			DirNodeStack.pop_back();

			if (!CalcLocalFileSysData(DirNodeStack,pCurNode,iLevel,uiSecOffset,Progress))
				return false;
		}

		uiLastSec = uiSecOffset;
		return true;
	}

	bool CDiscImageWriter::ValidateTreeNode(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
		CFileTreeNode *pNode,int iLevel)
	{
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pNode->m_Children.begin(); itFile !=
			pNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				if (iLevel >= m_Iso9660.GetMaxDirLevel())
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
	bool CDiscImageWriter::ValidateTree(CFileTree &FileTree)
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

	int CDiscImageWriter::Create(const TCHAR *szFullPath,CFileSet &Files,CProgressEx &Progress)
	{
		m_pLog->AddLine(_T("CDiscImageWriter::Create"));

		if (!m_pFileStream->Open(szFullPath))
		{
			m_pLog->AddLine(_T("  Error: Unable to obtain file handle to \"%s\"."),szFullPath);
			Progress.AddLogEntry(CProgressEx::LT_ERROR,g_StringTable.GetString(ERROR_OPENWRITE),szFullPath);
			return RESULT_FAIL;
		}

		// Calculate path table sizes.
		unsigned __int64 uiPathTableSizeNormal = 0;
		if (!CalcPathTableSize(Files,false,uiPathTableSizeNormal,Progress))
		{
			m_pLog->AddLine(_T("  Error: Unable to calculate path table size."));

			Fail(szFullPath);
			return RESULT_FAIL;
		}

		unsigned __int64 uiPathTableSizeJoliet = 0;
		if (m_bUseJoliet && !CalcPathTableSize(Files,true,uiPathTableSizeJoliet,Progress))
		{
			m_pLog->AddLine(_T("  Error: Unable to calculate Joliet path table size."));

			Fail(szFullPath);
			return RESULT_FAIL;
		}

		if (uiPathTableSizeNormal > 0xFFFFFFFF || uiPathTableSizeJoliet > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: The path table is too large, %I64d and %I64d bytes."),
				uiPathTableSizeNormal,uiPathTableSizeJoliet);
			Progress.AddLogEntry(CProgressEx::LT_ERROR,g_StringTable.GetString(ERROR_PATHTABLESIZE));

			Fail(szFullPath);
			return RESULT_FAIL;
		}

		// The first 16 sectors are reserved for system use (write 0s).
		char szTemp[1] = { 0 };

		unsigned long ulProcessedSize = 0;
		for (unsigned int i = 0; i < ISO9660_SECTOR_SIZE << 4; i++)
			m_pOutStream->Write(szTemp,1,&ulProcessedSize);

		// Volume descriptor size.
		unsigned long ulVolDescSize = sizeof(tVolDescPrimary) + sizeof(tVolDescSetTerm);
		if (m_ElTorito.GetBootImageCount() > 0)
			ulVolDescSize += sizeof(tVolDescElToritoRecord);
		if (m_Iso9660.HasVolDescSuppl())
			ulVolDescSize += sizeof(tVolDescSuppl);
		if (m_bUseJoliet)
			ulVolDescSize += sizeof(tVolDescSuppl);

		// Boot catalog size (0 if no boot images is used).
		unsigned __int64 uiBootCatSize = 0;
		if (m_ElTorito.GetBootImageCount() > 0)
			uiBootCatSize = m_ElTorito.GetBootCatSize();

		// Boot catalog size and boot images data size.
		unsigned __int64 uiPosBootCat = m_pOutStream->GetSector() + BytesToSector(ulVolDescSize);
		unsigned __int64 uiPosBootData = uiPosBootCat + BytesToSector(uiBootCatSize);
		unsigned __int64 uiEndBootData = uiPosBootData;
		
		if (m_ElTorito.GetBootImageCount() > 0)
		{
			if (uiPosBootData > 0xFFFFFFFF || uiEndBootData > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Errro: Invalid boot catalog sector range (%I64d to %I64d)."),
					uiPosBootCat,uiEndBootData);

				Fail(szFullPath);
				return RESULT_FAIL;
			}

			if (!m_ElTorito.CalculateFileSysData(uiPosBootData,uiEndBootData))
			{
				Fail(szFullPath);
				return RESULT_FAIL;
			}
		}

		unsigned long ulPosPathTableNormalL = (unsigned long)uiEndBootData;
		unsigned long ulPosPathTableNormalM = ulPosPathTableNormalL + BytesToSector(uiPathTableSizeNormal);
		unsigned long ulPosPathTableJolietL = ulPosPathTableNormalM + BytesToSector(uiPathTableSizeNormal);
		unsigned long ulPosPathTableJolietM = ulPosPathTableJolietL + BytesToSector(uiPathTableSizeJoliet);
		unsigned long ulRootExtentLoc = ulPosPathTableJolietM + BytesToSector(uiPathTableSizeJoliet);

		m_pLog->AddLine(_T("  Sizes: %I64d, %I64d, %I64d, %d."),uiPathTableSizeNormal,uiPathTableSizeJoliet,
			uiBootCatSize,ulVolDescSize);
		m_pLog->AddLine(_T("  Locations: %I64d, %d, %d, %d, %d, %d."),uiPosBootCat,ulPosPathTableNormalL,
			ulPosPathTableNormalM,ulPosPathTableJolietL,ulPosPathTableJolietM,ulRootExtentLoc);

		// Create a file tree.
		CFileTree FileTree(m_pLog);
		FileTree.CreateFromFileSet(Files);

		unsigned __int64 uiLastDataSec = 0;
		if (!CalcFileSysData(FileTree,ulRootExtentLoc,uiLastDataSec,Progress))
		{
			m_pLog->AddLine(_T("  Error: Unable to calculate file system data."));

			Fail(szFullPath);
			return RESULT_FAIL;
		}

		// Get system time.
		GetLocalTime(&m_stImageCreate);

		if (!m_Iso9660.WriteVolDescPrimary(m_pOutStream,m_stImageCreate,(unsigned long)uiLastDataSec,
				(unsigned long)uiPathTableSizeNormal,ulPosPathTableNormalL,ulPosPathTableNormalM,
				ulRootExtentLoc,(unsigned long)FileTree.GetRoot()->m_uiDataLenNormal))
		{
			Fail(szFullPath);
			return RESULT_FAIL;
		}

		// Write the El Torito boot record at sector 17 if necessary.
		if (m_ElTorito.GetBootImageCount() > 0)
		{
			if (!m_ElTorito.WriteBootRecord(m_pOutStream,(unsigned long)uiPosBootCat))
			{
				m_pLog->AddLine(_T("  Error: Failed to write boot record at sector %d."),uiPosBootCat);

				Fail(szFullPath);
				return RESULT_FAIL;
			}

			m_pLog->AddLine(_T("  Wrote El Torito boot record at sector %d."),uiPosBootCat);
		}

		// Write ISO9660 descriptor.
		if (m_Iso9660.HasVolDescSuppl())
		{
			if (!m_Iso9660.WriteVolDescSuppl(m_pOutStream,m_stImageCreate,(unsigned long)uiLastDataSec,
				(unsigned long)uiPathTableSizeNormal,ulPosPathTableNormalL,ulPosPathTableNormalM,
				ulRootExtentLoc,(unsigned long)FileTree.GetRoot()->m_uiDataLenNormal))
			{
				m_pLog->AddLine(_T("  Error: Failed to write supplementary volume descriptor."));
				Fail(szFullPath);
				return RESULT_FAIL;
			}
		}

		// Write the Joliet header.
		if (m_bUseJoliet)
		{
			unsigned long ulRootExtentLocJoliet = ulRootExtentLoc +
				BytesToSector(FileTree.GetRoot()->m_uiDataLenNormal);

			if (!m_Joliet.WriteVolDesc(m_pOutStream,m_stImageCreate,(unsigned long)uiLastDataSec,
				(unsigned long)uiPathTableSizeJoliet,ulPosPathTableJolietL,ulPosPathTableJolietM,
				ulRootExtentLocJoliet,(unsigned long)FileTree.GetRoot()->m_uiDataLenJoliet))
			{
				Fail(szFullPath);
				return false;
			}
		}

		if (!m_Iso9660.WriteVolDescSetTerm(m_pOutStream))
		{
			Fail(szFullPath);
			return false;
		}

		// Write the El Torito boot catalog and boot image data.
		if (m_ElTorito.GetBootImageCount() > 0)
		{
			if (!m_ElTorito.WriteBootCatalog(m_pOutStream))
			{
				m_pLog->AddLine(_T("  Error: Failed to write boot catalog."));

				Fail(szFullPath);
				return false;
			}

			if (!m_ElTorito.WriteBootImages(m_pOutStream))
			{
				m_pLog->AddLine(_T("  Error: Failed to write images."));

				Fail(szFullPath);
				return false;
			}
		}

		// Write the path tables.
		if (!WritePathTable(Files,FileTree,false,false,Progress))
		{
			m_pLog->AddLine(_T("  Error: Failed to write path table (LSBF)."));

			Fail(szFullPath);
			return RESULT_FAIL;
		}
		if (!WritePathTable(Files,FileTree,false,true,Progress))
		{
			m_pLog->AddLine(_T("  Error: Failed to write path table (MSBF)."));

			Fail(szFullPath);
			return RESULT_FAIL;
		}

		if (m_bUseJoliet)
		{
			if (!WritePathTable(Files,FileTree,true,false,Progress))
			{
				m_pLog->AddLine(_T("  Error: Failed to write Joliet path table (LSBF)."));

				Fail(szFullPath);
				return RESULT_FAIL;
			}
			if (!WritePathTable(Files,FileTree,true,true,Progress))
			{
				m_pLog->AddLine(_T("  Error: Failed to write Joliet path table (MSBF)."));

				Fail(szFullPath);
				return RESULT_FAIL;
			}
		}

		// To help keep track of the progress.
		CFilesProgress FilesProgress(uiLastDataSec * ISO9660_SECTOR_SIZE - ulRootExtentLoc * ISO9660_SECTOR_SIZE);

		// Write the directory entries.
		switch (WriteDirectories(FileTree,Progress,FilesProgress))
		{
			case RESULT_FAIL:
				m_pLog->AddLine(_T("  Error: Failed to write directories and file data."));

				Fail(szFullPath);
				return RESULT_FAIL;

			case RESULT_CANCEL:
				Fail(szFullPath);
				return RESULT_CANCEL;
		}

#ifdef _DEBUG
		FileTree.PrintTree();
#endif

		if (!Close())
			return RESULT_FAIL;

		return RESULT_OK;
	}

	/*
		Should be called when create operation fails or cancel so that the
		broken image can be removed and the file handle closed.
	*/
	bool CDiscImageWriter::Fail(const TCHAR *szFullPath)
	{
		m_pOutStream->Flush();
		if (m_pFileStream->Close())
		{
			fs_deletefile(szFullPath);
			return true;
		}

		return false;
	}

	bool CDiscImageWriter::Close()
	{
		m_pOutStream->Flush();
		return m_pFileStream->Close();
	}

	void CDiscImageWriter::SetVolumeLabel(const TCHAR *szLabel)
	{
		m_Iso9660.SetVolumeLabel(szLabel);
		m_Joliet.SetVolumeLabel(szLabel);
	}

	void CDiscImageWriter::SetTextFields(const TCHAR *szSystem,const TCHAR *szVolSetIdent,
										 const TCHAR *szPublIdent,const TCHAR *szPrepIdent)
	{
		m_Iso9660.SetTextFields(szSystem,szVolSetIdent,szPublIdent,szPrepIdent);
		m_Joliet.SetTextFields(szSystem,szVolSetIdent,szPublIdent,szPrepIdent);
	}

	void CDiscImageWriter::SetFileFields(const TCHAR *szCopyFileIdent,
										 const TCHAR *szAbstFileIdent,
										 const TCHAR *szBiblFileIdent)
	{
		m_Iso9660.SetFileFields(szCopyFileIdent,szAbstFileIdent,szBiblFileIdent);
		m_Joliet.SetFileFields(szCopyFileIdent,szAbstFileIdent,szBiblFileIdent);
	}

	void CDiscImageWriter::SetInterchangeLevel(CIso9660::eInterLevel InterLevel)
	{
		m_Iso9660.SetInterchangeLevel(InterLevel);
	}

	void CDiscImageWriter::SetIncludeFileVerInfo(bool bIncludeInfo)
	{
		m_Iso9660.SetIncludeFileVerInfo(bIncludeInfo);
		m_Joliet.SetIncludeFileVerInfo(bIncludeInfo);
	}

	bool CDiscImageWriter::AddBootImageNoEmu(const TCHAR *szFullPath,bool bBootable,
		unsigned short usLoadSegment,unsigned short usSectorCount)
	{
		return m_ElTorito.AddBootImageNoEmu(szFullPath,bBootable,usLoadSegment,usSectorCount);
	}

	bool CDiscImageWriter::AddBootImageFloppy(const TCHAR *szFullPath,bool bBootable)
	{
		return m_ElTorito.AddBootImageFloppy(szFullPath,bBootable);
	}

	bool CDiscImageWriter::AddBootImageHardDisk(const TCHAR *szFullPath,bool bBootable)
	{
		return m_ElTorito.AddBootImageHardDisk(szFullPath,bBootable);
	}
};
