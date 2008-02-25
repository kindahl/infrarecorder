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
#include <set>
#include <map>
#include <vector>
#include <string>
#include "../../Common/StringUtil.h"
#include "../../Common/FileStream.h"
#include "../../Common/Progress.h"
#include "../../Common/Log.h"
#include "Iso9660.h"
#include "FileSet.h"
#include "FileTree.h"
#include "SectorStream.h"
#include "Const.h"
#include "Joliet.h"
#include "ElTorito.h"

#define DISCIMAGEWRITER_IO_BUFFER_SIZE			0x10000
#define DISCIMAGEWRITER_FILENAME_BUFFER_LEN		206			// Must be enough to hold the largest possible string using
															// any of the supported file system extensions.

namespace ckFileSystem
{
	class CDiscImageWriter
	{
	private:
		enum eSysDirType
		{
			TYPE_CURRENT,
			TYPE_PARENT
		};

		CLog *m_pLog;

		COutFileStream *m_pFileStream;
		CSectorOutStream *m_pOutStream;

		// Different standard implementations.
		CIso9660 m_Iso9660;
		CJoliet m_Joliet;
		CElTorito m_ElTorito;

		SYSTEMTIME m_stImageCreate;	// The time when the Create function was called.

		// Restrictions.
		bool m_bUseFileTimes;
		bool m_bUseJoliet;

		bool Close();
		bool Fail(const TCHAR *szFullPath);

		bool WritePathTable(CFileSet &Files,CFileTree &FileTree,bool bJolietTable,bool bMSBF,
			CProgressEx &Progress);

		bool WriteSysDirectory(CFileTreeNode *pParent,eSysDirType Type,unsigned long ulDataPos);

		int WriteFileNode(CFileTreeNode *pNode,CProgressEx &Progress,
			CFilesProgress &FilesProgress);
		int WriteLocalDirEntry(CFileTreeNode *pLocalNode,bool bJoliet,int iLevel,
			CProgressEx &Progress,CFilesProgress &FilesProgress);
		int WriteLocalDirectory(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
			CFileTreeNode *pLocalNode,int iLevel,CProgressEx &Progress,
			CFilesProgress &FilesProgress);
		int WriteDirectories(CFileTree &FileTree,CProgressEx &Progress,
			CFilesProgress &FilesProgress);

		bool CalcPathTableSize(CFileSet &Files,bool bJolietTable,unsigned __int64 &uiPathTableSize,
			CProgressEx &Progress);
		bool CalcLocalDirEntrySize(CFileTreeNode *pLocalNode,bool bJoliet,int iLevel,
			unsigned long &ulDirSecSize);
		bool CalcLocalFileSysData(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
			CFileTreeNode *pLocalNode,int iLevel,unsigned __int64 &uiSecOffset,
			CProgressEx &Progress);
		bool CalcFileSysData(CFileTree &FileTree,unsigned __int64 uiStartSec,
			unsigned __int64 &uiLastSec,CProgressEx &Progress);

	public:
		bool ValidateTreeNode(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
			CFileTreeNode *pNode,int iLevel);
		bool ValidateTree(CFileTree &FileTree);

	public:
		CDiscImageWriter(CLog *pLog);
		~CDiscImageWriter();	

		int Create(const TCHAR *szFullPath,CFileSet &Files,CProgressEx &Progress);

		void SetVolumeLabel(const TCHAR *szLabel);
		void SetTextFields(const TCHAR *szSystem,const TCHAR *szVolSetIdent,
			const TCHAR *szPublIdent,const TCHAR *szPrepIdent);
		void SetFileFields(const TCHAR *ucCopyFileIdent,const TCHAR *ucAbstFileIdent,
			const TCHAR *ucBiblIdent);
		void SetInterchangeLevel(CIso9660::eInterLevel InterLevel);
		void SetIncludeFileVerInfo(bool bIncludeInfo);

		bool AddBootImageNoEmu(const TCHAR *szFullPath,bool bBootable,
			unsigned short usLoadSegment,unsigned short usSectorCount);
		bool AddBootImageFloppy(const TCHAR *szFullPath,bool bBootable);
		bool AddBootImageHardDisk(const TCHAR *szFullPath,bool bBootable);
	};
};
