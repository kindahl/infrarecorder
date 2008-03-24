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
#include <vector>
#include <deque>
#include "SectorManager.h"
#include "SectorStream.h"
#include "FileTree.h"
#include "Udf.h"

namespace ckFileSystem
{
	class CUdfWriter : public ISectorClient
	{
	private:
		// Identifiers of different sector ranges.
		enum
		{
			SR_INITIALDESCRIPTORS,
			SR_MAINDESCRIPTORS,
			SR_FILESETCONTENTS
		};

		CLog *m_pLog;
		CSectorOutStream *m_pOutStream;
		CSectorManager *m_pSectorManager;

		// File system attributes.
		bool m_bUseFileTimes;

		// Different standard implementations.
		CUdf *m_pUdf;

		// Sizes of different structures.
		unsigned __int64 m_uiPartLength;
		tUdfExtentAd m_MainVolDescSeqExtent;
		tUdfExtentAd m_ReserveVolDescSeqExtent;

		// The time when this object was created.
		SYSTEMTIME m_stImageCreate;

		// File system preparation functions.
		void CalcLocalNodeLengths(std::vector<CFileTreeNode *> &DirNodeStack,
			CFileTreeNode *pLocalNode);
		void CalcNodeLengths(CFileTree &FileTree);

		unsigned __int64 CalcIdentSize(CFileTreeNode *pLocalNode);
		unsigned __int64 CalcNodeSizeTotal(CFileTreeNode *pLocalNode);
		unsigned __int64 CalcNodeLinksTotal(CFileTreeNode *pLocalNode);
		unsigned __int64 CalcParitionLength(CFileTree &FileTree);

		// Write functions.
		bool WriteLocalParitionDir(std::deque<CFileTreeNode *> &DirNodeQueue,
			CFileTreeNode *pLocalNode,unsigned long &ulCurPartSec,unsigned __int64 &uiUniqueIdent);
		bool WritePartitionEntries(CFileTree &FileTree);

	public:
		CUdfWriter(CLog *pLog,CSectorOutStream *pOutStream,
			CSectorManager *pSectorManager,CUdf *pUdf,bool bUseFileTimes);
		~CUdfWriter();

		int AllocateHeader();
		int AllocatePartition(CFileTree &FileTree);

		int WriteHeader();
		int WritePartition(CFileTree &FileTree);
		int WriteTail();
	};
};
