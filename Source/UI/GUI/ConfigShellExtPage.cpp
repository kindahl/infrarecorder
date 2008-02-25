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
#include "ConfigShellExtPage.h"
#include "Settings.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "WinVer.h"
#include "NewFileExtDlg.h"
#include "ShellExtUtil.h"

CConfigShellExtPage::CConfigShellExtPage()
{
	m_hToolBarImageList = NULL;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_SHELLEXT,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CConfigShellExtPage::~CConfigShellExtPage()
{
	if (m_hToolBarImageList)
		ImageList_Destroy(m_hToolBarImageList);
}

bool CConfigShellExtPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a config translation section.
	if (!pLNG->EnterSection(_T("config")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDC_SHELLEXTCHECK,szStrValue))
		SetDlgItemText(IDC_SHELLEXTCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_SHELLEXTSUBMENUCHECK,szStrValue))
		SetDlgItemText(IDC_SHELLEXTSUBMENUCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_SHELLEXTICONCHECK,szStrValue))
		SetDlgItemText(IDC_SHELLEXTICONCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_SHELLEXTTEXTSTATIC,szStrValue))
		SetDlgItemText(IDC_SHELLEXTTEXTSTATIC,szStrValue);

	return true;
}

bool CConfigShellExtPage::OnApply()
{
	// Remember the configuration.
	g_GlobalSettings.m_bShellExtSubMenu = IsDlgButtonChecked(IDC_SHELLEXTSUBMENUCHECK) == TRUE;
	g_GlobalSettings.m_bShellExtIcon = IsDlgButtonChecked(IDC_SHELLEXTICONCHECK) == TRUE;

	if (IsDlgButtonChecked(IDC_SHELLEXTCHECK))
		RegisterShellExtension();
	else
		UnregisterShellExtension();

	// Associate the individual file extensions.
	RegisterListView();

	return true;
}

void CConfigShellExtPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/configuration.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

void CConfigShellExtPage::InitToolBarImageList()
{
	// Create the image list.
	HBITMAP hBitmap;
	
	if (g_WinVer.m_ulMajorCCVersion >= 6)
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MINITOOLBARBITMAP));

		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32,0,6);
		ImageList_Add(m_hToolBarImageList,hBitmap,NULL);
	}
	else
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MINITOOLBARBITMAP_));

		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
		ImageList_AddMasked(m_hToolBarImageList,hBitmap,RGB(255,0,255));
	}

	DeleteObject(hBitmap);
}

void CConfigShellExtPage::AddToolBarButton(int iCommand,int iBitmap)
{
	TBBUTTON tbButton;
	tbButton.fsState = TBSTATE_ENABLED;
	tbButton.fsStyle = TBSTYLE_BUTTON;
	tbButton.iBitmap = iBitmap;
	tbButton.idCommand = iCommand;
	tbButton.iString = 0;
	tbButton.dwData = 0;
	m_ToolBar.InsertButton(m_ToolBar.GetButtonCount(),&tbButton);
}

void CConfigShellExtPage::CreateToolBarCtrl()
{
	RECT rcListView;
	m_ListView.GetWindowRect(&rcListView);
	ScreenToClient(&rcListView);

	RECT rcToolBar = { 0,0,100,100 };
	m_ToolBar.Create(m_hWnd,rcToolBar,NULL,ATL_SIMPLE_TOOLBAR_PANE_STYLE,NULL);
	m_ToolBar.SetImageList(m_hToolBarImageList);
	m_ToolBar.SetButtonStructSize();

	// Create the buttons.
	AddToolBarButton(ID_SHELLEXT_ADD,1);
	AddToolBarButton(ID_SHELLEXT_REMOVE,3);
	m_ToolBar.EnableButton(ID_SHELLEXT_REMOVE,false);	// Disabled by default.

	// Update the toolbar position.
	int iToolBarWidth = 0;
	RECT rcButton;

	for (int i = 0; i < m_ToolBar.GetButtonCount(); i++)
	{
		m_ToolBar.GetItemRect(i,&rcButton);
		iToolBarWidth += rcButton.right - rcButton.left;
	}

	m_ToolBar.SetWindowPos(NULL,
		rcListView.right - iToolBarWidth,
		rcListView.top - HIWORD(m_ToolBar.GetButtonSize()),
		iToolBarWidth,
		HIWORD(m_ToolBar.GetButtonSize()),0);
}

void CConfigShellExtPage::CheckListView()
{
	int iDelimiter,iLength;
	TCHAR *szString;
	TCHAR *szExt;

	// Open registry.
	CRegistry Reg;
	Reg.SetRoot(HKEY_CLASSES_ROOT);

	int iItemIndex = -1;
	iItemIndex = m_ListView.GetNextItem(iItemIndex,LVNI_ALL);

	while (iItemIndex != -1)
	{
		szString = (TCHAR *)m_ListView.GetItemData(iItemIndex);
		iDelimiter = FirstDelimiter(szString,'|');
		szExt = szString + iDelimiter + 1;

		// Parse the extension list.
		iLength = lstrlen(szExt);
		iDelimiter = 0;

		bool bCheck = true;

		for (int i = 0; i < iLength; i++)
		{
			if (szExt[i] == ',')
			{
				// Skip empty extensions.
				if (iDelimiter == i)
				{
					iDelimiter++;
					continue;
				}

				szExt[i] = '\0';
				//MessageBox(szExt + iDelimiter);
				bCheck = bCheck && IsExtensionInstalled(szExt + iDelimiter,&Reg);
				szExt[i] = ',';

				iDelimiter = i + 1;

				// Automatically left trim the (next) string.
				while (i + 1 < iLength &&
					szExt[i + 1] == ' ')
				{
					iDelimiter++;
					i++;
				}
			}
		}

		//MessageBox(szExt + iDelimiter);
		bCheck = bCheck && IsExtensionInstalled(szExt + iDelimiter,&Reg);

		m_ListView.SetCheckState(iItemIndex,bCheck);

		iItemIndex = m_ListView.GetNextItem(iItemIndex,LVNI_ALL);
	}
}

/*
	CConfigShellExtPage::RegisterListItem
	-------------------------------------
	Registers or unregisters the list view item with the specified item index.
*/
void CConfigShellExtPage::RegisterListItem(CRegistry *pReg,int iItemIndex)
{
	TCHAR *szString = (TCHAR *)m_ListView.GetItemData(iItemIndex);
	int iDelimiter = FirstDelimiter(szString,'|');
	TCHAR *szExt = szString + iDelimiter + 1;

	// Parse the extension list.
	int iLength = lstrlen(szExt);
	iDelimiter = 0;

	bool bRegister = m_ListView.GetCheckState(iItemIndex) == TRUE;

	for (int i = 0; i < iLength; i++)
	{
		if (szExt[i] == ',')
		{
			// Skip empty extensions.
			if (iDelimiter == i)
			{
				iDelimiter++;
				continue;
			}

			szExt[i] = '\0';

			// Register or unregister the file extension.
			if (bRegister)
			{
				// We separate project files from disc images since they need
				// another desciption.
				if (!lstrcmp(szExt,_T(".irp")))
					InstallProjectExtension(pReg);
				else
					InstallExtension(szExt + iDelimiter,pReg);
			}
			else
				UninstallExtension(szExt + iDelimiter,pReg);

			szExt[i] = ',';

			iDelimiter = i + 1;

			// Automatically left trim the (next) string.
			while (i + 1 < iLength &&
				szExt[i + 1] == ' ')
			{
				iDelimiter++;
				i++;
			}
		}
	}

	// Register or unregister the file extension.
	if (bRegister)
	{
		// We separate project files from disc images since they need another desciption.
		if (!lstrcmp(szExt,_T(".irp")))
			InstallProjectExtension(pReg);
		else
			InstallExtension(szExt + iDelimiter,pReg);
	}
	else
		UninstallExtension(szExt + iDelimiter,pReg);
}

/*
	CConfigShellExtPage::RegisterListView
	-------------------------------------
	Registers and unregisters all extensions found in the list view depending on
	the item check state.
*/
void CConfigShellExtPage::RegisterListView()
{
	// Open registry.
	CRegistry Reg;
	Reg.SetRoot(HKEY_CLASSES_ROOT);

	int iItemIndex = -1;
	iItemIndex = m_ListView.GetNextItem(iItemIndex,LVNI_ALL);

	while (iItemIndex != -1)
	{
		// Register or unregister the item.
		RegisterListItem(&Reg,iItemIndex);

		iItemIndex = m_ListView.GetNextItem(iItemIndex,LVNI_ALL);
	}
}

LRESULT CConfigShellExtPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Initialize the list view.
	m_ListView = GetDlgItem(IDC_SHELLEXTLIST);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_ListView.InsertColumn(0,lngGetString(COLUMN_DESCRIPTION),LVCFMT_LEFT,196,0);
	m_ListView.InsertColumn(1,lngGetString(COLUMN_EXTENSIONS),LVCFMT_LEFT,150,0);

	// Fill the list view.
	int iItemCount = 0;
	std::list<tstring>::iterator itNodeObject;
	for (itNodeObject = g_GlobalSettings.m_szShellExt.begin(); itNodeObject != g_GlobalSettings.m_szShellExt.end(); itNodeObject++)
	{
		tstring apa = (*itNodeObject);

		m_ListView.AddItem(iItemCount,0,LPSTR_TEXTCALLBACK);
		m_ListView.AddItem(iItemCount,1,LPSTR_TEXTCALLBACK);
		m_ListView.SetItemData(iItemCount,(DWORD_PTR)(*itNodeObject).c_str());

		iItemCount++;
	}

	// Check the list view items.
	CheckListView();

	// Tool bar.
	InitToolBarImageList();
	CreateToolBarCtrl();

	// Load configuration.
	CheckDlgButton(IDC_SHELLEXTCHECK,IsShellExtensionRegistered());
	CheckDlgButton(IDC_SHELLEXTSUBMENUCHECK,g_GlobalSettings.m_bShellExtSubMenu);
	CheckDlgButton(IDC_SHELLEXTICONCHECK,g_GlobalSettings.m_bShellExtIcon);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CConfigShellExtPage::OnListAdd(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CNewFileExtDlg NewFileExtDlg;

	if (NewFileExtDlg.DoModal() == IDOK)
	{
		TCHAR szBuffer[128];
		lstrcpy(szBuffer,NewFileExtDlg.m_szDescBuffer);
		lstrcat(szBuffer,_T("|"));
		lstrcat(szBuffer,NewFileExtDlg.m_szExtBuffer);

		g_GlobalSettings.m_szShellExt.push_back(szBuffer);

		// Add the list item.
		int iItemCount = m_ListView.GetItemCount();

		m_ListView.AddItem(iItemCount,0,LPSTR_TEXTCALLBACK);
		m_ListView.AddItem(iItemCount,1,LPSTR_TEXTCALLBACK);
		m_ListView.SetItemData(iItemCount,(DWORD_PTR)(*--g_GlobalSettings.m_szShellExt.end()).c_str());
	}

	bHandled = false;
	return 0;
}

LRESULT CConfigShellExtPage::OnListRemove(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	int iSelItem = m_ListView.GetSelectedIndex();

	if (m_ListView.GetCheckState(iSelItem))
	{
		m_ListView.SetCheckState(iSelItem,false);

		// Unregister the item extensions.
		CRegistry Reg;
		Reg.SetRoot(HKEY_CLASSES_ROOT);

		RegisterListItem(&Reg,iSelItem);
	}

	// Remove the list view item.
	const TCHAR *szString = (const TCHAR *)m_ListView.GetItemData(iSelItem);
	m_ListView.DeleteItem(m_ListView.GetSelectedIndex());

	// Locate and remove the data from the global settings.
	std::list<tstring>::iterator itNodeObject;
	for (itNodeObject = g_GlobalSettings.m_szShellExt.begin(); itNodeObject != g_GlobalSettings.m_szShellExt.end(); itNodeObject++)
	{
		if (szString == (*itNodeObject).c_str())
		{
			g_GlobalSettings.m_szShellExt.erase(itNodeObject);
			break;
		}
	}

	bHandled = false;
	return 0;
}

LRESULT CConfigShellExtPage::OnToolBarGetInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;

	// The string ID is the same as the button ID.
	LPTOOLTIPTEXT pTipText = (LPTOOLTIPTEXT)pNMH;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a hint translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("hint")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr((unsigned long)pTipText->hdr.idFrom,szStrValue))
			{
				pTipText->lpszText = szStrValue;
				return 0;
			}
		}
	}

	pTipText->lpszText = MAKEINTRESOURCE(pTipText->hdr.idFrom);
	return 0;
}

LRESULT CConfigShellExtPage::OnListItemChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	if (m_ToolBar.IsWindow())
	{
		if (m_ListView.GetSelectedCount() > 0)
			m_ToolBar.EnableButton(ID_SHELLEXT_REMOVE,true);
		else
			m_ToolBar.EnableButton(ID_SHELLEXT_REMOVE,false);
	}

	bHandled = false;
	return 0;
}

LRESULT CConfigShellExtPage::OnListGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)pNMH;
	const TCHAR *szString = (const TCHAR *)pDispInfo->item.lParam;

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		int iDelimiter = FirstDelimiter(szString,'|');

		switch (pDispInfo->item.iSubItem)
		{
			case 0:
				{
					TCHAR *pDelimiter = (TCHAR *)szString + iDelimiter;

					*pDelimiter = '\0';
					lstrcpy(pDispInfo->item.pszText,szString);
					*pDelimiter = '|';
				}
				break;

			case 1:
				{
					lstrcpy(pDispInfo->item.pszText,(TCHAR *)szString + iDelimiter + 1);
				}
				break;
		}
	}

	bHandled = false;
	return 0;
}
