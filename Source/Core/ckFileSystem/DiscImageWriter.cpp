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
#include "DiscImageWriter.h"
#include "StringTable.h"
#include "SectorManager.h"
#include "Iso9660Writer.h"
#include "UdfWriter.h"
#include "DvdVideo.h"

namespace ckFileSystem
{
	CDiscImageWriter::CDiscImageWriter(CLog *pLog,eFileSystem FileSystem) : m_FileSystem(FileSystem),
		m_pLog(pLog),m_ElTorito(pLog),m_Udf(FileSystem == FS_DVDVIDEO)
	{
		m_pFileStream = new COutFileStream();
		m_pOutStream = new CSectorOutStream(m_pFileStream,DISCIMAGEWRITER_IO_BUFFER_SIZE,ISO9660_SECTOR_SIZE);
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

	/*
		Calculates file system specific data such as extent location and size for a
		single file.
	*/
	bool CDiscImageWriter::CalcLocalFileSysData(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
		CFileTreeNode *pLocalNode,int iLevel,unsigned __int64 &uiSecOffset,CProgressEx &Progress)
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
			else
			{
				// Validate file size.
				/*if (m_FileSystem == FS_ISO9660 || m_FileSystem == FS_ISO9660_JOLIET || m_FileSystem == FS_DVDVIDEO)
				{
					if ((*itFile)->m_uiFileSize > ISO9660_MAX_EXTENT_SIZE && !m_Iso9660.AllowsFragmentation())
					{
						m_pLog->AddLine(_T("  Warning: Skipping \"%s\", the file is larger than 4 GiB."),
							(*itFile)->m_FileName.c_str());
						Progress.AddLogEntry(CProgressEx::LT_WARNING,g_StringTable.GetString(WARNING_SKIP4GFILE),
							(*itFile)->m_FileName.c_str());

						continue;
					}
				}*/

				if ((*itFile)->m_uiFileSize > ISO9660_MAX_EXTENT_SIZE && !m_Iso9660.AllowsFragmentation())
				{
					if (m_FileSystem == FS_ISO9660 || m_FileSystem == FS_ISO9660_JOLIET || m_FileSystem == FS_DVDVIDEO)
					{
						m_pLog->AddLine(_T("  Warning: Skipping \"%s\", the file is larger than 4 GiB."),
							(*itFile)->m_FileName.c_str());
						Progress.AddLogEntry(CProgressEx::LT_WARNING,g_StringTable.GetString(WARNING_SKIP4GFILE),
							(*itFile)->m_FileName.c_str());

						continue;
					}
					else if (m_FileSystem == FS_ISO9660_UDF || m_FileSystem == FS_ISO9660_UDF_JOLIET)
					{
						m_pLog->AddLine(_T("  Warning: THe file \"%s\" is larger than 4 GiB. It will not be visible in the ISO9660/Joliet file system."),
							(*itFile)->m_FileName.c_str());
						Progress.AddLogEntry(CProgressEx::LT_WARNING,g_StringTable.GetString(WARNING_SKIP4GFILEISO),
							(*itFile)->m_FileName.c_str());
					}
				}

				(*itFile)->m_uiDataSizeNormal = (*itFile)->m_uiFileSize;
				(*itFile)->m_uiDataSizeJoliet = (*itFile)->m_uiFileSize;
				(*itFile)->m_uiDataPosNormal = uiSecOffset;
				(*itFile)->m_uiDataPosJoliet = uiSecOffset;

				uiSecOffset += (*itFile)->m_uiDataSizeNormal/ISO9660_SECTOR_SIZE;
				if ((*itFile)->m_uiDataSizeNormal % ISO9660_SECTOR_SIZE != 0)
					uiSecOffset++;

				// Pad if necessary.
				uiSecOffset += (*itFile)->m_ulDataPadLen;
			}
		}

		return true;
	}

	/*
		Calculates file system specific data such as location of extents and sizes of
		extents.
	*/
	bool CDiscImageWriter::CalcFileSysData(CFileTree &FileTree,CProgressEx &Progress,
		unsigned __int64 uiStartSec,unsigned __int64 &uiLastSec)
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

	int CDiscImageWriter::WriteFileNode(CFileTreeNode *pNode,CProgressEx &Progress,
		CFilesProgress &FilesProgress)
	{
		CInFileStream FileStream;
		if (!FileStream.Open(pNode->m_FileFullPath.c_str()))
		{
			m_pLog->AddLine(_T("  Error: Unable to obtain file handle to \"%s\"."),
				pNode->m_FileFullPath.c_str());
			Progress.AddLogEntry(CProgressEx::LT_ERROR,g_StringTable.GetString(ERROR_OPENREAD),
				pNode->m_FileFullPath.c_str());
			return RESULT_FAIL;
		}

		char szBuffer[DISCIMAGEWRITER_IO_BUFFER_SIZE];
		unsigned long ulProcessedSize = 0;

		unsigned __int64 uiReadSize = 0;
		while (uiReadSize < pNode->m_uiFileSize)
		{
			// Check if we should abort.
			if (Progress.IsCanceled())
				return RESULT_CANCEL;

			if (FileStream.Read(szBuffer,DISCIMAGEWRITER_IO_BUFFER_SIZE,&ulProcessedSize) != STREAM_OK)
			{
				m_pLog->AddLine(_T("  Error: Unable read file: %s."),pNode->m_FileFullPath.c_str());
				return RESULT_FAIL;
			}

			if (ulProcessedSize == 0)
			{
				// We may have a problem. The file size may have changed since specied in file list.
				m_pLog->AddLine(_T("  Error: File size missmatch on \"%s\". Reported size %I64d bytes versus actual size %I64d bytes."),
					pNode->m_FileFullPath.c_str(),pNode->m_uiFileSize,uiReadSize);
				return RESULT_FAIL;
			}

			uiReadSize += ulProcessedSize;

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

	int CDiscImageWriter::WriteLocalFileData(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
		CFileTreeNode *pLocalNode,int iLevel,CProgressEx &Progress,CFilesProgress &FilesProgress)
	{
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
				else
					DirNodeStack.push_back(std::make_pair(*itFile,iLevel + 1));
			}
			else
			{
				// Validate file size.
				if (m_FileSystem == FS_ISO9660 || m_FileSystem == FS_ISO9660_JOLIET || m_FileSystem == FS_DVDVIDEO)
				{
					if ((*itFile)->m_uiFileSize > ISO9660_MAX_EXTENT_SIZE && !m_Iso9660.AllowsFragmentation())
						continue;
				}

				switch (WriteFileNode(*itFile,Progress,FilesProgress))
				{
					case RESULT_FAIL:
						m_pLog->AddLine(_T("  Error: Unable to write node \"%s\" to (%I64d,%I64d)."),
							(*itFile)->m_FileName.c_str(),(*itFile)->m_uiDataPosNormal,(*itFile)->m_uiDataSizeNormal);
						return RESULT_FAIL;

					case RESULT_CANCEL:
						return RESULT_CANCEL;
				}

				// Pad if necessary.
				char szTemp[1] = { 0 };
				unsigned long ulProcessedSize;
				for (unsigned int i = 0; i < (*itFile)->m_ulDataPadLen; i++)
				{
					for (unsigned int j = 0; j < ISO9660_SECTOR_SIZE; j++)
						m_pOutStream->Write(szTemp,1,&ulProcessedSize);
				}
			}
		}

		return RESULT_OK;
	}

	int CDiscImageWriter::WriteFileData(CFileTree &FileTree,
		CProgressEx &Progress,CFilesProgress &FilesProgress)
	{
		CFileTreeNode *pCurNode = FileTree.GetRoot();

		std::vector<std::pair<CFileTreeNode *,int> > DirNodeStack;
		int iResult = WriteLocalFileData(DirNodeStack,pCurNode,1,Progress,FilesProgress);
		if (iResult != RESULT_OK)
			return iResult;

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			int iLevel = DirNodeStack[DirNodeStack.size() - 1].second;
			DirNodeStack.pop_back();

			iResult = WriteLocalFileData(DirNodeStack,pCurNode,iLevel,Progress,FilesProgress);
			if (iResult != RESULT_OK)
				return iResult;
		}

		return RESULT_OK;
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

		// The first 16 sectors are reserved for system use (write 0s).
		char szTemp[1] = { 0 };

		unsigned long ulProcessedSize = 0;
		for (unsigned int i = 0; i < ISO9660_SECTOR_SIZE << 4; i++)
			m_pOutStream->Write(szTemp,1,&ulProcessedSize);

		// Create a file tree.
		CFileTree FileTree(m_pLog);
		FileTree.CreateFromFileSet(Files);

		// Calculate padding if DVD-Video file system.
		if (m_FileSystem == FS_DVDVIDEO)
		{
			CDvdVideo DvdVideo(m_pLog);
			if (!DvdVideo.CalcFilePadding(FileTree))
			{
				m_pLog->AddLine(_T("  Error: Failed to calculate file padding for DVD-Video file system."));
				return Fail(RESULT_FAIL,szFullPath);
			}

			DvdVideo.PrintFilePadding(FileTree);
		}

		bool bUseIso = m_FileSystem != FS_UDF;
		bool bUseUdf = m_FileSystem == FS_ISO9660_UDF || m_FileSystem == FS_ISO9660_UDF_JOLIET ||
			m_FileSystem == FS_UDF || m_FileSystem == FS_DVDVIDEO;
		bool bUseJoliet = m_FileSystem == FS_ISO9660_JOLIET || m_FileSystem == FS_ISO9660_UDF_JOLIET;

		CSectorManager SectorManager(16);
		CIso9660Writer IsoWriter(m_pLog,m_pOutStream,&SectorManager,&m_Iso9660,&m_Joliet,&m_ElTorito,true,bUseJoliet);
		CUdfWriter UdfWriter(m_pLog,m_pOutStream,&SectorManager,&m_Udf,true);

		int iResult = RESULT_FAIL;

		// FIXME: Put failure messages to Progress.
		if (bUseIso)
		{
			iResult = IsoWriter.AllocateHeader();
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		if (bUseUdf)
		{
			iResult = UdfWriter.AllocateHeader();
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		if (bUseIso)
		{
			iResult = IsoWriter.AllocatePathTables(Progress,Files);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);

			iResult = IsoWriter.AllocateDirEntries(FileTree,Progress);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		if (bUseUdf)
		{
			iResult = UdfWriter.AllocatePartition(FileTree);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		// Allocate file data.
		unsigned __int64 uiFirstDataSec = SectorManager.GetNextFree();
		unsigned __int64 uiLastDataSec = 0;
		if (!CalcFileSysData(FileTree,Progress,uiFirstDataSec,uiLastDataSec))
		{
			m_pLog->AddLine(_T("  Error: Could not calculate necessary file system information."));
			return Fail(RESULT_FAIL,szFullPath);
		}

		/*TCHAR szTemp2[128];
		lsprintf(szTemp2,_T("%I64d %I64d"),uiFirstDataSec,uiLastDataSec);
		MessageBox(NULL,szTemp2,_T(""),MB_OK);*/

		SectorManager.AllocateDataSectors(uiLastDataSec - uiFirstDataSec);

		if (bUseIso)
		{
			/*iResult = IsoWriter.AllocateFileData(Progress,FileTree);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);*/

			iResult = IsoWriter.WriteHeader(Files,FileTree,Progress);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		if (bUseUdf)
		{
			iResult = UdfWriter.WriteHeader();
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		if (bUseIso)
		{
			iResult = IsoWriter.WritePathTables(Files,FileTree,Progress);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);

			iResult = IsoWriter.WriteDirEntries(FileTree,Progress);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		if (bUseUdf)
		{
			iResult = UdfWriter.WritePartition(FileTree);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		// To help keep track of the progress.
		CFilesProgress FilesProgress(SectorManager.GetDataLength() * ISO9660_SECTOR_SIZE);
		iResult = WriteFileData(FileTree,Progress,FilesProgress);
		if (iResult != RESULT_OK)
			return Fail(iResult,szFullPath);

		if (bUseUdf)
		{
			iResult = UdfWriter.WriteTail();
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
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
	int CDiscImageWriter::Fail(int iResult,const TCHAR *szFullPath)
	{
		m_pOutStream->Flush();
		if (m_pFileStream->Close())
			fs_deletefile(szFullPath);

		return iResult;
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
		m_Udf.SetVolumeLabel(szLabel);
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

	void CDiscImageWriter::SetPartAccessType(CUdf::ePartAccessType AccessType)
	{
		m_Udf.SetPartAccessType(AccessType);
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
