/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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
#include <atlcrack.h>	// COMMAND_RANGE_HANDLER_EX
#include "MainView.h"
#include "space_meter.hh"
#include "project_tree_view_ctrl.hh"
#include "project_list_view_ctrl.hh"
#include "custom_container.hh"
#include "label_container.hh"
#include "ProjectManager.h"
#include "PidlHelper.h"
#include "shell_list_view_ctrl.hh"
#include "CtrlMessages.h"
#include "DirectoryMonitor.h"
#include "custom_header_ctrl.hh"
#include "ToolBarManager.h"
#include "custom_toolbar_ctrl.hh"
#include "mini_html_ctrl.hh"
#include "Settings.h"
#include "welcome_pane.hh"

// HACK: Enable doublebuffering on XP systems.
#if (_WIN32_WINNT < 0x501)
#define LVS_EX_DOUBLEBUFFER		0x00010000
#endif

#if (WINVER < 0x0500)
#define MIIM_FTYPE				0x00000100
#endif

// Specified the height of the spacemeter.
#define MAINFRAME_SPACEMETER_HEIGHT			30

// Specifies the index of certain menu items.
#define MENU_ACTIONS_INDEX					2
#define MENU_VIEW_INDEX						3
#define MENU_DISCINFO_INDEX					8
#define MENU_EJECTDISC_INDEX				11
#define MENU_TOOLBARS_INDEX					0

// Specifies which ID the first menu item will have, the second will have + 1 and so on.
#define MENU_DISCINFO_IDBASE				1000
#define MENU_EJECTDISC_IDBASE				2000
#define MENU_DRIVEMENU_MAX					128

#define PROJECTTREE_IMAGEINDEX_FOLDER		0
#define PROJECTTREE_IMAGEINDEX_DATA			1
#define PROJECTTREE_IMAGEINDEX_AUDIO		2
#define PROJECTTREE_IMAGEINDEX_MIXED		3
#define PROJECTTREE_IAMGEINDEX_DVDVIDEO		4

// Tool bar IDs.
#define REBAR_MENUBAR_ID					1
#define REBAR_TOOLBAR_ID					2

// Data structure of each node in the shell tree.
class CShellTreeItemInfo
{
public:

	CShellTreeItemInfo()
	{
		memset(this,0,sizeof(CShellTreeItemInfo));
	}

	LPITEMIDLIST pidlSelf;
	LPITEMIDLIST pidlFullyQual;
	IShellFolder *pParentFolder;
	DWORD dwFlags;
};

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
private:
	HIMAGELIST m_hMainSmallImageList;
	HIMAGELIST m_hMainLargeImageList;
	HIMAGELIST m_hMiniToolBarImageList;
	CCommandBarCtrl m_CmdBar;
	CCustomToolBarCtrl m_ToolBar;
	CHorSplitterWindow m_SpaceMeterView;
	CMainView m_MainView;

	CSplitterWindow m_ExplorerView;
	CSplitterWindow m_ProjectView;
	CSpaceMeter m_SpaceMeter;
	CProjectTreeViewCtrl m_ProjectTreeView;
	CWelcomePane m_WelcomePane;

	CCustomContainer m_ProjectListViewContainer;
	CLabelContainer m_ProjectTreeViewContainer;
	CCustomContainer m_ShellListViewContainer;
	CLabelContainer m_ShellTreeViewContainer;
	CCustomHeaderCtrl m_ProjectListViewHeader;

	HIMAGELIST m_hProjectTreeImageList;

	CDirectoryMonitor m_DirectoryMonitor;
	CShellListViewCtrl *m_pShellListView;
	CTreeViewCtrlEx m_ShellTreeView;

	// If set to false all accelerators will be disabled.
	bool m_bEnableAccel;

	// Uses to keep track of the device index (in the device manager) from the
	// drive menus. For some reason it's not possible to retrieve the data member
	// of a menu item when it has been clicked on.
	ckmmc::Device *m_DriveMenuDeviceMap[MENU_DRIVEMENU_MAX];

	// Is set to true if changing the tree selection is allowed.
	bool m_bEnableTreeSelection;

	//void AutoRunCheck();

	bool m_bEnableAutoRun;
	bool EnableAutoRun(bool bEnable);

	// Set to true if the welcome pane is currently active.
	bool m_bWelcomePane;

	HWND CreateToolBarCtrl();

	void InitializeMainSmallImageList();
	void InitializeMiniToolBarImageList();
	void InitializeMainView();
	bool InitializeShellTreeView();
	void InitializeExplorerView(unsigned int uiSplitterPos);
	void InitializeProjectView(unsigned int uiSplitterPos);
	void InitializeProjectImageLists();

	bool Translate();

	void FillDriveMenus();

	void SetTitleFile(const TCHAR *szFullName);
	void SetTitleNormal();

	bool SaveProjectAs();
	bool SaveProjectPrompt();

	// Shell related.
	bool OpenSpecialFolder(int iFolder);
	bool OpenFolder(TCHAR *szFullPath,HTREEITEM hFrom,bool bExpandMyComp);
	bool EnumTreeObjects(HTREEITEM hParentItem,IShellFolder *pParentFolder,
		LPITEMIDLIST pidlParent);
	bool AddFolder(TCHAR *szFullPath);
	bool RemoveFolder(LPITEMIDLIST pidlFullyQual);
	bool RenameFolder(LPITEMIDLIST pidlOldFullyQual,LPITEMIDLIST pidlNewFullyQual);
	bool MoveFolder(LPITEMIDLIST pidlFullyQual,TCHAR *szNewName);
	bool ItemExist(HTREEITEM hParentItem,LPITEMIDLIST pidl);
	HTREEITEM FindItemFromPath(LPITEMIDLIST pidl);

	void DisplayContextMenuOnShellTree(POINT ptPos,bool bWasWithKeyboard);

public:
	DECLARE_FRAME_WND_CLASS(NULL,IDR_MAINFRAME)

	CMainFrame();
	~CMainFrame();

	// FIXME: This is damn ugly.
	int m_iDefaultProjType;
	int m_iDefaultMedia;
	bool m_bDefaultProjDVDVideo;
	bool m_bDefaultWizard;

	TCHAR m_szProjectFile[MAX_PATH];

	// Public controls.
	CProjectListViewCtrl m_ProjectListView;
	HMENU m_hProjListSelMenu;
	HMENU m_hProjListNoSelMenu;
	CMenu m_ShellTreeMenu;

	// PIDL helper object.
	CPidlHelper m_PidlHelp;

private:
	virtual BOOL PreTranslateMessage(MSG *pMsg)
	{
		if (!m_bEnableAccel)
			return FALSE;

		if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		// Let the shell list view control enter keys sent to it.
		/*if (pMsg->hwnd == m_pShellListView->GetListViewHandle() &&
			(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN))
		{
			return FALSE;
		}

		return IsDialogMessage(pMsg);*/

		// Let the IsDialogMessage function process tab keys so it can help us
		// tab between controls.
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
		{
			return IsDialogMessage(pMsg);
		}

		return FALSE;
	}

	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}

public:
	void ShowWelcomePane(bool bShow)
	{
		m_bWelcomePane = bShow;

		if (bShow)
		{
			m_SpaceMeterView.SetSplitterPane(SPLIT_PANE_TOP,m_WelcomePane);
			m_SpaceMeterView.SetSinglePaneMode(SPLIT_PANE_TOP);
		}
		else
		{
			m_SpaceMeterView.SetSinglePaneMode();
			m_SpaceMeterView.SetSplitterPane(SPLIT_PANE_BOTTOM,m_SpaceMeter);
			m_SpaceMeterView.SetSplitterPane(SPLIT_PANE_TOP,m_MainView);

			// Setup splitters.
			RECT rcClient;
			::GetClientRect(m_hWndClient,&rcClient);

			m_SpaceMeterView.SetSplitterPos(rcClient.bottom - rcClient.top - MAINFRAME_SPACEMETER_HEIGHT);
			m_MainView.SetSplitterPos((rcClient.bottom - rcClient.top - MAINFRAME_SPACEMETER_HEIGHT)/2);

			int iSplitterPos = (rcClient.right - rcClient.left) / 4;
			m_ExplorerView.SetSplitterPos(iSplitterPos);
			m_ProjectView.SetSplitterPos(iSplitterPos);

			WINDOWPLACEMENT wp;
			GetWindowPlacement(&wp);

			if (wp.showCmd != SW_MAXIMIZE)
			{
				// Load the last used window position and size.
				if (g_DynamicSettings.m_rcWindow.left != -1 &&
					g_DynamicSettings.m_rcWindow.right != -1 &&
					g_DynamicSettings.m_rcWindow.top != -1 &&
					g_DynamicSettings.m_rcWindow.bottom != -1)
				{
					SetWindowPos(HWND_TOP,&g_DynamicSettings.m_rcWindow,0);
				}
				else
				{
					GetWindowRect(&g_DynamicSettings.m_rcWindow);

					g_DynamicSettings.m_rcWindow.right += 300;
					g_DynamicSettings.m_rcWindow.bottom += 200;

					SetWindowPos(HWND_TOP,&g_DynamicSettings.m_rcWindow,0);
				}
			}
		}
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		// Edit menu.
		UPDATE_ELEMENT(ID_EDIT_NEWFOLDER,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_RENAME,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_REMOVE,UPDUI_MENUPOPUP)

		// Actions menu.
		UPDATE_ELEMENT(ID_BURNCOMPILATION_DISCIMAGE,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_ACTIONS_IMPORTSESSION,UPDUI_MENUPOPUP)
		
		// View menu.
		UPDATE_ELEMENT(ID_VIEW_STANDARDTOOLBAR,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_LARGEICONS,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_SMALLICONS,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_LIST,UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_DETAILS,UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

private:
#if _ATL_VER <= 0x0300
	BEGIN_MSG_MAP_EX(CMainFrame)
#else
	BEGIN_MSG_MAP(CMainFrame)
#endif
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE,OnClose)
		MESSAGE_HANDLER(WM_SHELLCHANGE,OnShellChange)
		MESSAGE_HANDLER(WM_GETISHELLBROWSER,OnGetIShellBrowser)
		MESSAGE_HANDLER(WM_CONTEXTMENU,OnContextMenu)

		// Shell list view.
		MESSAGE_HANDLER(WM_SLVC_BROWSEOBJECT,OnSLVBrowseObject)
		MESSAGE_HANDLER(WM_SLVC_DONEBROWSEOBJECT,OnSLVDoneBrowseObject)
		MESSAGE_HANDLER(WM_SLVC_CHANGEFOLDER,OnSLVChangeFolder)
		MESSAGE_HANDLER(WM_SLVC_CHANGEFOLDERLINK,OnSLVChangeFolderLink)

		// Shell tree view.
		NOTIFY_HANDLER(IDC_SHELLTREEVIEW,TVN_GETDISPINFO,OnSTVGetDispInfo)
		NOTIFY_HANDLER(IDC_SHELLTREEVIEW,TVN_ITEMEXPANDING,OnSTVItemExpanding)
		NOTIFY_HANDLER(IDC_SHELLTREEVIEW,TVN_DELETEITEM,OnSTVDeleteItem)
		NOTIFY_HANDLER(IDC_SHELLTREEVIEW,TVN_SELCHANGED,OnSTVSelChanged)
		NOTIFY_HANDLER(IDC_SHELLTREEVIEW,TVN_BEGINDRAG,OnSTVBeginDrag)
		NOTIFY_HANDLER(IDC_SHELLTREEVIEW,NM_RCLICK,OnSTVRClick)

		// Project tree view.
		NOTIFY_HANDLER(IDC_PROJECTTREEVIEW,TVN_GETDISPINFO,OnPTVGetDispInfo)
		NOTIFY_HANDLER(IDC_PROJECTTREEVIEW,TVN_SELCHANGING,OnPTVSelChanging)
		NOTIFY_HANDLER(IDC_PROJECTTREEVIEW,TVN_SELCHANGED,OnPTVSelChanged)
		NOTIFY_HANDLER(IDC_PROJECTTREEVIEW,TVN_BEGINLABELEDIT,OnPTVBeginLabelEdit)
		NOTIFY_HANDLER(IDC_PROJECTTREEVIEW,TVN_ENDLABELEDIT,OnPTVEndLabelEdit)
		NOTIFY_HANDLER(IDC_PROJECTTREEVIEW,TVN_BEGINDRAG,OnPTVBeginDrag)
		NOTIFY_HANDLER(IDC_PROJECTTREEVIEW,NM_RCLICK,OnPTVRClick)

		// Project list view.
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,LVN_GETDISPINFO,OnPLVGetDispInfo)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,LVN_BEGINLABELEDIT,OnPLVBeginLabelEdit)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,LVN_ENDLABELEDIT,OnPLVEndLabelEdit)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,NM_DBLCLK,OnPLVDblClk)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,NM_RCLICK,OnPLVRClick)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,LVN_ITEMCHANGED,OnPLVItemChanged)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,LVN_DELETEITEM,OnPLVDeleteItem)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,LVN_BEGINDRAG,OnPLVBeginDrag)
		NOTIFY_HANDLER(IDC_PROJECTLISTVIEW,LVN_COLUMNCLICK,OnPLVColumnClick)

		// Toolbar.
		NOTIFY_HANDLER(ATL_IDW_TOOLBAR,TBN_BEGINADJUST,g_ToolBarManager.OnToolBarBeginAdjust)
		NOTIFY_HANDLER(ATL_IDW_TOOLBAR,TBN_INITCUSTOMIZE,g_ToolBarManager.OnToolBarInitCustomize)
		NOTIFY_HANDLER(ATL_IDW_TOOLBAR,TBN_QUERYINSERT,g_ToolBarManager.OnToolBarQueryInsert)
		NOTIFY_HANDLER(ATL_IDW_TOOLBAR,TBN_QUERYDELETE,g_ToolBarManager.OnToolBarQueryDelete)
		NOTIFY_HANDLER(ATL_IDW_TOOLBAR,TBN_GETBUTTONINFO,g_ToolBarManager.OnToolBarGetButtonInfo)
		NOTIFY_HANDLER(ATL_IDW_TOOLBAR,TBN_RESET,g_ToolBarManager.OnToolBarReset)

		// For translation purposes.
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnToolTipGetInfo)
		MESSAGE_HANDLER(WM_MENUSELECT,OnMenuSelect)

		// File menu.
		COMMAND_ID_HANDLER(ID_NEWPROJECT_DATACD,OnNewProjectDataCD)
		COMMAND_ID_HANDLER(ID_NEWPROJECT_DATACDMS,OnNewProjectDataCDMS)
		COMMAND_ID_HANDLER(ID_NEWPROJECT_DATADVD,OnNewProjectDataDVD)
		COMMAND_ID_HANDLER(ID_NEWPROJECT_AUDIO,OnNewProjectAudio)
		COMMAND_ID_HANDLER(ID_NEWPROJECT_MIXED,OnNewProjectMixed)
		COMMAND_ID_HANDLER(ID_NEWPROJECT_DVDVIDEO,OnNewProjectDVDVideo)
		COMMAND_ID_HANDLER(ID_FILE_OPEN,OnFileOpen)
		COMMAND_ID_HANDLER(ID_FILE_SAVE,OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS,OnFileSaveAs)
		COMMAND_ID_HANDLER(ID_FILE_PROJECTPROPERTIES,OnFileProjectproperties)
		COMMAND_ID_HANDLER(ID_APP_EXIT,OnFileExit)

		// Edit menu.
		COMMAND_ID_HANDLER(ID_EDIT_NEWFOLDER,g_ProjectManager.OnNewFolder)
		COMMAND_ID_HANDLER(ID_EDIT_RENAME,g_ProjectManager.OnRename)
		COMMAND_ID_HANDLER(ID_EDIT_REMOVE,g_ProjectManager.OnRemove)
		COMMAND_ID_HANDLER(ID_ADD_SELECTED,OnAdd)
		COMMAND_ID_HANDLER(ID_ADD_ALL,OnAdd)
		COMMAND_ID_HANDLER(ID_EDIT_IMPORT,OnImport)
		COMMAND_ID_HANDLER(ID_EDIT_SELECTALL,OnSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_INVERTSELECTION,OnInvertSel)

		// Actions menu.
		COMMAND_ID_HANDLER(ID_BURNCOMPILATION_COMPACTDISC,OnBurncompilationCompactdisc)
		COMMAND_ID_HANDLER(ID_BURNCOMPILATION_DISCIMAGE,OnBurncompilationDiscimage)
		COMMAND_ID_HANDLER(ID_ACTIONS_BURNIMAGE,OnActionsBurnimage)
		COMMAND_ID_HANDLER(ID_COPYDISC_COMPACTDISC,OnCopydiscCompactdisc)
		COMMAND_ID_HANDLER(ID_COPYDISC_DISCIMAGE,OnCopydiscDiscimage)
		COMMAND_ID_HANDLER(ID_ACTIONS_MANAGETRACKS,OnActionsManagetracks)
		COMMAND_ID_HANDLER(ID_ACTIONS_ERASERE,OnActionsErasere)
		COMMAND_ID_HANDLER(ID_ACTIONS_FIXATEDISC,OnActionsFixatedisc)
		COMMAND_RANGE_HANDLER_EX(MENU_DISCINFO_IDBASE,MENU_DISCINFO_IDBASE + MENU_DRIVEMENU_MAX - 1,OnActionsDiscInfo)
		COMMAND_ID_HANDLER(ID_ACTIONS_IMPORTSESSION,OnActionsImportsession)
		COMMAND_RANGE_HANDLER_EX(MENU_EJECTDISC_IDBASE,MENU_EJECTDISC_IDBASE + MENU_DRIVEMENU_MAX - 1,OnActionsEjectDisc)

		// View menu.
		COMMAND_ID_HANDLER(ID_VIEW_STANDARDTOOLBAR,OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_TBCUSTOMIZE,OnViewTBCustomize)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR,OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_VIEW_PROGRAMLOG,OnViewProgramlog)
		COMMAND_ID_HANDLER(ID_VIEW_LARGEICONS,m_ProjectListView.OnViewLargeIcons)
		COMMAND_ID_HANDLER(ID_VIEW_SMALLICONS,m_ProjectListView.OnViewSmallIcons)
		COMMAND_ID_HANDLER(ID_VIEW_LIST,m_ProjectListView.OnViewList)
		COMMAND_ID_HANDLER(ID_VIEW_DETAILS,m_ProjectListView.OnViewDetails)
		COMMAND_ID_HANDLER(ID_VIEW_UPLEVEL,OnUpLevel)

		// Options menu.
		COMMAND_ID_HANDLER(ID_OPTIONS_CONFIGURATION,OnOptionsConfiguration)
		COMMAND_ID_HANDLER(ID_OPTIONS_DEVICES,OnOptionsDevices)

		// Help menu.
		COMMAND_ID_HANDLER(ID_HELP_HELPTOPICS,OnHelpHelptopics)
		COMMAND_ID_HANDLER(ID_APP_ABOUT,OnAppAbout)
		
		// Shell tree popup menu.
		COMMAND_ID_HANDLER(ID_POPUPMENU_SHELLTREE_PROPERTIES,OnShellTreeProperties)

		// Project list popup menu.
		COMMAND_ID_HANDLER(ID_POPUPMENU_PROPERTIES,OnFileProjectproperties)

		// Custom.
		COMMAND_ID_HANDLER(ID_SHELL_PASTE,OnShellPaste)

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	HMENU GetToolBarsMenu();
	HIMAGELIST GetToolBarSmall();
	HIMAGELIST GetToolBarLarge();

	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnShellChange(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnGetIShellBrowser(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	// Shell list view.
	LRESULT OnSLVBrowseObject(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSLVDoneBrowseObject(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSLVChangeFolder(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSLVChangeFolderLink(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnSTVGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnSTVItemExpanding(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnSTVDeleteItem(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnSTVSelChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnSTVBeginDrag(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnSTVRClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);

	LRESULT OnPTVGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPTVSelChanging(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPTVSelChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPTVBeginLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPTVEndLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPTVBeginDrag(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPTVRClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);

	LRESULT OnPLVGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVBeginLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVEndLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVDblClk(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVRClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVItemChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVDeleteItem(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVBeginDrag(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnPLVColumnClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);

	// For translation purposes.
	LRESULT OnToolTipGetInfo(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnMenuSelect(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	// File menu.
	LRESULT OnNewProjectDataCD(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnNewProjectDataCDMS(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnNewProjectDataDVD(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnNewProjectAudio(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnNewProjectMixed(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnNewProjectDVDVideo(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFileOpen(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFileSave(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFileSaveAs(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnShellTreeProperties(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFileProjectproperties(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFileExit(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	// Edit menu.
	LRESULT OnAdd(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnImport(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnSelectAll(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnInvertSel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	// Actions menu.
	LRESULT OnBurncompilationCompactdisc(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnBurncompilationDiscimage(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnActionsBurnimage(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCopydiscCompactdisc(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCopydiscDiscimage(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnActionsManagetracks(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnActionsErasere(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnActionsFixatedisc(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnActionsDiscInfo(UINT uNotifyCode,int nID,CWindow wnd);
	LRESULT OnActionsImportsession(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnActionsEjectDisc(UINT uNotifyCode,int nID,CWindow wnd);
	
	// View menu.
	LRESULT OnViewToolBar(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnViewTBCustomize(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnViewStatusBar(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnViewProgramlog(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnUpLevel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	// Options menu.
	LRESULT OnOptionsConfiguration(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnOptionsDevices(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	// Help menu.
	LRESULT OnHelpHelptopics(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnAppAbout(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	// Custom.
	LRESULT OnShellPaste(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};

extern CMainFrame *g_pMainFrame;