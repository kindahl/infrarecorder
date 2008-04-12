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
#include "FileTree.h"

namespace ckFileSystem
{
	CFileTree::CFileTree(CLog *pLog) :
		m_pLog(pLog),m_pRootNode(NULL)
	{
		m_ulDirCount = 0;
		m_ulFileCount = 0;
	}

	CFileTree::~CFileTree()
	{
		if (m_pRootNode != NULL)
		{
			delete m_pRootNode;
			m_pRootNode = NULL;
		}
	}

	CFileTreeNode *CFileTree::GetRoot()
	{
		return m_pRootNode;
	}

	CFileTreeNode *CFileTree::GetChildFromFileName(CFileTreeNode *pParent,const TCHAR *szFileName)
	{
		std::vector<CFileTreeNode *>::const_iterator itChild;
		for (itChild = pParent->m_Children.begin(); itChild !=
			pParent->m_Children.end(); itChild++)
		{
			if (!lstrcmp(szFileName,(*itChild)->m_FileName.c_str()))
				return *itChild;
		}

		return NULL;
	}

	void CFileTree::AddFileFromPath(const CFileDescriptor &File)
	{
		size_t iDirPathLen = File.m_InternalPath.length(),iPrevDelim = 0,iPos;
		tstring CurDirName;
		CFileTreeNode *pCurNode = m_pRootNode;

		for (iPos = 0; iPos < iDirPathLen; iPos++)
		{
			if (File.m_InternalPath.c_str()[iPos] == '/')
			{
				if (iPos > (iPrevDelim + 1))
				{
					// Obtain the name of the current directory.
					CurDirName.erase();
					for (size_t j = iPrevDelim + 1; j < iPos; j++)
						CurDirName.push_back(File.m_InternalPath.c_str()[j]);

					pCurNode = GetChildFromFileName(pCurNode,CurDirName.c_str());
					if (pCurNode == NULL)
						m_pLog->AddLine(_T("  Error: Unable to find child node \"%s\"."),CurDirName.c_str());
				}

				iPrevDelim = iPos;
			}
		}

		// We now have our parent.
		const TCHAR *szFileName = File.m_InternalPath.c_str() + iPrevDelim + 1;

		// Check if imported.
		unsigned char ucImportFlag = 0;
		void *pImportData = NULL;
		if (File.m_ucFlags & CFileDescriptor::FLAG_IMPORTED)
		{
			ucImportFlag = CFileTreeNode::FLAG_IMPORTED;
			pImportData = File.m_pData;
		}

		if (File.m_ucFlags & CFileDescriptor::FLAG_DIRECTORY)
		{
			pCurNode->m_Children.push_back(new CFileTreeNode(pCurNode,szFileName,
				File.m_ExternalPath.c_str(),0,true,0,CFileTreeNode::FLAG_DIRECTORY | ucImportFlag,
				pImportData));

			m_ulDirCount++;
		}
		else
		{
			pCurNode->m_Children.push_back(new CFileTreeNode(pCurNode,szFileName,
				File.m_ExternalPath.c_str(),File.m_uiFileSize,true,0,ucImportFlag,pImportData));

			m_ulFileCount++;
		}
	}

	void CFileTree::CreateFromFileSet(const CFileSet &Files)
	{
		if (m_pRootNode != NULL)
			delete m_pRootNode;

		m_pRootNode = new CFileTreeNode(NULL,_T(""),_T(""),0,true,0,
			CFileTreeNode::FLAG_DIRECTORY);

		CFileSet::const_iterator itFile;
		for (itFile = Files.begin(); itFile != Files.end(); itFile++)
			AddFileFromPath(*itFile);
	}

	CFileTreeNode *CFileTree::GetNodeFromPath(const CFileDescriptor &File)
	{
		size_t iDirPathLen = File.m_InternalPath.length(),iPrevDelim = 0,iPos;
		tstring CurDirName;
		CFileTreeNode *pCurNode = m_pRootNode;

		for (iPos = 0; iPos < iDirPathLen; iPos++)
		{
			if (File.m_InternalPath.c_str()[iPos] == '/')
			{
				if (iPos > (iPrevDelim + 1))
				{
					// Obtain the name of the current directory.
					CurDirName.erase();
					for (size_t j = iPrevDelim + 1; j < iPos; j++)
						CurDirName.push_back(File.m_InternalPath.c_str()[j]);

					pCurNode = GetChildFromFileName(pCurNode,CurDirName.c_str());
					if (pCurNode == NULL)
						m_pLog->AddLine(_T("  Error: Unable to find child node \"%s\"."),CurDirName.c_str());
				}

				iPrevDelim = iPos;
			}
		}

		// We now have our parent.
		const TCHAR *szFileName = File.m_InternalPath.c_str() + iPrevDelim + 1;

		return GetChildFromFileName(pCurNode,szFileName);
	}

	CFileTreeNode *CFileTree::GetNodeFromPath(const TCHAR *szInternalPath)
	{
		size_t iDirPathLen = lstrlen(szInternalPath),iPrevDelim = 0,iPos;
		tstring CurDirName;
		CFileTreeNode *pCurNode = m_pRootNode;

		for (iPos = 0; iPos < iDirPathLen; iPos++)
		{
			if (szInternalPath[iPos] == '/')
			{
				if (iPos > (iPrevDelim + 1))
				{
					// Obtain the name of the current directory.
					CurDirName.erase();
					for (size_t j = iPrevDelim + 1; j < iPos; j++)
						CurDirName.push_back(szInternalPath[j]);

					pCurNode = GetChildFromFileName(pCurNode,CurDirName.c_str());
					if (pCurNode == NULL)
						m_pLog->AddLine(_T("  Error: Unable to find child node \"%s\"."),CurDirName.c_str());
				}

				iPrevDelim = iPos;
			}
		}

		// We now have our parent.
		const TCHAR *szFileName = szInternalPath + iPrevDelim + 1;

		return GetChildFromFileName(pCurNode,szFileName);
	}

	/**
		@eturn the number of files in the tree, fragmented files are counted once.
	*/
	unsigned long CFileTree::GetDirCount()
	{
		return m_ulDirCount;
	}

	/**
		@return the number of directories in the tree, the root is not included.
	*/
	unsigned long CFileTree::GetFileCount()
	{
		return m_ulFileCount;
	}

#ifdef _DEBUG
	void CFileTree::PrintLocalTree(std::vector<std::pair<CFileTreeNode *,int> > &DirNodeStack,
								   CFileTreeNode *pLocalNode,int iIndent)
	{
		std::vector<CFileTreeNode *>::const_iterator itFile;
		for (itFile = pLocalNode->m_Children.begin(); itFile !=
			pLocalNode->m_Children.end(); itFile++)
		{
			if ((*itFile)->m_ucFileFlags & CFileTreeNode::FLAG_DIRECTORY)
			{
				DirNodeStack.push_back(std::make_pair(*itFile,iIndent));
			}
			else
			{
				for (int i = 0; i < iIndent; i++)
					m_pLog->AddString(_T(" "));

				m_pLog->AddString(_T("<f>"));
				m_pLog->AddString((*itFile)->m_FileName.c_str());
				m_pLog->AddLine(_T(" (%I64d:%I64d,%I64d:%I64d,%I64d:%I64d)"),(*itFile)->m_uiDataPosNormal,
					(*itFile)->m_uiDataSizeNormal,(*itFile)->m_uiDataPosJoliet,
					(*itFile)->m_uiDataSizeJoliet,(*itFile)->m_uiUdfSize,(*itFile)->m_uiUdfSizeTot);
			}
		}
	}

	void CFileTree::PrintTree()
	{
		if (m_pRootNode == NULL)
			return;

		CFileTreeNode *pCurNode = m_pRootNode;
		int iIndent = 0;

		m_pLog->AddLine(_T("CFileTree::PrintTree"));
		m_pLog->AddLine(_T("  <root> (%I64d:%I64d,%I64d:%I64d,%I64d:%I64d)"),pCurNode->m_uiDataPosNormal,
				pCurNode->m_uiDataSizeNormal,pCurNode->m_uiDataPosJoliet,
				pCurNode->m_uiDataSizeJoliet,pCurNode->m_uiUdfSize,pCurNode->m_uiUdfSizeTot);

		std::vector<std::pair<CFileTreeNode *,int> > DirNodeStack;
		PrintLocalTree(DirNodeStack,pCurNode,4);

		while (DirNodeStack.size() > 0)
		{ 
			pCurNode = DirNodeStack[DirNodeStack.size() - 1].first;
			iIndent = DirNodeStack[DirNodeStack.size() - 1].second;

			DirNodeStack.pop_back();

			// Print the directory name.
			for (int i = 0; i < iIndent; i++)
				m_pLog->AddString(_T(" "));

			m_pLog->AddString(_T("<d>"));
			m_pLog->AddString(pCurNode->m_FileName.c_str());
			m_pLog->AddLine(_T(" (%I64d:%I64d,%I64d:%I64d,%I64d:%I64d)"),pCurNode->m_uiDataPosNormal,
				pCurNode->m_uiDataSizeNormal,pCurNode->m_uiDataPosJoliet,
				pCurNode->m_uiDataSizeJoliet,pCurNode->m_uiUdfSize,pCurNode->m_uiUdfSizeTot);

			PrintLocalTree(DirNodeStack,pCurNode,iIndent + 2);
		}
	}
#endif
};
