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
#include <queue>
#include "../../Common/StringUtil.h"
#include "../../Common/FileStream.h"
#include "../../Common/Progress.h"
#include "../../Common/Log.h"
#include "FileSet.h"
#include "FileTree.h"
#include "SectorStream.h"
#include "Const.h"
#include "Iso9660.h"
#include "Joliet.h"
#include "ElTorito.h"
#include "Udf.h"

#define DISCIMAGEWRITER_IO_BUFFER_SIZE			0x10000

namespace ckFileSystem
{
	class CDiscImageWriter
	{
	public:
		enum eFileSystem
		{
			FS_ISO9660,
			FS_ISO9660_JOLIET,
			FS_ISO9660_UDF,
			FS_ISO9660_UDF_JOLIET,
			FS_UDF,
			FS_DVDVIDEO
		};

	private:
		CLog *m_pLog;

		// What file system should be created.
		eFileSystem m_FileSystem;

		// Different standard implementations.
		CIso9660 m_Iso9660;
		CJoliet m_Joliet;
		CElTorito m_ElTorito;
		CUdf m_Udf;

		bool CalcLocalFileSysData(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
			CFileTreeNode *pLocalNode,int iLevel,unsigned __int64 &uiSecOffset,CProgressEx &Progress);
		bool CalcFileSysData(CFileTree &FileTree,CProgressEx &Progress,
			unsigned __int64 uiStartSec,unsigned __int64 &uiLastSec);

		int WriteFileNode(CSectorOutStream &OutStream,CFileTreeNode *pNode,
			CProgressEx &Progress,CFilesProgress &FilesProgress);
		int WriteLocalFileData(CSectorOutStream &OutStream,
			std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
			CFileTreeNode *pLocalNode,int iLevel,CProgressEx &Progress,CFilesProgress &FilesProgress);
		int WriteFileData(CSectorOutStream &OutStream,CFileTree &FileTree,CProgressEx &Progress,
			CFilesProgress &FilesProgress);

		void GetInternalPath(CFileTreeNode *pChildNode,tstring &NodePath,
			bool bExternalPath,bool bJoliet);
		void CreateLocalFilePathMap(CFileTreeNode *pLocalNode,
			std::vector<CFileTreeNode *> &DirNodeStack,
			std::map<tstring,tstring> &FilePathMap,bool bJoliet);
		void CreateFilePathMap(CFileTree &FileTree,std::map<tstring,tstring> &FilePathMap,bool bJoliet);

		int Fail(int iResult,CSectorOutStream &OutStream);

	public:
		CDiscImageWriter(CLog *pLog,eFileSystem FileSystem);
		~CDiscImageWriter();	

		int Create(CSectorOutStream &OutStream,CFileSet &Files,CProgressEx &Progress,
			unsigned long ulSectorOffset = 0,std::map<tstring,tstring> *pFilePathMap = NULL);

		// File system modifiers, mixed set for Joliet, UDF and ISO9660.
		void SetVolumeLabel(const TCHAR *szLabel);
		void SetTextFields(const TCHAR *szSystem,const TCHAR *szVolSetIdent,
			const TCHAR *szPublIdent,const TCHAR *szPrepIdent);
		void SetFileFields(const TCHAR *ucCopyFileIdent,const TCHAR *ucAbstFileIdent,
			const TCHAR *ucBiblIdent);
		void SetInterchangeLevel(CIso9660::eInterLevel InterLevel);
		void SetIncludeFileVerInfo(bool bIncludeInfo);
		void SetPartAccessType(CUdf::ePartAccessType AccessType);
		void SetRelaxMaxDirLevel(bool bRelaxRestriction);
		void SetLongJolietNames(bool bEnable);

		bool AddBootImageNoEmu(const TCHAR *szFullPath,bool bBootable,
			unsigned short usLoadSegment,unsigned short usSectorCount);
		bool AddBootImageFloppy(const TCHAR *szFullPath,bool bBootable);
		bool AddBootImageHardDisk(const TCHAR *szFullPath,bool bBootable);
	};
};
