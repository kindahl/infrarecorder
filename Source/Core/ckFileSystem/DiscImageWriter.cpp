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
		m_pOutStream = new CSectorOutStream(m_pFileStream,ISO9660WRITER_IO_BUFFER_SIZE,ISO9660_SECTOR_SIZE);
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
		bool bUseUdf = m_FileSystem == FS_ISO9660_UDF || m_FileSystem == FS_UDF || m_FileSystem == FS_DVDVIDEO;
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
		}

		if (bUseUdf)
		{
			iResult = UdfWriter.AllocatePartition(FileTree);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		if (bUseIso)
		{
			iResult = IsoWriter.AllocateFileData(Progress,FileTree);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);

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
		}

		if (bUseUdf)
		{
			iResult = UdfWriter.WritePartition(FileTree);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

		// FIXME: The files needs to be written even if not using a ISO9660 file system.
		//		  Perhaps separate the file data from the directory entries?
		if (bUseIso)
		{
			iResult = IsoWriter.WriteFileData(FileTree,Progress);
			if (iResult != RESULT_OK)
				return Fail(iResult,szFullPath);
		}

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
