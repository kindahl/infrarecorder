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
#include "../../Common/StringUtil.h"
#include "../../Common/Log.h"
#include "FileSet.h"
#include "FileTree.h"
#include "IfoReader.h"

#define DVDVIDEO_BLOCK_SIZE			2048

namespace ckFileSystem
{
	class CDvdVideo
	{
	private:
		enum eFileSetType
		{
			FST_INFO,
			FST_BACKUP,
			FST_MENU,
			FST_TITLE
		};

		CLog *m_pLog;

		unsigned __int64 SizeToDvdLen(unsigned __int64 uiFileSize);

		CFileTreeNode *FindVideoNode(CFileTree &FileTree,eFileSetType Type,unsigned long ulNumber);

		bool GetTotalTitlesSize(tstring &FilePath,eFileSetType Type,unsigned long ulNumber,
			unsigned __int64 &uiFileSize);
		bool ReadFileSetInfoRoot(CFileTree &FileTree,CIfoVmgData &VmgData,
			std::vector<unsigned long> &TitleSetSectors);
		bool ReadFileSetInfo(CFileTree &FileTree,std::vector<unsigned long> &TitleSetSectors);

	public:
		CDvdVideo(CLog *pLog);
		~CDvdVideo();

		bool PrintFilePadding(CFileTree &FileTree);

		bool CalcFilePadding(CFileTree &FileTree);
	};
};
