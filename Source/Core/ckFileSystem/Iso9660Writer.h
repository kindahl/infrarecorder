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
#include "../../Common/StringUtil.h"
#include "../../Common/Progress.h"
#include "SectorManager.h"
#include "FileSet.h"
#include "FileTree.h"
#include "Iso9660.h"
#include "Joliet.h"
#include "ElTorito.h"

#define ISO9660WRITER_IO_BUFFER_SIZE			0x10000
#define ISO9660WRITER_FILENAME_BUFFER_LEN		206			// Must be enough to hold the largest possible string using
															// any of the supported file system extensions.
namespace ckFileSystem
{
	class CIso9660Writer : public ISectorClient
	{
	private:
		// Identifiers of different sector ranges.
		enum
		{
			SR_DESCRIPTORS,
			SR_BOOTCATALOG,
			SR_BOOTDATA,
			SR_PATHTABLE_NORMAL_L,
			SR_PATHTABLE_NORMAL_M,
			SR_PATHTABLE_JOLIET_L,
			SR_PATHTABLE_JOLIET_M
		};

		// Identifiers the two different types of system directories.
		enum eSysDirType
		{
			TYPE_CURRENT,
			TYPE_PARENT
		};

		CLog *m_pLog;
		CSectorOutStream *m_pOutStream;
		CSectorManager *m_pSectorManager;

		// Different standard implementations.
		CIso9660 *m_pIso9660;
		CJoliet *m_pJoliet;
		CElTorito *m_pElTorito;

		// File system attributes.
		bool m_bUseJoliet;
		bool m_bUseFileTimes;

		// Sizes of different structures.
		unsigned __int64 m_uiPathTableSizeNormal;
		unsigned __int64 m_uiPathTableSizeJoliet;

		// The time when this object was created.
		SYSTEMTIME m_stImageCreate;

		// File system preparation functions.
		bool CalcPathTableSize(CFileSet &Files,bool bJolietTable,
			unsigned __int64 &uiPathTableSize,CProgressEx &Progress);

		bool CalcLocalDirEntrySize(CFileTreeNode *pLocalNode,bool bJoliet,int iLevel,
			unsigned long &ulDirSecSize);
		bool CalcLocalFileSysData(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
			CFileTreeNode *pLocalNode,int iLevel,unsigned __int64 &uiSecOffset,
			CProgressEx &Progress);
		bool CalcFileSysData(CFileTree &FileTree,unsigned __int64 uiStartSec,
			unsigned __int64 &uiLastSec,CProgressEx &Progress);

		// Write functions.
		bool WritePathTable(CFileSet &Files,CFileTree &FileTree,bool bJolietTable,
			bool bMSBF,CProgressEx &Progress);

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

	public:
		CIso9660Writer(CLog *pLog,CSectorOutStream *pOutStream,
			CSectorManager *pSectorManager,CIso9660 *pIso9660,CJoliet *pJoliet,
			CElTorito *pElTorito,bool bUseFileTimes,bool bUseJoliet);
		~CIso9660Writer();

		int AllocateHeader();
		int AllocatePathTables(CProgressEx &Progress,CFileSet &Files);
		int AllocateFileData(CProgressEx &Progress,CFileTree &FileTree);

		int WriteHeader(CFileSet &Files,CFileTree &FileTree,CProgressEx &Progress);
		int WritePathTables(CFileSet &Files,CFileTree &FileTree,CProgressEx &Progress);
		int WriteFileData(CFileTree &FileTree,CProgressEx &Progress);

		// Helper functions.
		bool ValidateTreeNode(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
			CFileTreeNode *pNode,int iLevel);
		bool ValidateTree(CFileTree &FileTree);
	};
};
