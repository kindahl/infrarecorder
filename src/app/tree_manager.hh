/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <list>
#include <vector>
#include <ckfilesystem/fileset.hh>
#include <ckfilesystem/isoreader.hh>
#include <ckfilesystem/isowriter.hh>
#include <base/xml_processor.hh>
#include <base/string_container.hh>

#define PROJECTITEM_FLAG_ISFOLDER					1
#define PROJECTITEM_FLAG_ISLOCKED					2
#define PROJECTITEM_FLAG_ISIMPORTED					4
#define PROJECTITEM_FLAG_ISDVDVIDEO					8
#define PROJECTITEM_FLAG_ISPROJECTROOT				16

class CProjectNode;

// This structure represents a file (or folder) inside the project view.
class CItemData
{
public:
	class CAudioData
	{
	public:
		unsigned __int64 uiTrackLength;
		TCHAR szTrackTitle[160];
		TCHAR szTrackArtist[160];

		CAudioData()
		{
			uiTrackLength = 0;
			szTrackTitle[0] = '\0';
			szTrackArtist[0] = '\0';
		}
	};

	typedef ckfilesystem::IsoImportData CIsoData;

private:
	TCHAR m_szFileName[MAX_PATH];	// File name in the project (disc image).
	TCHAR m_szFilePath[MAX_PATH];	// File path in the project (disc image).

	void FileNameChanged();
	void FilePathChanged();

	// Only allocated when needed.
	CAudioData *m_pAudioData;
	CIsoData *m_pIsoData;

public:
	CItemData();
	~CItemData();

	//TCHAR szFileName[MAX_PATH];		// File name in the project (disc image).
	//TCHAR szFilePath[MAX_PATH];		// File path in the project (disc image).
	TCHAR szFullPath[MAX_PATH];			// Real file path on the harddrive.
	TCHAR szFileType[80];
	unsigned short usFileDate;
	unsigned short usFileTime;
	unsigned __int64 uiSize;		// In data and mixed-mode projects this
									// variable holds the file size in bytes.
									// In audio projects it holds the same
									// information as uiTrackLength.

	// Only used in audio mode. Audio track length in milliseconds.
	// UPDATE: An object with this information is only created when needed.
	/*unsigned __int64 uiTrackLength;
	TCHAR szTrackTitle[160];
	TCHAR szTrackArtist[160];*/

	unsigned char ucFlags;

	// Getters and setters.
	void SetFileName(const TCHAR *szFileName);
	const TCHAR *GetFileName();
	void SetFilePath(const TCHAR *szFilePath);
	const TCHAR *GetFilePath();

	TCHAR *BeginEditFileName();
	void EndEditFileName();
	TCHAR *BeginEditFilePath();
	void EndEditFilePath();

	bool HasAudioData();
	CAudioData *GetAudioData();
	bool HasIsoData();
	CIsoData *GetIsoData();
};

class CProjectNode
{
public:
	std::list<CProjectNode *> m_Children;
	std::list<CItemData *> m_Files;
	CProjectNode *m_pParent;
    CItemData *pItemData;
	int iIconIndex;
	HTREEITEM m_hTreeItem;

    CProjectNode(CProjectNode *pParent)
    {
		m_pParent = pParent;
		m_hTreeItem = NULL;

		// Initialize the default data.
		pItemData = new CItemData();
		pItemData->ucFlags = PROJECTITEM_FLAG_ISFOLDER;
    }
        
    ~CProjectNode()
    {
        delete pItemData;

		// Free the children.
		std::list <CProjectNode *>::iterator itNodeObject;
		for (itNodeObject = m_Children.begin(); itNodeObject != m_Children.end(); itNodeObject++)
			delete *itNodeObject;

		m_Children.clear();

		// Free the file data.
		std::list <CItemData *>::iterator itFileObject;
		for (itFileObject = m_Files.begin(); itFileObject != m_Files.end(); itFileObject++)
			delete *itFileObject;

		m_Files.clear();
    }

	void Sort(unsigned int uiSortColumn,bool bSortUp,bool bSortAudio);
};

class CChildComparator
{
private:
	unsigned int m_uiSortColumn;
	bool m_bSortUp;
	bool m_bSortAudio;

public:
	CChildComparator(unsigned int uiSortColumn,bool bSortUp,bool bSortAudio)
	{
		m_uiSortColumn = uiSortColumn;
		m_bSortUp = bSortUp;
		m_bSortAudio = bSortAudio;
	}

	bool operator() (const CProjectNode *pSafeNode1,const CProjectNode *pSafeNode2);
};

class CFileComparator
{
private:
	unsigned int m_uiSortColumn;
	bool m_bSortUp;
	bool m_bSortAudio;

public:
	CFileComparator(unsigned int uiSortColumn,bool bSortUp,bool bSortAudio)
	{
		m_uiSortColumn = uiSortColumn;
		m_bSortUp = bSortUp;
		m_bSortAudio = bSortAudio;
	}

	bool operator() (const CItemData *pSafeItemData1,const CItemData *pSafeItemData2);
};

class CTreeManager
{
private:
	CTreeViewCtrlEx *m_pTreeView;
	CListViewCtrl *m_pListView;

	CProjectNode *m_pRootNode;
	CProjectNode *m_pCurrentNode;

	TCHAR m_szCurrentPath[MAX_PATH];

	HTREEITEM GetTreeChildFromParent(HTREEITEM hParentItem,TCHAR *szText);
	bool HasChildren(HTREEITEM hItem,bool bHasChildren);

	CProjectNode *GetDirFromParent(CProjectNode *pParent,const TCHAR *szName);
	CProjectNode *NodalizePath(const TCHAR *szPath);
	CProjectNode *GetChildFromParent(CProjectNode *pParentNode,const TCHAR *szText);

	void ListNode(CProjectNode *pNode);

	void RebuildLocalPaths(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack);
	unsigned __int64 GetLocalSizeFromNode(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack);

	void SaveLocalNodeFileData(CXmlProcessor *pXml,CProjectNode *pNode,
		std::vector<CProjectNode *> &FolderStack,unsigned int &uiFileCount,
		unsigned int uiRootLength);

	void GetLocalPathList(ckfilesystem::FileSet &Files,CProjectNode *pNode,
		std::vector<CProjectNode *> &FolderStack,int iPathStripLen);

	void GetLocalNodeContents(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack,
		unsigned __int64 &uiFileCount,unsigned __int64 &uiNodeCount);
	void RecursiveLocalSetFlags(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack,
		unsigned char ucFlags);

public:
	CTreeManager();
	~CTreeManager();

	void AssignControls(CTreeViewCtrlEx *pTreeView,CListViewCtrl *pListView);
	void CreateTree(const TCHAR *szRootName,int iImage);
	void DestroyTree();

	HTREEITEM AddTreeNode(HTREEITEM hParentItem,CProjectNode *pNode);
	CProjectNode *AddPath(const TCHAR *szPath);
	CProjectNode *InsertVirtualRoot(const TCHAR *szNodeName,int iImage);
	void SelectPath(const TCHAR *szPath);
	void Refresh();
	void RebuildPaths(const TCHAR *szStartPath);
	bool RemoveEntry(CProjectNode *pNode);
	bool RemoveEntry(CProjectNode *pNode,CItemData *pItemData);
	bool RemoveEntry(const TCHAR *szLocalPath,const TCHAR *szFullPath);
	bool MoveEntry(CProjectNode *pParent,CItemData *pItemData,CProjectNode *pNewParent);

	bool IsSubNode(CProjectNode *pNode1,CProjectNode *pNode2);

	CItemData *GetChildItem(CProjectNode *pParent,const TCHAR *szName);
	CProjectNode *GetChildNode(CProjectNode *pParent,const TCHAR *szName);

	CProjectNode *ResolveNode(CProjectNode *pParent,CItemData *pNodeItem);

	void GetCurrentPath(TCHAR *szCurrentPath);
	void SetCurrentPath(const TCHAR *szCurrentPath);
	CProjectNode *GetCurrentNode();
	CProjectNode *GetRootNode();
	CProjectNode *GetDirFromParent(CProjectNode *pParent,TCHAR *szText);
	CProjectNode *GetNodeFromPath(const TCHAR *szPath);

	unsigned __int64 GetNodeSize(CProjectNode *pNode);
	unsigned __int64 GetNodeSize(CProjectNode *pParentNode,CItemData *pItemData);
	void GetNodeContents(CProjectNode *pRootNode,unsigned __int64 &uiFileCount,
		unsigned __int64 &uiNodeCount);
	void RecursiveSetFlags(CProjectNode *pRootNode,unsigned char ucFlags);
	void DeleteImportedItems(CProjectNode *pRootNode);
	void GetNodeFullPaths(CProjectNode *pRootNode,std::vector<TCHAR *> &FullPaths);
	void GetNodeFiles(CProjectNode *pNode,std::vector<CItemData *> &Files);
	void ListNodeFiles(CProjectNode *pNode,CListViewCtrl *pListView);

	bool HasExtraAudioData(CProjectNode *pNode);

	// Save/load routines.
	void SaveNodeFileData(CXmlProcessor *pXml,CProjectNode *pRootNode);
	void SaveNodeAudioData(CXmlProcessor *pXml,CProjectNode *pRootNode);
	bool LoadNodeFileData(CXmlProcessor *pXml,CProjectNode *pRootNode);
	bool LoadNodeAudioData(CXmlProcessor *pXml,CProjectNode *pRootNode,int iProjectType);

	void GetPathList(ckfilesystem::FileSet &Files,CProjectNode *pRootNode,int iPathStripLen = 0);

	void ImportLocalIsoTree(ckfilesystem::IsoTreeNode *pLocalIsoNode,CProjectNode *pLocalNode,
		std::vector<std::pair<ckfilesystem::IsoTreeNode *,CProjectNode *> > &FolderStack);
	void ImportIsoTree(ckfilesystem::IsoTreeNode *pIsoRootNode,CProjectNode *pRootNode);
};

extern CTreeManager g_TreeManager;
