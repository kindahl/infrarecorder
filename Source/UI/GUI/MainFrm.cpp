/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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

#include "stdafx.h"
#include <ckcore/filestream.hh>
#include <ckcore/linereader.hh>
#include "../../Common/StringUtil.h"
#include "resource.h"
#include "Settings.h"
#include "Registry.h"
#include "LogDlg.h"
#include "DevicesDlg.h"
#include "InfraRecorder.h"
#include "Core.h"
#include "Core2.h"
#include "SettingsManager.h"
#include "StringTable.h"
#include "TreeManager.h"
#include "WinVer.h"
#include "ProjectPropDlg.h"
#include "LangUtil.h"
#include "ConfigDlg.h"
#include "ActionManager.h"
#include "DiscDlg.h"
#include "Scsi.h"
#include "ImportSessionDlg.h"
#include "DriveLetterDlg.h"
#include "Core2DriverAspi.h"
#include "Core2Stream.h"
#include "ProjectDropSource.h"
#include "FilesDataObject.h"
#include "MainFrm.h"

CMainFrame::CMainFrame() : m_pShellListView(NULL),m_bWelcomePane(false)
{
	m_iDefaultProjType = PROJECTTYPE_DATA;
	m_iDefaultMedia = -1;			// Use default for each project type.
	m_bDefaultProjDVDVideo = false;
	m_bDefaultWizard = true;

	// Empty the file name.
	m_szProjectFile[0] = '\0';

	m_hMainSmallImageList = NULL;
	m_hMainLargeImageList = NULL;
	m_hMiniToolBarImageList = NULL;

	// By default we allow the tree selection to change.
	m_bEnableTreeSelection = true;

	// By default we don't ignore accelerators.
	m_bEnableAccel = true;
}

CMainFrame::~CMainFrame()
{
}

/*
	CMainFrame::AutoRunCheck
	------------------------
	Checks if autorun is enabled. Windows polls the CD drive periodically, which
	may result in recording failure.
*/
/*void CMainFrame::AutoRunCheck()
{
	CRegistry Reg;
	Reg.SetRoot(HKEY_LOCAL_MACHINE);

	if (Reg.OpenKey(_T("SYSTEM\\CurrentControlSet\\Services\\Cdrom\\"),false))
	{
		bool bAutoRun = false;

		if (Reg.ReadBool(_T("AutoRun"),bAutoRun))
		{
			if (bAutoRun)
			{
				if (lngMessageBox(m_hWnd,CONFIRM_AUTORUNENABLED,GENERAL_WARNING,MB_YESNO | MB_ICONWARNING) == IDYES)
				{
					if (!Reg.WriteBool(_T("AutoRun"),false))
						lngMessageBox(m_hWnd,ERROR_REGISTRYWRITE,GENERAL_ERROR,MB_OK | MB_ICONERROR);
				}
			}
		}

		Reg.CloseKey();
	}
}*/

/*
	CMainFrame::EnableAutoRun
	-------------------------
	Enabled or decible the Windows CD-ROM autorun feature. It returns the old
	registry setting.
*/
bool CMainFrame::EnableAutoRun(bool bEnable)
{
	bool bResult = false;

	CRegistry Reg;
	Reg.SetRoot(HKEY_LOCAL_MACHINE);

	if (Reg.OpenKey(_T("SYSTEM\\CurrentControlSet\\Services\\Cdrom\\"),false))
	{
		Reg.ReadBool(_T("AutoRun"),bResult);
		Reg.WriteBool(_T("AutoRun"),bEnable);
		Reg.CloseKey();
	}

	return bResult;
}

HWND CMainFrame::CreateToolBarCtrl()
{
	unsigned long ulToolBarStyle = g_DynamicSettings.m_iToolBarText == TOOLBAR_TEXT_SHOWRIGHT ? TBSTYLE_LIST : 0;
	HWND hWndToolBar = m_ToolBar.Create(m_hWnd,rcDefault,NULL,ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_ADJUSTABLE | ulToolBarStyle,NULL,ATL_IDW_TOOLBAR);

	m_ToolBar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	m_ToolBar.SetImageList(g_DynamicSettings.m_iToolBarIcon == TOOLBAR_ICON_SMALL ?
		m_hMainSmallImageList : m_hMainLargeImageList);
	
	// Add all toolbar buttons.
	g_ToolBarManager.FillToolBarCtrl(&m_ToolBar);

	return hWndToolBar;
}

void CMainFrame::InitializeMainSmallImageList()
{
	// Create the image list.
	HBITMAP hSmallBitmap,hLargeBitmap;

	// Get color depth (minimum requirement is 32-bits for alpha blended images).
	int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP),BITSPIXEL);
	
	if (g_WinVer.m_ulMajorCCVersion >= 6 && iBitsPixel >= 32)
	{
		hSmallBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MAINSMALLBITMAP));
		m_hMainSmallImageList = ImageList_Create(16,16,ILC_COLOR32,0,19);
		ImageList_Add(m_hMainSmallImageList,hSmallBitmap,NULL);

		hLargeBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MAINLARGEBITMAP));
		m_hMainLargeImageList = ImageList_Create(32,32,ILC_COLOR32,0,19);
		ImageList_Add(m_hMainLargeImageList,hLargeBitmap,NULL);
	}
	else
	{
		hSmallBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MAINSMALLBITMAP_));
		m_hMainSmallImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,19);
		ImageList_AddMasked(m_hMainSmallImageList,hSmallBitmap,RGB(255,0,255));

		hLargeBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MAINLARGEBITMAP_));
		m_hMainLargeImageList = ImageList_Create(32,32,ILC_COLOR32 | ILC_MASK,0,19);
		ImageList_AddMasked(m_hMainLargeImageList,hLargeBitmap,RGB(255,0,255));
	}

	DeleteObject(hSmallBitmap);
	DeleteObject(hLargeBitmap);
}

void CMainFrame::InitializeMiniToolBarImageList()
{
	// Create the image list.
	HBITMAP hBitmap;

	// Get color depth (minimum requirement is 32-bits for alpha blended images).
	int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP),BITSPIXEL);
	
	if (g_WinVer.m_ulMajorCCVersion >= 6 && iBitsPixel >= 32)
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MINITOOLBARBITMAP));

		m_hMiniToolBarImageList = ImageList_Create(16,16,ILC_COLOR32,0,6);
		ImageList_Add(m_hMiniToolBarImageList,hBitmap,NULL);
	}
	else
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MINITOOLBARBITMAP_));

		m_hMiniToolBarImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
		ImageList_AddMasked(m_hMiniToolBarImageList,hBitmap,RGB(255,0,255));
	}

	DeleteObject(hBitmap);
}

void CMainFrame::InitializeMainView()
{
	m_hWndClient = m_SpaceMeterView.Create(m_hWnd,rcDefault,
		NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
	m_SpaceMeterView.m_cxySplitBar = 2;		// Force the splitter size 2, Vista uses a larger size by default.

	m_SpaceMeterView.SetSplitterExtendedStyle(SPLIT_NONINTERACTIVE | SPLIT_BOTTOMALIGNED);

	// Space meter.
	m_SpaceMeter.Create(m_SpaceMeterView,rcDefault,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN);
	m_SpaceMeter.Initialize();

	// Main view.
	m_MainView.Create(m_SpaceMeterView,rcDefault,NULL,WS_CHILD | WS_VISIBLE |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
	m_MainView.m_cxySplitBar = 4;	// Force the splitter size 2, Vista uses a larger size by default.

	UpdateLayout();

	// FIXME:
	m_WelcomePane.Create(m_SpaceMeterView,rcDefault,NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP,
		WS_EX_CLIENTEDGE);

	// Update the splitter origins.
	RECT rcClient;
	::GetClientRect(m_hWndClient,&rcClient);

	if (m_bDefaultWizard)
	{
		m_bWelcomePane = true;

		m_SpaceMeterView.SetSplitterPane(SPLIT_PANE_TOP,m_WelcomePane);
		m_SpaceMeterView.SetSinglePaneMode(SPLIT_PANE_TOP);
	}
	else
	{
		m_SpaceMeterView.SetSplitterPane(SPLIT_PANE_BOTTOM,m_SpaceMeter);
		m_SpaceMeterView.SetSplitterPane(SPLIT_PANE_TOP,m_MainView);
		m_SpaceMeterView.SetSplitterPos(rcClient.bottom - rcClient.top - MAINFRAME_SPACEMETER_HEIGHT);
	}

	m_MainView.SetSplitterPos((rcClient.bottom - rcClient.top - MAINFRAME_SPACEMETER_HEIGHT)/2);
}

bool CMainFrame::InitializeShellTreeView()
{
	// Get the handle of the system image list.
	SHFILEINFO shFileInfo;
	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(_T("C:\\"),0,
		&shFileInfo,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	IShellFolder *pDesktop;
	ITEMIDLIST *pidl;
	TV_ITEM tvItem = { 0 };
	TV_INSERTSTRUCT tvInsert = { 0 };

	// Assign the image list handle and set the scroll time.
	m_ShellTreeView.SetImageList(hImageListSmall,LVSIL_NORMAL);
	m_ShellTreeView.SetScrollTime(100);

	// Get the location of the desktop folder.
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&pidl)))
	{
		if (FAILED(SHGetDesktopFolder(&pDesktop)))
		{
			// Free the pidl.
			IMalloc *pMalloc;
			if (SUCCEEDED(SHGetMalloc(&pMalloc)))
				pMalloc->Free(pidl);

			return false;
		}

		CShellTreeItemInfo *pItemInfo = new CShellTreeItemInfo;

		if (pItemInfo == NULL)
		{
			// Free the pidl
			IMalloc *pMalloc;
			if (SUCCEEDED(SHGetMalloc(&pMalloc)))
				pMalloc->Free(pidl);

			pDesktop->Release();
			return false;
		}

		// On the root node pidlSelf is offcourse equal to pidlFullyQual.
		pItemInfo->pidlSelf = pidl;
		pItemInfo->pidlFullyQual = pidl;

		// Set the tree view item parameters.
		tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
		tvItem.lParam = (LPARAM)pItemInfo;
		tvItem.pszText = LPSTR_TEXTCALLBACK;
		tvItem.iImage = tvItem.iSelectedImage = I_IMAGECALLBACK;
		tvItem.cChildren = TRUE;
		tvInsert.item = tvItem;
		tvInsert.hInsertAfter = TVI_LAST;

		// Insert the root item in the tree and expand it to insert its contents.
		HTREEITEM hItem = m_ShellTreeView.InsertItem(&tvInsert);
		m_ShellTreeView.Expand(hItem);

		// Check if the selected folder is the desktop. In that case we should go
		// to the dekstop root, not the <drive>:/Documents and Settings/<user name>/Desktop
		// folder.
		TCHAR szDesktopPath[MAX_PATH];
		m_PidlHelp.GetPathName(pidl,szDesktopPath);

		if (!lstrcmp(szDesktopPath,g_DynamicSettings.m_szShellDir))
		{
			m_ShellTreeView.SelectItem(hItem);
		}
		else
		{
			// The selected folder is not the desktop.
			OpenFolder(g_DynamicSettings.m_szShellDir,hItem,true);
			
			CShellTreeItemInfo *pItemInfo2 = (CShellTreeItemInfo *)m_ShellTreeView.GetItemData(m_ShellTreeView.GetSelectedItem());

			if (pItemInfo2->dwFlags == 0)
			{
				if (pItemInfo2->pParentFolder == 0)
					m_pShellListView->BrowseObject(pItemInfo2->pidlSelf,SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
				else			
					m_pShellListView->BrowseObject(pItemInfo2->pidlFullyQual,SBSP_SAMEBROWSER | SBSP_ABSOLUTE);	
			}
		}

		pDesktop->Release();

		// Register the directory monitor.
		if (!m_DirectoryMonitor.Register(m_hWnd,WM_SHELLCHANGE,SHCNE_MKDIR |
			SHCNE_RENAMEFOLDER | SHCNE_RMDIR,pidl,true))
		{
			MessageBox(_T("An error occured when trying to register the directory monitor. As a result the InfraRecorder explorer tree will not automatically synchronize with Windows Explorer."),_T("Error"),MB_OK | MB_ICONERROR);
		}
	}

	return true;
}

void CMainFrame::InitializeExplorerView(unsigned int uiSplitterPos)
{
	m_ExplorerView.Create(m_MainView,rcDefault,
		NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
	m_ExplorerView.m_cxySplitBar = 4;	// Force the splitter size 2, Vista uses a larger size by default.

	UpdateLayout();

	m_MainView.SetSplitterPane(SPLIT_PANE_TOP,m_ExplorerView);
	m_ExplorerView.SetSplitterPos(uiSplitterPos);

	// Create the list view container.
	m_ShellListViewContainer.Create(m_ExplorerView,rcDefault,NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT,(unsigned int)0,NULL);
	m_ShellListViewContainer.SetImageList(m_hMiniToolBarImageList);

	// Add container buttons.
	m_ShellListViewContainer.AddToolBarButton(ID_VIEW_UPLEVEL,0);
	m_ShellListViewContainer.AddToolBarSeparator();
	m_ShellListViewContainer.AddToolBarButton(ID_ADD_SELECTED,1);
	m_ShellListViewContainer.AddToolBarButton(ID_ADD_ALL,2);
	m_ShellListViewContainer.UpdateToolBar();

	// Create the shell list view.
	m_pShellListView = new CShellListViewCtrl(m_ShellListViewContainer,m_hWnd);
	m_ShellListViewContainer.SetClient(*m_pShellListView);

	// Create the tree view container.
	m_ShellTreeViewContainer.Create(m_ExplorerView,rcDefault,NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT,(unsigned int)0,NULL);
	m_ShellTreeViewContainer.SetHeaderHeight(m_ShellListViewContainer.GetHeaderHeight());
	m_ShellTreeViewContainer.SetLabelText(lngGetString(TITLE_EXPLORERVIEW));

	// Create the shell tree view.
	m_ShellTreeView.Create(m_ShellTreeViewContainer,rcDefault,NULL, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | 
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, 
		WS_EX_CLIENTEDGE,IDC_SHELLTREEVIEW);

	m_ShellTreeViewContainer.SetClient(m_ShellTreeView);

	// Setup the view.
	m_ExplorerView.SetSplitterPane(SPLIT_PANE_LEFT,m_ShellTreeViewContainer);
	m_ExplorerView.SetSplitterPane(SPLIT_PANE_RIGHT,m_ShellListViewContainer);

	// Initialize the shell tree view.
	InitializeShellTreeView();
}

void CMainFrame::InitializeProjectView(unsigned int uiSplitterPos)
{
	m_ProjectView.Create(m_MainView,rcDefault,
		NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
	m_ProjectView.m_cxySplitBar = 4;	// Force the splitter size 2, Vista uses a larger size by default.

	UpdateLayout();

	m_MainView.SetSplitterPane(SPLIT_PANE_BOTTOM,m_ProjectView);
	m_ProjectView.SetSplitterPos(uiSplitterPos);

	// Create the list view container.
	m_ProjectListViewContainer.Create(m_ProjectView,rcDefault,NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT,(unsigned int)0,NULL);
	m_ProjectListViewContainer.SetImageList(m_hMiniToolBarImageList);

	// Add container buttons.
	m_ProjectListViewContainer.AddToolBarButton(ID_VIEW_UPLEVEL,0);
	m_ProjectListViewContainer.AddToolBarSeparator();
	m_ProjectListViewContainer.AddToolBarButton(ID_EDIT_NEWFOLDER,4);
	m_ProjectListViewContainer.AddToolBarButton(ID_EDIT_RENAME,5);
	m_ProjectListViewContainer.AddToolBarSeparator();
	m_ProjectListViewContainer.AddToolBarButton(ID_EDIT_REMOVE,3);
	m_ProjectListViewContainer.UpdateToolBar();

	// Create the list view.
	m_ProjectListView.Create(m_ProjectListViewContainer,rcDefault,NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | 
		LVS_REPORT | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS/* | LVS_EDITLABELS*/,
		WS_EX_CLIENTEDGE,IDC_PROJECTLISTVIEW);
	m_ProjectListViewHeader.SubclassWindow(m_ProjectListView.GetHeader());

	m_ProjectListViewContainer.SetClient(m_ProjectListView);
	m_ProjectListViewContainer.SetCustomDrawHandler(m_ProjectListView,IDC_PROJECTLISTVIEW);

	// Enable the extended list view styles.
	unsigned long ulLVExStyle = LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT;
	/*if (g_GlobalSettings.m_bListGridLines)
		ulLVExStyle |= LVS_EX_GRIDLINES;
	if (g_GlobalSettings.m_bListTrackSel)
		ulLVExStyle |= LVS_EX_TRACKSELECT;*/

	m_ProjectListView.SetExtendedListViewStyle(ulLVExStyle);

	// Create the list view menus.
	m_hProjListSelMenu = LoadMenu(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_PROJLISTSELMENU));
	m_hProjListNoSelMenu = LoadMenu(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_PROJLISTNOSELMENU));

	// Make the view style check items to radio items.
	CMenuItemInfo mii;
	mii.fMask = MIIM_FTYPE;
	mii.fType = MFT_RADIOCHECK;

	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,FALSE,&mii);
	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,FALSE,&mii);
	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_LIST,FALSE,&mii);
	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_DETAILS,FALSE,&mii);

	// Create the tree view container.
	m_ProjectTreeViewContainer.Create(m_ProjectView,rcDefault,NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,WS_EX_CONTROLPARENT,(unsigned int)0,NULL);
	m_ProjectTreeViewContainer.SetHeaderHeight(m_ProjectListViewContainer.GetHeaderHeight());
	m_ProjectTreeViewContainer.SetLabelText(lngGetString(TITLE_PROJECTVIEW));

	// Create the tree view.
	m_ProjectTreeView.Create(m_ProjectTreeViewContainer,rcDefault,NULL, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | 
		TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_EDITLABELS, 
		WS_EX_CLIENTEDGE,IDC_PROJECTTREEVIEW);

	m_ProjectTreeViewContainer.SetClient(m_ProjectTreeView);
	m_ProjectTreeViewContainer.SetCustomDrawHandler(m_ProjectTreeView,IDC_PROJECTTREEVIEW);

	InitializeProjectImageLists();

	// Setup the view.
	m_ProjectView.SetSplitterPane(SPLIT_PANE_LEFT,m_ProjectTreeViewContainer);
	m_ProjectView.SetSplitterPane(SPLIT_PANE_RIGHT,m_ProjectListViewContainer);

	// Assign controls.
	g_TreeManager.AssignControls(&m_ProjectTreeView,&m_ProjectListView);
	g_ProjectManager.AssignControls(&m_ProjectView,&m_ProjectListViewContainer,&m_SpaceMeter,&m_ProjectListView,&m_ProjectTreeView);

	// Prepare the apropriate project type.
	switch (m_iDefaultProjType)
	{
		case PROJECTTYPE_DATA:
			g_ProjectManager.NewDataProject(m_iDefaultMedia != -1 ? m_iDefaultMedia :
											SPACEMETER_SIZE_703MB);

			if (m_bDefaultProjDVDVideo)
				g_ProjectSettings.m_iFileSystem = FILESYSTEM_DVDVIDEO;
			break;

		case PROJECTTYPE_AUDIO:
			g_ProjectManager.NewAudioProject(m_iDefaultMedia != -1 ? m_iDefaultMedia :
											 SPACEMETER_SIZE_80MIN);
			break;

		case PROJECTTYPE_MIXED:
			g_ProjectManager.NewMixedProject(m_iDefaultMedia != -1 ? m_iDefaultMedia :
											 SPACEMETER_SIZE_703MB);
			break;
	}
}

void CMainFrame::InitializeProjectImageLists()
{
	// Create the list view image list.
	SHFILEINFO shFileInfo = { 0 };

	HIMAGELIST hImageListSmall = (HIMAGELIST)::SHGetFileInfo(_T(""),
		0,&shFileInfo,sizeof(shFileInfo),SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	m_ProjectListView.SetImageList(hImageListSmall,LVSIL_SMALL);

	HIMAGELIST hImageListLarge = (HIMAGELIST)::SHGetFileInfo(_T(""),
		0,&shFileInfo,sizeof(shFileInfo),SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICON);
	m_ProjectListView.SetImageList(hImageListLarge,LVSIL_NORMAL);

	// Create the tree image list.
	m_hProjectTreeImageList = ImageList_Create(16,16,ILC_COLOR32,4,5);
	HICON hIcon;

	HINSTANCE hInstance = LoadLibrary(_T("shell32.dll"));
		hIcon = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(4),IMAGE_ICON,16,16,/*LR_DEFAULTCOLOR*/LR_LOADTRANSPARENT);
	FreeLibrary(hInstance);

	ImageList_AddIcon(m_hProjectTreeImageList,hIcon);
	DestroyIcon(hIcon);

	hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_DATAICON),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	ImageList_AddIcon(m_hProjectTreeImageList,hIcon);
	DestroyIcon(hIcon);

	hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_AUDIOICON),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	ImageList_AddIcon(m_hProjectTreeImageList,hIcon);
	DestroyIcon(hIcon);

	hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_MIXEDICON),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	ImageList_AddIcon(m_hProjectTreeImageList,hIcon);
	DestroyIcon(hIcon);

	hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_DVDVIDEOICON),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	ImageList_AddIcon(m_hProjectTreeImageList,hIcon);
	DestroyIcon(hIcon);

	m_ProjectTreeView.SetImageList(m_hProjectTreeImageList,LVSIL_NORMAL);
}

bool CMainFrame::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a main translation section.
	if (!pLng->EnterSection(_T("main")))
		return false;

	// Sub menus.
	TCHAR *szStrValue;

	HMENU hMainMenu = m_CmdBar.GetMenu();
	HMENU hCurMenu;

	if (pLng->GetValuePtr(ID_MENUROOT_FILE,szStrValue))			// File.
	{
		hCurMenu = ::GetSubMenu(hMainMenu,0);
		::ModifyMenu(hMainMenu,0,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_MENUROOT_EDIT,szStrValue))			// Edit.
	{
		hCurMenu = ::GetSubMenu(hMainMenu,1);
		::ModifyMenu(hMainMenu,1,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_MENUROOT_ACTIONS,szStrValue))		// Actions.
	{
		hCurMenu = ::GetSubMenu(hMainMenu,2);
		::ModifyMenu(hMainMenu,2,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_MENUROOT_VIEW,szStrValue))			// View.
	{
		hCurMenu = ::GetSubMenu(hMainMenu,3);
		::ModifyMenu(hMainMenu,3,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);

		// Modify the popup menu.
		hCurMenu = GetSubMenu(m_hProjListNoSelMenu,0);
		HMENU hViewMenu = GetSubMenu(hCurMenu,0);

		::ModifyMenu(hCurMenu,0,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hViewMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_MENUROOT_OPTIONS,szStrValue))		// Options.
	{
		hCurMenu = ::GetSubMenu(hMainMenu,4);
		::ModifyMenu(hMainMenu,4,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_MENUROOT_HELP,szStrValue))			// Help.
	{
		hCurMenu = ::GetSubMenu(hMainMenu,5);
		::ModifyMenu(hMainMenu,5,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	m_CmdBar.AttachMenu(hMainMenu);
	HMENU hSubMenu = GetSubMenu(hMainMenu,0);

	if (pLng->GetValuePtr(ID_FILE_NEWPROJECT,szStrValue))		// File -> New Project.
	{
		hCurMenu = ::GetSubMenu(hSubMenu,0);
		::ModifyMenu(hSubMenu,0,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	hSubMenu = GetSubMenu(hMainMenu,1);

	if (pLng->GetValuePtr(ID_EDIT_ADDPROJECT,szStrValue))		// Edit -> Add.
	{
		hCurMenu = ::GetSubMenu(hSubMenu,3);
		::ModifyMenu(hSubMenu,3,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	hSubMenu = GetSubMenu(hMainMenu,2);

	if (pLng->GetValuePtr(ID_ACTIONS_BURNCOMPILATION,szStrValue))		// Actions -> Burn Compilation.
	{
		hCurMenu = ::GetSubMenu(hSubMenu,0);
		::ModifyMenu(hSubMenu,0,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_ACTIONS_COPYDISC,szStrValue))				// Actions -> Copy Disc.
	{
		hCurMenu = ::GetSubMenu(hSubMenu,2);
		::ModifyMenu(hSubMenu,2,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_ACTIONS_DISCINFORMATION,szStrValue))
	{
		hCurMenu = ::GetSubMenu(hSubMenu,MENU_DISCINFO_INDEX);
		::ModifyMenu(hSubMenu,MENU_DISCINFO_INDEX,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	if (pLng->GetValuePtr(ID_ACTIONS_EJECT,szStrValue))					// Actions -> Eject Disc.
	{
		hCurMenu = ::GetSubMenu(hSubMenu,MENU_EJECTDISC_INDEX);
		::ModifyMenu(hSubMenu,MENU_EJECTDISC_INDEX,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	hSubMenu = GetSubMenu(hMainMenu,3);

	if (pLng->GetValuePtr(ID_VIEW_TOOLBARS,szStrValue))					// View -> Toolbars.
	{
		hCurMenu = ::GetSubMenu(hSubMenu,0);
		::ModifyMenu(hSubMenu,0,MF_POPUP | MF_BYPOSITION | MF_STRING,(UINT_PTR)hCurMenu,szStrValue);
	}

	// File menu.
	hCurMenu = GetSubMenu(hMainMenu,0);
	if (pLng->GetValuePtr(ID_FILE_OPEN,szStrValue))
	{
		TCHAR szMenuString[64];
		lstrncpy(szMenuString,szStrValue,sizeof(szMenuString) / sizeof(TCHAR) - 8);
		lstrcat(szMenuString,_T("\tCtrl+O"));

		ModifyMenu(hCurMenu,ID_FILE_OPEN,MF_BYCOMMAND | MF_STRING,ID_FILE_OPEN,(LPCTSTR)szMenuString);
	}
	if (pLng->GetValuePtr(ID_FILE_SAVE,szStrValue))
	{
		TCHAR szMenuString[64];
		lstrncpy(szMenuString,szStrValue,sizeof(szMenuString) / sizeof(TCHAR) - 8);
		lstrcat(szMenuString,_T("\tCtrl+S"));

		ModifyMenu(hCurMenu,ID_FILE_SAVE,MF_BYCOMMAND | MF_STRING,ID_FILE_SAVE,(LPCTSTR)szMenuString);
	}
	if (pLng->GetValuePtr(ID_FILE_SAVE_AS,szStrValue))
		ModifyMenu(hCurMenu,ID_FILE_SAVE_AS,MF_BYCOMMAND | MF_STRING,ID_FILE_SAVE_AS,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_FILE_PROJECTPROPERTIES,szStrValue))
		ModifyMenu(hCurMenu,ID_FILE_PROJECTPROPERTIES,MF_BYCOMMAND | MF_STRING,ID_FILE_PROJECTPROPERTIES,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_APP_EXIT,szStrValue))
		ModifyMenu(hCurMenu,ID_APP_EXIT,MF_BYCOMMAND | MF_STRING,ID_APP_EXIT,(LPCTSTR)szStrValue);

	// New project menu.
	hCurMenu = GetSubMenu(hCurMenu,0);
	if (pLng->GetValuePtr(ID_NEWPROJECT_DATACD,szStrValue))
		ModifyMenu(hCurMenu,ID_NEWPROJECT_DATACD,MF_BYCOMMAND | MF_STRING,ID_NEWPROJECT_DATACD,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_NEWPROJECT_DATACDMS,szStrValue))
		ModifyMenu(hCurMenu,ID_NEWPROJECT_DATACDMS,MF_BYCOMMAND | MF_STRING,ID_NEWPROJECT_DATACDMS,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_NEWPROJECT_AUDIO,szStrValue))
		ModifyMenu(hCurMenu,ID_NEWPROJECT_AUDIO,MF_BYCOMMAND | MF_STRING,ID_NEWPROJECT_AUDIO,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_NEWPROJECT_MIXED,szStrValue))
		ModifyMenu(hCurMenu,ID_NEWPROJECT_MIXED,MF_BYCOMMAND | MF_STRING,ID_NEWPROJECT_MIXED,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_NEWPROJECT_DVDVIDEO,szStrValue))
		ModifyMenu(hCurMenu,ID_NEWPROJECT_DVDVIDEO,MF_BYCOMMAND | MF_STRING,ID_NEWPROJECT_DVDVIDEO,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_NEWPROJECT_DATADVD,szStrValue))
		ModifyMenu(hCurMenu,ID_NEWPROJECT_DATADVD,MF_BYCOMMAND | MF_STRING,ID_NEWPROJECT_DATADVD,(LPCTSTR)szStrValue);

	// Edit menu.
	hCurMenu = GetSubMenu(hMainMenu,1);
	if (pLng->GetValuePtr(ID_EDIT_NEWFOLDER,szStrValue))
	{
		ModifyMenu(hCurMenu,ID_EDIT_NEWFOLDER,MF_BYCOMMAND | MF_STRING,ID_EDIT_NEWFOLDER,(LPCTSTR)szStrValue);

		// Modify the popup menu.
		ModifyMenu(m_hProjListNoSelMenu,ID_EDIT_NEWFOLDER,MF_BYCOMMAND | MF_STRING,ID_EDIT_NEWFOLDER,(LPCTSTR)szStrValue);
	}
	if (pLng->GetValuePtr(ID_EDIT_RENAME,szStrValue))
	{
		TCHAR szMenuString[64];
		lstrncpy(szMenuString,szStrValue,sizeof(szMenuString) / sizeof(TCHAR) - 4);
		lstrcat(szMenuString,_T("\tF2"));

		ModifyMenu(hCurMenu,ID_EDIT_RENAME,MF_BYCOMMAND | MF_STRING,ID_EDIT_RENAME,(LPCTSTR)szMenuString);

		// Modify the popup menu.
		ModifyMenu(m_hProjListSelMenu,ID_EDIT_RENAME,MF_BYCOMMAND | MF_STRING,ID_EDIT_RENAME,(LPCTSTR)szStrValue);
	}
	if (pLng->GetValuePtr(ID_EDIT_REMOVE,szStrValue))
	{
		TCHAR szMenuString[64];
		lstrncpy(szMenuString,szStrValue,sizeof(szMenuString) / sizeof(TCHAR) - 5);
		lstrcat(szMenuString,_T("\tDel"));

		ModifyMenu(hCurMenu,ID_EDIT_REMOVE,MF_BYCOMMAND | MF_STRING,ID_EDIT_REMOVE,(LPCTSTR)szMenuString);

		// Modify the popup menu.
		ModifyMenu(m_hProjListSelMenu,ID_EDIT_REMOVE,MF_BYCOMMAND | MF_STRING,ID_EDIT_REMOVE,(LPCTSTR)szStrValue);
	}
	if (pLng->GetValuePtr(ID_EDIT_IMPORT,szStrValue))
		ModifyMenu(hCurMenu,ID_EDIT_IMPORT,MF_BYCOMMAND | MF_STRING,ID_EDIT_IMPORT,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_EDIT_SELECTALL,szStrValue))
	{
		TCHAR szMenuString[64];
		lstrncpy(szMenuString,szStrValue,sizeof(szMenuString) / sizeof(TCHAR) - 8);
		lstrcat(szMenuString,_T("\tCtrl+A"));

		ModifyMenu(hCurMenu,ID_EDIT_SELECTALL,MF_BYCOMMAND | MF_STRING,ID_EDIT_SELECTALL,(LPCTSTR)szMenuString);
	}
	if (pLng->GetValuePtr(ID_EDIT_INVERTSELECTION,szStrValue))
		ModifyMenu(hCurMenu,ID_EDIT_INVERTSELECTION,MF_BYCOMMAND | MF_STRING,ID_EDIT_INVERTSELECTION,(LPCTSTR)szStrValue);

	// Add menu.
	hCurMenu = GetSubMenu(hCurMenu,3);
	if (pLng->GetValuePtr(ID_ADD_SELECTED,szStrValue))
		ModifyMenu(hCurMenu,ID_ADD_SELECTED,MF_BYCOMMAND | MF_STRING,ID_ADD_SELECTED,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_ADD_ALL,szStrValue))
		ModifyMenu(hCurMenu,ID_ADD_ALL,MF_BYCOMMAND | MF_STRING,ID_ADD_ALL,(LPCTSTR)szStrValue);

	// Actions menu.
	hCurMenu = GetSubMenu(hMainMenu,2);
	if (pLng->GetValuePtr(ID_ACTIONS_BURNIMAGE,szStrValue))
		ModifyMenu(hCurMenu,ID_ACTIONS_BURNIMAGE,MF_BYCOMMAND | MF_STRING,ID_ACTIONS_BURNIMAGE,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_ACTIONS_MANAGETRACKS,szStrValue))
		ModifyMenu(hCurMenu,ID_ACTIONS_MANAGETRACKS,MF_BYCOMMAND | MF_STRING,ID_ACTIONS_MANAGETRACKS,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_ACTIONS_ERASERE,szStrValue))
		ModifyMenu(hCurMenu,ID_ACTIONS_ERASERE,MF_BYCOMMAND | MF_STRING,ID_ACTIONS_ERASERE,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_ACTIONS_FIXATEDISC,szStrValue))
		ModifyMenu(hCurMenu,ID_ACTIONS_FIXATEDISC,MF_BYCOMMAND | MF_STRING,ID_ACTIONS_FIXATEDISC,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_ACTIONS_IMPORTSESSION,szStrValue))
		ModifyMenu(hCurMenu,ID_ACTIONS_IMPORTSESSION,MF_BYCOMMAND | MF_STRING,ID_ACTIONS_IMPORTSESSION,(LPCTSTR)szStrValue);

	// Burn compilation menu.
	hCurMenu = GetSubMenu(hCurMenu,0);
	if (pLng->GetValuePtr(ID_BURNCOMPILATION_DISCIMAGE,szStrValue))
		ModifyMenu(hCurMenu,ID_BURNCOMPILATION_DISCIMAGE,MF_BYCOMMAND | MF_STRING,ID_BURNCOMPILATION_DISCIMAGE,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_BURNCOMPILATION_COMPACTDISC,szStrValue))
		ModifyMenu(hCurMenu,ID_BURNCOMPILATION_COMPACTDISC,MF_BYCOMMAND | MF_STRING,ID_BURNCOMPILATION_COMPACTDISC,(LPCTSTR)szStrValue);

	// Copy disc menu (same strings as compilation menu).
	hCurMenu = GetSubMenu(GetSubMenu(hMainMenu,2),2);
	if (pLng->GetValuePtr(ID_BURNCOMPILATION_DISCIMAGE,szStrValue))
		ModifyMenu(hCurMenu,ID_COPYDISC_DISCIMAGE,MF_BYCOMMAND | MF_STRING,ID_COPYDISC_DISCIMAGE,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_BURNCOMPILATION_COMPACTDISC,szStrValue))
		ModifyMenu(hCurMenu,ID_COPYDISC_COMPACTDISC,MF_BYCOMMAND | MF_STRING,ID_COPYDISC_COMPACTDISC,(LPCTSTR)szStrValue);

	// View menu.
	hCurMenu = GetSubMenu(hMainMenu,3);
	if (pLng->GetValuePtr(ID_VIEW_STATUS_BAR,szStrValue))
		ModifyMenu(hCurMenu,ID_VIEW_STATUS_BAR,MF_BYCOMMAND | MF_STRING,ID_VIEW_STATUS_BAR,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_VIEW_PROGRAMLOG,szStrValue))
		ModifyMenu(hCurMenu,ID_VIEW_PROGRAMLOG,MF_BYCOMMAND | MF_STRING,ID_VIEW_PROGRAMLOG,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_VIEW_LARGEICONS,szStrValue))
	{
		ModifyMenu(hCurMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_STRING,ID_VIEW_LARGEICONS,(LPCTSTR)szStrValue);

		// Modify the popup menu.
		ModifyMenu(GetSubMenu(m_hProjListNoSelMenu,0),ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_STRING,ID_VIEW_LARGEICONS,(LPCTSTR)szStrValue);
	}
	if (pLng->GetValuePtr(ID_VIEW_SMALLICONS,szStrValue))
	{
		ModifyMenu(hCurMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_STRING,ID_VIEW_SMALLICONS,(LPCTSTR)szStrValue);

		// Modify the popup menu.
		ModifyMenu(GetSubMenu(m_hProjListNoSelMenu,0),ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_STRING,ID_VIEW_SMALLICONS,(LPCTSTR)szStrValue);
	}
	if (pLng->GetValuePtr(ID_VIEW_LIST,szStrValue))
	{
		ModifyMenu(hCurMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_STRING,ID_VIEW_LIST,(LPCTSTR)szStrValue);

		// Modify the popup menu.
		ModifyMenu(GetSubMenu(m_hProjListNoSelMenu,0),ID_VIEW_LIST,MF_BYCOMMAND | MF_STRING,ID_VIEW_LIST,(LPCTSTR)szStrValue);
	}
	if (pLng->GetValuePtr(ID_VIEW_DETAILS,szStrValue))
	{
		ModifyMenu(hCurMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_STRING,ID_VIEW_DETAILS,(LPCTSTR)szStrValue);

		// Modify the popup menu.
		ModifyMenu(GetSubMenu(m_hProjListNoSelMenu,0),ID_VIEW_DETAILS,MF_BYCOMMAND | MF_STRING,ID_VIEW_DETAILS,(LPCTSTR)szStrValue);
	}

	// Toolbars menu.
	hCurMenu = GetSubMenu(hCurMenu,0);
	if (pLng->GetValuePtr(ID_VIEW_STANDARDTOOLBAR,szStrValue))
		ModifyMenu(hCurMenu,ID_VIEW_STANDARDTOOLBAR,MF_BYCOMMAND | MF_STRING,ID_VIEW_STANDARDTOOLBAR,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_VIEW_TBCUSTOMIZE,szStrValue))
		ModifyMenu(hCurMenu,ID_VIEW_TBCUSTOMIZE,MF_BYCOMMAND | MF_STRING,ID_VIEW_TBCUSTOMIZE,(LPCTSTR)szStrValue);

	// Options menu.
	hCurMenu = GetSubMenu(hMainMenu,4);
	if (pLng->GetValuePtr(ID_OPTIONS_CONFIGURATION,szStrValue))
		ModifyMenu(hCurMenu,ID_OPTIONS_CONFIGURATION,MF_BYCOMMAND | MF_STRING,ID_OPTIONS_CONFIGURATION,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_OPTIONS_DEVICES,szStrValue))
		ModifyMenu(hCurMenu,ID_OPTIONS_DEVICES,MF_BYCOMMAND | MF_STRING,ID_OPTIONS_DEVICES,(LPCTSTR)szStrValue);

	// Help menu.
	hCurMenu = GetSubMenu(hMainMenu,5);
	if (pLng->GetValuePtr(ID_HELP_HELPTOPICS,szStrValue))
		ModifyMenu(hCurMenu,ID_HELP_HELPTOPICS,MF_BYCOMMAND | MF_STRING,ID_HELP_HELPTOPICS,(LPCTSTR)szStrValue);
	if (pLng->GetValuePtr(ID_APP_ABOUT,szStrValue))
	{
		TCHAR szTitle[32];
		::LoadString(_Module.GetResourceInstance(),IDR_MAINFRAME,szTitle,32);

		TCHAR szBuffer[64];
		lstrcpy(szBuffer,szStrValue);
		lstrcat(szBuffer,_T(" "));
		lstrcat(szBuffer,szTitle);
		lstrcat(szBuffer,_T("..."));

		ModifyMenu(hCurMenu,ID_APP_ABOUT,MF_BYCOMMAND | MF_STRING,ID_APP_ABOUT,(LPCTSTR)szBuffer);
	}

	// Project list seleced menu.
	if (pLng->GetValuePtr(ID_POPUPMENU_PROPERTIES,szStrValue))
	{
		ModifyMenu(m_hProjListSelMenu,ID_POPUPMENU_PROPERTIES,MF_BYCOMMAND | MF_STRING,
			ID_POPUPMENU_PROPERTIES,(LPCTSTR)szStrValue);
	}

	return true;
}

void CMainFrame::FillDriveMenus()
{
	// Update the menu.
	HMENU hActionsMenu = GetSubMenu(m_CmdBar.GetMenu(),MENU_ACTIONS_INDEX);
	HMENU hDiscMenu = GetSubMenu(hActionsMenu,MENU_DISCINFO_INDEX);
	HMENU hEjectMenu = GetSubMenu(hActionsMenu,MENU_EJECTDISC_INDEX);

	// Remove the old items.
	int iMenuItemCount = GetMenuItemCount(hEjectMenu);
	int iMenuIndex = 0;
	for (int i = iMenuItemCount - 1; i >= 0; i--)
	{
		RemoveMenu(hDiscMenu,i,MF_BYPOSITION);
		RemoveMenu(hEjectMenu,i,MF_BYPOSITION);
	}

	if (g_DeviceManager.GetDeviceCount() > 0)
	{
		for (unsigned int i = 0; i < g_DeviceManager.GetDeviceCount(); i++)
		{
			if (!g_DeviceManager.IsDeviceReader(i))
				continue;

			tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(i);

			TCHAR szDeviceName[128];
			g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

			AppendMenu(hDiscMenu,MF_STRING,MENU_DISCINFO_IDBASE + iMenuIndex,szDeviceName);
			AppendMenu(hEjectMenu,MF_STRING,MENU_EJECTDISC_IDBASE + iMenuIndex,szDeviceName);

			m_iDriveMenuDeviceMap[iMenuIndex] = i;
			
			// Increase the menu index counter.
			iMenuIndex++;
		}
	}
	else
	{
		AppendMenu(hDiscMenu,MF_STRING,0,lngGetString(EJECTMENU_NODRIVES));
		AppendMenu(hEjectMenu,MF_STRING,0,lngGetString(EJECTMENU_NODRIVES));

		CMenuItemInfo mii;
		mii.fMask = MIIM_STATE;
		mii.fState = MFS_GRAYED;
		SetMenuItemInfo(hEjectMenu,0,TRUE,&mii);
	}
}

void CMainFrame::SetTitleFile(const TCHAR *szFullName)
{
	// Update the title.
	TCHAR szTitle[MAX_PATH + 32];
	::LoadString(_Module.GetResourceInstance(),IDR_MAINFRAME,szTitle,32);
	lstrcat(szTitle,_T(" - "));

	TCHAR szFileName[MAX_PATH];
	lstrcpy(szFileName,szFullName);
	ExtractFileName(szFileName);

	lstrcat(szTitle,szFileName);

	SetWindowText(szTitle);
}

void CMainFrame::SetTitleNormal()
{
	// Update the title.
	TCHAR szTitle[32];
	::LoadString(_Module.GetResourceInstance(),IDR_MAINFRAME,szTitle,32);

	SetWindowText(szTitle);
}

bool CMainFrame::SaveProjectAs()
{
	WTL::CFileDialog FileDialog(false,_T("irp"),_T("Untitled"),OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("Project Files (*.irp)\0*.irp\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
	{
		lstrcpy(m_szProjectFile,FileDialog.m_szFileName);

		// Save the project.
		g_ProjectManager.SaveProject(m_szProjectFile);

		// Update the title.
		SetTitleFile(m_szProjectFile);
		return true;
	}

	return false;
}

/*
	CMainFrame::SaveProjectPrompt
	-----------------------------
	Asks the user to if the project should be saved if it has been modified.
	The function returns false if the operation was aborted, otherwise true.
*/
bool CMainFrame::SaveProjectPrompt()
{
	if (g_ProjectManager.GetModified())
	{
		switch (lngMessageBox(m_hWnd,CONFIRM_SAVEPROJECT,GENERAL_QUESTION,MB_YESNOCANCEL | MB_ICONQUESTION))
		{
			case IDYES:
				if (m_szProjectFile[0] == '\0')
					if (!SaveProjectAs())
						return false;
				else
					g_ProjectManager.SaveProject(m_szProjectFile);
				break;

			case IDCANCEL:
				return false;
		};
	}

	return true;
}

LRESULT CMainFrame::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Main small image list (menus and small toolbar buttons).
	InitializeMainSmallImageList();

	// Create command bar window.
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd,rcDefault,NULL,ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.AttachMenu(GetMenu());
	m_CmdBar.m_hImageList = m_hMainSmallImageList;

	// Set the menu items image index.
	m_CmdBar.m_arrCommand.Add(ID_FILE_OPEN);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SAVE);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SAVE_AS);
	m_CmdBar.m_arrCommand.Add(ID_FILE_PROJECTPROPERTIES);
	m_CmdBar.m_arrCommand.Add(ID_APP_EXIT);
	m_CmdBar.m_arrCommand.Add(ID_EDIT_NEWFOLDER);
	m_CmdBar.m_arrCommand.Add(ID_EDIT_RENAME);
	m_CmdBar.m_arrCommand.Add(ID_EDIT_REMOVE);
	m_CmdBar.m_arrCommand.Add(ID_ACTIONS_BURNCOMPILATION);	// Not visible.
	m_CmdBar.m_arrCommand.Add(ID_ACTIONS_BURNIMAGE);
	m_CmdBar.m_arrCommand.Add(ID_ACTIONS_COPYDISC);			// Not visible.
	m_CmdBar.m_arrCommand.Add(ID_ACTIONS_MANAGETRACKS);
	m_CmdBar.m_arrCommand.Add(ID_ACTIONS_ERASERE);
	m_CmdBar.m_arrCommand.Add(ID_ACTIONS_FIXATEDISC);
	m_CmdBar.m_arrCommand.Add(ID_VIEW_PROGRAMLOG);
	m_CmdBar.m_arrCommand.Add(ID_OPTIONS_CONFIGURATION);
	m_CmdBar.m_arrCommand.Add(ID_OPTIONS_DEVICES);
	m_CmdBar.m_arrCommand.Add(ID_HELP_HELPTOPICS);
	m_CmdBar.m_arrCommand.Add(ID_APP_ABOUT);

	// Remove old menu.
	SetMenu(NULL);

	HWND hWndToolBar = CreateToolBarCtrl();

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBandCtrl(m_hWndToolBar,hWndCmdBar,REBAR_MENUBAR_ID,NULL,true,0,FALSE);
	AddSimpleReBarBandCtrl(m_hWndToolBar,hWndToolBar,REBAR_TOOLBAR_ID,NULL,true,0,FALSE);

	CreateSimpleStatusBar();

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_STANDARDTOOLBAR,1);
	UISetCheck(ID_VIEW_STATUS_BAR,1);

	// Register object for message filtering and idle updates.
	CMessageLoop *pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// Make the view style check items to radio items.
	CMenuItemInfo mii;
	mii.fMask = MIIM_FTYPE;
	mii.fType = MFT_RADIOCHECK;

	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,FALSE,&mii);
	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,FALSE,&mii);
	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_LIST,FALSE,&mii);
	SetMenuItemInfo(m_hProjListNoSelMenu,ID_VIEW_DETAILS,FALSE,&mii);

	// Mini tool bar image list.
	InitializeMiniToolBarImageList();

	// Initialize views.
	RECT rcClient;
	GetClientRect(&rcClient);
	int iSplitterPos = (rcClient.right - rcClient.left) / 4;

	InitializeMainView();
	InitializeExplorerView(iSplitterPos);

	bool bWelcomePane = false;

	if (bWelcomePane)
		m_SpaceMeterView.SetSinglePaneMode(SPLIT_PANE_TOP);
	else
		InitializeProjectView(iSplitterPos);

	// Perform an autorun check.
	//if (g_GlobalSettings.m_bAutoRunCheck)
	//	AutoRunCheck();
	// WARNING: The auto run check has been experimentally replaced by temporarily
	// disabling the auto run while InfraRecorder is running.
	m_bEnableAutoRun = EnableAutoRun(false);

	// Fill drive menus.
	FillDriveMenus();

	// Apply the settings.
	if (!bWelcomePane)
		g_DynamicSettings.Apply();

	// Show/hide the tool bar and status bar.
	if (!g_DynamicSettings.m_bViewToolBar)
	{
		UISetCheck(ID_VIEW_STANDARDTOOLBAR,0);
		UpdateLayout();

		::SendMessage(m_hWndToolBar,RB_SHOWBAND,
			::SendMessage(m_hWndToolBar,RB_IDTOINDEX,REBAR_TOOLBAR_ID,0),false);
	}

	if (!g_DynamicSettings.m_bViewStatusBar)
	{
		UISetCheck(ID_VIEW_STATUS_BAR,0);
		UpdateLayout();

		::ShowWindow(m_hWndStatusBar,SW_HIDE);
	}

	// Initialize drag and drop.
	ATLVERIFY(SUCCEEDED(RegisterDragDrop(m_ProjectTreeView,m_ProjectTreeView.m_pDropTarget)));
	ATLVERIFY(SUCCEEDED(RegisterDragDrop(m_ProjectListView,m_ProjectListView.m_pDropTarget)));

	// Translate the window.
	Translate();

	if (m_szProjectFile[0] != '\0')
	{
		if (g_ProjectManager.LoadProject(m_szProjectFile))
		{
			// Update the view.
			g_TreeManager.Refresh();

			SetTitleFile(m_szProjectFile);
		}
	}

	// Confirm buffer.
	TCHAR szBuffer[32];
	::LoadString(_Module.GetResourceInstance(),128,szBuffer,32);

	size_t iBufferLen = lstrlen(szBuffer);
	for (size_t i = 0; i < iBufferLen; i++)
		szBuffer[i] = szBuffer[i] ^ 0xFF;

	TCHAR szTarget[] = { 0x49^0xFF,0x6E^0xFF,0x66^0xFF,0x72^0xFF,0x61^0xFF,
						 0x52^0xFF,0x65^0xFF,0x63^0xFF,0x6F^0xFF,0x72^0xFF,
						 0x64^0xFF,0x65^0xFF,0x72^0xFF,'\0' };

	if (lstrcmp(szTarget,szBuffer))
	{
		TCHAR szBuffer[] = { 0x59^0xFF,0x6F^0xFF,0x75^0xFF,0x72^0xFF,0x20^0xFF,
			0x76^0xFF,0x65^0xFF,0x72^0xFF,0x73^0xFF,0x69^0xFF,0x6F^0xFF,0x6E^0xFF,
			0x20^0xFF,0x6F^0xFF,0x66^0xFF,0x20^0xFF,0x49^0xFF,0x6E^0xFF,0x66^0xFF,
			0x72^0xFF,0x61^0xFF,0x52^0xFF,0x65^0xFF,0x63^0xFF,0x6F^0xFF,0x72^0xFF,
			0x64^0xFF,0x65^0xFF,0x72^0xFF,0x20^0xFF,0x68^0xFF,0x61^0xFF,0x73^0xFF,
			0x20^0xFF,0x62^0xFF,0x65^0xFF,0x65^0xFF,0x6E^0xFF,0x20^0xFF,0x61^0xFF,
			0x6C^0xFF,0x74^0xFF,0x65^0xFF,0x72^0xFF,0x65^0xFF,0x64^0xFF,0x2C^0xFF,
			0x20^0xFF,0x70^0xFF,0x6C^0xFF,0x65^0xFF,0x61^0xFF,0x73^0xFF,0x65^0xFF,
			0x20^0xFF,0x6D^0xFF,0x61^0xFF,0x6B^0xFF,0x65^0xFF,0x20^0xFF,0x73^0xFF,
			0x75^0xFF,0x72^0xFF,0x65^0xFF,0x20^0xFF,0x74^0xFF,0x68^0xFF,0x61^0xFF,
			0x74^0xFF,0x20^0xFF,0x79^0xFF,0x6F^0xFF,0x75^0xFF,0x20^0xFF,0x68^0xFF,
			0x61^0xFF,0x76^0xFF,0x65^0xFF,0x20^0xFF,0x64^0xFF,0x6F^0xFF,0x77^0xFF,
			0x6E^0xFF,0x6C^0xFF,0x6F^0xFF,0x61^0xFF,0x64^0xFF,0x65^0xFF,0x64^0xFF,
			0x20^0xFF,0x79^0xFF,0x6F^0xFF,0x75^0xFF,0x72^0xFF,0x20^0xFF,0x76^0xFF,
			0x65^0xFF,0x72^0xFF,0x73^0xFF,0x69^0xFF,0x6F^0xFF,0x6E^0xFF,0x20^0xFF,
			0x66^0xFF,0x72^0xFF,0x6F^0xFF,0x6D^0xFF,0x20^0xFF,0x68^0xFF,0x74^0xFF,
			0x74^0xFF,0x70^0xFF,0x3A^0xFF,0x2F^0xFF,0x2F^0xFF,0x69^0xFF,0x6E^0xFF,
			0x66^0xFF,0x72^0xFF,0x61^0xFF,0x72^0xFF,0x65^0xFF,0x63^0xFF,0x6F^0xFF,
			0x72^0xFF,0x64^0xFF,0x65^0xFF,0x72^0xFF,0x2E^0xFF,0x6F^0xFF,0x72^0xFF,
			0x67^0xFF,'\0' };

		size_t iBufferLen = lstrlen(szBuffer);
		for (size_t i = 0; i < iBufferLen; i++)
			szBuffer[i] = szBuffer[i] ^ 0xFF;

		MessageBox(szBuffer,_T(""),MB_OK | MB_ICONWARNING);
	}

	return 0;
}

HMENU CMainFrame::GetToolBarsMenu()
{
	// Just divided the different sections up a bit so it's easier to see.
	HMENU hViewMenu = GetSubMenu(m_CmdBar.GetMenu(),MENU_VIEW_INDEX);
	HMENU hToolBarsMenu = GetSubMenu(hViewMenu,MENU_TOOLBARS_INDEX);

	return hToolBarsMenu;
}

HIMAGELIST CMainFrame::GetToolBarSmall()
{
	return m_hMainSmallImageList;
}

HIMAGELIST CMainFrame::GetToolBarLarge()
{
	return m_hMainLargeImageList;
}

LRESULT CMainFrame::OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// If auto run was enabled before InfraRecorder was started, re-enable it.
	if (m_bEnableAutoRun)
		EnableAutoRun(true);

	// Uninitialize the shell list view drag and drop.
	ATLVERIFY(SUCCEEDED(RevokeDragDrop(m_ProjectListView)));
	ATLVERIFY(SUCCEEDED(RevokeDragDrop(m_ProjectTreeView)));

	// Save the configuration.
	g_SettingsManager.Save();

	// Destroy the m_pShellView object.
	if (m_pShellListView != NULL)
	{
		m_ShellListViewContainer.SetClient(NULL);
		delete m_pShellListView;
	}

	// Deregister the directory monitor.
	m_DirectoryMonitor.Deregister();

	// Destroy project tree view image list.
	if (m_hProjectTreeImageList)
		ImageList_Destroy(m_hProjectTreeImageList);

	// Destroy the project list view menus.
	DestroyMenu(m_hProjListSelMenu);
	DestroyMenu(m_hProjListNoSelMenu);

	// Destroy the mini tool bar image list.
	if (m_hMiniToolBarImageList)
		ImageList_Destroy(m_hMiniToolBarImageList);

	// Destroy the main small image list.
	if (m_hMainSmallImageList)
		ImageList_Destroy(m_hMainSmallImageList);

	// If we don't call PostQuitMessage() ourselves,
	// CFrameWindowImplBase::OnDestroy() will do it for us,
	// but then the application will end with exit code 1,
	// and applications which terminate normally should
	// actually end with exit code 0.
	ATLASSERT( bHandled );  // Should have been set by WTL.
	PostQuitMessage(0);

	return 0;
}

LRESULT CMainFrame::OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// FIXME: Add a check to see if the project is empty here. It's no idea to promt
	//        to save if the project is empty.
	if (!SaveProjectPrompt())
	{
		bHandled = true;
		return TRUE;
	}

	// Remember the current window position and size.
	if (IsWindowVisible() && !IsIconic() && !IsZoomed() && !m_bWelcomePane)
		GetWindowRect(&g_DynamicSettings.m_rcWindow);

	g_DynamicSettings.m_bWinMaximized = IsZoomed() == TRUE;

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnShellChange(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// This message is send completeley asynchronous. This will case a problem,
	// especially since the Windows Shell seems to send a couple identical messages
	// every time. For example: If the AddFolder routine has begin its execution and
	// it has checked so no other identical items exsists, it can be interupted by
	// a new message. This will cause duplicate folders to be added.
	// This problem is solved by ignoring all shell change notificiation while
	// processing one.
	static bool bLocked = false;

	if (!bLocked)
		bLocked = true;
	else
		return 0;

	LPITEMIDLIST *ppidls = reinterpret_cast<LPITEMIDLIST *>(wParam);
	static bool bPhase = true;

	TCHAR szPath[MAX_PATH];

	switch (lParam)
	{
		// A directory was created.
		case SHCNE_MKDIR:
			m_PidlHelp.GetPathName(ppidls[0],szPath);
			AddFolder(szPath);
			break;

		// A folder was renamed.
		case SHCNE_RENAMEFOLDER:
			RenameFolder(ppidls[0],ppidls[1]);
			break;

		// A folder was removed.
		case SHCNE_RMDIR:
			RemoveFolder(ppidls[0]);
			break;
	}

	bLocked = false;
	return 0;
}

LRESULT CMainFrame::OnGetIShellBrowser(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// This is very important, we need to redirect this message to the main frame that can return
	// the correct IShellBrowser object. If we do not answer to this message the CreateViewObject
	// function call will fail on Windows 98 systems for all other directories than the desktop.
	bHandled = TRUE;
	return (LRESULT)m_pShellListView;
}

LRESULT CMainFrame::OnSLVBrowseObject(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	HWND hWnd = (HWND)lParam;
	m_ShellListViewContainer.SetClient(hWnd);

	bHandled = true;
	return 0;
}

LRESULT CMainFrame::OnSLVDoneBrowseObject(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	HTREEITEM hSelectedItem = m_ShellTreeView.GetSelectedItem();
	CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hSelectedItem);

	TCHAR szFullName[MAX_PATH];

	if (pItemInfo->pParentFolder)
	{
		m_PidlHelp.GetDisplayPathName(pItemInfo->pParentFolder,pItemInfo->pidlSelf,szFullName,MAX_PATH - 1);
	}
	else
	{
		IShellFolder *pDesktopFolder;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			m_PidlHelp.GetDisplayPathName(pDesktopFolder,pItemInfo->pidlFullyQual,szFullName,MAX_PATH - 1);
		}
	}

	// Update the current path. (This is not the same as display name.)
	if (pItemInfo->pParentFolder)
	{
		m_PidlHelp.GetPathName(pItemInfo->pParentFolder,pItemInfo->pidlSelf,szFullName,MAX_PATH - 1);
	}
	else
	{
		IShellFolder *pDesktopFolder;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			m_PidlHelp.GetPathName(pDesktopFolder,pItemInfo->pidlFullyQual,szFullName,MAX_PATH - 1);
		}
	}

	// Remember the new path if the user has selected that option.
	if (g_GlobalSettings.m_bRememberShell)
		lstrcpy(g_DynamicSettings.m_szShellDir,szFullName);

	bHandled = true;
	return 0;
}

bool CMainFrame::OpenSpecialFolder(int iFolder)
{
	LPITEMIDLIST pidl;
	SHGetSpecialFolderLocation(NULL,iFolder,&pidl);

	// First the root item (desktop) is expanded.
	HTREEITEM hRootItem = m_ShellTreeView.GetRootItem();
	m_ShellTreeView.Expand(hRootItem);

	// Then we search desktop item for the special folder.
	HTREEITEM hItem = m_ShellTreeView.GetChildItem(hRootItem);

	while (hItem)
	{
		CShellTreeItemInfo *pItemInfo = 
			(CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hItem);

		if (pItemInfo->pParentFolder->CompareIDs(SHCIDS_CANONICALONLY,pItemInfo->pidlSelf,pidl) == 0)
		{
			m_bEnableTreeSelection = false;
				m_ShellTreeView.SelectItem(hItem);
				m_ShellTreeView.Expand(hItem);
			m_bEnableTreeSelection = true;

			return true;
		}

		hItem = m_ShellTreeView.GetNextVisibleItem(hItem);
	}

	return false;
}

bool CMainFrame::OpenFolder(TCHAR *szFullPath,HTREEITEM hFrom,bool bExpandMyComp)
{
	// We need to make sure that the "My Computer" item is selected and expaned.
	if (bExpandMyComp)
	{
		if (!OpenSpecialFolder(CSIDL_DRIVES))
			return false;
	}

	HTREEITEM hItem = m_ShellTreeView.GetChildItem(hFrom);
	CShellTreeItemInfo *pItemInfo = 
		(CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hItem);

	TCHAR szCurName[MAX_PATH];

	// It's important that we include the trailing backslash on the current path. Otherwise
	// we might get a false match in the comparison later.
	IncludeTrailingBackslash(szFullPath);
	int iFullLength = lstrlen(szFullPath);

	m_bEnableTreeSelection = false;

	while (hItem)
	{
		pItemInfo = NULL;
		pItemInfo = (CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hItem);

		ATLASSERT(pItemInfo != NULL);
		m_PidlHelp.GetPathName(pItemInfo->pParentFolder,pItemInfo->pidlSelf,szCurName,MAX_PATH - 1);

		IncludeTrailingBackslash(szCurName);
		int iCurLength = lstrlen(szCurName);

#ifdef UNICODE
		if (!_wcsnicmp(szCurName,szFullPath,iCurLength))
#else
		if (!_strnicmp(szCurName,szFullPath,iCurLength))
#endif
		{
			m_ShellTreeView.SelectItem(hItem);
			m_ShellTreeView.Expand(hItem);

			if (iCurLength == iFullLength)
				break;
		}

		hItem = m_ShellTreeView.GetNextVisibleItem(hItem);
	}

	m_bEnableTreeSelection = true;
	return true;
}

/*
	CMainFrame::AddFolder
	---------------------
	Add a new folder to the tree. The fullpath should be specified. It returns
	true upon success, false otherwise.
*/
bool CMainFrame::AddFolder(TCHAR *szFullPath)
{
	// Get the full pidl of the new folder.
	LPITEMIDLIST pidl;
	if (!m_PidlHelp.GetPidl(szFullPath,&pidl))
		return false;

	if (ExtractFilePath(szFullPath))
	{
		// Get the pidl of the parent.
		LPITEMIDLIST pidlParent;
		if (!m_PidlHelp.GetPidl(szFullPath,&pidlParent))
			return false;

		// Locate the parent in the tree.
		HTREEITEM hParentItem = FindItemFromPath(pidlParent);
		if (!hParentItem)
			return false;

		// Why does this event get called twice for some newly created folders. If
		// SHCNE_MKDIR was received only once we would not have to test this.
		if (ItemExist(hParentItem,pidl))
			return false;

		// Check if the file doesn't have any children. If that is the case, we
		// should not add the new items, just update the item count. The item
		// will be added automaticly when expanding the node.
		TVITEMEX tvParentItem;
		tvParentItem.hItem = hParentItem;
		tvParentItem.mask = TVIF_CHILDREN | TVIF_STATE;
		if (m_ShellTreeView.GetItem(&tvParentItem))
		{
			if (tvParentItem.cChildren == 0 || !(tvParentItem.state & TVIS_EXPANDEDONCE))
			{
				tvParentItem.cChildren = 1;
				m_ShellTreeView.SetItem(&tvParentItem);
				return true;
			}
		}

		// The parent already has children so we need to add the new folder manually.
		CShellTreeItemInfo *pParentInfo = (CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hParentItem);

		LPITEMIDLIST parentpidl,childpidl;
		m_PidlHelp.Split(pidl,&parentpidl,&childpidl);
		IShellFolder *pParentFolder;

		// If the parent has no parent, the desktop should be used as parent.
		if (pParentInfo->pParentFolder == 0)
		{
			if (FAILED(SHGetDesktopFolder(&pParentFolder)))
				return false;
		}
		else if (FAILED(pParentInfo->pParentFolder->BindToObject(pParentInfo->pidlSelf,
					NULL,IID_IShellFolder,(void **)&pParentFolder)))
		{
			return false;
		}

		TV_ITEM tvItem = { 0 };
		TV_INSERTSTRUCT tvInsert = { 0 };
		DWORD dwAttribs;

		CShellTreeItemInfo *pItemInfo = new CShellTreeItemInfo;
		pItemInfo->pidlSelf = childpidl;
		// We can't use ppidls[0], we need to allocate our own one.
		pItemInfo->pidlFullyQual = m_PidlHelp.ConcatenatePidl(parentpidl,childpidl);;
			pParentFolder->AddRef();
		pItemInfo->pParentFolder = pParentFolder;

		ZeroMemory(&tvItem,sizeof(tvItem));
		tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
		tvItem.pszText = LPSTR_TEXTCALLBACK;
		tvItem.iImage = tvItem.iSelectedImage = I_IMAGECALLBACK;
		tvItem.lParam = (LPARAM)pItemInfo;

		dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | /*SFGAO_DISPLAYATTRMASK | */SFGAO_CANRENAME;
		pParentFolder->GetAttributesOf(1,(LPCITEMIDLIST *)&childpidl,&dwAttribs);
		
		tvItem.cChildren = (dwAttribs & SFGAO_HASSUBFOLDER);

		if (dwAttribs & SFGAO_SHARE)
		{
			tvItem.mask |= TVIF_STATE;
			tvItem.stateMask |= TVIS_OVERLAYMASK;
			tvItem.state |= INDEXTOOVERLAYMASK(1);
		}

		tvInsert.item = tvItem;
		tvInsert.hInsertAfter = TVI_LAST;
		//tvInsert.hInsertAfter = TVI_SORT;
		tvInsert.hParent = hParentItem;
		
		// Insert the tree item.
		m_ShellTreeView.InsertItem(&tvInsert);

		pParentFolder->Release();
	}

	return true;
}

/*
	CMainFrame::RemoveFolder
	-------.----------------
	Removes a node from the tree. The item data is freed. It returns true
	if no errors occured and false otherwise.
*/
bool CMainFrame::RemoveFolder(LPITEMIDLIST pidlFullyQual)
{
	// Locate the item in the tree.
	HTREEITEM hItem = FindItemFromPath(pidlFullyQual);
	if (!hItem)
		return false;

	if (SUCCEEDED(m_ShellTreeView.DeleteItem(hItem)))
		return true;

	return false;
}

/*
	CMainFrame::RenameFolder
	------------------------
	Renames the tree item and updates the pidl accordingly. It returns true if
	no errors occured and false otherwise.
*/
bool CMainFrame::RenameFolder(LPITEMIDLIST pidlOldFullyQual,LPITEMIDLIST pidlNewFullyQual)
{
	// Locate the item in the tree.
	HTREEITEM hItem = FindItemFromPath(pidlOldFullyQual);
	if (!hItem)
		return false;

	CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hItem);
	if (!pItemInfo)
		return false;

	// Extract the full path name from the new pidl.
	TCHAR szFullPath[MAX_PATH];
	m_PidlHelp.GetPathName(pidlNewFullyQual,szFullPath);

	// Compare the two paths where the new folder and old folder are located.
	// If they differ the folder has been moved, otherwise we can just rename the
	// existing one.
	TCHAR szOldFullPath[MAX_PATH];
	m_PidlHelp.GetPathName(pidlOldFullyQual,szOldFullPath);

	TCHAR szTemp[MAX_PATH];
	lstrcpy(szTemp,szFullPath);
	
	ExtractFilePath(szOldFullPath);
	ExtractFilePath(szTemp);

	// If the paths match move the folder.
	if (lstrcmp(szTemp,szOldFullPath))
		return MoveFolder(pidlOldFullyQual,szFullPath);

	// Generate a new pidl from the path name above. (Why doesn't the first work?)
	LPITEMIDLIST pidlNew;
	if (!m_PidlHelp.GetPidl(szFullPath,&pidlNew))
		return false;

	// Split the pidl into parent and child parts.
	LPITEMIDLIST pidlParent,pidlChild;
	m_PidlHelp.Split(pidlNew,&pidlParent,&pidlChild);

	pItemInfo->pidlSelf = pidlChild;
	// We can't use pidlOldFullyQual, we need to allocate our own one.
	pItemInfo->pidlFullyQual = m_PidlHelp.ConcatenatePidl(pidlParent,pidlChild);

	// Manually update the tree item text.
	SHFILEINFO shFileInfo;

	if (SHGetFileInfo((LPCTSTR)pItemInfo->pidlFullyQual,0,&shFileInfo,
		sizeof(shFileInfo),SHGFI_PIDL | SHGFI_DISPLAYNAME))
	{
		m_ShellTreeView.SetItemText(hItem,shFileInfo.szDisplayName);
	}

	// Sort all children to the active parent (not recursive).
	// Update: I don't want to sort the tree. This will prevent all system folder
	//         to stay at top in the tree.
	//m_ShellTreeView.SortChildren(m_ShellTreeView.GetParentItem(hItem),FALSE);

	return true;
}

bool CMainFrame::MoveFolder(LPITEMIDLIST pidlFullyQual,TCHAR *szNewName)
{
	if (RemoveFolder(pidlFullyQual))
		return AddFolder(szNewName);

	return false;
}

bool CMainFrame::ItemExist(HTREEITEM hParentItem,LPITEMIDLIST pidl)
{
	HTREEITEM hItem = m_ShellTreeView.GetChildItem(hParentItem);

	while (hItem)
	{
		CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hItem);
		
		if (pItemInfo)
		{
			TCHAR szPath1[MAX_PATH],szPath2[MAX_PATH];
			m_PidlHelp.GetPathName(pidl,szPath1);
			m_PidlHelp.GetPathName(pItemInfo->pidlFullyQual,szPath2);

			if (!lstrcmp(szPath1,szPath2))
				return true;
		}

		hItem = m_ShellTreeView.GetNextVisibleItem(hItem);
	}

	return false;
}

HTREEITEM CMainFrame::FindItemFromPath(LPITEMIDLIST pidl)
{
	HTREEITEM hItem = m_ShellTreeView.GetRootItem();

	while (hItem)
	{
		CShellTreeItemInfo *pItemInfo = NULL;
		pItemInfo = (CShellTreeItemInfo *)m_ShellTreeView.GetItemData(hItem);

		TCHAR szPath1[MAX_PATH],szPath2[MAX_PATH];
		m_PidlHelp.GetPathName(pidl,szPath1);
		m_PidlHelp.GetPathName(pItemInfo->pidlFullyQual,szPath2);

		if (!lstrcmp(szPath1,szPath2))
			return hItem;

		hItem = m_ShellTreeView.GetNextVisibleItem(hItem);
	}

	return 0;
}

LRESULT CMainFrame::OnSLVChangeFolder(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	//const LPCITEMIDLIST pidl = (LPCITEMIDLIST)wParam;
	const TCHAR *szNewPathName = (TCHAR *)lParam;

	TCHAR szFullPath[MAX_PATH];
	lstrcpy(szFullPath,szNewPathName);

	// We must expand the current node.
	m_ShellTreeView.Expand(m_ShellTreeView.GetSelectedItem());

	// Open the path in the tree view, there is no need to expand the My Computer
	// node since the path is relative.
	OpenFolder(szFullPath,m_ShellTreeView.GetSelectedItem(),false);

	bHandled = true;
	return 0;
}

LRESULT CMainFrame::OnSLVChangeFolderLink(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	//const LPCITEMIDLIST pidl = (LPCITEMIDLIST)wParam;
	const TCHAR *szNewPathName = (TCHAR *)lParam;

	TCHAR szFullPath[MAX_PATH];
	lstrcpy(szFullPath,szNewPathName);

	// Go to the link path, the My Computer node needs to be expanded.
	OpenFolder(szFullPath,m_ShellTreeView.GetSelectedItem(),true);

	bHandled = true;
	return 0;
}

bool CMainFrame::EnumTreeObjects(HTREEITEM hParentItem,IShellFolder* pParentFolder,LPITEMIDLIST pidlParent)
{
	IEnumIDList *pEnum;

	if (SUCCEEDED(pParentFolder->EnumObjects(NULL,SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN,&pEnum)))
	{
		ITEMIDLIST *pidl;
		DWORD dwFetched = 1;
		DWORD dwAttribs;
		TV_ITEM tvItem = {0};
		TV_INSERTSTRUCT tvInsert = {0};

		while (SUCCEEDED(pEnum->Next(1,&pidl,&dwFetched)) && dwFetched)
		{
			CShellTreeItemInfo *pItemInfo = new CShellTreeItemInfo();
			pItemInfo->pidlSelf = pidl;
			pItemInfo->pidlFullyQual = m_PidlHelp.ConcatenatePidl(pidlParent,pidl);

			pParentFolder->AddRef();

			pItemInfo->pParentFolder = pParentFolder;

			ZeroMemory(&tvItem,sizeof(tvItem));
			tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
			tvItem.pszText = LPSTR_TEXTCALLBACK;
			tvItem.iImage = tvItem.iSelectedImage = I_IMAGECALLBACK;
			tvItem.lParam = (LPARAM)pItemInfo;

			// UPDATE: SFGAO_DISPLAYATTRMASK should never be used according to MSDN documentation.
			/*SHDESCRIPTIONID sdiDesc;
			bool bIsFloppy = false;
			if (SUCCEEDED(SHGetDataFromIDList(pParentFolder,pidl,SHGDFIL_DESCRIPTIONID,&sdiDesc,sizeof(sdiDesc))))
			{
				if (sdiDesc.dwDescriptionId == SHDID_COMPUTER_DRIVE35 || sdiDesc.dwDescriptionId == SHDID_COMPUTER_DRIVE525)
					bIsFloppy = true;
			}

			if (bIsFloppy)
			{
				dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_CANRENAME;
				pParentFolder->GetAttributesOf(1,(LPCITEMIDLIST *)&pidl,&dwAttribs);
			}
			else
			{
				dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_DISPLAYATTRMASK | SFGAO_CANRENAME;
				pParentFolder->GetAttributesOf(1,(LPCITEMIDLIST *)&pidl,&dwAttribs);
			}*/

			dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_CANRENAME;
			pParentFolder->GetAttributesOf(1,(LPCITEMIDLIST *)&pidl,&dwAttribs);

			tvItem.cChildren = (dwAttribs & SFGAO_HASSUBFOLDER);

			if (dwAttribs & SFGAO_SHARE)
			{
				tvItem.mask |= TVIF_STATE;
				tvItem.stateMask |= TVIS_OVERLAYMASK;
				tvItem.state |= INDEXTOOVERLAYMASK(1);
			}

			tvInsert.item = tvItem;
			tvInsert.hInsertAfter = TVI_LAST;
			tvInsert.hParent = hParentItem;

			// Insert the tree item.
			m_ShellTreeView.InsertItem(&tvInsert);

			dwFetched = 0;
		}

		pEnum->Release();
		return true;
	}

	return false;
}

LRESULT CMainFrame::OnSTVGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	LPNMTVDISPINFO lpDispInfo = (LPNMTVDISPINFO)pNMH;
	CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)lpDispInfo->item.lParam;
	SHFILEINFO shFileInfo;

	if (lpDispInfo->item.mask & TVIF_TEXT)
	{
		if (SHGetFileInfo((LPCTSTR)pItemInfo->pidlFullyQual,0,&shFileInfo,
			sizeof(shFileInfo),SHGFI_PIDL | SHGFI_DISPLAYNAME))
		{
			lstrcpy(lpDispInfo->item.pszText,shFileInfo.szDisplayName);
		}
	}
	  
	if (lpDispInfo->item.mask & TVIF_IMAGE)
	{
		if (SHGetFileInfo((LPCTSTR)pItemInfo->pidlFullyQual,0,&shFileInfo,
			sizeof(shFileInfo),SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY))
		{
			lpDispInfo->item.iImage = shFileInfo.iIcon;
		}
	}

	if (lpDispInfo->item.mask & TVIF_SELECTEDIMAGE)
	{
		if (SHGetFileInfo((LPCTSTR)pItemInfo->pidlFullyQual,0,&shFileInfo,
			sizeof(shFileInfo),SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
		{
			lpDispInfo->item.iSelectedImage = shFileInfo.iIcon;
		}
	}

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnSTVItemExpanding(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	LPNMTREEVIEW pNMTV = (LPNMTREEVIEW)pNMH;
	IShellFolder *pParentFolder;

	bHandled = false;

	if (pNMTV->action == TVE_COLLAPSE)
	{
		m_ShellTreeView.Expand(pNMTV->itemNew.hItem,
			TVE_COLLAPSE | TVE_COLLAPSERESET);
	}
    else if (pNMTV->action == TVE_EXPAND)
	{
		HCURSOR hCursor;
		TVITEM tvItem = {0};
		tvItem.mask = TVIF_PARAM;
		tvItem.hItem = pNMTV->itemNew.hItem;

		if (!m_ShellTreeView.GetItem(&tvItem))
			return 0;

		CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)tvItem.lParam;
		hCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
		
		if (pItemInfo->pParentFolder == 0)
		{
			if (FAILED(SHGetDesktopFolder(&pParentFolder)))
				return 0;
		}
		else if (FAILED(pItemInfo->pParentFolder->BindToObject(pItemInfo->pidlSelf,
			NULL,IID_IShellFolder,(void **)&pParentFolder)))
		{
			return 0;
		}
	
		m_ShellTreeView.SetRedraw(FALSE);
			EnumTreeObjects(pNMTV->itemNew.hItem,pParentFolder,
				pItemInfo->pidlFullyQual);
		m_ShellTreeView.SetRedraw(TRUE);

		pParentFolder->Release();
		SetCursor(hCursor);
	}

	return 0;
}

LRESULT CMainFrame::OnSTVDeleteItem(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	LPNMTREEVIEW pNMTV = (LPNMTREEVIEW)pNMH;
    IMalloc *pMalloc;
	CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)pNMTV->itemOld.lParam;

	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		if (pItemInfo->dwFlags == 0)
		{
			pMalloc->Free(pItemInfo->pidlSelf);
			pMalloc->Release();

			if (pItemInfo->pParentFolder)
			{
				pItemInfo->pParentFolder->Release();
				pMalloc->Free(pItemInfo->pidlFullyQual);
			}
		}
	}
      
	delete pItemInfo;

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnSTVSelChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = false;

	if (!m_bEnableTreeSelection)
		return 0;

	LPNMTREEVIEW pNMTV = (LPNMTREEVIEW)pNMH;
	CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)pNMTV->itemNew.lParam;

	// This check should not be needed. However if some syncronization error between the
	// shell list view and shell tree view occur this pItemInfo might be NULL.
	if (!pItemInfo)
		return 0;

	if (pItemInfo->dwFlags == 0)
	{
		if (pItemInfo->pParentFolder == 0)
			m_pShellListView->BrowseObject(pItemInfo->pidlSelf,SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
		else			
			m_pShellListView->BrowseObject(pItemInfo->pidlFullyQual,SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
	}

	return 0;
}

LRESULT CMainFrame::OnSTVBeginDrag(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = false;

	if (!m_bEnableTreeSelection)
		return 0;

	LPNMTREEVIEW pNMTV = (LPNMTREEVIEW)pNMH;
	CShellTreeItemInfo *pItemInfo = (CShellTreeItemInfo *)pNMTV->itemNew.lParam;

	// This check should not be needed. However if some syncronization error between the
	// shell list view and shell tree view occur this pItemInfo might be NULL.
	if (!pItemInfo)
		return 0;

	TCHAR szFileName[MAX_PATH];
	if (pItemInfo->pParentFolder)
	{
		m_PidlHelp.GetPathName(pItemInfo->pParentFolder,pItemInfo->pidlSelf,szFileName,MAX_PATH - 1);
	}
	else
	{
		IShellFolder *pDesktopFolder;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			m_PidlHelp.GetPathName(pDesktopFolder,pItemInfo->pidlFullyQual,szFileName,MAX_PATH - 1);
		}
	}

	// We don't allow dragging special system items with virtual file names that begins with "::{".
	if (szFileName[0] == ':')
		return 0;

	CProjectDropSource *pDropSource = new CProjectDropSource();
	CFilesDataObject *pDataObject = new CFilesDataObject();

	// Add all file names to the data object.
	pDataObject->AddFile(szFileName);

	DWORD dwEffect = 0;
	::DoDragDrop(pDataObject,pDropSource,DROPEFFECT_COPY,&dwEffect);

	pDropSource->Release();
	pDataObject->Release();

	return 0;
}

LRESULT CMainFrame::OnPTVGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	LPNMTVDISPINFO lpDispInfo = (LPNMTVDISPINFO)pNMH;
	CProjectNode *pNode = (CProjectNode *)lpDispInfo->item.lParam;

	if (lpDispInfo->item.mask & TVIF_TEXT)
		lstrcpy(lpDispInfo->item.pszText,pNode->pItemData->GetFileName());

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPTVSelChanging(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	// If a mixed-mode project is active we want to prevent the root node from
	// beeing selected.
	if (g_ProjectManager.GetProjectType() == PROJECTTYPE_MIXED)
	{
		LPNMTREEVIEW lpDispInfo = (LPNMTREEVIEW)pNMH;
		CProjectNode *pNode = (CProjectNode *)lpDispInfo->itemNew.lParam;

		if (pNode == g_TreeManager.GetRootNode())
		{
			bHandled = true;
			return TRUE;
		}
	}

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPTVSelChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	LPNMTREEVIEW pNMTV = (LPNMTREEVIEW)pNMH;

	bool bChangePath = true;
	int iImage,iSelImage;
	iImage = iSelImage = -1;

	m_ProjectTreeView.GetItemImage(pNMTV->itemNew.hItem,iImage,iSelImage);

	switch (iImage)
	{
		case PROJECTTREE_IMAGEINDEX_MIXED:
			bChangePath = false;
			break;

		case PROJECTTREE_IMAGEINDEX_AUDIO:
			g_ProjectManager.AudioSelected();
			break;

		default:
			g_ProjectManager.DataSelected();
			break;
	};

	// Select the new path.
	CProjectNode *pNode = (CProjectNode *)pNMTV->itemNew.lParam;	

	if (bChangePath)
	{
		TCHAR szNewPath[MAX_PATH];
		lstrcpy(szNewPath,pNode->pItemData->GetFilePath());
		lstrcat(szNewPath,pNode->pItemData->GetFileName());
		lstrcat(szNewPath,_T("\\"));

		g_TreeManager.SelectPath(szNewPath);
	}

	// Update the project manager.
	g_ProjectManager.TreeSetActionNode(pNode);

	// Update the menu items if focused.
	if (::GetFocus() == m_ProjectTreeView)
		g_ProjectManager.NotifyTreeSelChanged(pNode);

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPTVBeginLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	LPNMTVDISPINFO lpDispInfo = (LPNMTVDISPINFO)pNMH;
	CProjectNode *pNode = (CProjectNode *)lpDispInfo->item.lParam;

	// We want to prevent the root and virtual audio root node from beeing
	// renamed if we are dealing with a mixed project.
	if (g_ProjectManager.GetProjectType() == PROJECTTYPE_MIXED)
	{
		if (pNode == g_ProjectManager.GetMixAudioRootNode())
		{
			bHandled = true;
			return TRUE;
		}
	}

	// We can't rename locked items.
	if (pNode->pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED)
	{
		bHandled = true;
		return TRUE;
	}

	// Disable all accelerators.
	m_bEnableAccel = false;

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPTVEndLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;

	// Enable the accelerators.
	m_bEnableAccel = true;

	// Check for an empty name.
	LPNMTVDISPINFO lpDispInfo = (LPNMTVDISPINFO)pNMH;
	if (lpDispInfo->item.pszText == NULL || lpDispInfo->item.pszText[0] == '\0')
		return TRUE;

	// Make sure that file name does not contain illegal characters.
	if (FirstDelimiter(lpDispInfo->item.pszText,'\\') != -1 ||
		FirstDelimiter(lpDispInfo->item.pszText,'/') != -1)
	{
		return TRUE;
	}

	CProjectNode *pNode = (CProjectNode *)lpDispInfo->item.lParam;
	if (pNode != g_TreeManager.GetRootNode())
	{
		// Update the file name.
		pNode->pItemData->SetFileName(lpDispInfo->item.pszText);

		TCHAR szFullName[MAX_PATH];
		lstrcpy(szFullName,pNode->pItemData->GetFilePath());
		lstrcat(szFullName,pNode->pItemData->GetFileName());
		lstrcat(szFullName,_T("\\"));

		g_TreeManager.RebuildPaths(szFullName);

		m_ProjectListView.ForceRedraw();

		if (g_ProjectManager.GetProjectType() == PROJECTTYPE_MIXED &&
			pNode == g_ProjectManager.GetMixDataRootNode())
		{
			lstrcpy(g_ProjectSettings.m_szLabel,lpDispInfo->item.pszText);
		}

		if (pNode == g_TreeManager.GetCurrentNode())
			g_TreeManager.SetCurrentPath(szFullName);
	}
	else
	{
		// Update label.
		if (g_ProjectManager.GetProjectType() == PROJECTTYPE_DATA)
			lstrcpy(g_ProjectSettings.m_szLabel,lpDispInfo->item.pszText);
	}

	g_ProjectManager.SetModified(true);
	return TRUE;
}

LRESULT CMainFrame::OnPTVBeginDrag(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	m_ProjectTreeView.BeginDrag((LPNMTREEVIEW)pNMH);

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPTVRClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	POINT CursorPos,ClientPos;
	GetCursorPos(&CursorPos);

	ClientPos = CursorPos;
	::ScreenToClient(pNMH->hwndFrom,&ClientPos);

	TVHITTESTINFO tvHitTestInfo = { 0 };
	tvHitTestInfo.pt = ClientPos;
	m_ProjectTreeView.HitTest(&tvHitTestInfo);

	if ((tvHitTestInfo.flags & TVHT_ONITEMLABEL) != 0)
	{
		//m_ProjectTreeView.EditLabel(tvHitTestInfo.hItem);

		TVITEM tvItem = { 0 };
		tvItem.mask = TVIF_PARAM;
		tvItem.hItem = tvHitTestInfo.hItem;
		if (m_ProjectTreeView.GetItem(&tvItem) != FALSE)
		{
			g_ProjectManager.TreeSetActionNode((CProjectNode *)tvItem.lParam);
			g_ProjectManager.NotifyTreeSelChanged((CProjectNode *)tvItem.lParam);

			// TrackPopupMenuEx is asynchronous, it may return before the WM_COMMAND
			// message is sent. This walkaround forces the WM_COMMAND message to be
			// processed before this function returns.
			int iID = TrackPopupMenuEx(GetSubMenu(m_hProjListSelMenu,0),TPM_NONOTIFY | TPM_RETURNCMD,CursorPos.x,CursorPos.y,m_hWnd,NULL);
			::SendMessage(m_hWnd,WM_COMMAND,(WPARAM)iID,NULL);
		}
	}

	g_ProjectManager.NotifyTreeSelChanged((CProjectNode *)m_ProjectTreeView.GetSelectedItem().GetData());

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)pNMH;
	CItemData *pItemData = (CItemData *)pDispInfo->item.lParam;

	LVCOLUMN lvColumn = { 0 };
	lvColumn.mask = LVCF_SUBITEM;
	m_ProjectListView.GetColumn(pDispInfo->item.iSubItem,&lvColumn);

	if (pDispInfo->item.mask & LVIF_IMAGE)
	{
		SHFILEINFO shFileInfo;

		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISFOLDER)
		{
			// HACK: Force a folder icon to be used.
			if (SHGetFileInfo(_T(""),FILE_ATTRIBUTE_DIRECTORY,&shFileInfo,
				sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX))
			{
				pDispInfo->item.iImage = shFileInfo.iIcon;
			}
		}
		else
		{
			if (SHGetFileInfo(pItemData->GetFileName(),FILE_ATTRIBUTE_NORMAL,&shFileInfo,
				sizeof(shFileInfo),
				SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX))
			{
				pDispInfo->item.iImage = shFileInfo.iIcon;
			}
		}
	}

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		if (g_ProjectManager.GetViewType() == PROJECTVIEWTYPE_DATA)
		{
			switch (lvColumn.iSubItem)
			{
				case COLUMN_SUBINDEX_NAME:
					lstrcpy(pDispInfo->item.pszText,pItemData->GetFileName());
					break;

				case COLUMN_SUBINDEX_TYPE:
					lstrcpy(pDispInfo->item.pszText,pItemData->szFileType);
					break;

				case COLUMN_SUBINDEX_MODIFIED:
					wsprintf(pDispInfo->item.pszText,_T("%04u-%02u-%02u %02u:%02u:%02u"),
						(pItemData->usFileDate >> 9) + 1980, (pItemData->usFileDate >> 5) & 0x0F, (pItemData->usFileDate & 0x1F),
						(pItemData->usFileTime >> 11), (pItemData->usFileTime >> 5) & 0x3F, (pItemData->usFileTime & 0x1F) * 2);
					break;

				case COLUMN_SUBINDEX_SIZE:
					if (pItemData->ucFlags & PROJECTITEM_FLAG_ISFOLDER)
						pDispInfo->item.pszText[0] = '\0';
					else
						lsprintf(pDispInfo->item.pszText,_T("%I64d"),pItemData->uiSize);
					break;

				case COLUMN_SUBINDEX_PATH:
					lstrcpy(pDispInfo->item.pszText,pItemData->GetFilePath());
					break;
			}
		}
		else
		{
			switch (lvColumn.iSubItem)
			{
				case COLUMN_SUBINDEX_TRACK:
					if ((unsigned int)pDispInfo->item.pszText != 0xFFFFFFFF)
					lsprintf(pDispInfo->item.pszText,_T("%d"),pDispInfo->item.iItem + 1);
					break;

				case COLUMN_SUBINDEX_TITLE:
					// If no title is specified the file name is displayed.
					if (pItemData->GetAudioData()->szTrackTitle[0] == '\0')
						lstrcpy(pDispInfo->item.pszText,pItemData->GetFileName());
					else
						lstrcpy(pDispInfo->item.pszText,pItemData->GetAudioData()->szTrackTitle);
					break;

				case COLUMN_SUBINDEX_LENGTH:
					lsprintf(pDispInfo->item.pszText,_T("%.2d:%.2d:%.2d"),
						(unsigned int)(pItemData->GetAudioData()->uiTrackLength/(1000 * 3600)),
						(unsigned int)((pItemData->GetAudioData()->uiTrackLength/(1000 * 60)) % 60),
						(unsigned int)((pItemData->GetAudioData()->uiTrackLength/1000) % 60));
					break;

				case COLUMN_SUBINDEX_LOCATION:
					lstrcpy(pDispInfo->item.pszText,pItemData->szFullPath);
					break;
			}
		}
	}

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVBeginLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)pNMH;
	CItemData *pItemData = (CItemData *)pDispInfo->item.lParam;

	// We can't rename locked items.
	if (pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED)
	{
		bHandled = true;
		return TRUE;
	}

	// Disable all accelerators.
	m_bEnableAccel = false;

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVEndLabelEdit(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;

	// Enable the accelerators.
	m_bEnableAccel = true;

	// Check for an empty name.
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)pNMH;
	if (pDispInfo->item.pszText == NULL || pDispInfo->item.pszText[0] == '\0')
		return TRUE;

	// Make sure that file name does not contain illegal characters.
	if (FirstDelimiter(pDispInfo->item.pszText,'\\') != -1 ||
		FirstDelimiter(pDispInfo->item.pszText,'/') != -1)
	{
		return TRUE;
	}

	// Check for an existing file or folder with the same name.
	if (g_TreeManager.GetChildItem(g_TreeManager.GetCurrentNode(),pDispInfo->item.pszText) != NULL)
	{
		TCHAR szMessage[MAX_PATH];
		lsnprintf_s(szMessage,sizeof(szMessage),lngGetString(ERROR_EXISTINGFILENAME),
			pDispInfo->item.pszText);
		MessageBox(szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
		return TRUE;
	}

	// FIXME: A name check should be performed before accepting the name change.
	//        The file name might contain illegal characters.

	// Update the file name.
	CItemData *pItemData = (CItemData *)pDispInfo->item.lParam;
	pItemData->SetFileName(pDispInfo->item.pszText);

	// Update the file type (if it's not a folder).
	if (!(pItemData->ucFlags & PROJECTITEM_FLAG_ISFOLDER))
	{
		SHFILEINFO shFileInfo;
		if (SHGetFileInfo(pItemData->GetFileName(),FILE_ATTRIBUTE_NORMAL,&shFileInfo,
			sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
		{
			lstrcpy(pItemData->szFileType,shFileInfo.szTypeName);
		}
		else
		{
			lstrcpy(pItemData->szFileType,_T(""));
		}
	}
	else
	{
		TCHAR szFullName[MAX_PATH];
		lstrcpy(szFullName,pItemData->GetFilePath());
		lstrcat(szFullName,pItemData->GetFileName());
		lstrcat(szFullName,_T("\\"));

		g_TreeManager.RebuildPaths(szFullName);
	}

	// Force the tree view to update.
	RECT rcTree;
	m_ProjectTreeView.GetClientRect(&rcTree);
	m_ProjectTreeView.InvalidateRect(&rcTree);

	g_ProjectManager.SetModified(true);

	return TRUE;
}

LRESULT CMainFrame::OnPLVDblClk(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	// First we make sure that an item is selected.
	int iSelItem = -1;
	iSelItem = m_ProjectListView.GetNextItem(iSelItem,LVNI_SELECTED);

	if (iSelItem == -1)
		return 0;

	CItemData *pItemData = (CItemData *)m_ProjectListView.GetItemData(iSelItem);

	if (pItemData != NULL)
	{
		// If we double-clicked on a folder we should browse its contents.
		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISFOLDER)
		{
			// Select the folder in the tree view.
			HTREEITEM hTreeItem = m_ProjectTreeView.GetSelectedItem();
			m_ProjectTreeView.Expand(hTreeItem);

			hTreeItem = m_ProjectTreeView.GetChildItem(hTreeItem);

			while (hTreeItem)
			{
				CProjectNode *pTreeNode = (CProjectNode *)m_ProjectTreeView.GetItemData(hTreeItem);
				if (pTreeNode)
				{
					//if (!lstrcmp(pTreeNode->pItemData->GetFileName(),pItemData->GetFileName()))
					if (pTreeNode->pItemData == pItemData)
					{
						m_ProjectTreeView.SelectItem(hTreeItem);
						break;
					}
				}

				hTreeItem = m_ProjectTreeView.GetNextVisibleItem(hTreeItem);
			}			
		}
	}

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVRClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	POINT CursorPos;
	GetCursorPos(&CursorPos);

	// Display different popup menus depending on if any item(s) is selected or not.
	if (m_ProjectListView.GetSelectedCount() > 0)
		TrackPopupMenuEx(GetSubMenu(m_hProjListSelMenu,0),0,CursorPos.x,CursorPos.y,m_hWnd,NULL);
	else
		TrackPopupMenuEx(GetSubMenu(m_hProjListNoSelMenu,0),0,CursorPos.x,CursorPos.y,m_hWnd,NULL);

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVItemChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	if (::GetFocus() == m_ProjectListView)
		g_ProjectManager.NotifyListSelChanged(m_ProjectListView.GetSelectedCount());

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVDeleteItem(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	g_ProjectManager.NotifyListSelChanged(0);

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVBeginDrag(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	m_ProjectListView.BeginDrag((LPNMLISTVIEW)pNMH);

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnPLVColumnClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	LPNMLISTVIEW lpNMListView = (LPNMLISTVIEW)pNMH;

	// The 'Track' column in audio projects should not be sortable. 
	if (g_ProjectManager.GetViewType() == PROJECTVIEWTYPE_AUDIO &&
		lpNMListView->iSubItem == 0)
		return 0;

	m_ProjectListViewHeader.ColumnClick(lpNMListView->iSubItem);

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnToolTipGetInfo(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;

	// The string ID is the same as the button ID.
	LPTOOLTIPTEXT pTipText = (LPTOOLTIPTEXT)pNMH;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a hint translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("hint")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr((unsigned long)pTipText->hdr.idFrom,szStrValue))
			{
				pTipText->lpszText = szStrValue;
				return 0;
			}
		}
	}

	pTipText->lpszText = MAKEINTRESOURCE(pTipText->hdr.idFrom);
	return 0;
}

LRESULT CMainFrame::OnMenuSelect(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
	{
		bHandled = false;
		return TRUE;
	}
	
	// Make sure that there is a hint translation section.
	if (!g_LanguageSettings.m_pLngProcessor->EnterSection(_T("hint")))
	{
		bHandled = false;
		return TRUE;
	}

	int iIndex = LOWORD(wParam);
	int iFlags = HIWORD(wParam);

	if (!(iFlags & MF_POPUP))
	{
		TCHAR szTemp[25];
		lsprintf(szTemp,_T("%d (%d)"),iIndex,(int)((iFlags & MF_POPUP) > 0));

		TCHAR *szStrValue;
		if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(iIndex,szStrValue))
			::SendMessage(m_hWndStatusBar,SB_SETTEXT,0,(LPARAM)szStrValue);
	}
	else
	{
		::SendMessage(m_hWndStatusBar,SB_SETTEXT,0,(LPARAM)_T(""));
	}

	bHandled = true;
	return 0;
}

LRESULT CMainFrame::OnNewProjectDataCD(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!SaveProjectPrompt())
		return 0;

	// Disable the welcome screen if active.
	ShowWelcomePane(false);

	SetTitleNormal();
	m_szProjectFile[0] = '\0';

	g_ProjectManager.NewDataProject(SPACEMETER_SIZE_703MB);
	return 0;
}

LRESULT CMainFrame::OnNewProjectDataCDMS(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!SaveProjectPrompt())
		return 0;

	// Disable the welcome screen if active.
	ShowWelcomePane(false);

	SetTitleNormal();
	m_szProjectFile[0] = '\0';

	g_ProjectManager.NewDataProject(SPACEMETER_SIZE_703MB);

	g_ProjectSettings.m_iFileSystem = FILESYSTEM_ISO9660;
	g_ProjectSettings.m_iIsoFormat = 1;	// Mode 2 (multi-session).

	return 0;
}

LRESULT CMainFrame::OnNewProjectDataDVD(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!SaveProjectPrompt())
		return 0;

	// Disable the welcome screen if active.
	ShowWelcomePane(false);

	SetTitleNormal();
	m_szProjectFile[0] = '\0';

	g_ProjectManager.NewDataProject(SPACEMETER_SIZE_DVD);
	return 0;
}

LRESULT CMainFrame::OnNewProjectAudio(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!SaveProjectPrompt())
		return 0;

	// Disable the welcome screen if active.
	ShowWelcomePane(false);

	SetTitleNormal();
	m_szProjectFile[0] = '\0';

	g_ProjectManager.NewAudioProject(SPACEMETER_SIZE_80MIN);
	return 0;
}

LRESULT CMainFrame::OnNewProjectMixed(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!SaveProjectPrompt())
		return 0;

	// Disable the welcome screen if active.
	ShowWelcomePane(false);

	SetTitleNormal();
	m_szProjectFile[0] = '\0';

	g_ProjectManager.NewMixedProject(SPACEMETER_SIZE_703MB);
	return 0;
}

LRESULT CMainFrame::OnNewProjectDVDVideo(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!SaveProjectPrompt())
		return 0;

	// Disable the welcome screen if active.
	ShowWelcomePane(false);

	SetTitleNormal();
	m_szProjectFile[0] = '\0';

	g_ProjectManager.NewDataProject(SPACEMETER_SIZE_DVD);

	g_ProjectSettings.m_iFileSystem = FILESYSTEM_DVDVIDEO;
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (!SaveProjectPrompt())
		return 0;

	WTL::CFileDialog FileDialog(true,0,0,OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("Project Files (*.irp)\0*.irp\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
	{
		// Disable the welcome screen if active.
		ShowWelcomePane(false);

		if (g_ProjectManager.LoadProject(FileDialog.m_szFileName))
		{
			// Update the view.
			g_TreeManager.Refresh();

			lstrcpy(m_szProjectFile,FileDialog.m_szFileName);
			SetTitleFile(m_szProjectFile);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnFileSave(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (m_szProjectFile[0] == '\0')
		SaveProjectAs();
	else
		g_ProjectManager.SaveProject(m_szProjectFile);

	return 0;
}

LRESULT CMainFrame::OnFileSaveAs(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	SaveProjectAs();
	return 0;
}

LRESULT CMainFrame::OnFileProjectproperties(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CProjectPropDlg ProjectPropDlg;
	if (ProjectPropDlg.DoModal() == IDOK)
	{
		if (g_ProjectManager.GetViewType() == PROJECTVIEWTYPE_AUDIO)
		{
			// Force the list view to redraw.
			m_ProjectListView.ForceRedraw();
		}

		// Update the label.
		g_ProjectManager.SetDiscLabel(g_ProjectSettings.m_szLabel);
		g_ProjectManager.SetModified(true);
	}

	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnAdd(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CIDA *pData = m_pShellListView->BeginGetItems(wID == ID_ADD_SELECTED);
	if (pData == NULL)
		return 0;

	// Prepare a project file transaction.
	CProjectManager::CFileTransaction Transaction;

	TCHAR szFileName[MAX_PATH];
	IShellFolder *pParentFolder = m_pShellListView->GetParentShellFolder();
	for (unsigned int i = 0; i < pData->cidl; i++)
	{
		char *pcData = reinterpret_cast<char *>(pData) + pData->aoffset[1 + i];
		LPCITEMIDLIST pidl = (LPITEMIDLIST)pcData;

		DWORD dwAttribs = SFGAO_FILESYSTEM;
		if (pParentFolder->GetAttributesOf(1,&pidl,&dwAttribs) == NOERROR)
		{
			// We're not interested in non filesystem objects like the recycle bin and my computer.
			if (dwAttribs & SFGAO_FILESYSTEM)
			{
				m_PidlHelp.GetPathName(pParentFolder,pidl,szFileName,MAX_PATH - 1);
				Transaction.AddFile(szFileName);
			}
		}
	}

	m_pShellListView->EndGetItems();
	return 0;
}

LRESULT CMainFrame::OnImport(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	WTL::CFileDialog FileDialog(true,0,0,OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("File List (*.txt,*.m3u)\0*.txt;*.m3u\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
	{
		if (!g_ProjectManager.Import(FileDialog.m_szFileName))
			lngMessageBox(m_hWnd,ERROR_PROJECT_IMPORT,GENERAL_ERROR,MB_OK | MB_ICONERROR);
	}

	return 0;
}

LRESULT CMainFrame::OnSelectAll(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Get the focused list view control.
	HWND hWndListView;
	if (::GetFocus() == m_pShellListView->GetListViewHandle())
		hWndListView = m_pShellListView->GetListViewHandle();
	else
		hWndListView = m_ProjectListView;

	// Select all items in the list view.
	LVITEM lvItem = { 0 };
	lvItem.state = LVIS_SELECTED;
	lvItem.stateMask = LVIS_SELECTED;

	for (int i = 0; i < (int)::SendMessage(hWndListView,LVM_GETITEMCOUNT,0,0L); i++)
		::SendMessage(hWndListView,LVM_SETITEMSTATE,i,(LPARAM)&lvItem);

	return 0;
}

LRESULT CMainFrame::OnInvertSel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Get the focused list view control.
	HWND hWndListView;
	if (::GetFocus() == m_pShellListView->GetListViewHandle())
		hWndListView = m_pShellListView->GetListViewHandle();
	else
		hWndListView = m_ProjectListView;

	// Invert the selection.
	LVITEM lvItem = { 0 };
	lvItem.state = LVIS_SELECTED;
	lvItem.stateMask = LVIS_SELECTED;

	for (int i = 0; i < (int)::SendMessage(hWndListView,LVM_GETITEMCOUNT,0,0L); i++)
	{
		lvItem.state = ::SendMessage(hWndListView,LVM_GETITEMSTATE,
			i,(LPARAM)LVIS_SELECTED)? 0 : LVIS_SELECTED;
		::SendMessage(hWndListView,LVM_SETITEMSTATE,i,(LPARAM)&lvItem);
	}

	return 0;
}

LRESULT CMainFrame::OnBurncompilationCompactdisc(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ActionManager.BurnCompilation(m_hWnd,false);
	return 0;
}

LRESULT CMainFrame::OnBurncompilationDiscimage(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (g_ProjectManager.GetProjectType() == PROJECTTYPE_MIXED)
	{
		if (lngMessageBox(m_hWnd,CONFIRM_CREATEMIXIMAGE,GENERAL_QUESTION,MB_YESNO | MB_ICONQUESTION) == IDNO)
			return 0;
	}

	g_ActionManager.CreateImage(m_hWnd,false);
	return 0;
}

LRESULT CMainFrame::OnActionsBurnimage(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ActionManager.BurnImage(m_hWnd,false);
	return 0;
}

LRESULT CMainFrame::OnCopydiscCompactdisc(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ActionManager.CopyDisc(m_hWnd,false);
	return 0;
}

LRESULT CMainFrame::OnCopydiscDiscimage(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ActionManager.CopyImage(m_hWnd,false);
	return 0;
}

LRESULT CMainFrame::OnActionsManagetracks(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ActionManager.ManageTracks(false);
	return 0;
}

LRESULT CMainFrame::OnActionsErasere(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ActionManager.Erase(m_hWnd,false);
	return 0;
}

LRESULT CMainFrame::OnActionsFixatedisc(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ActionManager.Fixate(m_hWnd,false);
	return 0;
}

LRESULT CMainFrame::OnActionsDiscInfo(UINT uNotifyCode,int nID,CWindow wnd)
{
	// GetVolumeInformation() [see below] can take some time.
	CWaitCursor WaitCursor;		// This displays the hourglass cursor.

	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(m_iDriveMenuDeviceMap[nID - MENU_DISCINFO_IDBASE]);

	TCHAR szDriveLetter[4];
	szDriveLetter[0] = pDeviceInfo->Address.m_cDriveLetter;
	szDriveLetter[1] = ':';
	szDriveLetter[2] = '\\';
	szDriveLetter[3] = '\0';

	TCHAR szTitle[128];
	lstrcpy(szTitle,lngGetString(PROPERTIES_TITLE));

	// Get disc label.
	TCHAR szDiscLabel[64];
	szDiscLabel[0] = '\0';
	GetVolumeInformation(szDriveLetter,szDiscLabel,63,NULL,NULL,0,NULL,0);
	
	// Include the drive letter.
	if (szDiscLabel[0] != '\0')
	{
		lstrcat(szTitle,szDiscLabel);

		lstrcat(szTitle,_T(" (X:)"));
		szTitle[lstrlen(szTitle) - 3] = szDriveLetter[0];
	}
	else
	{
		lstrcat(szTitle,_T("X:"));
		szTitle[lstrlen(szTitle) - 2] = szDriveLetter[0];
	}

	CDiscDlg DiscDlg(szTitle,szDiscLabel,&pDeviceInfo->Address);
	DiscDlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnActionsImportsession(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_pLogDlg->print_line(_T("CMainFrame::OnActionsImportsession"));

	// We can only import to projects containing a data track.
	CProjectNode *pDataRootNode = NULL;
	switch (g_ProjectManager.GetProjectType())
	{
		case PROJECTTYPE_DATA:
			pDataRootNode = g_TreeManager.GetRootNode();
			break;

		case PROJECTTYPE_MIXED:
			pDataRootNode = g_ProjectManager.GetMixDataRootNode();
			break;

		//case PROJECTTYPE_AUDIO:
		default:
			return 0;
	}

	if (g_ProjectSettings.m_iFileSystem != FILESYSTEM_ISO9660)
	{
		if (lngMessageBox(m_hWnd,WARNING_IMPORTFS,GENERAL_WARNING,MB_YESNO | MB_ICONWARNING) == IDYES)
			g_ProjectSettings.m_iFileSystem = FILESYSTEM_ISO9660;
		else
			return 0;
	}

	CImportSessionDlg ImportSessionDlg;
	if (ImportSessionDlg.DoModal() == IDOK)
	{
		// Remove any previously imported sessions.
		g_ProjectManager.DeleteImportedItems();

		// Make sure a valid track was selected.
		if (ImportSessionDlg.m_pSelTrackData == NULL)
		{
			g_pLogDlg->print_line(_T("  Error: Invalid track selection in import session dialog."));
			return 0;
		}

		// Import the new session.
		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(ImportSessionDlg.m_uiDeviceIndex);

		CCore2Device Device;
		if (!Device.Open(&pDeviceInfo->Address))
		{
			g_pLogDlg->print_line(_T("  Error: Failed to open device when trying to import session."));
			return 0;
		}
		
		CCore2Info Info;
		CCore2TrackInfo TrackInfo;
		if (!Info.ReadTrackInformation(&Device,CCore2Info::TIT_TRACK,0xFF,&TrackInfo))
		{
			g_pLogDlg->print_line(_T("  Error: Failed to read track information when trying to import session."));
			return 0;
		}

		// Import file tree.
		CCore2InStream InStream(g_pLogDlg,&Device,0,
			ImportSessionDlg.m_pSelTrackData->m_ulTrackAddr + ImportSessionDlg.m_pSelTrackData->m_ulTrackLen);

		ckfilesystem::Iso9660Reader Reader(*g_pLogDlg);
		Reader.read(InStream,ImportSessionDlg.m_pSelTrackData->m_ulTrackAddr);
		//Reader.PrintTree();

		g_TreeManager.ImportIso9660Tree(Reader.get_root(),pDataRootNode);
		g_TreeManager.Refresh();

		// Update the space meter.
		m_SpaceMeter.SetAllocatedSize(ImportSessionDlg.m_uiAllocatedSize);
		
		// Update the (internal) project settings.
		g_ProjectSettings.m_bMultiSession = true;
		g_ProjectSettings.m_uiImportTrackAddr = ImportSessionDlg.m_pSelTrackData->m_ulTrackAddr;
		g_ProjectSettings.m_uiImportTrackLen = ImportSessionDlg.m_pSelTrackData->m_ulTrackLen;
		g_ProjectSettings.m_uiNextWritableAddr = TrackInfo.m_ulNextWritableAddr;
		g_ProjectSettings.m_uiDeviceIndex = ImportSessionDlg.m_uiDeviceIndex;
		g_ProjectSettings.m_iIsoFormat = 1;	// Mode 2 (multi-session)

		g_pLogDlg->print_line(_T("  Imported session: %I64d-%I64d, %I64d."),
			g_ProjectSettings.m_uiImportTrackAddr,
			g_ProjectSettings.m_uiImportTrackAddr + g_ProjectSettings.m_uiImportTrackLen,
			g_ProjectSettings.m_uiNextWritableAddr);
	}

	return 0;
}

LRESULT CMainFrame::OnActionsEjectDisc(UINT uNotifyCode,int nID,CWindow wnd)
{
	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(m_iDriveMenuDeviceMap[nID - MENU_EJECTDISC_IDBASE]);

	if (!g_Core.EjectDisc(pDeviceInfo,false))
		lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_DynamicSettings.m_bViewToolBar = !g_DynamicSettings.m_bViewToolBar;

	::SendMessage(m_hWndToolBar,RB_SHOWBAND,
		::SendMessage(m_hWndToolBar,RB_IDTOINDEX,REBAR_TOOLBAR_ID,0),
		g_DynamicSettings.m_bViewToolBar);

	UISetCheck(ID_VIEW_STANDARDTOOLBAR,g_DynamicSettings.m_bViewToolBar);
	UpdateLayout();

	return 0;
}

LRESULT CMainFrame::OnViewTBCustomize(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ToolBarManager.Customize();

	// Update the height of the toolbar band.
	int iBandIndex = (int)::SendMessage(m_hWndToolBar,RB_IDTOINDEX,REBAR_TOOLBAR_ID,0);
	if (iBandIndex != -1)
	{
		REBARBANDINFO rbInfo;
		rbInfo.cbSize = sizeof(REBARBANDINFO);
		rbInfo.fMask = RBBIM_CHILDSIZE;

		if (::SendMessage(m_hWndToolBar,RB_GETBANDINFO,(WPARAM)iBandIndex,(LPARAM)&rbInfo) != 0)
		{
			RECT rcToolBar;
			m_ToolBar.GetClientRect(&rcToolBar);
			rbInfo.cyMinChild = rcToolBar.bottom - rcToolBar.top;

			::SendMessage(m_hWndToolBar,RB_SETBANDINFO,(WPARAM)iBandIndex,(LPARAM)&rbInfo);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_DynamicSettings.m_bViewStatusBar = !g_DynamicSettings.m_bViewStatusBar;
	::ShowWindow(m_hWndStatusBar,g_DynamicSettings.m_bViewStatusBar ? SW_SHOWNOACTIVATE : SW_HIDE);

	UISetCheck(ID_VIEW_STATUS_BAR,g_DynamicSettings.m_bViewStatusBar);
	UpdateLayout();

	return 0;
}

LRESULT CMainFrame::OnViewProgramlog(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_pLogDlg->Show();
	return 0;
}

LRESULT CMainFrame::OnUpLevel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Check which up level button that was clicked.
	if (hWndCtl == m_ProjectListViewContainer)
	{
		HTREEITEM hParent = m_ProjectTreeView.GetParentItem(m_ProjectTreeView.GetSelectedItem());

		if (hParent != NULL)
			m_ProjectTreeView.SelectItem(hParent);
	}
	else if (hWndCtl == m_ShellListViewContainer)
	{
		HTREEITEM hParent = m_ShellTreeView.GetParentItem(m_ShellTreeView.GetSelectedItem());
		
		if (hParent != NULL)
			m_ShellTreeView.SelectItem(hParent);
	}

	return 0;
}

LRESULT CMainFrame::OnOptionsConfiguration(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CConfigDlg ConfigDlg;
	ConfigDlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnOptionsDevices(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CDevicesDlg DevicesDlg;
	DevicesDlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnHelpHelptopics(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/infra_recorder/introduction.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CAboutDlg AboutDlg;
	AboutDlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnShellPaste(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (OpenClipboard())
	{
		if (IsClipboardFormatAvailable(CF_HDROP))
		{
			HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);

			if (hDrop != NULL)
			{
				// Prepare a project file transaction.
				CProjectManager::CFileTransaction Transaction;

				unsigned int uiNumFiles = ::DragQueryFile(hDrop,0xFFFFFFFF,NULL,NULL);
				TCHAR szFullName[MAX_PATH];
				
				for (unsigned int i = 0; i < uiNumFiles; i++)
				{
					if (DragQueryFile(hDrop,i,szFullName,MAX_PATH - 1))
						Transaction.AddFile(szFullName);
				}
			}
		}

		CloseClipboard();
	}

	return 0;
}