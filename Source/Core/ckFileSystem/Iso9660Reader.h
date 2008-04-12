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
#include "../../Common/Log.h"
#include "../../Common/Stream.h"
#include "../../Common/StringUtil.h"
#include "Iso9660.h"

namespace ckFileSystem
{
	// A CIso9660TreeNode tree node contains every information needed to write
	// an ISO9660 directory record.
	class CIso9660TreeNode
	{
	private:
		CIso9660TreeNode *m_pParent;

	public:
		std::vector<CIso9660TreeNode *> m_Children;

		unsigned char m_ucFileFlags;
		unsigned char m_ucFileUnitSize;
		unsigned char m_ucInterleaveGapSize;
		unsigned short m_usVolSeqNumber;
		unsigned long m_ulExtentLocation;
		unsigned long m_ulExtentLength;

		tDirRecordDateTime m_RecDateTime;

		tstring m_FileName;

		CIso9660TreeNode(CIso9660TreeNode *pParent,const TCHAR *szFileName,
			unsigned long ulExtentLocation,unsigned long ulExtentLength,
			unsigned short usVolSeqNumber,unsigned char ucFileFlags,
			unsigned char ucFileUnitSize,unsigned char ucInterleaveGapSize,
			tDirRecordDateTime &RecDateTime) : m_pParent(pParent)
		{
			m_ucFileFlags = ucFileFlags;
			m_ucFileUnitSize = ucFileUnitSize;
			m_ucInterleaveGapSize = ucInterleaveGapSize;
			m_usVolSeqNumber = usVolSeqNumber;
			m_ulExtentLocation = ulExtentLocation;
			m_ulExtentLength = ulExtentLength;

			memcpy(&m_RecDateTime,&RecDateTime,sizeof(tDirRecordDateTime));

			if (szFileName != NULL)
				m_FileName = szFileName;
		}

		~CIso9660TreeNode()
		{
			// Free the children.
			std::vector<CIso9660TreeNode *>::iterator itNode;
			for (itNode = m_Children.begin(); itNode != m_Children.end(); itNode++)
				delete *itNode;

			m_Children.clear();
		}

		CIso9660TreeNode *GetParent()
		{
			return m_pParent;
		}
	};

	class CIso9660Reader
	{
	private:
		CLog *m_pLog;

		CIso9660TreeNode *m_pRootNode;

		bool ReadDirEntry(CSeekInStream &InStream,
			std::vector<CIso9660TreeNode *> &DirEntries,
			CIso9660TreeNode *pParentNode,bool bJoliet);

	public:
		CIso9660Reader(CLog *pLog);
		~CIso9660Reader();

		bool Read(CSeekInStream &InStream,unsigned long ulStartSector);

		CIso9660TreeNode *GetRoot()
		{
			return m_pRootNode;
		}

	#ifdef _DEBUG
		void PrintLocalTree(std::vector<std::pair<CIso9660TreeNode *,int> > &DirNodeStack,
			CIso9660TreeNode *pLocalNode,int iIndent);
		void PrintTree();
	#endif
	};
};
