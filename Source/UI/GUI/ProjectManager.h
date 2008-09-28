/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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
#include <vector>
#include <ckcore/crcstream.hh>
#include <ckfilesystem/discimagehelper.hh>
#include "../../Common/XMLProcessor.h"
#include "SpaceMeter.h"
#include "TreeManager.h"
#include "CustomContainer.h"
#include "AdvancedProgress.h"
#include "ConfirmFileReplaceDlg.h"

// Specifies what column index each data column has.
#define COLUMN_SUBINDEX_NAME				0
#define COLUMN_SUBINDEX_SIZE				1
#define COLUMN_SUBINDEX_TYPE				2
#define COLUMN_SUBINDEX_MODIFIED			3
#define COLUMN_SUBINDEX_PATH				4

// Specifies what column index each audio column has.
#define COLUMN_SUBINDEX_TRACK				0
#define COLUMN_SUBINDEX_TITLE				1
#define COLUMN_SUBINDEX_LENGTH				2
#define COLUMN_SUBINDEX_LOCATION			3

#define PROJECTVIEWTYPE_DATA				0
#define PROJECTVIEWTYPE_AUDIO				1

#define PROJECTTYPE_DATA					0
#define PROJECTTYPE_AUDIO					1
#define PROJECTTYPE_MIXED					2

// What project file version does this build use.
#define PROJECTMANAGER_FILEVERSION			3

/// Class for project content management.
/**
	Implements core project functionallity such as creating and loading projects,
	adding, removing and moving files.
*/
class CProjectManager
{
private:
	int m_iProjectType;
	int m_iViewType;
	int m_iActiveView;				// Tells us what view (tree or list) that last had focus.
	bool m_bProjectDVD;				// Is set to true if the project is to be recorded on a DVD media.
	bool m_bModified;				// Set to true if the project has been modified since the last save.

	CProjectNode *m_pActionNode;	// Used for random internal temporary purposes.

	CSplitterWindow *m_pProjectView;
	CCustomContainer *m_pContainer;
	CSpaceMeter *m_pSpaceMeter;
	CListViewCtrl *m_pListView;
	CTreeViewCtrlEx *m_pTreeView;

	CProjectNode *m_pMixDataNode;
	CProjectNode *m_pMixAudioNode;

	void SetupDataListView();
	void SetupAudioListView();

	bool DecodeAudioTrack(const TCHAR *szFullPath,const TCHAR *szFullTempPath,
		CAdvancedProgress *pProgress);

	bool VerifyLocalFiles(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack,
		CAdvancedProgress *pProgress,TCHAR *szFileNameBuffer,int iPathStripLen,
		ckcore::Progresser &FileProgresser,unsigned __int64 &uiFailCount,
		std::map<tstring,tstring> &FilePathMap);

	bool GenerateNewFolderName(CProjectNode *pParent,TCHAR *szFolderName,
		unsigned int uiFolderNameSize);

	void CloseProject();
	void SaveProjectData(CXMLProcessor *pXML);
	bool LoadProjectData(CXMLProcessor *pXML);
	void SaveProjectFileSys(CXMLProcessor *pXML);
	bool LoadProjectFileSys(CXMLProcessor *pXML);
	void SaveProjectISO(CXMLProcessor *pXML);
	bool LoadProjectISO(CXMLProcessor *pXML);
	void SaveProjectFields(CXMLProcessor *pXML);
	bool LoadProjectFields(CXMLProcessor *pXML);
	void SaveProjectBoot(CXMLProcessor *pXML);
	bool LoadProjectBoot(CXMLProcessor *pXML);

	enum eActiveView
	{
		AV_TREE,
		AV_LIST
	};

public:
	/// Class for performing file transactions within a project.
	/**
		Implements support for adding and moving files to/within a project.
	*/
	class CFileTransaction
	{
	public:
		enum eMode
		{
			MODE_NORMAL,
			MODE_IMPORT
		};

	private:
		eMode m_Mode;
		CConfirmFileReplaceDlg m_ReplaceDlg;

		void AddFilesInFolder(CProjectNode *pParentNode,std::vector<CProjectNode *> &FolderStack);

		bool AddDataFile(CProjectNode *pParentNode,const TCHAR *szFileName,
			const TCHAR *szFilePath,const TCHAR *szFullPath,FILETIME *pFileTime,
			unsigned __int64 uiSize);
		CItemData *AddDataFile(CProjectNode *pParentNode,const TCHAR *szFullPath);
		CProjectNode *AddFolder(CProjectNode *pParentNode,const TCHAR *szFolderName,
			const TCHAR *szFolderPath,const TCHAR *szFullPath,FILETIME *pFileTime);
		CProjectNode *AddFolder(CProjectNode *pParentNode,const TCHAR *szFullPath);
		bool AddAudioFile(CProjectNode *pParentNode,const TCHAR *szFullPath);

	public:
		CFileTransaction(eMode Mode = MODE_NORMAL);
		~CFileTransaction();

		bool AddFile(const TCHAR *szFullPath,CProjectNode *pTargetNode = NULL);
		CItemData *AddFile(const TCHAR *szFullPath,const TCHAR *szProjectPath);

		bool MoveFile(CProjectNode *pItemParent,CItemData *pItemData,CProjectNode *pNewParent);
		bool MoveFileToCurrent(CProjectNode *pItemParent,CItemData *pItemData);
	};

	CProjectManager();
	~CProjectManager();

	LRESULT OnNewFolder(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnRename(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnRemove(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	void AssignControls(CSplitterWindow *pProjectView,CCustomContainer *pContainer,
		CSpaceMeter *pSpaceMeter,CListViewCtrl *pListView,CTreeViewCtrlEx *pTreeView);

	void EnableAll(int iID,bool bEnable,HMENU hMenu = NULL);

	void NewDataProject(bool bDVD);
	void NewAudioProject();
	void NewMixedProject();

	void DataSelected();
	void AudioSelected();

	bool ListAddNewFolder();
	bool TreeAddNewFolder(CProjectNode *pParentNode);

	void RemoveFile(CProjectNode *pParentNode,CItemData *pItemData);

	void ListRemoveSel();
	void TreeRemoveNode(CProjectNode *pNode);
	void NotifyListSelChanged(unsigned int uiSelCount);
	void NotifyTreeSelChanged(CProjectNode *pNode);
	void TreeSetActionNode(CProjectNode *pNode);

	void TreeSetActive();
	void ListSetActive();

	void DeleteImportedItems();

	int GetViewType();
	int GetProjectType();
	void GetProjectContents(unsigned __int64 &uiFileCount,unsigned __int64 &uiFolderCount,
		unsigned __int64 &uiTrackCount);
	unsigned __int64 GetProjectSize();
	bool GetProjectDVDState();
	//unsigned __int64 GetProjectAudioSize();
	CProjectNode *GetMixDataRootNode();
	CProjectNode *GetMixAudioRootNode();

	void ListAudioTracks(CListViewCtrl *pListView);
	void GetAudioTracks(std::vector<TCHAR *> &AudioTracks);
	bool DecodeAudioTracks(std::vector<TCHAR *> &AudioTracks,
		std::vector<TCHAR *> &DecodedTracks,CAdvancedProgress *pProgress);
	bool SaveCDText(const TCHAR *szFullPath);

	bool VerifyCompilation(CAdvancedProgress *pProgress,const TCHAR *szDriveLetter,
		std::map<tstring,tstring> &FilePathMap);

	void SetDiscLabel(TCHAR *szLabelName);

	void SetModified(bool bModified);
	bool GetModified();

	bool SaveProject(const TCHAR *szFullPath);
	bool LoadProject(const TCHAR *szFullPath);
};

extern CProjectManager g_ProjectManager;
